#ifndef HAL_TIM_H
#define HAL_TIM_H

#include <stdint.h>

/**
 * @brief 启动定时器
 * @param timer_periph 定时器外设编号 (TIMER0, TIMER1, ...)
 */
void HAL_TIM_Enable(uint32_t timer_periph);

/**
 * @brief 停止定时器
 * @param timer_periph 定时器外设编号
 */
void HAL_TIM_Disable(uint32_t timer_periph);

/**
 * @brief 读取定时器计数值
 * @param timer_periph 定时器外设编号
 * @return 当前计数值
 */
uint32_t HAL_TIM_GetCounter(uint32_t timer_periph);

/**
 * @brief 设置定时器计数值
 * @param timer_periph 定时器外设编号
 * @param value 要设置的计数值
 */
void HAL_TIM_SetCounter(uint32_t timer_periph, uint32_t value);

#endif // HAL_TIM_H
