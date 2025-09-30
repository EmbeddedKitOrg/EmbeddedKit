#ifndef __EK_COROMESSAGE_H
#define __EK_COROMESSAGE_H

#include "../Kernel/Kernel.h"
#include "../../DataStruct/Queue/EK_Queue.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_USE_MESSAGE_QUEUE == 1)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
/* ========================= 数据结构 ========================= */

/**
 * @brief 消息队列结构体
 * @details 用于管理一组消息队列。
 */
typedef struct EK_CoroMsg_t
{
    EK_Queue_t *Msg_Queue; /**< 底层数据队列(EK_Queue.h) */
    EK_Size_t Msg_ItemSize; /**< 队列中每个消息的大小（字节） */
    EK_Size_t Msg_ItemCapacity; /**< 总共可容纳多少个消息 */
    bool Msg_isDynamic; /**< 是否来源于动态创建 */

    EK_CoroList_t Msg_SendWaitList; /**< 因队列满而等待发送的任务列表 */
    EK_CoroList_t Msg_RecvWaitList; /**< 因队列空而等待接收的任务列表 */
} EK_CoroMsg_t;

typedef EK_CoroMsg_t *EK_CoroMsgHanler_t; /**< 动态类型消息队列句柄 */
typedef EK_CoroMsg_t *EK_CoroMsgStaticHanler_t; /**< 静态类型消息队列句柄 */

/* ========================= 函数声明区 ========================= */
EK_CoroMsgHanler_t EK_pMsgCreate(EK_Size_t item_size, EK_Size_t item_amount);
EK_CoroMsgStaticHanler_t
EK_pMsgCreateStatic(EK_CoroMsg_t *msg, void *buffer, EK_Size_t item_size, EK_Size_t item_amount);
EK_Result_t EK_rMsgDelete(EK_CoroMsg_t *msg);
EK_Result_t EK_rMsgSend(EK_CoroMsgHanler_t msg, void *tx_buffer, uint32_t timeout);
EK_Result_t EK_rMsgReceive(EK_CoroMsgHanler_t msg, void *rx_buffer, uint32_t timeout);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_CORO_USE_MESSAGE_QUEUE == 1 */

#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROMESSAGE_H */