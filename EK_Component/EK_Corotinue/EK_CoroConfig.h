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
 * @warning: 此处必须要包含你的设备的文件头！
 * @example: #include "stm32f4xx_hal.h"
 */
#include "stm32f4xx_hal.h"

/*CMx芯片自包含*/
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

/*CMSIS 头文件自动包含*/
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
 * @brief 判断FPU是否启用
 * 
 */
#if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
#define EK_CORO_FPU_USED (1)
#else
#define EK_CORO_FPU_USED (0)
#endif /* __FPU_PRESENT == 1 && __FPU_USED == 1 */

/**
 * @brief 协程优先级分组数目
 * 
 */
#ifndef EK_CORO_PRIORITY_GROUPS
#define EK_CORO_PRIORITY_GROUPS (16)
#endif /* EK_CORO_PRIORITY_GROUPS */

/*
裸函数宏
具体来说，当一个函数被声明为 naked 时，编译器不会为它生成：
  1.  函数前奏（Prologue）: 通常是保存调用者寄存器（如LR）、设置堆栈指针等代码。
  2.  函数尾声（Epilogue）: 通常是恢复寄存器、清理堆栈、返回等代码。
*/
#ifndef __naked
#if defined(__GNUC__) || defined(__clang__)
#define __naked __attribute__((naked))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __naked __naked
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __naked __declspec(naked)
#else
#define __naked
#endif /* __GNUC__ || __clang__ || __IAR_SYSTEMS_ICC__ || __CC_ARM || __ARMCC_VERSION */
#endif /* __naked */

/**
 * @brief 系统的时钟频率(HZ)
 * 
 */
#ifndef EK_CORO_SYSTEM_FREQ
#define EK_CORO_SYSTEM_FREQ (SystemCoreClock)
#endif /*EK_CORO_SYSTEM_FREQ*/

/**
 * @brief SysTick中断周期
 * 
 */
#ifndef EK_CORO_TICK_RATE_HZ
#define EK_CORO_TICK_RATE_HZ (1000)
#endif /*EK_CORO_TICK_RATE_HZ*/

/**
 * @brief 栈溢出检测方法
 * @details 0: 禁用栈溢出检测; 1: 方法1(检测栈底填充值); 2: 方法2(检测栈指针是否超出范围)
 * @note 方法1性能较好但检测范围有限; 方法2检测更全面但性能开销稍大
 */
#ifndef EK_CORO_STACK_OVERFLOW_CHECK_ENABLE
#define EK_CORO_STACK_OVERFLOW_CHECK_ENABLE (0) // 0:禁用 1:方法1 2:方法2
#endif /*EK_CORO_STACK_OVERFLOW_CHECK_ENABLE*/

/**
 * @brief 协程空闲任务堆栈大小
 * 
 */
#ifndef EK_CORO_IDLE_TASK_STACK_SIZE
#define EK_CORO_IDLE_TASK_STACK_SIZE (256) // 定义空闲任务的堆栈大小
#endif /* EK_CORO_IDLE_TASK_STACK_SIZE */

/**
 * @brief 是否开启空闲钩子函数
 * @details 如果开启 每次进入空闲任务就会调用一次 EK_CoroIdleHook 函数 用户可以自己实现具体内容
 */
#ifndef EK_CORO_IDLE_HOOK_ENABLE
#define EK_CORO_IDLE_HOOK_ENABLE (0)
#endif /* EK_CORO_IDLE_HOOK_ENABLE */

/**
 * @brief 是否使能高水位检测功能
 * @details 启用后会统计每个任务的栈使用历史最大值，可用于调试和优化内存使用
 */
#ifndef EK_HIGH_WATER_MARK_ENABLE
#define EK_HIGH_WATER_MARK_ENABLE (0)
#endif /* EK_HIGH_WATER_MARK_ENABLE */

/**
 * @brief 是否使能消息队列
 * 
 */
#ifndef EK_CORO_MESSAGE_QUEUE_ENABLE
#define EK_CORO_MESSAGE_QUEUE_ENABLE (1) // 1:使能 0:失能
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE */

/**
 * @brief 是否使能信号量
 *
 */
#ifndef EK_CORO_SEMAPHORE_ENABLE
#define EK_CORO_SEMAPHORE_ENABLE (0) // 1:使能 0:失能
#endif /* EK_CORO_SEMAPHORE_ENABLE */

#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROCONFIG_H */