#ifndef EK_HAL_UART_H
#define EK_HAL_UART_H

#include "ek_hal_base.h"

typedef struct hal_ek_uart_t hal_ek_uart_t;

struct hal_ek_uart_t
{
    ek_hal_base_t base;

    uint32_t baudrate;

    bool (*write)(void *src, size_t size);
    bool (*read)(void *dst, size_t size);
};

#endif /* EK_HAL_UART_H */
