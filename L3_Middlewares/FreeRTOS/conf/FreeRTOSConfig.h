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
 * - configTICK_TYPE_WIDTH_IN_BITS: 节拍计数器类型位宽
 * - configMAX_PRIORITIES: 最大任务优先级数量，实际可用为0~(configMAX_PRIORITIES-1)
 * - configUSE_PREEMPTION: 使用抢占式调度器，1=启用抢占
 * - configUSE_TIME_SLICING: 使用时间片调度(同等优先级任务轮转)
 * - configUSE_TICKLESS_IDLE: 低功耗无节拍空闲模式，0=禁用
 * - configUSE_MUTEXES: 使用互斥信号量
 * - configUSE_PORT_OPTIMISED_TASK_SELECTION: 使用硬件指令(CLZ)加速任务选择(ARM CM4专用)
 * - configIDLE_SHOULD_YIELD: 空闲任务是否主动让出CPU给同优先级任务
 * ======================================================================== */
#define configCPU_CLOCK_HZ                      (SystemCoreClock)
#define configTICK_RATE_HZ                      1000
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS
#define configMAX_PRIORITIES                    5
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_TICKLESS_IDLE                 0
#define configUSE_MUTEXES                       1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configIDLE_SHOULD_YIELD                 1

/* ========================================================================
 * 内存配置
 * - configTOTAL_HEAP_SIZE: FreeRTOS堆总大小(字节)，heap_4.c管理的内存池
 * - configSUPPORT_STATIC_ALLOCATION: 支持静态内存分配
 * - configSUPPORT_DYNAMIC_ALLOCATION: 支持动态内存分配
 * - configAPPLICATION_ALLOCATED_HEAP: 用户自定义堆内存定义(如需放置在CCM RAM)
 * - configSTACK_ALLOCATION_FROM_SEPARATE_HEAP: 任务栈从独立堆分配(需实现pvPortMallocStack/vPortFreeStack)
 * - configENABLE_HEAP_PROTECTOR: 启用堆保护(heap_4/heap_5边界检查和指针混淆)
 * - configHEAP_CLEAR_MEMORY_ON_FREE: 释放内存时清零
 * - configUSE_MINI_LIST_ITEM: 使用精简链表项节省RAM(可能违反严格别名规则)
 * - configSTACK_DEPTH_TYPE: 栈深度类型定义
 * - configMESSAGE_BUFFER_LENGTH_TYPE: 消息缓冲区长度类型
 * ======================================================================== */
#define configTOTAL_HEAP_SIZE                     (32 * 1024)
#define configSUPPORT_STATIC_ALLOCATION           0
#define configSUPPORT_DYNAMIC_ALLOCATION          1
#define configAPPLICATION_ALLOCATED_HEAP          0
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP 0
#define configENABLE_HEAP_PROTECTOR               0
#define configHEAP_CLEAR_MEMORY_ON_FREE           0
#define configUSE_MINI_LIST_ITEM                  1
#define configSTACK_DEPTH_TYPE                    size_t
#define configMESSAGE_BUFFER_LENGTH_TYPE          size_t

/* ========================================================================
 * 钩子函数与检测配置
 * - configUSE_IDLE_HOOK: 空闲任务执行时运行空闲钩子函数
 * - configUSE_TICK_HOOK: 每次节拍中断执行后运行节拍钩子函数
 * - configUSE_MALLOC_FAILED_HOOK: 内存申请失败钩子
 * - configUSE_DAEMON_TASK_STARTUP_HOOK: 定时器守护任务启动钩子
 * - configUSE_SB_COMPLETED_CALLBACK: 流/消息缓冲区完成回调
 * - configCHECK_FOR_STACK_OVERFLOW: 栈溢出检测 (0=禁用, 1=简单检测, 2=深度检测)
 * ======================================================================== */
#define configUSE_IDLE_HOOK                0
#define configUSE_TICK_HOOK                0
#define configUSE_MALLOC_FAILED_HOOK       0
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0
#define configUSE_SB_COMPLETED_CALLBACK    0
#define configCHECK_FOR_STACK_OVERFLOW     0

/* ========================================================================
 * 任务相关配置
 * - configMINIMAL_STACK_SIZE: 最小任务栈大小(字)，空闲任务/定时器任务使用
 * - configMAX_TASK_NAME_LEN: 任务名称最大长度(含NULL终止符)
 * - configTASK_NOTIFICATION_ARRAY_ENTRIES: 任务通知数组条目数
 * - configUSE_TASK_NOTIFICATIONS: 启用任务通知功能
 * - configUSE_APPLICATION_TASK_TAG: 启用任务标签功能
 * - configNUM_THREAD_LOCAL_STORAGE_POINTERS: 线程本地存储指针数量
 * - configUSE_NEWLIB_REENTRANT: 为每个任务分配newlib重入结构
 * - configUSE_STATS_FORMATTING_FUNCTIONS: 启用 vTaskList/vTaskGetRunTimeStats 格式化输出
 * - configSTATS_BUFFER_MAX_LENGTH: 统计信息缓冲区最大长度
 * ======================================================================== */
#define configMINIMAL_STACK_SIZE                128
#define configMAX_TASK_NAME_LEN                 16
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_APPLICATION_TASK_TAG          0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0
#define configUSE_NEWLIB_REENTRANT              0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
#define configSTATS_BUFFER_MAX_LENGTH           0xFFFF

/* ========================================================================
 * 软件定时器配置 
 * - configUSE_TIMERS: 启用软件定时器功能
 * - configTIMER_TASK_PRIORITY: 定时器服务任务优先级(建议设为最高)
 * - configTIMER_QUEUE_LENGTH: 定时器命令队列长度
 * - configTIMER_TASK_STACK_DEPTH: 定时器任务栈大小
 * ======================================================================== */
#define configUSE_TIMERS             1
#define configTIMER_TASK_PRIORITY    (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH     10
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE

/* ========================================================================
 * 队列/信号量配置
 * - configUSE_QUEUE_SETS: 启用队列集功能
 * - configQUEUE_REGISTRY_SIZE: 队列注册表大小(调试器使用)
 * - configUSE_RECURSIVE_MUTEXES: 启用递归互斥锁
 * - configUSE_COUNTING_SEMAPHORES: 启用计数型信号量
 * ======================================================================== */
#define configUSE_QUEUE_SETS          0
#define configQUEUE_REGISTRY_SIZE     0
#define configUSE_RECURSIVE_MUTEXES   1
#define configUSE_COUNTING_SEMAPHORES 1

/* ========================================================================
 * 事件组配置
 * - configUSE_EVENT_GROUPS: 启用事件组功能(需包含event_groups.c)
 * ======================================================================== */
#define configUSE_EVENT_GROUPS 1

/* ========================================================================
 * 流缓冲区配置
 * - configUSE_STREAM_BUFFERS: 启用流缓冲区/消息缓冲区功能(需包含stream_buffer.c)
 * ======================================================================== */
#define configUSE_STREAM_BUFFERS 1

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
#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_xResumeFromISR              1
#define INCLUDE_xTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_uxTaskGetStackHighWaterMark 0
#define INCLUDE_xTaskGetIdleTaskHandle      0
#define INCLUDE_eTaskGetState               0
#define INCLUDE_xEventGroupSetBitFromISR    1
#define INCLUDE_xTimerPendFunctionCall      1
#define INCLUDE_xTaskAbortDelay             0
#define INCLUDE_xTaskGetHandle              0
#define INCLUDE_xTaskResumeFromISR          1

/* ========================================================================
 * 兼容性配置
 * - configENABLE_BACKWARD_COMPATIBILITY: 启用旧版API名称映射
 * ======================================================================== */
#define configENABLE_BACKWARD_COMPATIBILITY 1

/* ========================================================================
 * 中断配置
 * - configLIBRARY_LOWEST_INTERRUPT_PRIORITY: 最低优先级(15)
 * - configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY: FreeRTOS可管理的最高优先级(5);
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
 * - configENABLE_MPU: 启用内存保护单元(MPU)
 * - configENABLE_FPU: 启用浮点单元(FPU)
 * - configENABLE_TRUSTZONE: 启用TrustZone支持(ARMv8-M)
 * - configRUN_FREERTOS_SECURE_ONLY: 仅在安全侧运行FreeRTOS
 * - configENABLE_MVE: 启用M-Profile矢量扩展(Cortex-M55/M85)
 * ======================================================================== */
#define configENABLE_MPU               0
#define configENABLE_FPU               0
#define configENABLE_TRUSTZONE         0
#define configRUN_FREERTOS_SECURE_ONLY 0
#define configENABLE_MVE               0

/* ========================================================================
 * MPU 相关配置 (仅当 configENABLE_MPU=1 时有效)
 * - configTOTAL_MPU_REGIONS: MPU区域数量(通常为8或16)
 * - configTEX_S_C_B_FLASH: Flash区域的TEX/S/C/B位配置
 * - configTEX_S_C_B_SRAM: SRAM区域的TEX/S/C/B位配置
 * - configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY: 仅允许内核代码进行特权提升
 * - configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS: 允许非特权任务进入临界区
 * - configUSE_MPU_WRAPPERS_V1: 使用v1版本MPU包装器(0=使用v2)
 * - configPROTECTED_KERNEL_OBJECT_POOL_SIZE: 受保护内核对象池大小(v2 MPU)
 * - configSYSTEM_CALL_STACK_SIZE: 系统调用栈大小(字)(v2 MPU)
 * - configENABLE_ACCESS_CONTROL_LIST: 启用访问控制列表(v2 MPU)
 * ======================================================================== */
#define configTOTAL_MPU_REGIONS                                8
#define configTEX_S_C_B_FLASH                                  0x07UL
#define configTEX_S_C_B_SRAM                                   0x07UL
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY            1
#define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS             0
#define configUSE_MPU_WRAPPERS_V1                              0
#define configPROTECTED_KERNEL_OBJECT_POOL_SIZE                10
#define configSYSTEM_CALL_STACK_SIZE                           128
#define configENABLE_ACCESS_CONTROL_LIST                       0
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0

/* ========================================================================
 * ARMv8-M 安全侧配置
 * - secureconfigMAX_SECURE_CONTEXTS: 可调用安全侧的最大任务数
 * - configKERNEL_PROVIDED_STATIC_MEMORY: 内核提供静态内存分配实现
 * ======================================================================== */
#define secureconfigMAX_SECURE_CONTEXTS     5
#define configKERNEL_PROVIDED_STATIC_MEMORY 1

/* ========================================================================
 * SMP(对称多处理)配置 (多核系统使用)
 * - configNUMBER_OF_CORES: 处理器核心数量(单核时为1或不定义)
 * - configRUN_MULTIPLE_PRIORITIES: 允许不同优先级任务同时运行
 * - configUSE_CORE_AFFINITY: 启用核心亲和性功能
 * - configTASK_DEFAULT_CORE_AFFINITY: 默认核心亲和性掩码
 * - configUSE_TASK_PREEMPTION_DISABLE: 允许单独禁用任务抢占
 * - configUSE_PASSIVE_IDLE_HOOK: 启用被动空闲钩子
 * - configTIMER_SERVICE_TASK_CORE_AFFINITY: 定时器任务核心亲和性
 * ======================================================================== */
/* #define configNUMBER_OF_CORES                  1 */
#define configRUN_MULTIPLE_PRIORITIES          0
#define configUSE_CORE_AFFINITY                0
#define configTASK_DEFAULT_CORE_AFFINITY       tskNO_AFFINITY
#define configUSE_TASK_PREEMPTION_DISABLE      0
#define configUSE_PASSIVE_IDLE_HOOK            0
#define configTIMER_SERVICE_TASK_CORE_AFFINITY tskNO_AFFINITY

/* ========================================================================
 * 调试与运行时统计
 * - configGENERATE_RUN_TIME_STATS: 生成任务运行时间统计(需提供时钟源)
 * - configUSE_TRACE_FACILITY: 启用跟踪功能(任务结构包含额外调试字段)
 * ======================================================================== */
#define configGENERATE_RUN_TIME_STATS 0
#define configUSE_TRACE_FACILITY      0

/* ========================================================================
 * ARMv7-M/ARMv8-M 端口特定配置
 * - configCHECK_HANDLER_INSTALLATION: 验证FreeRTOS中断处理程序安装正确性
 * ======================================================================== */
#define configCHECK_HANDLER_INSTALLATION 1

/* ========================================================================
 * 钩子函数原型声明 
 * ======================================================================== */
/* void vApplicationIdleHook(void); */
/* void vApplicationTickHook(void); */
/* void vApplicationMallocFailedHook(void); */
/* void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName); */
/* void vApplicationDaemonTaskStartupHook(void); */

#endif /* FREERTOS_CONFIG_H */
