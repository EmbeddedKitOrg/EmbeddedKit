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

#include "../../EK_Common.h"
#include "../../DataStruct/Queue/EK_Queue.h"
#include "../../DataStruct/List/EK_List.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 类型定义区 ========================= */

/**
 * @brief 串口队列结构体
 * @note 用于管理串口数据的缓冲队列和定时发送机制
 */
typedef struct
{
    EK_Node_t *Serial_Owner; /**< 拥有该结构的链表节点 */
    EK_Queue_t *Serial_Queue; /**< 数据缓冲队列 */
    uint16_t Serial_Timer; /**< 定时器(高8位:设定值, 低8位:当前值) */
    void *(*Serial_SendCallBack)(void *); /**< 数据发送回调函数 */
} EK_SeiralQueue_t;

/* ========================= 函数声明区 ========================= */
EK_Result_t EK_rSerialInit_Dynamic(void);
EK_Result_t EK_rSerialInit_Static(void);
EK_Result_t EK_rSerialCreateQueue_Dyanmic(EK_SeiralQueue_t *serial_fifo,
                                          void *(*send_func)(void *),
                                          uint16_t priority,
                                          size_t capacity);
EK_Result_t EK_rSerialCreateQueue_Static(
    EK_SeiralQueue_t *serial_fifo, void *buffer, void *(*send_func)(void *), uint16_t priority, size_t capacity);
EK_Result_t EK_rSerialPrintf(EK_SeiralQueue_t *serial_fifo, size_t buffer_size, const char *format, ...);
EK_Result_t EK_rSerialPoll(uint32_t (*get_tick)(void));

#ifdef __cplusplus
}
#endif

#endif