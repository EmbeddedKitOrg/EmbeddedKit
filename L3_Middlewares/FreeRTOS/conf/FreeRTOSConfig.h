/**
 * @file FreeRTOSConfig.h
 * @brief FreeRTOS 配置文件 - EmbeddedKit 项目
 *
 * 本文件定义了 FreeRTOS 在 EmbeddedKit 项目中的所有配置参数。
 * 针对 STM32F429 (ARM Cortex-M4F, 168MHz) 进行了优化配置。
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ============================================================================ */
/*                            基础配置参数                                     */
/* ============================================================================ */

/**
 * @brief MCU 系统时钟频率 (Hz)
 * @note 必须与实际系统时钟配置一致，STM32F429 典型值为 168MHz
 */
#define configCPU_CLOCK_HZ              168000000UL

/**
 * @brief RTOS 系统节拍频率 (Hz)
 * @note 常用值: 100Hz (10ms), 1000Hz (1ms)
 */
#define configTICK_RATE_HZ              1000

/**
 * @brief 系统节拍计数器位宽
 * @note 0 = 16位, 1 = 32位 (ARM Cortex-M 推荐)
 */
#define configUSE_16_BIT_TICKS          0

/**
 * @brief 最大任务优先级数量
 * @note 实际可用的优先级为 1 ~ configMAX_PRIORITIES-1
 */
#define configMAX_PRIORITIES            5

/**
 * @brief 使用抢占式调度器
 * @note 1=启用抢占, 0=使用协作式调度
 */
#define configUSE_PREEMPTION            1

/**
 * @brief 使用时间片调度 (同等优先级任务轮转)
 */
#define configUSE_TIME_SLICING          1

/**
 * @brief 使用互斥信号量
 * @note 必须启用才能使用递归互斥锁
 */
#define configUSE_MUTEXES               1

/**
 * @brief 空闲任务执行时运行空闲钩子函数
 */
#define configUSE_IDLE_HOOK             0

/**
 * @brief 每次节拍中断执行后运行节拍钩子函数
 */
#define configUSE_TICK_HOOK             0

/* ============================================================================ */
/*                            内存配置                                         */
/* ============================================================================ */

/**
 * @brief FreeRTOS 堆总大小 (字节)
 * @note heap_4.c 管理的内存池大小
 * @warning 需要根据实际可用 RAM 调整，STM32F429 有 256KB SRAM
 */
#define configTOTAL_HEAP_SIZE           (32 * 1024)

/**
 * @brief 支持静态内存分配
 */
#define configSUPPORT_STATIC_ALLOCATION 0

/**
 * @brief 支持动态内存分配
 */
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* ============================================================================ */
/*                            任务相关配置                                     */
/* ============================================================================ */

/**
 * @brief 最小任务栈大小 (字)
 * @note 空闲任务使用的栈大小
 */
#define configMINIMAL_STACK_SIZE         128

/**
 * @brief 任务名称最大长度
 */
#define configMAX_TASK_NAME_LEN         16

/**
 * @brief 任务本地存储缓冲区索引 (0 = 不使用)
 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1

/* ============================================================================ */
/*                            队列/信号量配置                                  */
/* ============================================================================ */

/**
 * @brief 队列注册机制 (调试用)
 */
#define configUSE_QUEUE_SETS            0

/**
 * @brief 互斥信号量使用递归
 */
#define configUSE_RECURSIVE_MUTEXES     1

/**
 * @brief 使用计数型信号量
 */
#define configUSE_COUNTING_SEMAPHORES   1

/* ============================================================================ */
/*                            断言配置                                         */
/* ============================================================================ */

/**
 * @brief 断言宏定义
 * @note 断言失败时禁用中断并进入死循环
 */
#define configASSERT(x) if((x) == 0) { taskDISABLE_INTERRUPTS(); for(;;); }

/* ============================================================================ */
/*                            API 函数包含配置                                 */
/* ============================================================================ */

#define INCLUDE_vTaskPrioritySet        1
#define INCLUDE_uxTaskPriorityGet       1
#define INCLUDE_vTaskDelete             1
#define INCLUDE_vTaskSuspend            1
#define INCLUDE_xTaskDelayUntil         1
#define INCLUDE_vTaskDelay              1
#define INCLUDE_xTaskGetSchedulerState  1
#define INCLUDE_xTaskGetCurrentTaskHandle 1

/* ============================================================================ */
/*                            协程配置 (本项目不使用)                          */
/* ============================================================================ */

#define USE_CO_ROUTINES                 0

/* ============================================================================ */
/*                            中断配置                                         */
/* ============================================================================ */

/**
 * @brief 系统节拍中断优先级配置
 * @note Cortex-M4F 优先级位数为 4，范围 0-15 (0 最高)
 */

/**
 * @brief 库配置的最低中断优先级 (数值越大优先级越低)
 */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15

/**
 * @brief 库配置的最高系统调用中断优先级
 * @note 优先级高于此值的中断不能调用 FreeRTOS API
 */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/**
 * @brief 内核使用的最低中断优先级
 */
#define configKERNEL_INTERRUPT_PRIORITY \
    ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/**
 * @brief 最高系统调用中断优先级 (转换为硬件值)
 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* ============================================================================ */
/*                            硬件特定配置                                     */
/* ============================================================================ */

/**
 * @brief Cortex-M4F 优先级位数
 */
#define configPRIO_BITS                   4

/**
 * @brief SysTick 中断优先级
 */
#define xPortPendSVHandler              PendSV_Handler
#define vPortSVCHandler                 SVC_Handler

/* ============================================================================ */
/*                            ARM Cortex-M 特定配置                            */
/* ============================================================================ */

/* 使能 ARMv8-M 专用扩展 (不适用于 Cortex-M4F) */
#define configENABLE_MPU                0
#define configENABLE_FZ                 0
#define configENABLE_TRUSTZONE          0

/* ============================================================================ */
/*                            调试配置                                         */
/* ============================================================================ */

/**
 * @brief 生成运行时任务统计信息
 */
#define configGENERATE_RUN_TIME_STATS   0

/**
 * @brief 使用追踪钩子
 */
#define configUSE_TRACE_FACILITY        0

/* ============================================================================ */
/*                            应用特定钩子函数说明                             */
/* ============================================================================ */

/**
 * @note 以下钩子函数需要在用户代码中实现 (通常在 L5_App 层):
 *
 * - vApplicationIdleHook()           (当 configUSE_IDLE_HOOK = 1 时)
 * - vApplicationTickHook()            (当 configUSE_TICK_HOOK = 1 时)
 * - vApplicationStackOverflowHook()   (任务名作为 char* 传入)
 * - vApplicationMallocFailedHook()
 * - vApplicationGetIdleTaskMemory()  (当 configSUPPORT_STATIC_ALLOCATION = 1 时)
 * - vApplicationGetTimerTaskMemory() (当 configSUPPORT_STATIC_ALLOCATION = 1 时)
 */

#endif /* FREERTOS_CONFIG_H */
