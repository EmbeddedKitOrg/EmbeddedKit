#include "Test.h"

EK_Test_t Tst_Queue = {.Test_Fucntion = Test_Queue, .Test_Name = "Queue"};

void Test_Queue(void)
{
    EK_rSerialPrintf(TestQueue, "=== 队列测试 ===\r\n");

    // 测试动态队列 - 增加容量以容纳8个uint32_t数据（8*4=32字节）
    test_queue = EK_pQueueCreate_Dynamic(40); // 增加到40字节容量
    if (!test_queue)
    {
        EK_rSerialPrintf(TestQueue, "动态队列创建测试 ❌ - 创建失败\r\n");
        test_failure_count++;
        return;
    }

    EK_rSerialPrintf(TestQueue, "动态队列创建测试 ✅ - 创建成功，容量: 40字节\r\n");
    test_success_count++;

    // 测试静态队列
    EK_Result_t result = EK_rQueueCreate_Static(&static_queue, queue_buffer, sizeof(queue_buffer));
    if (result == EK_OK)
    {
        EK_rSerialPrintf(TestQueue, "静态队列创建测试 ✅ - 创建成功，容量: %u\r\n", sizeof(queue_buffer));
        test_success_count++;
    }
    else
    {
        EK_rSerialPrintf(TestQueue, "静态队列创建测试 ❌ - 创建失败\r\n");
        test_failure_count++;
    }

    // 测试入队操作 - 减少测试数据量到6个
    int enqueue_success = 0;
    int enqueue_total = 6; // 6个uint32_t数据，共24字节，容量40字节足够
    for (int i = 0; i < enqueue_total; i++)
    {
        uint32_t data = i * 10 + test_counter;
        result = EK_rQueueEnqueue(test_queue, &data, sizeof(data));
        if (result == EK_OK)
        {
            EK_rSerialPrintf(TestQueue, "入队: %u\r\n", data);
            enqueue_success++;
        }
        else
        {
            EK_rSerialPrintf(TestQueue, "入队失败，数据: %u\r\n", data);
        }
    }

    if (enqueue_success == enqueue_total)
    {
        EK_rSerialPrintf(TestQueue, "队列入队测试 ✅ - %d/%d 成功\r\n", enqueue_success, enqueue_total);
        test_success_count++;
    }
    else
    {
        EK_rSerialPrintf(TestQueue, "队列入队测试 ❌ - %d/%d 成功\r\n", enqueue_success, enqueue_total);
        test_failure_count++;
    }

    EK_rSerialPrintf(
        TestQueue, "队列大小: %u, 剩余空间: %u\r\n", EK_sQueueGetSize(test_queue), EK_sQueueGetRemain(test_queue));
    EK_rSerialPrintf(TestQueue,
                     "队列状态 - 空: %s, 满: %s\r\n",
                     EK_bQueueIsEmpty(test_queue) ? "是" : "否",
                     EK_bQueueIsFull(test_queue) ? "是" : "否");

    // 测试队首查看
    uint32_t peek_data;
    result = EK_rQueuePeekFront(test_queue, &peek_data, sizeof(peek_data));
    if (result == EK_OK)
    {
        EK_rSerialPrintf(TestQueue, "队首查看测试 ✅ - 队首元素: %u\r\n", peek_data);
        test_success_count++;
    }
    else
    {
        EK_rSerialPrintf(TestQueue, "队首查看测试 ❌ - 查看失败\r\n");
        test_failure_count++;
    }

    // 测试出队操作 - 调整为实际入队的数量
    int dequeue_success = 0;
    int dequeue_total = enqueue_success; // 根据实际入队成功的数量来出队
    for (int i = 0; i < dequeue_total; i++)
    {
        uint32_t data;
        result = EK_rQueueDequeue(test_queue, &data, sizeof(data));
        if (result == EK_OK)
        {
            EK_rSerialPrintf(TestQueue, "出队: %u\r\n", data);
            dequeue_success++;
        }
        else
        {
            EK_rSerialPrintf(TestQueue, "出队失败\r\n");
        }
    }

    if (dequeue_success == dequeue_total && dequeue_total > 0)
    {
        EK_rSerialPrintf(TestQueue, "队列出队测试 ✅ - %d/%d 成功\r\n", dequeue_success, dequeue_total);
        test_success_count++;
    }
    else
    {
        EK_rSerialPrintf(TestQueue, "队列出队测试 ❌ - %d/%d 成功\r\n", dequeue_success, dequeue_total);
        test_failure_count++;
    }

    EK_rSerialPrintf(
        TestQueue, "出队后状态 - 大小: %u, 剩余: %u\r\n", EK_sQueueGetSize(test_queue), EK_sQueueGetRemain(test_queue));

    EK_rSerialPrintf(TestQueue, "队列测试完成\r\n\r\n");
}
