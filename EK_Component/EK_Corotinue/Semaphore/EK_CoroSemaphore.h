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

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_SEMAPHORE_ENABLE == 1)

/* ========================= 辅助宏 ========================= */
/**
 * @brief 信号量状态查询宏
 * @details 提供轻量级的信号量状态查询功能
 */
#define EK_uSemGetCount(sem_handler)    ((sem_handler)->Sem_Count)
#define EK_uSemGetFree(sem_handler)     ((sem_handler)->Sem_MaxCount - (sem_handler)->Sem_Count)
#define EK_bSemIsFull(sem_handler)      ((sem_handler)->Sem_Count >= (sem_handler)->Sem_MaxCount)
#define EK_bSemIsEmpty(sem_handler)     ((sem_handler)->Sem_Count == 0)
#define EK_uSemGetCapacity(sem_handler) ((sem_handler)->Sem_MaxCount)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ========================= 数据结构 ========================= */

/**
 * @brief 协程信号量结构体
 * @details 用于管理协程间的信号量同步机制。
 *          采用轻量级计数器模式，用简单的计数变量替代复杂的队列结构。
 *          支持动态和静态创建，大幅减少内存占用和提升操作效率。
 */
typedef struct EK_CoroSem_t
{
    uint32_t Sem_Count; /**< 当前可用信号量数量 */
    uint32_t Sem_MaxCount; /**< 最大信号量数量 */
    EK_CoroList_t Sem_WaitList; /**< 等待获取信号量的协程列表 */
    bool Sem_isDynamic; /**< 是否来源于动态创建的标志位 */
} EK_CoroSem_t;

typedef EK_CoroSem_t *EK_CoroSemHanlder_t; /**< 动态类型信号量句柄 */
typedef EK_CoroSem_t *EK_CoroSemStaticHanlder_t; /**< 静态类型信号量句柄 */

/* ========================= 函数声明区 ========================= */
EK_CoroSemHanlder_t EK_pSemCreate(uint16_t init_count, uint16_t max_count);
EK_CoroSemStaticHanlder_t EK_pSemCreateStatic(EK_CoroSem_t *sem, uint32_t init_count, uint32_t max_count);
EK_Result_t EK_rSemTake(EK_CoroSemHanlder_t sem, uint32_t timeout);
EK_Result_t EK_rSemGive(EK_CoroSemHanlder_t sem);
EK_Result_t EK_rSemClean(EK_CoroSemHanlder_t sem);
EK_Result_t EK_rSemDelete(EK_CoroSem_t *sem);

// 二值信号量
#define EK_pSemBinaryCreate(init_val) EK_pSemCreate((init_val == 0 ? 0 : 1), 1)
#define EK_pSemBinaryCreateStatic(sem_ptr, buffer, init_val) \
    EK_pSemCreateStatic(sem_ptr, buffer, (init_val == 0 ? 0 : 1), 1)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_CORO_SEMAPHORE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROSEMAPHORE_H */