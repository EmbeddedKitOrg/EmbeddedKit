#include "hal_pwm.h"
#include "gd32f4xx_timer.h"

/**
 * @brief 启动 PWM 输出
 */
void HAL_PWM_Start(uint32_t timer_periph, uint16_t channel)
{
    timer_enable(timer_periph);
    timer_channel_output_state_config(timer_periph, channel, TIMER_CCX_ENABLE);
}

/**
 * @brief 停止 PWM 输出
 */
void HAL_PWM_Stop(uint32_t timer_periph, uint16_t channel)
{
    timer_channel_output_state_config(timer_periph, channel, TIMER_CCX_DISABLE);
}

/**
 * @brief 设置 PWM 占空比
 */
void HAL_PWM_SetDuty(uint32_t timer_periph, uint16_t channel, uint32_t duty)
{
    uint32_t period = timer_autoreload_value_config_get(timer_periph);
    uint32_t pulse = (period * duty) / 10000;
    timer_channel_output_pulse_value_config(timer_periph, channel, pulse);
}

/**
 * @brief 设置 PWM 频率
 */
void HAL_PWM_SetFrequency(uint32_t timer_periph, uint32_t freq, uint32_t timer_clk)
{
    uint32_t prescaler = 0;
    uint32_t period = timer_clk / freq - 1;

    // 如果 period 太大，需要使用预分频
    while (period > 65535)
    {
        prescaler++;
        period = (timer_clk / (prescaler + 1)) / freq - 1;
    }

    timer_prescaler_config(timer_periph, prescaler, TIMER_PSC_RELOAD_NOW);
    timer_autoreload_value_config(timer_periph, period);
}

/**
 * @brief 获取定时器周期值
 */
uint32_t HAL_PWM_GetPeriod(uint32_t timer_periph)
{
    return timer_autoreload_value_config_get(timer_periph);
}
