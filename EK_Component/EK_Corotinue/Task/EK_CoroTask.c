/**
 * @file EK_CoroTask.c
 * @brief 协程任务管理API实现
 * @details 实现了所有面向用户的任务操作函数。
 * @author N1ntyNine99
 * @date 2025-09-25
 * @version 1.8
 */

#include "EK_CoroTask.h"

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
 * @brief 任务意外退出的函数返回值
 * 
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

// EK_CoroTask.c

/**
 * @brief 初始化TCB的上下文
 * @details 
 * 在任务的堆栈顶部创建一个伪造的上下文帧。这个帧的布局必须精确匹配
 * PendSV_Handler中上下文保存的顺序，以及Cortex-M硬件异常处理机制。
 * 堆栈布局 (从高地址 -> 低地址):
 * - xPSR: 初始程序状态寄存器 (Thumb模式, T-bit = 1)
 * - PC  : 任务入口函数地址
 * - LR  : 任务返回地址 (v_coro_exit)
 * - R12, R3, R2, R1
 * - R0  : 任务入口函数的参数
 * --- 以上由硬件自动管理 ---
 * - EXC_RETURN: 特殊的连接寄存器值，指导异常如何返回 (0xFFFFFFFD)
 * - R11, R10, R9, R8, R7, R6, R5, R4: C函数需要保存的寄存器
 * --- 以上由软件(PendSV_Handler)手动管理 ---
 * * @param tcb 指向要初始化上下文的任务控制块 (TCB)
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
    dynamic_tcb->TCB_Node.CoroNode_OwnerTCB = dynamic_tcb;
    dynamic_tcb->TCB_Node.CoroNode_Next = NULL;
    dynamic_tcb->TCB_Node.CoroNode_OwnerList = NULL;

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)
    dynamic_tcb->TCB_pMsg = NULL;
    dynamic_tcb->TCB_MsgResult = EK_OK;
#endif

    // 初始化任务的上下文（模拟CPU寄存器入栈）
    v_task_init_context(dynamic_tcb);

    // 将新创建的任务插入到就绪链表中
    EK_rKernelInsert_Tail(&EK_CoroKernelReadyList[priority], dynamic_tcb);

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
    static_tcb->TCB_Node.CoroNode_OwnerTCB = static_tcb;
    static_tcb->TCB_Node.CoroNode_Next = NULL;
    static_tcb->TCB_Node.CoroNode_OwnerList = NULL;

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)
    static_tcb->TCB_pMsg = NULL;
    static_tcb->TCB_MsgResult = EK_OK;
#endif

    // 初始化任务的上下文
    v_task_init_context(static_tcb);

    // 将任务插入到就绪链表
    EK_rKernelInsert_Tail(&EK_CoroKernelReadyList[priority], static_tcb);

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

    // 禁止操作空闲任务
    if (target_tcb == EK_CoroKernelIdleHandler)
    {
        *result = EK_INVALID_PARAM;
        return;
    }

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = EK_CoroKernelCurrentTCB;
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
    EK_Result_t op_res = EK_rKernelMove_Tail(&EK_CoroKernelSuspendList, target_tcb);

    if (result) *result = op_res;

    // 自挂起的话就请求一次调度
    if (self_suspend && op_res == EK_OK) EK_vKernelYield();
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

    // 禁止操作空闲任务
    if (target_tcb == EK_CoroKernelIdleHandler)
    {
        *result = EK_INVALID_PARAM;
        return;
    }

    // 空指针处理
    if (target_tcb == NULL)
    {
        if (result) *result = EK_NULL_POINTER;
        return;
    }

    // 更新当前TCB的状态
    target_tcb->TCB_State = EK_CORO_READY;

    // 移动到就绪链表
    EK_Result_t op_res = EK_rKernelMove_Tail(&EK_CoroKernelReadyList[target_tcb->TCB_Priority], target_tcb);
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

    // 禁止操作空闲任务
    if (target_tcb == EK_CoroKernelIdleHandler)
    {
        *result = EK_INVALID_PARAM;
        return;
    }

    // 空指针处理
    if (target_tcb == NULL)
    {
        target_tcb = EK_CoroKernelCurrentTCB;
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
        EK_CoroKernelDeleteTCB = target_tcb;
        *result = EK_rKernelRemove(target_tcb->TCB_Node.CoroNode_OwnerList, target_tcb);
        EK_vKernelYield(); // 此调用不会返回
    }
    else
    {
        // 删除其他任务
        EK_Result_t op_res = EK_rKernelRemove(target_tcb->TCB_Node.CoroNode_OwnerList, target_tcb);
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
    EK_CoroTCB_t *current = EK_CoroKernelCurrentTCB;
    if (current == NULL) return;

    // 禁止操作空闲任务
    if (current == EK_CoroKernelIdleHandler)
    {
        return;
    }

    if (xticks > UINT32_MAX) xticks = UINT32_MAX;

    // 设置当前TCB的唤醒时间
    current->TCB_WakeUpTime = xticks + EK_CoroKernelGetTick();

    // 设置当前的TCB状态为阻塞
    current->TCB_State = EK_CORO_BLOCKED;

    // 按照唤醒时间插入到阻塞链表
    EK_rKernelMove(&EK_CoroKernelBlockList, current);

    // 请求一次调度
    EK_vKernelYield(); // 此调用不会返回
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
    EK_CoroTCB_t *current = EK_CoroKernelCurrentTCB;
    if (current == NULL) return;

    // 设置当前的TCB状态为就绪
    current->TCB_State = EK_CORO_READY;
    // 插入到就绪链表
    EK_rKernelInsert_Tail(&EK_CoroKernelReadyList[current->TCB_Priority], current);

    // 请求调度
    EK_vKernelYield();
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

    // 禁止操作空闲任务
    if (target_tcb == EK_CoroKernelIdleHandler)
    {
        *result = EK_INVALID_PARAM;
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
        tcb = EK_CoroKernelCurrentTCB;
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
