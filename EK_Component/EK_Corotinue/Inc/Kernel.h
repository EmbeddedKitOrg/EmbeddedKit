/**
 * @file Kernel.h
 * @brief EK_Coroutine 内核核心头文件
 * @details
 *  定义了内核所需的所有核心数据结构 (EK_CoroTCB_t, EK_CoroList_t 等)。
 *  定义了用于调度和临界区保护的关键宏。
 *  声明了所有内核全局变量 (例如任务列表、当前任务指针等)。
 *  声明了所有内核函数，包括生命周期、中断处理及底层的链表操作函数。
 *  此文件是内核唯一的内部API入口。
 * @author N1ntyNine99
 * @date 2025-09-22
 * @version 1.5
 */

#ifndef __KERNEL_H
#define __KERNEL_H

#include "EK_CoroMarco.h"
#if (EK_CORO_ENABLE == 1)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ========================= 宏定义 ========================= */

/**
 * @brief 请求一次任务调度。
 * @details
 *  此函数用于触发一次任务调度。它仅设置调度请求标志并触发PendSV异常。
 *  实际的调度逻辑将在PendSV中断处理中执行。
 *  当前正在运行的任务在调用此函数之前，应该已经被放回了某个链表（如就绪链表或阻塞链表）。
 */
#define EK_vKernelYield()                   \
    do                                      \
    {                                       \
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; \
        __DSB();                            \
        __ISB();                            \
    } while (0)

/**
 * @brief 在中断中判断是否需要切换上下文
 * @details
 *  根据传入的 X 来判断是否需要切换上下文，X为true的时候会切换
 */
#define EK_vKernelYield_From_ISR(X) \
    if (X == true) EK_vKernelYield()

/**
 * @brief 获取链表的第一个有效节点。
 * @details
 *  使用哨兵节点设计，快速获取链表的第一个有效节点。
 *  如果链表为空，返回哨兵节点本身。
 * @param list 链表指针
 * @return 第一个有效节点的指针，如果链表为空则返回哨兵节点
 */
#define EK_pKernelListGetFirst(list) ((list)->List_Dummy.CoroNode_Next)

/**
 * @brief 获取链表的最后一个有效节点。
 * @details
 *  使用哨兵节点设计，快速获取链表的最后一个有效节点。
 *  如果链表为空，返回哨兵节点本身。
 * @param list 链表指针
 * @return 最后一个有效节点的指针，如果链表为空则返回哨兵节点
 */
#define EK_pKernelListGetLast(list) ((list)->List_Dummy.CoroNode_Prev)

/**
 * @brief 检查链表是否为空。
 * @details
 *  使用哨兵节点设计，快速检查链表是否为空。
 * @param list 链表指针
 * @return true 表示链表为空，false 表示链表不为空
 */
#define EK_bKernelListIsEmpty(list) ((list)->List_Count == 0)

/**
 * @brief 获取哨兵节点指针。
 * @details
 *  返回链表的哨兵节点指针，用于链表遍历的起始或结束标记。
 * @param list 链表指针
 * @return 哨兵节点的指针
 */
#define EK_pKernelListGetDummy(list) ((EK_CoroListNode_t *)&(list)->List_Dummy)

// 临界区函数声明
void EK_vEnterCritical(void);
void EK_vExitCritical(void);

/**
 * @brief 可嵌套进入临界区
 * 
 */
#define EK_ENTER_CRITICAL() EK_vEnterCritical()

/**
 * @brief 退出临界区
 *
 */
#define EK_EXIT_CRITICAL() EK_vExitCritical()

/**
 * @brief 不使用HAL库生成的SysTick和PendSV和SVC Handler
 * 
 */
#define EK_DISABLE_HAL_HANDLER()       \
    __weak void PendSV_Handler(void);  \
    __weak void SysTick_Handler(void); \
    __weak void SVC_Handler(void)

/* ========================= 数据结构 ========================= */
typedef void (*EK_CoroFunction_t)(void *arg); //协程任务的入口函数指针类型
typedef uint32_t EK_CoroStack_t; // 堆栈元素类型
/**
 * @brief 协程的运行状态枚举.
 */
typedef enum
{
    EK_CORO_READY = 0, /**< 任务已准备就绪，可以被调度执行. */
    EK_CORO_BLOCKED, /**< 任务被阻塞，例如等待延时结束. */
    EK_CORO_RUNNING, /**< 任务正在运行. */
    EK_CORO_SUSPENDED, /**< 任务被挂起，不会被调度器调度. */
} EK_CoroState_t;

/**
 * @brief 协程链表节点结构体.
 * @details 用于将协程任务控制块 (TCB) 链接到不同的状态链表中 (如就绪链表、阻塞链表).
 */
typedef struct EK_CoroListNode_t
{
    struct EK_CoroListNode_t *CoroNode_Next; /**< 指向链表中的下一个节点. */
    struct EK_CoroListNode_t *CoroNode_Prev; /**< 指向链表中的上一个节点. */
    void *CoroNode_Owner; /**< 指向拥有该节点的的拥有者 */
    void *CoroNode_List; /**< 指向该节点所属的链表 (EK_CoroList_t *). */
} EK_CoroListNode_t;

/**
 * @brief 协程链表Mini节点结构体.
 * @details 用于链表中的哨兵节点
 */
typedef struct EK_CoroListMiniNode_t
{
    EK_CoroListNode_t *CoroNode_Next; /**< 指向链表中的下一个节点. */
    EK_CoroListNode_t *CoroNode_Prev; /**< 指向链表中的上一个节点. */
} EK_CoroListMiniNode_t;

/**
 * @brief 协程链表管理结构体.
 * @details 用于管理一组协程，例如就绪链表、阻塞链表等。
 */
typedef struct EK_CoroList_t
{
    EK_CoroListMiniNode_t List_Dummy; /**< 哨兵节点 */
    uint16_t List_Count; /**< 链表中节点的数量. */
} EK_CoroList_t;

/**
 * @brief 事件结果枚举
 * @details 用于表示事件操作的结果状态。
 */
typedef enum
{
    EK_CORO_EVENT_NONE = 0, /**< 默认状态，未记录唤醒来源 */
    EK_CORO_EVENT_PENDING, /**< 挂起状态，有事件等待 */
    EK_CORO_EVENT_OK, /**< 事件成功完成 */
    EK_CORO_EVENT_TIMEOUT, /**< 事件等待超时 */
    EK_CORO_EVENT_DELETED /**< 事件对象被删除 */
} EK_CoroEventResult_t;

/**
 * @brief 协程任务控制块 (Task Control Block).
 * @details 这是内核管理协程的核心数据结构，包含了协程运行所需的所有信息。
 */
#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1)
struct EK_CoroMsg_t; // 前向声明消息结构体
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */

typedef struct EK_CoroTCB_t
{
    EK_CoroStack_t *TCB_StackPointer; /**< 协程的栈顶指针. */
    void *TCB_Arg; /**< 协程入口函数的参数. */
    void *TCB_StackStart; /**< 协程栈的起始(低)地址. */
    EK_Size_t TCB_StackSize; /**< 协程栈的总大小 (以字节为单位). */
    EK_CoroState_t TCB_State; /**< 协程的当前状态. */
    EK_CoroFunction_t TCB_Entry; /**< 协程的入口函数地址. */
    uint8_t TCB_Priority; /**< 协程的优先级 (数值越小，优先级越高). */
    bool TCB_isDynamic; /**< 标记协程是否为动态创建 (用于内存管理). */
    uint32_t TCB_WakeUpTime; /**< 要被唤醒的tick */
    uint32_t TCB_LastWakeUpTime; /**< 上次唤醒的tick，用于delayUntil功能 */
    EK_CoroListNode_t TCB_StateNode; /**< 用于将此TCB链入状态管理链表的节点. */

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1)
    EK_CoroListNode_t TCB_EventNode; /**< 用于将此TCB链入事件管理链表的节点. */
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 */

#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1)
    EK_CoroEventResult_t TCB_EventResult; /**< 最近一次唤醒的原因 */
#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 || EK_CORO_SEMAPHORE_ENABLE == 1 || EK_CORO_TASK_NOTIFY_ENABLE == 1 */

#if (EK_CORO_TASK_NOTIFY_ENABLE == 1)
    EK_CoroTaskNotifyState_t TCB_NotifyState; /**< 任务通知状态位图. */
    uint8_t TCB_NotifyValue[EK_CORO_TASK_NOTIFY_GROUP]; /**< 任务通知状态的数量. */
#endif /* EK_CORO_TASK_NOTIFY_ENABLE == 1 */

#if (EK_HIGH_WATER_MARK_ENABLE == 1)
    void *TCB_StackEnd; /**< 协程栈的结束(高)地址，用于高水位检测. */
    EK_Size_t TCB_StackHighWaterMark; /**< 协程栈的高水位标记 (历史最大使用量). */
#endif /* EK_HIGH_WATER_MARK_ENABLE == 1 */

} EK_CoroTCB_t;

typedef EK_CoroTCB_t *EK_CoroHandler_t; // 动态类型的指针
typedef EK_CoroTCB_t *EK_CoroStaticHandler_t; // 静态类型的指针

/* ========================= 内核状态访问函数 ========================= */
uint32_t EK_uKernelGetTick(void);
EK_CoroList_t *EK_pKernelGetReadyList(uint8_t priority);
EK_CoroList_t *EK_pKernelGetSuspendList(void);
EK_CoroList_t *EK_pKernelGetCurrentBlockList(void);
EK_CoroList_t *EK_pKernelGetNextBlockList(void);
EK_CoroTCB_t *EK_pKernelGetCurrentTCB(void);
EK_CoroStaticHandler_t EK_pKernelGetIdleHandler(void);
EK_CoroTCB_t *EK_pKernelGetDeleteTCB(void);
void EK_vKernelSetDeleteTCB(EK_CoroTCB_t *tcb);

/* ========================= 链表操作函数 ========================= */
// 链表初始化
void EK_vKernelListInit(EK_CoroList_t *list);

// 链表插入函数
EK_Result_t EK_rKernelInsert_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelInsert_Prio(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelInsert_Head(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelInsert_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node);

// 链表删除函数
EK_Result_t EK_rKernelRemove(EK_CoroList_t *list, EK_CoroListNode_t *node);

// 链表移动函数
EK_Result_t EK_rKernelMove_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelMove_Prio(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelMove_Head(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelMove_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node);

/* ========================= 内核核心API函数 ========================= */
EK_Size_t EK_uKernelGetFreeHeap(void);
void EK_vKernelInit(void);
void EK_vKernelStart(void);

/* ========================= 异常处理函数 ========================= */
void SVC_Handler(void);
void PendSV_Handler(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*EK_CORO_ENABLE == 1*/

#endif /* __KERNEL_H */
