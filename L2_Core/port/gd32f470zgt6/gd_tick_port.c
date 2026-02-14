#include "../../hal/inc/ek_hal_tick.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "bsp_timer.h"

// 外部 SysTick 变量（需要在 BSP 层定义）
extern volatile uint32_t systick_count;

// 硬件信息结构体
typedef struct
{
    volatile uint32_t *tick_ptr;
} gd_tick_info;

// ops 实现
static void _init(ek_hal_tick_t *const dev);
static uint32_t _get(ek_hal_tick_t *const dev);
static void _delay(ek_hal_tick_t *const dev, uint32_t ms);

static const ek_tick_ops_t gd_tick_ops = {
    .init = _init,
    .get = _get,
    .delay = _delay,
};

// 硬件信息
static gd_tick_info tick_info = {
    .tick_ptr = &uwTick,
};

// 设备实例
static ek_hal_tick_t drv_tick;

// 注册到 HAL
void gd_tick_drv_init(void)
{
    ek_hal_tick_register(&drv_tick, "TICK", &gd_tick_ops, &tick_info);
}

EK_EXPORT_HARDWARE(gd_tick_drv_init);

// 内部函数
static void _init(ek_hal_tick_t *const dev)
{
    ek_assert_param(dev != NULL);
    BSP_Timer_Tick_Init();
}

static uint32_t _get(ek_hal_tick_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_tick_info *info = (gd_tick_info *)dev->dev_info;
    return *(info->tick_ptr);
}

static void _delay(ek_hal_tick_t *const dev, uint32_t ms)
{
    ek_assert_param(dev != NULL);
    uint32_t start = _get(dev);
    while ((_get(dev) - start) < ms);
}
