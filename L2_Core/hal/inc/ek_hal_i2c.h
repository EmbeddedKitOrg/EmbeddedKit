#ifndef EK_HAL_I2C_H
#define EK_HAL_I2C_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_i2c_t ek_hal_i2c_t;
typedef struct ek_i2c_ops_t ek_i2c_ops_t;

/** @brief I2C 寄存器地址宽度 */
typedef enum
{
    EK_HAL_I2C_MEM_8B,
    EK_HAL_I2C_MEM_16B,
} ek_hal_i2c_size_t;

/** @brief I2C 操作函数集 */
struct ek_i2c_ops_t
{
    void (*init)(ek_hal_i2c_t *const dev);
    bool (*write)(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *txdata, size_t size);
    bool (*read)(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *rxdata, size_t size);
    bool (*mem_write)(ek_hal_i2c_t *const dev,
                      uint16_t dev_addr,
                      uint16_t mem_addr,
                      ek_hal_i2c_size_t mem_size,
                      uint8_t *txdata,
                      size_t size);
    bool (*mem_read)(ek_hal_i2c_t *const dev,
                     uint16_t dev_addr,
                     uint16_t mem_addr,
                     ek_hal_i2c_size_t mem_size,
                     uint8_t *rxdata,
                     size_t size);
};

/** @brief I2C 设备结构体 */
struct ek_hal_i2c_t
{
    ek_list_node_t node;
    const char *name;
    const ek_i2c_ops_t *ops;
    void *dev_info;

    uint32_t speed_hz;
    bool lock;
};

extern ek_list_node_t ek_hal_i2c_head;

void ek_hal_i2c_register(ek_hal_i2c_t *const dev, const char *name, const ek_i2c_ops_t *ops, void *dev_info);
ek_hal_i2c_t *ek_hal_i2c_find(const char *name);
bool ek_hal_i2c_write(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *txdata, size_t size);
bool ek_hal_i2c_read(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *rxdata, size_t size);
bool ek_hal_i2c_mem_write(ek_hal_i2c_t *const dev,
                          uint16_t dev_addr,
                          uint16_t mem_addr,
                          ek_hal_i2c_size_t mem_size,
                          uint8_t *txdata,
                          size_t size);
bool ek_hal_i2c_mem_read(ek_hal_i2c_t *const dev,
                         uint16_t dev_addr,
                         uint16_t mem_addr,
                         ek_hal_i2c_size_t mem_size,
                         uint8_t *rxdata,
                         size_t size);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_I2C_H
