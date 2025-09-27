#ifndef __EK_COROMESSAGE_H
#define __EK_COROMESSAGE_H

#include "../Kernel/Kernel.h"
#include "../../DataStruct/Queue/EK_Queue.h"

#if (EK_CORO_USE_MESSAGE_QUEUE == 1)

#ifdef __cplusplus
extern "C"
{
#endif

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
} EK_CoroMsg_t;

typedef EK_CoroMsg_t *EK_CoroMsgHanler_t; // 动态类型消息队列句柄
typedef EK_CoroMsg_t *EK_CoroMsgStaticHanler_t; // 静态类型消息队列句柄

#ifdef __cplusplus
}
#endif

#endif

#endif