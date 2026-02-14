#include "../../hal/inc/ek_hal_i2c.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "gd32f4xx_i2c.h"
#include "../../../L1_MCU/GD32F470ZGT6/Hal/Inc/hal_iic.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define I2C_TIMEOUT         (10)

// 硬件信息结构体
typedef struct
{
    uint32_t i2c_periph;
} gd_i2c_info;

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

static const ek_i2c_ops_t gd_i2c_ops = {
    .init = _init,
    .write = _write,
    .read = _read,
    .mem_write = _mem_write,
    .mem_read = _mem_read,
};

// 硬件信息
static gd_i2c_info i2c0_info = {
    .i2c_periph = I2C0,
};

// 设备实例
static ek_hal_i2c_t drv_i2c0 = {
    .speed_hz = 400000,
};

// 注册到 HAL
void gd_i2c_drv_init(void)
{
    ek_hal_i2c_register(&drv_i2c0, "I2C0", &gd_i2c_ops, &i2c0_info);
}

EK_EXPORT_HARDWARE(gd_i2c_drv_init);

// 内部函数
static void _init(ek_hal_i2c_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
}

static bool _write(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(txdata != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_i2c_info *info = (gd_i2c_info *)dev->dev_info;
    EK_HAL_LOCK_ON(dev);

    // 调用 L1 HAL，mem_addr 设为 0（不使用内存地址）
    HAL_IIC_Transmit(info->i2c_periph, dev_addr, 0, txdata, size, I2C_TIMEOUT);

    EK_HAL_LOCK_OFF(dev);
    return true;
}

static bool _read(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *rxdata, size_t size)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(rxdata != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_i2c_info *info = (gd_i2c_info *)dev->dev_info;
    EK_HAL_LOCK_ON(dev);

    // 调用 L1 HAL，mem_addr 设为 0（不使用内存地址）
    HAL_IIC_Receive(info->i2c_periph, dev_addr, 0, rxdata, size, I2C_TIMEOUT);

    EK_HAL_LOCK_OFF(dev);
    return true;
}

static bool _mem_write(ek_hal_i2c_t *const dev,
                       uint16_t dev_addr,
                       uint16_t mem_addr,
                       ek_hal_i2c_size_t mem_size,
                       uint8_t *txdata,
                       size_t size)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(txdata != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_i2c_info *info = (gd_i2c_info *)dev->dev_info;
    EK_HAL_LOCK_ON(dev);

    // 调用 L1 HAL，传入内存地址
    // 注意：mem_size 参数在 L1 HAL 中未使用，假设为 8 位地址
    (void)mem_size; // L1 HAL 只支持 8 位内存地址
    HAL_IIC_Transmit(info->i2c_periph, dev_addr, mem_addr, txdata, size, I2C_TIMEOUT);

    EK_HAL_LOCK_OFF(dev);
    return true;
}

static bool _mem_read(ek_hal_i2c_t *const dev,
                      uint16_t dev_addr,
                      uint16_t mem_addr,
                      ek_hal_i2c_size_t mem_size,
                      uint8_t *rxdata,
                      size_t size)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(rxdata != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_i2c_info *info = (gd_i2c_info *)dev->dev_info;
    EK_HAL_LOCK_ON(dev);

    // 调用 L1 HAL，传入内存地址
    // 注意：mem_size 参数在 L1 HAL 中未使用，假设为 8 位地址
    (void)mem_size; // L1 HAL 只支持 8 位内存地址
    HAL_IIC_Receive(info->i2c_periph, dev_addr, mem_addr, rxdata, size, I2C_TIMEOUT);

    EK_HAL_LOCK_OFF(dev);
    return true;
}
