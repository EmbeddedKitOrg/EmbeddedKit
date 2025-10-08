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
#include "stm32f4xx.h" // 为了访问SCB寄存器
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
ALWAYS_STATIC_INLINE void v_msg_delay(EK_CoroMsg_t *msg, EK_CoroTCB_t *tcb, uint32_t timeout, bool isRecv)
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

/**
 * @brief 从等待列表中取出一个等待任务
 * @details
 *  用于统一处理发送/接收等待链表的节点获取与移除逻辑。
 *  返回的TCB已从等待链表中安全移除，可进一步拷贝数据或唤醒。
 * @param wait_list 目标等待链表指针。
 * @return EK_CoroTCB_t* 成功返回等待的任务指针，链表为空或参数无效返回NULL。
 */
ALWAYS_STATIC_INLINE EK_CoroTCB_t *p_msg_take_waiter(EK_CoroList_t *wait_list)
{
    if (wait_list == NULL || wait_list->List_Count == 0) return NULL;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)wait_list->List_Head->CoroNode_Owner;
    EK_rKernelRemove(wait_list, &tcb->TCB_EventNode);

    return tcb;
}

/**
 * @brief 内部唤醒等待消息队列的任务
 * @details
 *  设置事件结果并将任务重新放入就绪链表。
 *  需配合 p_msg_take_waiter 一起使用，确保任务已从等待链表移除。
 * @param tcb 要唤醒的协程TCB。
 * @param result 唤醒原因，对应 EK_CoroEventResult_t 枚举。
 */
ALWAYS_STATIC_INLINE void v_msg_wake(EK_CoroTCB_t *tcb, EK_CoroEventResult_t result)
{
    if (tcb == NULL) return;

    tcb->TCB_EventResult = result;
    tcb->TCB_State = EK_CORO_READY;
    EK_rKernelMove_Tail(EK_pKernelGetReadyList(tcb->TCB_Priority), &tcb->TCB_StateNode);
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
    EK_CoroTCB_t *tcb_waiter = NULL;
    while ((tcb_waiter = p_msg_take_waiter(&msg->Msg_SendWaitList)) != NULL)
    {
        v_msg_wake(tcb_waiter, EK_CORO_EVENT_DELETED);
    }

    // 唤醒所有等待接收的任务
    while ((tcb_waiter = p_msg_take_waiter(&msg->Msg_RecvWaitList)) != NULL)
    {
        v_msg_wake(tcb_waiter, EK_CORO_EVENT_DELETED);
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
    // 判断是不是在中断中
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;

    if (msg == NULL || tx_buffer == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB();

    // 检查是否有任务在等待接收 (实现汇合)
    if (msg->Msg_RecvWaitList.List_Count > 0)
    {
        // 直接将数据发送给等待时间最长的接收者
        EK_CoroTCB_t *tcb_wait_to_recv = p_msg_take_waiter(&msg->Msg_RecvWaitList);
        if (tcb_wait_to_recv != NULL)
        {
            EK_vMemCpy(tcb_wait_to_recv->TCB_MsgData, tx_buffer, msg->Msg_ItemSize);
            v_msg_wake(tcb_wait_to_recv, EK_CORO_EVENT_OK);
        }

        EK_EXIT_CRITICAL();
        return EK_OK;
    }

    // 如果没有任务等待接收，则尝试将消息放入队列
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg); // 安全获取队列指针
    if (EK_uQueueGetRemain(queue) >= msg->Msg_ItemSize)
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
    // 判断是不是在中断中
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;

    if (msg == NULL || rx_buffer == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB();

    // 安全获取队列指针
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);

    // 检查队列中是否有消息
    if (EK_uQueueGetSize(queue) >= msg->Msg_ItemSize)
    {
        // 队列有数据，直接出队
        EK_rQueueDequeue(queue, rx_buffer, msg->Msg_ItemSize);

        // 检查是否有任务在等待发送 (队列腾出了空间)
        if (msg->Msg_SendWaitList.List_Count > 0)
        {
            // 从等待发送的列表中取出一个任务
            EK_CoroTCB_t *tcb_wait_to_send = p_msg_take_waiter(&msg->Msg_SendWaitList);
            if (tcb_wait_to_send != NULL)
            {
                // 将等待发送的数据放入队列
                EK_rQueueEnqueue(queue, tcb_wait_to_send->TCB_MsgData, msg->Msg_ItemSize);

                // 设置事件结果并唤醒发送者
                v_msg_wake(tcb_wait_to_send, EK_CORO_EVENT_OK);
            }
        }

        EK_EXIT_CRITICAL();
        return EK_OK;
    }

    // 队列为空，检查是否有任务在等待发送 (实现汇合)
    if (msg->Msg_SendWaitList.List_Count > 0)
    {
        // 直接从等待发送的任务获取数据
        EK_CoroTCB_t *tcb_wait_to_send = p_msg_take_waiter(&msg->Msg_SendWaitList);
        if (tcb_wait_to_send != NULL)
        {
            // 将数据直接拷贝到当前任务的缓冲区
            EK_vMemCpy(rx_buffer, tcb_wait_to_send->TCB_MsgData, msg->Msg_ItemSize);

            // 设置事件结果并唤醒发送者
            v_msg_wake(tcb_wait_to_send, EK_CORO_EVENT_OK);

            EK_EXIT_CRITICAL();
            return EK_OK;
        }
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

/**
 * @brief 查看消息队列头部的消息但不移除
 * @details
 *  该函数用于在不改变队列内容的情况下读取队列头部的消息。
 *  即使队列为空也不会阻塞，调用者需要检查返回值。
 * @param msg 消息队列的句柄。
 * @param rx_buffer 指向用于接收数据的缓冲区的指针。
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功将消息复制到缓冲区。
 *         - EK_NULL_POINTER: msg 或 rx_buffer 为空。
 *         - EK_EMPTY: 队列为空，无可读取的数据。
 */
EK_Result_t EK_rMsgPeek(EK_CoroMsgHanler_t msg, void *rx_buffer)
{
    if (msg == NULL || rx_buffer == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    // 安全获取底层队列指针
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);
    EK_Result_t op_res;

    // 如果队列中没有完整的一条消息则返回空
    if (EK_uQueueGetSize(queue) < msg->Msg_ItemSize)
    {
        op_res = EK_EMPTY;
    }
    else
    {
        // 调用底层Peek接口复制但不移除队列内容
        op_res = EK_rQueuePeekFront(queue, rx_buffer, msg->Msg_ItemSize);
    }

    EK_EXIT_CRITICAL();

    return op_res;
}

/**
 * @brief 获取消息队列已存储的消息数量
 * @param msg 消息队列的句柄。
 * @return EK_Size_t 当前队列中的消息数量，失败时返回0。
 */
EK_Size_t EK_uMsgGetCount(EK_CoroMsgHanler_t msg)
{
    if (msg == NULL || msg->Msg_ItemSize == 0) return 0;

    EK_ENTER_CRITICAL();

    // 已使用的字节数转化为消息数量
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);
    EK_Size_t used_bytes = EK_uQueueGetSize(queue);

    EK_EXIT_CRITICAL();

    return used_bytes / msg->Msg_ItemSize;
}

/**
 * @brief 获取消息队列剩余可用的消息数量
 * @param msg 消息队列的句柄。
 * @return EK_Size_t 剩余可容纳的消息数量，失败时返回0。
 */
EK_Size_t EK_uMsgGetFree(EK_CoroMsgHanler_t msg)
{
    if (msg == NULL || msg->Msg_ItemSize == 0) return 0;

    EK_ENTER_CRITICAL();

    // 剩余可写字节转换成消息数量
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);
    EK_Size_t remain_bytes = EK_uQueueGetRemain(queue);

    EK_EXIT_CRITICAL();

    return remain_bytes / msg->Msg_ItemSize;
}

/**
 * @brief 获取消息队列总共的消息数量
 * @param msg 消息队列的句柄。
 * @return EK_Size_t 总共的消息数量，失败时返回0。
 */
EK_Size_t EK_uMsgGetCapacity(EK_CoroMsgHanler_t msg)
{
    if (msg == NULL || msg->Msg_ItemSize == 0) return 0;

    EK_ENTER_CRITICAL();

    // 剩余可写字节转换成消息数量
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);

    EK_EXIT_CRITICAL();

    return queue->Queue_Capacity / msg->Msg_ItemSize;
}

/**
 * @brief 检查消息队列是否已满
 * @details 检查指定的消息队列是否已经达到最大容量，无法再接收新消息
 * @param msg 消息队列句柄
 * @return bool 队列已满返回true，否则返回false；当msg为NULL时返回false
 * @note 此函数不会阻塞，只是查询队列的当前状态
 */
bool EK_bMsgIsFull(EK_CoroMsgHanler_t msg)
{
    // 参数有效性检查
    if (msg == NULL) return false;
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);
    return EK_bQueueIsFull(queue);
}

/**
 * @brief 检查消息队列是否为空
 * @details 检查指定的消息队列是否没有任何消息可供接收
 * @param msg 消息队列句柄
 * @return bool 队列为空返回true，否则返回false；当msg为NULL时返回false
 * @note 此函数不会阻塞，只是查询队列的当前状态
 */
bool EK_bMsgIsEmpty(EK_CoroMsgHanler_t msg)
{
    // 参数有效性检查
    if (msg == NULL) return false;
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);
    return EK_bQueueIsEmpty(queue);
}

/**
 * @brief 清空消息队列中的全部数据
 * @details
 *  将队列重置为空状态，并唤醒所有因队列满而等待发送的任务。
 *  接收等待列表保持不变，调用者可根据需要继续发送新消息。
 * @param msg 消息队列的句柄。
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功清空队列。
 *         - EK_NULL_POINTER: msg 为空。
 *         - EK_ERROR: 在中断上下文中调用。
 */
EK_Result_t EK_rMsgClean(EK_CoroMsgHanler_t msg)
{
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;
    if (msg == NULL) return EK_NULL_POINTER;

    bool need_yield = false;

    EK_ENTER_CRITICAL();

    // 重置队列读写指针
    EK_Queue_t *queue = EK_MSG_GET_QUEUE(msg);
    EK_Result_t op_res = EK_rQueueClean(queue);

    if (op_res != EK_OK)
    {
        EK_EXIT_CRITICAL();
        return op_res;
    }

    // 唤醒所有等待发送的任务，让它们重新尝试
    EK_CoroTCB_t *tcb_waiter = NULL;
    while ((tcb_waiter = p_msg_take_waiter(&msg->Msg_SendWaitList)) != NULL)
    {
        v_msg_wake(tcb_waiter, EK_CORO_EVENT_OK);
        need_yield = true;
    }

    EK_EXIT_CRITICAL();

    if (need_yield)
    {
        EK_vKernelYield();
    }

    return EK_OK;
}

#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */
