#include "../../hal/inc/ek_hal_dma.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "hal_dma.h"
#include "gd32f4xx_dma.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

// 硬件信息结构体
typedef struct
{
    uint32_t dma_periph;
    uint8_t channel;
} gd_dma_info;

// ops 实现
static void _init(ek_hal_dma_t *const dev);
static bool _transfer(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir);
static bool _transfer_it(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir);
static void _abort(ek_hal_dma_t *const dev);

static const ek_dma_ops_t gd_dma_ops = {
    .init = _init,
    .transfer = _transfer,
    .transfer_it = _transfer_it,
    .abort = _abort,
};

// 硬件信息
static gd_dma_info dma0_info = {
    .dma_periph = DMA0,
    .channel = DMA_CH0,
};

// 设备实例
static ek_hal_dma_t drv_dma0;

// 注册到 HAL
void gd_dma_drv_init(void)
{
    ek_hal_dma_register(&drv_dma0, "DMA0", &gd_dma_ops, &dma0_info);
}

EK_EXPORT_HARDWARE(gd_dma_drv_init);

// 内部函数
static void _init(ek_hal_dma_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
}

static bool _transfer(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir)
{
    ek_assert_param(dev != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_dma_info *info = (gd_dma_info *)dev->dev_info;
    EK_HAL_LOCK_ON(dev);

    // 转换方向枚举
    hal_dma_direction_t hal_dir;
    switch (dir)
    {
    case EK_HAL_DMA_DIR_M2M:
        hal_dir = HAL_DMA_DIR_M2M;
        break;
    case EK_HAL_DMA_DIR_M2P:
        hal_dir = HAL_DMA_DIR_M2P;
        break;
    case EK_HAL_DMA_DIR_P2M:
        hal_dir = HAL_DMA_DIR_P2M;
        break;
    default:
        EK_HAL_LOCK_OFF(dev);
        return false;
    }

    bool result = HAL_DMA_Transfer(info->dma_periph, info->channel, src, dst, size, hal_dir);

    EK_HAL_LOCK_OFF(dev);
    return result;
}

static bool _transfer_it(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir)
{
    ek_assert_param(dev != NULL);
    if (EK_HAL_LOCK_TEST(dev)) return false;

    // TODO: 中断模式 DMA 传输
    // 需要配置 DMA 中断，在中断处理函数中调用回调
    // 中断处理函数示例：void DMA_IRQHandler(void) { ... }
    // 需要使能 DMA 中断：dma_interrupt_enable(dma_periph, channel, DMA_INT_FTF);
    // 需要配置 NVIC：nvic_irq_enable(DMA_IRQn, priority, sub_priority);

    (void)src;
    (void)dst;
    (void)size;
    (void)dir;

    return false;
}

static void _abort(ek_hal_dma_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_dma_info *info = (gd_dma_info *)dev->dev_info;
    HAL_DMA_Abort(info->dma_periph, info->channel);
}
