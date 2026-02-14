#include "hal_tim.h"
#include "gd32f4xx_timer.h"

/**
 * @brief 启动定时器
 */
void HAL_TIM_Enable(uint32_t timer_periph)
{
    timer_enable(timer_periph);
}

/**
 * @brief 停止定时器
 */
void HAL_TIM_Disable(uint32_t timer_periph)
{
    timer_disable(timer_periph);
}

/**
 * @brief 读取定时器计数值
 */
uint32_t HAL_TIM_GetCounter(uint32_t timer_periph)
{
    return timer_counter_read(timer_periph);
}

/**
 * @brief 设置定时器计数值
 */
void HAL_TIM_SetCounter(uint32_t timer_periph, uint32_t value)
{
    timer_counter_value_config(timer_periph, value);
}
