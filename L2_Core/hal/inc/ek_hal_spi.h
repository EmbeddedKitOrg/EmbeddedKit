#ifndef EK_HAL_SPI_H
#define EK_HAL_SPI_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_spi_t ek_hal_spi_t;
typedef struct ek_spi_ops_t ek_spi_ops_t;

/** @brief SPI 操作函数集 */
struct ek_spi_ops_t
{
    void (*init)(ek_hal_spi_t *const dev);
    bool (*write)(ek_hal_spi_t *const dev, uint8_t *txdata, size_t size);
    bool (*read)(ek_hal_spi_t *const dev, uint8_t *rxdata, size_t size);
    bool (*write_read)(ek_hal_spi_t *const dev, uint8_t *txdata, uint8_t *rxdata, size_t size);
};

/** @brief SPI 设备结构体 */
struct ek_hal_spi_t
{
    ek_list_node_t node;
    const char *name;
    const ek_spi_ops_t *ops;
    void *dev_info;

    bool lock;
};

extern ek_list_node_t ek_hal_spi_head;

void ek_hal_spi_register(ek_hal_spi_t *const dev, const char *name, const ek_spi_ops_t *ops, void *dev_info);
ek_hal_spi_t *ek_hal_spi_find(const char *name);
bool ek_hal_spi_write(ek_hal_spi_t *const dev, uint8_t *txdata, size_t size);
bool ek_hal_spi_read(ek_hal_spi_t *const dev, uint8_t *rxdata, size_t size);
bool ek_hal_spi_write_read(ek_hal_spi_t *const dev, uint8_t *txdata, uint8_t *rxdata, size_t size);

#endif // EK_HAL_SPI_H
