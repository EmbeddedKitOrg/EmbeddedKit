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

#include "../EK_CoroConfig.h"
#if (EK_CORO_ENABLE == 1)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ========================= 宏定义 ========================= */
/**
 * @brief 优先级相关变量
 * @details EK_CORO_PRIORITY_MOUNT:优先级数目
 *          EK_CORO_MAX_PRIORITY_NBR:最高优先级对应的值
 *          EK_BitMap_t:位图类型
 */
#if (EK_CORO_PRIORITY_GROUPS <= 8)
#define EK_CORO_PRIORITY_MOUNT   (8)
#define EK_CORO_MAX_PRIORITY_NBR (0x80UL)
typedef uint8_t EK_BitMap_t;
#elif (EK_CORO_PRIORITY_GROUPS <= 16)
#define EK_CORO_PRIORITY_MOUNT   (16)
#define EK_CORO_MAX_PRIORITY_NBR (0x8000UL)
typedef uint16_t EK_BitMap_t;
#else
#define EK_CORO_PRIORITY_MOUNT   (32)
#define EK_CORO_MAX_PRIORITY_NBR (0x80000000UL)
typedef uint32_t EK_BitMap_t;
#endif /* EK_CORO_PRIORITY_GROUPS selection */

/**
 * @brief 最大阻塞时间，设置这个代表一直阻塞
 * 
 */
#define EK_MAX_DELAY (UINT32_MAX)

/**
 * @brief 进入临界区宏 (支持嵌套, CMSIS版)
 */
#define EK_ENTER_CRITICAL()                    \
    uint32_t primask_status = __get_PRIMASK(); \
    __disable_irq();

/**
 * @brief 退出临界区宏 (支持嵌套, CMSIS版)
 */
#define EK_EXIT_CRITICAL() __set_PRIMASK(primask_status);

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
    void *CoroNode_Owner; /**< 指向拥有该节点的的拥有者 */
    void *CoroNode_List; /**< 指向该节点所属的链表 (EK_CoroList_t *). */
} EK_CoroListNode_t;

/**
 * @brief 协程链表管理结构体.
 * @details 用于管理一组协程，例如就绪链表、阻塞链表等。
 */
typedef struct EK_CoroList_t
{
    EK_CoroListNode_t *List_Head; /**< 指向链表的头节点. */
    EK_CoroListNode_t *List_Tail; /**< 指向链表的尾节点. */
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
#if (EK_CORO_USE_MESSAGE_QUEUE == 1)
struct EK_CoroMsg_t; // 前向声明消息结构体
#endif /* EK_CORO_USE_MESSAGE_QUEUE == 1 */

typedef struct EK_CoroTCB_t
{
    EK_CoroStack_t *TCB_SP; /**< 协程的栈顶指针 (Stack Pointer). */
    void *TCB_Arg; /**< 协程入口函数的参数. */
    void *TCB_StackBase; /**< 协程栈的起始(低)地址. */
    uint16_t TCB_Priority; /**< 协程的优先级 (数值越小，优先级越高). */
    bool TCB_isDynamic; /**< 标记协程是否为动态创建 (用于内存管理). */
    uint32_t TCB_WakeUpTime; /**< 要被唤醒的tick */
    EK_Size_t TCB_StackSize; /**< 协程栈的总大小 (以字节为单位). */
    EK_CoroState_t TCB_State; /**< 协程的当前状态. */
    EK_CoroFunction_t TCB_Entry; /**< 协程的入口函数地址. */
    EK_CoroListNode_t TCB_StateNode; /**< 用于将此TCB链入状态管理链表的节点. */
    EK_CoroListNode_t TCB_EventNode; /**< 用于将此TCB链入事件管理链表的节点. */

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)
    EK_CoroEventResult_t TCB_EventResult; /**< 最近一次唤醒的原因 */
    void *TCB_MsgData; /**< 用于消息队列，指向等待任务的数据缓冲区 */
#endif /* EK_CORO_USE_MESSAGE_QUEUE == 1 */
} EK_CoroTCB_t;

typedef EK_CoroTCB_t *EK_CoroHandler_t; // 动态类型的指针
typedef EK_CoroTCB_t *EK_CoroStaticHandler_t; // 静态类型的指针

/* ========================= 内核全局变量声明 ========================= */
extern uint32_t EK_CoroKernelTick;
extern EK_CoroList_t EK_CoroKernelReadyList[EK_CORO_PRIORITY_GROUPS];
extern EK_CoroList_t EK_CoroKernelBlockList;
extern EK_CoroList_t EK_CoroKernelSuspendList;
extern EK_CoroTCB_t *EK_CoroKernelCurrentTCB;
extern EK_CoroTCB_t *EK_CoroKernelDeleteTCB;
extern EK_CoroStaticHandler_t EK_CoroKernelIdleHandler;

/* ========================= 内核核心API函数 ========================= */
void EK_vKernelYield(void);
EK_Result_t EK_rKernelInsert_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelInsert_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelRemove(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelMove_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelMove_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node);
EK_Result_t EK_rKernelMove_Prio(EK_CoroList_t *list, EK_CoroListNode_t *node);
void EK_vKernelInit(void);
void EK_vKernelStart(void);
void EK_vTickHandler(void);
void EK_vKernelPendSV_Handler(void);

/* ========================= PendSV ========================= */
void PendSV_Handler(void) __naked;
#define EK_vPendSVHandler() __ASM volatile("b EK_vKernelPendSV_Handler")

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*EK_CORO_ENABLE == 1*/

#endif /* __KERNEL_H */
