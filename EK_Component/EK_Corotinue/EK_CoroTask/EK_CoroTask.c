/**
 * @file EK_CoroTask.c
 * @brief 协程任务管理API实现
 * @details 实现了所有面向用户的任务操作函数。
 * @author N1ntyNine99
 * @date 2025-09-20
 * @version 1.4
 */

#include "EK_CoroTask.h"

/**
 * @brief 协程任务栈填充值
 * @details 用于检测栈使用高水位
 */
#define EK_STACK_FILL_PATTERN (0xCD)

/**
 * @brief 
 * 
 */
static void v_coro_exit(void)
{
    // 尝试删除当前的任务
    EK_vCoroDelete(NULL, NULL);

    while (1)
    {
        // 理论上永远不会调用到这里
    }
}

/**
 * @brief 初始化TCB的上下文
 * @details 创建一个与PendSV异常进入时硬件自动保存的栈帧相兼容的栈帧结构。
 * @param tcb 想要初始化的TCB
 */
static inline void v_task_init_context(EK_CoroTCB_t *tcb)
{
    // 使用公共库函数填充整个栈区域，用于栈使用情况的监控
    EK_vMemSet(tcb->TCB_StackBase, EK_STACK_FILL_PATTERN, tcb->TCB_StackSize);

    // 注意：stk 的类型是 EK_CoroStack_t (即 uint32_t*), 而不是 EK_CoroStack_t*
    EK_CoroStack_t stk = (EK_CoroStack_t)((uint8_t *)tcb->TCB_StackBase + tcb->TCB_StackSize);

    // 使用 uintptr_t 进行地址对齐，这是更安全和可移植的做法
    stk = (EK_CoroStack_t)((uintptr_t)stk & ~0x07UL);

    *(--stk) = 0x01000000UL; // xPSR (Thumb bit)
    // 使用 uintptr_t 来进行指针到整数的转换，以消除警告
    *(--stk) = (uintptr_t)tcb->TCB_Entry; // PC (任务入口)
    *(--stk) = (uintptr_t)v_coro_exit; // LR (任务返回地址)
    *(--stk) = 0; // R12
    *(--stk) = 0; // R3
    *(--stk) = 0; // R2
    *(--stk) = 0; // R1
    *(--stk) = (uintptr_t)tcb->TCB_Arg; // R0 (任务参数)

    /* PendSV手动保存的上下文 */
    *(--stk) = 0xFFFFFFFD; // LR (EXC_RETURN: 返回线程模式, 使用PSP)
    *(--stk) = 0; // R11
    *(--stk) = 0; // R10
    *(--stk) = 0; // R9
    *(--stk) = 0; // R8
    *(--stk) = 0; // R7
    *(--stk) = 0; // R6
    *(--stk) = 0; // R5
    *(--stk) = 0; // R4

    tcb->TCB_SP = stk;
}

/**
 * @brief 让出CPU同时更改协程链表
 * 
 */
static inline void v_task_yield(void)
{
    // 查找下一个指向的任务块
    Kernel_NextTCB = (EK_CoroTCB_t *)Kernel_ReadyList.List_Head->CoroNode_OwnerTCB;
    // 如果选择的下个任务不是空闲任务就要从链表中删除
    if (Kernel_NextTCB != Kernel_IdleTaskHandler)
    {
        r_remove_node(&Kernel_ReadyList, Kernel_NextTCB);
    }
    // 更改任务状态
    Kernel_NextTCB->TCB_State = EK_CORO_RUNNING;

    // 请求一次调度
    EK_KERNEL_YIELD();
}

/**
 * @brief 动态创建一个协程
 * @param task_func 任务函数指针
 * @param task_arg 传递给任务的参数
 * @param priority 任务优先级
 * @param stack_size 任务堆栈大小 (字节)
 * @return EK_CoroHandler_t 协程句柄, NULL表示失败
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

    // 初始化TCB中的各个成员
    dynamic_tcb->TCB_Entry = task_func;
    dynamic_tcb->TCB_Arg = task_arg;
    dynamic_tcb->TCB_StackBase = stack;
    dynamic_tcb->TCB_Priority = priority;
    dynamic_tcb->TCB_StackSize = stack_size;
    dynamic_tcb->TCB_isDynamic = true; // 标记为动态创建
    dynamic_tcb->TCB_State = EK_CORO_READY;
    dynamic_tcb->TCB_Node.CoroNode_OwnerTCB = dynamic_tcb;
    dynamic_tcb->TCB_Node.CoroNode_Next = NULL;
    dynamic_tcb->TCB_Node.CoroNode_OwnerList = NULL;

    // 初始化任务的上下文（模拟CPU寄存器入栈）
    v_task_init_context(dynamic_tcb);

    // 将新创建的任务插入到就绪链表中
    r_insert_node(&Kernel_ReadyList, dynamic_tcb);

    // 返回任务句柄
    return (EK_CoroHandler_t)dynamic_tcb;
}

/**
 * @brief 静态创建一个协程
 * @param static_tcb 用户提供的静态TCB结构体指针
 * @param task_func 任务函数指针
 * @param task_arg 传递给任务的参数
 * @param priority 任务优先级
 * @param stack 用户提供的静态堆栈内存指针
 * @param stack_size 任务堆栈大小 (字节)
 * @return EK_CoroStaticHandler_t 静态协程句柄, NULL表示失败
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

    // 初始化用户传入的TCB结构体
    static_tcb->TCB_Entry = task_func;
    static_tcb->TCB_Arg = task_arg;
    static_tcb->TCB_Priority = priority;
    static_tcb->TCB_StackBase = stack;
    static_tcb->TCB_StackSize = stack_size;
    static_tcb->TCB_isDynamic = false; // 标记为静态创建
    static_tcb->TCB_State = EK_CORO_READY;
    static_tcb->TCB_Node.CoroNode_OwnerTCB = static_tcb;
    static_tcb->TCB_Node.CoroNode_Next = NULL;
    static_tcb->TCB_Node.CoroNode_OwnerList = NULL;

    // 初始化任务的上下文
    v_task_init_context(static_tcb);

    // 将任务插入到就绪链表
    r_insert_node(&Kernel_ReadyList, static_tcb);

    // 返回任务句柄
    return (EK_CoroHandler_t)static_tcb;
}

/**
 * @brief 挂起一个指定的协程
 * @param task_handle 要挂起的任务句柄，若为NULL则挂起当前任务。
 * @param result 指向操作结果的指针，用于返回状态。
 */
void EK_vCoroSuspend(EK_CoroHandler_t task_handle, EK_Result_t *result)
{
    bool self_suspend = false;
    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = Kernel_CurrentTCB;
        if (target_tcb == NULL)
        {
            if (result) *result = EK_ERROR;
            return;
        }
        self_suspend = true;
    }

    // 设置当前的状态
    target_tcb->TCB_State = EK_CORO_SUSPENDED;

    // 移动节点到挂起链表
    EK_Result_t op_res = r_move_node(&Kernel_SuspendList, target_tcb);

    if (result) *result = op_res;

    // 自挂起的话就请求一次调度
    if (self_suspend && op_res == EK_OK) v_task_yield();
}

/**
 * @brief 恢复一个指定的协程
 * @param task_handle 要恢复的任务句柄。
 * @param result 指向操作结果的指针，用于返回状态。
 */
void EK_vCoroResume(EK_CoroHandler_t task_handle, EK_Result_t *result)
{
    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    // 空指针处理
    if (target_tcb == NULL)
    {
        if (result) *result = EK_NULL_POINTER;
        return;
    }

    // 更新当前TCB的状态
    target_tcb->TCB_State = EK_CORO_READY;

    // 移动到就绪链表
    EK_Result_t op_res = r_move_node(&Kernel_ReadyList, target_tcb);
    if (result) *result = op_res;
}

/**
 * @brief 删除一个指定的协程
 * @param task_handle 要删除的任务句柄，若为NULL则删除当前任务。
 * @param result 指向操作结果的指针，用于返回状态。
 */
void EK_vCoroDelete(EK_CoroHandler_t task_handle, EK_Result_t *result)
{
    bool self_delete = false;

    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = Kernel_CurrentTCB;
        if (target_tcb == NULL)
        {
            if (result) *result = EK_ERROR;
            return;
        }
        self_delete = true;
    }

    // 对于静态任务，我们不能释放其资源，只能将其挂起
    if (target_tcb->TCB_isDynamic == false)
    {
        EK_vCoroSuspend(target_tcb, result);
        return;
    }

    // 对于动态任务
    if (self_delete)
    {
        // 标记等待删除，然后请求调度
        Kernel_DeleteTCB = target_tcb;
        *result = r_remove_node(target_tcb->TCB_Node.CoroNode_OwnerList, target_tcb);
        v_task_yield(); // 此调用不会返回
    }
    else
    {
        // 删除其他任务
        EK_Result_t op_res = r_remove_node(target_tcb->TCB_Node.CoroNode_OwnerList, target_tcb);
        if (op_res == EK_OK)
        {
            EK_FREE(target_tcb->TCB_StackBase);
            EK_FREE(target_tcb);
        }
        if (result) *result = op_res;
    }
}

/**
 * @brief 将当前协程延时指定的Tick数
 * @details
 *  - 将当前任务从就绪链表移动到阻塞链表。
 *  - 设置任务的延时滴答数。
 *  - 触发一次任务调度。
 * @param xticks 要延时的Tick数
 */
void EK_vCoroDelay(uint32_t xticks)
{
    // 获取当前的TCB
    EK_CoroTCB_t *current = Kernel_CurrentTCB;
    if (current == NULL) return;

    if (xticks > UINT32_MAX) xticks = UINT32_MAX;

    // 设置当前TCB的延时
    current->TCB_DelayTicks = xticks;

    // 设置当前的TCB状态为阻塞
    current->TCB_State = EK_CORO_BLOCKED;

    // 插入到阻塞链表
    r_move_node(&Kernel_BlockList, current);

    // 请求一次调度
    v_task_yield(); // 此调用不会返回
}

/**
 * @brief 当前协程主动让出CPU
 * @details
 *  - 将当前正在运行的任务重新放回就绪链表。
 *  - 任务状态由 RUNNING 变为 READY。
 *  - 触发一次任务调度，让其他协程有机会运行。
 */
void EK_vCoroYield(void)
{
    // 获取当前的TCB
    EK_CoroTCB_t *current = Kernel_CurrentTCB;
    if (current == NULL) return;

    // 设置当前的TCB状态为就绪
    current->TCB_State = EK_CORO_READY;

    // 插入到就绪链表
    r_move_node(&Kernel_ReadyList, current);

    // 请求调度
    v_task_yield();
}

/**
 * @brief 修改当指定任务的优先级
 * 
 * @param task_handle 要删除的任务句柄，若为NULL则删除当前任务。
 * @param priority 指定的优先级
 * @param result 指向操作结果的指针，用于返回状态。
 */
void EK_vCoroSetPriority(EK_CoroHandler_t task_handle, uint16_t priority, EK_Result_t *result)
{
    // 获取当前任务句柄
    EK_CoroTCB_t *target_tcb = (EK_CoroTCB_t *)task_handle;

    if (priority > UINT16_MAX) priority = UINT16_MAX;

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = Kernel_CurrentTCB;
        if (target_tcb == NULL)
        {
            if (result) *result = EK_ERROR;
            return;
        }
    }

    task_handle->TCB_Priority = priority;
    *result = EK_OK;
}

/**
 * @brief 获取指定协程的栈(全部大小)
 * @param task_handle 要查询的任务句柄，若为NULL则查询当前任务。
 * @return EK_Size_t 栈大小 (字节)
 */
EK_Size_t EK_uCoroGetStack(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)task_handle;
    if (tcb == NULL)
    {
        tcb = Kernel_CurrentTCB;
    }

    if (tcb == NULL)
    {
        return 0;
    }

    return tcb->TCB_StackSize;
}

/**
 * @brief 获取指定协程的栈使用量 (高水位)
 * @param task_handle 要查询的任务句柄，若为NULL则查询当前任务。
 * @return EK_Size_t 栈使用量 (字节)
 */
EK_Size_t EK_uCoroGetStackUsage(EK_CoroHandler_t task_handle)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)task_handle;
    if (tcb == NULL)
    {
        tcb = Kernel_CurrentTCB;
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
