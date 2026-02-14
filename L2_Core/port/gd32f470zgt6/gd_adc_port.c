#include "../../hal/inc/ek_hal_adc.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "hal_adc.h"
#include "gd32f4xx_adc.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

// 硬件信息结构体
typedef struct
{
    uint32_t adc_periph;
    uint8_t channel;
} gd_adc_info;

// ops 实现
static void _init(ek_hal_adc_t *const dev);
static uint32_t _read(ek_hal_adc_t *const dev);
static bool _read_dma(ek_hal_adc_t *const dev, uint32_t *buffer, size_t size);
static void _adc_start(ek_hal_adc_t *const dev);
static void _adc_stop(ek_hal_adc_t *const dev);

static const ek_adc_ops_t gd_adc_ops = {
    .init = _init,
    .read = _read,
    .read_dma = _read_dma,
    .start = _adc_start,
    .stop = _adc_stop,
};

// 硬件信息
static gd_adc_info adc0_info = {
    .adc_periph = ADC0,
    .channel = 0,
};

// 设备实例
static ek_hal_adc_t drv_adc0 = {
    .sample_rate = 1000000,
    .resolution = EK_HAL_ADC_RES_12B,
};

// 注册到 HAL
void gd_adc_drv_init(void)
{
    ek_hal_adc_register(&drv_adc0, "ADC0", &gd_adc_ops, &adc0_info);
}

EK_EXPORT_HARDWARE(gd_adc_drv_init);

// 内部函数
static void _init(ek_hal_adc_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
}

static uint32_t _read(ek_hal_adc_t *const dev)
{
    ek_assert_param(dev != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return 0;

    gd_adc_info *info = (gd_adc_info *)dev->dev_info;
    EK_HAL_LOCK_ON(dev);

    uint32_t value = HAL_ADC_ReadSingle(info->adc_periph, info->channel);

    EK_HAL_LOCK_OFF(dev);
    return value;
}

static bool _read_dma(ek_hal_adc_t *const dev, uint32_t *buffer, size_t size)
{
    ek_assert_param(dev != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    // TODO: DMA 中断处理
    // 需要配置 ADC DMA 中断，在中断处理函数中调用回调
    // 中断处理函数示例：void DMA_ADC_IRQHandler(void) { ... }
    HAL_ADC_Start_DMA((uint16_t *)buffer, size);
    return true;
}

static void _adc_start(ek_hal_adc_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_adc_info *info = (gd_adc_info *)dev->dev_info;
    HAL_ADC_Start(info->adc_periph);
}

static void _adc_stop(ek_hal_adc_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_adc_info *info = (gd_adc_info *)dev->dev_info;
    HAL_ADC_Stop(info->adc_periph);
}
