/**
 * @file EK_CoroSemaphore.c
 * @brief 协程信号量模块实现文件
 * @details 实现协程任务间的信号量同步机制，包含动态和静态信号量的创建、删除、
 *          获取和释放功能。支持任务阻塞与唤醒机制，提供超时控制和FIFO公平性保证。
 *          采用轻量级计数器模式，大幅简化实现并提升性能。
 *          使用双节点机制实现状态-事件分离，与协程调度器完美集成。
 * @author N1ntyNine99
 * @date 2025-10-10
 * @version v2.0
 */

/* ========================= 头文件包含区 ========================= */
#include "EK_CoroSemaphore.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_SEMAPHORE_ENABLE == 1)

#include "../Task/EK_CoroTask.h"

/* ========================= 内部函数 ========================= */

/**
 * @brief 内部信号量释放函数（非阻塞版本）
 * @details 增加信号量计数，不处理等待协程的唤醒。
 *          此函数为纯计数操作，用于内部实现，不检查等待列表。
 *
 * @param sem 信号量指针
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功增加计数
 *         - EK_FULL: 信号量已满
 *         - EK_INVALID_PARAM: 参数无效
 */
ALWAYS_STATIC_INLINE EK_Result_t r_sem_give(EK_CoroSem_t *sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_INVALID_PARAM;

    // 检查是否已达到最大值
    if (sem->Sem_Count >= sem->Sem_MaxCount)
    {
        return EK_FULL;
    }

    // 简单增加计数
    sem->Sem_Count++;
    return EK_OK;
}

/**
 * @brief 内部信号量获取函数（非阻塞版本）
 * @details 减少信号量计数，不处理协程阻塞逻辑。
 *          此函数为纯计数操作，用于内部实现，不进行阻塞等待。
 *
 * @param sem 信号量指针
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功减少计数
 *         - EK_EMPTY: 信号量为空
 *         - EK_INVALID_PARAM: 参数无效
 */
ALWAYS_STATIC_INLINE EK_Result_t r_sem_take(EK_CoroSem_t *sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_INVALID_PARAM;

    // 检查是否有可用信号量
    if (sem->Sem_Count == 0)
    {
        return EK_EMPTY;
    }

    // 简单减少计数
    sem->Sem_Count--;
    return EK_OK;
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
ALWAYS_STATIC_INLINE void v_sem_delay(EK_CoroSem_t *sem, EK_CoroTCB_t *tcb, uint32_t timeout)
{
    if (timeout == 0 || sem == NULL || tcb == NULL) return;

    // 设置事件结果为等待中
    tcb->TCB_EventResult = EK_CORO_EVENT_PENDING;

    // 将当前TCB的事件节点加入信号量等待列表
    EK_rKernelMove_Prio(&sem->Sem_WaitList, &tcb->TCB_EventNode);

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
ALWAYS_STATIC_INLINE EK_Result_t r_sem_wake(EK_CoroSem_t *sem, EK_CoroTCB_t *tcb)
{
    tcb->TCB_WakeUpTime = EK_uKernelGetTick();
    if (tcb->TCB_EventResult == EK_CORO_EVENT_OK)
    {
        return EK_OK;
    }
    else if (tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT)
    {
        // 超时情况：从等待列表中移除事件节点（若仍在等待链表中）
        if (tcb->TCB_EventNode.CoroNode_List == &sem->Sem_WaitList)
        {
            EK_rKernelRemove(&sem->Sem_WaitList, &tcb->TCB_EventNode);
        }
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

/**
 * @brief 从信号量等待列表中取出一个等待的协程
 * @details 从指定的等待列表中取出第一个等待的协程任务控制块，
 *          并将该协程从等待列表中移除。通常在有信号量可用时调用。
 * @param sem 等待操作的信号量
 * @return EK_CoroTCB_t* 指向取出的协程任务控制块的指针，如果列表为空则返回NULL
 */
ALWAYS_STATIC_INLINE EK_CoroTCB_t *p_sem_take_waiter(EK_CoroSem_t *sem)
{
    if (sem == NULL || sem->Sem_WaitList.List_Count == 0) return NULL;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)sem->Sem_WaitList.List_Head->CoroNode_Owner;
    EK_rKernelRemove(&sem->Sem_WaitList, &tcb->TCB_EventNode);

    return tcb;
}

/**
 * @brief 唤醒等待信号量的协程
 * @details 将指定的协程任务设置为就绪状态，设置事件结果，
 *          并将其移动到对应优先级的就绪队列末尾，准备被调度执行。
 *          通常在信号量可用且需要唤醒等待协程时调用。
 * @param tcb 指向要唤醒的协程任务控制块的指针
 * @param result 事件结果，通常为EK_CORO_OK或EK_CORO_TIMEOUT
 */
ALWAYS_STATIC_INLINE void v_sem_wake(EK_CoroTCB_t *tcb, EK_CoroEventResult_t result)
{
    if (tcb == NULL) return;

    tcb->TCB_EventResult = result;
    tcb->TCB_State = EK_CORO_READY;
    EK_rKernelMove_Head(EK_pKernelGetReadyList(tcb->TCB_Priority), &tcb->TCB_StateNode);
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

    // 初始化结构体成员
    sem->Sem_Count = (init_count > max_count) ? max_count : init_count;
    sem->Sem_MaxCount = max_count;
    sem->Sem_isDynamic = true;

    // 初始化等待列表
    sem->Sem_WaitList.List_Count = 0;
    sem->Sem_WaitList.List_Head = NULL;
    sem->Sem_WaitList.List_Tail = NULL;

    EK_EXIT_CRITICAL();

    return (EK_CoroSemHanlder_t)sem;
}

/**
 * @brief 静态创建一个信号量
 * @details 使用用户提供的信号量结构体创建信号量，完全避免动态内存分配。
 *          静态创建的信号量具有确定性内存使用特性，适合资源受限的嵌入式环境。
 *
 * @param sem 指向用户提供的静态信号量结构体的指针
 * @param init_count 初始令牌数量（信号量初始计数值）
 * @param max_count 最大令牌数量（信号量最大计数值）
 *
 * @return EK_CoroSemStaticHanlder_t 成功时返回信号量句柄，失败返回NULL
 */
EK_CoroSemStaticHanlder_t EK_pSemCreateStatic(EK_CoroSem_t *sem, uint32_t init_count, uint32_t max_count)
{
    // 参数有效性检查
    if (max_count == 0 || sem == NULL) return NULL;

    EK_ENTER_CRITICAL();

    // 初始化结构体成员
    sem->Sem_Count = (init_count > max_count) ? max_count : init_count;
    sem->Sem_MaxCount = max_count;
    sem->Sem_isDynamic = false;

    // 初始化等待列表
    sem->Sem_WaitList.List_Count = 0;
    sem->Sem_WaitList.List_Head = NULL;
    sem->Sem_WaitList.List_Tail = NULL;

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
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;
    if (sem == NULL) return EK_NULL_POINTER;
    if (EK_pKernelGetCurrentTCB() == EK_pKernelGetIdleHandler()) return EK_INVALID_PARAM;

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 当前的TCB

    while (1)
    {
        EK_ENTER_CRITICAL();

        // 当前信号量中有可用令牌
        if (sem->Sem_Count > 0)
        {
            EK_EXIT_CRITICAL();
            return r_sem_take(sem);
        }

        // 没有可用信号量
        EK_EXIT_CRITICAL();

        // 不阻塞
        if (timeout == 0)
        {
            return EK_EMPTY;
        }

        // 阻塞
        v_sem_delay(sem, current_tcb, timeout);

        // --从这里唤醒--

        // 超时并且没有对应的信号量 退出
        if (current_tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT && sem->Sem_Count == 0)
        {
            return EK_TIMEOUT;
        }

        // 否则重试
    }
}

/**
 * @brief 释放信号量（V操作）
 * @details 向信号量释放一个令牌。实现汇合操作：
 *          1. 如果有协程正在等待获取信号量，直接唤醒等待时间最长的协程
 *          2. 如果没有等待协程，增加信号量计数
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

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 获取当前的TCB

    EK_ENTER_CRITICAL();

    // 有协程在等待信号量，直接唤醒等待时间最长的协程（FIFO）
    if (sem->Sem_WaitList.List_Count > 0)
    {
        // 获取等待链表头部的协程（等待时间最长）
        EK_CoroTCB_t *wait_tcb = p_sem_take_waiter(sem);

        // 设置唤醒原因并加入就绪链表
        v_sem_wake(wait_tcb, EK_CORO_EVENT_OK);

        EK_EXIT_CRITICAL();

        // 唤醒的任务的优先级高于当前的任务 主动上下文切换一次
        if (wait_tcb->TCB_Priority < current_tcb->TCB_Priority) EK_vCoroYield();

        return EK_OK;
    }
    // 没有协程等待，直接增加计数
    else
    {
        EK_Result_t res = r_sem_give(sem);
        EK_EXIT_CRITICAL();
        return res;
    }
}

/**
 * @brief 清空信号量中的令牌并唤醒等待任务
 * @details 将信号量计数重置为0，所有因等待令牌而阻塞的任务将以超时状态返回，
 *          方便上层逻辑执行错误恢复或重新尝试。
 * @param sem 信号量句柄
 * @return EK_Result_t 操作结果（EK_OK, EK_NULL_POINTER, EK_ERROR 等）
 */
EK_Result_t EK_rSemClean(EK_CoroSemHanlder_t sem)
{
    // 参数有效性检查
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;
    if (sem == NULL) return EK_NULL_POINTER;

    bool need_yield = false; // 是否需要山下文切换

    EK_ENTER_CRITICAL();

    // 重置信号量计数
    sem->Sem_Count = 0;

    // 唤醒等待信号量的任务
    EK_CoroTCB_t *wait_tcb = NULL;
    while ((wait_tcb = p_sem_take_waiter(sem)) != NULL)
    {
        v_sem_wake(wait_tcb, EK_CORO_EVENT_TIMEOUT);
        need_yield = true;
    }

    EK_EXIT_CRITICAL();

    if (need_yield)
    {
        EK_vKernelYield();
    }

    return EK_OK;
}

/**
 * @brief 删除信号量
 * @details 唤醒所有等待任务并释放底层资源，动态创建的信号量会释放控制块。
 * @param sem 要删除的信号量指针
 * @return EK_Result_t 操作结果（EK_OK 或错误码）
 */
EK_Result_t EK_rSemDelete(EK_CoroSem_t *sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_NULL_POINTER;

    bool need_yield = false; //是否需要上下文切换

    EK_ENTER_CRITICAL();

    // 唤醒所有等待信号量的任务
    EK_CoroTCB_t *wait_tcb = NULL;
    while ((wait_tcb = p_sem_take_waiter(sem)) != NULL)
    {
        v_sem_wake(wait_tcb, EK_CORO_EVENT_DELETED);
        need_yield = true;
    }

    // 根据是否动态来删除信号量
    bool is_dynamic = sem->Sem_isDynamic;

    if (is_dynamic)
    {
        EK_CORO_FREE(sem);
    }

    EK_EXIT_CRITICAL();

    if (need_yield)
    {
        EK_vKernelYield();
    }

    return EK_OK;
}

#endif /* EK_CORO_SEMAPHORE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */