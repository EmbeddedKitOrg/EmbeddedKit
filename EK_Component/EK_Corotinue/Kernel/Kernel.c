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

#if (EK_CORO_ENABLE == 1)

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1)
#include "../Message/EK_CoroMessage.h"
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */

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
STATIC_INLINE uint8_t v_kernel_find_msb_index(EK_BitMap_t val)
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
#endif /* EK_KERNEL_CLZ selection */

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
#define EK_CORO_IDLE_TASK_STACK_SIZE (512) // 定义空闲任务的堆栈大小
#endif /* EK_CORO_IDLE_TASK_STACK_SIZE */

/* ========================= 全局变量(内部)定义区 ========================= */
/*状态链表*/
static EK_CoroList_t KernelReadyList[EK_CORO_PRIORITY_GROUPS]; // 就绪任务列表
static EK_CoroList_t KernelBlockList1; // 阻塞任务列表 1
static EK_CoroList_t KernelBlockList2; // 阻塞任务列表 2
static EK_CoroList_t KernelSuspendList; // 挂起任务列表

/* 阻塞链表指针 */
static EK_CoroList_t *KernelCurrentBlockPointer; // 用于指向当前就绪的阻塞链表
static EK_CoroList_t *KernelNextBlockPointer; // 用于指向溢出的就绪的阻塞链表

/*TCB 相关*/
static EK_CoroTCB_t *KernelCurrentTCB; // 当前正在运行的任务TCB指针
static EK_CoroTCB_t *KernelToDeleteTCB; // 等待被删除的任务TCB指针
static EK_CoroTCB_t *KernelNextTCB; // 下一个任务TCB
static EK_CoroTCB_t *KernelIdleTCB; // 空闲任务TCB指针
static EK_CoroStaticHandler_t KernelIdleTCB_Handler; // 空闲任务句柄

/*标志位*/
static bool KernelIdleYield = false; // 调度请求标志位, 由TickHandler在唤醒任务时设置
static bool KernelIsInited = false; // 内核初始化状态标志
static volatile EK_BitMap_t KernelReadyBitMap; // 就绪链表位图
static uint32_t KernelTick; //时基

/*临界区*/
static volatile uint32_t KernelCriticalNesting = 0U; // 临界区嵌套计数（任务上下文）
static uint32_t KernelSavedPrimask = 0U; // 临界区退出时恢复的 PRIMASK（任务上下文）
static volatile uint32_t ISRCriticalNesting = 0U; // ISR临界区嵌套计数（中断上下文）
static uint32_t ISRSavedPrimask = 0U; // ISR上下文退出时恢复的 PRIMASK

/* ========================= 临界区管理实现区 ========================= */

/**
 * @brief 进入内核临界区，禁止中断（ISR自适应版本）
 * @details
 *  此函数实现嵌套临界区保护机制，自动适配ISR和任务上下文：
 *  1. 先检测当前是否在中断中
 *  2. 保存当前中断状态(PRIMASK)
 *  3. 禁止所有中断
 *  4. 根据上下文选择不同的处理策略
 *  5. 增加对应上下文的嵌套计数器
 *  6. 添加数据内存屏障确保操作顺序
 * @note 支持嵌套调用，ISR和任务上下文使用独立的计数器
 */
void EK_vEnterCritical(void)
{
    // 在禁用中断前先检测当前上下文
    bool in_interrupt = EK_IS_IN_INTERRUPT();
    uint32_t primask = __get_PRIMASK(); // 获取当前中断状态
    __disable_irq(); // 禁止所有中断

    if (in_interrupt)
    {
        // ISR上下文：保存进入前状态并增加嵌套计数
        if (ISRCriticalNesting == 0U)
        {
            ISRSavedPrimask = primask; // 保存ISR进入前的中断状态
        }
        ISRCriticalNesting++;
    }
    else
    {
        // 任务上下文：保存状态并增加计数
        if (KernelCriticalNesting == 0U) // 首次进入临界区
        {
            KernelSavedPrimask = primask; // 保存原始中断状态，用于退出时恢复
        }
        KernelCriticalNesting++; // 增加嵌套计数
    }

    __DMB(); // 数据内存屏障，确保操作完成
}

/**
 * @brief 退出内核临界区，恢复中断状态（ISR自适应版本）
 * @details
 *  此函数实现嵌套临界区的安全退出机制，自动适配ISR和任务上下文：
 *  1. 使用保存的原始中断状态判断上下文
 *  2. 检查对应上下文的嵌套计数器，防止过度退出
 *  3. 减少对应上下文的嵌套计数器
 *  4. 任务上下文最外层退出时恢复原始中断状态
 *  5. 添加数据内存屏障确保操作顺序
 * @note ISR和任务上下文使用独立的计数器，只有任务上下文才恢复中断状态
 */
void EK_vExitCritical(void)
{
    if (ISRCriticalNesting > 0)
    {
        // ISR上下文：减少嵌套计数，必要时恢复状态
        ISRCriticalNesting--; // 减少嵌套计数
        if (ISRCriticalNesting == 0U)
        {
            __DMB(); // 数据内存屏障，确保操作完成
            __set_PRIMASK(ISRSavedPrimask); // 恢复ISR进入前的中断状态
        }
    }
    else
    {
        // 任务上下文：减少计数并可能恢复状态
        if (KernelCriticalNesting == 0U) // 检查是否过度退出
        {
            return; // 过度退出，直接返回
        }
        KernelCriticalNesting--; // 减少嵌套计数
        if (KernelCriticalNesting == 0U) // 最外层退出
        {
            __DMB(); // 数据内存屏障，确保操作完成
            __set_PRIMASK(KernelSavedPrimask); // 恢复原始中断状态
        }
    }
}

/* ========================= 栈溢出检测实现区 ========================= */
#if (EK_CORO_STACK_OVERFLOW_CHECK_ENABLE > 0)
/**
 * @brief 弱定义的栈溢出钩子函数
 * @details
 *  当检测到栈溢出时调用此函数。用户可以重写此函数以实现自定义的
 *  错误处理逻辑，如记录错误日志、重启系统等。
 *  默认实现为空函数，用户可根据需要重写。
 * @param overflow_tcb 发生栈溢出的任务控制块
 */
__weak void EK_vStackOverflowHook(EK_CoroTCB_t *overflow_tcb)
{
    /* 用户可以重写此函数来处理栈溢出错误 */
    /* 默认实现为空，防止链接错误 */

    /* 示例处理代码（用户可根据需求实现）：
     * 1. 记录错误信息
     * 2. 挂起或删除溢出任务
     * 3. 系统重启或进入安全模式
     * 4. 触发看门狗复位等
     */

    /* 防止未使用变量警告 */
    UNUSED_VAR(overflow_tcb);

    /* 空实现 - 等待用户重写 */
    while (1)
    {
        /* 用户可选择在此处实现错误处理逻辑 */
        /* 如不重写，系统将继续运行但可能不稳定 */
    }
}

/**
 * @brief 栈溢出检测函数
 * @details
 *  仿照 FreeRTOS 的 taskCHECK_FOR_STACK_OVERFLOW 实现栈溢出检测。
 *  支持两种检测方法：
 *  方法1：检测栈底填充值是否被覆盖（性能较好）
 *  方法2：检测栈指针是否超出栈范围（更全面）
 * @param tcb 要检测的任务控制块
 */
STATIC_INLINE void v_check_stack_overflow(EK_CoroTCB_t *tcb)
{
    if (tcb == NULL || tcb->TCB_StackStart == NULL)
    {
        return;
    }

#if (EK_CORO_STACK_OVERFLOW_CHECK_ENABLE == 1)
    /* 方法1：检测栈底填充值是否被覆盖 */
    uint8_t *stack_bottom = (uint8_t *)tcb->TCB_StackStart;

    // 检查栈底的几个字节是否仍为填充值
    if (stack_bottom[0] != EK_STACK_FILL_PATTERN || stack_bottom[1] != EK_STACK_FILL_PATTERN ||
        stack_bottom[2] != EK_STACK_FILL_PATTERN || stack_bottom[3] != EK_STACK_FILL_PATTERN)
    {
        // 栈溢出，调用钩子函数
        EK_vStackOverflowHook(tcb);
    }

#elif (EK_CORO_STACK_OVERFLOW_CHECK_ENABLE == 2)
    /* 方法2：检测栈指针是否超出栈范围 */
    if (tcb->TCB_StackPointer != NULL)
    {
        uint8_t *stack_ptr = (uint8_t *)tcb->TCB_StackPointer;
        uint8_t *stack_bottom = (uint8_t *)tcb->TCB_StackStart;
#if (EK_HIGH_WATER_MARK_ENABLE == 1)
        uint8_t *stack_top = (uint8_t *)tcb->TCB_StackEnd;
#else
        uint8_t *stack_top = (uint8_t *)((uint8_t *)tcb->TCB_StackStart + tcb->TCB_StackSize);
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */

        // 检查栈指针是否超出栈范围
        if (stack_ptr < stack_bottom || stack_ptr >= stack_top)
        {
            // 栈溢出，调用钩子函数
            EK_vStackOverflowHook(tcb);
        }
    }
#endif /* EK_CORO_STACK_OVERFLOW_CHECK_ENABLE */
}
#endif /* (EK_CORO_STACK_OVERFLOW_CHECK_ENABLE > 0) */

/* ========================= 高水位标记计算区 ========================= */

#if (EK_HIGH_WATER_MARK_ENABLE == 1)
/**
 * @brief 独立的高水位标记计算函数
 * @details
 *  计算栈的高水位标记（历史最大使用量）并保存到 TCB 中。
 *  此函数专门用于高水位计算，不包含栈溢出检测逻辑。
 *
 *  注意：ARM Cortex-M 的栈是向下增长的，从栈底向上查找
 *  第一个被修改的字节，计算栈使用量。
 * @param tcb 要检测的任务控制块
 */
STATIC_INLINE void v_calculate_stack_high_water_mark(EK_CoroTCB_t *tcb)
{
    // 参数有效性检查
    if (tcb == NULL || tcb->TCB_StackStart == NULL || tcb->TCB_StackEnd == NULL)
    {
        return;
    }

    // 获取栈的起始和结束地址
    uint8_t *stack_base = (uint8_t *)tcb->TCB_StackStart; // 栈底（低地址）
    uint8_t *stack_limit = (uint8_t *)tcb->TCB_StackEnd; // 栈顶（高地址）
    EK_Size_t used_bytes = 0;

    // 栈地址有效性检查
    if (stack_limit <= stack_base)
    {
        return;
    }

    // 从栈底向上查找第一个被修改的字节
    uint8_t *check_ptr = stack_base;
    while (check_ptr < stack_limit)
    {
        // 如果当前字节不是填充值，说明这里被使用过
        if (*check_ptr != EK_STACK_FILL_PATTERN)
        {
            // 计算从栈顶到这里的使用量
            used_bytes = (stack_limit - check_ptr);
            break;
        }
        check_ptr++;
    }

    // 如果整个栈都是填充值，使用保守估计（上下文帧大小）
    if (used_bytes == 0)
    {
        used_bytes = sizeof(EK_CoroTCB_t);
    }

    // 更新高水位标记（只有在当前使用量更大时才更新）
    if (used_bytes > tcb->TCB_StackHighWaterMark)
    {
        tcb->TCB_StackHighWaterMark = used_bytes;
    }
}
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */

/* ========================= 空闲任务实现区 ========================= */

static uint8_t Kernel_IdleTaskStack[EK_CORO_IDLE_TASK_STACK_SIZE];
static EK_CoroTCB_t Kernel_IdleTaskTCB;

#if (EK_CORO_IDLE_HOOK_ENABLE == 1)
/**
 * @brief 空闲钩子函数
 * 
 * @return __weak 
 */
__weak void EK_CoroIdleHook(void)
{
}
#endif /* EK_CORO_IDLE_HOOK_ENABLE == 1 */

/**
 * @brief 空闲任务函数 会处理处理任务删除等逻辑
 * @details 如果 EK_CORO_IDLE_HOOK_ENABLE 被设置为1 每次空闲任务还会调用一次 EK_CoroIdleHook
 * 
 * @param arg 
 */
static void Kernel_CoroIdleFunction(void *arg)
{
    UNUSED_VAR(arg);
    while (1)
    {
        if (KernelToDeleteTCB != NULL)
        {
            if (KernelToDeleteTCB->TCB_isDynamic)
            {
                EK_CORO_FREE(KernelToDeleteTCB->TCB_StackStart);
                EK_CORO_FREE(KernelToDeleteTCB);
            }
            KernelToDeleteTCB = NULL;
        }

        if (KernelIdleYield == true)
        {
            KernelIdleYield = false;
            EK_vCoroYield();
        }
#if (EK_CORO_IDLE_HOOK_ENABLE == 1)
        EK_CoroIdleHook();
#endif /* EK_CORO_IDLE_HOOK_ENABLE == 1 */
    }
}

/* ========================= 内核状态访问器实现区 ========================= */

/**
 * @brief 获取系统时钟滴答数
 * @return 当前系统滴答计数
 * @note 此函数用于获取自系统启动以来的滴答数，1ms分辨率
 */
uint32_t EK_uKernelGetTick(void)
{
    return KernelTick;
}

/**
 * @brief 获取指定优先级的就绪链表
 * @param priority 优先级值（数值越小优先级越高）
 * @return 指向就绪链表的指针
 * @note 当优先级超出范围时，自动映射到最低优先级
 */
EK_CoroList_t *EK_pKernelGetReadyList(uint8_t priority)
{
    uint8_t index = priority;
    if (index >= EK_CORO_PRIORITY_GROUPS)
    {
        index = EK_CORO_PRIORITY_GROUPS - 1;
    }
    return &KernelReadyList[index];
}

/**
 * @brief 获取挂起链表
 * @return 指向挂起链表的指针
 * @note 此链表包含所有被挂起的协程任务
 */
EK_CoroList_t *EK_pKernelGetSuspendList(void)
{
    return &KernelSuspendList;
}

/**
 * @brief 获取当前阻塞链表
 * @return 指向当前阻塞链表的指针
 * @note 用于内核内部管理，指向当前正在处理的阻塞事件链表
 */
EK_CoroList_t *EK_pKernelGetCurrentBlockList(void)
{
    return KernelCurrentBlockPointer;
}

/**
 * @brief 获取下一个阻塞链表
 * @return 指向下一个阻塞链表的指针
 * @note 用于内核内部管理，指向下一个待处理的阻塞事件链表
 */
EK_CoroList_t *EK_pKernelGetNextBlockList(void)
{
    return KernelNextBlockPointer;
}

/**
 * @brief 获取当前运行的协程任务控制块
 * @return 指向当前TCB的指针
 * @note 如果没有任务运行则返回NULL
 */
EK_CoroTCB_t *EK_pKernelGetCurrentTCB(void)
{
    return KernelCurrentTCB;
}

/**
 * @brief 获取空闲任务处理器
 * @return 空闲任务处理器
 * @note 当系统没有其他任务可运行时，内核会调用此处理器
 */
EK_CoroStaticHandler_t EK_pKernelGetIdleHandler(void)
{
    return KernelIdleTCB_Handler;
}

/**
 * @brief 获取待删除的任务控制块
 * @return 指向待删除TCB的指针
 * @note 标记需要安全删除的任务
 */
EK_CoroTCB_t *EK_pKernelGetDeleteTCB(void)
{
    return KernelToDeleteTCB;
}

/**
 * @brief 设置待删除的任务控制块
 * @param tcb 指向要删除的任务控制块指针
 * @note 将任务标记为待删除，由内核在适当时机安全删除
 */
void EK_vKernelSetDeleteTCB(EK_CoroTCB_t *tcb)
{
    KernelToDeleteTCB = tcb;
}

/* ========================= 链表函数控制区 ========================= */
/**
 * @brief 从一个链表中移除一个指定的协程节点。
 * @details
 *  此函数会在指定的链表中查找并移除一个节点。它会正确处理头节点、尾节点和中间节点的情况，
 *  并更新链表的计数器和头/尾指针。移除后，节点的归属链表指针会被清空。
 *  如果一个就绪链表因此变为空，对应的就绪位图位也会被清除。
 * @param list 从中移除节点的链表。
 * @param node_to_remove 要移除的协程节点。
 * @return EK_Result_t 操作结果 (EK_OK, EK_NULL_POINTER, EK_NOT_FOUND)。
 */
EK_Result_t EK_rKernelRemove(EK_CoroList_t *list, EK_CoroListNode_t *node_to_remove)
{
    // 入参检查
    if (list == NULL || node_to_remove == NULL) return EK_NULL_POINTER;
    if (list->List_Count == 0) return EK_ERROR;
    if (node_to_remove->CoroNode_List != list) return EK_ERROR;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node_to_remove->CoroNode_Owner; // 获取TCB

    EK_ENTER_CRITICAL();

    // 利用双向链表特性 (O(1) 删除)
    if (node_to_remove->CoroNode_Prev != NULL)
    {
        node_to_remove->CoroNode_Prev->CoroNode_Next = node_to_remove->CoroNode_Next;
    }
    else // 移除的是头节点
    {
        list->List_Head = node_to_remove->CoroNode_Next;
    }

    if (node_to_remove->CoroNode_Next != NULL)
    {
        node_to_remove->CoroNode_Next->CoroNode_Prev = node_to_remove->CoroNode_Prev;
    }
    else // 移除的是尾节点
    {
        list->List_Tail = node_to_remove->CoroNode_Prev;
    }

    // 更新链表信息
    list->List_Count--;

    // 更新节点信息
    node_to_remove->CoroNode_List = NULL;
    node_to_remove->CoroNode_Next = NULL;
    node_to_remove->CoroNode_Prev = NULL;

    // 如果就绪链表变为空，则清除对应的位图位
    if (list->List_Count == 0 && list >= KernelReadyList && list < KernelReadyList + EK_CORO_PRIORITY_GROUPS)
    {
        EK_vClearBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 将一个协程节点按唤醒时间升序插入到链表中。
 * @details
 *  这是一个公共接口，用于将节点按照其 `TCB_WakeUpTime` 成员的值从小到大插入到链表中。
 *  主要用于将任务插入到阻塞链表 `EK_CoroKernelBlockList`。
 * @param list 目标链表。
 * @param node 要插入的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelInsert_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_Owner;
    node->CoroNode_Next = NULL;
    node->CoroNode_Prev = NULL;

    EK_ENTER_CRITICAL();

    if (list->List_Head == NULL) // 如果链表为空
    {
        list->List_Head = node;
        list->List_Tail = node;
    }
    else // 按照时间顺序插入链表
    {
        EK_CoroListNode_t *current = list->List_Head;
        while (current != NULL)
        {
            EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current->CoroNode_Owner;
            if (tcb->TCB_WakeUpTime < current_tcb->TCB_WakeUpTime) break;
            current = current->CoroNode_Next;
        }

        if (current != NULL) // 插入到current节点之前
        {
            node->CoroNode_Next = current;
            node->CoroNode_Prev = current->CoroNode_Prev;
            if (current->CoroNode_Prev != NULL)
            {
                current->CoroNode_Prev->CoroNode_Next = node;
            }
            else // 插入到头部
            {
                list->List_Head = node;
            }
            current->CoroNode_Prev = node;
        }
        else // 插入到尾部
        {
            node->CoroNode_Prev = list->List_Tail;
            list->List_Tail->CoroNode_Next = node;
            list->List_Tail = node;
        }
    }

    node->CoroNode_List = list;
    list->List_Count++;

    // 判断是不是就绪链表
    if (list >= KernelReadyList && list < KernelReadyList + EK_CORO_PRIORITY_GROUPS)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();
    return EK_OK;
}

/**
 * @brief 将一个协程节点插入到链表的尾部。
 * @details
 *  这是一个公共接口，以 O(1) 的时间复杂度将节点添加到链表的末尾。
 *  主要用于将任务添加到就绪链表 `KernelReadyList` 或挂起链表 `KernelSuspendList`。
 * @param list 目标链表。
 * @param node 要插入的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelInsert_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_Owner;
    node->CoroNode_Next = NULL;
    node->CoroNode_Prev = NULL;

    EK_ENTER_CRITICAL();

    if (list->List_Head == NULL) // 如果链表为空
    {
        list->List_Head = node;
        list->List_Tail = node;
    }
    else // 尾部插入 (O(1) 操作)
    {
        node->CoroNode_Prev = list->List_Tail;
        list->List_Tail->CoroNode_Next = node;
        list->List_Tail = node;
    }

    node->CoroNode_List = list;
    list->List_Count++;

    // 判断是不是就绪链表
    if (list >= KernelReadyList && list < KernelReadyList + EK_CORO_PRIORITY_GROUPS)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();
    return EK_OK;
}

/**
 * @brief 将一个协程节点按优先级升序插入到链表中。
 * @details
 *  这是一个公共接口，用于将节点按照其 `TCB_Priority` 成员的值从小到大插入到链表中 (值越小优先级越高)。
 * @param list 目标链表。
 * @param node 要插入的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelInsert_Prio(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_Owner;
    node->CoroNode_Next = NULL;
    node->CoroNode_Prev = NULL;

    EK_ENTER_CRITICAL();

    if (list->List_Head == NULL) // 如果链表为空
    {
        list->List_Head = node;
        list->List_Tail = node;
    }
    else // 按照优先级顺序插入链表
    {
        EK_CoroListNode_t *current = list->List_Head;
        while (current != NULL)
        {
            EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current->CoroNode_Owner;
            if (tcb->TCB_Priority < current_tcb->TCB_Priority) break; // 找到了插入点 (新任务优先级更高)
            current = current->CoroNode_Next;
        }

        if (current != NULL) // 插入到current节点之前
        {
            node->CoroNode_Next = current;
            node->CoroNode_Prev = current->CoroNode_Prev;
            if (current->CoroNode_Prev != NULL)
            {
                current->CoroNode_Prev->CoroNode_Next = node;
            }
            else // 插入到头部
            {
                list->List_Head = node;
            }
            current->CoroNode_Prev = node;
        }
        else // 插入到尾部
        {
            node->CoroNode_Prev = list->List_Tail;
            list->List_Tail->CoroNode_Next = node;
            list->List_Tail = node;
        }
    }

    node->CoroNode_List = list;
    list->List_Count++;

    // 判断是不是就绪链表
    if (list >= KernelReadyList && list < KernelReadyList + EK_CORO_PRIORITY_GROUPS)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();
    return EK_OK;
}

/**
 * @brief 将一个协程节点移动到另一个链表，并按唤醒时间升序排序。
 * @details
 *  此函数将节点从当前链表移除，然后按唤醒时间 `TCB_WakeUpTime` 升序插入到目标链表中。
 *  常用于将任务从就绪链表移动到阻塞链表。
 * @param list 目标链表。
 * @param node 要移动的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelMove_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    if (node->CoroNode_List != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)node->CoroNode_List, node);
        if (remove_res != EK_OK) return remove_res;
    }

    return EK_rKernelInsert_WakeUpTime(list, node);
}

/**
 * @brief 将一个协程节点移动到另一个链表的尾部。
 * @details
 *  此函数将节点从当前链表移除，然后插入到目标链表的末尾。
 *  常用于将任务从阻塞/挂起链表移回到就绪链表。
 * @param list 目标链表。
 * @param node 要移动的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelMove_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    if (node->CoroNode_List != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)node->CoroNode_List, node);
        if (remove_res != EK_OK) return remove_res;
    }

    return EK_rKernelInsert_Tail(list, node);
}

/**
 * @brief 将一个协程节点移动到另一个链表，并按优先级升序排序。
 * @details
 *  此函数将节点从当前链表移除，然后按任务优先级 `TCB_Priority` 升序插入到目标链表中。
 * @param list 目标链表。
 * @param node 要移动的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelMove_Prio(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    if (node->CoroNode_List != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)node->CoroNode_List, node);
        if (remove_res != EK_OK) return remove_res;
    }

    return EK_rKernelInsert_Prio(list, node);
}

/* ========================= 内核核心API函数 ========================= */

__naked static void v_kernel_start(void)
{
    __ASM volatile(
        // 加载第一个任务的堆栈指针到 PSP
        "ldr r0, =KernelCurrentTCB \n"
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
 * @brief 初始化协程内核。
 * @details
 *  此函数必须在创建任何协程或启动内核之前调用一次。
 *  它负责初始化内存池、所有任务列表（就绪、阻塞、挂起等）、
 *  内核全局变量，并创建系统空闲任务。
 */
void EK_vKernelInit(void)
{
    if (KernelIsInited == true) return;

    // 关闭SysTick中断，确保初始化期间不被时钟中断打扰
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;

    // 初始化内存池
    while (EK_bMemPool_Init() != true);

    // 初始化就绪链表
    for (uint8_t i = 0; i < EK_CORO_PRIORITY_GROUPS; i++)
    {
        KernelReadyList[i].List_Count = 0;
        KernelReadyList[i].List_Head = NULL;
        KernelReadyList[i].List_Tail = NULL;
    }

    // 初始化阻塞链表1
    KernelBlockList1.List_Count = 0;
    KernelBlockList1.List_Head = NULL;
    KernelBlockList1.List_Tail = NULL;

    // 初始化阻塞链表2
    KernelBlockList2.List_Count = 0;
    KernelBlockList2.List_Head = NULL;
    KernelBlockList2.List_Tail = NULL;

    // 初始化挂起链表
    KernelSuspendList.List_Count = 0;
    KernelSuspendList.List_Head = NULL;
    KernelSuspendList.List_Tail = NULL;

    // 初始化位图
    KernelReadyBitMap = 0;

    // 初始化Tick
    KernelTick = 0;

    // 初始化指针
    KernelCurrentTCB = NULL;
    KernelToDeleteTCB = NULL;
    KernelNextTCB = NULL;

    // 初始化当前就绪链表的指针
    KernelCurrentBlockPointer = &KernelBlockList1;
    KernelNextBlockPointer = &KernelBlockList2;

    // 创建空闲任务
    KernelIdleTCB_Handler = EK_pCoroCreateStatic(&Kernel_IdleTaskTCB,
                                                 Kernel_CoroIdleFunction,
                                                 NULL,
                                                 EK_CORO_PRIORITY_MOUNT - 1, // 最低优先级
                                                 Kernel_IdleTaskStack,
                                                 EK_CORO_IDLE_TASK_STACK_SIZE);

    // 设置SysTick中断优先级为最低
    NVIC_SetPriority(SysTick_IRQn, 0xFF);

    // 设置PendSV中断优先级为最低
    NVIC_SetPriority(PendSV_IRQn, 0xFF);

    // 配置SysTick
    while (SysTick_Config(EK_CORO_SYSTEM_FREQ / EK_CORO_TICK_RATE_HZ));

    // 开启SysTick中断
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;

    KernelIsInited = true;
}

/**
 * @brief 启动协程调度器。
 * @details
 *  此函数启动整个协程系统。它会选择就绪列表中优先级最高的任务作为第一个任务来运行。
 *  在启动之前，它会检查内核是否已初始化，并保存用户提供的获取系统Tick的函数。
 *  一旦调用，它将通过 `v_kernel_start` 执行第一次上下文切换，并且不会返回。
 * @note 此函数永不返回。
 */
void EK_vKernelStart(void)
{
    if (KernelIsInited == false) EK_vKernelInit();

    EK_ENTER_CRITICAL();

    if (KernelReadyBitMap == 0)
    {
        return;
    }

    // 找到优先级最高的TCB 启动
    uint8_t highest_prio = EK_KERNEL_GET_HIGHEST_PRIO(KernelReadyBitMap);
    KernelCurrentTCB = (EK_CoroTCB_t *)KernelReadyList[highest_prio].List_Head->CoroNode_Owner;
    KernelCurrentTCB->TCB_State = EK_CORO_RUNNING;
    EK_rKernelRemove(&KernelReadyList[highest_prio], &KernelCurrentTCB->TCB_StateNode);

    EK_EXIT_CRITICAL();

    // 手动触发一次调度来启动第一个任务
    v_kernel_start();

    // 此函数不应返回
    while (1);
}

/**
 * @brief 请求一次任务调度。
 * @details
 *  此函数用于触发一次任务调度。它会从就绪位图中找到当前最高优先级的就绪任务，
 *  将其设置为下一个要运行的任务 (`KernelNextTCB`)，然后通过触发 PendSV 异常来进行上下文切换。
 *  当前正在运行的任务在调用此函数之前，应该已经被放回了某个链表（如就绪链表或阻塞链表）。
 *
 *  在任务切换之前，会执行栈溢出检测以确保系统稳定性。
 */
void EK_vKernelYield(void)
{
    EK_ENTER_CRITICAL();

    // 找到最高优先级
    uint8_t highest_prio = EK_KERNEL_GET_HIGHEST_PRIO(KernelReadyBitMap);

    // 更新链表
    KernelNextTCB = (EK_CoroTCB_t *)KernelReadyList[highest_prio].List_Head->CoroNode_Owner;
    KernelNextTCB->TCB_State = EK_CORO_RUNNING;

    // 将TCB移除
    EK_rKernelRemove(&KernelReadyList[highest_prio], &KernelNextTCB->TCB_StateNode);

    // 在任务切换前检查下一个任务的栈溢出情况并计算高水位标记
    // 注意：这里检查的是即将运行的任务，确保它在运行前栈是安全的
    // 执行栈溢出检测（如果启用）
#if (EK_CORO_STACK_OVERFLOW_CHECK_ENABLE > 0)
    v_check_stack_overflow(KernelNextTCB);
#endif /* EK_CORO_STACK_OVERFLOW_CHECK_ENABLE > 0 */
    // 执行高水位标记计算（如果启用）
#if (EK_HIGH_WATER_MARK_ENABLE == 1)
    v_calculate_stack_high_water_mark(KernelNextTCB);
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */

    EK_EXIT_CRITICAL();

    // 申请一次调度
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}

/**
 * @brief 内核时钟节拍处理函数。
 * @details
 *  此函数应在系统的时钟节拍中断（如 SysTick_Handler）中被周期性调用。
 *  它负责处理所有带超时的阻塞。它会遍历延时阻塞列表
 *  以及所有等待消息队列的阻塞列表，检查是否有任务的延时已经到期。
 *  如果一个任务的 `TCB_WakeUpTime` 小于或等于当前Tick，该任务将被唤醒并移至就绪链表。
 *  此函数能够正确处理系统Tick值的溢出情况。
 */
void EK_vTickHandler(void)
{
    static uint32_t last_tick = 0;
    EK_ENTER_CRITICAL();

    KernelTick++;

    // 处理因 EK_vCoroDelay() 而阻塞的任务
    if (KernelCurrentBlockPointer->List_Count > 0)
    {
        // 获取当前状态节点
        EK_CoroListNode_t *current_state_node = KernelCurrentBlockPointer->List_Head;
        while (current_state_node != NULL)
        {
            EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current_state_node->CoroNode_Owner;

            // 预先保存下一个节点，防止当前节点被移动后链表断裂
            EK_CoroListNode_t *next_node = current_state_node->CoroNode_Next;

            // 永久阻塞的任务需要被显式唤醒，Tick处理器直接跳过
            if (current_tcb->TCB_WakeUpTime == EK_MAX_DELAY)
            {
                current_state_node = next_node;
                continue;
            }

            // 当前任务未达到唤醒时间
            if (KernelTick < current_tcb->TCB_WakeUpTime)
            {
                // 由于阻塞链表是按唤醒时间升序排列的，如果当前任务还没到期，后续任务也一定没到期
                break;
            }
#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1)
            // 如果任务正在等待某个事件 (即其事件节点在某个等待列表中)
            if (current_tcb->TCB_EventResult == EK_CORO_EVENT_PENDING)
            {
                // 那么延时到期意味着事件超时
                current_tcb->TCB_EventResult = EK_CORO_EVENT_TIMEOUT;
            }
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1 */

            // 任务延时已到，将其唤醒
            current_tcb->TCB_State = EK_CORO_READY;
            // 更新任务的最后唤醒时间
            current_tcb->TCB_LastWakeUpTime = KernelTick;
            EK_rKernelMove_Tail(&KernelReadyList[current_tcb->TCB_Priority], &current_tcb->TCB_StateNode);

            // 如果当前CPU正处于空闲状态，并且我们唤醒了一个任务，那么就需要请求一次调度
            if (KernelCurrentTCB == KernelIdleTCB_Handler)
            {
                KernelIdleYield = true;
            }
            current_state_node = next_node;
        }
    }

    // Tick值溢出 要交换链表了
    if (last_tick > KernelTick)
    {
        EK_CoroList_t *temp = KernelCurrentBlockPointer; // 存储当前的链表
        KernelCurrentBlockPointer = KernelNextBlockPointer; // 交换链表到下一个tick周期的链表
        KernelNextBlockPointer = temp; // 获得新的链表
    }

    last_tick = KernelTick; // 上一次的时间

    EK_EXIT_CRITICAL();
}

/**
 * @brief Coroutine内核的PendSV处理函数 (上下文切换核心)。
 * @details
 *  这是一个裸函数，实现了实际的上下文切换逻辑。它必须由用户在实际的 `PendSV_Handler` 中调用。
 *  其主要职责是：
 *  1. 保存当前任务的CPU寄存器（R4-R11, LR）到其堆栈。
 *  2. 将更新后的堆栈指针保存到当前任务的TCB中。
 *  3. 将 `KernelNextTCB` 设置为 `KernelCurrentTCB`。
 *  4. 从新任务的TCB中加载其堆栈指针。
 *  5. 从新任务的堆栈中恢复其CPU寄存器。
 *  6. 异常返回，CPU开始执行新任务的代码。
 */
__naked void EK_vKernelPendSV_Handler(void)
{
    __ASM volatile(
        // 保存当前任务的上下文
        "mrs r0, psp \n"

#if (EK_CORO_FPU_ENABLE == 1)
        // 检查是否需要保存FPU状态
        "ldr r2, =0xE000EF34 \n" // FPCCR寄存器
        "ldr r3, [r2] \n"
        "tst r3, #0x01 \n" // 检查LSPACT位
        "beq no_fpu_save \n"

        // 保存FPU寄存器S16-S31
        "vstmdb r0!, {s16-s31} \n"
        "vmrs r2, fpscr \n"
        "stmdb r0!, {r2} \n"

        "no_fpu_save: \n"
#endif /* EK_CORO_FPU_ENABLE == 1 */

        // 保存通用寄存器
        "stmdb r0!, {r4-r11, lr} \n"

        // 保存栈指针到TCB
        "ldr r1, =KernelCurrentTCB \n"
        "ldr r1, [r1] \n"
        "str r0, [r1] \n"

        // 任务切换
        "ldr r0, =KernelNextTCB \n"
        "ldr r0, [r0] \n"
        "ldr r1, =KernelCurrentTCB \n"
        "str r0, [r1] \n"

        // 恢复新任务的栈指针
        "ldr r1, =KernelCurrentTCB \n"
        "ldr r1, [r1] \n"
        "ldr r0, [r1] \n"

        // 恢复通用寄存器
        "ldmia r0!, {r4-r11, lr} \n"

#if (EK_CORO_FPU_ENABLE == 1)
        // 检查是否需要恢复FPU状态
        "tst lr, #0x10 \n" // 检查EXC_RETURN的bit 4
        "bne no_fpu_restore \n"

        // 恢复FPU状态
        "ldmia r0!, {r2} \n"
        "vmsr fpscr, r2 \n"
        "vldmia r0!, {s16-s31} \n"

        "no_fpu_restore: \n"
#endif /* EK_CORO_FPU_ENABLE == 1 */

        "msr psp, r0 \n"
        "bx lr \n");
}

#endif /*EK_CORO_ENABLE == 1*/