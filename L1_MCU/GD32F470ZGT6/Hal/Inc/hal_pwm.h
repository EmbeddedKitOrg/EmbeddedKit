#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 启动 PWM 输出
 * @param timer_periph 定时器外设编号 (TIMER0, TIMER1, ...)
 * @param channel 定时器通道 (TIMER_CH_0, TIMER_CH_1, ...)
 */
void HAL_PWM_Start(uint32_t timer_periph, uint16_t channel);

/**
 * @brief 停止 PWM 输出
 * @param timer_periph 定时器外设编号
 * @param channel 定时器通道
 */
void HAL_PWM_Stop(uint32_t timer_periph, uint16_t channel);

/**
 * @brief 设置 PWM 占空比
 * @param timer_periph 定时器外设编号
 * @param channel 定时器通道
 * @param duty 占空比 (0-10000 表示 0.00%-100.00%)
 */
void HAL_PWM_SetDuty(uint32_t timer_periph, uint16_t channel, uint32_t duty);

/**
 * @brief 设置 PWM 频率
 * @param timer_periph 定时器外设编号
 * @param freq 目标频率 (Hz)
 * @param timer_clk 定时器时钟频率 (Hz)
 */
void HAL_PWM_SetFrequency(uint32_t timer_periph, uint32_t freq, uint32_t timer_clk);

/**
 * @brief 获取定时器周期值
 * @param timer_periph 定时器外设编号
 * @return 自动重装载值 (ARR)
 */
uint32_t HAL_PWM_GetPeriod(uint32_t timer_periph);

#endif // HAL_PWM_H
