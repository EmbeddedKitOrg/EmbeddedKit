/**
 * @file EK_Queue.h
 * @brief 队列数据结构头文件
 * @details 定义了队列的数据结构和操作接口
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#ifndef __EK_QUEUE_H
#define __EK_QUEUE_H

#include "../../EK_Config.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* ========================= 类型定义区 ========================= */
typedef struct EK_Queue_t
{
    void *Queue_Buf; /**< 队列缓冲区 */
    uint16_t Queue_Front; /**< 第一个元素的索引 待出队的元素索引 */
    uint16_t Queue_Rear; /**< 最后一个元素的下一个位置的索引 */
    EK_Size_t Queue_Size; /**< 当前的元素个数 */
    EK_Size_t Queue_Capacity; /**< 队列容量 */
    bool Queue_isDynamic; /**< 判断是否是动态创建的队列 */
} EK_Queue_t;

/* ========================= 函数声明区 ========================= */
bool EK_bQueueIsEmpty(EK_Queue_t *queue);
bool EK_bQueueIsFull(EK_Queue_t *queue);
EK_Size_t EK_sQueueGetSize(EK_Queue_t *queue);
EK_Size_t EK_sQueueGetRemain(EK_Queue_t *queue);
EK_Queue_t *EK_pQueueCreate(EK_Size_t capacity);
EK_Result_t EK_rQueueCreateStatic(EK_Queue_t *queue_handler, void *buffer, const EK_Size_t capacity);
EK_Result_t EK_rQueueClean(EK_Queue_t *queue);
EK_Result_t EK_rQueueDelete(EK_Queue_t *queue);
EK_Result_t EK_rQueueEnqueue(EK_Queue_t *queue, void *data, EK_Size_t data_size);
EK_Result_t EK_rQueueDequeue(EK_Queue_t *queue, void *data_buffer, EK_Size_t data_size);
EK_Result_t EK_rQueuePeekFront(EK_Queue_t *queue, void *data_buffer, EK_Size_t data_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EK_QUEUE_H */
