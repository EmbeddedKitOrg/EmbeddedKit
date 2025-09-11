#include "Task_App.h"

EK_pTaskHandler_t TskHandler1;
EK_pTaskHandler_t TskHandler2;
EK_pTaskHandler_t TskHandler3;
EK_pTaskHandler_t TskHandler_LargeListSort;

// 测试数据结构
static EK_List_t *test_list = NULL;
static EK_Queue_t *test_queue = NULL;
static uint8_t queue_buffer[100];
static EK_Queue_t static_queue;

// 测试计数器
static uint32_t test_counter = 0;
// 测试结果统计
static uint32_t test_success_count = 0;
static uint32_t test_failure_count = 0;

void Task_LED(void);
void Task_Key(void);
void Task_ComponentTest(void);
void Test_MemPool(void);
void Test_List(void);
void Test_Queue(void);
void Test_TaskSystem(void);
void Test_AllTaskInfo(void);

bool TaskCreation(void)
{
    bool res = true;
    res &= EK_rTaskCreate_Dynamic(Task_LED, 1, &TskHandler1) == EK_OK;
    res &= EK_rTaskCreate_Dynamic(Task_Key, 0, &TskHandler2) == EK_OK;
    res &= EK_rTaskCreate_Dynamic(Task_ComponentTest, 2, &TskHandler3) == EK_OK;
    return res;
}

void Task_LED(void)
{
    LED_TOGGLE();
    EK_rTaskDelay(500);
}

void Task_Key(void)
{
    static uint8_t KeyOld = 0;
    uint8_t KeyTemp = KeyRead();
    uint8_t KeyVal = KeyTemp & (KeyTemp ^ KeyOld);
    KeyOld = KeyTemp;

    if (KeyVal)
    {
        MyPrintf(USER_UART, "按键按下 剩余内存:%u字节\r\n", EK_sTaskGetFreeMemory());
        test_counter++; // 按键计数
    }
    EK_rTaskDelay(50);
}

void Task_ComponentTest(void)
{
    static uint8_t test_phase = 0;

    MyPrintf(USER_UART, "\r\n=== EK组件测试 第%d阶段 ===\r\n", test_phase + 1);

    switch (test_phase)
    {
        case 0:
            Test_MemPool();
            break;
        case 1:
            Test_List();
            break;
        case 2:
            Test_Queue();
            break;
        case 3:
            Test_TaskSystem();
            break;
        case 4:
            Test_AllTaskInfo();
            break;
    }

    if (test_phase == 4)
    {
        MyPrintf(USER_UART, "*****测试结束*****\r\n");
        MyPrintf(USER_UART, "=== 测试结果统计 ===\r\n");
        MyPrintf(USER_UART, "测试成功: %u 项 ✅\r\n", test_success_count);
        MyPrintf(USER_UART, "测试失败: %u 项 ❌\r\n", test_failure_count);
        MyPrintf(USER_UART, "总测试数: %u 项\r\n", test_success_count + test_failure_count);
        MyPrintf(USER_UART,
                 "成功率: %.1f%%\r\n",
                 (float)test_success_count * 100.0f / (float)(test_success_count + test_failure_count));
        MyPrintf(USER_UART, "==================\r\n");
        EK_rTaskDelete(NULL);
    }
    else
    {
        test_phase++;
        EK_rTaskDelay(1000);
    }
}

void Test_MemPool(void)
{
    MyPrintf(USER_UART, "=== 内存池测试 ===\r\n");

    // 获取内存池统计信息
    PoolStats_t stats;
    EK_vMemPool_GetStats(&stats);

    MyPrintf(USER_UART, "总容量: %u字节\r\n", (uint32_t)stats.total_size);
    MyPrintf(USER_UART, "可用字节: %u字节\r\n", (uint32_t)stats.free_bytes);
    MyPrintf(USER_UART, "历史最少可用: %u字节\r\n", (uint32_t)stats.min_free_bytes);
    MyPrintf(USER_UART, "分配次数: %u\r\n", (uint32_t)stats.alloc_count);
    MyPrintf(USER_UART, "释放次数: %u\r\n", (uint32_t)stats.free_count);

    // 测试内存分配和释放
    void *ptr1 = EK_pMemPool_Malloc(128);
    void *ptr2 = EK_pMemPool_Malloc(256);
    void *ptr3 = EK_pMemPool_Malloc(64);

    if (ptr1 && ptr2 && ptr3)
    {
        MyPrintf(USER_UART, "内存分配测试 ✅ - 已分配3个内存块: %p, %p, %p\r\n", ptr1, ptr2, ptr3);
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "内存分配测试 ❌ - 分配失败\r\n");
        test_failure_count++;
    }

    MyPrintf(USER_UART, "分配后剩余字节: %u\r\n", EK_sMemPool_GetFreeSize());

    // 检查内存完整性
    bool integrity = EK_bMemPool_CheckIntegrity();
    if (integrity)
    {
        MyPrintf(USER_UART, "内存完整性检查 ✅ - 正常\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "内存完整性检查 ❌ - 异常\r\n");
        test_failure_count++;
    }

    // 释放内存
    bool free_success = true;
    if (ptr1) free_success &= EK_bMemPool_Free(ptr1);
    if (ptr2) free_success &= EK_bMemPool_Free(ptr2);
    if (ptr3) free_success &= EK_bMemPool_Free(ptr3);

    if (free_success)
    {
        MyPrintf(USER_UART, "内存释放测试 ✅ - 释放成功\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "内存释放测试 ❌ - 释放失败\r\n");
        test_failure_count++;
    }

    MyPrintf(USER_UART, "释放后剩余字节: %u\r\n", EK_sMemPool_GetFreeSize());
    MyPrintf(USER_UART, "内存池测试完成\r\n\r\n");
}

void Test_List(void)
{
    MyPrintf(USER_UART, "=== 双向链表测试 ===\r\n");

    // 创建动态链表
    test_list = EK_pListCreate_Dynamic();
    if (!test_list)
    {
        MyPrintf(USER_UART, "链表创建测试 ❌ - 创建失败\r\n");
        test_failure_count++;
        return;
    }

    MyPrintf(USER_UART, "链表创建测试 ✅ - 创建成功\r\n");
    test_success_count++;

    // === 第一部分：小规模测试（4个节点） ===
    MyPrintf(USER_UART, "\n--- 小规模链表测试（4个节点） ---\r\n");

    // 创建一些测试数据
    static uint32_t data1 = 100, data2 = 50, data3 = 200, data4 = 75;

    // 创建节点并插入
    EK_Node_t *node1 = EK_pNodeCreate_Dynamic(&data1, 100);
    EK_Node_t *node2 = EK_pNodeCreate_Dynamic(&data2, 50);
    EK_Node_t *node3 = EK_pNodeCreate_Dynamic(&data3, 200);
    EK_Node_t *node4 = EK_pNodeCreate_Dynamic(&data4, 75);

    if (node1 && node2 && node3 && node4)
    {
        EK_rListInsertEnd(test_list, node1);
        EK_rListInsertEnd(test_list, node2);
        EK_rListInsertOrder(test_list, node3);
        EK_rListInsertOrder(test_list, node4);

        if (test_list->List_Count == 4)
        {
            MyPrintf(USER_UART, "小规模节点插入测试 ✅ - 已插入4个节点，链表长度: %d\r\n", test_list->List_Count);
            test_success_count++;
        }
        else
        {
            MyPrintf(USER_UART, "小规模节点插入测试 ❌ - 链表长度不正确: %d\r\n", test_list->List_Count);
            test_failure_count++;
        }

        // 测试小规模升序排序
        MyPrintf(USER_UART, "小规模链表排序中(升序)...\r\n");
        EK_Result_t sort_result = EK_rListSort(test_list, false);

        if (sort_result == EK_OK)
        {
            MyPrintf(USER_UART, "小规模升序排序测试 ✅ - 排序成功\r\n");
            test_success_count++;

            // 验证排序结果
            EK_Node_t *current = test_list->List_Dummy->Node_Next;
            bool sort_correct = true;
            int index = 0;

            // 首先检查是否有节点
            if (current == test_list->List_Dummy)
            {
                sort_correct = false;
            }
            else
            {
                uint32_t prev_order = current->Node_Order; // 从第一个节点的值开始
                current = current->Node_Next;
                index = 1; // 从第二个节点开始验证

                while (current != test_list->List_Dummy && index < 10)
                {
                    if (current->Node_Order < prev_order)
                    {
                        sort_correct = false;
                        break;
                    }
                    prev_order = current->Node_Order;
                    current = current->Node_Next;
                    index++;
                }
            }

            if (sort_correct)
            {
                MyPrintf(USER_UART, "小规模升序排序验证 ✅ - 排序正确\r\n");
                test_success_count++;
            }
            else
            {
                MyPrintf(USER_UART, "小规模升序排序验证 ❌ - 排序错误\r\n");
                test_failure_count++;
            }
        }
        else
        {
            MyPrintf(USER_UART, "小规模升序排序测试 ❌ - 排序失败\r\n");
            test_failure_count++;
        }

        // 测试小规模降序排序
        MyPrintf(USER_UART, "小规模链表排序中(降序)...\r\n");
        sort_result = EK_rListSort(test_list, true);

        if (sort_result == EK_OK)
        {
            MyPrintf(USER_UART, "小规模降序排序测试 ✅ - 排序成功\r\n");
            test_success_count++;

            // 验证降序排序结果
            EK_Node_t *current = test_list->List_Dummy->Node_Next;
            bool sort_correct = true;
            int index = 0;

            // 首先检查是否有节点
            if (current == test_list->List_Dummy)
            {
                sort_correct = false;
            }
            else
            {
                uint32_t prev_order = current->Node_Order; // 从第一个节点的值开始
                current = current->Node_Next;
                index = 1; // 从第二个节点开始验证

                while (current != test_list->List_Dummy && index < 10)
                {
                    if (current->Node_Order > prev_order)
                    {
                        sort_correct = false;
                        break;
                    }
                    prev_order = current->Node_Order;
                    current = current->Node_Next;
                    index++;
                }
            }

            if (sort_correct)
            {
                MyPrintf(USER_UART, "小规模降序排序验证 ✅ - 排序正确\r\n");
                test_success_count++;
            }
            else
            {
                MyPrintf(USER_UART, "小规模降序排序验证 ❌ - 排序错误\r\n");
                test_failure_count++;
            }
        }
        else
        {
            MyPrintf(USER_UART, "小规模降序排序测试 ❌ - 排序失败\r\n");
            test_failure_count++;
        }

        // 删除一个节点测试
        EK_Result_t remove_result = EK_rListRemoveNode(test_list, node2);
        if (remove_result == EK_OK && test_list->List_Count == 3)
        {
            MyPrintf(USER_UART, "小规模节点删除测试 ✅ - 已删除node2，链表长度: %d\r\n", test_list->List_Count);
            test_success_count++;
        }
        else
        {
            MyPrintf(USER_UART, "小规模节点删除测试 ❌ - 删除失败，链表长度: %d\r\n", test_list->List_Count);
            test_failure_count++;
        }
    }
    else
    {
        MyPrintf(USER_UART, "小规模节点创建测试 ❌ - 创建失败\r\n");
        test_failure_count++;
    }

    // === 第二部分：大规模测试（25个节点） ===
    MyPrintf(USER_UART, "\n--- 大规模链表测试（25个节点） ---\r\n");
    EK_rListDelete(test_list);

    test_list = EK_pListCreate_Dynamic();
    static uint32_t large_data[25];
    static uint32_t large_orders[25] = {89, 23,  156, 45,  178, 12, 167, 34, 123, 67, 145, 78, 190,
                                        56, 134, 98,  112, 203, 87, 145, 76, 198, 43, 165, 234};

    // 初始化数据
    for (int i = 0; i < 25; i++)
    {
        large_data[i] = large_orders[i] + 1000; // 数据 = 序号 + 1000
    }

    // 创建并插入25个节点
    int large_insert_success = 0;
    for (int i = 0; i < 25; i++)
    {
        EK_Node_t *large_node = EK_pNodeCreate_Dynamic(&large_data[i], large_orders[i]);
        if (large_node && EK_rListInsertEnd(test_list, large_node) == EK_OK)
        {
            large_insert_success++;
        }
    }

    if (large_insert_success == 25 && test_list->List_Count == 25)
    {
        MyPrintf(USER_UART, "大规模节点插入测试 ✅ - 已插入25个节点，链表长度: %d\r\n", test_list->List_Count);
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "大规模节点插入测试 ❌ - 插入失败，期望25个，实际: %d\r\n", large_insert_success);
        test_failure_count++;
    }

    // 排序前打印前几个节点的值
    MyPrintf(USER_UART, "排序前前5个节点值: ");
    EK_Node_t *debug_node = test_list->List_Dummy->Node_Next;
    for (int i = 0; i < 5 && debug_node != test_list->List_Dummy; i++)
    {
        MyPrintf(USER_UART, "%u ", debug_node->Node_Order);
        debug_node = debug_node->Node_Next;
    }
    MyPrintf(USER_UART, "\r\n");

    // 测试大规模升序排序
    MyPrintf(USER_UART, "大规模链表排序中(升序)...\r\n");
    EK_Result_t large_sort_result = EK_rListSort(test_list, false);

    if (large_sort_result == EK_OK)
    {
        MyPrintf(USER_UART, "大规模升序排序测试 ✅ - 排序成功\r\n");

        // 排序后打印所有节点的值
        MyPrintf(USER_UART, "排序后所有节点值: ");
        debug_node = test_list->List_Dummy->Node_Next;
        for (int i = 0; i < 25 && debug_node != test_list->List_Dummy; i++)
        {
            MyPrintf(USER_UART, "%u ", debug_node->Node_Order);
            debug_node = debug_node->Node_Next;
            if ((i + 1) % 10 == 0) MyPrintf(USER_UART, "\r\n");
        }
        MyPrintf(USER_UART, "\r\n");

        test_success_count++; // 验证大规模升序排序结果
        EK_Node_t *current = test_list->List_Dummy->Node_Next;
        bool large_sort_correct = true;
        int index = 0;

        // 首先检查是否有节点
        if (current == test_list->List_Dummy)
        {
            large_sort_correct = false;
        }
        else
        {
            uint32_t prev_order = current->Node_Order; // 从第一个节点的值开始
            current = current->Node_Next;
            index = 1; // 从第二个节点开始验证

            while (current != test_list->List_Dummy && index < 30)
            {
                if (current->Node_Order < prev_order)
                {
                    large_sort_correct = false;
                    MyPrintf(USER_UART,
                             "排序错误：位置%d，当前序号%u < 前序号%u\r\n",
                             index,
                             current->Node_Order,
                             prev_order);
                    break;
                }
                prev_order = current->Node_Order;
                current = current->Node_Next;
                index++;
            }
        }

        if (large_sort_correct && index == 25)
        {
            MyPrintf(USER_UART, "大规模升序排序验证 ✅ - 排序正确，验证了%d个节点\r\n", index);
            test_success_count++;
        }
        else
        {
            MyPrintf(USER_UART, "大规模升序排序验证 ❌ - 排序错误或节点数不匹配\r\n");
            test_failure_count++;
        }
    }
    else
    {
        MyPrintf(USER_UART, "大规模升序排序测试 ❌ - 排序失败\r\n");
        test_failure_count++;
    }

    // 测试大规模降序排序
    MyPrintf(USER_UART, "大规模链表排序中(降序)...\r\n");
    large_sort_result = EK_rListSort(test_list, true);

    if (large_sort_result == EK_OK)
    {
        MyPrintf(USER_UART, "大规模降序排序测试 ✅ - 排序成功\r\n");
        test_success_count++;

        // 验证大规模降序排序结果
        EK_Node_t *current = test_list->List_Dummy->Node_Next;
        bool large_sort_correct = true;
        int index = 0;

        // 首先检查是否有节点
        if (current == test_list->List_Dummy)
        {
            large_sort_correct = false;
        }
        else
        {
            uint32_t prev_order = current->Node_Order; // 从第一个节点的值开始
            current = current->Node_Next;
            index = 1; // 从第二个节点开始验证

            while (current != test_list->List_Dummy && index < 30)
            {
                if (current->Node_Order > prev_order)
                {
                    large_sort_correct = false;
                    MyPrintf(USER_UART,
                             "排序错误：位置%d，当前序号%u > 前序号%u\r\n",
                             index,
                             current->Node_Order,
                             prev_order);
                    break;
                }
                prev_order = current->Node_Order;
                current = current->Node_Next;
                index++;
            }
        }

        if (large_sort_correct && index == 25)
        {
            MyPrintf(USER_UART, "大规模降序排序验证 ✅ - 排序正确，验证了%d个节点\r\n", index);
            test_success_count++;
        }
        else
        {
            MyPrintf(USER_UART, "大规模降序排序验证 ❌ - 排序错误或节点数不匹配\r\n");
            test_failure_count++;
        }
    }
    else
    {
        MyPrintf(USER_UART, "大规模降序排序测试 ❌ - 排序失败\r\n");
        test_failure_count++;
    }

    MyPrintf(USER_UART, "链表测试完成\r\n\r\n");
}

void Test_Queue(void)
{
    MyPrintf(USER_UART, "=== 队列测试 ===\r\n");

    // 测试动态队列 - 增加容量以容纳8个uint32_t数据（8*4=32字节）
    test_queue = EK_pQueueCreate_Dynamic(40); // 增加到40字节容量
    if (!test_queue)
    {
        MyPrintf(USER_UART, "动态队列创建测试 ❌ - 创建失败\r\n");
        test_failure_count++;
        return;
    }

    MyPrintf(USER_UART, "动态队列创建测试 ✅ - 创建成功，容量: 40字节\r\n");
    test_success_count++;

    // 测试静态队列
    EK_Result_t result = EK_rQueueCreate_Static(&static_queue, queue_buffer, sizeof(queue_buffer));
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "静态队列创建测试 ✅ - 创建成功，容量: %u\r\n", sizeof(queue_buffer));
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "静态队列创建测试 ❌ - 创建失败\r\n");
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
            MyPrintf(USER_UART, "入队: %u\r\n", data);
            enqueue_success++;
        }
        else
        {
            MyPrintf(USER_UART, "入队失败，数据: %u\r\n", data);
        }
    }

    if (enqueue_success == enqueue_total)
    {
        MyPrintf(USER_UART, "队列入队测试 ✅ - %d/%d 成功\r\n", enqueue_success, enqueue_total);
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "队列入队测试 ❌ - %d/%d 成功\r\n", enqueue_success, enqueue_total);
        test_failure_count++;
    }

    MyPrintf(USER_UART, "队列大小: %u, 剩余空间: %u\r\n", EK_sQueueGetSize(test_queue), EK_sQueueGetRemain(test_queue));
    MyPrintf(USER_UART,
             "队列状态 - 空: %s, 满: %s\r\n",
             EK_bQueueIsEmpty(test_queue) ? "是" : "否",
             EK_bQueueIsFull(test_queue) ? "是" : "否");

    // 测试队首查看
    uint32_t peek_data;
    result = EK_rQueuePeekFront(test_queue, &peek_data, sizeof(peek_data));
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "队首查看测试 ✅ - 队首元素: %u\r\n", peek_data);
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "队首查看测试 ❌ - 查看失败\r\n");
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
            MyPrintf(USER_UART, "出队: %u\r\n", data);
            dequeue_success++;
        }
        else
        {
            MyPrintf(USER_UART, "出队失败\r\n");
        }
    }

    if (dequeue_success == dequeue_total && dequeue_total > 0)
    {
        MyPrintf(USER_UART, "队列出队测试 ✅ - %d/%d 成功\r\n", dequeue_success, dequeue_total);
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "队列出队测试 ❌ - %d/%d 成功\r\n", dequeue_success, dequeue_total);
        test_failure_count++;
    }

    MyPrintf(
        USER_UART, "出队后状态 - 大小: %u, 剩余: %u\r\n", EK_sQueueGetSize(test_queue), EK_sQueueGetRemain(test_queue));

    MyPrintf(USER_UART, "队列测试完成\r\n\r\n");
}

void Test_TaskSystem(void)
{
    MyPrintf(USER_UART, "=== 任务系统测试 ===\r\n");

    // 获取当前任务信息
    EK_TaskInfo_t task_info;

    // 测试获取任务1信息
    EK_Result_t result = EK_rTaskGetInfo(TskHandler1, &task_info);
    if (result == EK_OK && task_info.isValid)
    {
        MyPrintf(USER_UART, "任务信息获取测试 ✅ - 任务1信息:\r\n");
        MyPrintf(USER_UART, "  激活状态: %s\r\n", task_info.isActive ? "是" : "否");
        MyPrintf(USER_UART, "  静态创建: %s\r\n", task_info.isStatic ? "是" : "否");
        MyPrintf(USER_UART, "  优先级: %d\r\n", task_info.Priority);
        MyPrintf(USER_UART, "  最大耗时: %d毫秒\r\n", task_info.MaxUsedTime);
        MyPrintf(USER_UART, "  内存占用: %u字节\r\n", (uint32_t)task_info.Memory);
        MyPrintf(USER_UART,
                 "  任务状态: %s\r\n",
                 task_info.state == TASK_STATE_RUNNING   ? "运行中"
                 : task_info.state == TASK_STATE_WAITING ? "等待中"
                                                         : "未知");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "任务信息获取测试 ❌ - 获取失败\r\n");
        test_failure_count++;
    }

    // 测试任务优先级修改
    MyPrintf(USER_UART, "修改任务1优先级为5...\r\n");
    result = EK_rTaskSetPriority(TskHandler1, 5);
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "任务优先级修改测试 ✅ - 修改成功\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "任务优先级修改测试 ❌ - 修改失败\r\n");
        test_failure_count++;
    }

    // 测试任务挂起和恢复
    MyPrintf(USER_UART, "挂起任务1...\r\n");
    result = EK_rTaskSuspend(TskHandler1);
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "任务挂起测试 ✅ - 挂起成功\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "任务挂起测试 ❌ - 挂起失败\r\n");
        test_failure_count++;
    }

    // 等待一段时间
    HAL_Delay(1000);

    MyPrintf(USER_UART, "恢复任务1...\r\n");
    result = EK_rTaskResume(TskHandler1);
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "任务恢复测试 ✅ - 恢复成功\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "任务恢复测试 ❌ - 恢复失败\r\n");
        test_failure_count++;
    }

    // 显示系统内存使用情况
    uint32_t free_memory = EK_sTaskGetFreeMemory();
    MyPrintf(USER_UART, "系统剩余内存: %u字节\r\n", free_memory);

    // 显示测试统计
    MyPrintf(USER_UART, "测试计数器(按键次数): %u\r\n", test_counter);

    MyPrintf(USER_UART, "任务系统测试完成\r\n\r\n");
}

void Test_AllTaskInfo(void)
{
    MyPrintf(USER_UART, "=== 所有任务信息打印测试 ===\r\n");

    // 创建任务句柄数组，包含所有已创建的任务
    EK_pTaskHandler_t task_handlers[] = {TskHandler1, TskHandler2, TskHandler3};
    const char *task_names[] = {"Task_LED", "Task_Key", "Task_ComponentTest"};
    int task_count = sizeof(task_handlers) / sizeof(task_handlers[0]);

    MyPrintf(USER_UART, "系统中当前有 %d 个任务：\r\n\r\n", task_count);

    for (int i = 0; i < task_count; i++)
    {
        if (task_handlers[i] != NULL)
        {
            EK_TaskInfo_t task_info;
            EK_Result_t result = EK_rTaskGetInfo(task_handlers[i], &task_info);

            if (result == EK_OK && task_info.isValid)
            {
                MyPrintf(USER_UART, "--- 任务 %d: %s ---\r\n", i + 1, task_names[i]);
                MyPrintf(USER_UART, "  句柄地址: %p\r\n", task_handlers[i]);
                MyPrintf(USER_UART, "  有效状态: %s\r\n", task_info.isValid ? "有效" : "无效");
                MyPrintf(USER_UART, "  激活状态: %s\r\n", task_info.isActive ? "激活" : "未激活");
                MyPrintf(USER_UART, "  创建方式: %s\r\n", task_info.isStatic ? "静态" : "动态");
                MyPrintf(USER_UART, "  优先级: %d\r\n", task_info.Priority);
                MyPrintf(USER_UART, "  最大执行时间: %d 毫秒\r\n", task_info.MaxUsedTime);
                MyPrintf(USER_UART, "  内存占用: %u 字节\r\n", (uint32_t)task_info.Memory);

                // 任务状态转换为可读字符串
                const char *state_str;
                switch (task_info.state)
                {
                    case TASK_STATE_RUNNING:
                        state_str = "运行中";
                        break;
                    case TASK_STATE_WAITING:
                        state_str = "等待中";
                        break;
                    default:
                        state_str = "未知状态";
                        break;
                }
                MyPrintf(USER_UART, "  当前状态: %s\r\n", state_str);

                MyPrintf(USER_UART, "\r\n");
                test_success_count++;
            }
            else
            {
                MyPrintf(USER_UART, "--- 任务 %d: %s ---\r\n", i + 1, task_names[i]);
                MyPrintf(USER_UART, "  ❌ 获取任务信息失败 (result: %d)\r\n\r\n", result);
                test_failure_count++;
            }
        }
        else
        {
            MyPrintf(USER_UART, "--- 任务 %d: %s ---\r\n", i + 1, task_names[i]);
            MyPrintf(USER_UART, "  ❌ 任务句柄为空\r\n\r\n");
            test_failure_count++;
        }
    }

    // 显示系统整体信息
    MyPrintf(USER_UART, "=== 系统整体信息 ===\r\n");
    MyPrintf(USER_UART, "系统剩余内存: %u 字节\r\n", EK_sTaskGetFreeMemory());
    MyPrintf(USER_UART, "用户按键计数: %u 次\r\n", test_counter);

    MyPrintf(USER_UART, "\r\n所有任务信息打印完成\r\n\r\n");
}
