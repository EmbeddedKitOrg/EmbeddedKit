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

#include "../../EK_Config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 宏定义 ========================= */
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

/**
 * @brief 让出CPU，请求一次任务调度
 * 
 */
#define EK_KERNEL_YIELD()                   \
    do                                      \
    {                                       \
        SCB->ICSR = SCB_ICSR_PENDSVSET_Msk; \
        __DSB();                            \
        __ISB();                            \
    } while (0)

/* ========================= 数据结构 ========================= */

typedef void (*EK_CoroFunction_t)(void *arg); //协程任务的入口函数指针类型
typedef uint32_t *EK_CoroStack_t; // 堆栈指针类型
/**
 * @brief 协程的运行状态枚举.
 */
typedef enum
{
    EK_CORO_READY = 0, /**< 任务已准备就绪，可以被调度执行. */
    EK_CORO_BLOCKED, /**< 任务被阻塞，例如等待延时结束. */
    EK_CORO_RUNNING, /**< 任务正在运行. */
    EK_CORO_SUSPENDED /**< 任务被挂起，不会被调度器调度. */
} EK_CoroState_t;

/**
 * @brief 协程链表节点结构体.
 * @details 用于将协程任务控制块 (TCB) 链接到不同的状态链表中 (如就绪链表、阻塞链表).
 */
typedef struct EK_CoroListNode_t
{
    struct EK_CoroListNode_t *CoroNode_Next; /**< 指向链表中的下一个节点. */
    void *CoroNode_OwnerTCB; /**< 指向拥有该节点的协程TCB (EK_CoroTCB_t *). */
    void *CoroNode_OwnerList; /**< 指向该节点所属的链表 (EK_CoroList_t *). */
} EK_CoroListNode_t;

/**
 * @brief 协程任务控制块 (Task Control Block).
 * @details 这是内核管理协程的核心数据结构，包含了协程运行所需的所有信息。
 */
typedef struct EK_CoroTCB_t
{
    EK_CoroStack_t *TCB_SP; /**< 协程的栈顶指针 (Stack Pointer). */
    void *TCB_Arg; /**< 协程入口函数的参数. */
    void *TCB_StackBase; /**< 协程栈的起始(低)地址. */
    uint16_t TCB_Priority; /**< 协程的优先级 (数值越小，优先级越高). */
    bool TCB_isDynamic; /**< 标记协程是否为动态创建 (用于内存管理). */
    uint32_t TCB_DelayTicks; /**< 协程需要延时的节拍数. */
    EK_Size_t TCB_StackSize; /**< 协程栈的总大小 (以字节为单位). */
    EK_CoroState_t TCB_State; /**< 协程的当前状态. */
    EK_CoroFunction_t TCB_Entry; /**< 协程的入口函数地址. */
    EK_CoroListNode_t TCB_Node; /**< 用于将此TCB链入管理链表的节点. */
} EK_CoroTCB_t;

typedef EK_CoroTCB_t *EK_CoroHandler_t; // 动态类型的指针
typedef EK_CoroTCB_t *EK_CoroStaticHandler_t; // 静态类型的指针

/**
 * @brief 协程链表管理结构体.
 * @details 用于管理一组协程，例如就绪链表、阻塞链表等。
 */
typedef struct EK_CoroList_t
{
    EK_CoroListNode_t *List_Head; /**< 指向链表的头节点. */
    EK_Size_t List_Count; /**< 链表中节点的数量. */
} EK_CoroList_t;

/* ========================= 内核全局变量声明 ========================= */
extern EK_CoroList_t Kernel_ReadyList;
extern EK_CoroList_t Kernel_BlockList;
extern EK_CoroList_t Kernel_SuspendList;
extern EK_CoroTCB_t *Kernel_CurrentTCB;
extern EK_CoroTCB_t *Kernel_NextTCB;
extern EK_CoroTCB_t *Kernel_DeleteTCB;
extern EK_CoroStaticHandler_t Kernel_IdleTaskHandler;

/* ========================= 内核核心API函数 ========================= */
EK_Result_t r_insert_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb);
EK_Result_t r_remove_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb);
EK_Result_t r_move_node(EK_CoroList_t *list, EK_CoroTCB_t *tcb);
void EK_vKernelInit(void);
void EK_vKernelStart(void);
void EK_vTickHandler(void);
void EK_vPendSVHandler(void);

#ifdef __cplusplus
}
#endif

#endif
