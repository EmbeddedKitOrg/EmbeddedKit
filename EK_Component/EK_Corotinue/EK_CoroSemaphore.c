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
#include "./Inc/EK_CoroSemaphore.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_SEMAPHORE_ENABLE == 1)

#include "../Task/Inc/EK_CoroTask.h"

/* ========================= 内部函数 ========================= */

#if (EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1)

/**
 * @brief 继承当前任务的优先级
 *
 * 当高优先级任务等待互斥锁时，该函数将提升互斥锁持有者的优先级
 * 以避免优先级反转问题。该函数仅在互斥锁模式下有效。
 *
 * @param sem 指向互斥锁信号量的指针
 * @return EK_Result_t 操作结果
 *         - EK_OK: 优先级继承成功
 *         - EK_NULL_POINTER: 信号量指针为空
 *         - EK_INVALID_PARAM: 参数无效（非互斥锁或持有者为空）
 *
 * @note 该函数会检查等待链表，如果有更高优先级的任务在等待，
 *       则将互斥锁持有者的优先级提升到最高等待任务的优先级
 */
ALWAYS_STATIC_INLINE EK_Result_t r_mutex_inherit_priority(EK_CoroSem_t *sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_NULL_POINTER;
    if (sem->Mutex_Holder == NULL || sem->Sem_isMutex == false) return EK_INVALID_PARAM;

    // 先保存原始优先级（如果还未保存）
    if (sem->Mutex_OriginalPriority == -1)
    {
        sem->Mutex_OriginalPriority = sem->Mutex_Holder->TCB_Priority;
    }

    // 如果等待链表非空
    if (EK_bKernelListIsEmpty(&sem->Sem_WaitList) != true)
    {
        // 等待链表已按优先级排序，头部即为最高优先级任务
        uint8_t highest_prio =
            ((EK_CoroTCB_t *)(EK_pKernelListGetFirst(&sem->Sem_WaitList)->CoroNode_Owner))->TCB_Priority;

        // 优先级继承
        if (highest_prio < sem->Mutex_Holder->TCB_Priority)
        {
            EK_CoroTCB_t *holder = sem->Mutex_Holder;

            holder->TCB_Priority = highest_prio;

            // 如果任务在就绪态，插入到新优先级链表
            if (holder->TCB_State == EK_CORO_READY)
            {
                EK_rKernelMove_Head(EK_pKernelGetReadyList(highest_prio), &holder->TCB_StateNode);
            }
        }
    }

    return EK_OK;
}

/**
 * @brief 恢复当前任务的优先级
 *
 * 当互斥锁被释放时，该函数将互斥锁持有者的优先级恢复到原始值
 * 以完成优先级继承机制。该函数仅在互斥锁模式下有效。
 *
 * @param sem 指向互斥锁信号量的指针
 * @return EK_Result_t 操作结果
 *         - EK_OK: 优先级恢复成功
 *         - EK_NULL_POINTER: 信号量指针为空
 *         - EK_INVALID_PARAM: 参数无效（非互斥锁、持有者为空或原始优先级未设置）
 *
 * @note 该函数仅在任务持有互斥锁且原始优先级已保存时才执行恢复操作
 *       恢复后会将任务重新插入到对应优先级的就绪链表中（如果任务处于就绪态）
 */
ALWAYS_STATIC_INLINE EK_Result_t r_mutex_restore_priority(EK_CoroSem_t *sem)
{
    // 参数有效性检查
    if (sem == NULL) return EK_NULL_POINTER;
    if (sem->Mutex_Holder == NULL || sem->Sem_isMutex == false || sem->Mutex_OriginalPriority == -1)
    {
        return EK_INVALID_PARAM;
    }

    EK_CoroTCB_t *holder = sem->Mutex_Holder; // 获取持有者TCB
    uint8_t current_prio = holder->TCB_Priority; // 当前任务的优先级
    uint8_t original_prio = sem->Mutex_OriginalPriority; // 原优先级

    // 发生了优先级继承 要恢复
    if (current_prio != original_prio)
    {
        holder->TCB_Priority = original_prio;

        // 如果任务在就绪态，插入到原始优先级链表
        if (holder->TCB_State == EK_CORO_READY)
        {
            EK_rKernelMove_Head(EK_pKernelGetReadyList(original_prio), &holder->TCB_StateNode);
        }
    }

    sem->Mutex_OriginalPriority = -1;
    return EK_OK;
}

#endif /* EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1 */

/**
 * @brief 内部信号量释放函数（非阻塞版本）
 * @details 增加信号量计数，不处理等待协程的唤醒。
 *          此函数为纯计数操作，用于内部实现，不检查等待列表。
 *          仅进行最基本的计数增加操作，不处理任何互斥锁逻辑。
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
    if (sem == NULL) return EK_NULL_POINTER;

    // 检查是否达到最大值
    if (sem->Sem_Count >= sem->Sem_MaxCount) return EK_FULL;

    // 简单增加计数
    sem->Sem_Count++;
    return EK_OK;
}

/**
 * @brief 内部信号量获取函数（非阻塞版本）
 * @details 减少信号量计数，不处理协程阻塞逻辑。
 *          此函数为纯计数操作，用于内部实现，不进行阻塞等待。
 *          仅进行最基本的计数减少操作，不处理任何互斥锁逻辑。
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
    if (sem == NULL) return EK_NULL_POINTER;

    // 检查是否有可用信号量
    if (sem->Sem_Count == 0) return EK_EMPTY;

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
 * @brief 从信号量等待列表中取出一个等待的协程
 * @details 从指定的等待列表中取出第一个等待的协程任务控制块，
 *          并将该协程从等待列表中移除。通常在有信号量可用时调用。
 * @param sem 等待操作的信号量
 * @return EK_CoroTCB_t* 指向取出的协程任务控制块的指针，如果列表为空则返回NULL
 */
ALWAYS_STATIC_INLINE EK_CoroTCB_t *p_sem_take_waiter(EK_CoroSem_t *sem)
{
    if (sem == NULL || sem->Sem_WaitList.List_Count == 0) return NULL;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)EK_pKernelListGetFirst(&sem->Sem_WaitList)->CoroNode_Owner;
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
 * @param is_mutex 是否是互斥量
 * @param is_recursive 是否是递归互斥
 *
 * @return EK_CoroSem_t * 成功时返回信号量句柄，失败返回NULL
 */
EK_CoroSem_t *EK_pSemGenericCreate(uint16_t init_count, uint16_t max_count, bool is_mutex, bool is_recursive)
{
    // 参数有效性检查：最大值不能为0
    if (max_count == 0) return NULL;

#if (EK_CORO_MUTEX_ENABLE == 0)
    UNUSED_VAR(is_mutex);
#endif /* EK_CORO_MUTEX_ENABLE == 0 */

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

    // 初始化互斥量相关
#if (EK_CORO_MUTEX_ENABLE == 1)
    sem->Sem_isMutex = is_mutex;
    sem->Mutex_Holder = NULL;

    // 递归互斥量
    sem->Mutex_RecursiveCount = 0;
    sem->Mutex_isRecursive = is_recursive;

#if (EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1)
    sem->Mutex_OriginalPriority = -1;
#endif /* EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1 */
#endif /* EK_CORO_MUTEX_ENABLE == 1 */

    // 初始化等待列表
    EK_vKernelListInit(&sem->Sem_WaitList);

    EK_EXIT_CRITICAL();

    return (EK_CoroSemHanlder_t)sem; // 使用统一的信号量句柄类型
}

/**
 * @brief 静态创建一个信号量
 * @details 使用用户提供的信号量结构体创建信号量，完全避免动态内存分配。
 *          静态创建的信号量具有确定性内存使用特性，适合资源受限的嵌入式环境。
 *
 * @param sem 指向用户提供的静态信号量结构体的指针
 * @param init_count 初始令牌数量（信号量初始计数值）
 * @param max_count 最大令牌数量（信号量最大计数值）
 * @param is_mutex 是否是互斥量
 * @param is_recursive 是否是递归量
 *
 * @return EK_CoroSem_t * 成功时返回信号量句柄，失败返回NULL
 */
EK_CoroSem_t *
EK_pSemGenericCreateStatic(EK_CoroSem_t *sem, uint32_t init_count, uint32_t max_count, bool is_mutex, bool is_recursive)
{
    // 参数有效性检查
    if (max_count == 0 || sem == NULL) return NULL;

#if (EK_CORO_MUTEX_ENABLE == 0)
    UNUSED_VAR(is_mutex);
#endif /* EK_CORO_MUTEX_ENABLE == 0 */

    EK_ENTER_CRITICAL();

    // 初始化结构体成员
    sem->Sem_Count = (init_count > max_count) ? max_count : init_count;
    sem->Sem_MaxCount = max_count;
    sem->Sem_isDynamic = false;

    // 初始化互斥量相关
#if (EK_CORO_MUTEX_ENABLE == 1)
    sem->Sem_isMutex = is_mutex;
    sem->Mutex_Holder = NULL;

    // 递归互斥量
    sem->Mutex_RecursiveCount = 0;
    sem->Mutex_isRecursive = is_recursive;

#if (EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1)
    sem->Mutex_OriginalPriority = -1;
#endif /* EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1 */
#endif /* EK_CORO_MUTEX_ENABLE == 1 */

    // 初始化等待列表
    // 初始化等待列表
    EK_vKernelListInit(&sem->Sem_WaitList);

    EK_EXIT_CRITICAL();

    return (EK_CoroSemStaticHanlder_t)sem; // 使用统一的信号量句柄类型
}

/**
 * @brief 获取信号量（P操作）
 * @details 尝试从信号量获取一个令牌。实现快速路径和慢速路径分离：
 *          1. 快速路径：如果有可用令牌，立即获取并返回成功
 *          2. 慢速路径：如果无可用令牌，根据超时设置选择阻塞等待或立即返回
 *          支持FIFO公平性，按照请求顺序唤醒等待协程。
 *          处理互斥锁的持有者设置、递归获取和优先级继承逻辑。
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
    EK_Result_t op_res;

    while (1)
    {
        EK_ENTER_CRITICAL();

        // 处理互斥锁相关逻辑
#if (EK_CORO_MUTEX_ENABLE == 1)
        if (sem->Sem_isMutex == true)
        {
            // 递归互斥锁的特殊处理
            if (sem->Mutex_isRecursive == true && current_tcb == sem->Mutex_Holder)
            {
                // 递归获取：增加递归计数器，但不减少信号量计数
                sem->Mutex_RecursiveCount++;
                EK_EXIT_CRITICAL();
                return EK_OK; // 直接返回成功，不需要继续处理信号量计数
            }

            // 对于互斥锁，如果当前任务已经是持有者，不需要处理信号量计数
            if (current_tcb == sem->Mutex_Holder)
            {
                EK_EXIT_CRITICAL();
                return EK_OK;
            }
        }
#endif /* EK_CORO_MUTEX_ENABLE == 1 */

        // 检查信号量计数
        if (sem->Sem_Count > 0)
        {
            // 调用底层函数减少计数
            op_res = r_sem_take(sem);
            if (op_res == EK_OK)
            {
                // 处理互斥锁相关逻辑
#if (EK_CORO_MUTEX_ENABLE == 1)
                if (sem->Sem_isMutex == true)
                {
                    // 设置持有者
                    sem->Mutex_Holder = current_tcb;

                    // 处理递归互斥锁初始化
                    if (sem->Mutex_isRecursive == true)
                    {
                        sem->Mutex_RecursiveCount = 1;
                    }

                    // 处理优先级继承
#if (EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1)
                    op_res = r_mutex_inherit_priority(sem);
                    if (op_res != EK_OK)
                    {
                        EK_EXIT_CRITICAL();
                        return op_res;
                    }
#endif /* EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1 */
                }
#endif /* EK_CORO_MUTEX_ENABLE == 1 */

                EK_EXIT_CRITICAL();
                return EK_OK;
            }
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
 *          处理互斥锁的持有者检查、递归释放和优先级恢复逻辑。
 *
 * @param sem 信号量句柄
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功释放信号量
 *         - EK_NULL_POINTER: 参数无效
 *         - EK_FULL: 信号量已满（理论上不应发生）
 *         - EK_ERROR: 互斥量持有者不匹配
 */
EK_Result_t EK_rSemGive(EK_CoroSemHanlder_t sem)
{
    // 参数有效性检查
    if (EK_IS_IN_INTERRUPT() == true) EK_ERROR;
    if (sem == NULL) return EK_NULL_POINTER;

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 获取当前的TCB
    EK_Result_t op_res;
    bool sem_count_plus_flag = true; // 是否需要增加信号量计数

    EK_ENTER_CRITICAL();

    // 处理互斥锁相关逻辑
#if (EK_CORO_MUTEX_ENABLE == 1)
    if (sem->Sem_isMutex == true)
    {
        // 检查持有者：只有持有者才可以给出互斥量
        if (sem->Mutex_Holder != current_tcb)
        {
            EK_EXIT_CRITICAL();
            return EK_ERROR;
        }

        // 处理递归互斥锁
        if (sem->Mutex_isRecursive == true)
        {
            // 递归释放：减少递归计数器
            sem->Mutex_RecursiveCount--;

            // 如果递归计数器仍然大于0，说明还在持有锁，不增加信号量计数
            if (sem->Mutex_RecursiveCount > 0)
            {
                sem_count_plus_flag = false;
            }
        }

        // 处理优先级恢复
#if (EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1)
        // 只有当递归计数器为0或非递归互斥锁时才恢复优先级
        if (sem->Mutex_isRecursive == false || sem->Mutex_RecursiveCount == 0)
        {
            op_res = r_mutex_restore_priority(sem);
            if (op_res != EK_OK)
            {
                EK_EXIT_CRITICAL();
                return op_res;
            }
        }
#endif /* EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1 */

        // 只有当递归计数器为0或非递归互斥锁时才清除持有者
        if (sem->Mutex_isRecursive == false || sem->Mutex_RecursiveCount == 0)
        {
            sem->Mutex_Holder = NULL;
        }
    }
#endif /* EK_CORO_MUTEX_ENABLE == 1 */

    // 有协程在等待信号量，直接唤醒等待时间最长的协程（FIFO）
    if (sem->Sem_WaitList.List_Count > 0)
    {
        // 只有需要增加计数时才调用底层函数
        if (sem_count_plus_flag)
        {
            op_res = r_sem_give(sem);
            if (op_res != EK_OK)
            {
                EK_EXIT_CRITICAL();
                return op_res;
            }
        }

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
        if (sem_count_plus_flag)
        {
            op_res = r_sem_give(sem);
        }
        else
        {
            op_res = EK_OK; // 递归释放但不增加计数的情况
        }

        EK_EXIT_CRITICAL();
        return op_res;
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

/**
 * @brief 释放信号量（中断服务程序版本）
 * @details 此函数专用于中断服务程序中释放信号量，支持快速的非阻塞操作。
 *          与普通版本的区别：
 *          1. 只能在中断上下文中调用，非中断环境会返回失败
 *          2. 不会阻塞调用者，信号量操作立即完成
 *          3. 通过higher_prio_wake参数通知是否需要进行任务切换
 *          4. 不处理互斥锁的递归逻辑和优先级继承（中断中不适合处理复杂逻辑）
 *
 *          释放流程：
 *          1. 验证中断上下文和参数有效性
 *          2. 进入临界区保护共享数据
 *          3. 如果有任务等待获取信号量，直接唤醒最高优先级任务
 *          4. 否则增加信号量计数
 *          5. 更新higher_prio_wake标志指示是否需要调度
 *
 *          中断安全性：此函数使用临界区保护，确保中断环境下的安全访问
 *          非阻塞特性：中断中不能阻塞，所有操作都是原子性的
 *          调度提示：通过higher_prio_wake提示内核是否需要立即进行任务切换
 *
 * @param sem 信号量句柄，必须为有效的信号量指针
 * @param higher_prio_wake 输出参数，用于指示是否唤醒了更高优先级的任务：
 *                        - true: 唤醒了更高优先级任务，建议在中断退出后进行任务切换
 *                        - false: 未唤醒更高优先级任务，无需特殊处理
 *
 * @return bool 操作结果：
 *         - true: 信号量释放成功
 *         - false: 释放失败（非中断上下文、sem为空、互斥量持有者不匹配等）
 *
 * @note 此函数只能在中断服务程序中调用
 * @note 调用者需要检查higher_prio_wake参数，如果为true则应该触发任务切换
 * @note 此函数不会阻塞调用者，适合在中断上下文中使用
 * @note 对于互斥量，需要检查持有者匹配（但不在中断中处理优先级继承）
 *
 * @warning 不要在非中断上下文中调用此函数
 * @warning higher_prio_wake参数必须指向有效的bool变量
 * @warning 互斥量的递归逻辑在中断中不适用
 *
 * @see EK_rSemGive() 普通版本的信号量释放函数
 * @see EK_rSemTake() 信号量获取函数
 */
bool EK_bSemGive_FromISR(EK_CoroSemHanlder_t sem, bool *higher_prio_wake)
{
    // 参数有效性检查
    if (EK_IS_IN_INTERRUPT() == false) return false;
    if (sem == NULL || higher_prio_wake == NULL) return false;

    EK_ENTER_CRITICAL();

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 获取当前的TCB
    EK_Result_t op_res;
    bool need_yield = false;
    bool sem_count_plus_flag = true; // 是否需要增加信号量计数

    // 处理互斥锁相关逻辑（简化版，不处理优先级继承）
#if (EK_CORO_MUTEX_ENABLE == 1)
    if (sem->Sem_isMutex == true)
    {
        // 检查持有者：只有持有者才可以给出互斥量
        if (sem->Mutex_Holder != current_tcb)
        {
            EK_EXIT_CRITICAL();
            return false;
        }

        // 中断中不处理递归互斥锁的复杂逻辑，直接处理基本逻辑
        if (sem->Mutex_isRecursive == true)
        {
            // 递归释放：减少递归计数器
            sem->Mutex_RecursiveCount--;

            // 如果递归计数器仍然大于0，说明还在持有锁，不增加信号量计数
            if (sem->Mutex_RecursiveCount > 0)
            {
                sem_count_plus_flag = false;
            }
        }

        // 中断中不处理优先级恢复逻辑，只清除持有者
        if (sem->Mutex_isRecursive == false || sem->Mutex_RecursiveCount == 0)
        {
            sem->Mutex_Holder = NULL;
        }
    }
#endif /* EK_CORO_MUTEX_ENABLE == 1 */

    // 有协程在等待信号量，直接唤醒等待时间最长的协程（FIFO）
    if (sem->Sem_WaitList.List_Count > 0)
    {
        // 只有需要增加计数时才调用底层函数
        if (sem_count_plus_flag)
        {
            op_res = r_sem_give(sem);
            if (op_res != EK_OK)
            {
                EK_EXIT_CRITICAL();
                return false;
            }
        }

        // 获取等待链表头部的协程（等待时间最长）
        EK_CoroTCB_t *wait_tcb = p_sem_take_waiter(sem);

        // 设置唤醒原因并加入就绪链表
        v_sem_wake(wait_tcb, EK_CORO_EVENT_OK);

        // 如果唤醒的任务的优先级比当前任务的优先级高 则申请一次调度
        if (wait_tcb->TCB_Priority < current_tcb->TCB_Priority) need_yield = true;
    }
    // 没有协程等待，直接增加计数
    else
    {
        if (sem_count_plus_flag)
        {
            op_res = r_sem_give(sem);
            if (op_res != EK_OK)
            {
                EK_EXIT_CRITICAL();
                return false;
            }
        }
        // 递归释放但不增加计数的情况，直接成功
    }

    *higher_prio_wake |= need_yield;

    EK_EXIT_CRITICAL();

    return true;
}

#endif /* EK_CORO_SEMAPHORE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */