#include "Serial.h"

uint16_t MyPrintf(const char *format, ...)
{
    char buffer[100];
    va_list args;
    va_start(args, format);
    uint16_t len = vsnprintf(buffer, 100, format, args);
    va_end(args);
    HAL_UART_Transmit(USER_HUART, buffer, len, HAL_MAX_DELAY);
    return len;
}