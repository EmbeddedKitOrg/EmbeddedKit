# Queue 队列组件

## 概述

Queue组件实现了基于循环缓冲区的FIFO（先进先出）队列数据结构，支持动态和静态内存管理。该组件采用字节流方式存储数据，可以存储任意类型的数据，提供高效的入队和出队操作。

## 特性

- **循环缓冲区**：采用环形缓冲区实现，空间利用率高
- **FIFO操作**：严格的先进先出队列语义
- **字节流存储**：支持任意类型数据的存储和检索
- **双重内存管理**：支持动态分配和静态分配两种方式
- **空间检查**：提供队列状态检查和剩余空间查询
- **安全操作**：完整的参数检查和边界条件处理
- **统一错误处理**：使用DS_Result_t统一返回码体系

## 数据结构

### Queue_t - 队列结构
```c
typedef struct Queue_t
{
    void *Queue_Buf;              /**< 队列缓冲区 */
    index_t Queue_Front;          /**< 第一个元素的索引 待出队的元素索引 */
    index_t Queue_Rear;           /**< 最后一个元素的下一个位置的索引 */
    size_t Queue_Size;            /**< 当前的元素个数 */
    const size_t Queue_Capacity;  /**< 队列容量 */
    const bool Queue_isDynamic;   /**< 判断是否是动态创建的队列 */
} Queue_t;
```

## API接口

### 队列创建和销毁
- `Queue_t *QueueCreate_Dynamic(size_t capacity)` - 动态创建队列
- `DS_Result_t QueueCreate_Static(Queue_t *queue_handler, void *buffer, const size_t capacity)` - 静态创建队列
- `DS_Result_t QueueDelete(Queue_t *queue)` - 删除队列

### 队列状态检查
- `bool QueueIsEmpty(Queue_t *queue)` - 检查队列是否为空
- `bool QueueIsFull(Queue_t *queue)` - 检查队列是否已满
- `size_t QueueGetSize(Queue_t *queue)` - 获取队列当前数据大小
- `size_t QueueGetRemain(Queue_t *queue)` - 获取队列剩余空间

### 数据操作
- `DS_Result_t QueueEnqueue(Queue_t *queue, void *data, size_t data_size)` - 入队操作
- `DS_Result_t QueueDequeue(Queue_t *queue, void *data_buffer, size_t data_size)` - 出队操作
- `DS_Result_t QueuePeekFront(Queue_t *queue, void *data_buffer, size_t data_size)` - 查看队头数据

## 使用示例

### 动态队列使用
```c
#include "Queue.h"

// 创建1024字节的动态队列
Queue_t *queue = QueueCreate_Dynamic(1024);
if (queue == NULL) {
    // 处理创建失败
    return;
}

// 入队操作
int data[] = {1, 2, 3, 4, 5};
DS_Result_t result = QueueEnqueue(queue, data, sizeof(data));
if (result != DS_SUCCESS) {
    // 处理入队失败
}

// 出队操作
int recv_data[5];
result = QueueDequeue(queue, recv_data, sizeof(recv_data));
if (result != DS_SUCCESS) {
    // 处理出队失败
}

// 查看队头数据（不移除）
int peek_data[5];
result = QueuePeekFront(queue, peek_data, sizeof(peek_data));

// 删除队列
QueueDelete(queue);
```

### 静态队列使用
```c
#include "Queue.h"

// 静态分配内存
Queue_t my_queue;
uint8_t buffer[512];

// 创建静态队列
DS_Result_t result = QueueCreate_Static(&my_queue, buffer, sizeof(buffer));
if (result != DS_SUCCESS) {
    // 处理创建失败
    return;
}

// 使用队列进行数据操作
char message[] = "Hello, Queue!";
QueueEnqueue(&my_queue, message, strlen(message) + 1);

char recv_message[32];
QueueDequeue(&my_queue, recv_message, strlen(message) + 1);

// 静态队列不需要显式删除，但可以调用QueueDelete进行清理
QueueDelete(&my_queue);
```

### 队列状态检查
```c
// 检查队列状态
if (QueueIsEmpty(queue)) {
    printf("Queue is empty\n");
}

if (QueueIsFull(queue)) {
    printf("Queue is full\n");
}

printf("Queue size: %zu bytes\n", QueueGetSize(queue));
printf("Queue remaining: %zu bytes\n", QueueGetRemain(queue));
```

## 内存管理

### 动态队列
- 队列结构体和缓冲区都通过malloc分配
- 使用QueueDelete自动释放所有相关内存
- 适用于运行时确定大小的场景

### 静态队列
- 队列结构体和缓冲区由用户管理
- QueueDelete只清空队列内容，不释放用户内存
- 适用于嵌入式系统等内存受限环境

## 注意事项

1. **容量规划**：队列容量应根据实际数据大小合理规划
2. **数据对齐**：存储结构体数据时注意内存对齐问题
3. **线程安全**：当前实现不是线程安全的，多线程环境需要额外同步
4. **错误处理**：务必检查所有API的返回值
5. **内存泄漏**：动态队列使用完毕后必须调用QueueDelete

## 性能特性

- **时间复杂度**：入队、出队操作均为O(1)
- **空间复杂度**：额外空间复杂度为O(1)
- **内存利用率**：循环缓冲区设计，空间利用率接近100%

## 错误码

组件使用DS_Result_t统一错误码：
- `DS_SUCCESS` - 操作成功
- `DS_ERROR_NULL_POINTER` - 空指针错误
- `DS_ERROR_INVALID_PARAM` - 无效参数
- `DS_ERROR_FULL` - 队列已满
- `DS_ERROR_EMPTY` - 队列为空
- `DS_ERROR_INSUFFICIENT_SPACE` - 空间不足

## 版本信息

- **版本**: 1.0
- **作者**: N1ntyNine99
- **日期**: 2025-09-08
- **版权**: Copyright (c) 2025 EmbedKit Project
