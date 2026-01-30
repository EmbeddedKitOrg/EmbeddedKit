#ifndef EK_HAL_I2C_H
#define EK_HAL_I2C_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_i2c_t ek_hal_i2c_t;

typedef enum
{
    EK_HAL_I2C_MEM_8B,
    EK_HAL_I2C_MEM_16B,
} ek_hal_i2c_mem_size_t;

struct ek_hal_i2c_t
{
    uint8_t idx;
    ek_list_node_t node;
    uint32_t speed_hz;

    bool lock;

    void (*init)(void);
    bool (*write)(uint16_t dev_addr, uint8_t *txdata, size_t size);
    bool (*read)(uint16_t dev_addr, uint8_t *txdata, size_t size);
    bool (*mem_write)(
        uint16_t dev_addr, uint16_t mem_addr, ek_hal_i2c_mem_size_t mem_size, uint8_t *txdata, size_t size);
    bool (*mem_read)(
        uint16_t dev_addr, uint16_t mem_addr, ek_hal_i2c_mem_size_t mem_size, uint8_t *rxdata, size_t size);
};

extern ek_list_node_t ek_hal_i2c_head;

extern ek_hal_i2c_t hal_drv_i2c1;

void ek_hal_i2c_init(void);

#endif
