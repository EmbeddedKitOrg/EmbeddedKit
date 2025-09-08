/**
 * @file Queue.c
 * @brief 队列数据结构实现文件
 * @details 提供队列的创建、入队、出队等基本操作功能
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#include "Queue.h"

Queue_t *QueueCreate_Dynamic(size_t capacity)
{
    if (capacity == 0) return NULL;

    Queue_t *queue = (Queue_t *)_MALLOC(sizeof(Queue_t));
    if (queue == NULL) return NULL;

    queue->Queue_Buf = _MALLOC(capacity);
    if (queue->Queue_Buf == NULL) return NULL;

    queue->Queue_Front = queue->Queue_Rear = queue->Queue_Size = 0;
    queue->Queue_Capacity = capacity;
}