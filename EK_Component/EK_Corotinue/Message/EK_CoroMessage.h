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
/* ========================= 辅助宏 ========================= */
/**
 * @brief 安全获取消息队列的队列指针
 * @details 根据创建类型返回正确的队列指针
 */
#define EK_MSG_GET_QUEUE(msg_handler) \
    ((msg_handler)->Msg_isDynamic ? (msg_handler)->Msg_Queue : &(msg_handler)->Msg_QueueStatic)

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
    bool Msg_isDynamic; /**< 是否来源于动态创建 */

    EK_CoroList_t Msg_SendWaitList; /**< 因队列满而等待发送的任务列表 */
    EK_CoroList_t Msg_RecvWaitList; /**< 因队列空而等待接收的任务列表 */
} EK_CoroMsg_t;

typedef EK_CoroMsg_t *EK_CoroMsgHanler_t; /**< 动态类型消息队列句柄 */
typedef EK_CoroMsg_t *EK_CoroMsgStaticHanler_t; /**< 静态类型消息队列句柄 */

/* ========================= 消息队列状态查询函数 ========================= */
#define EK_bMsgIsFull(msg_handler)      EK_bQueueIsFull(EK_MSG_GET_QUEUE(msg_handler))
#define EK_bMsgIsEmpty(msg_handler)     EK_bQueueIsEmpty(EK_MSG_GET_QUEUE(msg_handler))
#define EK_uMsgGetCount(msg_handler)    (EK_uQueueGetSize(EK_MSG_GET_QUEUE(msg_handler)) / msg_handler->Msg_ItemSize)
#define EK_uMsgGetFree(msg_handler)     (EK_uQueueGetRemain(EK_MSG_GET_QUEUE(msg_handler)) / msg_handler->Msg_ItemSize)
#define EK_uMsgGetCapacity(msg_handler) (EK_MSG_GET_QUEUE(msg_handler)->Queue_Capacity / msg_handler->Msg_ItemSize)

/* ========================= 函数声明区 ========================= */
EK_CoroMsgHanler_t EK_pMsgCreate(EK_Size_t item_size, EK_Size_t item_amount);
EK_CoroMsgStaticHanler_t
EK_pMsgCreateStatic(EK_CoroMsg_t *msg, void *buffer, EK_Size_t item_size, EK_Size_t item_amount);
EK_Result_t EK_rMsgDelete(EK_CoroMsg_t *msg);
EK_Result_t EK_rMsgSend(EK_CoroMsgHanler_t msg, void *tx_buffer, uint32_t timeout, bool over_write);
bool EK_bMsgSend_FromISR(EK_CoroMsgHanler_t msg, void *tx_buffer, bool *higher_prio_wake, bool over_write);
EK_Result_t EK_rMsgReceive(EK_CoroMsgHanler_t msg, void *rx_buffer, uint32_t timeout);
EK_Result_t EK_rMsgPeek(EK_CoroMsgHanler_t msg, void *rx_buffer);
EK_Result_t EK_rMsgClean(EK_CoroMsgHanler_t msg);
/* ========================= 中断服务程序API ========================= */

/**
 * @brief 向消息队列尾部发送消息（FIFO模式）
 * @details 此宏函数将消息添加到队列尾部，如果队列已满，则根据timeout参数进行阻塞。
 *          采用先进先出(FIFO)策略，保证消息按发送顺序被接收。
 *          如果有任务因等待接收消息而阻塞，会直接唤醒该任务并传递数据。
 *
 * @param msg_handler 消息队列句柄
 * @param tx_buffer 指向要发送数据的缓冲区
 * @param timeout 队列满时的等待超时时间（毫秒），0表示不等待，EK_MAX_DELAY表示永久等待
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 消息发送成功
 *         - EK_NULL_POINTER: msg_handler或tx_buffer为空
 *         - EK_ERROR: 在中断上下文中调用或当前任务为空闲任务
 *         - EK_FULL: 队列满且timeout为0
 *         - EK_TIMEOUT: 等待超时
 */
#define EK_rMsgSendToBack(msg_handler, tx_buffer, timeout) EK_rMsgSend(msg_handler, tx_buffer, timeout, false)

/**
 * @brief 向消息队列发送消息（覆盖模式）
 * @details 此宏函数发送消息时，如果队列已满，将覆盖队列中最旧的消息。
 *          采用覆盖策略，确保新消息总能发送成功，但会丢失队列中最旧的数据。
 *          适用于最新数据最重要的场景，如传感器数据更新。
 *          如果有任务因等待接收消息而阻塞，会直接唤醒该任务并传递数据。
 *
 * @param msg_handler 消息队列句柄
 * @param tx_buffer 指向要发送数据的缓冲区
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 消息发送成功
 *         - EK_NULL_POINTER: msg_handler或tx_buffer为空
 *         - EK_ERROR: 在中断上下文中调用或当前任务为空闲任务
 */
#define EK_rMsgOverWrite(msg_handler, tx_buffer) EK_rMsgSend(msg_handler, tx_buffer, 0, true)

/**
 * @brief 向消息队列尾部发送消息（中断服务程序FIFO模式）
 * @details 此宏函数专用于中断服务程序中将消息添加到队列尾部，采用先进先出(FIFO)策略。
 *          与普通版本的区别：
 *          1. 只能在中断上下文中调用，不会阻塞调用者
 *          2. 队列满时直接返回失败，不进行等待
 *          3. 通过higher_prio_wake参数指示是否需要进行任务切换
 *
 *          发送策略：
 *          - 如果有任务因等待接收消息而阻塞，会直接唤醒该任务并传递数据
 *          - 否则将消息添加到队列尾部，如果队列满则返回失败
 *
 * @param msg_handler 消息队列句柄，必须为有效的消息队列指针
 * @param tx_buffer 指向要发送数据的缓冲区，数据将被复制到队列中
 * @param higher_prio_wake 输出参数，用于指示是否唤醒了更高优先级的任务：
 *                        - true: 唤醒了更高优先级任务，建议在中断退出后进行任务切换
 *                        - false: 未唤醒更高优先级任务，无需特殊处理
 *
 * @return bool 操作结果：
 *         - true: 消息发送成功
 *         - false: 发送失败（非中断上下文、msg_handler为空、队列满）
 *
 * @note 此宏函数只能在中断服务程序中调用
 * @note 调用者需要检查higher_prio_wake参数，如果为true则应该触发任务切换
 * @note 队列满时不会等待，直接返回失败
 * @note 确保tx_buffer指向的数据在中断处理期间保持有效
 *
 * @warning 不要在非中断上下文中调用此宏函数
 * @warning higher_prio_wake参数必须指向有效的bool变量
 *
 * @see EK_rMsgSendToBack() 普通版本的FIFO模式发送宏
 * @see EK_bMsgSendOverWrite_FromISR() 中断版本的覆盖模式发送宏
 * @see EK_bMsgSend_FromISR() 中断版本的基础发送函数
 */
#define EK_bMsgSendToBack_FromISR(msg_handler, tx_buffer, higher_prio_wake) \
    EK_bMsgSend_FromISR(msg_handler, tx_buffer, higher_prio_wake, false)

/**
 * @brief 向消息队列发送消息（中断服务程序覆盖模式）
 * @details 此宏函数专用于中断服务程序中发送消息，如果队列已满，将覆盖队列中最旧的消息。
 *          与普通版本的区别：
 *          1. 只能在中断上下文中调用，不会阻塞调用者
 *          2. 队列满时直接覆盖最旧消息，总是发送成功（除非参数无效）
 *          3. 通过higher_prio_wake参数指示是否需要进行任务切换
 *
 *          覆盖策略：
 *          - 如果有任务因等待接收消息而阻塞，会直接唤醒该任务并传递数据
 *          - 否则将消息添加到队列，队列满时覆盖最旧消息
 *
 *          适用场景：
 *          - 最新数据最重要的场景，如传感器数据更新
 *          - 不关心历史数据丢失的场景
 *          - 需要确保数据及时传递的场景
 *
 * @param msg_handler 消息队列句柄，必须为有效的消息队列指针
 * @param tx_buffer 指向要发送数据的缓冲区，数据将被复制到队列中
 * @param higher_prio_wake 输出参数，用于指示是否唤醒了更高优先级的任务：
 *                        - true: 唤醒了更高优先级任务，建议在中断退出后进行任务切换
 *                        - false: 未唤醒更高优先级任务，无需特殊处理
 *
 * @return bool 操作结果：
 *         - true: 消息发送成功（覆盖模式下只要参数有效就返回true）
 *         - false: 发送失败（非中断上下文、msg_handler为空）
 *
 * @note 此宏函数只能在中断服务程序中调用
 * @note 调用者需要检查higher_prio_wake参数，如果为true则应该触发任务切换
 * @note 覆盖模式下不会因队列满而失败
 * @note 确保tx_buffer指向的数据在中断处理期间保持有效
 *
 * @warning 不要在非中断上下文中调用此宏函数
 * @warning higher_prio_wake参数必须指向有效的bool变量
 * @warning 注意覆盖模式会丢失队列中最旧的消息
 *
 * @see EK_rMsgOverWrite() 普通版本的覆盖模式发送宏
 * @see EK_bMsgSendToBack_FromISR() 中断版本的FIFO模式发送宏
 * @see EK_bMsgSend_FromISR() 中断版本的基础发送函数
 */
#define EK_bMsgSendOverWrite_FromISR(msg_handler, tx_buffer, higher_prio_wake) \
    EK_bMsgSend_FromISR(msg_handler, tx_buffer, higher_prio_wake, true)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */

#endif /* EK_CORO_ENABLE == 1 */

#endif /* __EK_COROMESSAGE_H */