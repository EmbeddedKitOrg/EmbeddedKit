/**
 * @file EK_CoroConfig.h
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
 * @example: #include "stm32f4xx_hal.h"
 */
#include "stm32f4xx_hal.h"

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
#error "Cannot detect Cortex-M Core. Please include the correct core header manually in EK_CoroConfig.h."
#endif /* Cortex-M core detection */

/**
 * @brief 判断FPU是否启用
 * @warning 禁止修改
 */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
#define EK_CORO_FPU_ENABLE (1)
#else
#define EK_CORO_FPU_ENABLE (0)
#endif /* __FPU_PRESENT == 1 && __FPU_USED == 1 */

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
 * EK_CORO_MUTEX_RECURSIVE_ENABLE         - 是否使能递归互斥量，仅在 EK_CORO_MUTEX_ENABLE 有效时才有意义
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
#define EK_CORO_MUTEX_RECURSIVE_ENABLE            (0)
#define EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE (0)
#endif /* EK_CORO_MUTEX_ENABLE == 0 */

#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROCONFIG_H */