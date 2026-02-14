#include "../../hal/inc/ek_hal_dac.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "gd32f4xx_dac.h"
#include "hal_dac.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

// 硬件信息结构体
typedef struct
{
    uint32_t dac_periph;
    uint32_t channel;
} gd_dac_info;

// ops 实现
static void _init(ek_hal_dac_t *const dev);
static bool _write(ek_hal_dac_t *const dev, uint32_t value);
static bool _write_dma(ek_hal_dac_t *const dev, uint32_t *buffer, size_t size);
static void _dac_start(ek_hal_dac_t *const dev);
static void _dac_stop(ek_hal_dac_t *const dev);

static const ek_dac_ops_t gd_dac_ops = {
    .init = _init,
    .write = _write,
    .write_dma = _write_dma,
    .start = _dac_start,
    .stop = _dac_stop,
};

// 硬件信息
static gd_dac_info dac_info = {
    .dac_periph = DAC0,
    .channel = DAC_OUT1,
};

// 设备实例
static ek_hal_dac_t drv_dac = {
    .sample_rate = 1000000,
};

// 注册到 HAL
void gd_dac_drv_init(void)
{
    ek_hal_dac_register(&drv_dac, "DAC", &gd_dac_ops, &dac_info);
}

EK_EXPORT_HARDWARE(gd_dac_drv_init);

// 内部函数
static void _init(ek_hal_dac_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
}

static bool _write(ek_hal_dac_t *const dev, uint32_t value)
{
    ek_assert_param(dev != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_dac_info *info = (gd_dac_info *)dev->dev_info;
    EK_HAL_LOCK_ON(dev);

    HAL_DAC_WriteSingle(info->dac_periph, info->channel, (uint16_t)value);

    EK_HAL_LOCK_OFF(dev);
    return true;
}

static bool _write_dma(ek_hal_dac_t *const dev, uint32_t *buffer, size_t size)
{
    ek_assert_param(dev != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    // TODO: DMA 中断处理
    // 需要配置 DAC DMA 中断，在中断处理函数中调用回调
    // 中断处理函数示例：void DMA_DAC_IRQHandler(void) { ... }
    HAL_DAC_Start_DMA((uint8_t *)buffer, size);

    return true;
}

static void _dac_start(ek_hal_dac_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_dac_info *info = (gd_dac_info *)dev->dev_info;
    HAL_DAC_Start(info->dac_periph, info->channel);
}

static void _dac_stop(ek_hal_dac_t *const dev)
{
    ek_assert_param(dev != NULL);
    HAL_DAC_Stop();
}
