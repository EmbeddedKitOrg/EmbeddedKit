/**
 * @file test_list.c
 * @brief EK_List 链表模块测试程序
 * @details 测试双向链表的所有功能函数，并在终端显示链表变化过程
 * @author Test Program
 * @date 2025-01-09
 * @version 1.0
 */

#include "../DataStruct/List/EK_List.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================= 测试数据结构 ========================= */
typedef struct
{
    int value;
    char name[32];
} TestData_t;

/* ========================= 测试结果记录 ========================= */
typedef struct
{
    char test_name[64];
    bool passed;
} TestResult_t;

static TestResult_t test_results[20];
static int total_tests = 0;
static int passed_tests = 0;

/**
 * @brief 记录测试结果
 * @param test_name 测试名称
 * @param passed 是否通过
 */
void record_test_result(const char *test_name, bool passed)
{
    if (total_tests < 20)
    {
        strncpy(test_results[total_tests].test_name, test_name, sizeof(test_results[total_tests].test_name) - 1);
        test_results[total_tests].test_name[sizeof(test_results[total_tests].test_name) - 1] = '\0';
        test_results[total_tests].passed = passed;
        total_tests++;
        if (passed)
        {
            passed_tests++;
            printf("✅ %s: 通过\n", test_name);
        }
        else
        {
            printf("❌ %s: 失败\n", test_name);
        }
    }
}

/**
 * @brief 显示测试总结
 */
void show_test_summary()
{
    printf("\n\n========================================\n");
    printf("           测试总结报告\n");
    printf("========================================\n");
    printf("总测试数: %d\n", total_tests);
    printf("通过测试: %d\n", passed_tests);
    printf("失败测试: %d\n", total_tests - passed_tests);
    printf("通过率: %.1f%%\n", total_tests > 0 ? (float)passed_tests / total_tests * 100 : 0);
    printf("\n详细结果:\n");
    for (int i = 0; i < total_tests; i++)
    {
        printf("%s %s\n", test_results[i].passed ? "✅" : "❌", test_results[i].test_name);
    }
    printf("========================================\n");
}

/* ========================= 辅助函数 ========================= */

/**
 * @brief 打印链表状态
 * @param list 要打印的链表
 * @param title 标题
 */
void print_list(EK_List_t *list, const char *title)
{
    printf("\n=== %s ===\n", title);
    if (list == NULL)
    {
        printf("链表为空指针\n");
        return;
    }

    printf("链表节点数量: %d\n", list->List_Count);

    if (list->List_Count == 0)
    {
        printf("链表为空\n");
        return;
    }

    printf("链表内容 (从头到尾): ");
    EK_Node_t *current = list->List_Dummy->Node_Next;
    int index = 0;
    while (current != list->List_Dummy && index < 20)
    { // 防止无限循环
        TestData_t *data = (TestData_t *)current->Node_Data;
        if (data)
        {
            printf("[%d:%s(order:%u)] ", data->value, data->name, current->Node_Order);
        }
        else
        {
            printf("[NULL(order:%u)] ", current->Node_Order);
        }
        current = current->Node_Next;
        index++;
    }
    printf("\n");

    // 反向打印验证双向链表
    printf("链表内容 (从尾到头): ");
    current = list->List_Dummy->Node_Prev;
    index = 0;
    while (current != list->List_Dummy && index < 20)
    {
        TestData_t *data = (TestData_t *)current->Node_Data;
        if (data)
        {
            printf("[%d:%s(order:%u)] ", data->value, data->name, current->Node_Order);
        }
        else
        {
            printf("[NULL(order:%u)] ", current->Node_Order);
        }
        current = current->Node_Prev;
        index++;
    }
    printf("\n");
}

/**
 * @brief 创建测试数据
 * @param value 数值
 * @param name 名称
 * @return 测试数据指针
 */
TestData_t *create_test_data(int value, const char *name)
{
    TestData_t *data = (TestData_t *)malloc(sizeof(TestData_t));
    if (data)
    {
        data->value = value;
        strncpy(data->name, name, sizeof(data->name) - 1);
        data->name[sizeof(data->name) - 1] = '\0';
    }
    return data;
}

/**
 * @brief 打印操作结果
 * @param result 操作结果
 * @param operation 操作名称
 */
void print_result(EK_Result_t result, const char *operation)
{
    const char *result_str;
    switch (result)
    {
        case EK_OK:
            result_str = "成功";
            break;
        case EK_ERROR:
            result_str = "通用错误";
            break;
        case EK_INVALID_PARAM:
            result_str = "参数错误";
            break;
        case EK_TIMEOUT:
            result_str = "超时错误";
            break;
        case EK_NO_MEMORY:
            result_str = "内存不足";
            break;
        case EK_NOT_INITIALIZED:
            result_str = "未初始化";
            break;
        case EK_NOT_FOUND:
            result_str = "未找到";
            break;
        case EK_ALREADY_EXISTS:
            result_str = "已存在";
            break;
        case EK_FULL:
            result_str = "已满";
            break;
        case EK_EMPTY:
            result_str = "为空";
            break;
        case EK_INSUFFICIENT_SPACE:
            result_str = "空间不足";
            break;
        case EK_UNKNOWN:
            result_str = "未知错误";
            break;
        case EK_NULL_POINTER:
            result_str = "空指针错误";
            break;
        default:
            result_str = "未知状态";
            break;
    }
    printf("%s: %s\n", operation, result_str);
}

/**
 * @brief 等待用户按键继续
 */
void wait_for_key()
{
    printf("\n按回车键继续...");
    getchar();
}

/* ========================= 测试函数 ========================= */

/**
 * @brief 测试节点创建功能
 */
void test_node_creation()
{
    printf("\n\n========== 测试节点创建功能 ==========\n");
    bool test_passed = true;

    // 测试静态节点创建
    printf("\n--- 测试静态节点创建 ---\n");
    EK_Node_t static_node;
    TestData_t *data1 = create_test_data(100, "静态节点");
    EK_Result_t result = EK_rNodeCreate_Static(&static_node, data1, 100);
    print_result(result, "静态节点创建");

    if (result == EK_OK)
    {
        TestData_t *node_data = (TestData_t *)static_node.Node_Data;
        printf("节点数据: value=%d, name=%s, order=%u\n", node_data->value, node_data->name, static_node.Node_Order);
        if (node_data->value != 100 || static_node.Node_Order != 100)
        {
            test_passed = false;
        }
    }
    else
    {
        test_passed = false;
    }

    // 测试动态节点创建
    printf("\n--- 测试动态节点创建 ---\n");
    TestData_t *data2 = create_test_data(200, "动态节点");
    EK_Node_t *dynamic_node = EK_pNodeCreate_Dynamic(data2, 200);

    if (dynamic_node)
    {
        printf("动态节点创建: 成功\n");
        TestData_t *node_data = (TestData_t *)dynamic_node->Node_Data;
        printf("节点数据: value=%d, name=%s, order=%u\n", node_data->value, node_data->name, dynamic_node->Node_Order);
        if (node_data->value != 200 || dynamic_node->Node_Order != 200)
        {
            test_passed = false;
        }

        // 清理动态节点
        free(dynamic_node->Node_Data);
        free(dynamic_node);
    }
    else
    {
        printf("动态节点创建: 失败\n");
        test_passed = false;
    }

    // 清理静态节点数据
    free(data1);

    record_test_result("节点创建功能测试", test_passed);
    wait_for_key();
}

/**
 * @brief 测试链表创建功能
 */
void test_list_creation()
{
    printf("\n\n========== 测试链表创建功能 ==========\n");
    bool test_passed = true;

    // 测试静态链表创建
    printf("\n--- 测试静态链表创建 ---\n");
    EK_List_t static_list;
    EK_Node_t dummy_node;
    EK_Result_t result = EK_rListCreate_Static(&static_list, &dummy_node);
    print_result(result, "静态链表创建");
    print_list(&static_list, "静态链表初始状态");

    if (result != EK_OK || static_list.List_Count != 0)
    {
        test_passed = false;
    }

    // 测试动态链表创建
    printf("\n--- 测试动态链表创建 ---\n");
    EK_List_t *dynamic_list = EK_pListCreate_Dynamic();

    if (dynamic_list)
    {
        printf("动态链表创建: 成功\n");
        print_list(dynamic_list, "动态链表初始状态");

        if (dynamic_list->List_Count != 0)
        {
            test_passed = false;
        }

        // 清理动态链表
        free(dynamic_list->List_Dummy);
        free(dynamic_list);
    }
    else
    {
        printf("动态链表创建: 失败\n");
        test_passed = false;
    }

    record_test_result("链表创建功能测试", test_passed);
    wait_for_key();
}

/**
 * @brief 测试链表插入功能
 */
void test_list_insertion()
{
    printf("\n\n========== 测试链表插入功能 ==========\n");
    bool test_passed = true;

    // 创建测试链表
    EK_List_t *list = EK_pListCreate_Dynamic();
    if (!list)
    {
        printf("链表创建失败，无法进行插入测试\n");
        record_test_result("链表插入功能测试", false);
        return;
    }

    print_list(list, "初始空链表");

    // 测试尾部插入
    printf("\n--- 测试尾部插入 ---\n");
    for (int i = 1; i <= 3; i++)
    {
        TestData_t *data = create_test_data(i * 10, "尾部节点");
        EK_Node_t *node = EK_pNodeCreate_Dynamic(data, i * 10);
        if (node)
        {
            EK_Result_t result = EK_rListInsertEnd(list, node);
            printf("插入节点 %d: ", i);
            print_result(result, "");
            print_list(list, "当前链表状态");
            if (result != EK_OK || list->List_Count != i)
            {
                test_passed = false;
            }
        }
        else
        {
            test_passed = false;
        }
    }

    wait_for_key();

    // 测试头部插入
    printf("\n--- 测试头部插入 ---\n");
    int expected_count = list->List_Count;
    for (int i = 1; i <= 2; i++)
    {
        TestData_t *data = create_test_data(i, "头部节点");
        EK_Node_t *node = EK_pNodeCreate_Dynamic(data, i);
        if (node)
        {
            EK_Result_t result = EK_rListInsertHead(list, node);
            printf("头部插入节点 %d: ", i);
            print_result(result, "");
            print_list(list, "当前链表状态");
            expected_count++;
            if (result != EK_OK || list->List_Count != expected_count)
            {
                test_passed = false;
            }
        }
        else
        {
            test_passed = false;
        }
    }

    wait_for_key();

    // 测试按序插入
    printf("\n--- 测试按序插入 ---\n");
    int orders[] = {15, 25, 5};
    expected_count = list->List_Count;
    for (int i = 0; i < 3; i++)
    {
        TestData_t *data = create_test_data(orders[i], "按序节点");
        EK_Node_t *node = EK_pNodeCreate_Dynamic(data, orders[i]);
        if (node)
        {
            EK_Result_t result = EK_rListInsertOrder(list, node);
            printf("按序插入节点 order=%d: ", orders[i]);
            print_result(result, "");
            print_list(list, "当前链表状态");
            expected_count++;
            if (result != EK_OK || list->List_Count != expected_count)
            {
                test_passed = false;
            }
        }
        else
        {
            test_passed = false;
        }
    }

    // 清理链表
    while (list->List_Count > 0)
    {
        EK_Node_t *node = list->List_Dummy->Node_Next;
        EK_rListRemoveNode(list, node);
        free(node->Node_Data);
        free(node);
    }
    free(list->List_Dummy);
    free(list);

    record_test_result("链表插入功能测试", test_passed);
    wait_for_key();
}

/**
 * @brief 测试链表删除功能
 */
void test_list_removal()
{
    printf("\n\n========== 测试链表删除功能 ==========\n");
    bool test_passed = true;

    // 创建测试链表并添加节点
    EK_List_t *list = EK_pListCreate_Dynamic();
    if (!list)
    {
        printf("链表创建失败，无法进行删除测试\n");
        record_test_result("链表删除功能测试", false);
        return;
    }

    // 添加测试节点
    EK_Node_t *nodes[5];
    for (int i = 0; i < 5; i++)
    {
        TestData_t *data = create_test_data((i + 1) * 10, "测试节点");
        nodes[i] = EK_pNodeCreate_Dynamic(data, (i + 1) * 10);
        if (nodes[i])
        {
            EK_rListInsertEnd(list, nodes[i]);
        }
    }

    print_list(list, "删除测试前的链表");

    // 测试删除中间节点
    printf("\n--- 删除中间节点 (第3个节点) ---\n");
    if (nodes[2])
    {
        int count_before = list->List_Count;
        EK_Result_t result = EK_rListRemoveNode(list, nodes[2]);
        print_result(result, "删除中间节点");
        print_list(list, "删除后的链表");
        if (result != EK_OK || list->List_Count != count_before - 1)
        {
            test_passed = false;
        }
        free(nodes[2]->Node_Data);
        free(nodes[2]);
        nodes[2] = NULL;
    }
    else
    {
        test_passed = false;
    }

    wait_for_key();

    // 测试删除头节点
    printf("\n--- 删除头节点 ---\n");
    if (nodes[0])
    {
        int count_before = list->List_Count;
        EK_Result_t result = EK_rListRemoveNode(list, nodes[0]);
        print_result(result, "删除头节点");
        print_list(list, "删除后的链表");
        if (result != EK_OK || list->List_Count != count_before - 1)
        {
            test_passed = false;
        }
        free(nodes[0]->Node_Data);
        free(nodes[0]);
        nodes[0] = NULL;
    }
    else
    {
        test_passed = false;
    }

    wait_for_key();

    // 测试删除尾节点
    printf("\n--- 删除尾节点 ---\n");
    if (nodes[4])
    {
        int count_before = list->List_Count;
        EK_Result_t result = EK_rListRemoveNode(list, nodes[4]);
        print_result(result, "删除尾节点");
        print_list(list, "删除后的链表");
        if (result != EK_OK || list->List_Count != count_before - 1)
        {
            test_passed = false;
        }
        free(nodes[4]->Node_Data);
        free(nodes[4]);
        nodes[4] = NULL;
    }
    else
    {
        test_passed = false;
    }

    // 清理剩余节点
    for (int i = 0; i < 5; i++)
    {
        if (nodes[i])
        {
            EK_rListRemoveNode(list, nodes[i]);
            free(nodes[i]->Node_Data);
            free(nodes[i]);
        }
    }

    free(list->List_Dummy);
    free(list);

    record_test_result("链表删除功能测试", test_passed);
    wait_for_key();
}

/**
 * @brief 测试链表移动功能
 */
void test_list_move()
{
    printf("\n\n========== 测试链表移动功能 ==========\n");
    bool test_passed = true;

    // 创建源链表和目标链表
    EK_List_t *src_list = EK_pListCreate_Dynamic();
    EK_List_t *dst_list = EK_pListCreate_Dynamic();

    if (!src_list || !dst_list)
    {
        printf("链表创建失败，无法进行移动测试\n");
        record_test_result("链表移动功能测试", false);
        return;
    }

    // 在源链表中添加节点
    EK_Node_t *move_node = NULL;
    for (int i = 1; i <= 4; i++)
    {
        TestData_t *data = create_test_data(i * 10, "源节点");
        EK_Node_t *node = EK_pNodeCreate_Dynamic(data, i * 10);
        if (node)
        {
            EK_rListInsertEnd(src_list, node);
            if (i == 2) move_node = node; // 记录要移动的节点
        }
    }

    // 在目标链表中添加一些节点
    for (int i = 1; i <= 2; i++)
    {
        TestData_t *data = create_test_data(i * 100, "目标节点");
        EK_Node_t *node = EK_pNodeCreate_Dynamic(data, i * 100);
        if (node)
        {
            EK_rListInsertEnd(dst_list, node);
        }
    }

    print_list(src_list, "移动前的源链表");
    print_list(dst_list, "移动前的目标链表");

    // 测试节点移动
    if (move_node)
    {
        printf("\n--- 移动节点 (order=20) 到目标链表 ---\n");
        int src_count_before = src_list->List_Count;
        int dst_count_before = dst_list->List_Count;
        EK_Result_t result = EK_rListMoveNode(src_list, dst_list, move_node, 150);
        print_result(result, "节点移动");

        print_list(src_list, "移动后的源链表");
        print_list(dst_list, "移动后的目标链表");

        if (result != EK_OK || src_list->List_Count != src_count_before - 1 ||
            dst_list->List_Count != dst_count_before + 1)
        {
            test_passed = false;
        }
    }
    else
    {
        test_passed = false;
    }

    // 清理链表
    while (src_list->List_Count > 0)
    {
        EK_Node_t *node = src_list->List_Dummy->Node_Next;
        EK_rListRemoveNode(src_list, node);
        free(node->Node_Data);
        free(node);
    }

    while (dst_list->List_Count > 0)
    {
        EK_Node_t *node = dst_list->List_Dummy->Node_Next;
        EK_rListRemoveNode(dst_list, node);
        free(node->Node_Data);
        free(node);
    }

    free(src_list->List_Dummy);
    free(src_list);
    free(dst_list->List_Dummy);
    free(dst_list);

    record_test_result("链表移动功能测试", test_passed);
    wait_for_key();
}

/**
 * @brief 测试链表排序功能
 */
void test_list_sort()
{
    printf("\n\n========== 测试链表排序功能 ==========\n");
    bool test_passed = true;

    // 创建测试链表
    EK_List_t *list = EK_pListCreate_Dynamic();
    if (!list)
    {
        printf("链表创建失败，无法进行排序测试\n");
        record_test_result("链表排序功能测试", false);
        return;
    }

    // 添加乱序节点
    int orders[] = {50, 20, 80, 10, 60, 30, 90, 40, 70};
    int count = sizeof(orders) / sizeof(orders[0]);

    for (int i = 0; i < count; i++)
    {
        TestData_t *data = create_test_data(orders[i], "排序节点");
        EK_Node_t *node = EK_pNodeCreate_Dynamic(data, orders[i]);
        if (node)
        {
            EK_rListInsertEnd(list, node);
        }
    }

    print_list(list, "排序前的链表");

    // 测试升序排序
    printf("\n--- 升序排序 ---\n");
    EK_Result_t result = EK_rListSort(list, false);
    print_result(result, "升序排序");
    print_list(list, "升序排序后的链表");

    // 验证升序排序结果
    bool asc_test_passed = true;
    if (result == EK_OK)
    {
        EK_Node_t *current = list->List_Dummy->Node_Next;
        while (current && current->Node_Next != list->List_Dummy)
        {
            if (current->Node_Order > current->Node_Next->Node_Order)
            {
                asc_test_passed = false;
                test_passed = false;
                break;
            }
            current = current->Node_Next;
        }
    }
    else
    {
        asc_test_passed = false;
        test_passed = false;
    }

    printf("升序排序测试: %s\n", asc_test_passed ? "通过" : "失败");
    wait_for_key();

    // 测试降序排序
    printf("\n--- 降序排序 ---\n");
    result = EK_rListSort(list, true);
    print_result(result, "降序排序");
    print_list(list, "降序排序后的链表");

    // 验证降序排序结果
    bool desc_test_passed = true;
    if (result == EK_OK)
    {
        EK_Node_t *current = list->List_Dummy->Node_Next;
        while (current && current->Node_Next != list->List_Dummy)
        {
            if (current->Node_Order < current->Node_Next->Node_Order)
            {
                desc_test_passed = false;
                test_passed = false;
                break;
            }
            current = current->Node_Next;
        }
    }
    else
    {
        desc_test_passed = false;
        test_passed = false;
    }

    printf("降序排序测试: %s\n", desc_test_passed ? "通过" : "失败");
    wait_for_key();

    // 清理当前链表
    while (list->List_Count > 0)
    {
        EK_Node_t *node = list->List_Dummy->Node_Next;
        EK_rListRemoveNode(list, node);
        free(node->Node_Data);
        free(node);
    }
    free(list->List_Dummy);
    free(list);

    // 测试大规模排序（超过20个节点）
    printf("\n--- 大规模排序测试（25个节点）---\n");
    EK_List_t *large_list = EK_pListCreate_Dynamic();
    if (!large_list)
    {
        printf("大规模测试链表创建失败\n");
        test_passed = false;
    }
    else
    {
        // 添加25个随机顺序的节点
        int large_orders[] = {85, 23, 67, 12, 94, 45, 78, 31, 56, 89, 14, 72, 38,
                              91, 25, 63, 47, 82, 19, 74, 36, 58, 93, 41, 76};
        int large_count = sizeof(large_orders) / sizeof(large_orders[0]);

        for (int i = 0; i < large_count; i++)
        {
            TestData_t *data = create_test_data(large_orders[i], "大规模节点");
            EK_Node_t *node = EK_pNodeCreate_Dynamic(data, large_orders[i]);
            if (node)
            {
                EK_rListInsertEnd(large_list, node);
            }
        }

        printf("大规模链表节点数: %u\n", large_list->List_Count);

        // 测试大规模升序排序
        printf("\n大规模升序排序...\n");
        EK_Result_t large_result = EK_rListSort(large_list, false);
        print_result(large_result, "大规模升序排序");

        // 验证大规模升序排序结果
        bool large_asc_passed = true;
        if (large_result == EK_OK)
        {
            EK_Node_t *current = large_list->List_Dummy->Node_Next;
            while (current && current->Node_Next != large_list->List_Dummy)
            {
                if (current->Node_Order > current->Node_Next->Node_Order)
                {
                    large_asc_passed = false;
                    test_passed = false;
                    break;
                }
                current = current->Node_Next;
            }
        }
        else
        {
            large_asc_passed = false;
            test_passed = false;
        }

        printf("大规模升序排序测试: %s\n", large_asc_passed ? "通过" : "失败");

        // 测试大规模降序排序
        printf("\n大规模降序排序...\n");
        large_result = EK_rListSort(large_list, true);
        print_result(large_result, "大规模降序排序");

        // 验证大规模降序排序结果
        bool large_desc_passed = true;
        if (large_result == EK_OK)
        {
            EK_Node_t *current = large_list->List_Dummy->Node_Next;
            while (current && current->Node_Next != large_list->List_Dummy)
            {
                if (current->Node_Order < current->Node_Next->Node_Order)
                {
                    large_desc_passed = false;
                    test_passed = false;
                    break;
                }
                current = current->Node_Next;
            }
        }
        else
        {
            large_desc_passed = false;
            test_passed = false;
        }

        printf("大规模降序排序测试: %s\n", large_desc_passed ? "通过" : "失败");

        // 清理大规模链表
        while (large_list->List_Count > 0)
        {
            EK_Node_t *node = large_list->List_Dummy->Node_Next;
            EK_rListRemoveNode(large_list, node);
            free(node->Node_Data);
            free(node);
        }
        free(large_list->List_Dummy);
        free(large_list);
    }

    record_test_result("链表排序功能测试", test_passed);
    wait_for_key();
}

/**
 * @brief 测试边界情况
 */
void test_edge_cases()
{
    printf("\n\n========== 测试边界情况 ==========\n");
    bool test_passed = true;

    // 测试空指针参数
    printf("\n--- 测试空指针参数 ---\n");
    EK_Result_t result;

    result = EK_rNodeCreate_Static(NULL, NULL, 0);
    print_result(result, "空指针节点创建");
    if (result == EK_OK) test_passed = false; // 应该返回错误

    result = EK_rListCreate_Static(NULL, NULL);
    print_result(result, "空指针链表创建");
    if (result == EK_OK) test_passed = false; // 应该返回错误

    result = EK_rListInsertEnd(NULL, NULL);
    print_result(result, "空指针插入");
    if (result == EK_OK) test_passed = false; // 应该返回错误

    result = EK_rListRemoveNode(NULL, NULL);
    print_result(result, "空指针删除");
    if (result == EK_OK) test_passed = false; // 应该返回错误

    result = EK_rListSort(NULL, false);
    print_result(result, "空指针排序");
    if (result == EK_OK) test_passed = false; // 应该返回错误

    // 测试空链表操作
    printf("\n--- 测试空链表操作 ---\n");
    EK_List_t *empty_list = EK_pListCreate_Dynamic();
    if (empty_list)
    {
        print_list(empty_list, "空链表");

        result = EK_rListSort(empty_list, false);
        print_result(result, "空链表排序");
        if (result != EK_OK) test_passed = false; // 空链表排序应该成功

        free(empty_list->List_Dummy);
        free(empty_list);
    }
    else
    {
        test_passed = false;
    }

    record_test_result("边界情况测试", test_passed);
    wait_for_key();
}

/* ========================= 主函数 ========================= */

int main()
{
    printf("========================================\n");
    printf("       EK_List 链表模块测试程序\n");
    printf("========================================\n");
    printf("本程序将测试双向链表的所有功能函数\n");
    printf("包括：节点创建、链表创建、插入、删除、移动、排序等\n");
    printf("========================================\n");

    wait_for_key();

    // 执行各项测试
    test_node_creation();
    test_list_creation();
    test_list_insertion();
    test_list_removal();
    test_list_move();
    test_list_sort();
    test_edge_cases();

    printf("\n\n========================================\n");
    printf("           所有测试完成！\n");
    printf("========================================\n");

    // 显示测试总结果
    show_test_summary();

    return 0;
}