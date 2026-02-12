#include "../../hal/inc/ek_hal_tick.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "main.h"

// ops 实现
static void _init(ek_hal_tick_t *const dev);
static uint32_t _get(ek_hal_tick_t *const dev);
static void _delay(ek_hal_tick_t *const dev, uint32_t xticks);

static const ek_tick_ops_t st_tick_ops = {
    .init = _init,
    .get = _get,
    .delay = _delay,
};

// 设备实例
static ek_hal_tick_t drv_default_ticker = {
    .ms_per_tick = 1,
};

// 注册到 HAL
void st_tick_drv_init(void)
{
    ek_hal_tick_register(&drv_default_ticker, "DEFAULT_TICK", &st_tick_ops, NULL);
}

EK_EXPORT_HARDWARE(st_tick_drv_init);

// 内部函数
static void _init(ek_hal_tick_t *const dev)
{
    ek_assert_param(dev != NULL);
    // SysTick 由系统启动时初始化
}

static uint32_t _get(ek_hal_tick_t *const dev)
{
    (void)dev;
    return uwTick;
}

static void _delay(ek_hal_tick_t *const dev, uint32_t xticks)
{
    ek_assert_param(dev != NULL);

    uint32_t tick_to_wait = dev->ops->get(dev) + xticks;

    while (tick_to_wait >= dev->ops->get(dev));
}
