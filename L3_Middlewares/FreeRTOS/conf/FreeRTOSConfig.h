/**
 * @file FreeRTOSConfig.h
 * @brief FreeRTOS 配置文件 - EmbeddedKit 项目
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

extern uint32_t SystemCoreClock;

/* ========================================================================
 * 基础配置参数
 * - configCPU_CLOCK_HZ: MCU系统时钟频率(Hz)，使用SystemCoreClock变量确保动态更新
 * - configTICK_RATE_HZ: RTOS系统节拍频率(Hz)，常用值1000Hz(1ms)
 * - configUSE_16_BIT_TICKS: 系统节拍计数器位宽，0=32位(ARM Cortex-M推荐)
 * - configMAX_PRIORITIES: 最大任务优先级数量，实际可用为0~(configMAX_PRIORITIES-1)
 * - configUSE_PREEMPTION: 使用抢占式调度器，1=启用抢占
 * - configUSE_TIME_SLICING: 使用时间片调度(同等优先级任务轮转)
 * - configUSE_MUTEXES: 使用互斥信号量
 * - configUSE_PORT_OPTIMISED_TASK_SELECTION: 使用硬件指令(CLZ)加速任务选择(ARM CM4专用)
 * ======================================================================== */
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configTICK_RATE_HZ                      1000
#define configUSE_16_BIT_TICKS                  0
#define configMAX_PRIORITIES                    5
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_MUTEXES                       1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1

/* ========================================================================
 * 内存配置
 * - configTOTAL_HEAP_SIZE: FreeRTOS堆总大小(字节)，heap_4.c管理的内存池
 * - configSUPPORT_STATIC_ALLOCATION: 支持静态内存分配
 * - configSUPPORT_DYNAMIC_ALLOCATION: 支持动态内存分配
 * - configAPPLICATION_ALLOCATED_HEAP: 用户自定义堆内存定义(如需放置在CCM RAM)
 * ======================================================================== */
#define configTOTAL_HEAP_SIZE            (32 * 1024)
#define configSUPPORT_STATIC_ALLOCATION  0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configAPPLICATION_ALLOCATED_HEAP 0

/* ========================================================================
 * 钩子函数与检测配置
 * - configUSE_IDLE_HOOK: 空闲任务执行时运行空闲钩子函数
 * - configUSE_TICK_HOOK: 每次节拍中断执行后运行节拍钩子函数
 * - configUSE_MALLOC_FAILED_HOOK: 内存申请失败钩子
 * - configCHECK_FOR_STACK_OVERFLOW: 栈溢出检测 (1=简单检测, 2=深度检测)
 * ======================================================================== */
#define configUSE_IDLE_HOOK            0
#define configUSE_TICK_HOOK            0
#define configUSE_MALLOC_FAILED_HOOK   0
#define configCHECK_FOR_STACK_OVERFLOW 0

/* ========================================================================
 * 任务相关配置
 * - configMINIMAL_STACK_SIZE: 最小任务栈大小(字)，空闲任务/定时器任务使用
 * - configMAX_TASK_NAME_LEN: 任务名称最大长度
 * - configTASK_NOTIFICATION_ARRAY_ENTRIES: 任务本地存储缓冲区索引
 * - configUSE_STATS_FORMATTING_FUNCTIONS: 启用 vTaskList/vTaskGetRunTimeStats 格式化输出
 * ======================================================================== */
#define configMINIMAL_STACK_SIZE              128
#define configMAX_TASK_NAME_LEN               16
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1
#define configUSE_STATS_FORMATTING_FUNCTIONS  1

/* ========================================================================
 * 软件定时器配置 
 * - configUSE_TIMERS: 启用软件定时器功能
 * - configTIMER_TASK_PRIORITY: 定时器服务任务优先级(建议设为最高)
 * - configTIMER_QUEUE_LENGTH: 定时器命令队列长度
 * - configTIMER_TASK_STACK_DEPTH: 定时器任务栈大小
 * ======================================================================== */
#define configUSE_TIMERS             0
#define configTIMER_TASK_PRIORITY    (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH     10
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE

/* ========================================================================
 * 队列/信号量配置
 * - configUSE_QUEUE_SETS: 队列注册机制
 * - configUSE_RECURSIVE_MUTEXES: 启用递归互斥锁
 * - configUSE_COUNTING_SEMAPHORES: 启用计数型信号量
 * ======================================================================== */
#define configUSE_QUEUE_SETS          0
#define configUSE_RECURSIVE_MUTEXES   1
#define configUSE_COUNTING_SEMAPHORES 1

/* ========================================================================
 * 协程配置 (现代 FreeRTOS 项目极少使用，通常用 Task 代替)
 * - configUSE_CO_ROUTINES: 启用协程功能
 * - configMAX_CO_ROUTINE_PRIORITIES: 协程的最大优先级数量
 * ======================================================================== */
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES 2

/* ========================================================================
 * 断言配置
 * - configASSERT: 推荐实现为打印出错的文件名和行号
 * ======================================================================== */
#define configASSERT(x)           \
    if ((x) == 0)                 \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for (;;);                 \
    }

/* ========================================================================
 * API函数包含配置
 * ======================================================================== */
#define INCLUDE_vTaskPrioritySet          1
#define INCLUDE_uxTaskPriorityGet         1
#define INCLUDE_vTaskDelete               1
#define INCLUDE_vTaskSuspend              1
#define INCLUDE_xTaskDelayUntil           1
#define INCLUDE_vTaskDelay                1
#define INCLUDE_xTaskGetSchedulerState    1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xTimerPendFunctionCall    1

/* ========================================================================
 * 中断配置
 * - configLIBRARY_LOWEST_INTERRUPT_PRIORITY: 最低优先级(15)
 * - configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY: FreeRTOS可管理的最高优先级(5)
 * - xPortPendSVHandler: 映射到标准 PendSV_Handler
 * - vPortSVCHandler: 映射到标准 SVC_Handler
 * - xPortSysTickHandler: 映射到标准 SysTick_Handler 
 * ======================================================================== */
#ifdef __NVIC_PRIO_BITS
#    define configPRIO_BITS __NVIC_PRIO_BITS
#else
#    define configPRIO_BITS 4
#endif

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY              (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY         (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#define xPortPendSVHandler                           PendSV_Handler
#define vPortSVCHandler                              SVC_Handler
#define xPortSysTickHandler                          SysTick_Handler

/* ========================================================================
 * ARM Cortex-M 安全扩展配置
 * ======================================================================== */
#define configENABLE_MPU       0
#define configENABLE_FZ        0
#define configENABLE_TRUSTZONE 0

/* ========================================================================
 * 调试与运行时统计
 * ======================================================================== */
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY      0

#ifdef __cplusplus
extern "C"
{
#endif

/* 钩子函数原型声明 */
// void vApplicationIdleHook(void);
// void vApplicationTickHook(void);
// void vApplicationMallocFailedHook(void);
// void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);

#ifdef __cplusplus
}
#endif

#endif /* FREERTOS_CONFIG_H */
