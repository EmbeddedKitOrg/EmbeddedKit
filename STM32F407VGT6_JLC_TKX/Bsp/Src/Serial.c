/**
 * @file Serial.c
 * @brief 串口通信模块实现
 * @details 实现以下功能：
 *          1. 环形缓冲区数据管理，支持数据帧解析
 *          2. 基于DMA的串口数据接收与发送
 *          3. 格式化输出功能(基于printf重定向)
 *          4. 数据校验与指令提取
 * @note   数据帧格式（使用校验位）：包头(1byte) + 长度(1byte) + 数据(nbyte) + 校验位(1byte)
 *         数据帧格式（不使用校验位）：包头(1byte) + 长度(1byte) + 数据(nbyte)
 * @author N1ntyNine99
 * @date 2025-04-17
 */

#include "Serial.h"
/* ========================= 变量定义区 ========================= */
uint8_t RxSerial_Buffer[RX_TX_BUFFERS]; //数据接收缓存
FlagStatus Serial_RxCplt_Flag = RESET; //数据接收完成标志位
#if CIRCULAR_BUFFER_ENABLE == 1
static BufferType_t Buffer_ReadIndex = 0; //读数据指针
static BufferType_t Buffer_WriteIndex = 0; //写数据指针
uint8_t Circle_Buffer[CIRCLBUFFERS_SIZE]; //环形缓冲区
uint8_t Serial_Command[CMD_MAX_LENGTH - (CHECKSUM_ENABLE ? 3 : 2)]; //指令数组
#endif // CIRCULAR_BUFFER_ENABLE == 1

/* ========================= 串口初始化函数 ========================= */
/**
 * @brief 初始化串口模块
 * 
 */
void Serial_Init(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(USER_UART, RxSerial_Buffer, sizeof(RxSerial_Buffer));
}

/* ========================= 串口输出函数 ========================= */
/**
 * @brief 格式化串口输出函数(类似printf)
 * @param 使用的串口句柄
 * @param format 格式化字符串
 * @param ... 可变参数
 * 
 * @return 格式化的字符数目 
 */
uint16_t MyPrintf(UART_HandleTypeDef *huart, const char *format, ...)
{
    static char txbuffer[RX_TX_BUFFERS];
    uint16_t len = 0;
    va_list args;
    va_start(args, format);
    len = vsnprintf(txbuffer, RX_TX_BUFFERS, format, args);
    va_end(args);
    HAL_UART_Transmit(huart, (uint8_t *)&txbuffer, len, HAL_MAX_DELAY);
    return len;
}

/**
 * @brief 格式化串口输出函数(类似printf),通过DMA输出
 * @param 使用的串口句柄
 * @param format 格式化字符串
 * @param ... 可变参数
 * 
 * @return 格式化的字符数目 
 */
uint16_t MyPrintf_DMA(UART_HandleTypeDef *huart, const char *format, ...)
{
    static char txbuffer[RX_TX_BUFFERS];
    uint16_t len = 0;
    va_list args;
    va_start(args, format);
    len = vsnprintf(txbuffer, RX_TX_BUFFERS, format, args);
    va_end(args);
    HAL_UART_Transmit_DMA(huart, (uint8_t *)&txbuffer, len);
    return len;
}

#if CIRCULAR_BUFFER_ENABLE == 0
/**
 * @brief 串口空闲中断
 * @note 放入HAL_UARTEx_RxEventCallback中
 * 
 */
void Serial_Idle_Handler(void)
{
    HAL_UARTEx_ReceiveToIdle_DMA(USER_UART, RxSerial_Buffer, sizeof(RxSerial_Buffer));
    Serial_RxCplt_Flag = SET;
}

#endif // CIRCULAR_BUFFER_ENABLE == 0

#if CIRCULAR_BUFFER_ENABLE == 1
/* ========================= 串口输空闲中断处理数据 ========================= */
/**
 * @brief 串口空闲中断
 * @note 放入HAL_UARTEx_RxEventCallback中
 * 
 */
void Serial_Idle_Handler(void)
{
    Buffer_Write(RxSerial_Buffer, Size);
    HAL_UARTEx_ReceiveToIdle_DMA(USER_UART, RxSerial_Buffer, sizeof(RxSerial_Buffer));
}

/* ========================= 环形缓冲区 ========================= */

/**
 * @brief 增加数据写指针
 * 
 * @param Length 增加的数据的长度
 */
static inline void Buffer_AddWriteIndex(BufferType_t Length)
{
    Buffer_WriteIndex += Length;
    Buffer_WriteIndex %= CIRCLBUFFERS_SIZE; //避免数组越界
}

/**
 * @brief 增加数据读取指针
 * 
 * @param Length 增加的读取指针的长度
 */
static inline void Buffer_AddReadIndex(BufferType_t Length)
{
    Buffer_ReadIndex += Length;
    Buffer_ReadIndex %= CIRCLBUFFERS_SIZE;
}

/**
 * @brief 获取读取的一个数据
 * 
 * @param index 要读取的数据的下标
 * @return uint8_t 返回的要读取的数据
 */
static inline uint8_t Buffer_ReadData(BufferType_t index)
{
    BufferType_t temp = index % CIRCLBUFFERS_SIZE;
    return Circle_Buffer[temp];
}

/**
 * @brief 获取环形缓冲区中未处理的数据长度
 * 
 * @return BufferType_t 未处理的数据长度
 */
static inline BufferType_t Buffer_GetUnprocData(void)
{
    return (Buffer_WriteIndex + CIRCLBUFFERS_SIZE - Buffer_ReadIndex) % CIRCLBUFFERS_SIZE;
}

/**
 * @brief 获取当前缓冲区中剩余的空间
 * 
 * @return BufferType_t 返回剩余的空间
 */
static inline BufferType_t Buffer_GetRemain(void)
{
    return (CIRCLBUFFERS_SIZE - Buffer_GetUnprocData());
}

/**
 * @brief 计算数据校验
 * 
 * @param Data 需要校验的数据包的首地址
 * @param Length 数据包中的数据长度
 * @return uint8_t 校验和
 */
static uint8_t Buffer_Calibration(uint8_t *Data, uint8_t Length)
{
#if CHECKSUM_ENABLE == 1
    uint8_t sum = 0;
    for (uint8_t i = 1; i < Length; i++)
    {
        sum += Data[i];
    }
    return sum;
#else
    (void)Data; // 避免未使用变量警告
    (void)Length; // 避免未使用变量警告
    return 0; // 不使用校验位时返回0
#endif
}

/**
 * @brief 向缓冲区中写入一个数据
 * 
 * @param Data 数据的首地址
 * @param Length 数据长度
 * @return uint8_t 返回写入到缓冲区的数据长度
 */
uint8_t Buffer_Write(uint8_t *Data, uint8_t Length)
{
    if (Buffer_GetRemain() < Length) return 0;

    if (Buffer_WriteIndex + Length < CIRCLBUFFERS_SIZE)
    {
        EK_vMemCpy(Circle_Buffer + Buffer_WriteIndex, Data, Length);
        Buffer_AddWriteIndex(Length);
    }
    else
    {
        BufferType_t first_part = CIRCLBUFFERS_SIZE - Buffer_WriteIndex;
        BufferType_t second_part = Length - first_part;
        EK_vMemCpy(Circle_Buffer + Buffer_WriteIndex, Data, first_part);
        EK_vMemCpy(Circle_Buffer, Data + first_part, second_part);
        Buffer_AddWriteIndex(Length);
    }
    return Length;
}

/**
 * @brief 尝试获取一个指令
 * 
 * @param Command 想要获取的指令
 * @return uint8_t 返回0为没找到对应指令
*                  若获取到指令,返回指令长度
 */
uint8_t Buffer_Read(BufferType_t *Command)
{
    while (1)
    {
        //当前未处理指令长度小于最小指令长度,返回0
        if (Buffer_GetUnprocData() < CMD_MIN_LENGTH) return 0;

        //当前不是包头,将读取指针移动一格然后从头开始下一个循环
        if (Circle_Buffer[Buffer_ReadIndex] != DATAPACK_HEAD)
        {
            Buffer_AddReadIndex(1);
            continue;
        }

        //是包头就会进入这里
        uint8_t datapack_length = Buffer_ReadData(Buffer_ReadIndex + 1); //读取当前数据帧的数据长度
        if (Buffer_GetUnprocData() < datapack_length) return 0; //剩余未处理的数据长度小于数据长度 返回0

#if CHECKSUM_ENABLE == 1
        uint8_t datapack_cali = Buffer_Calibration(&Circle_Buffer[Buffer_ReadIndex], datapack_length);

        //校验不通过则直接跳过
        if (datapack_cali != Circle_Buffer[Buffer_ReadIndex + datapack_length - 1])
        {
            Buffer_AddReadIndex(1);
            continue;
        }
#endif

        //如果校验通过或不使用校验位,则是一个完整的数据,放入Command数组
        for (uint8_t i = 0; i < datapack_length - (CHECKSUM_ENABLE ? 3 : 2); i++)
        {
            Serial_Command[i] = Buffer_ReadData(Buffer_ReadIndex + i + 2);
        }

        Buffer_AddReadIndex(datapack_length);
        return datapack_length;
    }
}
#endif // CIRCULAR_BUFFER_ENABLE == 1