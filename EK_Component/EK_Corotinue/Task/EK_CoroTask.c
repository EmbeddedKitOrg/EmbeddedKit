/**
 * @file EK_CoroTask.c
 * @brief 协程任务管理API实现
 * @details 实现了所有面向用户的任务操作函数。
 * @author N1ntyNine99
 * @date 2025-09-25
 * @version 1.8
 */

#include "EK_CoroTask.h"

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)
#include "../Message/EK_CoroMessage.h"
#endif /* EK_CORO_USE_MESSAGE_QUEUE == 1 */

#if (EK_CORO_ENABLE == 1)

/**
 * @brief 协程任务栈填充值
 * @details 用于检测栈使用高水位
 */
#define EK_STACK_FILL_PATTERN (0xCD)

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
    if (EK_CoroKernelCurrentTCB == NULL)
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
static void v_task_init_context(EK_CoroTCB_t *tcb)
{
    EK_CoroStack_t *stk;
    stk = (EK_CoroStack_t *)((uint8_t *)tcb->TCB_StackBase + tcb->TCB_StackSize);
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
    *(--stk) = INITIAL_EXC_RETURN; // EXC_RETURN:无FPU版本
    *(--stk) = 0; // R11
    *(--stk) = 0; // R10
    *(--stk) = 0; // R9
    *(--stk) = 0; // R8
    *(--stk) = 0; // R7
    *(--stk) = 0; // R6
    *(--stk) = 0; // R5
    *(--stk) = 0; // R4

    // 更新TCB中的堆栈指针
    tcb->TCB_SP = stk;
}
/**
 * @brief 动态创建一个协程。
 * @details
 *  此函数使用 `EK_MALLOC` 动态分配任务控制块 (TCB) 和任务堆栈。
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
    EK_CoroTCB_t *dynamic_tcb = (EK_CoroTCB_t *)EK_MALLOC(sizeof(EK_CoroTCB_t));
    if (dynamic_tcb == NULL) return NULL;

    // 为任务堆栈分配内存
    void *stack = EK_MALLOC(stack_size);
    if (stack == NULL)
    {
        // 如果堆栈分配失败，则释放已分配的TCB内存，防止内存泄漏
        EK_FREE(dynamic_tcb);
        return NULL;
    }

    // 判断priority是否超过预设
    if (priority >= EK_CORO_PRIORITY_MOUNT)
    {
        priority = EK_CORO_PRIORITY_MOUNT - 1;
    }

    // 初始化TCB中的各个成员
    dynamic_tcb->TCB_Entry = task_func;
    dynamic_tcb->TCB_Arg = task_arg;
    dynamic_tcb->TCB_StackBase = stack;
    dynamic_tcb->TCB_Priority = priority;
    dynamic_tcb->TCB_StackSize = stack_size;
    dynamic_tcb->TCB_WakeUpTime = 0;
    dynamic_tcb->TCB_isDynamic = true; // 标记为动态创建
    dynamic_tcb->TCB_State = EK_CORO_READY;
    dynamic_tcb->TCB_StateNode.CoroNode_Owner = dynamic_tcb;
    dynamic_tcb->TCB_StateNode.CoroNode_Next = NULL;
    dynamic_tcb->TCB_StateNode.CoroNode_Prev = NULL;
    dynamic_tcb->TCB_StateNode.CoroNode_List = NULL;
    dynamic_tcb->TCB_EventNode.CoroNode_Owner = dynamic_tcb;
    dynamic_tcb->TCB_EventNode.CoroNode_Next = NULL;
    dynamic_tcb->TCB_EventNode.CoroNode_Prev = NULL;
    dynamic_tcb->TCB_EventNode.CoroNode_List = NULL;

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)
    dynamic_tcb->TCB_EventResult = EK_CORO_EVENT_NONE;
    dynamic_tcb->TCB_MsgData = NULL;
#endif /* EK_CORO_USE_MESSAGE_QUEUE == 1 */

    // 初始化任务的上下文（模拟CPU寄存器入栈）
    v_task_init_context(dynamic_tcb);

    EK_ENTER_CRITICAL();
    // 将新创建的任务插入到就绪链表中
    EK_rKernelInsert_Tail(&EK_CoroKernelReadyList[priority], &dynamic_tcb->TCB_StateNode);
    EK_EXIT_CRITICAL();

    // 返回任务句柄
    return (EK_CoroHandler_t)dynamic_tcb;
}

/**
 * @brief 使用静态内存创建一个协程。
 * @details
 *  此函数使用用户提供的 TCB 结构体和堆栈缓冲区来创建一个任务，避免了动态内存分配。
 *  创建成功后，任务被置于就绪状态，并根据其优先级插入到相应的就绪链表中，等待调度器执行。
 * @param static_tcb 用户提供的静态 TCB 结构体指针。
 * @param task_func 任务的入口函数指针。
 * @param task_arg 传递给任务入口函数的参数。
 * @param priority 任务的优先级 (数值越小，优先级越高)。
 * @param stack 用户提供的静态堆栈内存指针。
 * @param stack_size 任务堆栈的大小 (以字节为单位)。
 * @return EK_CoroStaticHandler_t 成功时返回静态协程的句柄，参数无效时返回 NULL。
 */
EK_CoroStaticHandler_t EK_pCoroCreateStatic(EK_CoroTCB_t *static_tcb,
                                            EK_CoroFunction_t task_func,
                                            void *task_arg,
                                            uint16_t priority,
                                            void *stack,
                                            EK_Size_t stack_size)
{
    // 确保所有必要的指针都已提供
    if (static_tcb == NULL || task_func == NULL || stack == NULL) return NULL;

    // 判断priority是否超过预设
    if (priority >= EK_CORO_PRIORITY_MOUNT)
    {
        priority = EK_CORO_PRIORITY_MOUNT - 1;
    }

    // 初始化用户传入的TCB结构体
    static_tcb->TCB_Entry = task_func;
    static_tcb->TCB_Arg = task_arg;
    static_tcb->TCB_Priority = priority;
    static_tcb->TCB_StackBase = stack;
    static_tcb->TCB_StackSize = stack_size;
    static_tcb->TCB_WakeUpTime = 0;
    static_tcb->TCB_isDynamic = false; // 标记为静态创建
    static_tcb->TCB_State = EK_CORO_READY;
    static_tcb->TCB_StateNode.CoroNode_Owner = static_tcb;
    static_tcb->TCB_StateNode.CoroNode_Next = NULL;
    static_tcb->TCB_StateNode.CoroNode_Prev = NULL;
    static_tcb->TCB_StateNode.CoroNode_List = NULL;
    static_tcb->TCB_EventNode.CoroNode_Owner = static_tcb;
    static_tcb->TCB_EventNode.CoroNode_Next = NULL;
    static_tcb->TCB_EventNode.CoroNode_Prev = NULL;
    static_tcb->TCB_EventNode.CoroNode_List = NULL;

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)
    static_tcb->TCB_EventResult = EK_CORO_EVENT_NONE;
    static_tcb->TCB_MsgData = NULL;
#endif /* EK_CORO_USE_MESSAGE_QUEUE == 1 */

    // 初始化任务的上下文
    v_task_init_context(static_tcb);

    EK_ENTER_CRITICAL();
    // 将任务插入到就绪链表
    EK_rKernelInsert_Tail(&EK_CoroKernelReadyList[priority], &static_tcb->TCB_StateNode);
    EK_EXIT_CRITICAL();

    // 返回任务句柄
    return (EK_CoroHandler_t)static_tcb;
}

/**
 * @brief 挂起一个指定的协程。
 * @details
 *  将指定的任务从其当前所在的链表（通常是就绪链表）中移除，并放入挂起链表 `EK_CoroKernelSuspendList`。
 *  被挂起的任务将不会被调度器选中，直到它被 `EK_vCoroResume` 恢复。
 *  如果任务句柄为 NULL，则会挂起当前正在运行的任务，并立即触发一次任务调度。
 * @param task_handle 要挂起的任务句柄。若为 NULL，则挂起当前任务。
 * @param result (可选) 指向 `EK_Result_t` 的指针，用于返回操作结果。
 */
void EK_vCoroSuspend(EK_CoroHandler_t task_handle, EK_Result_t *result)
{
    bool self_suspend = false;
    EK_Result_t op_res;
    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    EK_ENTER_CRITICAL();
    // 禁止操作空闲任务
    if (target_tcb == EK_CoroKernelIdleHandler)
    {
        if (result) *result = EK_INVALID_PARAM;
        EK_EXIT_CRITICAL();
        return;
    }

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = EK_CoroKernelCurrentTCB;
        if (target_tcb == NULL)
        {
            if (result) *result = EK_ERROR;
            EK_EXIT_CRITICAL();
            return;
        }
        self_suspend = true;
    }

    // 设置当前的状态
    target_tcb->TCB_State = EK_CORO_SUSPENDED;

    // 移动节点到挂起链表
    op_res = EK_rKernelMove_Tail(&EK_CoroKernelSuspendList, &target_tcb->TCB_StateNode);

    if (result) *result = op_res;

    // 自挂起的话就请求一次调度
    if (self_suspend && op_res == EK_OK)
    {
        EK_EXIT_CRITICAL();
        EK_vKernelYield();
    }
    else
    {
        EK_EXIT_CRITICAL();
    }
}

/**
 * @brief 恢复一个被挂起的协程。
 * @details
 *  将指定的任务从挂起链表 `EK_CoroKernelSuspendList` 中移除，并根据其优先级放回到就绪链表的末尾。
 *  恢复后的任务将能够再次被调度器调度。此函数对一个未被挂起的任务操作是安全的，但没有效果。
 * @param task_handle 要恢复的任务句柄。
 * @param result (可选) 指向 `EK_Result_t` 的指针，用于返回操作结果。
 */
void EK_vCoroResume(EK_CoroHandler_t task_handle, EK_Result_t *result)
{
    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    EK_ENTER_CRITICAL();
    // 禁止操作空闲任务
    if (target_tcb == EK_CoroKernelIdleHandler)
    {
        if (result) *result = EK_INVALID_PARAM;
        EK_EXIT_CRITICAL();
        return;
    }

    // 空指针处理
    if (target_tcb == NULL)
    {
        if (result) *result = EK_NULL_POINTER;
        EK_EXIT_CRITICAL();
        return;
    }

    // 更新当前TCB的状态
    target_tcb->TCB_State = EK_CORO_READY;

    // 移动到就绪链表
    EK_Result_t op_res =
        EK_rKernelMove_Tail(&EK_CoroKernelReadyList[target_tcb->TCB_Priority], &target_tcb->TCB_StateNode);
    if (result) *result = op_res;

    // 如果唤醒的任务比当前任务优先级更高，则请求调度
    if (target_tcb->TCB_Priority < EK_CoroKernelCurrentTCB->TCB_Priority)
    {
        EK_EXIT_CRITICAL();
        EK_vKernelYield();
        return EK_OK;
    }

    EK_EXIT_CRITICAL();
}

/**
 * @brief 删除一个指定的协程。
 * @details
 *  - 对于动态创建的任务：将其从内核的调度列表中移除，并释放其 TCB 和堆栈所占用的内存。
 *  - 对于静态创建的任务：由于其资源是静态分配的，不能被释放，因此该函数会将其挂起，效果等同于调用 `EK_vCoroSuspend`。
 *  - 如果任务句柄为 NULL，则会删除当前任务。这种情况下，删除操作会被延迟到空闲任务中执行，以确保安全。
 * @param task_handle 要删除的任务句柄。若为 NULL，则删除当前任务。
 * @param result (可选) 指向 `EK_Result_t` 的指针，用于返回操作结果。
 */
void EK_vCoroDelete(EK_CoroHandler_t task_handle, EK_Result_t *result)
{
    bool self_delete = false;
    EK_Result_t op_res;

    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    EK_ENTER_CRITICAL();
    // 禁止操作空闲任务
    if (target_tcb == EK_CoroKernelIdleHandler)
    {
        if (result) *result = EK_INVALID_PARAM;
        EK_EXIT_CRITICAL();
        return;
    }

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = EK_CoroKernelCurrentTCB;
        if (target_tcb == NULL)
        {
            if (result) *result = EK_ERROR;
            EK_EXIT_CRITICAL();
            return;
        }
        self_delete = true;
    }

    // 对于静态任务，我们不能释放其资源，只能将其挂起
    if (target_tcb->TCB_isDynamic == false)
    {
        EK_EXIT_CRITICAL();
        EK_vCoroSuspend(target_tcb, result);
        return;
    }

    // 对于动态任务
    if (self_delete)
    {
        // 标记等待删除，然后请求调度
        EK_CoroKernelDeleteTCB = target_tcb;
        op_res = EK_rKernelRemove(target_tcb->TCB_StateNode.CoroNode_List, &target_tcb->TCB_StateNode);
        if (result) *result = op_res;
        EK_EXIT_CRITICAL();
        EK_vKernelYield(); // 此调用不会返回
    }
    else
    {
        // 删除其他任务
        op_res = EK_rKernelRemove(target_tcb->TCB_StateNode.CoroNode_List, &target_tcb->TCB_StateNode);
        if (op_res == EK_OK)
        {
            EK_FREE(target_tcb->TCB_StackBase);
            EK_FREE(target_tcb);
        }
        if (result) *result = op_res;
        EK_EXIT_CRITICAL();
    }
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
    EK_ENTER_CRITICAL();
    // 获取当前的TCB
    EK_CoroTCB_t *current = EK_CoroKernelCurrentTCB;
    if (current == NULL)
    {
        EK_EXIT_CRITICAL();
        return;
    }

    // 禁止操作空闲任务
    if (current == EK_CoroKernelIdleHandler)
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
        uint32_t temp = xticks + EK_CoroKernelTick;

        // 设置当前TCB的唤醒时间
        current->TCB_WakeUpTime = (temp == EK_MAX_DELAY ? EK_MAX_DELAY + 1 : temp);

        if (current->TCB_WakeUpTime < EK_CoroKernelTick)
        {
            // 唤醒时间小于当前时基，说明是溢出后的链表
            EK_rKernelMove_WakeUpTime(EK_CoroKernelNextBlock, &current->TCB_StateNode);
        }
        else
        {
            // 唤醒时间大于等于当前时基，说明是当前周期链表
            EK_rKernelMove_WakeUpTime(EK_CoroKernelCurrBlock, &current->TCB_StateNode);
        }
    }

    // 设置当前的TCB状态为阻塞
    current->TCB_State = EK_CORO_BLOCKED;

    EK_EXIT_CRITICAL();
    // 请求一次调度
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
    if ((target_tcb->TCB_StateNode.CoroNode_List != EK_CoroKernelCurrBlock &&
         target_tcb->TCB_StateNode.CoroNode_List != EK_CoroKernelNextBlock) ||
        target_tcb->TCB_WakeUpTime != EK_MAX_DELAY)
    {
        EK_EXIT_CRITICAL();
        return EK_INVALID_PARAM; // 任务不在阻塞列表或不是永久阻塞
    }

    // 将任务移动到就绪链表
    target_tcb->TCB_State = EK_CORO_READY;
    EK_Result_t result =
        EK_rKernelMove_Tail(&EK_CoroKernelReadyList[target_tcb->TCB_Priority], &target_tcb->TCB_StateNode);

    if (result == EK_OK)
    {
        // 如果唤醒的任务比当前任务优先级更高，则请求调度
        if (target_tcb->TCB_Priority < EK_CoroKernelCurrentTCB->TCB_Priority)
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
    EK_ENTER_CRITICAL();
    // 获取当前的TCB
    EK_CoroTCB_t *current = EK_CoroKernelCurrentTCB;
    if (current == NULL)
    {
        EK_EXIT_CRITICAL();
        return;
    }

    // 设置当前的TCB状态为就绪
    current->TCB_State = EK_CORO_READY;
    // 插入到就绪链表
    EK_rKernelInsert_Tail(&EK_CoroKernelReadyList[current->TCB_Priority], &current->TCB_StateNode);

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
    if (target_tcb == EK_CoroKernelIdleHandler)
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
        target_tcb = EK_CoroKernelCurrentTCB;
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
        tcb = EK_CoroKernelCurrentTCB;
    }

    if (tcb == NULL)
    {
        return 0;
    }

    return tcb->TCB_StackSize;
}

/**
 * @brief 获取指定协程的堆栈历史最大使用量 (高水位)。
 * @details
 *  通过从栈底向上检查预设的填充值 (`EK_STACK_FILL_PATTERN`) 是否被覆盖，
 *  来计算出从任务开始运行到当前时刻，堆栈使用量的峰值。
 *  这对于调试和优化任务的堆栈大小非常有用。
 *  **注意**: 此功能要求在创建任务时，堆栈被填充了 `EK_STACK_FILL_PATTERN`。
 *  (当前版本在创建任务时未填充，此功能可能不准确，需在创建任务时增加填充逻辑)
 * @param task_handle 要查询的任务句柄。若为 NULL，则查询当前任务。
 * @return EK_Size_t 任务的堆栈高水位使用量 (以字节为单位)。
 */
EK_Size_t EK_uCoroGetStackUsage(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)task_handle;
    if (tcb == NULL)
    {
        tcb = EK_CoroKernelCurrentTCB;
    }

    if (tcb == NULL)
    {
        return 0;
    }

    uint8_t *stack_ptr = (uint8_t *)tcb->TCB_StackBase;
    EK_Size_t unused_size = 0;

    // 从栈底开始向上检查，直到找到第一个被修改过的字节
    while (*stack_ptr == EK_STACK_FILL_PATTERN && stack_ptr < (uint8_t *)tcb->TCB_StackBase + tcb->TCB_StackSize)
    {
        stack_ptr++;
    }

    // 计算未使用的大小
    unused_size = (EK_Size_t)(stack_ptr - (uint8_t *)tcb->TCB_StackBase);

    // 返回已使用的大小
    return tcb->TCB_StackSize - unused_size;
}

#endif /* EK_CORO_ENABLE == 1 */