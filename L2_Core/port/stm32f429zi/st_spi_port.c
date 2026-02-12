#include "../../hal/inc/ek_hal_spi.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "spi.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define STM32F429XX_TIMEOUT (100)

// 硬件信息结构体
typedef struct
{
    SPI_HandleTypeDef *hspi;
} st_spi_info;

// ops 实现
static void _init(ek_hal_spi_t *const dev);
static bool _write(ek_hal_spi_t *const dev, uint8_t *txdata, size_t size);
static bool _read(ek_hal_spi_t *const dev, uint8_t *rxdata, size_t size);
static bool _write_read(ek_hal_spi_t *const dev, uint8_t *txdata, uint8_t *rxdata, size_t size);

static const ek_spi_ops_t st_spi_ops = {
    .init = _init,
    .write = _write,
    .read = _read,
    .write_read = _write_read,
};

// 硬件信息
static st_spi_info spi1_info = {
    .hspi = &hspi5,
};

// 设备实例
static ek_hal_spi_t drv_spi1;

// 注册到 HAL
void st_spi_drv_init(void)
{
    ek_hal_spi_register(&drv_spi1, "SPI1", &st_spi_ops, &spi1_info);
}

EK_EXPORT_HARDWARE(st_spi_drv_init);

// 内部函数
static void _init(ek_hal_spi_t *const dev)
{
    ek_assert_param(dev != NULL);
    // CubeMX 已完成 SPI 硬件初始化
}

static bool _write(ek_hal_spi_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_spi_info *info = (st_spi_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_SPI_Transmit(info->hspi, txdata, size, STM32F429XX_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}

static bool _read(ek_hal_spi_t *const dev, uint8_t *rxdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_spi_info *info = (st_spi_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_SPI_Receive(info->hspi, rxdata, size, STM32F429XX_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}

static bool _write_read(ek_hal_spi_t *const dev, uint8_t *txdata, uint8_t *rxdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_spi_info *info = (st_spi_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive(info->hspi, txdata, rxdata, size, STM32F429XX_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}
