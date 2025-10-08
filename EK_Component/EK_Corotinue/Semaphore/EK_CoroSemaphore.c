/**
 * @file EK_CoroSemaphore.c
 * @brief 协程信号量模块实现文件
 * @details 实现协程任务间的信号量同步机制，包含动态和静态信号量的创建、删除、
 *          获取和释放功能。支持任务阻塞与唤醒机制，提供超时控制和FIFO公平性保证。
 *          采用队列令牌机制实现信号量计数管理，复用现有的队列系统。
 *          使用双节点机制实现状态-事件分离，与协程调度器完美集成。
 * @author N1ntyNine99
 * @date 2025-10-07
 * @version v1.0
 */

/* ========================= 头文件包含区 ========================= */
#include "EK_CoroSemaphore.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_SEMAPHORE_ENABLE == 1)

#include "../Task/EK_CoroTask.h"

/* ========================= 内部辅助宏 ========================= */
/**
 * @brief 安全获取信号量的队列指针
 * @details 根据创建类型返回正确的队列指针，支持动态和静态创建的统一访问
 */
#define EK_SEM_GET_QUEUE(sem) ((sem)->Sem_isDynamic ? (sem)->Sem_Queue : &(sem)->Sem_QueueStatic)

/**
 * @brief 信号量令牌大小定义
 * @details 每个令牌占用1字节，令牌值本身不重要，只关心数量
 */
#define EK_SEM_TOKEN_SIZE (sizeof(uint8_t)) // 信号量的令牌大小 1 byte

/* ========================= 内部函数 ========================= */

/**
 * @brief 内部信号量释放函数（非阻塞版本）
 * @details 向信号量队列添加一个令牌，不处理等待协程的唤醒。
 *          此函数为纯队列操作，用于内部实现，不检查等待列表。
 *
 * @param sem 信号量指针
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功添加令牌
 *         - EK_FULL: 队列已满
 *         - EK_INVALID_PARAM: 参数无效
 */
static inline EK_Result_t r_sem_give(EK_CoroSem_t *sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_INVALID_PARAM;

    EK_ENTER_CRITICAL();

    // 获取当前信号量的队列
    EK_Queue_t *queue = EK_SEM_GET_QUEUE(sem);

    // 检查队列是否已满
    if (EK_bQueueIsFull(queue))
    {
        EK_EXIT_CRITICAL();
        return EK_FULL;
    }

    // 使用临时令牌变量
    uint8_t token = 1; // 令牌值可以是任意非零值，只关心数量
    EK_Result_t res = EK_rQueueEnqueue(queue, &token, EK_SEM_TOKEN_SIZE);

    EK_EXIT_CRITICAL();

    return res;
}

/**
 * @brief 内部信号量获取函数（非阻塞版本）
 * @details 从信号量队列取出一个令牌，不处理协程阻塞逻辑。
 *          此函数为纯队列操作，用于内部实现，不进行阻塞等待。
 *
 * @param sem 信号量指针
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功获取令牌
 *         - EK_EMPTY: 队列为空
 *         - EK_INVALID_PARAM: 参数无效
 */
static inline EK_Result_t r_sem_take(EK_CoroSem_t *sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_INVALID_PARAM;

    EK_ENTER_CRITICAL();

    // 获取当前信号量的队列
    EK_Queue_t *queue = EK_SEM_GET_QUEUE(sem);

    // 检查队列是否为空
    if (EK_bQueueIsEmpty(queue))
    {
        EK_EXIT_CRITICAL();
        return EK_EMPTY;
    }

    // 使用临时令牌变量
    uint8_t token;
    EK_Result_t res = EK_rQueueDequeue(queue, &token, EK_SEM_TOKEN_SIZE);

    EK_EXIT_CRITICAL();

    return res;
}

/**
 * @brief 内部信号量阻塞函数
 * @details 将协程添加到信号量的等待列表并设置阻塞状态。
 *          使用事件节点进行管理，支持超时控制和优先级调度。
 *
 * @param sem 信号量指针
 * @param tcb 要阻塞的协程TCB指针
 * @param timeout 阻塞超时时间
 */
static inline void v_sem_delay(EK_CoroSem_t *sem, EK_CoroTCB_t *tcb, uint32_t timeout)
{
    if (timeout == 0 || sem == NULL || tcb == NULL) return;

    EK_ENTER_CRITICAL();

    // 设置事件结果为等待中
    tcb->TCB_EventResult = EK_CORO_EVENT_PENDING;

    // 将当前TCB的事件节点加入信号量等待列表
    EK_rKernelMove_Prio(&sem->Sem_WaitList, &tcb->TCB_EventNode);

    EK_EXIT_CRITICAL();

    // 任务阻塞等待
    EK_vCoroDelay(timeout);
}

/**
 * @brief 内部信号量唤醒结果处理函数
 * @details 检查协程被唤醒的原因，处理超时和删除情况。
 *          对于超时情况，需要从等待列表中移除协程节点。
 *
 * @param sem 信号量指针
 * @param tcb 被唤醒的协程TCB指针
 * @return EK_Result_t 唤醒结果：
 *         - EK_OK: 成功获取信号量
 *         - EK_TIMEOUT: 等待超时
 *         - EK_NOT_FOUND: 信号量被删除
 *         - EK_ERROR: 其他错误
 */
static inline EK_Result_t r_sem_wake(EK_CoroSem_t *sem, EK_CoroTCB_t *tcb)
{
    if (tcb->TCB_EventResult == EK_CORO_EVENT_OK)
    {
        return EK_OK;
    }
    else if (tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT)
    {
        // 超时情况：从等待列表中移除事件节点
        EK_rKernelRemove(&sem->Sem_WaitList, &tcb->TCB_EventNode);
        return EK_TIMEOUT;
    }
    else if (tcb->TCB_EventResult == EK_CORO_EVENT_DELETED)
    {
        return EK_NOT_FOUND;
    }
    else
    {
        return EK_ERROR;
    }
}
/* ========================= 公开API ========================= */

/**
 * @brief 动态创建一个信号量
 * @details 使用动态内存分配创建信号量对象，初始化指定数量的令牌。
 *          动态创建的信号量需要在使用后手动删除释放内存。
 *          适用于运行时动态创建同步对象的场景。
 *
 * @param init_count 初始令牌数量（信号量初始计数值）
 * @param max_count 最大令牌数量（信号量最大计数值）
 *
 * @return EK_CoroSemHanlder_t 成功时返回信号量句柄，失败返回NULL
 */
EK_CoroSemHanlder_t EK_pSemCreate(uint16_t init_count, uint16_t max_count)
{
    // 参数有效性检查：最大值不能为0
    if (max_count == 0) return NULL;

    EK_ENTER_CRITICAL();

    // 动态创建信号量结构体
    EK_CoroSem_t *sem = (EK_CoroSem_t *)EK_CORO_MALLOC(sizeof(EK_CoroSem_t));
    if (sem == NULL)
    {
        EK_EXIT_CRITICAL();
        return NULL;
    }

    // 动态创建底层队列
    EK_Queue_t *queue = EK_pQueueCreate(max_count * EK_SEM_TOKEN_SIZE);
    if (queue == NULL)
    {
        EK_CORO_FREE(sem);
        EK_EXIT_CRITICAL();
        return NULL;
    }

    // 初始化结构体成员
    sem->Sem_Queue = queue; // 设置动态队列指针
    sem->Sem_MaxCount = max_count; // 设置最大令牌数
    sem->Sem_isDynamic = true; // 标记为动态创建

    // 初始化等待列表
    sem->Sem_WaitList.List_Count = 0;
    sem->Sem_WaitList.List_Head = NULL;
    sem->Sem_WaitList.List_Tail = NULL;

    // 批量添加初始令牌
    if (init_count != 0)
    {
        for (uint16_t i = 0; i < init_count; i++)
        {
            EK_Result_t res = r_sem_give(sem);
            if (res != EK_OK)
            {
                // 初始化失败，清理已分配的资源
                EK_rQueueDelete(queue);
                EK_CORO_FREE(sem);
                EK_EXIT_CRITICAL();
                return NULL;
            }
        }
    }

    EK_EXIT_CRITICAL();

    return (EK_CoroSemHanlder_t)sem;
}

/**
 * @brief 静态创建一个信号量
 * @details 使用用户提供的缓冲区创建信号量，完全避免动态内存分配。
 *          静态创建的信号量具有确定性内存使用特性，适合资源受限的嵌入式环境。
 *          使用内嵌队列结构体，确保真正的静态创建。
 *
 * @param sem 指向用户提供的静态信号量结构体的指针
 * @param buffer 指向用户提供的静态缓冲区，用于存储令牌
 * @param init_count 初始令牌数量（信号量初始计数值）
 * @param max_count 最大令牌数量（信号量最大计数值）
 *
 * @return EK_CoroSemStaticHanlder_t 成功时返回信号量句柄，失败返回NULL
 */
EK_CoroSemStaticHanlder_t EK_pSemCreateStatic(EK_CoroSem_t *sem, void *buffer, uint32_t init_count, uint32_t max_count)
{
    // 参数有效性检查
    if (max_count == 0 || sem == NULL || buffer == NULL) return NULL;

    EK_ENTER_CRITICAL();

    // 使用内嵌队列结构体创建静态队列
    EK_Result_t res = EK_pQueueCreateStatic(&sem->Sem_QueueStatic, buffer, max_count * EK_SEM_TOKEN_SIZE);
    if (res != EK_OK)
    {
        EK_EXIT_CRITICAL();
        return NULL;
    }

    // 初始化结构体成员
    sem->Sem_MaxCount = max_count; // 设置最大令牌数
    sem->Sem_isDynamic = false; // 标记为静态创建

    // 初始化等待列表
    sem->Sem_WaitList.List_Count = 0;
    sem->Sem_WaitList.List_Head = NULL;
    sem->Sem_WaitList.List_Tail = NULL;

    // 批量添加初始令牌
    if (init_count != 0)
    {
        for (uint32_t i = 0; i < init_count; i++)
        {
            res = r_sem_give(sem);
            if (res != EK_OK)
            {
                EK_EXIT_CRITICAL();
                return NULL;
            }
        }
    }

    EK_EXIT_CRITICAL();

    return (EK_CoroSemStaticHanlder_t)sem;
}

/**
 * @brief 获取信号量（P操作）
 * @details 尝试从信号量获取一个令牌。实现快速路径和慢速路径分离：
 *          1. 快速路径：如果有可用令牌，立即获取并返回成功
 *          2. 慢速路径：如果无可用令牌，根据超时设置选择阻塞等待或立即返回
 *          支持FIFO公平性，按照请求顺序唤醒等待协程。
 *
 * @param sem 信号量句柄
 * @param timeout 等待超时时间（tick数），0表示不等待，EK_MAX_DELAY表示永久等待
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功获取信号量
 *         - EK_EMPTY: 无可用令牌且不等待
 *         - EK_TIMEOUT: 等待超时
 *         - EK_NULL_POINTER: 参数无效
 */
EK_Result_t EK_rSemTake(EK_CoroSemHanlder_t sem, uint32_t timeout)
{
    // 参数有效性检查
    if (sem == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    EK_Result_t res;

    // 快速路径：尝试从队列获取令牌
    res = r_sem_take(sem);

    // 成功获取到信号量
    if (res == EK_OK)
    {
        EK_EXIT_CRITICAL();
        return EK_OK;
    }
    // 队列为空，需要处理等待逻辑
    else if (res == EK_EMPTY)
    {
        // 不需要等待，直接返回失败
        if (timeout == 0)
        {
            EK_EXIT_CRITICAL();
            return EK_EMPTY;
        }
        // 需要阻塞等待
        else
        {
            // 获取当前协程的TCB
            EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB();
            EK_EXIT_CRITICAL();

            // 进入阻塞等待状态
            v_sem_delay(sem, current_tcb, timeout);

            // 协程被唤醒后，检查唤醒原因
            return r_sem_wake(sem, current_tcb);
        }
    }

    // 未知错误情况
    EK_EXIT_CRITICAL();
    return EK_UNKNOWN;
}

/**
 * @brief 释放信号量（V操作）
 * @details 向信号量释放一个令牌。实现汇合操作：
 *          1. 如果有协程正在等待获取信号量，直接唤醒等待时间最长的协程
 *          2. 如果没有等待协程，将令牌放回队列中增加信号量计数
 *          支持优先级调度和FIFO公平性保证。
 *
 * @param sem 信号量句柄
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功释放信号量
 *         - EK_NULL_POINTER: 参数无效
 *         - EK_FULL: 信号量已满（理论上不应发生）
 */
EK_Result_t EK_rSemGive(EK_CoroSemHanlder_t sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    // 优先级1：有协程在等待信号量，直接唤醒等待时间最长的协程（FIFO）
    if (sem->Sem_WaitList.List_Count > 0)
    {
        // 获取等待链表头部的协程（等待时间最长）
        EK_CoroListNode_t *wait_node = sem->Sem_WaitList.List_Head;
        EK_CoroTCB_t *wait_tcb = (EK_CoroTCB_t *)wait_node->CoroNode_Owner;

        // 从等待列表中移除该协程
        EK_rKernelRemove(&sem->Sem_WaitList, wait_node);

        // 设置唤醒原因和状态
        wait_tcb->TCB_EventResult = EK_CORO_EVENT_OK;
        wait_tcb->TCB_State = EK_CORO_READY;

        // 加入对应优先级的就绪列表
        EK_rKernelInsert_Prio(EK_pKernelGetReadyList(wait_tcb->TCB_Priority), &wait_tcb->TCB_StateNode);

        EK_EXIT_CRITICAL();
        return EK_OK;
    }
    // 优先级2：没有协程等待，直接将令牌放回队列
    else
    {
        EK_EXIT_CRITICAL();
        return r_sem_give(sem);
    }
}

#endif /* EK_CORO_SEMAPHORE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */
