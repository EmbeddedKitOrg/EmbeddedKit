#ifndef EK_HAL_SPI_H
#define EK_HAL_SPI_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_spi_t ek_hal_spi_t;

struct ek_hal_spi_t
{
    uint8_t idx;
    ek_list_node_t node;

    bool lock;

    void (*init)(void);
    bool (*write)(uint8_t *txdata, size_t size);
    bool (*read)(uint8_t *rxdata, size_t size);
    bool (*write_read)(uint8_t *txdata, uint8_t *rxdata, size_t size);
};

extern ek_list_node_t ek_hal_spi_head;

extern ek_hal_spi_t hal_drv_spi1;

void ek_hal_spi_init(void);

#endif // EK_HAL_SPI_H
