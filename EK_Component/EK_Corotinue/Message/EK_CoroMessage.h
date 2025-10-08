/**
 * @file EK_CoroMessage.h
 * @brief 协程消息队列模块头文件
 * @details 实现协程任务间的消息传递机制，支持动态和静态消息队列创建。
 *          提供消息发送、接收、队列管理等功能，包含任务阻塞和唤醒机制。
 *          采用双节点机制实现状态-事件分离，支持汇合操作和超时控制。
 * @author N1ntyNine99
 * @date 2025-09-30
 * @version v1.0
 */

#ifndef __EK_COROMESSAGE_H
#define __EK_COROMESSAGE_H

#include "../Kernel/Kernel.h"
#include "../../DataStruct/Queue/EK_Queue.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1)

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
/* ========================= 数据结构 ========================= */

/**
 * @brief 消息队列结构体
 * @details 用于管理一组消息队列。
 *          支持动态和静态创建，静态创建时内嵌队列结构体避免动态分配。
 */
typedef struct EK_CoroMsg_t
{
    union
    {
        EK_Queue_t *Msg_Queue; /**< 动态创建时的队列指针 */
        EK_Queue_t Msg_QueueStatic; /**< 静态创建时的内嵌队列结构体 */
    };

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

#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */

#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROMESSAGE_H */