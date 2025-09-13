/**
 * @file EK_Queue.c
 * @brief 队列数据结构实现文件
 * @details 实现了基于循环缓冲区的FIFO队列数据结构，支持动态和静态内存管理。
 *          提供队列的创建、销毁、入队、出队、查看等基本操作，
 *          以及队列状态检查和空间管理功能。
 * @note 队列采用字节流方式存储数据，支持任意类型数据的存储
 * @warning 使用前请确保正确初始化队列结构
 * 
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */
#include "EK_Queue.h"

/**
 * @brief 检查队列是否为空
 * @param queue 要检查的队列指针
 * @return bool 队列为空返回true，非空或队列指针无效返回false
 * @note 当队列指针为NULL时返回false，表示检查失败
 */
bool EK_bQueueIsEmpty(EK_Queue_t *queue)
{
    if (queue == NULL) return false;
    return (queue->Queue_Size == 0);
}

/**
 * @brief 检查队列是否已满
 * @param queue 要检查的队列指针
 * @return bool 队列已满返回true，未满或队列指针无效返回false
 * @note 当队列指针为NULL时返回false，表示检查失败
 */
bool EK_bQueueIsFull(EK_Queue_t *queue)
{
    if (queue == NULL) return false;
    return (queue->Queue_Size >= queue->Queue_Capacity);
}

/**
 * @brief 获取队列当前存储的数据大小
 * @param queue 队列指针
 * @return size_t 返回队列中数据的字节数，队列指针无效时返回0
 * @note 返回值表示队列中实际存储的数据量，不是队列容量
 */
size_t EK_sQueueGetSize(EK_Queue_t *queue)
{
    if (queue == NULL) return 0;
    return queue->Queue_Size;
}

/**
 * @brief 获取队列剩余可用空间大小
 * @param queue 队列指针
 * @return size_t 返回队列剩余可用的字节数，队列指针无效时返回0
 * @note 返回值表示还可以向队列中写入多少字节的数据
 */
size_t EK_sQueueGetRemain(EK_Queue_t *queue)
{
    if (queue == NULL) return 0;
    int64_t temp = queue->Queue_Capacity - queue->Queue_Size;
    return temp > 0 ? (size_t)temp : 0;
}

/**
 * @brief 动态创建一个队列（使用动态内存分配）
 * @param capacity 队列容量（字节数）
 * @return EK_Queue_t* 返回创建的队列指针，失败返回NULL
 * @note 适用于动态分配场景，队列内存由malloc管理，需要使用QueueDelete释放
 */
EK_Queue_t *EK_pQueueCreate_Dynamic(size_t capacity)
{
    if (capacity == 0) return NULL;

    // 分配缓冲区内存
    void *buffer = _MALLOC(capacity);
    if (buffer == NULL) return NULL;

    // 分配队列结构体内存
    EK_Queue_t *queue = (EK_Queue_t *)_MALLOC(sizeof(EK_Queue_t));
    if (queue == NULL)
    {
        _FREE(buffer);
        return NULL;
    }

    // 直接赋值初始化结构体
    queue->Queue_Buf = buffer;
    queue->Queue_Front = 0;
    queue->Queue_Rear = 0;
    queue->Queue_Size = 0;
    queue->Queue_Capacity = capacity;
    queue->Queue_isDynamic = true;

    return queue;
}

/**
 * @brief 静态创建一个队列（使用用户提供的内存）
 * @param queue_handler 用户提供的队列结构体指针
 * @param buffer 用户提供的缓冲区内存
 * @param capacity 队列容量（字节数）
 * @return EK_Result_t 创建成功返回EK_OK，失败返回对应错误码
 * @note 适用于静态分配场景，队列结构体和缓冲区内存都由用户管理
 */
EK_Result_t EK_rQueueCreate_Static(EK_Queue_t *queue_handler, void *buffer, const size_t capacity)
{
    // 判断入参是否无效
    if (queue_handler == NULL || capacity == 0) return EK_INVALID_PARAM;

    // 直接赋值初始化结构体
    queue_handler->Queue_Buf = buffer;
    queue_handler->Queue_Front = 0;
    queue_handler->Queue_Rear = 0;
    queue_handler->Queue_Size = 0;
    queue_handler->Queue_Capacity = capacity;
    queue_handler->Queue_isDynamic = false;

    return EK_OK;
}

/**
 * @brief 清空队列中的所有数据
 * @details 重置队列的前后指针和大小计数器，使队列变为空状态
 * @param queue 队列指针
 * @return EK_Result_t 操作结果
 * @retval EK_OK 清空成功
 * @retval EK_NULL_POINTER 队列指针为空
 * @note 只重置队列状态，不清零缓冲区内容以提高性能
 */
EK_Result_t EK_rQueueClean(EK_Queue_t *queue)
{
    // 参数有效性检查
    if (queue == NULL) return EK_NULL_POINTER;

    // 重置队列状态为空
    queue->Queue_Front = 0;
    queue->Queue_Rear = 0;
    queue->Queue_Size = 0;

    return EK_OK;
}

/**
 * @brief 删除队列并释放相关资源
 * @param queue 要删除的队列指针
 * @return EK_Result_t 删除成功返回EK_OK，失败返回对应错误码
 * @note 对于动态队列会释放malloc分配的内存，对于静态队列只清空缓冲区
 * @warning 删除后队列指针将变为无效，不应再使用
 */
EK_Result_t EK_rQueueDelete(EK_Queue_t *queue)
{
    if (queue == NULL) return EK_NULL_POINTER;

    // 处理动态队列
    if (queue->Queue_isDynamic == true)
    {
        // 先释放缓冲区内存
        if (queue->Queue_Buf != NULL)
        {
            _FREE(queue->Queue_Buf);
        }
        // 再释放队列结构体内存
        _FREE(queue);
        return EK_OK;
    }

    // 处理静态队列
    else
    {
        // 清空缓冲区（如果缓冲区存在）
        if (queue->Queue_Buf != NULL && queue->Queue_Size > 0)
        {
            memset(queue->Queue_Buf, 0, queue->Queue_Size);
        }
        // 重置队列状态
        queue->Queue_Buf = NULL;
        queue->Queue_Front = 0;
        queue->Queue_Rear = 0;
        queue->Queue_Size = 0;
        return EK_OK;
    }
}

/**
 * @brief 向队列尾部添加数据（入队操作）
 * @param queue 队列指针
 * @param data 要添加的数据指针
 * @param data_size 数据大小（字节数）
 * @return EK_Result_t 入队成功返回EK_OK，失败返回对应错误码
 * @note 会检查队列剩余空间是否足够，数据会被复制到队列内部缓冲区
 * @warning 确保data指向的内存区域至少有data_size字节有效数据
 */
EK_Result_t EK_rQueueEnqueue(EK_Queue_t *queue, void *data, size_t data_size)
{
    // 参数有效性检查
    if (queue == NULL || data == NULL || data_size == 0) return EK_INVALID_PARAM;

    // 检查队列是否满了
    if (EK_bQueueIsFull(queue)) return EK_FULL;

    // 检测剩余空间是否足够
    if (EK_sQueueGetRemain(queue) < data_size) return EK_INSUFFICIENT_SPACE;

    // 计算写入缓冲区的起始位置
    uint8_t *start_addr = (uint8_t *)queue->Queue_Buf + queue->Queue_Rear;

    // 将数据复制到缓冲区
    memcpy(start_addr, data, data_size);

    // 更新队列指针
    queue->Queue_Rear = (queue->Queue_Rear + data_size) % queue->Queue_Capacity;

    // 更新大小
    queue->Queue_Size += data_size;

    return EK_OK;
}

/**
 * @brief 从队列头部取出数据（出队操作）
 * @param queue 队列指针
 * @param data_buffer 用于接收出队数据的缓冲区指针
 * @param data_size 要取出的数据大小（字节数）
 * @return EK_Result_t 操作结果，成功返回EK_OK
 * @note 会检查队列中数据是否足够，数据会从队列中移除并复制到data_buffer中
 */
EK_Result_t EK_rQueueDequeue(EK_Queue_t *queue, void *data_buffer, size_t data_size)
{
    // 参数有效性检查
    if (queue == NULL || data_buffer == NULL || data_size == 0) return EK_INVALID_PARAM;

    // 检查队列是否为空
    if (EK_bQueueIsEmpty(queue)) return EK_EMPTY;

    // 检查队列是否有所需求的数据数目
    if (EK_sQueueGetSize(queue) < data_size) return EK_INSUFFICIENT_SPACE;

    // 计算读取位置
    uint8_t *read_addr = (uint8_t *)queue->Queue_Buf + queue->Queue_Front;

    // 将数据复制到用户提供的缓冲区
    memcpy(data_buffer, read_addr, data_size);

    // 更新队列指针
    queue->Queue_Front = (queue->Queue_Front + data_size) % queue->Queue_Capacity;

    // 更新大小
    queue->Queue_Size -= data_size;

    return EK_OK;
}

/**
 * @brief 查看队列头部数据但不移除（窥视操作）
 * @param queue 队列指针
 * @param data_buffer 用于接收数据的缓冲区指针
 * @param data_size 要查看的数据大小（字节数）
 * @return EK_Result_t 操作结果，成功返回EK_OK
 * @note 只查看数据不会从队列中移除，队列状态保持不变
 */
EK_Result_t EK_rQueuePeekFront(EK_Queue_t *queue, void *data_buffer, size_t data_size)
{
    // 参数有效性检查
    if (queue == NULL || data_buffer == NULL || data_size == 0) return EK_INVALID_PARAM;

    // 检查队列是否为空
    if (EK_bQueueIsEmpty(queue)) return EK_EMPTY;

    // 检查队列是否有所需求的数据数目
    if (EK_sQueueGetSize(queue) < data_size) return EK_INSUFFICIENT_SPACE;

    // 计算读取位置
    uint8_t *read_addr = (uint8_t *)queue->Queue_Buf + queue->Queue_Front;

    // 将数据复制到用户提供的缓冲区
    memcpy(data_buffer, read_addr, data_size);

    return EK_OK;
}
