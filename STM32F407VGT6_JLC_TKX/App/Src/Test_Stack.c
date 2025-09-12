#include "Test.h"

void Test_Stack(void)
{
    MyPrintf(USER_UART, "=== 栈测试 ===\r\n");

    // 测试动态栈 - 创建容量为80字节的栈
    test_stack = EK_pStackCreate_Dynamic(80);
    if (!test_stack)
    {
        MyPrintf(USER_UART, "动态栈创建测试 ❌ - 创建失败\r\n");
        test_failure_count++;
        return;
    }

    MyPrintf(USER_UART, "动态栈创建测试 ✅ - 创建成功，容量: 80字节\r\n");
    test_success_count++;

    // 测试静态栈
    EK_Result_t result = EK_rStackCreate_Static(&static_stack, stack_buffer, sizeof(stack_buffer));
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "静态栈创建测试 ✅ - 创建成功，容量: %u\r\n", sizeof(stack_buffer));
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "静态栈创建测试 ❌ - 创建失败\r\n");
        test_failure_count++;
    }

    // === 第一部分：基本入栈测试 ===
    MyPrintf(USER_UART, "\n--- 基本入栈测试 ---\r\n");
    
    // 测试入栈操作 - 压入6个不同类型的数据
    int push_success = 0;
    int push_total = 6;
    
    // 压入uint32_t数据
    for (int i = 0; i < 3; i++)
    {
        uint32_t data = (i + 1) * 100 + test_counter;
        result = EK_rStackPush(test_stack, &data, sizeof(data));
        if (result == EK_OK)
        {
            MyPrintf(USER_UART, "入栈uint32_t: %u\r\n", data);
            push_success++;
        }
        else
        {
            MyPrintf(USER_UART, "入栈失败，数据: %u\r\n", data);
        }
    }
    
    // 压入uint16_t数据
    for (int i = 0; i < 2; i++)
    {
        uint16_t data = (i + 1) * 50;
        result = EK_rStackPush(test_stack, &data, sizeof(data));
        if (result == EK_OK)
        {
            MyPrintf(USER_UART, "入栈uint16_t: %u\r\n", data);
            push_success++;
        }
        else
        {
            MyPrintf(USER_UART, "入栈失败，数据: %u\r\n", data);
        }
    }
    
    // 压入uint8_t数据
    uint8_t byte_data = 0xFF;
    result = EK_rStackPush(test_stack, &byte_data, sizeof(byte_data));
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "入栈uint8_t: 0x%02X\r\n", byte_data);
        push_success++;
    }
    else
    {
        MyPrintf(USER_UART, "入栈失败，数据: 0x%02X\r\n", byte_data);
    }

    if (push_success == push_total)
    {
        MyPrintf(USER_UART, "栈入栈测试 ✅ - %d/%d 成功\r\n", push_success, push_total);
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "栈入栈测试 ❌ - %d/%d 成功\r\n", push_success, push_total);
        test_failure_count++;
    }

    MyPrintf(USER_UART, "栈剩余空间: %u字节\r\n", EK_sStackGetRemain(test_stack));
    MyPrintf(USER_UART,
             "栈状态 - 空: %s, 满: %s\r\n",
             EK_bStackIsEmpty(test_stack) ? "是" : "否",
             EK_bStackIsFull(test_stack) ? "是" : "否");

    // === 第二部分：基本出栈测试（LIFO验证） ===
    MyPrintf(USER_UART, "\n--- 基本出栈测试（LIFO验证） ---\r\n");
    
    int pop_success = 0;
    int pop_total = push_success; // 根据实际入栈成功的数量来出栈
    
    // 按LIFO顺序出栈：先出uint8_t，再出uint16_t，最后出uint32_t
    
    // 出栈uint8_t数据
    uint8_t pop_byte;
    result = EK_rStackPop(test_stack, &pop_byte, sizeof(pop_byte));
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "出栈uint8_t: 0x%02X\r\n", pop_byte);
        if (pop_byte == 0xFF) // 验证数据正确性
        {
            MyPrintf(USER_UART, "uint8_t数据验证 ✅\r\n");
        }
        else
        {
            MyPrintf(USER_UART, "uint8_t数据验证 ❌ - 期望0xFF，实际0x%02X\r\n", pop_byte);
        }
        pop_success++;
    }
    else
    {
        MyPrintf(USER_UART, "出栈uint8_t失败\r\n");
    }
    
    // 出栈两个uint16_t数据
    for (int i = 1; i >= 0; i--)
    {
        uint16_t pop_data;
        result = EK_rStackPop(test_stack, &pop_data, sizeof(pop_data));
        if (result == EK_OK)
        {
            MyPrintf(USER_UART, "出栈uint16_t: %u\r\n", pop_data);
            uint16_t expected = (i + 1) * 50;
            if (pop_data == expected)
            {
                MyPrintf(USER_UART, "uint16_t数据验证 ✅\r\n");
            }
            else
            {
                MyPrintf(USER_UART, "uint16_t数据验证 ❌ - 期望%u，实际%u\r\n", expected, pop_data);
            }
            pop_success++;
        }
        else
        {
            MyPrintf(USER_UART, "出栈uint16_t失败\r\n");
        }
    }
    
    // 出栈三个uint32_t数据
    for (int i = 2; i >= 0; i--)
    {
        uint32_t pop_data;
        result = EK_rStackPop(test_stack, &pop_data, sizeof(pop_data));
        if (result == EK_OK)
        {
            MyPrintf(USER_UART, "出栈uint32_t: %u\r\n", pop_data);
            uint32_t expected = (i + 1) * 100 + test_counter;
            if (pop_data == expected)
            {
                MyPrintf(USER_UART, "uint32_t数据验证 ✅\r\n");
            }
            else
            {
                MyPrintf(USER_UART, "uint32_t数据验证 ❌ - 期望%u，实际%u\r\n", expected, pop_data);
            }
            pop_success++;
        }
        else
        {
            MyPrintf(USER_UART, "出栈uint32_t失败\r\n");
        }
    }

    if (pop_success == pop_total && pop_total > 0)
    {
        MyPrintf(USER_UART, "栈出栈测试 ✅ - %d/%d 成功\r\n", pop_success, pop_total);
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "栈出栈测试 ❌ - %d/%d 成功\r\n", pop_success, pop_total);
        test_failure_count++;
    }

    MyPrintf(USER_UART, "出栈后栈剩余空间: %u字节\r\n", EK_sStackGetRemain(test_stack));
    MyPrintf(USER_UART,
             "出栈后栈状态 - 空: %s, 满: %s\r\n",
             EK_bStackIsEmpty(test_stack) ? "是" : "否",
             EK_bStackIsFull(test_stack) ? "是" : "否");

    // === 第三部分：边界测试 ===
    MyPrintf(USER_UART, "\n--- 边界测试 ---\r\n");
    
    // 测试栈满情况 - 尽可能填满栈
    uint32_t fill_data = 0x12345678;
    int fill_count = 0;
    
    while (EK_sStackGetRemain(test_stack) >= sizeof(fill_data))
    {
        result = EK_rStackPush(test_stack, &fill_data, sizeof(fill_data));
        if (result == EK_OK)
        {
            fill_count++;
            fill_data++; // 每次数据递增，便于验证
        }
        else
        {
            break;
        }
    }
    
    MyPrintf(USER_UART, "填充测试 - 成功压入%d个uint32_t数据\r\n", fill_count);
    MyPrintf(USER_UART, "栈剩余空间: %u字节\r\n", EK_sStackGetRemain(test_stack));
    
    // 测试继续压入数据（应该失败）
    uint32_t overflow_data = 0xDEADBEEF;
    result = EK_rStackPush(test_stack, &overflow_data, sizeof(overflow_data));
    if (result == EK_INSUFFICIENT_SPACE)
    {
        MyPrintf(USER_UART, "栈满溢出保护测试 ✅ - 正确阻止溢出\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "栈满溢出保护测试 ❌ - 未正确阻止溢出\r\n");
        test_failure_count++;
    }
    
    // 测试空栈出栈（清空后再尝试出栈）
    int empty_count = 0;
    while (!EK_bStackIsEmpty(test_stack))
    {
        uint32_t temp_data;
        result = EK_rStackPop(test_stack, &temp_data, sizeof(temp_data));
        if (result == EK_OK)
        {
            empty_count++;
        }
        else
        {
            break;
        }
    }
    
    MyPrintf(USER_UART, "清空栈 - 成功出栈%d个数据\r\n", empty_count);
    
    // 尝试从空栈出栈
    uint32_t empty_pop_data;
    result = EK_rStackPop(test_stack, &empty_pop_data, sizeof(empty_pop_data));
    if (result == EK_EMPTY)
    {
        MyPrintf(USER_UART, "空栈保护测试 ✅ - 正确阻止空栈出栈\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "空栈保护测试 ❌ - 未正确阻止空栈出栈\r\n");
        test_failure_count++;
    }

    // === 第四部分：静态栈测试 ===
    MyPrintf(USER_UART, "\n--- 静态栈测试 ---\r\n");
    
    // 向静态栈压入一些数据
    int static_push_success = 0;
    for (int i = 0; i < 5; i++)
    {
        uint32_t static_data = i * 111;
        result = EK_rStackPush(&static_stack, &static_data, sizeof(static_data));
        if (result == EK_OK)
        {
            static_push_success++;
        }
    }
    
    MyPrintf(USER_UART, "静态栈压入测试 - 成功压入%d个数据\r\n", static_push_success);
    
    // 从静态栈出栈验证
    int static_pop_success = 0;
    for (int i = 4; i >= 0; i--)
    {
        uint32_t static_pop_data;
        result = EK_rStackPop(&static_stack, &static_pop_data, sizeof(static_pop_data));
        if (result == EK_OK)
        {
            uint32_t expected = i * 111;
            if (static_pop_data == expected)
            {
                static_pop_success++;
            }
        }
    }
    
    if (static_pop_success == static_push_success)
    {
        MyPrintf(USER_UART, "静态栈LIFO测试 ✅ - 数据验证正确\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "静态栈LIFO测试 ❌ - 数据验证失败\r\n");
        test_failure_count++;
    }

    // === 清理资源 ===
    // 删除动态栈
    result = EK_rStackDelete(test_stack);
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "动态栈删除测试 ✅ - 删除成功\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "动态栈删除测试 ❌ - 删除失败\r\n");
        test_failure_count++;
    }
    
    // 清理静态栈
    result = EK_rStackDelete(&static_stack);
    if (result == EK_OK)
    {
        MyPrintf(USER_UART, "静态栈清理测试 ✅ - 清理成功\r\n");
        test_success_count++;
    }
    else
    {
        MyPrintf(USER_UART, "静态栈清理测试 ❌ - 清理失败\r\n");
        test_failure_count++;
    }

    MyPrintf(USER_UART, "栈测试完成\r\n\r\n");
}