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
#if EK_USE_RTOS == 1
    return (uint32_t)xTaskGetTickCount();
#else
    static ek_hal_tick_t *ticker = NULL;
    if (ticker == NULL) ticker = ek_hal_tick_find("DEFAULT_TICK");
    ek_assert_param(ticker != NULL);
    return (uint32_t)ticker->ops->get(ticker);
#endif
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
