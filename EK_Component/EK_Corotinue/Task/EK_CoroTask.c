/**
 * @file EK_CoroTask.c
 * @brief 协程任务管理API实现
 * @details 实现了所有面向用户的任务操作函数。
 * @author N1ntyNine99
 * @date 2025-09-25
 * @version 1.8
 */

#include "EK_CoroTask.h"
#include "../../EK_Common.h"

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1)
#include "../Message/EK_CoroMessage.h"
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */

#if (EK_CORO_ENABLE == 1)

/**
 * @brief 返回线程模式，使用PSP堆栈
 * @details EXC_RETURN with no FPU context.
 */
#define INITIAL_EXC_RETURN (0xFFFFFFFDUL)

/**
 * @brief 任务意外退出处理函数。
 * @details
 *  所有协程任务的函数体都应该是一个永不返回的循环。
 *  如果一个任务函数意外返回（例如，没有使用 `while(1)`），
 *  CPU的PC指针将跳转到这里。这是一个错误状态，表明任务设计有误。
 *  此函数会进入一个死循环，以防止未知的程序行为。
 */
static void v_coro_exit(void)
{
    if (EK_pKernelGetCurrentTCB() == NULL)
    {
        while (1)
        {
            // 当前任务指向空
            // 建议排查内存池和各个任务栈的完整性
        }
    }

    while (1)
    {
        // 在协程中 每一个任务的内部都应该是死循环！
        // 当卡在这个死循环里请检查相关配置
    }
}

/**
 * @brief 初始化任务的初始上下文。
 * @details
 *  在任务的堆栈顶部创建一个伪造的上下文帧。这个帧的布局必须精确匹配
 *  `EK_vKernelPendSV_Handler` 中上下文保存/恢复的顺序，以及Cortex-M硬件异常处理机制。
 *  当任务第一次被调度时，PendSV处理器会像恢复一个已存在的任务一样“恢复”这个伪造的上下文，
 *  从而使PC指针跳转到任务的入口函数，开始执行任务。
 *
 *  堆栈布局 (从高地址 -> 低地址):
 *  - xPSR: 初始程序状态寄存器 (Thumb模式, T-bit = 1)
 *  - PC  : 任务入口函数地址
 *  - LR  : 任务返回地址 (v_coro_exit)
 *  - R12, R3, R2, R1
 *  - R0  : 任务入口函数的参数
 *  --- 以上由硬件在异常进入时自动压栈 ---
 *  - EXC_RETURN: 特殊的连接寄存器值，指导异常如何返回
 *  - R11, R10, R9, R8, R7, R6, R5, R4: C函数调用需要保存的寄存器
 *  --- 以上由软件(PendSV_Handler)手动压栈 ---
 * @param tcb 指向要初始化上下文的任务控制块 (TCB)。
 */
ALWAYS_STATIC_INLINE void v_task_init_context(EK_CoroTCB_t *tcb)
{
    EK_CoroStack_t *stk;
#if (EK_HIGH_WATER_MARK_ENABLE == 1)
    stk = (EK_CoroStack_t *)tcb->TCB_StackEnd; // 直接使用栈顶地址
#else
    stk = (EK_CoroStack_t *)((uint8_t *)tcb->TCB_StackStart + tcb->TCB_StackSize); // 计算栈顶地址
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */
    // 确保堆栈8字节对齐
    stk = (EK_CoroStack_t *)(((uintptr_t)stk) & ~0x07UL);

    // 伪造硬件自动保存的上下文
    *(--stk) = 0x01000000UL; // xPSR (T-bit = 1)
    *(--stk) = ((uintptr_t)tcb->TCB_Entry) | 0x01UL; // PC (任务入口点, Thumb-bit = 1)
    *(--stk) = (uintptr_t)v_coro_exit; // LR (任务返回地址)
    *(--stk) = 0; // R12
    *(--stk) = 0; // R3
    *(--stk) = 0; // R2
    *(--stk) = 0; // R1
    *(--stk) = (uintptr_t)tcb->TCB_Arg; // R0 (任务参数)

    // 伪造软件手动保存的上下文
    *(--stk) = INITIAL_EXC_RETURN; // EXC_RETURN:无FPU 初始化不需要管FPU
    *(--stk) = 0; // R11
    *(--stk) = 0; // R10
    *(--stk) = 0; // R9
    *(--stk) = 0; // R8
    *(--stk) = 0; // R7
    *(--stk) = 0; // R6
    *(--stk) = 0; // R5
    *(--stk) = 0; // R4

    // 更新TCB中的堆栈指针
    tcb->TCB_StackPointer = stk;
}

/**
 * @brief 动态创建一个协程。
 * @details
 *  此函数使用 `EK_CORO_MALLOC` 动态分配任务控制块 (TCB) 和任务堆栈。
 *  创建成功后，任务被置于就绪状态，并根据其优先级插入到相应的就绪链表中，等待调度器执行。
 * @param task_func 任务的入口函数指针。
 * @param task_arg 传递给任务入口函数的参数。
 * @param priority 任务的优先级 (数值越小，优先级越高)。
 * @param stack_size 任务堆栈的大小 (以字节为单位)。
 * @return EK_CoroHandler_t 成功时返回协程的句柄，内存分配失败时返回 NULL。
 */
EK_CoroHandler_t EK_pCoroCreate(EK_CoroFunction_t task_func, void *task_arg, uint16_t priority, EK_Size_t stack_size)
{
    // 检查入口函数指针是否有效
    if (task_func == NULL) return NULL;

    // 为任务控制块 (TCB) 分配内存
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)EK_CORO_MALLOC(sizeof(EK_CoroTCB_t));
    if (tcb == NULL) return NULL;

    // 为任务堆栈分配内存
    void *stack = EK_CORO_MALLOC(stack_size);
    if (stack == NULL)
    {
        // 如果堆栈分配失败，则释放已分配的TCB内存，防止内存泄漏
        EK_CORO_FREE(tcb);
        return NULL;
    }

    // 填充堆栈以便检测堆栈使用情况
    EK_vMemSet(stack, EK_STACK_FILL_PATTERN, stack_size);

    // 判断priority是否超过预设
    if (priority >= EK_CORO_PRIORITY_MOUNT)
    {
        priority = EK_CORO_PRIORITY_MOUNT - 1;
    }

    // 初始化TCB中的各个成员
    tcb->TCB_Entry = task_func;
    tcb->TCB_Arg = task_arg;
    tcb->TCB_StackStart = stack;
    tcb->TCB_Priority = priority;
    tcb->TCB_StackSize = stack_size;

    tcb->TCB_WakeUpTime = 0;
    tcb->TCB_LastWakeUpTime = 0;
    tcb->TCB_isDynamic = true;
    tcb->TCB_State = EK_CORO_READY;
    tcb->TCB_StateNode.CoroNode_Owner = tcb;
    tcb->TCB_StateNode.CoroNode_Next = NULL;
    tcb->TCB_StateNode.CoroNode_Prev = NULL;
    tcb->TCB_StateNode.CoroNode_List = NULL;

#if (EK_CORO_TASK_NOTIFY_ENABLE == 1)
    tcb->TCB_NotifyState = 0;
    EK_vMemSet(tcb->TCB_NotifyValue, 0, EK_CORO_TASK_NOTIFY_GROUP);
#endif /* EK_CORO_TASK_NOTIFY_ENABLE == 1 */

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1)
    tcb->TCB_EventNode.CoroNode_Owner = tcb;
    tcb->TCB_EventNode.CoroNode_Next = NULL;
    tcb->TCB_EventNode.CoroNode_Prev = NULL;
    tcb->TCB_EventNode.CoroNode_List = NULL;
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 */

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1)
    tcb->TCB_EventResult = EK_CORO_EVENT_NONE;
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1 */

#if (EK_HIGH_WATER_MARK_ENABLE == 1)
    tcb->TCB_StackEnd = (void *)((uint8_t *)stack + stack_size);
    tcb->TCB_StackHighWaterMark = 0;
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */

    // 初始化任务的上下文（模拟CPU寄存器入栈）
    v_task_init_context(tcb);

    // 将新创建的任务插入到就绪链表中
    EK_rKernelInsert_Tail(EK_pKernelGetReadyList(priority), &tcb->TCB_StateNode);

    // 返回任务句柄
    return (EK_CoroHandler_t)tcb;
}

/**
 * @brief 使用静态内存创建一个协程。
 * @details
 *  此函数使用用户提供的 TCB 结构体和堆栈缓冲区来创建一个任务，避免了动态内存分配。
 *  创建成功后，任务被置于就绪状态，并根据其优先级插入到相应的就绪链表中，等待调度器执行。
 * @param tcb 用户提供的静态 TCB 结构体指针。
 * @param task_func 任务的入口函数指针。
 * @param task_arg 传递给任务入口函数的参数。
 * @param priority 任务的优先级 (数值越小，优先级越高)。
 * @param stack 用户提供的静态堆栈内存指针。
 * @param stack_size 任务堆栈的大小 (以字节为单位)。
 * @return EK_CoroStaticHandler_t 成功时返回静态协程的句柄，参数无效时返回 NULL。
 */
EK_CoroStaticHandler_t EK_pCoroCreateStatic(EK_CoroTCB_t *tcb,
                                            EK_CoroFunction_t task_func,
                                            void *task_arg,
                                            uint16_t priority,
                                            void *stack,
                                            EK_Size_t stack_size)
{
    // 确保所有必要的指针都已提供
    if (tcb == NULL || task_func == NULL || stack == NULL) return NULL;

    // 填充堆栈以便检测堆栈使用情况
    EK_vMemSet(stack, EK_STACK_FILL_PATTERN, stack_size);

    // 判断priority是否超过预设
    if (priority >= EK_CORO_PRIORITY_MOUNT)
    {
        priority = EK_CORO_PRIORITY_MOUNT - 1;
    }

    // 初始化用户传入的TCB结构体
    tcb->TCB_Entry = task_func;
    tcb->TCB_Arg = task_arg;
    tcb->TCB_Priority = priority;
    tcb->TCB_StackStart = stack;
    tcb->TCB_StackSize = stack_size;

    tcb->TCB_WakeUpTime = 0;
    tcb->TCB_LastWakeUpTime = 0;
    tcb->TCB_isDynamic = false;
    tcb->TCB_State = EK_CORO_READY;
    tcb->TCB_StateNode.CoroNode_Owner = tcb;
    tcb->TCB_StateNode.CoroNode_Next = NULL;
    tcb->TCB_StateNode.CoroNode_Prev = NULL;
    tcb->TCB_StateNode.CoroNode_List = NULL;

#if (EK_CORO_TASK_NOTIFY_ENABLE == 1)
    tcb->TCB_NotifyState = 0;
    EK_vMemSet(tcb->TCB_NotifyValue, 0, EK_CORO_TASK_NOTIFY_GROUP);
#endif /* EK_CORO_TASK_NOTIFY_ENABLE == 1 */

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1)
    tcb->TCB_EventNode.CoroNode_Owner = tcb;
    tcb->TCB_EventNode.CoroNode_Next = NULL;
    tcb->TCB_EventNode.CoroNode_Prev = NULL;
    tcb->TCB_EventNode.CoroNode_List = NULL;
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 */

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1)
    tcb->TCB_EventResult = EK_CORO_EVENT_NONE;
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1 */

#if (EK_HIGH_WATER_MARK_ENABLE == 1)
    tcb->TCB_StackEnd = (void *)((uint8_t *)stack + stack_size);
    tcb->TCB_StackHighWaterMark = 0;
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */

    // 初始化任务的上下文
    v_task_init_context(tcb);

    // 将任务插入到就绪链表
    EK_rKernelInsert_Tail(EK_pKernelGetReadyList(priority), &tcb->TCB_StateNode);

    // 返回任务句柄
    return (EK_CoroHandler_t)tcb;
}

/**
 * @brief 挂起指定协程。
 * @details
 *  将目标任务从当前所属链表（通常为就绪链表）移至挂起链表 `EK_CoroKernelSuspendList`，
 *  使其暂时不会被调度。若入参为 `NULL`，则默认挂起当前正在运行的任务，
 *  并在成功挂起后触发一次调度切换。
 * @param task_handle 要挂起的任务句柄，传入 `NULL` 表示挂起当前任务。
 * @return EK_Result_t 操作结果：
 *         - EK_OK：挂起成功；
 *         - EK_INVALID_PARAM：尝试挂起空闲任务；
 *         - EK_ERROR：无法获取当前任务或在中断中执行自挂起；
 *         - 其他错误码：链表操作失败。
 */
EK_Result_t EK_rCoroSuspend(EK_CoroHandler_t task_handle)
{
    bool self_suspend = false;
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    EK_ENTER_CRITICAL();

    // 空闲任务不允许被挂起
    if (target_tcb == EK_pKernelGetIdleHandler())
    {
        EK_EXIT_CRITICAL();
        return EK_INVALID_PARAM;
    }

    // 处理空句柄，默认挂起当前任务
    if (target_tcb == NULL)
    {
        target_tcb = EK_pKernelGetCurrentTCB();
        if (target_tcb == NULL)
        {
            EK_EXIT_CRITICAL();
            return EK_ERROR;
        }
        self_suspend = true;
    }

    // 检查并清理事件节点：如果任务正在等待信号量/消息队列，需要从等待列表中移除
#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1)
    if (target_tcb->TCB_EventNode.CoroNode_List != NULL)
    {
        // 从事件等待列表中移除任务的事件节点
        EK_rKernelRemove(target_tcb->TCB_EventNode.CoroNode_List, &target_tcb->TCB_EventNode);
    }
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 */

    // 清理事件结果状态
#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1)
    target_tcb->TCB_EventResult = EK_CORO_EVENT_NONE;
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1 */

    // 更新任务状态并移入挂起链表
    target_tcb->TCB_State = EK_CORO_SUSPENDED;
    EK_Result_t op_res = EK_rKernelMove_Tail(EK_pKernelGetSuspendList(), &target_tcb->TCB_StateNode);

    if (self_suspend && op_res == EK_OK)
    {
        EK_EXIT_CRITICAL();

        // 中断上下文中禁止触发调度切换
        if (EK_IS_IN_INTERRUPT() == true)
        {
            return EK_ERROR;
        }

        EK_vKernelYield();
        return EK_OK;
    }

    EK_EXIT_CRITICAL();
    return op_res;
}

/**
 * @brief 恢复被挂起的协程。
 * @details
 *  将挂起链表中的目标任务重新插入对应优先级的就绪链表尾部，
 *  使其再次具备被调度的机会。若任务未挂起，链表迁移会返回失败码。
 * @param task_handle 要恢复的任务句柄。
 * @return EK_Result_t 操作结果：
 *         - EK_OK：恢复成功；
 *         - EK_INVALID_PARAM：尝试恢复空闲任务；
 *         - EK_NULL_POINTER：传入空句柄；
 *         - 其他错误码：链表迁移失败。
 */
EK_Result_t EK_rCoroResume(EK_CoroHandler_t task_handle)
{
    // 参数转换：获取目标任务的TCB指针
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    // 进入临界区：保护任务状态和链表操作
    EK_ENTER_CRITICAL();

    // 不能恢复空闲任务
    if (target_tcb == EK_pKernelGetIdleHandler())
    {
        EK_EXIT_CRITICAL();
        return EK_INVALID_PARAM;
    }

    // 检查空指针
    if (target_tcb == NULL)
    {
        EK_EXIT_CRITICAL();
        return EK_NULL_POINTER;
    }

    // 将任务状态设置为就绪
    target_tcb->TCB_State = EK_CORO_READY;

    // 将任务从挂起链表移到对应优先级的就绪链表尾部
    // 使用尾部插入，保证FIFO公平性
    EK_Result_t op_res =
        EK_rKernelMove_Tail(EK_pKernelGetReadyList(target_tcb->TCB_Priority), &target_tcb->TCB_StateNode);

    // 退出临界区：恢复中断
    EK_EXIT_CRITICAL();
    return op_res;
}

/**
 * @brief 删除指定协程。
 * @details
 *  - 动态任务：从调度链表移除并释放堆栈与 TCB；
 *  - 静态任务：无法释放资源，改为挂起任务作为等效操作；
 *  - 传入 `NULL`：表示删除当前任务，删除动作会在空闲任务中完成以保证安全。
 * @param task_handle 要删除的任务句柄，`NULL` 表示当前任务。
 * @return EK_Result_t 操作结果：
 *         - EK_OK：删除或挂起成功；
 *         - EK_INVALID_PARAM：尝试删除空闲任务；
 *         - EK_ERROR：无法获取当前任务或在中断中自删除；
 *         - 其他错误码：链表操作失败。
 */
EK_Result_t EK_rCoroDelete(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;
    bool self_delete = false;
    // 进入临界区：保护任务删除过程
    EK_ENTER_CRITICAL();

    // 不能删除空闲任务
    if (target_tcb == EK_pKernelGetIdleHandler())
    {
        EK_EXIT_CRITICAL();
        return EK_INVALID_PARAM;
    }

    // NULL表示删除当前任务
    if (target_tcb == NULL)
    {
        target_tcb = EK_pKernelGetCurrentTCB();
        if (target_tcb == NULL)
        {
            EK_EXIT_CRITICAL();
            return EK_ERROR;
        }
        self_delete = true;
    }

    // 静态任务 无法释放内存，改为挂起操作
    if (target_tcb->TCB_isDynamic == false)
    {
        EK_EXIT_CRITICAL();
        return EK_rCoroSuspend(target_tcb);
    }

    // 动态任务
    // 删除其所有相关的节点链表
    EK_Result_t op_res = EK_OK;
    EK_CoroList_t *state_list = target_tcb->TCB_StateNode.CoroNode_List; // 待删除任务的状态链表
    if (state_list != NULL) op_res = EK_rKernelRemove(state_list, &target_tcb->TCB_StateNode);

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1)

    EK_CoroList_t *event_list = target_tcb->TCB_EventNode.CoroNode_List; // 待删除任务的事件链表
    if (event_list != NULL) op_res = EK_rKernelRemove(event_list, &target_tcb->TCB_EventNode);

#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 */

    // 标记为待删除 让空闲任务来删除
    if (op_res == EK_OK) EK_vKernelSetDeleteTCB(target_tcb);

    if (self_delete == true)
    {
        // 退出临界区：恢复中断
        EK_EXIT_CRITICAL();

        // 中断上下文中禁止触发调度切换
        if (EK_IS_IN_INTERRUPT() == true)
        {
            return EK_ERROR;
        }
        EK_vKernelYield();
        return EK_OK;
    }

    // 退出临界区：恢复中断
    EK_EXIT_CRITICAL();
    return EK_OK;
}

/**
 * @brief 将当前协程延时指定的Tick数。
 * @details
 *  此函数会阻塞当前正在运行的任务。它将当前任务从就绪状态改变为阻塞状态，
 *  并根据延时时间 `xticks` 计算出未来的唤醒时间 `TCB_WakeUpTime`。
 *  然后将任务移入阻塞链表 `KernelBlockList1`，并立即触发一次任务调度。
 *  当系统Tick到达唤醒时间后，`EK_vTickHandler` 会自动唤醒该任务。
 * @param xticks 要延时的Tick数。如果设置为 `EK_MAX_DELAY`，任务将永久阻塞，直到被 `EK_rCoroWakeup` 显式唤醒。
 */
void EK_vCoroDelay(uint32_t xticks)
{
    // 判断是不是在中断中
    if (EK_IS_IN_INTERRUPT() == true) return;

    EK_ENTER_CRITICAL();

    // 获取当前的TCB
    EK_CoroTCB_t *current = EK_pKernelGetCurrentTCB();
    if (current == NULL)
    {
        EK_EXIT_CRITICAL();
        return;
    }

    // 禁止操作空闲任务
    if (current == EK_pKernelGetIdleHandler())
    {
        EK_EXIT_CRITICAL();
        return;
    }

    // 如果是永久阻塞
    if (xticks == EK_MAX_DELAY)
    {
        current->TCB_WakeUpTime = EK_MAX_DELAY;
    }
    else // 否则就是普通延时
    {
        // 计算唤醒时间
        uint32_t kernel_tick = EK_uKernelGetTick();
        uint32_t temp = xticks + kernel_tick;

        // 设置当前TCB的唤醒时间
        current->TCB_WakeUpTime = (temp == EK_MAX_DELAY ? EK_MAX_DELAY + 1 : temp);

        if (current->TCB_WakeUpTime < kernel_tick)
        {
            // 唤醒时间小于当前时基，说明是溢出后的链表
            EK_rKernelMove_WakeUpTime(EK_pKernelGetNextBlockList(), &current->TCB_StateNode);
        }
        else
        {
            // 唤醒时间大于等于当前时基，说明是当前周期链表
            EK_rKernelMove_WakeUpTime(EK_pKernelGetCurrentBlockList(), &current->TCB_StateNode);
        }
    }

    // 设置当前的TCB状态为阻塞
    current->TCB_State = EK_CORO_BLOCKED;

    EK_EXIT_CRITICAL();
    // 请求一次调度
    EK_vKernelYield(); // 此调用不会返回
}

/**
 * @brief 将当前协程延时直到指定的绝对时间点。
 * @details
 *  此函数实现了精确定时的周期性延时功能。与 EK_vCoroDelay 的相对延时不同，
 *  delayUntil 使用绝对时间来计算唤醒时间，确保任务执行的周期性不受任务执行时间的影响。
 *  函数会：
 *  1. 检查TCB中上次唤醒时间是否已初始化
 *  2. 计算下一次唤醒的绝对时间点
 *  3. 更新TCB中的上次唤醒时间记录
 *  4. 执行与 EK_vCoroDelay 类似的阻塞逻辑
 * @param xticks 期望的执行周期（以tick为单位）
 */
void EK_vCoroDelayUntil(uint32_t xticks)
{
    // 判断是不是在中断中
    if (EK_IS_IN_INTERRUPT() == true) return;

    // 参数检查
    if (xticks == 0)
    {
        return;
    }

    EK_ENTER_CRITICAL();

    // 获取当前的TCB
    EK_CoroTCB_t *current = EK_pKernelGetCurrentTCB();
    if (current == NULL)
    {
        EK_EXIT_CRITICAL();
        return;
    }

    // 禁止操作空闲任务
    if (current == EK_pKernelGetIdleHandler())
    {
        EK_EXIT_CRITICAL();
        return;
    }

    uint32_t current_tick = EK_uKernelGetTick();
    uint32_t next_wake_time;

    // 如果是第一次调用，初始化TCB_LastWakeUpTime
    if (current->TCB_LastWakeUpTime == 0)
    {
        current->TCB_LastWakeUpTime = current_tick;
    }

    // 计算下一次唤醒时间
    next_wake_time = current->TCB_LastWakeUpTime + xticks;

    // 更新TCB_LastWakeUpTime为下一次唤醒时间
    current->TCB_LastWakeUpTime = next_wake_time;

    // 计算延时时间
    if (next_wake_time > current_tick)
    {
        // 正常情况：未来时间点
        current->TCB_WakeUpTime = next_wake_time;

        // 根据唤醒时间选择阻塞链表
        if (current->TCB_WakeUpTime < current_tick)
        {
            // 唤醒时间小于当前时基，说明是溢出后的链表
            EK_rKernelMove_WakeUpTime(EK_pKernelGetNextBlockList(), &current->TCB_StateNode);
        }
        else
        {
            // 唤醒时间大于等于当前时基，说明是当前周期链表
            EK_rKernelMove_WakeUpTime(EK_pKernelGetCurrentBlockList(), &current->TCB_StateNode);
        }
    }
    else
    {
        // 如果错过了时间点，立即执行（不阻塞）
        // 将任务重新放回就绪链表
        current->TCB_State = EK_CORO_READY;
        EK_rKernelMove_Tail(EK_pKernelGetReadyList(current->TCB_Priority), &current->TCB_StateNode);
        EK_EXIT_CRITICAL();
        return;
    }

    // 设置当前TCB状态为阻塞
    current->TCB_State = EK_CORO_BLOCKED;

    EK_EXIT_CRITICAL();
    // 请求调度
    EK_vKernelYield(); // 此调用不会返回
}

/**
 * @brief 手动唤醒一个永久阻塞的协程。
 * @details
 *  此函数用于唤醒一个因调用 `EK_vCoroDelay(EK_MAX_DELAY)` 而永久阻塞的任务。
 *  它会将指定的任务从阻塞链表移动到就绪链表。如果被唤醒的任务的优先级高于当前正在运行的任务，
 *  函数会立即触发一次任务调度。
 * @param task_handle 要唤醒的任务句柄。
 * @return EK_Result_t 操作结果。如果任务不是永久阻塞状态，会返回 `EK_INVALID_PARAM`。
 */
EK_Result_t EK_rCoroWakeup(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    // 参数检查
    if (target_tcb == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    // 检查任务是否在阻塞链表中（可能是当前链表或溢出链表），并且是永久阻塞状态
    EK_CoroList_t *curr_block = EK_pKernelGetCurrentBlockList();
    EK_CoroList_t *next_block = EK_pKernelGetNextBlockList();
    if ((target_tcb->TCB_StateNode.CoroNode_List != curr_block &&
         target_tcb->TCB_StateNode.CoroNode_List != next_block) ||
        target_tcb->TCB_WakeUpTime != EK_MAX_DELAY)
    {
        EK_EXIT_CRITICAL();
        return EK_INVALID_PARAM; // 任务不在阻塞列表或不是永久阻塞
    }

    // 将任务移动到就绪链表
    target_tcb->TCB_State = EK_CORO_READY;
    EK_Result_t result =
        EK_rKernelMove_Tail(EK_pKernelGetReadyList(target_tcb->TCB_Priority), &target_tcb->TCB_StateNode);

    if (result == EK_OK)
    {
        // 如果唤醒的任务比当前任务优先级更高，则请求调度
        EK_CoroTCB_t *current = EK_pKernelGetCurrentTCB();
        if (current != NULL && target_tcb->TCB_Priority < current->TCB_Priority)
        {
            EK_EXIT_CRITICAL();
            EK_vKernelYield();
            return EK_OK;
        }
    }

    EK_EXIT_CRITICAL();
    return result;
}

/**
 * @brief 当前协程主动让出CPU执行权。
 * @details
 *  此函数将当前正在运行的任务放回到其对应优先级就绪链表的末尾，并立即触发一次任务调度。
 *  这使得与当前任务同优先级的其他就绪任务有机会运行，从而实现一种简单的“时间片轮转”效果。
 *  如果就绪链表中没有其他更高或同优先级的任务，调度后可能仍然会继续执行当前任务。
 */
void EK_vCoroYield(void)
{
    // 判断是不是在中断中
    if (EK_IS_IN_INTERRUPT() == true) return;

    EK_ENTER_CRITICAL();
    // 获取当前的TCB
    EK_CoroTCB_t *current = EK_pKernelGetCurrentTCB();
    if (current == NULL)
    {
        EK_EXIT_CRITICAL();
        return;
    }

    // 设置当前的TCB状态为就绪
    current->TCB_State = EK_CORO_READY;
    // 插入到就绪链表
    EK_rKernelMove_Tail(EK_pKernelGetReadyList(current->TCB_Priority), &current->TCB_StateNode);

    EK_EXIT_CRITICAL();
    // 请求调度
    EK_vKernelYield();
}

/**
 * @brief 修改指定任务的优先级。
 * @details
 *  动态地改变一个任务的优先级。如果任务当前在就绪链表中，它会被移动到新优先级对应的就绪链表。
 *  优先级的改变将在下一次任务调度时生效。
 * @param task_handle 要修改的任务句柄。若为 NULL，则修改当前任务。
 * @param priority 新的优先级。
 * @param result (可选) 指向 `EK_Result_t` 的指针，用于返回操作结果。
 */
void EK_vCoroSetPriority(EK_CoroHandler_t task_handle, uint16_t priority, EK_Result_t *result)
{
    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    EK_ENTER_CRITICAL();
    // 禁止操作空闲任务
    if (target_tcb == EK_pKernelGetIdleHandler())
    {
        if (result) *result = EK_INVALID_PARAM;
        EK_EXIT_CRITICAL();
        return;
    }

    // 判断priority是否超过预设
    if (priority >= EK_CORO_PRIORITY_MOUNT)
    {
        priority = EK_CORO_PRIORITY_MOUNT - 1;
    }

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = EK_pKernelGetCurrentTCB();
        if (target_tcb == NULL)
        {
            if (result) *result = EK_ERROR;
            EK_EXIT_CRITICAL();
            return;
        }
    }

    target_tcb->TCB_Priority = priority;
    if (result) *result = EK_OK;
    EK_EXIT_CRITICAL();
}

/**
 * @brief 获取指定协程的堆栈总大小。
 * @details 返回在创建任务时指定的堆栈大小。
 * @param task_handle 要查询的任务句柄。若为 NULL，则查询当前任务。
 * @return EK_Size_t 任务的堆栈总大小 (以字节为单位)。
 */
EK_Size_t EK_uCoroGetStack(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)task_handle;
    if (tcb == NULL)
    {
        tcb = EK_pKernelGetCurrentTCB();
    }

    if (tcb == NULL)
    {
        return 0;
    }

    return tcb->TCB_StackSize;
}

#if (EK_CORO_TASK_NOTIFY_ENABLE == 1)

/**
 * @brief 唤醒等待通知的任务
 *
 * @param task_handle 要唤醒的任务句柄
 * @return EK_Result_t 操作结果
 */
ALWAYS_STATIC_INLINE EK_Result_t r_task_notify_wake(EK_CoroHandler_t task_handle)
{
    // 如果本身任务就是就绪的 直接返回OK即可
    if (task_handle->TCB_State == EK_CORO_READY) return EK_OK;

    task_handle->TCB_State = EK_CORO_READY;
    task_handle->TCB_EventResult = EK_CORO_EVENT_OK;

    return EK_rKernelMove_Head(EK_pKernelGetReadyList(task_handle->TCB_Priority), &task_handle->TCB_StateNode);
}

/**
 * @brief 使用任务通知某一个任务
 * 
 * @param task_handle 想要通知的任务
 * @param bit 通知的某个位
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rCoroSendNotify(EK_CoroHandler_t task_handle, uint8_t bit)
{
    // 超过位图长度
    if (bit >= EK_CORO_TASK_NOTIFY_GROUP) return EK_INVALID_PARAM;

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 获取当前的

    EK_ENTER_CRITICAL();
    // 参数合法性检查
    if (task_handle == EK_pKernelGetIdleHandler() || task_handle == current_tcb || task_handle == NULL)
    {
        EK_EXIT_CRITICAL();
        return EK_INVALID_PARAM;
    }

    // 置位
    if (EK_bTestBit(&task_handle->TCB_NotifyState, bit) == false)
    {
        EK_vSetBit(&task_handle->TCB_NotifyState, bit);
    }
    task_handle->TCB_NotifyValue[bit] = EK_CLAMP(++task_handle->TCB_NotifyValue[bit], 0, UINT8_MAX);

    // 唤醒任务
    EK_Result_t res = r_task_notify_wake(task_handle);

    EK_EXIT_CRITICAL();

    // 如果唤醒的任务的优先级高于当前的任务 会切换一次上下文
    if (task_handle->TCB_Priority < current_tcb->TCB_Priority) EK_vCoroYield();

    return res;
}

/**
 * @brief 等待指定位置的通知
 *
 * @param bit 等待的位位置
 * @param timeout 超时时间（ms），0表示不阻塞
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rCoroWaitNotify(uint8_t bit, uint32_t timeout)
{
    EK_CoroTCB_t *target_tcb = EK_pKernelGetCurrentTCB(); // 获取当前的任务句柄
    if (target_tcb == NULL) return EK_ERROR;

    while (1)
    {
        EK_ENTER_CRITICAL();

        // 查看响应位是否有标记
        if (EK_bTestBit(&target_tcb->TCB_NotifyState, bit) == true)
        {
            target_tcb->TCB_NotifyValue[bit] = EK_CLAMP(--target_tcb->TCB_NotifyValue[bit], 0, UINT8_MAX);
            if (target_tcb->TCB_NotifyValue[bit] == 0)
            {
                EK_vClearBit(&target_tcb->TCB_NotifyState, bit);
            }
            EK_EXIT_CRITICAL();
            return EK_OK;
        }
        else
        {
            if (timeout == 0) return EK_EMPTY; // 无需阻塞直接返回
            EK_EXIT_CRITICAL();
            EK_vCoroDelay(timeout);

            // --从这里唤醒--
            // 超时直接退出
            if (target_tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT)
            {
                return EK_TIMEOUT;
            }
            // 不是超时就重试
        }
    }
}

#endif /* EK_CORO_TASK_NOTIFY_ENABLE == 1 */

#if (EK_HIGH_WATER_MARK_ENABLE == 1)
/**
 * @brief 获取指定协程的堆栈历史最大使用量 (高水位)。
 * @details
 *  获取在任务切换时预先计算并保存的栈高水位标记。
 *  由于每次任务切换都会自动计算高水位标记，此函数只需返回预计算的值，
 *  性能极高，适用于实时监控和调试。
 *
 *  高水位标记指的是栈顶到栈中最后一个未被修改位置的距离，数值越小表示
 *  栈使用得越少，剩余空间越大。返回值为距离栈顶的字节数。
 *
 * @param task_handle 要查询的任务句柄。若为 NULL，则查询当前任务。
 * @return EK_Size_t 任务的堆栈高水位标记 (距离栈顶的字节数)，0表示栈已满。
 */
EK_Size_t EK_uCoroGetHighWaterMark(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)task_handle;
    if (tcb == NULL)
    {
        tcb = EK_pKernelGetCurrentTCB();
    }

    if (tcb == NULL)
    {
        return 0;
    }

    // 直接返回预计算的高水位标记，性能极高
    return tcb->TCB_StackHighWaterMark;
}

/**
 * @brief 获取指定协程的堆栈当前使用量（重新计算版本）
 * @details
 *  重新计算当前栈使用量，用于调试和验证高水位标记的正确性。
 *  此函数性能较低，仅用于调试目的。
 *
 * @param task_handle 要查询的任务句柄。若为 NULL，则查询当前任务。
 * @return EK_Size_t 任务的堆栈当前使用量 (以字节为单位)。
 */
EK_Size_t EK_uCoroGetStackUsage_Debug(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)task_handle;
    if (tcb == NULL)
    {
        tcb = EK_pKernelGetCurrentTCB();
    }

    if (tcb == NULL || tcb->TCB_StackEnd == NULL)
    {
        return 0;
    }

    uint8_t *stack_end = (uint8_t *)tcb->TCB_StackEnd;
    uint8_t *stack_base = (uint8_t *)tcb->TCB_StackStart;
    EK_Size_t current_usage = 0;

    // 重新计算当前使用量
    while (current_usage < tcb->TCB_StackSize && stack_end - current_usage > stack_base &&
           *(stack_end - current_usage) != EK_STACK_FILL_PATTERN)
    {
        current_usage++;
    }

    return current_usage;
}
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */

#endif /* EK_CORO_ENABLE == 1 */