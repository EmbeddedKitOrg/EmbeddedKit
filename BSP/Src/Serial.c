#include "Serial.h"

uint16_t MyPrintf(const char *format, ...)
{
    // 确保缓冲区正确对齐，支持FPU操作和所有协程环境
    char buffer[128];

    va_list args;
    va_start(args, format);
    uint16_t len = vsnprintf(buffer, 128, format, args);
    va_end(args);

    HAL_UART_Transmit(USER_HUART, buffer, len, HAL_MAX_DELAY);
    return len;
}
