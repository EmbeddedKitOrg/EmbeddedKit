/**
 * @file EK_Serial.h
 * @brief 串口队列管理头文件
 * @details 基于队列和链表实现的串口数据缓冲和定时发送管理接口
 * @author N1ntyNine99
 * @date 2025-09-13
 * @version 1.0
 */

#ifndef __EK_SERIAL_H
#define __EK_SERIAL_H

#include "../../EK_Config.h"
#include "../../DataStruct/Queue/EK_Queue.h"
#include "../../DataStruct/List/EK_List.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ========================= 类型定义区 ========================= */

/**
 * @brief 串口队列结构体
 * @note 用于管理串口数据的缓冲队列和定时发送机制
 */
typedef struct
{
    EK_Node_t *Serial_Owner; /**< 拥有该结构的链表节点 */
    EK_Queue_t *Serial_Queue; /**< 数据缓冲队列 */
    uint8_t Serial_Timer; /**< 定时器 */
    bool Serial_IsDynamic; /**< 动态创建标志位 */
    union
    {
        void (*StaticCallBack)(void *, EK_Size_t); /**< 静态回调函数 */
        void (**DynamicCallBack)(void *, EK_Size_t); /**< 动态回调函数 */
    } Serial_SendCallBack; /**< 数据发送回调函数 */
} EK_SeiralQueue_t;

typedef EK_SeiralQueue_t *EK_pSeiralQueue_t; // EK_SeiralQueue_t的句柄(aka *EK_SeiralQueue_t)

/* ========================= 函数声明区 ========================= */
EK_Result_t EK_rSerialInit(void);
EK_Result_t EK_rSerialInitStatic(void);
EK_Result_t EK_rSerialCreateQueue(EK_pSeiralQueue_t *serial_fifo,
                                  void (*send_func)(void *, EK_Size_t),
                                  uint16_t priority,
                                  EK_Size_t capacity);
EK_Result_t EK_rSerialCreateQueueStatic(EK_pSeiralQueue_t serial_fifo,
                                        void *buffer,
                                        void (*send_func)(void *, EK_Size_t),
                                        uint16_t priority,
                                        EK_Size_t capacity);
EK_Result_t EK_rSerialPrintf(EK_pSeiralQueue_t serial_fifo, const char *format, ...);
EK_Result_t EK_rSerialPoll(uint32_t (*get_tick)(void));
EK_Result_t EK_rSerialDeleteQueue(EK_pSeiralQueue_t serial_fifo);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EK_SERIAL_H */