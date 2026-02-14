#include "../../hal/inc/ek_hal_pwm.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "hal_pwm.h"
#include "gd32f4xx_timer.h"

// 硬件信息结构体
typedef struct
{
    uint32_t timer_periph;
    uint16_t channel;
} gd_pwm_info;

// ops 实现
static void _init(ek_hal_pwm_t *const dev);
static void _pwm_start(ek_hal_pwm_t *const dev);
static void _pwm_stop(ek_hal_pwm_t *const dev);
static void _set_duty(ek_hal_pwm_t *const dev, uint32_t duty);
static void _set_freq(ek_hal_pwm_t *const dev, uint32_t freq);
static uint32_t _get_duty(ek_hal_pwm_t *const dev);
static uint32_t _get_freq(ek_hal_pwm_t *const dev);

static const ek_pwm_ops_t gd_pwm_ops = {
    .init = _init,
    .start = _pwm_start,
    .stop = _pwm_stop,
    .set_duty = _set_duty,
    .set_freq = _set_freq,
    .get_duty = _get_duty,
    .get_freq = _get_freq,
};

// 硬件信息
static gd_pwm_info pwm0_info = {
    .timer_periph = TIMER0,
    .channel = TIMER_CH_0,
};

// 设备实例
static ek_hal_pwm_t drv_pwm0 = {
    .frequency = 1000, // 默认 1kHz
    .duty_cycle = 5000, // 默认 50%
};

// 注册到 HAL
void gd_pwm_drv_init(void)
{
    ek_hal_pwm_register(&drv_pwm0, "PWM0", &gd_pwm_ops, &pwm0_info);
}

EK_EXPORT_HARDWARE(gd_pwm_drv_init);

// 内部函数
static void _init(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
}

static void _pwm_start(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_pwm_info *info = (gd_pwm_info *)dev->dev_info;

    HAL_PWM_Start(info->timer_periph, info->channel);
}

static void _pwm_stop(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_pwm_info *info = (gd_pwm_info *)dev->dev_info;

    HAL_PWM_Stop(info->timer_periph, info->channel);
}

static void _set_duty(ek_hal_pwm_t *const dev, uint32_t duty)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(duty <= 10000);

    gd_pwm_info *info = (gd_pwm_info *)dev->dev_info;

    HAL_PWM_SetDuty(info->timer_periph, info->channel, duty);

    // 更新设备状态
    dev->duty_cycle = duty;
}

static void _set_freq(ek_hal_pwm_t *const dev, uint32_t freq)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(freq > 0);

    gd_pwm_info *info = (gd_pwm_info *)dev->dev_info;

    // 定时器时钟频率（根据实际系统时钟配置）
    uint32_t timer_clk = 120000000;

    HAL_PWM_SetFrequency(info->timer_periph, freq, timer_clk);

    // 更新设备状态
    dev->frequency = freq;

    // 重新设置占空比
    _set_duty(dev, dev->duty_cycle);
}

static uint32_t _get_duty(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);
    return dev->duty_cycle;
}

static uint32_t _get_freq(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);
    return dev->frequency;
}
