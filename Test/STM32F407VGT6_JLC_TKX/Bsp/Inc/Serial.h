/**
 * @file Serial.h
 * @brief 串口通信模块头文件
 * @details 声明以下功能的接口：
 *          1. 环形缓冲区数据管理，支持数据帧解析
 *          2. 基于DMA的串口数据接收与发送
 *          3. 格式化输出功能(基于printf重定向)
 *          4. 数据校验与指令提取
 * @note   数据帧格式：包头(1byte) + 长度(1byte) + 数据(nbyte) + 校验位(1byte)
 * @author N1ntyNine99
 * @date 2025-04-17
 */

#ifndef __SERIAL_h
#define __SERIAL_h

#include "SysConfig.h"
/* ========================= 宏定义区 ========================= */

#define USER_UART              &huart1 //用户自选的串口号
#define RX_TX_BUFFERS          512 //发送\接收数据缓存的最大值

#define CIRCULAR_BUFFER_ENABLE 0 //使用环形缓冲区

#if CIRCULAR_BUFFER_ENABLE == 1

/*
    注意：当前环形缓冲要求一个数据包的数据数量小于0xFF个
    数据帧格式（使用校验位）：包头 * 1byte + 数据包长度 * 1byte + 其他数据 * nbyte + 校验位 * 1byte
    数据帧格式（不使用校验位）：包头 * 1byte + 数据包长度 * 1byte + 其他数据 * nbyte
 */

#define DATAPACK_HEAD   0xAA //数据包头
#define CHECKSUM_ENABLE 1 //是否使用校验位(1=使用,0=不使用)
#define CMD_MIN_LENGTH  (CHECKSUM_ENABLE ? 4 : 3) //指令最小长度
#define CMD_MAX_LENGTH  255 //指令最大长度

// 环形缓冲区相关定义
#define CIRCLBUFFERS_SIZE 255 //环形缓冲区的大小

#if CIRCLBUFFERS_SIZE <= UINT8_MAX
typedef uint8_t BufferType_t;
#endif

#if CIRCLBUFFERS_SIZE <= UINT16_MAX && CIRCLBUFFERS_SIZE > UINT8_MAX
typedef uint16_t BufferType_t;
#endif

#if CIRCLBUFFERS_SIZE <= UINT32_MAX && CIRCLBUFFERS_SIZE > UINT16_MAX
typedef uint32_t BufferType_t;
#endif

/* ========================= 外部引用变量 ========================= */
extern uint8_t Serial_Command[CMD_MAX_LENGTH - (CHECKSUM_ENABLE ? 3 : 2)]; //命令列表

/* ========================= 函数声明区 ========================= */
uint8_t Buffer_Write(uint8_t *Data, uint8_t Length);
uint8_t Buffer_Read(BufferType_t *Command);
#endif // CIRCULAR_BUFFER_ENABLE == 1

/* ========================= 通用外部引用变量 ========================= */
extern FlagStatus Serial_RxCplt_Flag; //数据接收完成标志位
extern uint8_t RxSerial_Buffer[RX_TX_BUFFERS];

/* ========================= 通用函数声明区 ========================= */
void Serial_Init(void);
uint16_t MyPrintf(UART_HandleTypeDef *huart, const char *format, ...);
uint16_t MyPrintf_DMA(UART_HandleTypeDef *huart, const char *format, ...);
void Serial_Idle_Handler(void);

#endif
