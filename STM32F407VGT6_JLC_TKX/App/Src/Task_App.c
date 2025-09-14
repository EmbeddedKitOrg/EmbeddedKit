#include "Task_App.h"

EK_pTaskHandler_t TskHandler1;
EK_pTaskHandler_t TskHandler2;
EK_pTaskHandler_t TskHandler3;
EK_pTaskHandler_t TskHandler_LargeListSort;
EK_pSeiralQueue_t TestQueue;

// 测试数据结构
EK_List_t *test_list = NULL;
EK_Queue_t *test_queue = NULL;
uint8_t queue_buffer[100];
EK_Queue_t static_queue;
EK_Stack_t *test_stack = NULL;
uint8_t stack_buffer[100];
EK_Stack_t static_stack;

// 测试计数器
uint32_t test_counter = 0;
// 测试结果统计
uint32_t test_success_count = 0;
uint32_t test_failure_count = 0;

void Task_LED(void);
void Task_Key(void);
void Task_ComponentTest(void);
void Test_MemPool(void);
void Test_List(void);
void Test_Queue(void);
void Test_TaskSystem(void);
void Test_AllTaskInfo(void);

void Send(void *data, size_t size)
{
    HAL_UART_Transmit_DMA(&huart1, data, size);
}

bool TaskCreation(void)
{
    bool res = true;
    res &= EK_rSerialInit_Dynamic() == EK_OK;
    res &= EK_rSerialCreateQueue_Dyanmic(&TestQueue, Send, 0, 2500) == EK_OK;
    res &= EK_rTaskCreate_Dynamic(Task_LED, 1, &TskHandler1) == EK_OK;
    res &= EK_rTaskCreate_Dynamic(Task_Key, 0, &TskHandler2) == EK_OK;
    res &= EK_rTaskCreate_Dynamic(Task_ComponentTest, 2, &TskHandler3) == EK_OK;
    return res;
}

void TaskIdle(void)
{
    EK_rSerialPoll(HAL_GetTick);
}

void Task_LED(void)
{
    LED_TOGGLE();

    EK_rTaskDelay(100);
}

void Task_Key(void)
{
    static uint8_t KeyOld = 0;
    uint8_t KeyTemp = KeyRead();
    uint8_t KeyVal = KeyTemp & (KeyTemp ^ KeyOld);
    KeyOld = KeyTemp;

    if (KeyVal)
    {
        EK_rSerialPrintf(TestQueue, "按键按下 剩余内存:%u字节\r\n", EK_sTaskGetFreeMemory());
        test_counter++; // 按键计数
    }
    EK_rTaskDelay(20);
}

void Task_ComponentTest(void)
{
    static uint8_t test_phase = 0;

    EK_rSerialPrintf(TestQueue, "\r\n=== EK组件测试 第%d阶段 ===\r\n", test_phase + 1);

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
            Test_Stack();
            break;
        case 4:
            Test_TaskSystem();
            break;
        case 5:
            Test_AllTaskInfo();
            break;
    }

    if (test_phase == 5)
    {
        EK_rSerialPrintf(TestQueue, "*****测试结束*****\r\n");
        EK_rSerialPrintf(TestQueue, "=== 测试结果统计 ===\r\n");
        EK_rSerialPrintf(TestQueue, "测试成功: %u 项 ✅\r\n", test_success_count);
        EK_rSerialPrintf(TestQueue, "测试失败: %u 项 ❌\r\n", test_failure_count);
        EK_rSerialPrintf(TestQueue, "总测试数: %u 项\r\n", test_success_count + test_failure_count);
        EK_rSerialPrintf(TestQueue,
                         "成功率: %.1f%%\r\n",
                         (float)test_success_count * 100.0f / (float)(test_success_count + test_failure_count));
        EK_rSerialPrintf(TestQueue, "==================\r\n");
        EK_rTaskDelete(NULL);
        EK_rTaskDelete(TskHandler1);
    }
    else
    {
        test_phase++;
        EK_rTaskDelay(2000); // 2s执行一个测试
    }
}
