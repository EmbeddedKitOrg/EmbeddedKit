#include "../../hal/inc/ek_hal_spi.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "gd32f4xx_spi.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define SPI_TIMEOUT (100000)

// 硬件信息结构体
typedef struct
{
    uint32_t spi_periph;
} gd_spi_info;

// ops 实现
static void _init(ek_hal_spi_t *const dev);
static bool _write(ek_hal_spi_t *const dev, uint8_t *txdata, size_t size);
static bool _read(ek_hal_spi_t *const dev, uint8_t *rxdata, size_t size);
static bool _write_read(ek_hal_spi_t *const dev, uint8_t *txdata, uint8_t *rxdata, size_t size);

static const ek_spi_ops_t gd_spi_ops = {
    .init = _init,
    .write = _write,
    .read = _read,
    .write_read = _write_read,
};

// 硬件信息
static gd_spi_info spi0_info = {
    .spi_periph = SPI0,
};

// 设备实例
static ek_hal_spi_t drv_spi0;

// 注册到 HAL
void gd_spi_drv_init(void)
{
    ek_hal_spi_register(&drv_spi0, "SPI0", &gd_spi_ops, &spi0_info);
}

EK_EXPORT_HARDWARE(gd_spi_drv_init);

// 内部函数
static void _init(ek_hal_spi_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
}

static bool _write(ek_hal_spi_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_spi_info *info = (gd_spi_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);

    for (size_t i = 0; i < size; i++)
    {
        // 等待发送缓冲区空
        uint32_t timeout = SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(info->spi_periph, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
            {
                EK_HAL_LOCK_OFF(dev);
                return false;
            }
        }

        spi_i2s_data_transmit(info->spi_periph, txdata[i]);
    }

    // 等待传输完成
    uint32_t timeout = SPI_TIMEOUT;
    while (SET == spi_i2s_flag_get(info->spi_periph, SPI_FLAG_TRANS))
    {
        if (--timeout == 0)
        {
            EK_HAL_LOCK_OFF(dev);
            return false;
        }
    }

    EK_HAL_LOCK_OFF(dev);
    return true;
}

static bool _read(ek_hal_spi_t *const dev, uint8_t *rxdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_spi_info *info = (gd_spi_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);

    for (size_t i = 0; i < size; i++)
    {
        // 发送 dummy 字节
        uint32_t timeout = SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(info->spi_periph, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
            {
                EK_HAL_LOCK_OFF(dev);
                return false;
            }
        }
        spi_i2s_data_transmit(info->spi_periph, 0xFF);

        // 等待接收数据
        timeout = SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(info->spi_periph, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
            {
                EK_HAL_LOCK_OFF(dev);
                return false;
            }
        }
        rxdata[i] = spi_i2s_data_receive(info->spi_periph);
    }

    EK_HAL_LOCK_OFF(dev);
    return true;
}

static bool _write_read(ek_hal_spi_t *const dev, uint8_t *txdata, uint8_t *rxdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_spi_info *info = (gd_spi_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);

    for (size_t i = 0; i < size; i++)
    {
        // 发送数据
        uint32_t timeout = SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(info->spi_periph, SPI_FLAG_TBE))
        {
            if (--timeout == 0)
            {
                EK_HAL_LOCK_OFF(dev);
                return false;
            }
        }
        spi_i2s_data_transmit(info->spi_periph, txdata[i]);

        // 接收数据
        timeout = SPI_TIMEOUT;
        while (RESET == spi_i2s_flag_get(info->spi_periph, SPI_FLAG_RBNE))
        {
            if (--timeout == 0)
            {
                EK_HAL_LOCK_OFF(dev);
                return false;
            }
        }
        rxdata[i] = spi_i2s_data_receive(info->spi_periph);
    }

    EK_HAL_LOCK_OFF(dev);
    return true;
}
