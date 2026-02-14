#include "../../hal/inc/ek_hal_tim.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "hal_tim.h"
#include "gd32f4xx_timer.h"

// 硬件信息结构体
typedef struct
{
    uint32_t timer_periph;
} gd_tim_info;

// ops 实现
static void _init(ek_hal_tim_base_t *const dev);
static void _tim_start(ek_hal_tim_base_t *const dev);
static void _tim_stop(ek_hal_tim_base_t *const dev);
static uint32_t _get(ek_hal_tim_base_t *const dev);
static void _set(ek_hal_tim_base_t *const dev, uint32_t value);

static const ek_tim_ops_t gd_tim_ops = {
    .init = _init,
    .start = _tim_start,
    .stop = _tim_stop,
    .get = _get,
    .set = _set,
};

// 硬件信息
static gd_tim_info tim2_info = {
    .timer_periph = TIMER2,
};

// 设备实例
static ek_hal_tim_base_t drv_tim2;

// 注册到 HAL
void gd_tim_drv_init(void)
{
    ek_hal_tim_register(&drv_tim2, "TIM2", &gd_tim_ops, &tim2_info);
}

EK_EXPORT_HARDWARE(gd_tim_drv_init);

// 内部函数
static void _init(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
}

static void _tim_start(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_tim_info *info = (gd_tim_info *)dev->dev_info;
    HAL_TIM_Enable(info->timer_periph);
}

static void _tim_stop(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_tim_info *info = (gd_tim_info *)dev->dev_info;
    HAL_TIM_Disable(info->timer_periph);
}

static uint32_t _get(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_tim_info *info = (gd_tim_info *)dev->dev_info;
    return HAL_TIM_GetCounter(info->timer_periph);
}

static void _set(ek_hal_tim_base_t *const dev, uint32_t value)
{
    ek_assert_param(dev != NULL);
    gd_tim_info *info = (gd_tim_info *)dev->dev_info;
    HAL_TIM_SetCounter(info->timer_periph, value);
}
