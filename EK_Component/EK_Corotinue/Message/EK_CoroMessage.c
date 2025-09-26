#include "EK_CoroMessage.h"
#include "../Task/EK_CoroTask.h"

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)

/**
 * @brief 唤醒一个正在等待此消息队列的任务
 */
static inline EK_CoroTCB_t *p_wakeup_task_from_list(EK_CoroMsg_t *msg, EK_CoroList_t *list_to_search)
{
    EK_CoroListNode_t *node = list_to_search->List_Head;
    while (node != NULL)
    {
        EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_OwnerTCB;
        if (tcb->TCB_pMsg == msg)
        {
            tcb->TCB_MsgResult = EK_OK;
            tcb->TCB_pMsg = NULL; // 清除事件关联
            EK_rKernelMove(&EK_CoroKernelReadyList, tcb); // 从阻塞列表移到就绪列表
            return tcb;
        }
        node = node->CoroNode_Next;
    }
    return NULL;
}

EK_CoroMsgHanler_t EK_pMsgCreate(EK_Size_t capacity, EK_Size_t item_size)
{
    if (item_size == 0 || capacity == 0) return NULL;

    // 为结构体分配内存
    EK_CoroMsg_t *msg = (EK_CoroMsg_t *)EK_MALLOC(sizeof(EK_CoroMsg_t));
    if (msg == NULL) return NULL;

    // 为消息队列分配内存
    EK_Queue_t *msg_queue = EK_pQueueCreate_Dynamic(capacity);
    if (msg_queue == NULL)
    {
        EK_FREE(msg);
        return NULL;
    }

    msg->Msg_Queue = msg_queue; // 存储队列

    msg->Msg_ItemSize = item_size; // 每条消息的字节大小
    msg->Msg_ItemCapacity = capacity / item_size; // 计算最大可用的消息数目

    msg->Msg_isDynamic = true; // 来源于动态创建

    return (EK_CoroMsgHanler_t)msg;
}

EK_CoroMsgStaticHanler_t EK_pMsgCreateStatic(EK_CoroMsg_t *msg, void *buffer, EK_Size_t capacity, EK_Size_t item_size)
{
    if (msg == NULL || buffer == NULL) return NULL;
    if (item_size == 0 || capacity == 0) return NULL;

    // 创建静态队列
    msg->Msg_Queue = 1; // 避免为NULL时创建失败
    EK_Result_t op_res = EK_rQueueCreate_Static(&msg->Msg_Queue, buffer, capacity);
    if (op_res != EK_OK) return NULL;

    msg->Msg_ItemSize = item_size; // 每条消息的字节大小
    msg->Msg_ItemCapacity = capacity / item_size; // 计算最大可用的消息数目

    msg->Msg_isDynamic = false; // 来源于静态创建

    return (EK_CoroMsgStaticHanler_t)msg;
}

EK_Result_t EK_rMsgSend(EK_CoroMsgHanler_t msg, void *buffer, uint32_t timeout)
{
    if (msg == NULL || buffer == NULL) return EK_NULL_POINTER;
    EK_CoroTCB_t *current_tcb = EK_CoroKernelCurrentTCB;
}

EK_Result_t EK_rMsgReceive(EK_CoroMsgHanler_t msg, void *buffer, uint32_t timeout)
{
    if (msg == NULL || buffer == NULL) return EK_NULL_POINTER;
    EK_CoroTCB_t *current_tcb = EK_CoroKernelCurrentTCB;
}

#endif