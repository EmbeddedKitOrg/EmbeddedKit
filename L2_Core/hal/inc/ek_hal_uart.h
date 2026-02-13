#ifndef EK_HAL_UART_H
#define EK_HAL_UART_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_uart_t ek_hal_uart_t;
typedef struct ek_uart_ops_t ek_uart_ops_t;

/** @brief UART 操作函数集 */
struct ek_uart_ops_t
{
    void (*init)(ek_hal_uart_t *const dev);
    bool (*write)(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
    bool (*write_dma)(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
    void (*read)(ek_hal_uart_t *const dev);
};

/** @brief UART 设备结构体 */
struct ek_hal_uart_t
{
    ek_list_node_t node;
    const char *name;
    const ek_uart_ops_t *ops;
    void *dev_info;

    uint32_t baudrate;
    uint16_t buf_size;
    uint8_t *rxbuffer;
    bool lock;
};

extern ek_list_node_t ek_hal_uart_head;

void ek_hal_uart_register(ek_hal_uart_t *const dev, const char *name, const ek_uart_ops_t *ops, void *dev_info);
ek_hal_uart_t *ek_hal_uart_find(const char *name);
bool ek_hal_uart_write(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
bool ek_hal_uart_write_dma(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
void ek_hal_uart_read(ek_hal_uart_t *const dev);

#endif // EK_HAL_UART_H
