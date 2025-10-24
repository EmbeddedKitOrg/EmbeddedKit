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
#include "./Inc/Kernel.h"
#include "../MemPool/Inc/EK_MemPool.h"
#include "EK_CoroTask.h"

#if (EK_CORO_ENABLE == 1)

/* ========================= 全局变量(内部)定义区 ========================= */
/*状态链表*/
EK_CoroList_t KernelReadyList[EK_CORO_PRIORITY_GROUPS]; // 就绪任务列表
EK_CoroList_t KernelBlockList1; // 阻塞任务列表 1
EK_CoroList_t KernelBlockList2; // 阻塞任务列表 2
EK_CoroList_t KernelSuspendList; // 挂起任务列表

/* 阻塞链表指针 */
static EK_CoroList_t *KernelCurrentBlockPointer; // 用于指向当前就绪的阻塞链表
static EK_CoroList_t *KernelNextBlockPointer; // 用于指向溢出的就绪的阻塞链表

/*TCB 相关*/
EK_CoroTCB_t *KernelCurrentTCB; // 当前正在运行的任务TCB指针
static EK_CoroTCB_t *KernelToDeleteTCB; // 等待被删除的任务TCB指针
static EK_CoroStaticHandler_t KernelIdleTCB_Handler; // 空闲任务句柄

/*标志位*/
static bool KernelIdleYield = false; // 调度请求标志位, 由TickHandler在唤醒任务时设置
static bool KernelIsInited = false; // 内核初始化状态标志
EK_BitMap_t KernelReadyBitMap; // 就绪链表位图
static volatile uint32_t KernelTick; //时基

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
 *  检测方法：
 *  检测栈指针是否超出栈范围（更全面）
 * @param tcb 要检测的任务控制块
 */
STATIC_INLINE void v_check_stack_overflow(EK_CoroTCB_t *tcb)
{
    if (tcb == NULL || tcb->TCB_StackStart == NULL)
    {
        return;
    }

#if (EK_CORO_STACK_OVERFLOW_CHECK_ENABLE == 1)
    /* 检测栈指针是否超出栈范围 */
    if (tcb->TCB_StackPointer != NULL)
    {
        uint8_t *stack_ptr = (uint8_t *)tcb->TCB_StackPointer;
        uint8_t *stack_bottom = (uint8_t *)tcb->TCB_StackStart;
#if (EK_HIGH_WATER_MARK_ENABLE == 1)
        uint8_t *stack_top = (uint8_t *)tcb->TCB_StackEnd;
#else
        uint8_t *stack_top = (uint8_t *)((uintptr_t)((uint8_t *)tcb->TCB_StackStart) + tcb->TCB_StackSize);
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

    EK_Size_t high_water_mark = tcb->TCB_StackSize - used_bytes; // 高水位值

    // 更新高水位标记（只有在当前使用量更大时才更新）
    if (high_water_mark < tcb->TCB_StackHighWaterMark || tcb->TCB_StackHighWaterMark == 0)
    {
        tcb->TCB_StackHighWaterMark = high_water_mark;
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
        EK_ENTER_CRITICAL();

#if (EK_CORO_IDLE_HOOK_ENABLE == 1)
        EK_CoroIdleHook();
#endif /* EK_CORO_IDLE_HOOK_ENABLE == 1 */

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
            EK_EXIT_CRITICAL();
            EK_vCoroYield();
        }

        EK_EXIT_CRITICAL();
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

/* ========================= 内核核心API函数 ========================= */

/**
 * @brief 内核调度逻辑函数（在PendSV中断中执行）。
 * @details
 *  此函数负责执行实际的调度逻辑，包括：
 *  1. 从就绪位图中找到最高优先级的就绪任务
 *  2. 设置下一个要运行的任务
 *  3. 从就绪链表中移除该任务
 *  4. 执行栈溢出检测和高水位标记计算
 *
 *  此函数设计为在PendSV中断上下文中调用，确保调度逻辑在中断环境中安全执行。
 *  调用者必须确保在中断上下文中且已进入临界区。
 */
void v_kernel_task_switch(void)
{
    // 检查是否有调度请求且就绪位图不为空

    // 找到最高优先级
    uint8_t highest_prio = EK_KERNEL_GET_HIGHEST_PRIO(KernelReadyBitMap);

    // 更新链表
    KernelCurrentTCB = (EK_CoroTCB_t *)EK_pKernelListGetFirst(&KernelReadyList[highest_prio])->CoroNode_Owner;
    KernelCurrentTCB->TCB_State = EK_CORO_RUNNING;

    // 在任务切换前检查下一个任务的栈溢出情况并计算高水位标记
    // 注意：这里检查的是即将运行的任务，确保它在运行前栈是安全的
    // 执行栈溢出检测（如果启用）
#if (EK_CORO_STACK_OVERFLOW_CHECK_ENABLE > 0)
    v_check_stack_overflow(KernelCurrentTCB);
#endif /* EK_CORO_STACK_OVERFLOW_CHECK_ENABLE > 0 */
    // 执行高水位标记计算（如果启用）
#if (EK_HIGH_WATER_MARK_ENABLE == 1)
    v_calculate_stack_high_water_mark(KernelCurrentTCB);
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */
}

/**
 * @brief 启动第一个任务的裸函数
 * @details
 *  这是一个裸函数，负责启动第一个任务。参考FreeRTOS的实现方式，
 *  使用SVC异常来启动第一个任务，确保正确的上下文切换和异常处理。
 *
 *  操作步骤：
 *  1. 重置MSP向量表指向栈顶
 *  2. 清除FPU使用标志
 *  3. 全局启用中断
 *  4. 触发SVC异常启动第一个任务
 *
 * @note 此函数只在内核启动时调用一次，调用后不再返回
 */
#if defined(__CC_ARM)
/* AC5编译器版本 - 使用__asm语法 */
// clang-format off
__asm static void v_kernel_start_first(void)
{
    ldr r0, = 0xE000ED08     /* 使用NVIC偏移寄存器定位栈地址 */
    ldr r0, [r0]
    ldr r0, [r0]
    msr msp, r0              /* 将MSP设置为栈的起始位置 */
    mov r0, #0                /* 清除FPU使用标志位 */
    msr control, r0
    cpsie i                   /* 全局启用中断 */
    cpsie f
    dsb
    isb
    svc 0                     /* 系统调用，启动第一个任务 */
    nop
    ltorg
}
// clang-format on
#else
/* 其他编译器版本 - 使用__naked属性 */
__naked static void v_kernel_start_first(void)
{
    __ASM volatile("ldr r0, =0xE000ED08  \n" /* 使用NVIC偏移寄存器定位栈地址 */
                   "ldr r0, [r0]            \n"
                   "ldr r0, [r0]            \n"
                   "msr msp, r0             \n" /* 将MSP设置为栈的起始位置 */
                   "mov r0, #0              \n" /* 清除FPU使用标志位 */
                   "msr control, r0         \n"
                   "cpsie i                 \n" /* 全局启用中断 */
                   "cpsie f                 \n"
                   "dsb                     \n"
                   "isb                     \n"
                   "svc 0                   \n" /* 系统调用，启动第一个任务 */
                   "nop                     \n"
                   ".ltorg                  \n");
}
#endif /* defined(__CC_ARM) */

#if (EK_CORO_FPU_ENABLE == 1)
/**
 * @brief 启用浮点处理单元(FPU)
 * @details
 *  这是一个裸函数，负责启用Cortex-M4F的FPU功能。
 *  其主要职责是配置CPACR(协处理器访问控制寄存器)，允许CPU访问FPU指令。
 *
 *  CPACR寄存器bits 20-23控制FPU访问权限：
 *  - bits 20-21: CP11 (FPU高寄存器) 访问权限
 *  - bits 22-23: CP10 (FPU低寄存器) 访问权限
 *  - 11b: 完全访问权限 (特权和非特权模式都可以使用FPU)
 *
 * @note 此函数必须在内核初始化时调用，确保任务可以使用FPU指令
 * @note 直接使用寄存器地址和位操作值，避免内联汇编中的宏解析问题
 */
#if defined(__CC_ARM)
/* AC5编译器版本 - 使用__asm语法 */
// clang-format off
__asm static void v_kernel_enbale_vfp(void)
{
    ldr.w r0, = 0xE000ED88  /* 加载CPACR寄存器地址 */
    ldr r1, [r0]            /* 读取当前CPACR值 */
    orr r1, r1, #(0xf << 20) /* 设置bits 20-23 = 1111，启用CP10和CP11完全访问 */
    str r1, [r0]            /* 写回CPACR寄存器 */
    bx r14                   /* 返回调用者 */
    ltorg                   /* 字符串池，用于地址常量 */
}
// clang-format on
#else
/* 其他编译器版本 - 使用__naked属性 */
__naked static void v_kernel_enbale_vfp(void)
{
    __ASM volatile("   ldr.w r0, =0xE000ED88   \n" /* 加载CPACR寄存器地址 */
                   "   ldr r1, [r0]            \n" /* 读取当前CPACR值 */
                   "                               \n"
                   "   orr r1, r1, #( 0xf << 20 ) \n" /* 设置bits 20-23 = 1111，启用CP10和CP11完全访问 */
                   "   str r1, [r0]            \n" /* 写回CPACR寄存器 */
                   "   bx r14                  \n" /* 返回调用者 */
                   "   .ltorg                  \n"); /* 字符串池，用于地址常量 */
}
#endif /* defined(__CC_ARM) */
#endif /* EK_CORO_FPU_ENABLE == 1 */

/**
 * @brief 获取内存池的剩余内存
 * 
 * @return EK_Size_t 
 */
EK_Size_t EK_uKernelGetFreeHeap(void)
{
    return EK_uMemPool_GetFreeSize();
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

    // 设置ASPEN和LSPEN位，使能懒惰堆栈
#if (EK_CORO_FPU_ENABLE == 1)
    v_kernel_enbale_vfp();

    *FPCCR |= ASPEN_LSPEN_BITS;
#endif /* EK_CORO_FPU_ENABLE == 1 */

    // 初始化就绪链表
    for (uint8_t i = 0; i < EK_CORO_PRIORITY_GROUPS; i++)
    {
        EK_vKernelListInit(&KernelReadyList[i]);
    }

    // 初始化阻塞链表1
    EK_vKernelListInit(&KernelBlockList1);

    // 初始化阻塞链表2
    EK_vKernelListInit(&KernelBlockList2);

    // 初始化挂起链表
    EK_vKernelListInit(&KernelSuspendList);

    // 初始化位图
    KernelReadyBitMap = 0;

    // 初始化Tick
    KernelTick = 0;

    // 初始化指针
    KernelCurrentTCB = NULL;
    KernelToDeleteTCB = NULL;

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

    // SVC中断优先级为最高
    NVIC_SetPriority(SVCall_IRQn, 0x00);

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
 *  一旦调用，它将通过 `v_kernel_start_first` 执行第一次上下文切换，并且不会返回。
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
    v_kernel_task_switch();

    EK_EXIT_CRITICAL();

    // 通过SVC启动第一个任务
    v_kernel_start_first();

    // 此函数不应返回
    while (1);
}

/**
 * @brief SVC异常处理函数 - 启动第一个任务
 * @details
 *  这是一个裸函数，用于启动第一个任务。它在SVC异常被触发时调用，
 *  负责从第一个任务的堆栈中恢复CPU寄存器并开始执行任务。
 *  此函数只会在系统启动时调用一次。
 *
 *  操作步骤：
 *  1. 获取当前TCB的任务栈顶指针
 *  2. 恢复R4-R11和LR寄存器
 *  3. 设置PSP为新的栈顶
 *  4. 清除basepri优先级屏蔽
 *  5. 异常返回到任务入口点
 *
 * @note 此函数为裸函数，直接处理SVC异常，不需要在其他地方调用
 */
#if defined(__CC_ARM)
/* AC5编译器版本 - 使用__asm语法 */
// clang-format off
__asm void SVC_Handler(void)
{
	extern KernelCurrentTCB;
	
    ldr r3, = KernelCurrentTCB /* 获取当前TCB的位置 */
    ldr r1, [r3]               /* 获取当前TCB */
    ldr r0, [r1]               /* 获取任务栈顶指针 */
    ldmia r0!, {r4 - r11, r14} /* 恢复核心寄存器和链接寄存器 */
    msr psp, r0                /* 设置PSP为新的栈顶 */
    isb                        /* 指令同步屏障 */
    mov r0, #0                 /* 清除basepri优先级屏蔽 */
    msr basepri, r0            /* 允许所有中断 */
    bx r14                     /* 异常返回到任务 */
    align 4
}
// clang-format on
#else
/* 其他编译器版本 - 使用__naked属性 */
__naked void SVC_Handler(void)
{
    __ASM volatile("ldr r3, =KernelCurrentTCB           \n" /* 获取当前TCB的位置 */
                   "ldr r1, [r3]                        \n" /* 获取当前TCB */
                   "ldr r0, [r1]                        \n" /* 获取任务栈顶指针 */
                   "ldmia r0!, {r4-r11, r14}            \n" /* 恢复核心寄存器和链接寄存器 */
                   "msr psp, r0                         \n" /* 设置PSP为新的栈顶 */
                   "isb                                 \n" /* 指令同步屏障 */
                   "mov r0, #0                          \n" /* 清除basepri优先级屏蔽 */
                   "msr basepri, r0                     \n" /* 允许所有中断 */
                   "bx r14                              \n" /* 异常返回到任务 */
                   "                                    \n"
                   ".align 4                            \n");
}
#endif /* defined(__CC_ARM) */

/**
 * @brief 内核时钟节拍处理函数
 * @details
 *  这是EK_Corotinue内核的系统时钟节拍处理函数，用于处理任务延时和超时。
 *  此函数直接由硬件SysTick中断调用，不需要在其他地方调用。
 *
 *  主要功能：
 *  1. 递增系统Tick计数器
 *  2. 遍历延时阻塞列表，检查是否有任务延时到期
 *  3. 将到期任务从阻塞状态移至就绪状态
 *  4. 处理等待事件的任务超时情况
 *  5. 更新就绪任务位图，触发PendSV进行调度
 *
 *  超时处理：
 *  - 延时到期的任务会被自动唤醒
 *  - 等待事件的任务超时后设置TCB_EventResult为EK_CORO_EVENT_TIMEOUT
 *  - 永久阻塞任务(EK_MAX_DELAY)不会被自动唤醒
 *
 * @note 此函数为裸函数，直接处理SysTick中断，调用EK_DISABLE_HAL_HANDLER()宏
 */
void SysTick_Handler(void)
{
    static uint32_t last_tick = 0;
    EK_ENTER_CRITICAL();

    KernelTick++;

    // 处理因 EK_vCoroDelay() 而阻塞的任务
    if (KernelCurrentBlockPointer->List_Count > 0)
    {
        // 获取第一个节点和哨兵节点
        EK_CoroListNode_t *dummy = EK_pKernelListGetDummy(KernelCurrentBlockPointer);
        EK_CoroListNode_t *current_state_node = EK_pKernelListGetFirst(KernelCurrentBlockPointer);

        while (current_state_node != dummy)
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
 * @brief PendSV异常处理函数 - 上下文切换核心
 * @details
 *  这是EK_Corotinue内核的上下文切换核心实现，是一个裸函数，直接处理PendSV异常。
 *  此函数负责在不同任务之间进行上下文切换，是整个协程系统的核心。
 *
 *  操作流程：
 *  1. 获取当前PSP，指向当前任务的栈顶
 *  2. 保存当前任务的CPU寄存器到堆栈
 *  3. 将新栈顶保存到当前任务的TCB
 *  4. 调用调度函数确定下一个要运行的任务
 *  5. 从新任务的TCB获取栈顶指针
 *  6. 恢复新任务的CPU寄存器
 *  7. 异常返回到新任务
 *
 *  FPU支持：
 *  - 有FPU版本：自动检测并保存/恢复FPU寄存器(s16-s31)
 *  - 无FPU版本：只处理核心寄存器，性能更优
 *
 * @note 此函数为裸函数，通过EK_DISABLE_HAL_HANDLER()宏弱定义HAL库的Handler
 */
#if (EK_CORO_FPU_ENABLE == 1)
/**
 * @brief PendSV异常处理函数 - 支持FPU版本
 * @details
 *  此版本支持浮点处理单元，会自动检测任务是否使用了FPU，
 *  并在需要时保存和恢复FPU寄存器(s16-s31)。
 */
#if defined(__CC_ARM)
/* AC5编译器版本 - 使用__asm语法 */
// clang-format off
__asm void PendSV_Handler(void)
{
	extern KernelCurrentTCB;
    extern v_kernel_task_switch;

    mrs r0, psp                /* 获取当前PSP */
    isb                        /* 指令同步屏障 */

    ldr r3, = KernelCurrentTCB /* 获取当前TCB指针 */
    ldr r2, [r3]               /* 获取当前TCB */

    tst r14, #0x10             /* 检查FPU使用标志 */
    it eq                       /* 如果为0则执行 */
    vstmdbeq r0!, {s16 - s31}  /* 保存FPU寄存器 */

    stmdb r0!, {r4 - r11, r14} /* 保存核心寄存器 */
    str r0, [r2]               /* 保存新栈顶到TCB */

    stmdb sp!, {r0, r3}        /* 保存寄存器到MSP栈 */
    bl v_kernel_task_switch     /* 调用任务切换 */
    ldmia sp!, {r0, r3}        /* 恢复寄存器 */

    ldr r1, [r3]               /* 获取新TCB */
    ldr r0, [r1]               /* 获取新栈顶 */

    ldmia r0!, {r4 - r11, r14} /* 恢复核心寄存器 */

    tst r14, #0x10             /* 检查FPU使用标志 */
    it eq                       /* 如果为0则执行 */
    vldmiaeq r0!, {s16 - s31}  /* 恢复FPU寄存器 */

    msr psp, r0                /* 设置新PSP */
    isb                        /* 指令同步屏障 */
    bx r14                     /* 异常返回 */

    align 4
}
// clang-format on
#else
/* 其他编译器版本 - 使用__naked属性 */
__naked void PendSV_Handler(void)
{
    __ASM volatile("mrs r0, psp                         \n" /* 获取当前PSP */
                   "isb                                 \n" /* 指令同步屏障 */
                   "                                    \n"
                   "ldr r3, =KernelCurrentTCB           \n" /* 获取当前TCB指针 */
                   "ldr r2, [r3]                        \n" /* 获取当前TCB */
                   "                                    \n"
                   "tst r14, #0x10                      \n" /* 检查FPU使用标志 */
                   "it eq                               \n" /* 如果为0则执行 */
                   "vstmdbeq r0!, {s16-s31}             \n" /* 保存FPU寄存器 */
                   "                                    \n"
                   "stmdb r0!, {r4-r11, r14}            \n" /* 保存核心寄存器 */
                   "str r0, [r2]                        \n" /* 保存新栈顶到TCB */
                   "                                    \n"
                   "stmdb sp!, {r0, r3}                 \n" /* 保存寄存器到MSP栈 */
                   "bl v_kernel_task_switch             \n" /* 调用任务切换 */
                   "ldmia sp!, {r0, r3}                 \n" /* 恢复寄存器 */
                   "                                    \n"
                   "ldr r1, [r3]                        \n" /* 获取新TCB */
                   "ldr r0, [r1]                        \n" /* 获取新栈顶 */
                   "                                    \n"
                   "ldmia r0!, {r4-r11, r14}            \n" /* 恢复核心寄存器 */
                   "                                    \n"
                   "tst r14, #0x10                      \n" /* 检查FPU使用标志 */
                   "it eq                               \n" /* 如果为0则执行 */
                   "vldmiaeq r0!, {s16-s31}             \n" /* 恢复FPU寄存器 */
                   "                                    \n"
                   "msr psp, r0                         \n" /* 设置新PSP */
                   "isb                                 \n" /* 指令同步屏障 */
                   "bx r14                              \n" /* 异常返回 */
                   "                                    \n"
                   ".align 4                            \n");
}
#endif /* defined(__CC_ARM) */
#else
/**
 * @brief PendSV异常处理函数 - 无FPU版本
 * @details
 *  此版本不包含FPU支持，只处理核心寄存器的保存和恢复，
 *  适用于没有FPU或不需要浮点运算的系统，性能更优。
 */
#if defined(__CC_ARM)
/* AC5编译器版本 - 使用__asm语法 */
// clang-format off
__asm void PendSV_Handler(void)
{
	extern KernelCurrentTCB;
    extern v_kernel_task_switch;
	
    mrs r0, psp                /* 获取当前PSP */
    isb                        /* 指令同步屏障 */

    ldr r3, = KernelCurrentTCB /* 获取当前TCB指针 */
    ldr r2, [r3]               /* 获取当前TCB */

    stmdb r0!, {r4 - r11, r14} /* 保存核心寄存器 */
    str r0, [r2]               /* 保存新栈顶到TCB */

    stmdb sp!, {r0, r3}        /* 保存寄存器到MSP栈 */
    bl v_kernel_task_switch     /* 调用任务切换 */
    ldmia sp!, {r0, r3}        /* 恢复寄存器 */

    ldr r1, [r3]               /* 获取新TCB */
    ldr r0, [r1]               /* 获取新栈顶 */

    ldmia r0!, {r4 - r11, r14} /* 恢复核心寄存器 */

    msr psp, r0                /* 设置新PSP */
    isb                        /* 指令同步屏障 */
    bx r14                     /* 异常返回 */

    align 4
}
// clang-format on
#else
/* 其他编译器版本 - 使用__naked属性 */
__naked void PendSV_Handler(void)
{
    __ASM volatile("mrs r0, psp                         \n" /* 获取当前PSP */
                   "isb                                 \n" /* 指令同步屏障 */
                   "                                    \n"
                   "ldr r3, =KernelCurrentTCB           \n" /* 获取当前TCB指针 */
                   "ldr r2, [r3]                        \n" /* 获取当前TCB */
                   "                                    \n"
                   "stmdb r0!, {r4-r11, r14}            \n" /* 保存核心寄存器 */
                   "str r0, [r2]                        \n" /* 保存新栈顶到TCB */
                   "                                    \n"
                   "stmdb sp!, {r0, r3}                 \n" /* 保存寄存器到MSP栈 */
                   "bl v_kernel_task_switch             \n" /* 调用任务切换 */
                   "ldmia sp!, {r0, r3}                 \n" /* 恢复寄存器 */
                   "                                    \n"
                   "ldr r1, [r3]                        \n" /* 获取新TCB */
                   "ldr r0, [r1]                        \n" /* 获取新栈顶 */
                   "                                    \n"
                   "ldmia r0!, {r4-r11, r14}            \n" /* 恢复核心寄存器 */
                   "                                    \n"
                   "msr psp, r0                         \n" /* 设置新PSP */
                   "isb                                 \n" /* 指令同步屏障 */
                   "bx r14                              \n" /* 异常返回 */
                   "                                    \n"
                   ".align 4                            \n");
}
#endif /* defined(__CC_ARM) */
#endif /* EK_CORO_FPU_ENABLE == 1 */

#endif /*EK_CORO_ENABLE == 1*/