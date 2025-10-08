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
        EK_Queue_t *Sem_Queue; /**< 动态创建时的队列指针，存储信号量令牌 */
        EK_Queue_t Sem_QueueStatic; /**< 静态创建时的内嵌队列结构体 */
    };

    EK_CoroList_t Sem_WaitList; /**< 等待获取信号量的协程列表 */
    bool Sem_isDynamic; /**< 是否来源于动态创建的标志位 */
} EK_CoroSem_t;

typedef EK_CoroSem_t *EK_CoroSemHanlder_t; /**< 动态类型信号量句柄 */
typedef EK_CoroSem_t *EK_CoroSemStaticHanlder_t; /**< 静态类型信号量句柄 */

/* ========================= 函数声明区 ========================= */
EK_CoroSemHanlder_t EK_pSemCreate(uint16_t init_count, uint16_t max_count);
EK_CoroSemStaticHanlder_t EK_pSemCreateStatic(EK_CoroSem_t *sem, void *buffer, uint32_t init_count, uint32_t max_count);
EK_Result_t EK_rSemTake(EK_CoroSemHanlder_t sem, uint32_t timeout);
EK_Result_t EK_rSemGive(EK_CoroSemHanlder_t sem);

// 二值信号量
#define EK_pSemBinaryCreate(__init_val__) EK_pSemCreate((__init_val__ == 0 ? 0 : 1), 1)
#define EK_pSemBinaryCreateStatic(__sem_ptr__, __buffer_ptr__, __init_val__) \
    EK_pSemCreateStatic(__sem_ptr__, __buffer__, (__init_val__ == 0 ? 0 : 1), 1)

/* ========================= 信号量状态查询函数 ========================= */
EK_Size_t EK_uSemGetCount(EK_CoroSemHanlder_t sem);
EK_Size_t EK_uSemGetFree(EK_CoroSemHanlder_t sem);
EK_Size_t EK_uSemGetCapacity(EK_CoroSemHanlder_t sem);
bool EK_bSemIsFull(EK_CoroSemHanlder_t sem);
bool EK_bSemIsEmpty(EK_CoroSemHanlder_t sem);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_CORO_SEMAPHORE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROSEMAPHORE_H */