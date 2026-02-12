#include "../../hal/inc/ek_hal_i2c.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "i2c.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define STM32F429XX_TIMEOUT (100)

// 硬件信息结构体
typedef struct
{
    I2C_HandleTypeDef *hi2c;
} st_i2c_info;

// ops 实现
static void _init(ek_hal_i2c_t *const dev);
static bool _write(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *txdata, size_t size);
static bool _read(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *rxdata, size_t size);
static bool _mem_write(ek_hal_i2c_t *const dev,
                       uint16_t dev_addr,
                       uint16_t mem_addr,
                       ek_hal_i2c_size_t mem_size,
                       uint8_t *txdata,
                       size_t size);
static bool _mem_read(ek_hal_i2c_t *const dev,
                      uint16_t dev_addr,
                      uint16_t mem_addr,
                      ek_hal_i2c_size_t mem_size,
                      uint8_t *rxdata,
                      size_t size);

static const ek_i2c_ops_t st_i2c_ops = {
    .init = _init,
    .write = _write,
    .read = _read,
    .mem_write = _mem_write,
    .mem_read = _mem_read,
};

// 硬件信息
static st_i2c_info i2c1_info = {
    .hi2c = &hi2c3,
};

// 设备实例
static ek_hal_i2c_t drv_i2c1 = {
    .speed_hz = 400000,
};

// 注册到 HAL
void st_i2c_drv_init(void)
{
    ek_hal_i2c_register(&drv_i2c1, "I2C1", &st_i2c_ops, &i2c1_info);
}

EK_EXPORT_HARDWARE(st_i2c_drv_init);

// 内部函数
static void _init(ek_hal_i2c_t *const dev)
{
    ek_assert_param(dev != NULL);
    // CubeMX 已完成 I2C 硬件初始化
}

static bool _write(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_i2c_info *info = (st_i2c_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(info->hi2c, dev_addr, txdata, size, STM32F429XX_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}

static bool _read(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *rxdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_i2c_info *info = (st_i2c_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_I2C_Master_Receive(info->hi2c, dev_addr, rxdata, size, STM32F429XX_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}

static bool _mem_write(ek_hal_i2c_t *const dev,
                       uint16_t dev_addr,
                       uint16_t mem_addr,
                       ek_hal_i2c_size_t mem_size,
                       uint8_t *txdata,
                       size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_i2c_info *info = (st_i2c_info *)dev->dev_info;
    uint16_t msize = (mem_size == EK_HAL_I2C_MEM_8B) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(info->hi2c, dev_addr, mem_addr, msize, txdata, size, STM32F429XX_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}

static bool _mem_read(ek_hal_i2c_t *const dev,
                      uint16_t dev_addr,
                      uint16_t mem_addr,
                      ek_hal_i2c_size_t mem_size,
                      uint8_t *rxdata,
                      size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_i2c_info *info = (st_i2c_info *)dev->dev_info;
    uint16_t msize = (mem_size == EK_HAL_I2C_MEM_8B) ? I2C_MEMADD_SIZE_8BIT : I2C_MEMADD_SIZE_16BIT;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(info->hi2c, dev_addr, mem_addr, msize, rxdata, size, STM32F429XX_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}
