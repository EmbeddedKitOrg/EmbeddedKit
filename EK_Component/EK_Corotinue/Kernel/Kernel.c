/**
 * @file Kernel.c
 * @brief EK_Coroutine 内核核心实现文件
 * @details
 *  实现了所有在 Kernel.h 中声明的函数。
 *  包含了内核的完整生命周期管理 (Init, Start)、上下文切换 (PendSV)、
 *  时间管理 (Tick)、以及底层的链表操作功能。
 *  同时，此文件还定义并实现了系统空闲任务。
 * @author N1ntyNine99
 * @date 2025-09-22
 * @version 1.5
 */

/* ========================= 头文件包含区 ========================= */
#include "Kernel.h"
#include "../MemPool/EK_MemPool.h"

/* ========================= 全局变量定义区 ========================= */

/** @brief 内核初始化状态标志 */
static bool Kernel_IsInited = false;

/** @brief 就绪任务列表 */
EK_CoroList_t Kernel_ReadyList;

/** @brief 阻塞任务列表 */
EK_CoroList_t Kernel_BlockList;

/** @brief 挂起任务列表 */
EK_CoroList_t Kernel_SuspendList;

/** @brief 当前正在运行的任务TCB指针 */
EK_CoroTCB_t *Kernel_CurrentTCB;

/** @brief 等待被删除的任务TCB指针 */
EK_CoroTCB_t *Kernel_NextTCB;

/** @brief 等待被删除的任务TCB指针 */
EK_CoroTCB_t *Kernel_DeleteTCB;

/** @brief 空闲任务TCB指针 */
EK_CoroTCB_t *Kernel_IdleTCB;

/** @brief 调度请求标志位，由TickHandler在唤醒任务时设置 */
static volatile bool Kernel_ReScheduleRequired = false;

/* ========================= 内部函数定义区 ========================= */

/**
 * @brief 启动第一个任务
 * @details 这是一个裸函数，只能由 EK_vKernelStart 调用一次。
 *          它负责将CPU上下文从主线程切换到第一个协程任务。
 */
__naked static void v_coro_launch(void)
{
    __asm volatile(
        // 加载第一个任务的堆栈指针到 PSP
        "ldr r0, =Kernel_CurrentTCB \n"
        "ldr r0, [r0] \n"
        "ldr r0, [r0] \n"
        "msr psp, r0 \n"

        // 切换 CONTROL 寄存器，使用 PSP 作为线程堆栈指针
        "mov r0, #2 \n"
        "msr control, r0 \n"
        "isb \n"

        // 恢复手动保存的上下文 (R4-R11 和 EXC_RETURN)
        "pop {r4-r11, lr} \n"

        // 恢复硬件保存的通用寄存器和任务的返回地址 (v_coro_exit)
        "pop {r0-r3, r12, lr} \n"

        // 此时，栈顶正好是任务的入口地址(PC)。直接将其弹出到 PC 寄存器，
        // 这将导致 CPU 立即跳转到该地址，开始执行第一个任务。
        "pop {pc} \n");
}

/**
 * @brief 向链表中插入一个tcb
 * 
 * @param list 待插入的链表
 * @param tcb 想要插入的tcb
 * @return EK_Result_t 
 */
EK_Result_t r_insert_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    // 空指针判断
    if (list == NULL || tcb == NULL) return EK_NULL_POINTER;

    EK_CoroListNode_t *node_to_insert = &tcb->TCB_Node; // 要插入的节点
    EK_CoroListNode_t *current = list->List_Head; // 遍历节点
    EK_CoroListNode_t *prev = NULL; // 要插入的节点的前一个节点

    EK_ENTER_CRITICAL();
    // 遍历链表
    while (current != NULL)
    {
        // 获取当前遍历的节点的TCB
        EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current->CoroNode_OwnerTCB;

        // 找到插入位置
        if (tcb->TCB_Priority < current_tcb->TCB_Priority) break;

        // 未找到位置 更新两个指针
        prev = current;
        current = current->CoroNode_Next;
    }
    // 更新当前的节点的Next节点
    node_to_insert->CoroNode_Next = current;

    // 头节点处理
    if (prev == NULL) list->List_Head = node_to_insert;
    else prev->CoroNode_Next = node_to_insert;

    // 更新节点信息
    node_to_insert->CoroNode_OwnerList = list;
    list->List_Count++;
    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 从链表中移除一个tcb
 * 
 * @param list 待移除的链表
 * @param tcb 想要移除的tcb
 * @return EK_Result_t 
 */
EK_Result_t r_remove_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    // 入参检测
    if (list == NULL || tcb == NULL) return EK_NULL_POINTER;
    if (list->List_Count == 0) return EK_ERROR;
    if (tcb->TCB_Node.CoroNode_OwnerList != list) return EK_ERROR;

    EK_CoroListNode_t *node_to_remove = &tcb->TCB_Node; // 要删除的节点
    EK_CoroListNode_t *current = list->List_Head; // 遍历节点
    EK_CoroListNode_t *prev = NULL; // 要插入的节点的前一个节点

    EK_ENTER_CRITICAL();
    // 遍历节点 找到要删除的节点
    while (current != NULL && current != node_to_remove)
    {
        prev = current;
        current = current->CoroNode_Next;
    }

    // 没有找到节点 返回not found
    if (current == NULL) return EK_NOT_FOUND;

    // prev == NUL 说明是头节点
    if (prev == NULL) list->List_Head = node_to_remove->CoroNode_Next;
    else prev->CoroNode_Next = node_to_remove->CoroNode_Next;

    // 更新节点相关参数
    node_to_remove->CoroNode_Next = NULL;
    node_to_remove->CoroNode_OwnerList = NULL;
    list->List_Count--;
    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 将tcb移动到指定链表
 * 
 * @param list 待移动的链表
 * @param tcb 想要移动的txb
 * @return EK_Result_t 
 */
EK_Result_t r_move_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    // 入参检测
    if (list == NULL || tcb == NULL) return EK_NULL_POINTER;
    if (tcb->TCB_Node.CoroNode_OwnerList == list) return EK_OK;

    // 当前节点有所属链表，则先将节点从原链表中移除
    if (tcb->TCB_Node.CoroNode_OwnerList != NULL)
    {
        EK_CoroList_t *source_list = (EK_CoroList_t *)tcb->TCB_Node.CoroNode_OwnerList;
        EK_Result_t remove_res = r_remove_node(source_list, tcb);
        if (remove_res != EK_OK) return remove_res;
    }

    // 将节点插入目标链表
    return r_insert_node(list, tcb);
}

/* ========================= 空闲任务实现区 ========================= */

#ifndef EK_IDLE_TASK_STACK_SIZE
#define EK_IDLE_TASK_STACK_SIZE (256) // 定义空闲任务的堆栈大小
#endif

// 空闲任务的TCB和堆栈的静态存储
static EK_CoroStack_t Kernel_IdleTaskStack[EK_IDLE_TASK_STACK_SIZE];
static EK_CoroTCB_t Kernel_IdleTaskTCB;
// 空闲任务的任务句柄
EK_CoroStaticHandler_t Kernel_IdleTaskHandler;

/**
 * @brief 空闲任务钩子函数的弱定义
 * @details 
 *  这是一个弱函数（weak function），用户可以在自己的代码中定义一个同名函数来覆盖它。
 *  此函数在系统空闲时，会在一个循环中被内核的空闲任务所调用。
 *  用户可以在此函数中实现单次的自定义行为，例如执行WFI指令进入低功耗模式、喂狗等。
 *  **注意：此函数的实现不应包含任何形式的死循环或长时间阻塞。**
 * 
 * @par 自定义实现示例
 *  @code
 *  void EK_CoroIdle(void)
 *  {
 *       // 执行WFI指令让CPU睡眠，直到下一次中断发生。
 *       __asm volatile ("wfi"); 
 *  }
 *  @endcode
 */
__weak void EK_CoroIdle(void)
{
    // 用户可在此实现单次的空闲操作，例如：
    // __asm volatile ("wfi");
}

/**
 * @brief 内核私有的空闲任务循环体
 * @details 这是空闲任务的真正入口点。它在一个无限循环中运行，负责：
 *          1. 调用用户可重写的 EK_CoroIdle() 钩子函数。
 *          2. 检查是否需要重新调度，并在需要时让出CPU。
 * @param arg 传递给任务的参数 (未使用)
 */
static void Kernel_CoroIdleFunction(void *arg)
{
    UNUSED_VAR(arg);
    while (1)
    {
        if (Kernel_DeleteTCB != NULL)
        {
            // 仅当任务是动态创建时才释放内存
            if (Kernel_DeleteTCB->TCB_isDynamic)
            {
                EK_FREE(Kernel_DeleteTCB->TCB_StackBase);
                EK_FREE(Kernel_DeleteTCB);
            }
            Kernel_DeleteTCB = NULL;
        }

        // 检查是否在中断中有任务被唤醒，需要调度
        if (Kernel_ReScheduleRequired == true)
        {
            // 清除标志位并让出CPU，以便调度器运行新就绪的任务
            Kernel_ReScheduleRequired = false;

            // 调用标准的Yield函数，让出CPU
            EK_vCoroYield();
        }

        // 执行用户定义的单次空闲操作
        EK_CoroIdle();
    }
}

/* ========================= 内核核心API函数 ========================= */

/**
 * @brief 初始化协程内核
 * @details
 *  - 初始化所有任务列表。
 *  - 初始化全局变量。
 *  - 将内核状态标记为已初始化。
 */
void EK_vKernelInit(void)
{
    if (Kernel_IsInited == true) return;

    //初始化内存池
    while (EK_bMemPool_Init() != true);

    // 初始化三个链表
    Kernel_ReadyList.List_Count = 0;
    Kernel_ReadyList.List_Head = NULL;

    Kernel_BlockList.List_Count = 0;
    Kernel_BlockList.List_Head = NULL;

    Kernel_SuspendList.List_Count = 0;
    Kernel_SuspendList.List_Head = NULL;

    // 初始化指针
    Kernel_CurrentTCB = NULL;
    Kernel_DeleteTCB = NULL;
    Kernel_NextTCB = NULL;

    // 创建空闲任务，它将保证就绪列表永远不为空
    Kernel_IdleTaskHandler = EK_pCoroCreateStatic(&Kernel_IdleTaskTCB,
                                                  Kernel_CoroIdleFunction,
                                                  NULL,
                                                  UINT16_MAX, // 最低优先级
                                                  Kernel_IdleTaskStack,
                                                  EK_IDLE_TASK_STACK_SIZE);

    Kernel_IsInited = true;
}

/**
 * @brief 启动协程调度器
 * @details
 *  - 确保内核已初始化。
 *  - 创建并注册内部的空闲任务。
 *  - 选出就绪列表中优先级最高的任务作为第一个任务。
 *  - 调用底层汇编函数 v_coro_launch() 启动第一个任务并开始调度。
 * @note 此函数不会返回。
 */
void EK_vKernelStart(void)
{
    if (Kernel_IsInited == false) EK_vKernelInit();

    // 手动指定第一个要运行的任务 (现在肯定不为空)
    Kernel_CurrentTCB = (EK_CoroTCB_t *)Kernel_ReadyList.List_Head->CoroNode_OwnerTCB;
    r_remove_node(&Kernel_ReadyList, Kernel_CurrentTCB);
    Kernel_CurrentTCB->TCB_State = EK_CORO_RUNNING;

    // 调用汇编函数启动第一个任务
    v_coro_launch();
}

/**
 * @brief 内核时钟节拍处理函数
 * @details
 *  此函数应在系统的时钟节拍中断（如SysTick_Handler）中被周期性调用。
 *  它负责遍历阻塞任务列表，将延时时间已到的任务唤醒到就绪列表中，
 *  并设置重新调度请求标志，以便空闲任务在适当时机让出CPU。
 */
void EK_vTickHandler(void)
{
    // 遍历阻塞链表
    if (Kernel_BlockList.List_Count > 0)
    {
        EK_CoroListNode_t *current_node = Kernel_BlockList.List_Head;

        // 遍历链表
        while (current_node != NULL)
        {
            // 必须先保存下一个节点，因为r_move_node会修改当前节点的链接
            EK_CoroListNode_t *next_node = current_node->CoroNode_Next;
            EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)(current_node->CoroNode_OwnerTCB);

            // 延时递减
            if (current_tcb->TCB_DelayTicks > 0)
            {
                current_tcb->TCB_DelayTicks--;
            }

            // 如果延时时间到，则唤醒任务
            if (current_tcb->TCB_DelayTicks == 0)
            {
                // 将任务移回就绪链表
                current_tcb->TCB_State = EK_CORO_READY;
                r_move_node(&Kernel_ReadyList, current_tcb);

                Kernel_ReScheduleRequired = true;
            }

            // 处理下一个节点
            current_node = next_node;
        }
    }
}

/**
 * @brief Coroutine内核的PendSV处理函数
 * @details 这是一个裸函数，必须由用户在实际的PendSV_Handler中调用。
 *          它负责保存和恢复上下文，并调用v_coro_do_schedule执行调度。
 */
__naked void EK_vPendSVHandler(void)
{
    __asm volatile(
        // 保存当前任务的上下文
        // 获取当前任务的PSP
        "mrs r0, psp \n"

#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
        // 测试 LR 的 bit 4, 判断 FPU 寄存器是否已由硬件自动压栈
        "tst lr, #0x10 \n"
        // 如果结果不为0 (Not Equal)，则跳转到标签 skip_fpu_save，跳过FPU寄存器保存
        "bne skip_fpu_save \n"
        // 如果结果为0，则顺序执行以下指令，保存 S16-S31
        "vstmdb r0!, {s16-s31} \n"
        "skip_fpu_save: \n"
#endif

        // 将核心寄存器 R4-R11 和 LR(EXC_RETURN) 压入当前任务的堆栈
        "stmdb r0!, {r4-r11, lr} \n"

        // 保存新的栈顶指针到 TCB
        "ldr r1, =Kernel_CurrentTCB \n"
        "ldr r1, [r1] \n"
        "str r0, [r1] \n"

        // 执行调度: Kernel_CurrentTCB = Kernel_NextTCB
        "ldr r0, =Kernel_NextTCB \n"
        "ldr r0, [r0] \n"
        "ldr r1, =Kernel_CurrentTCB \n"
        "str r0, [r1] \n"

        // 恢复新任务的上下文

        // 恢复新任务的SP
        "ldr r1, =Kernel_CurrentTCB \n"
        "ldr r1, [r1] \n"
        "ldr r0, [r1] \n"

        // 从新任务的堆栈中恢复 R4-R11 和 LR(EXC_RETURN)
        "ldmia r0!, {r4-r11, lr} \n"

#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
        // 同样，根据新任务的 LR 值，判断是否需要恢复 FPU 寄存器
        "tst lr, #0x10 \n"
        //    如果不为0，则跳转跳过 FPU 寄存器恢复
        "bne skip_fpu_restore \n"
        //    如果为0，则恢复 S16-S31
        "vldmia r0!, {s16-s31} \n"
        "skip_fpu_restore: \n"
#endif

        // 更新 PSP
        "msr psp, r0 \n"

        // 异常返回
        "bx lr \n");
}