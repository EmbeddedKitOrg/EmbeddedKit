/**
 * @file EK_CoroMessage.c
 * @brief 协程消息队列模块实现文件
 * @details 实现协程任务间的消息传递机制，包含动态和静态消息队列的创建、删除、
 *          发送和接收功能。支持任务阻塞与唤醒机制，提供汇合操作和超时控制。
 *          使用双链表管理等待队列，实现高效的任务调度和资源管理。
 * @author N1ntyNine99
 * @date 2025-09-30
 * @version v1.0
 */

/* ========================= 头文件包含区 ========================= */
#include "EK_CoroMessage.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1)

#include "../Task/EK_CoroTask.h"
/* ========================= 内部辅助宏 ========================= */
/**
 * @brief 安全获取消息队列的队列指针
 * @details 根据创建类型返回正确的队列指针
 */
#define EK_MSG_GET_QUEUE(msg) ((msg)->Msg_isDynamic ? (msg)->Msg_Queue : &(msg)->Msg_QueueStatic)

/* ========================= 内部函数 ========================= */
/**
 * @brief 将当前任务阻塞，并根据是发送还是接收操作，将其放入相应的等待列表。
 * 
 * @param msg 指向消息队列的指针。
 * @param tcb 指向要阻塞的任务控制块（TCB）的指针。
 * @param timeout 阻塞的超时时间。如果为 EK_MAX_DELAY，则永久阻塞。
 * @param isRecv 一个布尔值，如果任务因接收而阻塞，则为 true；如果因发送而阻塞，则为 false。
 */
static inline void v_msg_delay(EK_CoroMsg_t *msg, EK_CoroTCB_t *tcb, uint32_t timeout, bool isRecv)
{
    if (msg == NULL || tcb == NULL || timeout == 0) return;

    EK_ENTER_CRITICAL();

    // 原子化操作：将任务状态设置和事件节点添加在同一临界区内完成
    // 将TCB的状态设置为阻塞态
    tcb->TCB_State = EK_CORO_BLOCKED;

    // 根据标志位插入事件链表
    if (isRecv == false) EK_rKernelMove_Prio(&msg->Msg_SendWaitList, &tcb->TCB_EventNode);
    else EK_rKernelMove_Prio(&msg->Msg_RecvWaitList, &tcb->TCB_EventNode);

    EK_EXIT_CRITICAL();

    // 进入阻塞的延时
    EK_vCoroDelay(timeout);
}

/* ========================= 公开API ========================= */
/**
 * @brief 使用动态内存创建一个消息队列。
 * @details 此函数会动态分配消息队列控制块和用于存储消息的缓冲区。
 * 
 * @param item_size 队列中单个消息项的大小（以字节为单位）。
 * @param item_amount 队列中单个消息项的个数。
 * 
 * @return EK_CoroMsgHanler_t 成功时返回消息队列的句柄，如果内存分配失败则返回 NULL。
 */
EK_CoroMsgHanler_t EK_pMsgCreate(EK_Size_t item_size, EK_Size_t item_amount)
{
    if (item_size == 0 || item_amount == 0) return NULL;

    // 为结构体分配内存
    EK_CoroMsg_t *msg = (EK_CoroMsg_t *)EK_CORO_MALLOC(sizeof(EK_CoroMsg_t));
    if (msg == NULL) return NULL;

    // 为消息队列分配内存（动态创建时使用指针成员）
    EK_Queue_t *dynamic_queue = EK_pQueueCreate(item_size * item_amount);
    if (dynamic_queue == NULL)
    {
        EK_CORO_FREE(msg);
        return NULL;
    }
    msg->Msg_Queue = dynamic_queue; // 设置指针成员

    msg->Msg_ItemSize = item_size; // 每条消息的字节大小
    msg->Msg_ItemCapacity = item_amount; // 计算最大可用的消息数目

    // 初始化等待发送链表
    msg->Msg_SendWaitList.List_Count = 0;
    msg->Msg_SendWaitList.List_Head = NULL;
    msg->Msg_SendWaitList.List_Tail = NULL;

    // 初始化等待接收链表
    msg->Msg_RecvWaitList.List_Count = 0;
    msg->Msg_RecvWaitList.List_Head = NULL;
    msg->Msg_RecvWaitList.List_Tail = NULL;

    msg->Msg_isDynamic = true; // 来源于动态创建

    return (EK_CoroMsgHanler_t)msg;
}

/**
 * @brief 使用静态分配的内存来初始化一个消息队列。
 * @details 此函数使用用户提供的缓冲区和消息队列控制块进行初始化，完全避免了动态内存分配。
 *          使用内嵌的队列结构体，确保真正的静态创建。
 *
 * @param msg 指向静态消息队列结构体 `EK_CoroMsg_t` 的指针。
 * @param buffer 指向用于消息存储的静态缓冲区的指针。
 * @param item_size 队列中单个消息项的大小（以字节为单位）。
 * @param item_amount 队列中单个消息项的个数。
 *
 * @return EK_CoroMsgStaticHanler_t 成功时返回消息队列的句柄，如果参数无效则返回 NULL。
 */
EK_CoroMsgStaticHanler_t
EK_pMsgCreateStatic(EK_CoroMsg_t *msg, void *buffer, EK_Size_t item_size, EK_Size_t item_amount)
{
    if (msg == NULL || buffer == NULL) return NULL;
    if (item_size == 0 || item_amount == 0) return NULL;

    // 直接使用内嵌的队列结构体，完全避免动态分配
    EK_Queue_t *queue = &msg->Msg_QueueStatic;

    // 创建静态队列，完全使用静态内存
    EK_Result_t op_res = EK_rQueueCreateStatic(queue, buffer, item_amount * item_size);
    if (op_res != EK_OK)
    {
        return NULL; // 静态创建失败，无需释放内存
    }

    // 设置消息队列属性
    msg->Msg_Queue = queue; // 指向内嵌的队列结构体
    msg->Msg_ItemSize = item_size; // 每条消息的字节大小
    msg->Msg_ItemCapacity = item_amount; // 计算最大可用的消息数目

    // 初始化等待发送链表
    msg->Msg_SendWaitList.List_Count = 0;
    msg->Msg_SendWaitList.List_Head = NULL;
    msg->Msg_SendWaitList.List_Tail = NULL;

    // 初始化等待接收链表
    msg->Msg_RecvWaitList.List_Count = 0;
    msg->Msg_RecvWaitList.List_Head = NULL;
    msg->Msg_RecvWaitList.List_Tail = NULL;

    msg->Msg_isDynamic = false; // 来源于静态创建

    return (EK_CoroMsgStaticHanler_t)msg;
}

/**
 * @brief 删除一个消息队列。
 * @details 在删除队列之前，此函数会唤醒所有等待该队列的任务（包括发送和接收），
 *          并将它们移回就绪列表。同时，它会将消息队列本身从内核的等待列表中移除。
 *          对于动态创建的队列，它会释放其所有分配的内存。
 *          静态创建的队列不会被释放内存，但等待其上的任务仍会被唤醒。
 * 
 * @param msg 指向要删除的消息队列 `EK_CoroMsg_t` 的指针。
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功删除。
 *         - EK_NULL_POINTER: 如果传入的 msg 指针为空。
 */
EK_Result_t EK_rMsgDelete(EK_CoroMsg_t *msg)
{
    if (msg == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    // 唤醒所有等待发送的任务
    while (msg->Msg_SendWaitList.List_Count > 0)
    {
        EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)msg->Msg_SendWaitList.List_Head->CoroNode_Owner;

        // 从事件等待列表中移除
        EK_rKernelRemove(&msg->Msg_SendWaitList, &tcb->TCB_EventNode);

        // 设置事件结果为“已删除”
        tcb->TCB_EventResult = EK_CORO_EVENT_DELETED;

        // 将任务移至就绪列表 (EK_rKernelMove_Tail会处理从阻塞列表的移除)
        tcb->TCB_State = EK_CORO_READY;
        EK_rKernelMove_Tail(EK_pKernelGetReadyList(tcb->TCB_Priority), &tcb->TCB_StateNode);
    }

    // 唤醒所有等待接收的任务
    while (msg->Msg_RecvWaitList.List_Count > 0)
    {
        EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)msg->Msg_RecvWaitList.List_Head->CoroNode_Owner;

        // 从事件等待列表中移除
        EK_rKernelRemove(&msg->Msg_RecvWaitList, &tcb->TCB_EventNode);

        // 设置事件结果为“已删除”
        tcb->TCB_EventResult = EK_CORO_EVENT_DELETED;

        // 将任务移至就绪列表
        tcb->TCB_State = EK_CORO_READY;
        EK_rKernelMove_Tail(EK_pKernelGetReadyList(tcb->TCB_Priority), &tcb->TCB_StateNode);
    }

    // 释放队列结构体内存
    if (msg->Msg_isDynamic)
    {
        // 动态创建：释放底层队列和消息队列控制块
        EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg); // 安全获取队列指针
        EK_rQueueDelete(queue); // 释放底层队列
        EK_CORO_FREE(msg); // 释放消息队列控制块
    }
    else
    {
        // 静态创建：不需要释放任何内存
        // 内嵌的队列结构体是消息队列结构体的一部分，由用户管理其生命周期
        // 用户缓冲区也由用户自己管理
        // 所以这里什么都不用做
    }

    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 向消息队列发送一条消息。
 * @details 如果有任务正在等待接收消息，则直接将消息发送给等待时间最长的任务。
 *          如果没有任务等待，并且队列未满，则将消息放入队列。
 *          如果队列已满，则根据指定的超时时间，当前任务将被阻塞。
 * 
 * @param msg 消息队列的句柄。
 * @param tx_buffer 指向要发送数据的指针。
 * @param timeout 如果队列已满，等待发送的超时时间。设置为 0 表示不等待，设置为 EK_MAX_DELAY 表示永久等待。
 * 
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 消息成功发送或任务已进入等待状态。
 *         - EK_NULL_POINTER: msg 或 tx_buffer 为空。
 *         - EK_ERROR: 无法获取当前任务的 TCB。
 *         - EK_INSUFFICIENT_SPACE: 当 timeout 为 0 且队列已满时返回。
 */
EK_Result_t EK_rMsgSend(EK_CoroMsgHanler_t msg, void *tx_buffer, uint32_t timeout)
{
    if (msg == NULL || tx_buffer == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB();

    // 检查是否有任务在等待接收 (实现汇合)
    if (msg->Msg_RecvWaitList.List_Count > 0)
    {
        // 直接将数据发送给等待时间最长的接收者
        EK_CoroTCB_t *tcb_wait_to_recv = (EK_CoroTCB_t *)msg->Msg_RecvWaitList.List_Head->CoroNode_Owner;

        // 从等待列表中移除该接收者
        EK_rKernelRemove(&msg->Msg_RecvWaitList, &tcb_wait_to_recv->TCB_EventNode);

        // 将数据直接拷贝到接收者的缓冲区
        EK_vMemCpy(tcb_wait_to_recv->TCB_MsgData, tx_buffer, msg->Msg_ItemSize);

        // 设置事件结果并唤醒接收者
        tcb_wait_to_recv->TCB_EventResult = EK_CORO_EVENT_OK;
        tcb_wait_to_recv->TCB_State = EK_CORO_READY;
        EK_rKernelMove_Tail(EK_pKernelGetReadyList(tcb_wait_to_recv->TCB_Priority), &tcb_wait_to_recv->TCB_StateNode);

        EK_EXIT_CRITICAL();
        return EK_OK;
    }

    // 如果没有任务等待接收，则尝试将消息放入队列
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg); // 安全获取队列指针
    if (EK_sQueueGetRemain(queue) >= msg->Msg_ItemSize)
    {
        // 队列有空间，直接入队
        EK_rQueueEnqueue(queue, tx_buffer, msg->Msg_ItemSize);
        EK_EXIT_CRITICAL();
        return EK_OK;
    }

    // 队列已满，需要阻塞等待
    if (timeout == 0)
    {
        EK_EXIT_CRITICAL();
        return EK_INSUFFICIENT_SPACE;
    }

    // 阻塞当前任务
    current_tcb->TCB_MsgData = tx_buffer; // 保存发送缓冲区指针
    current_tcb->TCB_EventResult = EK_CORO_EVENT_PENDING;

    EK_EXIT_CRITICAL();
    v_msg_delay(msg, current_tcb, timeout, false); // 阻塞

    // --- 唤醒后 ---
    // 唤醒后，检查事件结果
    if (current_tcb->TCB_EventResult == EK_CORO_EVENT_OK)
    {
        // 成功，数据已被取走或放入队列
        return EK_OK;
    }
    else
    {
        // 仅在超时情况下，任务才需要自己清理事件节点
        if (current_tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT)
        {
            EK_ENTER_CRITICAL();
            EK_rKernelRemove(&msg->Msg_SendWaitList, &current_tcb->TCB_EventNode);
            EK_EXIT_CRITICAL();
        }
        // 如果是 EK_CORO_EVENT_DELETED，表示删除函数已经处理了节点，此处无需操作
        // 统一返回超时或错误
        return EK_TIMEOUT;
    }
}

/**
 * @brief 从消息队列接收一条消息。
 * @details 如果有任务因队列满而等待发送，则直接从等待时间最长的任务接收数据。
 *          如果没有任务等待发送，并且队列中有消息，则从队列中取出一个消息。
 *          如果队列为空，则根据指定的超时时间，当前任务将被阻塞。
 * 
 * @param msg 消息队列的句柄。
 * @param rx_buffer 指向用于存储接收数据的缓冲区的指针。
 * @param timeout 如果队列为空，等待接收的超时时间。设置为 0 表示不等待，设置为 EK_MAX_DELAY 表示永久等待。
 * 
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 消息成功接收或任务已进入等待状态。
 *         - EK_NULL_POINTER: msg 或 rx_buffer 为空。
 *         - EK_ERROR: 无法获取当前任务的 TCB。
 *         - EK_TIMEOUT: 当 timeout 为 0 且队列为空时返回。
 */
EK_Result_t EK_rMsgReceive(EK_CoroMsgHanler_t msg, void *rx_buffer, uint32_t timeout)
{
    if (msg == NULL || rx_buffer == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB();

    // 安全获取队列指针
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);

    // 检查队列中是否有消息
    if (EK_sQueueGetSize(queue) >= msg->Msg_ItemSize)
    {
        // 队列有数据，直接出队
        EK_rQueueDequeue(queue, rx_buffer, msg->Msg_ItemSize);

        // 检查是否有任务在等待发送 (队列腾出了空间)
        if (msg->Msg_SendWaitList.List_Count > 0)
        {
            // 从等待发送的列表中取出一个任务
            EK_CoroTCB_t *tcb_wait_to_send = (EK_CoroTCB_t *)msg->Msg_SendWaitList.List_Head->CoroNode_Owner;
            EK_rKernelRemove(&msg->Msg_SendWaitList, &tcb_wait_to_send->TCB_EventNode);

            // 将等待发送的数据放入队列
            EK_rQueueEnqueue(queue, tcb_wait_to_send->TCB_MsgData, msg->Msg_ItemSize);

            // 设置事件结果并唤醒发送者
            tcb_wait_to_send->TCB_EventResult = EK_CORO_EVENT_OK;
            tcb_wait_to_send->TCB_State = EK_CORO_READY;
            EK_rKernelMove_Tail(EK_pKernelGetReadyList(tcb_wait_to_send->TCB_Priority),
                                &tcb_wait_to_send->TCB_StateNode);
        }

        EK_EXIT_CRITICAL();
        return EK_OK;
    }

    // 队列为空，检查是否有任务在等待发送 (实现汇合)
    if (msg->Msg_SendWaitList.List_Count > 0)
    {
        // 直接从等待发送的任务获取数据
        EK_CoroTCB_t *tcb_wait_to_send = (EK_CoroTCB_t *)msg->Msg_SendWaitList.List_Head->CoroNode_Owner;
        EK_rKernelRemove(&msg->Msg_SendWaitList, &tcb_wait_to_send->TCB_EventNode);

        // 将数据直接拷贝到当前任务的缓冲区
        EK_vMemCpy(rx_buffer, tcb_wait_to_send->TCB_MsgData, msg->Msg_ItemSize);

        // 设置事件结果并唤醒发送者
        tcb_wait_to_send->TCB_EventResult = EK_CORO_EVENT_OK;
        tcb_wait_to_send->TCB_State = EK_CORO_READY;
        EK_rKernelMove_Tail(EK_pKernelGetReadyList(tcb_wait_to_send->TCB_Priority), &tcb_wait_to_send->TCB_StateNode);

        EK_EXIT_CRITICAL();
        return EK_OK;
    }

    // 队列为空且无任务等待发送，需要阻塞
    if (timeout == 0)
    {
        EK_EXIT_CRITICAL();
        return EK_EMPTY;
    }

    // 阻塞当前任务
    current_tcb->TCB_MsgData = rx_buffer; // 保存接收缓冲区指针
    current_tcb->TCB_EventResult = EK_CORO_EVENT_PENDING;

    EK_EXIT_CRITICAL();
    v_msg_delay(msg, current_tcb, timeout, true); // 阻塞

    // --- 唤醒后 ---
    // 唤醒后，检查事件结果
    if (current_tcb->TCB_EventResult == EK_CORO_EVENT_OK)
    {
        // 成功，数据已在缓冲区
        return EK_OK;
    }
    else
    {
        // 仅在超时情况下，任务才需要自己清理事件节点
        if (current_tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT)
        {
            EK_ENTER_CRITICAL();
            EK_rKernelRemove(&msg->Msg_RecvWaitList, &current_tcb->TCB_EventNode);
            EK_EXIT_CRITICAL();
        }
        // 如果是 EK_CORO_EVENT_DELETED，表示删除函数已经处理了节点，此处无需操作
        // 统一返回超时或错误
        return EK_TIMEOUT;
    }
}

#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */
