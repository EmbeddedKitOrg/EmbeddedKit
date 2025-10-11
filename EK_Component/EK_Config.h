/**
 * @file EK_Config.h
 * @brief EmbeddedKit 配置文件
 * @details 包含系统配置参数和编译选项
 * @author N1ntyNine99
 * @version 1.0
 * @date 2025-01-09
 */

#ifndef __EK_CONFIG_H
#define __EK_CONFIG_H

#include "EK_Common.h"

/* ================================ 内存管理配置 ================================ */
/**
 * @brief 内存管理配置选项
 *
 * EK_MALLOC(X)        - 内存分配宏定义，函数签名要求是 void* xxx(EK_Size_t)
 * EK_FREE(X)          - 内存释放宏定义，函数入参要求是 void*
 * MEMPOOL_SIZE        - 内存池总大小 (字节)，可根据系统资源调整，建议至少1KB
 * MEMPOOL_ALIGNMENT   - 内存对齐大小 (字节)，必须是2的幂次，通常为4或8字节
 */

extern void *EK_pMemPool_Malloc(EK_Size_t size);
extern bool EK_bMemPool_Free(void *ptr);

#define EK_MALLOC(X) EK_pMemPool_Malloc(X)
#define EK_FREE(X)               \
    do                           \
    {                            \
        if (X != NULL)           \
        {                        \
            EK_bMemPool_Free(X); \
            X = NULL;            \
        }                        \
    } while (0);
#define MEMPOOL_SIZE      (10240)
#define MEMPOOL_ALIGNMENT (8)

/* ================================ 数据结构配置 ================================ */
/**
 * @brief 数据结构配置选项
 *
 * LIST_RECURSION_SORT - 链表相关，是否启用递归排序 0:不启用 1:启用
 *                      启用递归排序可以提高排序效率，但可能导致栈溢出
 *                      禁用递归排序可以避免栈溢出，但可能导致排序效率下降
 */

#define LIST_RECURSION_SORT (1)

/* ================================ 串口通信配置 ================================ */
/**
 * @brief 串口通信配置选项
 *
 * SERIAL_TX_BUFFER     - 串口发送缓冲区大小 (字节)
 * SERIAL_MAX_SEND_SIZE - 每次轮询发送的最大字节数，限制单次发送数据量，确保消息顺序和实时性
 * SERIAL_FULL_STRATEGY - 队列满时的处理策略，0: 直接丢弃新数据; 1: 丢弃最老的数据腾出空间
 * SERIAL_OVER_TIME     - 串口轮询发送的默认间隔时间（单位：ms），当串口队列中有数据时，每隔这么久发送一次数据
 */

#define SERIAL_TX_BUFFER     (256)
#define SERIAL_MAX_SEND_SIZE (128)
#define SERIAL_FULL_STRATEGY (1)
#define SERIAL_OVER_TIME     (20)

/* ================================ 调度器配置 ================================ */
/**
 * @brief 调度器配置选项
 *
 * EK_CORO_ENABLE      - 启动协程调度器 1:ENABLE 0:DISABLE
 *                       当该选项为1 即启动协程调度器，此时普通任务调度器不会被编译
 *                       当该选项为0 即关闭协程调度器，此时会自动启用普通任务调度器
 * EK_NORMAL_SCHEDULER - 普通调度器开关，由 EK_CORO_ENABLE 自动控制，不要手动修改
 */

#define EK_CORO_ENABLE (1)

/** @warning :此处的宏禁止修改！*/
#if (EK_CORO_ENABLE == 1)
#define EK_NORMAL_SCHEDULER (0)
#else
#define EK_NORMAL_SCHEDULER (1)
#endif /* EK_CORO_ENABLE */

/* ================================ 协程系统配置 ================================ */
/**
 * @brief 协程系统配置选项 (仅在 EK_CORO_ENABLE=1 时生效)
 *
 * EK_CORO_SYSTEM_FREQ           - 系统时钟频率 (HZ)
 * EK_CORO_TICK_RATE_HZ          - SysTick中断周期 (HZ)
 * EK_CORO_PRIORITY_GROUPS       - 协程优先级组数目
 * EK_CORO_IDLE_TASK_STACK_SIZE  - 协程空闲任务堆栈大小 (字节)
 */

#if (EK_CORO_ENABLE == 1)

#define EK_CORO_SYSTEM_FREQ          (168000000)
#define EK_CORO_TICK_RATE_HZ         (1000)
#define EK_CORO_PRIORITY_GROUPS      (16)
#define EK_CORO_IDLE_TASK_STACK_SIZE (512)

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
 * EK_CORO_TASK_NOTIFY_ENABLE         - 是否使能任务通知， 1:使能 0:失能。开启后可以配置 EK_CORO_TASK_NOTIFY_GROUP 的值来配置
 *                                    通知组:每一个组都代表一个通道建议:8、16、32                                     
 * 
 * EK_CORO_MESSAGE_QUEUE_ENABLE       - 是否使能消息队列，1:使能 0:失能
 * EK_CORO_SEMAPHORE_ENABLE           - 是否使能信号量，1:使能 0:失能
 * EK_CORO_MUTEX_ENABLE               - 是否使能互斥锁，仅在 EK_CORO_SEMAPHORE_ENABLE 为1时有效，1:使能 0:失能
 * EK_CORO_MUTEX_RECURSIVE_ENABLE         - 是否使能递归互斥量，仅在 EK_CORO_MUTEX_ENABLE 有效时才有意义
 * EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE - 是否开启优先级继承，仅在 EK_CORO_SEMAPHORE_ENABLE 为1时有效，1:使能 0:失能
 */

#define EK_CORO_IDLE_HOOK_ENABLE                  (0)
#define EK_CORO_STACK_OVERFLOW_CHECK_ENABLE       (0)
#define EK_HIGH_WATER_MARK_ENABLE                 (0)
#define EK_CORO_TASK_NOTIFY_ENABLE                (0)
#define EK_CORO_MESSAGE_QUEUE_ENABLE              (0)
#define EK_CORO_SEMAPHORE_ENABLE                  (0)
#define EK_CORO_MUTEX_ENABLE                      (0)
#define EK_CORO_MUTEX_RECURSIVE_ENABLE            (0)
#define EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE (0)

#if (EK_CORO_TASK_NOTIFY_ENABLE == 1)
#define EK_CORO_TASK_NOTIFY_GROUP (8) // 此处可以配置任务通知的通知组数目
#endif /* EK_CORO_TASK_NOTIFY_ENABLE */

#endif /* EK_CORO_ENABLE */

#endif /* __EK_CONFIG_H */
