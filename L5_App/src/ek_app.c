#include "ek_app.h"

EK_IO_FPUTC()
{
    // 在这里放置传输一个字符的函数
    // e.g. fputc(ch, stdout);
    if (ch != '\0')
    {
        static ek_hal_uart_t *uart1 = NULL;
        if (uart1 == NULL) uart1 = ek_hal_uart_find("UART1");
        if (uart1 != NULL) ek_hal_uart_write(uart1, (uint8_t *)&ch, 1);
    }
}

EK_LOG_GET_TICK()
{
    // 这里返回你系统的tick
    // e.g. return HAL_GetTick();
    return (uint32_t)xTaskGetTickCount();
}

void ek_main(void)
{
    ek_heap_init();
    ek_io_init();
    ek_export_init();

    while (1)
    {
    }
}
