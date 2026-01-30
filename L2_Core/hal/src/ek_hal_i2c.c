#include "../inc/ek_hal_i2c.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define STM32F429XX_TIMEOUT (100)
static void i2c1_init(void);
static bool i2c1_send(uint16_t dev_addr, uint8_t *txdata, size_t size);
static bool i2c1_recieve(uint16_t dev_addr, uint8_t *rxdata, size_t size);
static bool
i2c1_mem_write(uint16_t dev_addr, uint16_t mem_addr, ek_hal_i2c_mem_size_t mem_size, uint8_t *txdata, size_t size);
static bool
i2c1_mem_read(uint16_t dev_addr, uint16_t mem_addr, ek_hal_i2c_mem_size_t mem_size, uint8_t *rxdata, size_t size);

ek_list_node_t ek_hal_i2c_head;

ek_hal_i2c_t hal_drv_i2c1 = {
    .idx = 1,
    .speed_hz = 400000,

    .lock = false,

    .init = i2c1_init,
    .read = i2c1_recieve,
    .write = i2c1_send,

    .mem_read = i2c1_mem_read,
    .mem_write = i2c1_mem_write,
};

static void i2c1_init(void)
{
    hal_drv_i2c1.lock = false;
}

static bool i2c1_send(uint16_t dev_addr, uint8_t *txdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_i2c1) == true) return false;

    EK_HAL_LOCK_ON(&hal_drv_i2c1);

    __UNUSED(dev_addr);
    __UNUSED(txdata);
    __UNUSED(size);
    // 具体的i2c底层

    EK_HAL_LOCK_OFF(&hal_drv_i2c1);

    return true;
}

static bool i2c1_recieve(uint16_t dev_addr, uint8_t *rxdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_i2c1) == true) return false;

    EK_HAL_LOCK_ON(&hal_drv_i2c1);

    __UNUSED(dev_addr);
    __UNUSED(rxdata);
    __UNUSED(size);
    // 具体的i2c底层

    EK_HAL_LOCK_OFF(&hal_drv_i2c1);

    return true;
}

static bool
i2c1_mem_write(uint16_t dev_addr, uint16_t mem_addr, ek_hal_i2c_mem_size_t mem_size, uint8_t *txdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_i2c1) == true) return false;

    // uint16_t msize = (mem_size == EK_HAL_I2C_MEM_8B) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;
    // msize 来判断从机设备长度

    EK_HAL_LOCK_ON(&hal_drv_i2c1);

    __UNUSED(dev_addr);
    __UNUSED(mem_addr);
    __UNUSED(txdata);
    __UNUSED(size);
    // 具体的i2c底层

    EK_HAL_LOCK_OFF(&hal_drv_i2c1);

    return true;
}

static bool
i2c1_mem_read(uint16_t dev_addr, uint16_t mem_addr, ek_hal_i2c_mem_size_t mem_size, uint8_t *rxdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_i2c1) == true) return false;

    // uint16_t msize = (mem_size == EK_HAL_I2C_MEM_8B) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;
    // msize 来判断从机设备长度

    EK_HAL_LOCK_ON(&hal_drv_i2c1);

    __UNUSED(dev_addr);
    __UNUSED(mem_addr);
    __UNUSED(rxdata);
    __UNUSED(size);
    // 具体的i2c底层

    EK_HAL_LOCK_OFF(&hal_drv_i2c1);

    return true;
}

void ek_hal_i2c_init(void)
{
    ek_list_init(&ek_hal_i2c_head);

    ek_list_add_tail(&ek_hal_i2c_head, &hal_drv_i2c1.node);
}

EK_EXPORT(ek_hal_i2c_init, 0);
