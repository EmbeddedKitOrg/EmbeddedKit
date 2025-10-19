/**
 * @file EK_CoroMarco.h
 * @brief 协程内核的底层配置文件
 * @details
 *  - 自动检测并包含对应的 Cortex-M 核心头文件。
 *  - 自动检测并包含对应编译器的 CMSIS 头文件。
 *  - 提供 __naked 宏，用于在不同编译器下定义裸函数。
 * @author N1ntyNine99
 * @date 2025-09-22
 * @version 1.0
 */

#ifndef __EK_COROCONFIG_H
#define __EK_COROCONFIG_H

#include "../EK_Config.h"

#if (EK_CORO_ENABLE == 1)

/**
 * @warning: 此处必须要包含你的设备的文件头！其他的宏禁止修改!
 * @example: #include "stm32f4xx.h" 、#include "stm32f1xx.h"
 */
#include "stm32f4xx.h"

/**
 * @brief CMSIS 头文件自动包含
 * @warning 禁止修改
 */
#if defined(__clang__)
#include "cmsis_armclang.h"
#elif defined(__GNUC__)
#include "cmsis_gcc.h"
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "cmsis_armcc.h"
#else
#error "Unsupported compiler. Please add a corresponding CMSIS header."
#endif /* Compiler CMSIS header selection */

/**
 * @brief CortexMx 芯片包自动包含
 * @warning 禁止修改
 */
#if defined(__CM7_REV)
#include "core_cm7.h"
#define CORTEX_M_CORE (7)
#elif defined(__CM4_REV)
#include "core_cm4.h"
#define CORTEX_M_CORE (4)
#elif defined(__CM3_REV)
#include "core_cm3.h"
#define CORTEX_M_CORE (3)
#elif defined(__CM0_REV)
#include "core_cm0.h"
#define CORTEX_M_CORE (0)
#elif defined(__CM0PLUS_REV)
#include "core_cm0plus.h"
#define CORTEX_M_CORE (0)
#else
#error "Cannot detect Cortex-M Core. Please include the correct core header manually in EK_CoroMarco.h."
#endif /* Cortex-M core detection */

/**
 * @brief 判断FPU是否启用
 * @warning 禁止修改
 *
 * 改进的FPU检测逻辑：
 * 1. 检查硬件是否支持FPU
 * 2. 检查编译器是否启用了FPU
 * 3. 检查运行时FPU是否已初始化
 */
#if (__FPU_PRESENT == 1)
// 硬件支持FPU，检查编译器配置
#if defined(__ARM_FP) && (__ARM_FP != 0)
// 编译器启用了FPU指令
#if defined(__SOFTFP__)
// 软浮点调用约定，但可能有硬件FPU支持
#define EK_CORO_FPU_ENABLE     (1)
#define EK_CORO_FPU_ABI_SOFTFP (1)
#else
// 硬浮点调用约定
#define EK_CORO_FPU_ENABLE   (1)
#define EK_CORO_FPU_ABI_HARD (1)
#endif
#else
// 编译器未启用FPU，使用软件浮点
#define EK_CORO_FPU_ENABLE (0)
#endif
#else
// 硬件不支持FPU
#define EK_CORO_FPU_ENABLE (0)
#endif /* __FPU_PRESENT == 1 */

/* FPU ABI类型定义 */
#ifndef EK_CORO_FPU_ABI_SOFTFP
#define EK_CORO_FPU_ABI_SOFTFP (0)
#endif
#ifndef EK_CORO_FPU_ABI_HARD
#define EK_CORO_FPU_ABI_HARD (0)
#endif

/* ================================ FPU 寄存器地址和位操作宏定义 ================================ */
// 定义FPCCR寄存器地址和要设置的位
#define FPCCR            ((volatile uint32_t *)(0xE000EF34UL))
#define ASPEN_LSPEN_BITS (0x3UL << 30UL)

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
ALWAYS_INLINE uint8_t v_kernel_find_msb_index(EK_BitMap_t val)
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
 * @brief 位图的位数
 * 
 */
#define EK_BITMAP_MAX_BIT (sizeof(EK_BitMap_t) * 8 - 1)

/**
 * @brief 计算最高的优先级
 * 
 */
#define EK_KERNEL_GET_HIGHEST_PRIO(__BITMAP__) (EK_BITMAP_MAX_BIT - EK_KERNEL_CLZ(__BITMAP__))

/**
 * @brief 将毫秒时间转换为系统时钟节拍数(Ticks)
 * @details 此宏用于将用户指定的时间(毫秒)转换为协程系统内部使用的时钟节拍数。
 *          系统时钟节拍是协程调度器的基本时间单位，由SysTick定时器定期产生中断。
 *
 * @param X 要转换的毫秒时间，必须是正整数或0
 * @return uint32_t 对应的系统时钟节拍数
 *
 * @note 此宏在编译时计算，无运行时开销
 * @note 当EK_CORO_TICK_RATE_HZ为1000时，转换结果等于输入值
 * @note 建议使用整数毫秒值以获得最佳精度
 * @note 对于小数毫秒值，建议四舍五入到最接近的整数
 *
 * @warning 传入负数会导致未定义行为
 * @warning 传入过大的值可能导致整数溢出
 * @warning 确保EK_CORO_TICK_RATE_HZ已正确配置
 *
 * @see EK_CORO_TICK_RATE_HZ 系统时钟节拍率配置
 * @see EK_vCoroDelay() 协程延时函数
 * @see EK_rMsgSend() 消息发送函数
 */
#define EK_MS_TO_TICKS(X) (((X) * EK_CORO_TICK_RATE_HZ) / 1000U)

/* ================================ 协程系统基础配置 ================================ */
/**
 * @brief 协程系统基础配置选项
 * @details 这些配置定义了协程系统的基本运行参数和底层特性
 *
 * EK_CORO_IDLE_TASK_STACK_SIZE  - 协程空闲任务堆栈大小 (字节)
 *                               空闲任务是系统在没有其他就绪任务时运行的特殊任务
 *                               负责系统清理、资源回收和功耗管理等功能
 *
 * EK_MAX_DELAY                  - 最大阻塞时间常量
 *                               用于表示无限期阻塞，通常用于需要等待外部事件唤醒的场景
 *                               在延时函数和事件等待函数中使用，值为UINT32_MAX表示最大32位无符号整数
 *
 * EK_STACK_FILL_PATTERN        - 协程任务栈填充模式
 *                               用于栈溢出检测和高水位标记计算的填充值
 *                               任务栈初始化时会用此值填充，运行时检查被覆盖的区域来评估栈使用情况
 *                               0xA5 (二进制10100101) 是常用的调试填充模式，容易检测内存修改
 *
 * EK_IS_IN_INTERRUPT()         - 中断上下文检测宏
 *                               通过读取Cortex-M的中断控制状态寄存器(ICSR)来判断当前是否在中断中
 *                               返回true表示在中断上下文，false表示在任务上下文
 *                               用于临界区管理、调度决策和API安全性检查
 *                               适用于Cortex-M3/M4/M7架构的IPSR寄存器
 */

#ifndef EK_CORO_IDLE_TASK_STACK_SIZE
#define EK_CORO_IDLE_TASK_STACK_SIZE (512) // 定义空闲任务的堆栈大小 (字节)
#endif /* EK_CORO_IDLE_TASK_STACK_SIZE */

#define EK_MAX_DELAY          (UINT32_MAX) // 最大阻塞时间，表示无限期阻塞

#define EK_STACK_FILL_PATTERN (0xA5) // 栈填充模式，用于栈溢出检测和高水位计算

#define EK_IS_IN_INTERRUPT()  (__get_IPSR() != 0U) // 中断上下文检测，适用于Cortex-M3/M4/M7

/* ================================ 协程优先级管理配置 ================================ */
/**
 * @brief 协程优先级管理配置
 * @details 基于EK_CORO_PRIORITY_GROUPS配置动态生成优先级管理相关的宏和类型定义
 *          采用位图算法实现O(1)时间复杂度的最高优先级任务查找
 *
 * 优先级系统特性：
 * - 数值越小优先级越高 (0为最高优先级)
 * - 使用位图管理就绪任务，每个位对应一个优先级级别
 * - 支持最多32个优先级级别，可根据系统需求配置
 * - 利用硬件CLZ指令实现快速优先级查找
 *
 * 配置参数说明：
 * EK_CORO_PRIORITY_MOUNT        - 优先级总数，向上取整到2的幂次，优化位图操作
 *                               8个优先级:  使用8位位图 (uint8_t)
 *                               16个优先级: 使用16位位图 (uint16_t)
 *                               32个优先级: 使用32位位图 (uint32_t)
 *
 * EK_CORO_MAX_PRIORITY_NBR      - 最高优先级对应的位图值
 *                               用于位图操作和优先级计算
 *
 * EK_BitMap_t                   - 位图数据类型
 *                               根据优先级组数量自动选择合适的整数类型
 *                               平衡内存使用和操作效率
 */
#if (EK_CORO_PRIORITY_GROUPS <= 8)
#define EK_CORO_PRIORITY_MOUNT   (8) // 支持8个优先级级别
#define EK_CORO_MAX_PRIORITY_NBR (0x80UL) // 最高优先级位掩码
typedef uint8_t EK_BitMap_t; // 8位位图类型
#elif (EK_CORO_PRIORITY_GROUPS <= 16)
#define EK_CORO_PRIORITY_MOUNT   (16) // 支持16个优先级级别
#define EK_CORO_MAX_PRIORITY_NBR (0x8000UL) // 最高优先级位掩码
typedef uint16_t EK_BitMap_t; // 16位位图类型
#else
#define EK_CORO_PRIORITY_MOUNT   (32) // 支持32个优先级级别
#define EK_CORO_MAX_PRIORITY_NBR (0x80000000UL) // 最高优先级位掩码
typedef uint32_t EK_BitMap_t; // 32位位图类型
#endif /* EK_CORO_PRIORITY_GROUPS selection */

/* ================================ 协程任务通知配置 ================================ */
/**
 * @brief 协程任务通知系统配置
 * @details 任务通知是一种轻量级的任务间通信机制，提供直接的任务间信号传递
 *          相比消息队列和信号量，具有更低的内存开销和更快的执行速度
 *
 * 任务通知特性：
 * - 每个任务内置最多32个通知位
 * - 支持位操作，可实现复杂的同步逻辑
 * - 直接针对特定任务，无需创建额外的同步对象
 * - 适用于简单的任务同步和事件通知场景
 *
 * 配置说明：
 * EK_CORO_TASK_NOTIFY_ENABLE   - 任务通知功能总开关
 *                               1: 启用任务通知功能
 *                               0: 禁用任务通知功能
 *
 * EK_CORO_TASK_NOTIFY_GROUP    - 任务通知位数量配置
 *                               ≤8: 使用8位无符号整数 (uint8_t)
 *                               ≤16: 使用16位无符号整数 (uint16_t)
 *                               >16: 使用32位无符号整数 (uint32_t)
 *
 * EK_CoroTaskNotifyState_t     - 任务通知状态数据类型
 *                               根据通知位数量自动选择合适的整数类型
 *                               用于存储任务的通知状态位和事件类型
 */
#if (EK_CORO_TASK_NOTIFY_ENABLE == 1)
#if (EK_CORO_TASK_NOTIFY_GROUP <= 8)
typedef uint8_t EK_CoroTaskNotifyState_t; // 支持8个通知位
#elif (EK_CORO_TASK_NOTIFY_GROUP <= 16)
typedef uint16_t EK_CoroTaskNotifyState_t; // 支持16个通知位
#else
typedef uint32_t EK_CoroTaskNotifyState_t; // 支持32个通知位
#endif /* EK_CORO_TASK_NOTIFY_GROUP implementation */
#endif /* EK_CORO_TASK_NOTIFY_ENABLE == 1 */

/**
 * @warning
 * 下面的宏定义都是提供的默认设置，请不要修改！
 * 想要具体配置 请到 `EK_Config.h` 中配置
 * 否则可能会出现未知的错误！
 */

/* ================================ 协程系统配置 ================================ */
/**
 * @brief 协程系统配置选项 (仅在 EK_CORO_ENABLE=1 时生效)
 *
 * EK_CORO_PRIORITY_GROUPS       - 协程优先级组数目
 * EK_CORO_SYSTEM_FREQ           - 系统时钟频率 (HZ)
 * EK_CORO_TICK_RATE_HZ          - SysTick中断周期 (HZ)
 * EK_CORO_IDLE_TASK_STACK_SIZE  - 协程空闲任务堆栈大小 (字节)
 */

#ifndef EK_CORO_PRIORITY_GROUPS
#define EK_CORO_PRIORITY_GROUPS (16)
#endif /* EK_CORO_PRIORITY_GROUPS */

#ifndef EK_CORO_SYSTEM_FREQ
#define EK_CORO_SYSTEM_FREQ (SystemCoreClock)
#endif /*EK_CORO_SYSTEM_FREQ*/

#ifndef EK_CORO_TICK_RATE_HZ
#define EK_CORO_TICK_RATE_HZ (1000)
#endif /*EK_CORO_TICK_RATE_HZ*/

#ifndef EK_CORO_IDLE_TASK_STACK_SIZE
#define EK_CORO_IDLE_TASK_STACK_SIZE (512)
#endif /* EK_CORO_IDLE_TASK_STACK_SIZE */

/* ================================ 协程功能配置 ================================ */
/**
 * @brief 协程功能配置选项 (仅在 EK_CORO_ENABLE=1 时生效)
 *
 * EK_CORO_IDLE_HOOK_ENABLE           - 是否开启空闲钩子函数
 *                                    如果开启，每次进入空闲任务就会调用一次 EK_CoroIdleHook 函数
 *                                    1:使能 0:失能
 *
 * EK_CORO_STACK_OVERFLOW_CHECK_ENABLE - 栈溢出检测方法
 *                                    0: 禁用栈溢出检测
 *                                    1: 方法1(检测栈底填充值)，性能较好但检测范围有限
 *                                    2: 方法2(检测栈指针是否超出范围)，检测更全面但性能开销稍大
 *
 * EK_HIGH_WATER_MARK_ENABLE          - 是否使能高水位检测功能
 *                                    启用后会统计每个任务的栈使用历史最大值，可用于调试和优化内存使用
 *                                    1:使能 0:失能
 * EK_CORO_TASK_NOTIFY_ENABLE         - 是否使能任务通知， 1:使能 0:失能
 * EK_CORO_MESSAGE_QUEUE_ENABLE       - 是否使能消息队列，1:使能 0:失能
 * EK_CORO_SEMAPHORE_ENABLE           - 是否使能信号量，1:使能 0:失能
 * EK_CORO_MUTEX_ENABLE               - 是否使能互斥锁，仅在 EK_CORO_SEMAPHORE_ENABLE 为1时有效，1:使能 0:失能
 * EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE - 是否开启优先级继承，仅在 EK_CORO_SEMAPHORE_ENABLE 为1时有效，1:使能 0:失能
 */

#ifndef EK_CORO_IDLE_HOOK_ENABLE
#define EK_CORO_IDLE_HOOK_ENABLE (0)
#endif /* EK_CORO_IDLE_HOOK_ENABLE */

#ifndef EK_CORO_STACK_OVERFLOW_CHECK_ENABLE
#define EK_CORO_STACK_OVERFLOW_CHECK_ENABLE (0)
#endif /*EK_CORO_STACK_OVERFLOW_CHECK_ENABLE*/

#ifndef EK_HIGH_WATER_MARK_ENABLE
#define EK_HIGH_WATER_MARK_ENABLE (0)
#endif /* EK_HIGH_WATER_MARK_ENABLE */

#ifndef EK_CORO_TASK_NOTIFY_ENABLE
#define EK_CORO_TASK_NOTIFY_ENABLE (0)
#endif /* EK_CORO_TASK_NOTIFY_ENABLE */

#ifndef EK_CORO_MESSAGE_QUEUE_ENABLE
#define EK_CORO_MESSAGE_QUEUE_ENABLE (0)
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE */

#ifndef EK_CORO_SEMAPHORE_ENABLE
#define EK_CORO_SEMAPHORE_ENABLE (0)
#endif /* EK_CORO_SEMAPHORE_ENABLE */

#if (EK_CORO_SEMAPHORE_ENABLE == 0)
#define EK_CORO_MUTEX_ENABLE (0)
#endif /* EK_CORO_SEMAPHORE_ENABLE == 0 */

#if (EK_CORO_MUTEX_ENABLE == 0)
#define EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE (0)
#endif /* EK_CORO_MUTEX_ENABLE == 0 */

#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROCONFIG_H */