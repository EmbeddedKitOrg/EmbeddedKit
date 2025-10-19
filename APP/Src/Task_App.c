#include "Task_App.h"

EK_CoroHandler_t KeyHandler;
EK_CoroHandler_t LedHandler;
EK_CoroHandler_t SerialHandler;
EK_CoroHandler_t OledHandler;

EK_CoroMsgHanler_t SeiralMsgHandler;
EK_CoroSemHanlder_t SerialSemDMA_OK;

float randm_float(float min, float max)
{
    return min + (max - min) * rand() / RAND_MAX;
}

void Task_Key(void *arg)
{
    uint8_t KeyOld = 0;
    while (1)
    {
        uint8_t KeyTemp = Key_ReadRaw();
        uint8_t KeyVal = KeyTemp & (KeyTemp ^ KeyOld);
        KeyOld = KeyTemp;

        if (KeyVal)
        {
            char *buffer = (char *)EK_CORO_MALLOC(50);
            uint16_t length = snprintf(buffer,
                                       50,
                                       "key[%d]: uwTick %f Use:%lu\r\n",
                                       KeyVal,
                                       randm_float(1.0f, 100.0f),
                                       EK_uCoroGetHighWaterMark(NULL));
            Message_t m = {.buf = buffer, .len = length};
            EK_rMsgSendToBack(SeiralMsgHandler, &m, EK_MAX_DELAY);
        }
        EK_vCoroDelayUntil(20);
    }
}

void Task_OLED(void *arg)
{
    // 只在任务开始时初始化一次随机数种子
    srand(uwTick);

    while (1)
    {
        // 首先运行测试函数诊断问题
        float test_value = 123.456f;

        // 测试1：直接传递浮点数字面量
        MyPrintf("Test1: %.2f\r\n", 123.456f);

        // 测试2：传递变量
        MyPrintf("Test2: %.2f\r\n", test_value);

        // 测试3：强制转换为double（ARM Cortex-M浮点数传递约定）
        MyPrintf("Test3: %.2f\r\n", (double)test_value);

        // 测试4：使用临时变量复制
        float temp = test_value;
        MyPrintf("Test4: %.2f\r\n", temp);
        // 原有的测试代码
        float num_f = randm_float(1.1f, 100.0f);
        MyPrintf("Random: %.2f\r\n", num_f);

        EK_vCoroDelay(1000);
    }
}

void Task_LED(void *arg)
{
    while (1)
    {
        LED_JLC_TOGGLE();
        EK_vCoroDelay(250);
    }
}

void Task_Serial(void *arg)
{
    Message_t MsgToSend;
    while (1)
    {
        if (EK_rMsgReceive(SeiralMsgHandler, &MsgToSend, EK_MAX_DELAY) == EK_OK)
        {
            HAL_UART_Transmit_DMA(USER_HUART, MsgToSend.buf, MsgToSend.len);

            if (EK_rSemBinaryTake(SerialSemDMA_OK, EK_MAX_DELAY) == EK_OK)
            {
                EK_CORO_FREE(MsgToSend.buf);
            }
        }
    }
}

void TaskCreation(void)
{
    SerialSemDMA_OK = EK_pSemBinaryCreate(1);
    SeiralMsgHandler = EK_pMsgCreate(sizeof(Message_t), 5);

    OledHandler = EK_pCoroCreate(Task_OLED, NULL, 0, 5000);
    KeyHandler = EK_pCoroCreate(Task_Key, NULL, 1, 1024);
    LedHandler = EK_pCoroCreate(Task_LED, NULL, 2, 256);
    SerialHandler = EK_pCoroCreate(Task_Serial, NULL, 0, 512);

    // while (1)
    // {
    //     srand(uwTick);
    //     float num_f = randm_float(1.1f, 100.0f);
    //     MyPrintf("uwTick:%.2f\r\n", num_f);

    //     HAL_Delay(500);
    // }
}