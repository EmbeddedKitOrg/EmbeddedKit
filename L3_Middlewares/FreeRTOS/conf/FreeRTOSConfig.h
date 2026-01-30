/**
 * @file FreeRTOSConfig.h
 * @brief FreeRTOS 配置文件 - EmbeddedKit 项目
 *
 * 本文件定义了 FreeRTOS 在 EmbeddedKit 项目中的所有配置参数。
 * 针对 STM32F429 (ARM Cortex-M4F, 168MHz) 进行了优化配置。
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ========================================================================
 * 基础配置参数
 * - configCPU_CLOCK_HZ: MCU系统时钟频率(Hz)，需与实际系统时钟配置一致
 * - configTICK_RATE_HZ: RTOS系统节拍频率(Hz)，常用值100Hz(10ms)或1000Hz(1ms)
 * - configUSE_16_BIT_TICKS: 系统节拍计数器位宽，0=32位(ARM Cortex-M推荐)，1=16位
 * - configMAX_PRIORITIES: 最大任务优先级数量，实际可用为1~(configMAX_PRIORITIES-1)
 * - configUSE_PREEMPTION: 使用抢占式调度器，1=启用抢占，0=使用协作式调度
 * - configUSE_TIME_SLICING: 使用时间片调度(同等优先级任务轮转)
 * - configUSE_MUTEXES: 使用互斥信号量，必须启用才能使用递归互斥锁
 * ======================================================================== */
#define configCPU_CLOCK_HZ     168000000UL
#define configTICK_RATE_HZ     1000
#define configUSE_16_BIT_TICKS 0
#define configMAX_PRIORITIES   5
#define configUSE_PREEMPTION   1
#define configUSE_TIME_SLICING 1
#define configUSE_MUTEXES      1

/* ========================================================================
 * 内存配置
 * - configTOTAL_HEAP_SIZE: FreeRTOS堆总大小(字节)，heap_4.c管理的内存池
 * - configSUPPORT_STATIC_ALLOCATION: 支持静态内存分配
 * - configSUPPORT_DYNAMIC_ALLOCATION: 支持动态内存分配
 * ======================================================================== */
#define configTOTAL_HEAP_SIZE            (32 * 1024)
#define configSUPPORT_STATIC_ALLOCATION  0
#define configSUPPORT_DYNAMIC_ALLOCATION 1

/* ========================================================================
 * 任务相关配置
 * - configMINIMAL_STACK_SIZE: 最小任务栈大小(字)，空闲任务使用的栈大小
 * - configMAX_TASK_NAME_LEN: 任务名称最大长度
 * - configTASK_NOTIFICATION_ARRAY_ENTRIES: 任务本地存储缓冲区索引，0表示不使用
 * ======================================================================== */
#define configMINIMAL_STACK_SIZE              128
#define configMAX_TASK_NAME_LEN               16
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1

/* ========================================================================
 * 钩子函数配置
 * - configUSE_IDLE_HOOK: 空闲任务执行时运行空闲钩子函数
 * - configUSE_TICK_HOOK: 每次节拍中断执行后运行节拍钩子函数
 * ======================================================================== */
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0

/* ========================================================================
 * 队列/信号量配置
 * - configUSE_QUEUE_SETS: 队列注册机制(调试用)
 * - configUSE_RECURSIVE_MUTEXES: 互斥信号量使用递归
 * - configUSE_COUNTING_SEMAPHORES: 使用计数型信号量
 * ======================================================================== */
#define configUSE_QUEUE_SETS          0
#define configUSE_RECURSIVE_MUTEXES   1
#define configUSE_COUNTING_SEMAPHORES 1

/* ========================================================================
 * 断言配置
 * - configASSERT: 断言宏定义，断言失败时禁用中断并进入死循环
 * ======================================================================== */
#define configASSERT(x)           \
    if ((x) == 0)                 \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for (;;);                 \
    }

/* ========================================================================
 * API函数包含配置
 * - INCLUDE_vTaskPrioritySet: 使能vTaskPrioritySet函数
 * - INCLUDE_uxTaskPriorityGet: 使能uxTaskPriorityGet函数
 * - INCLUDE_vTaskDelete: 使能vTaskDelete函数
 * - INCLUDE_vTaskSuspend: 使能vTaskSuspend函数
 * - INCLUDE_xTaskDelayUntil: 使能xTaskDelayUntil函数
 * - INCLUDE_vTaskDelay: 使能vTaskDelay函数
 * - INCLUDE_xTaskGetSchedulerState: 使能xTaskGetSchedulerState函数
 * - INCLUDE_xTaskGetCurrentTaskHandle: 使能xTaskGetCurrentTaskHandle函数
 * ======================================================================== */
#define INCLUDE_vTaskPrioritySet          1
#define INCLUDE_uxTaskPriorityGet         1
#define INCLUDE_vTaskDelete               1
#define INCLUDE_vTaskSuspend              1
#define INCLUDE_xTaskDelayUntil           1
#define INCLUDE_vTaskDelay                1
#define INCLUDE_xTaskGetSchedulerState    1
#define INCLUDE_xTaskGetCurrentTaskHandle 1

/* ========================================================================
 * 协程配置
 * - USE_CO_ROUTINES: 使能协程功能
 * ======================================================================== */
#define USE_CO_ROUTINES 0

/* ========================================================================
 * 中断配置
 * - configLIBRARY_LOWEST_INTERRUPT_PRIORITY: 库配置的最低中断优先级(数值越大优先级越低)
 * - configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY: 库配置的最高系统调用中断优先级
 * - configKERNEL_INTERRUPT_PRIORITY: 内核使用的最低中断优先级(转换为硬件值)
 * - configMAX_SYSCALL_INTERRUPT_PRIORITY: 最高系统调用中断优先级(转换为硬件值)
 * - xPortPendSVHandler: PendSV中断处理函数映射
 * - vPortSVCHandler: SVC中断处理函数映射
 * - xPortSysTickHandler:滴答计时器函数映射
 * ======================================================================== */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY              (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY         (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define xPortPendSVHandler                           PendSV_Handler
#define vPortSVCHandler                              SVC_Handler
#define xPortSysTickHandler                          SysTick_Handler

/* ========================================================================
 * 硬件特定配置
 * - configPRIO_BITS: Cortex-M4F优先级位数
 * ======================================================================== */
#define configPRIO_BITS 4

/* ========================================================================
 * ARM Cortex-M特定配置
 * - configENABLE_MPU: 使能ARMv8-M MPU扩展
 * - configENABLE_FZ: 使能ARMv8-M FZ扩展
 * - configENABLE_TRUSTZONE: 使能ARMv8-M TrustZone扩展
 * ======================================================================== */
#define configENABLE_MPU       0
#define configENABLE_FZ        0
#define configENABLE_TRUSTZONE 0

/* ========================================================================
 * 调试配置
 * - configGENERATE_RUN_TIME_STATS: 生成运行时任务统计信息
 * - configUSE_TRACE_FACILITY: 使用追踪钩子
 * ======================================================================== */
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY      0

/* ========================================================================
 * 应用特定钩子函数说明
 * ========================================================================
 * 以下钩子函数需要在用户代码中实现(通常在L5_App层):
 *
 * - vApplicationIdleHook()           (当configUSE_IDLE_HOOK = 1时)
 * - vApplicationTickHook()            (当configUSE_TICK_HOOK = 1时)
 * - vApplicationStackOverflowHook()   (任务名作为char*传入)
 * - vApplicationMallocFailedHook()
 * - vApplicationGetIdleTaskMemory()  (当configSUPPORT_STATIC_ALLOCATION = 1时)
 * - vApplicationGetTimerTaskMemory() (当configSUPPORT_STATIC_ALLOCATION = 1时)
 * ======================================================================== */

#endif /* FREERTOS_CONFIG_H */
