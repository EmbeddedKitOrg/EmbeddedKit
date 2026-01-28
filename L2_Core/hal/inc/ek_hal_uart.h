#ifndef EK_HAL_UART_H
#define EK_HAL_UART_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_uart_t ek_hal_uart_t;

struct ek_hal_uart_t
{
    uint8_t idx;
    uint32_t baudrate;
    uint16_t buf_size;
    uint8_t *rxbuffer;
    ek_list_node_t node;

    bool lock;

    void (*init)(void);
    bool (*write)(uint8_t *txdata, size_t size);
    void (*read)(void);
};

extern ek_list_node_t ek_hal_uart_head;

extern ek_hal_uart_t hal_drv_uart1;

void ek_hal_uart_init(void);

#endif // EK_HAL_UART_H
