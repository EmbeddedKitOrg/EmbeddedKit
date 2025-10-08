/**
 * @file EK_CoroSemaphore.h
 * @brief 协程信号量模块头文件
 * @details 实现基于协程的信号量同步机制，支持动态和静态信号量创建。
 *          采用队列令牌机制实现信号量计数管理，复用现有的队列系统。
 *          支持任务阻塞与唤醒机制，提供超时控制和FIFO公平性保证。
 *          使用双节点机制实现状态-事件分离，与协程调度器完美集成。
 * @author N1ntyNine99
 * @date 2025-10-07
 * @version v1.0
 */

#ifndef __EK_COROSEMAPHORE_H
#define __EK_COROSEMAPHORE_H

#include "../Kernel/Kernel.h"
#include "../../DataStruct/Queue/EK_Queue.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_SEMAPHORE_ENABLE == 1)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ========================= 数据结构 ========================= */

/**
 * @brief 协程信号量结构体
 * @details 用于管理协程间的信号量同步机制。
 *          采用队列令牌模式，将信号量计数值映射为队列中的令牌数量。
 *          支持动态和静态创建，使用union内嵌队列结构体避免动态分配。
 */
typedef struct EK_CoroSem_t
{
    union
    {
        EK_Queue_t *Sem_Queue;       /**< 动态创建时的队列指针，存储信号量令牌 */
        EK_Queue_t Sem_QueueStatic; /**< 静态创建时的内嵌队列结构体 */
    };

    uint16_t Sem_MaxCount;         /**< 信号量的最大计数值（最大令牌数） */
    EK_CoroList_t Sem_WaitList;    /**< 等待获取信号量的协程列表 */
    bool Sem_isDynamic;            /**< 是否来源于动态创建的标志位 */
} EK_CoroSem_t;

typedef EK_CoroSem_t *EK_CoroSemHanlder_t;      /**< 动态类型信号量句柄 */
typedef EK_CoroSem_t *EK_CoroSemStaticHanlder_t; /**< 静态类型信号量句柄 */

/* ========================= 函数声明区 ========================= */

/**
 * @brief 动态创建一个信号量
 * @details 使用动态内存分配创建信号量，初始化指定数量的令牌。
 *          适用于运行时动态创建同步对象的场景。
 *
 * @param init_count 初始令牌数量（信号量初始计数值）
 * @param max_count 最大令牌数量（信号量最大计数值）
 *
 * @return EK_CoroSemHanlder_t 成功时返回信号量句柄，失败返回NULL
 */
EK_CoroSemHanlder_t EK_pSemCreate(uint16_t init_count, uint16_t max_count);

/**
 * @brief 静态创建一个信号量
 * @details 使用用户提供的缓冲区创建信号量，完全避免动态内存分配。
 *          适用于资源受限的嵌入式环境和确定性内存使用场景。
 *
 * @param sem 指向静态信号量结构体的指针
 * @param buffer 指向用于令牌存储的静态缓冲区
 * @param init_count 初始令牌数量（信号量初始计数值）
 * @param max_count 最大令牌数量（信号量最大计数值）
 *
 * @return EK_CoroSemStaticHanlder_t 成功时返回信号量句柄，失败返回NULL
 */
EK_CoroSemStaticHanlder_t EK_pSemCreateStatic(EK_CoroSem_t *sem, void *buffer,
                                             uint32_t init_count, uint32_t max_count);

/**
 * @brief 获取信号量（P操作）
 * @details 尝试从信号量获取一个令牌。如果当前有可用令牌，立即返回成功；
 *          如果没有可用令牌，根据超时设置选择阻塞等待或立即返回失败。
 *          支持FIFO公平性，按照请求顺序唤醒等待协程。
 *
 * @param sem 信号量句柄
 * @param timeout 等待超时时间（tick数），0表示不等待，EK_MAX_DELAY表示永久等待
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功获取信号量
 *         - EK_EMPTY: 无可用令牌且不等待
 *         - EK_TIMEOUT: 等待超时
 *         - EK_NULL_POINTER: 参数无效
 */
EK_Result_t EK_rSemTake(EK_CoroSemHanlder_t sem, uint32_t timeout);

/**
 * @brief 释放信号量（V操作）
 * @details 向信号量释放一个令牌。如果有协程正在等待获取信号量，
 *          直接唤醒等待时间最长的协程；否则将令牌放回队列中。
 *          支持优先级调度和FIFO公平性保证。
 *
 * @param sem 信号量句柄
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功释放信号量
 *         - EK_NULL_POINTER: 参数无效
 *         - EK_FULL: 信号量已满（理论上不应发生）
 */
EK_Result_t EK_rSemGive(EK_CoroSemHanlder_t sem);

/* ========================= 使用示例 ========================= */
/**
 * @brief 信号量使用示例
 * @details 展示如何正确使用信号量进行协程同步
 *
 * @code
 * // 定义静态信号量和缓冲区
 * #define SEM_MAX_COUNT 5
 * static EK_CoroSem_t g_static_semaphore;
 * static uint8_t g_sem_buffer[SEM_MAX_COUNT];
 *
 * void producer_task(void *arg)
 * {
 *     // 静态创建信号量
 *     EK_CoroSemStaticHanlder_t sem = EK_pSemCreateStatic(
 *         &g_static_semaphore, g_sem_buffer, 0, SEM_MAX_COUNT);
 *
 *     while (1) {
 *         // 生产资源
 *         produce_resource();
 *
 *         // 释放信号量
 *         EK_rSemGive(sem);
 *
 *         EK_vCoroDelay(10);
 *     }
 * }
 *
 * void consumer_task(void *arg)
 * {
 *     EK_CoroSemStaticHanlder_t sem = EK_pSemCreateStatic(
 *         &g_static_semaphore, g_sem_buffer, 0, SEM_MAX_COUNT);
 *
 *     while (1) {
 *         // 等待信号量，最多等待100ms
 *         if (EK_rSemTake(sem, 100) == EK_OK) {
 *             // 消费资源
 *             consume_resource();
 *         } else {
 *             // 等待超时处理
 *             handle_timeout();
 *         }
 *
 *         EK_vCoroDelay(5);
 *     }
 * }
 * @endcode
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_CORO_SEMAPHORE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROSEMAPHORE_H */