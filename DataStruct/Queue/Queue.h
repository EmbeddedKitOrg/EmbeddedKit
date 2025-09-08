/**
 * @file Queue.h
 * @brief 队列数据结构头文件
 * @details 定义了队列的数据结构和操作接口
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#ifndef __QUEUE_H
#define __QUEUE_H

#include "../DataStruct.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef size_t index_t;

typedef struct
{
    void *Queue_Buf; // 队列缓冲区
    index_t Queue_Front; // 第一个元素的索引 待出队的元素索引
    index_t Queue_Rear; // 最后一个元素的下一个位置的索引
    size_t Queue_Size; // 当前的元素个数
    const size_t Queue_Capacity; // 队列容量
} Queue_t;

#ifdef __cplusplus
}
#endif

#endif
