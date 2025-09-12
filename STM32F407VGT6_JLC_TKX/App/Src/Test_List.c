#include "Test.h"

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
