#include "Test.h"

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
