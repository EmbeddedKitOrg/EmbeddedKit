/**
 * @file Kernel.c
 * @brief EK_Coroutine 内核核心实现文件
 * @details
 *  实现了所有在 Kernel.h 中声明的函数。
 *  包含了内核的完整生命周期管理 (Init, Start)、上下文切换 (PendSV)、
 *  时间管理 (Tick)、以及底层的链表操作功能。
 *  同时, 此文件还定义并实现了系统空闲任务。
 * @author N1ntyNine99
 * @date 2025-09-25
 * @version 1.8
 */

/* ========================= 头文件包含区 ========================= */
#include "Kernel.h"
#include "../MemPool/EK_MemPool.h"
#include "EK_CoroTask.h"

/* ========================= 内部宏定义区 ========================= */

/**
 * @brief 位图的位数
 * 
 */
#define EK_BITMAP_MAX_BIT (sizeof(EK_BitMap_t) * 8 - 1)

/**
 * @brief 使用内置的CLZ(Count Leading Zeros)计算最高有效位(MSB)的索引
 * @details 这是一个表达式宏, 返回一个uint8_t类型的值。
 *          例如 EK_KERNEL_CLZ(0b01100000) 会得到 6.
 *          注意：当传入的参数为0时, 硬件CLZ指令的行为是未定义的。
 *          本宏的实现不处理这种情况, 调用者必须保证传入的__BITMAP__不为0。
 */
#if (defined(__GNUC__) || defined(__clang__))
#define EK_KERNEL_CLZ(__BITMAP__) ((__BITMAP__) == 0 ? 0 : (uint8_t)(31 - __builtin_clz(__BITMAP__)))
#elif defined(__CC_ARM)
#define EK_KERNEL_CLZ(__BITMAP__) ((__BITMAP__) == 0 ? 0 : (uint8_t)(31 - __clz(__BITMAP__)))
#elif defined(__ICCARM__)
#define EK_KERNEL_CLZ(__BITMAP__) ((__BITMAP__) == 0 ? 0 : (uint8_t)(31 - __CLZ(__BITMAP__)))
#else
// 软件实现作为备用
static inline uint8_t v_kernel_find_msb_index(EK_BitMap_t val)
{
    if (val == 0) return 0;
    uint8_t msb_idx = 0;
    while ((val >>= 1) > 0)
    {
        msb_idx++;
    }
    return msb_idx;
}
#define EK_KERNEL_CLZ(__BITMAP__) v_kernel_find_msb_index(__BITMAP__)
#endif

/**
 * @brief 计算最高的优先级
 * 
 */
#define EK_KERNEL_GET_HIGHEST_PRIO(__BITMAP__) (EK_BITMAP_MAX_BIT - EK_KERNEL_CLZ(__BITMAP__))

/**
 * @brief 协程空闲任务堆栈大小
 * 
 */
#ifndef EK_CORO_IDLE_TASK_STACK_SIZE
#if (EK_CORO_FPU_USED == 0)
#define EK_CORO_IDLE_TASK_STACK_SIZE (256) // 定义空闲任务的堆栈大小
#else
#define EK_CORO_IDLE_TASK_STACK_SIZE (512) // 定义空闲任务的堆栈大小
#endif
#endif

/* ========================= 全局变量定义区 ========================= */
EK_CoroList_t EK_CoroKernelReadyList[EK_CORO_PRIORITY_GROUPS]; // 就绪任务列表
EK_CoroList_t EK_CoroKernelBlockList; // 阻塞任务列表
EK_CoroList_t EK_CoroKernelSuspendList; // 挂起任务列表
EK_CoroTCB_t *EK_CoroKernelCurrentTCB; // 当前正在运行的任务TCB指针
EK_CoroTCB_t *EK_CoroKernelDeleteTCB; // 等待被删除的任务TCB指针
EK_GetTickFunction_t EK_CoroKernelGetTick; //获取时基的函数
EK_CoroTCB_t *KernelNextTCB; // 下一个任务TCB
static EK_CoroTCB_t *EK_CoroKernelIdleTCB; // 空闲任务TCB指针
static bool KernelIdleYield = false; // 调度请求标志位, 由TickHandler在唤醒任务时设置
static bool KernelIsInited = false; // 内核初始化状态标志
static volatile EK_BitMap_t KernelReadyBitMap; // 就绪链表位图

/* ========================= 内部函数定义区 ========================= */
/**
 * @brief 向链表中插入一个tcb (内部函数)
 * 
 * @param list 待插入的链表
 * @param tcb 想要插入的tcb
 * @param is_sorted 是否按时间排序插入 (false则插入到尾部)
 * @return EK_Result_t 
 */
static inline EK_Result_t _r_insert_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb, bool is_sorted)
{
    if (list == NULL || tcb == NULL) return EK_NULL_POINTER;

    EK_CoroListNode_t *node_to_insert = &tcb->TCB_Node;
    node_to_insert->CoroNode_Next = NULL;

    EK_ENTER_CRITICAL();

    if (list->List_Head == NULL) // 如果链表为空
    {
        list->List_Head = node_to_insert;
        list->List_Tail = node_to_insert;
    }
    else if (is_sorted) // 按照时间顺序插入链表
    {
        EK_CoroListNode_t *current = list->List_Head;
        EK_CoroListNode_t *prev = NULL;

        while (current != NULL)
        {
            EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current->CoroNode_OwnerTCB;
            if (tcb->TCB_WakeUpTime < current_tcb->TCB_WakeUpTime) break;
            prev = current;
            current = current->CoroNode_Next;
        }

        if (prev == NULL) // 插入到头部
        {
            node_to_insert->CoroNode_Next = list->List_Head;
            list->List_Head = node_to_insert;
        }
        else // 插入到中间或尾部
        {
            node_to_insert->CoroNode_Next = prev->CoroNode_Next;
            prev->CoroNode_Next = node_to_insert;
        }

        if (node_to_insert->CoroNode_Next == NULL) // 如果插入到了尾部, 更新Tail指针
        {
            list->List_Tail = node_to_insert;
        }
    }
    else // 尾部插入 (O(1) 操作)
    {
        list->List_Tail->CoroNode_Next = node_to_insert;
        list->List_Tail = node_to_insert;
    }

    node_to_insert->CoroNode_OwnerList = list;
    list->List_Count++;

    if (list != &EK_CoroKernelBlockList && list != &EK_CoroKernelSuspendList)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

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
EK_Result_t EK_rKernelRemove(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    // 入参检查
    if (list == NULL || tcb == NULL) return EK_NULL_POINTER;
    if (list->List_Count == 0) return EK_ERROR;
    if (tcb->TCB_Node.CoroNode_OwnerList != list) return EK_ERROR;

    EK_CoroListNode_t *node_to_remove = &tcb->TCB_Node; // 要删除的TCB的节点
    EK_CoroListNode_t *current = list->List_Head; // 遍历节点
    EK_CoroListNode_t *prev = NULL; // 前个节点

    EK_ENTER_CRITICAL();

    // 找到要删除的节点
    while (current != NULL && current != node_to_remove)
    {
        prev = current;
        current = current->CoroNode_Next;
    }

    if (current == NULL) // 没有找到节点
    {
        EK_EXIT_CRITICAL();
        return EK_NOT_FOUND;
    }

    if (prev == NULL) // 移除的是头节点
    {
        list->List_Head = node_to_remove->CoroNode_Next;
    }
    else
    {
        prev->CoroNode_Next = node_to_remove->CoroNode_Next;
    }

    if (list->List_Tail == node_to_remove) // 如果移除的是尾节点, 更新Tail指针
    {
        list->List_Tail = prev;
    }

    // 更新链表信息
    list->List_Count--;

    // 更新节点信息
    node_to_remove->CoroNode_OwnerList = NULL;
    node_to_remove->CoroNode_Next = NULL;

    // 设置位图
    if (list->List_Count == 0 && list != &EK_CoroKernelBlockList && list != &EK_CoroKernelSuspendList)
    {
        EK_vClearBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 将tcb移动到指定链表 (内部函数)
 * 
 * @param list 待移动的链表
 * @param tcb 想要移动的tcb
 * @param is_sorted 是否按时间排序插入 (false则插入到尾部)
 * @return EK_Result_t 
 */
static inline EK_Result_t _r_move_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb, bool is_sorted)
{
    if (list == NULL || tcb == NULL) return EK_NULL_POINTER;

    if (tcb->TCB_Node.CoroNode_OwnerList != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)tcb->TCB_Node.CoroNode_OwnerList, tcb);
        if (remove_res != EK_OK) return remove_res;
    }

    return _r_insert_node(list, tcb, is_sorted);
}

/**
 * @brief 向链表中插入一个tcb
 * 
 * @param list 待插入的链表
 * @param tcb 想要插入的tcb
 * @details 按照唤醒时间从小到大的顺序插入链表
 * @return EK_Result_t 
 */
EK_Result_t EK_rKernelInsert(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    return _r_insert_node(list, tcb, true);
}

/**
 * @brief 向链表末尾插入一个tcb
 * 
 * @param list 待插入的链表
 * @param tcb 想要插入的tcb
 * @return EK_Result_t 
 */
EK_Result_t EK_rKernelInsert_Tail(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    return _r_insert_node(list, tcb, false);
}

/**
 * @brief 将tcb移动到指定链表
 * 
 * @param list 待移动的链表
 * @param tcb 想要移动的txb
 * @details 按照唤醒时间从小到大的顺序插入链表
 * @return EK_Result_t 
 */
EK_Result_t EK_rKernelMove(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    return _r_move_node(list, tcb, true);
}

/**
 * @brief 将tcb移动到指定链表的尾部
 * 
 * @param list 待移动的链表
 * @param tcb 想要移动的txb
 * @return EK_Result_t 
 */
EK_Result_t EK_rKernelMove_Tail(EK_CoroList_t *list, EK_CoroTCB_t *tcb)
{
    return _r_move_node(list, tcb, false);
}

/* ========================= 空闲任务实现区 ========================= */

static uint8_t Kernel_IdleTaskStack[EK_CORO_IDLE_TASK_STACK_SIZE];
static EK_CoroTCB_t Kernel_IdleTaskTCB;
EK_CoroStaticHandler_t EK_CoroKernelIdleHandler;

__weak void EK_CoroIdle(void)
{
}

static void Kernel_CoroIdleFunction(void *arg)
{
    UNUSED_VAR(arg);
    while (1)
    {
        if (EK_CoroKernelDeleteTCB != NULL)
        {
            if (EK_CoroKernelDeleteTCB->TCB_isDynamic)
            {
                EK_FREE(EK_CoroKernelDeleteTCB->TCB_StackBase);
                EK_FREE(EK_CoroKernelDeleteTCB);
            }
            EK_CoroKernelDeleteTCB = NULL;
        }

        if (KernelIdleYield == true)
        {
            KernelIdleYield = false;
            EK_vCoroYield();
        }

        EK_CoroIdle();
    }
}

/* ========================= 内核核心API函数 ========================= */

__naked static void v_kernel_start(void)
{
    __ASM volatile(
        // 加载第一个任务的堆栈指针到 PSP
        "ldr r0, =EK_CoroKernelCurrentTCB \n"
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
 * @brief 初始化协程内核
 * @details
 *  - 初始化所有任务列表。
 *  -初始化全局变量。
 *  -将内核状态标记为已初始化。
 */
void EK_vKernelInit(void)
{
    if (KernelIsInited == true) return;

    while (EK_bMemPool_Init() != true);

    for (uint8_t i = 0; i < EK_CORO_PRIORITY_GROUPS; i++)
    {
        EK_CoroKernelReadyList[i].List_Count = 0;
        EK_CoroKernelReadyList[i].List_Head = NULL;
        EK_CoroKernelReadyList[i].List_Tail = NULL;
    }
    EK_CoroKernelBlockList.List_Count = 0;
    EK_CoroKernelBlockList.List_Head = NULL;
    EK_CoroKernelBlockList.List_Tail = NULL;

    EK_CoroKernelSuspendList.List_Count = 0;
    EK_CoroKernelSuspendList.List_Head = NULL;
    EK_CoroKernelSuspendList.List_Tail = NULL;

    KernelReadyBitMap = 0;
    EK_CoroKernelCurrentTCB = NULL;
    EK_CoroKernelDeleteTCB = NULL;
    KernelNextTCB = NULL;
    EK_CoroKernelGetTick = NULL;

    EK_CoroKernelIdleHandler = EK_pCoroCreateStatic(&Kernel_IdleTaskTCB,
                                                    Kernel_CoroIdleFunction,
                                                    NULL,
                                                    EK_CORO_PRIORITY_MOUNT - 1, // 最低优先级
                                                    Kernel_IdleTaskStack,
                                                    EK_CORO_IDLE_TASK_STACK_SIZE);

    KernelIsInited = true;
}

/**
 * @brief 启动协程调度器
 * @details
 *  - 确保内核已初始化。
 *  - 选出就绪列表中优先级最高的任务作为第一个任务。
 *  - 调用裸函数 v_kernel_start() 启动第一个任务。
 * @note 此函数不会返回。
 */
void EK_vKernelStart(EK_GetTickFunction_t get_tick)
{
    if (KernelIsInited == false) EK_vKernelInit();
    if (get_tick == NULL) return;

    EK_ENTER_CRITICAL();

    EK_CoroKernelGetTick = get_tick;

    if (KernelReadyBitMap == 0)
    {
        return;
    }

    // 找到优先级最高的TCB 启动
    uint8_t highest_prio = EK_KERNEL_GET_HIGHEST_PRIO(KernelReadyBitMap);
    EK_CoroKernelCurrentTCB = (EK_CoroTCB_t *)EK_CoroKernelReadyList[highest_prio].List_Head->CoroNode_OwnerTCB;
    EK_CoroKernelCurrentTCB->TCB_State = EK_CORO_RUNNING;
    EK_rKernelRemove(&EK_CoroKernelReadyList[highest_prio], EK_CoroKernelCurrentTCB);

    EK_EXIT_CRITICAL();

    // 手动触发一次调度来启动第一个任务
    v_kernel_start();

    // 此函数不应返回
    while (1);
}

/**
 * @brief 让出CPU同时更改协程链表
 * 
 */
void EK_vKernelYield(void)
{
    EK_ENTER_CRITICAL();

    // 找到最高优先级
    uint8_t highest_prio = EK_KERNEL_GET_HIGHEST_PRIO(KernelReadyBitMap);

    // 更新链表
    KernelNextTCB = (EK_CoroTCB_t *)EK_CoroKernelReadyList[highest_prio].List_Head->CoroNode_OwnerTCB;
    KernelNextTCB->TCB_State = EK_CORO_RUNNING;

    // 将TCB移除
    EK_rKernelRemove(&EK_CoroKernelReadyList[highest_prio], KernelNextTCB);

    EK_EXIT_CRITICAL();

    // 申请一次调度
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}

/**
 * @brief 内核时钟节拍处理函数
 * @details
 *  此函数应在系统的时钟节拍中断（如SysTick_Handler）中被周期性调用。
 *  它负责遍历阻塞任务列表, 将延时时间已到的任务唤醒到就绪列表中。
 */
void EK_vTickHandler(void)
{
    // 无阻塞直接退出
    if (EK_CoroKernelBlockList.List_Count == 0) return;

    EK_ENTER_CRITICAL();

    uint32_t current_tick = EK_CoroKernelGetTick(); // 获取当前的Tick

    EK_CoroListNode_t *current_node = EK_CoroKernelBlockList.List_Head; // 遍历节点

    while (current_node != NULL)
    {
        EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current_node->CoroNode_OwnerTCB; // 获取当前的TCB

        // 当前的Tick小于阻塞链表当前节点的唤醒时间：说明该节点之后都没有任务唤醒
        if (current_tick < current_tcb->TCB_WakeUpTime) break;

        EK_CoroListNode_t *next_node = current_node->CoroNode_Next; // 存储下一个节点

        current_tcb->TCB_State = EK_CORO_READY; // 任务唤醒 先修改标志位

        EK_rKernelMove_Tail(&EK_CoroKernelReadyList[current_tcb->TCB_Priority], current_tcb); // 移动到就绪链表的尾节点

        // 如果当前为空闲任务 并且有任务唤醒 则向空闲任务中申请一次调度
        if (EK_CoroKernelCurrentTCB == EK_CoroKernelIdleHandler)
        {
            KernelIdleYield = true;
        }

        current_node = next_node; //步进
    }

    EK_EXIT_CRITICAL();
}

/**
 * @brief Coroutine内核的PendSV处理函数
 * @details 这是一个裸函数, 必须由用户在实际的PendSV_Handler中调用。
 *          它负责保存和恢复上下文, 并执行调度。
 */
__naked void EK_vKernelPendSV_Handler(void)
{
    __ASM volatile(
        // 保存当前任务的上下文
        // 获取当前任务的PSP
        "mrs r0, psp \n"

        // 将核心寄存器 R4-R11 和 LR(EXC_RETURN) 压入当前任务的堆栈
        "stmdb r0!, {r4-r11, lr} \n"

        // 保存新的栈顶指针到 TCB
        "ldr r1, =EK_CoroKernelCurrentTCB \n"
        "ldr r1, [r1] \n" // 解引用得到SP位置
        "str r0, [r1] \n" // 将r0寄存器中的数据存储的SP中

        // 执行调度: EK_CoroKernelCurrentTCB = KernelNextTCB
        "ldr r0, =KernelNextTCB \n"
        "ldr r0, [r0] \n"
        "ldr r1, =EK_CoroKernelCurrentTCB \n"
        "str r0, [r1] \n"

        // 恢复新任务的SP
        "ldr r1, =EK_CoroKernelCurrentTCB \n"
        "ldr r1, [r1] \n"
        "ldr r0, [r1] \n"

        // 从新任务的堆栈中恢复 R4-R11 和 LR(EXC_RETURN)
        "ldmia r0!, {r4-r11, lr} \n"

        // 更新 PSP
        "msr psp, r0 \n"

        // 异常返回
        "bx lr \n");
}
