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

#if (EK_CORO_MUTEX_ENABLE == 1)
    bool Sem_isMutex; /**< 是否是互斥量的标志位 */
    EK_CoroTCB_t *Mutex_Holder; /**< 互斥量持有者 */
    bool Mutex_isRecursive; /**< 互斥量是否递归 */
    uint16_t Mutex_RecursiveCount; /**< 递归互斥量计数器 */
#if (EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1)
    int8_t Mutex_OriginalPriority; /**< 互斥量持有者的原始优先级 默认为-1 */
#endif /* EK_CORO_MUTEX_PRIORITY_INHERITANCE_ENABLE == 1 */

#endif /* EK_CORO_MUTEX_ENABLE == 1 */

} EK_CoroSem_t;

typedef EK_CoroSem_t *EK_CoroSemHanlder_t; /**< 动态类型信号量句柄 */
typedef EK_CoroSem_t *EK_CoroSemStaticHanlder_t; /**< 静态类型信号量句柄 */

/* ========================= 函数声明区 ========================= */
EK_CoroSem_t *EK_pSemGenericCreate(uint16_t init_count, uint16_t max_count, bool is_mutex, bool is_recursive);
EK_CoroSem_t *EK_pSemGenericCreateStatic(
    EK_CoroSem_t *sem, uint32_t init_count, uint32_t max_count, bool is_mutex, bool is_recursive);
EK_Result_t EK_rSemTake(EK_CoroSemHanlder_t sem, uint32_t timeout);
EK_Result_t EK_rSemGive(EK_CoroSemHanlder_t sem);
EK_Result_t EK_rSemClean(EK_CoroSemHanlder_t sem);
EK_Result_t EK_rSemDelete(EK_CoroSem_t *sem);

/* ========================= ISR版本函数声明 ========================= */
bool EK_bSemGive_FromISR(EK_CoroSemHanlder_t sem, bool *higher_prio_wake);
bool EK_bSemTake_FromISR(EK_CoroSemHanlder_t sem, bool *higher_prio_wake);

/* ========================= 计数信号量宏 ========================= */
/**
 * @brief 创建计数信号量（动态分配）
 * @param init_count 初始计数值
 * @param max_count 最大计数值
 * @return 信号量句柄指针，失败返回NULL
 */
#define EK_pSemCreate(init_count, max_count) EK_pSemGenericCreate(init_count, max_count, false, false)

/**
 * @brief 创建计数信号量（静态分配）
 * @param sem_ptr 信号量结构体指针
 * @param init_count 初始计数值
 * @param max_count 最大计数值
 * @return 信号量句柄指针，失败返回NULL
 */
#define EK_pSemCreateStatic(sem_ptr, init_count, max_count) \
    EK_pSemGenericCreateStatic(sem_ptr, init_count, max_count, false, false)

/* ========================= 二值信号量宏 ========================= */
/**
 * @brief 创建二值信号量（动态分配）
 * @param init_val 初始值（0或非0）
 * @return 信号量句柄指针，失败返回NULL
 */
#define EK_pSemBinaryCreate(init_val) EK_pSemCreate((init_val == 0 ? 0 : 1), 1)

/**
 * @brief 创建二值信号量（静态分配）
 * @param sem_ptr 信号量结构体指针
 * @param init_val 初始值（0或非0）
 * @return 信号量句柄指针，失败返回NULL
 */
#define EK_pSemBinaryCreateStatic(sem_ptr, init_val) EK_pSemCreateStatic(sem_ptr, (init_val == 0 ? 0 : 1), 1)

/**
 * @brief 获取二值信号量
 * @param mutex_ptr 信号量句柄
 * @param timeout 超时时间（ms）
 * @return EK_OK 成功，其他值表示错误
 */
#define EK_rSemBinaryTake(mutex_ptr, timeout) EK_rSemTake(mutex_ptr, timeout)

/**
 * @brief 释放二值信号量
 * @param mutex_ptr 信号量句柄
 * @return EK_OK 成功，其他值表示错误
 */
#define EK_rSemBinaryGive(mutex_ptr) EK_rSemGive(mutex_ptr)

/**
 * @brief 释放二值信号量（中断版本）
 * @param mutex_ptr 信号量句柄
 * @param higher_prio_wake 指向bool变量的指针，用于指示是否需要上下文切换
 * @return true 成功，false 失败
 */
#define EK_bSemBinaryGive_FromISR(mutex_ptr, higher_prio_wake) EK_bSemGive_FromISR(mutex_ptr, higher_prio_wake)

/**
 * @brief 获取二值信号量（中断版本）
 * @param mutex_ptr 信号量句柄
 * @param higher_prio_wake 指向bool变量的指针，用于指示是否需要上下文切换
 * @return true 成功，false 失败
 */
#define EK_bSemBinaryTake_FromISR(mutex_ptr, higher_prio_wake) EK_bSemTake_FromISR(mutex_ptr, higher_prio_wake)

#if (EK_CORO_MUTEX_ENABLE == 1)

/* ========================= 互斥量宏 ========================= */
/**
 * @brief 创建互斥量（动态分配）
 * @return 互斥量句柄指针，失败返回NULL
 */
#define EK_pMutexCreate() EK_pSemGenericCreate(1, 1, true, false)

/**
 * @brief 创建互斥量（静态分配）
 * @param mutex_ptr 互斥量结构体指针
 * @return 互斥量句柄指针，失败返回NULL
 */
#define EK_pMutexCreateStatic(mutex_ptr) EK_pSemGenericCreateStatic(mutex_ptr, 1, 1, true, false)

/**
 * @brief 获取互斥量
 * @param mutex_ptr 互斥量句柄
 * @param timeout 超时时间（ms）
 * @return EK_OK 成功，其他值表示错误
 */
#define EK_rMutexTake(mutex_ptr, timeout) EK_rSemTake(mutex_ptr, timeout)

/**
 * @brief 释放互斥量
 * @param mutex_ptr 互斥量句柄
 * @return EK_OK 成功，其他值表示错误
 */
#define EK_rMutexGive(mutex_ptr) EK_rSemGive(mutex_ptr)

/**
 * @brief 释放互斥量（中断版本）
 * @param mutex_ptr 互斥量句柄
 * @param higher_prio_wake 指向bool变量的指针，用于指示是否需要上下文切换
 * @return true 成功，false 失败
 * @note 中断中不处理优先级继承逻辑
 */
#define EK_bMutexGive_FromISR(mutex_ptr, higher_prio_wake) EK_bSemGive_FromISR(mutex_ptr, higher_prio_wake)

/* ========================= 递归互斥量宏 ========================= */
/**
 * @brief 创建递归互斥量（动态分配）
 * @return 递归互斥量句柄指针，失败返回NULL
 */
#define EK_pMutexRecursiveCreate() EK_pSemGenericCreate(1, 1, true, true)

/**
 * @brief 创建递归互斥量（静态分配）
 * @param mutex_ptr 递归互斥量结构体指针
 * @return 递归互斥量句柄指针，失败返回NULL
 */
#define EK_pMutexRecursiveCreateStatic(mutex_ptr) EK_pSemGenericCreateStatic(mutex_ptr, 1, 1, true, true)

/**
 * @brief 获取递归互斥量
 * @param mutex_ptr 递归互斥量句柄
 * @param timeout 超时时间（ms）
 * @return EK_OK 成功，其他值表示错误
 */
#define EK_rMutexRecursiveTake(mutex_ptr, timeout) EK_rSemTake(mutex_ptr, timeout)

/**
 * @brief 释放递归互斥量
 * @param mutex_ptr 递归互斥量句柄
 * @return EK_OK 成功，其他值表示错误
 */
#define EK_rMutexRecursiveGive(mutex_ptr) EK_rSemGive(mutex_ptr)

/**
 * @brief 释放递归互斥量（中断版本）
 * @param mutex_ptr 递归互斥量句柄
 * @param higher_prio_wake 指向bool变量的指针，用于指示是否需要上下文切换
 * @return true 成功，false 失败
 * @note 中断中不处理优先级继承逻辑，但会处理基本的递归计数
 */
#define EK_bMutexRecursiveGive_FromISR(mutex_ptr, higher_prio_wake) EK_bSemGive_FromISR(mutex_ptr, higher_prio_wake)

#endif /* EK_CORO_MUTEX_ENABLE == 1 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_CORO_SEMAPHORE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROSEMAPHORE_H */