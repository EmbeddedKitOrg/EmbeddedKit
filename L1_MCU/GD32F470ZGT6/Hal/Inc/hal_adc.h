#ifndef __HAL_ADC_H
#define __HAL_ADC_H

#include "bsp_interface.h"
#include <stdint.h>

void HAL_ADC_Start_DMA(uint16_t *buffer, uint16_t size);

/**
 * @brief 单次 ADC 读取
 * @param adc_periph ADC 外设编号 (ADC0, ADC1, ADC2)
 * @param channel ADC 通道号
 * @return ADC 转换结果
 */
uint16_t HAL_ADC_ReadSingle(uint32_t adc_periph, uint8_t channel);

/**
 * @brief 启动 ADC
 * @param adc_periph ADC 外设编号
 */
void HAL_ADC_Start(uint32_t adc_periph);

/**
 * @brief 停止 ADC
 * @param adc_periph ADC 外设编号
 */
void HAL_ADC_Stop(uint32_t adc_periph);

#endif /* __HAL_ADC_H */

