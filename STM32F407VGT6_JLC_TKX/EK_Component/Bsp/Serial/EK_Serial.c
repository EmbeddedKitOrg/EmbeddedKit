/**
 * @file EK_Serial.c
 * @brief 串口数据收发管理组件
 * @details
 *  - 本组件提供一个统一的串口数据发送管理机制。
 *  - 它使用一个队列来缓冲待发送的数据，并通过轮询方式发送。
 *  - 支持动态和静态方式创建和初始化。
 *  - 数据通过 `EK_rSerialPrintf` 格式化后进入队列。
 *  - 每个串口队列可以绑定一个发送回调函数 `Serial_SendCallBack`，用于实际的硬件发送操作。
 * @author N1netyNine99
 * @date 2025-09-13
 * @version 1.0
 */
#include "EK_Serial.h"

/* ========================= 宏定义区 ========================= */

/**
 * @brief 串口轮询发送的默认间隔时间（单位：ms）
 * @details 当串口队列中有数据时，每隔这么久发送一次数据。
 */
#ifndef SERIAL_POLL_TIMER
#define SERIAL_POLL_TIMER (10)
#endif

/* ========================= 内部变量区 ========================= */

static EK_List_t *SerialManageList; // 链表管理所有的队列
static bool SerialIsInit = false; // 是否初始化

/* ========================= 公用API函数定义区 ========================= */

/**
 * @brief 动态初始化串口管理组件
 * @details
 *  - 动态分配内存创建一个链表，用于管理所有串口队列。
 *  - 此函数为全局初始化，只能调用一次。
 * @return EK_Result_t 操作结果
 * @retval EK_OK 初始化成功
 * @retval EK_ERROR 已经初始化过，或链表创建失败
 */
EK_Result_t EK_rSerialInit_Dynamic(void)
{
    // 全局只能初始化一次
    if (SerialIsInit == true) return EK_ERROR;

    // 使用动态创建一个链表
    SerialManageList = EK_pListCreate_Dynamic();
    if (SerialManageList == NULL) return EK_ERROR;

    // 初始化标志位置位
    SerialIsInit = true;
    return EK_OK;
}

/**
 * @brief 静态初始化串口管理组件
 * @details
 *  - 使用静态分配的内存来初始化链表，用于管理所有串口队列。
 *  - 此函数为全局初始化，只能调用一次。
 * @return EK_Result_t 操作结果
 * @retval EK_OK 初始化成功
 * @retval EK_ERROR 已经初始化过，或链表创建失败
 */
EK_Result_t EK_rSerialInit_Static(void)
{
    // 全局只能初始化一次
    if (SerialIsInit == true) return EK_ERROR;

    // 使用静态创建一个链表
    static EK_Node_t *SerialDummyNode;
    EK_Result_t res = EK_OK;
    res = EK_rListCreate_Static(SerialManageList, SerialDummyNode);
    // 初始化标志位置位
    if (res == EK_OK) SerialIsInit = true;
    return res;
}

/**
 * @brief 动态创建一个串口队列实例
 * @details
 *  - 动态分配 `EK_SeiralQueue_t` 结构体、内部队列和链表节点的内存。
 *  - 将创建的实例（通过其所属节点）添加至管理链表中。
 *  - 节点的 `Node_Data` 指向 `EK_SeiralQueue_t` 实例。
 * @param serial_fifo 指向串口队列句柄指针的指针，函数将为它分配内存。
 * @param send_func 用于发送数据的回调
 * @param priority 队列的优先级，用于在管理链表中排序。
 * @param capacity 队列的容量（可以存储的字节数）。
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rSerialCreateQueue_Dyanmic(EK_pSeiralQueue_t *serial_fifo,
                                          void (*send_func)(void *, size_t),
                                          uint16_t priority,
                                          size_t capacity)
{
    // 判断有无初始化
    if (SerialIsInit == false) return EK_NOT_INITIALIZED;

    // 参数检查
    if (serial_fifo == NULL || send_func == NULL) return EK_NULL_POINTER;

    // 分配结构体空间
    *serial_fifo = (EK_SeiralQueue_t *)EK_MALLOC(sizeof(EK_SeiralQueue_t));
    if (*serial_fifo == NULL)
    {
        return EK_NO_MEMORY;
    }

    // 分配队列空间
    (*serial_fifo)->Serial_Queue = EK_pQueueCreate_Dynamic(capacity);
    if ((*serial_fifo)->Serial_Queue == NULL)
    {
        EK_FREE(*serial_fifo);
        *serial_fifo = NULL;
        return EK_NO_MEMORY;
    }

    // 分配链表节点空间
    // 让节点的Data指向当前的EK_SeiralQueue_t结构体
    (*serial_fifo)->Serial_Owner = EK_pNodeCreate_Dynamic(*serial_fifo, priority);
    if ((*serial_fifo)->Serial_Owner == NULL)
    {
        // 需要释放之前分配的队列内存
        EK_rQueueDelete((*serial_fifo)->Serial_Queue);
        EK_FREE(*serial_fifo);
        *serial_fifo = NULL;
        return EK_NO_MEMORY;
    }

    // 设置回调
    (*serial_fifo)->Serial_SendCallBack = send_func;

    // 设置倒计时
    (*serial_fifo)->Serial_Timer = SERIAL_POLL_TIMER;

    // 将节点插入管理链表
    return EK_rListInsertOrder(SerialManageList, (*serial_fifo)->Serial_Owner);
}

/**
 * @brief 静态创建一个串口队列实例
 * @details
 *  - 使用用户提供的内存（`serial_fifo` 和 `buffer`）来初始化一个串口队列实例。
 *  - 将创建的实例（通过其所属节点）添加至管理链表中。
 *  - 节点的 `Node_Data` 指向 `EK_SeiralQueue_t` 实例。
 * @param serial_fifo 指向用户分配的 `EK_SeiralQueue_t` 结构体实例的指针。
 * @param buffer 用户为队列分配的缓冲区。
 * @param send_func 用于发送数据的回调
 * @param priority 队列的优先级，用于在管理链表中排序。
 * @param capacity 队列的容量（`buffer` 的大小）。
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rSerialCreateQueue_Static(
    EK_pSeiralQueue_t serial_fifo, void *buffer, void (*send_func)(void *, size_t), uint16_t priority, size_t capacity)
{
    // 判断有无初始化
    if (SerialIsInit == false) return EK_NOT_INITIALIZED;

    // 参数检查
    if (serial_fifo == NULL || send_func == NULL) return EK_NULL_POINTER;

    // 初始化队列
    EK_Result_t res = EK_rQueueCreate_Static(serial_fifo->Serial_Queue, buffer, capacity);
    if (res != EK_OK) return res;

    // 初始化节点
    // 让节点的Data指向当前的EK_SeiralQueue_t结构体
    res = EK_rNodeCreate_Static(serial_fifo->Serial_Owner, serial_fifo, priority);
    if (res != EK_OK) return res;

    // 设置回调
    serial_fifo->Serial_SendCallBack = send_func;

    // 设置倒计时
    serial_fifo->Serial_Timer = SERIAL_POLL_TIMER;

    // 将节点插入管理链表
    return EK_rListInsertOrder(SerialManageList, serial_fifo->Serial_Owner);
}

/**
 * @brief 向指定的串口队列发送格式化数据
 * @details
 *  - 此函数类似 `printf`，它会将格式化的字符串存入一个临时缓冲区。
 *  - 然后将该缓冲区的数据入队，等待 `EK_rSerialPoll` 函数轮询发送。
 * @param serial_fifo 目标串口队列的句柄。
 * @param buffer_size 临时缓冲区的最大大小。
 * @param format 格式化字符串。
 * @param ... 可变参数。
 * @return EK_Result_t 操作结果
 * @retval 其他 入队失败
 * @warning 传入的数据大小不能大于 `buffer_size`，如果过大会导致内存池滥用。
 */
EK_Result_t EK_rSerialPrintf(EK_pSeiralQueue_t serial_fifo, size_t buffer_size, const char *format, ...)
{
    // 判断有无初始化
    if (SerialIsInit == false) return EK_NOT_INITIALIZED;

    // 检查队列是否满了
    if (EK_bQueueIsFull(serial_fifo->Serial_Queue)) return EK_FULL;

    // 检测剩余空间是否足够
    if (EK_sQueueGetRemain(serial_fifo->Serial_Queue) < buffer_size) return EK_INSUFFICIENT_SPACE;

    // 动态分配缓冲区
    char *buffer = (char *)EK_MALLOC(buffer_size);
    if (buffer == NULL) return EK_NO_MEMORY;

    uint16_t len = 0;
    va_list args;
    va_start(args, format);
    len = vsnprintf(buffer, buffer_size, format, args);
    va_end(args);

    // 如果vsnprintf的返回值大于或等于缓冲区大小，这意味着输出被截断。
    // 在这种情况下，我们应该只将缓冲区大小的字节数入队。
    size_t enqueue_len = (len >= buffer_size) ? (buffer_size - 1) : len;

    // 将数据写入队列
    bool was_empty = EK_bQueueIsEmpty(serial_fifo->Serial_Queue);
    EK_Result_t res = EK_rQueueEnqueue(serial_fifo->Serial_Queue, buffer, enqueue_len);

    // 如果队列之前是空的，现在成功加入了数据，则启动超时定时器
    if (was_empty && res == EK_OK)
    {
        serial_fifo->Serial_Timer = SERIAL_POLL_TIMER;
    }

    // 立即释放缓冲区
    EK_FREE(buffer);

    return res;
}

/**
 * @brief 串口服务轮询函数
 * @details
 *  - 此函数应被周期性地（例如，在1ms的定时器中断或主循环中）调用。
 *  - 它会遍历所有已创建的串口队列。
 *  - 对于非空队列，它会进行倒计时。
 *  - 倒计时结束后，它会从队列中取出所有数据，并通过注册的回调函数 `Serial_SendCallBack` 发送出去。
 * @param get_tick 一个函数指针，用于获取当前的系统时间（tick）。
 * @return EK_Result_t 操作结果
 * @retval EK_OK 轮询成功
 */
EK_Result_t EK_rSerialPoll(uint32_t (*get_tick)(void))
{
    static uint32_t last_tick = 0;

    if (get_tick == NULL) return EK_NULL_POINTER;

    // 判断有无初始化
    if (SerialIsInit == false) return EK_NOT_INITIALIZED;

    // 检测是否为空
    if (SerialManageList->List_Count == 0) return EK_EMPTY;

    // 获取链表头
    EK_Node_t *curr = EK_pListGetHead(SerialManageList);
    if (curr == NULL) return EK_NULL_POINTER;

    // 10ms遍历一次
    if (get_tick() - last_tick > 10)
    {
        uint16_t loop_counter = 0; // 循环计数器

        while (curr != SerialManageList->List_Dummy && loop_counter < SerialManageList->List_Count)
        {
            // 当前节点的内容 即这个节点包含的EK_SeiralQueue_t结构体
            EK_SeiralQueue_t *curr_data = (EK_SeiralQueue_t *)(curr->Node_Data);

            // 添加数据有效性检查
            if (curr_data == NULL || curr_data->Serial_Queue == NULL)
            {
                // 跳过无效节点
                loop_counter++;
                curr = curr->Node_Next;
                continue;
            }

            // 如果当前的队列为空，直接跳过
            if (EK_bQueueIsEmpty(curr_data->Serial_Queue) == true)
            {
                // 移动到下一个节点
                loop_counter++;
                curr = curr->Node_Next;
                continue;
            }

            // 倒计时没清空
            if (curr_data->Serial_Timer > 0)
            {
                curr_data->Serial_Timer -= 10;
            }
            else //倒计时被清空
            {
                // 获取队列中实际存储的数据大小
                size_t queue_used_size = EK_sQueueGetSize(curr_data->Serial_Queue);

                // 分配缓冲区来存储出队的数据
                void *buffer = EK_MALLOC(queue_used_size);
                if (buffer == NULL)
                {
                    // 内存分配失败，清空队列
                    EK_rQueueClean(curr_data->Serial_Queue);
                    // 重置倒计时
                    curr_data->Serial_Timer = SERIAL_POLL_TIMER;
                    // 计数值增加
                    loop_counter++;
                    // 移动到下一个节点
                    curr = curr->Node_Next;
                    continue;
                }

                // 取出队列中的所有数据
                EK_Result_t res = EK_rQueueDequeue(curr_data->Serial_Queue, buffer, queue_used_size);

                if (res != EK_OK)
                {
                    // 出队失败，释放缓冲区并清空队列
                    EK_FREE(buffer);
                    EK_rQueueClean(curr_data->Serial_Queue);
                }
                else
                {
                    // 发送数据
                    curr_data->Serial_SendCallBack(buffer, queue_used_size);
                    // 释放数据
                    EK_FREE(buffer);
                }

                // 重置倒计时
                curr_data->Serial_Timer = SERIAL_POLL_TIMER;
            }

            // 计数值增加
            loop_counter++;
            // 移动到下一个节点
            curr = curr->Node_Next;
        }
        // 更新计时器
        last_tick = get_tick();
    }
    return EK_OK;
}
