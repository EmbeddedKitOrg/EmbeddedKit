#ifndef __HAL_DAC_H
#define __HAL_DAC_H

#include "bsp_interface.h"
#include <stdint.h>

void HAL_DAC_Start_DMA(const uint8_t *source, uint32_t length);
void HAL_DAC_Stop(void);

/**
 * @brief 单次 DAC 写入
 * @param dac_periph DAC 外设编号 (DAC0, DAC1)
 * @param channel DAC 通道 (DAC_OUT0, DAC_OUT1)
 * @param value 要输出的值 (0-4095 for 12-bit)
 */
void HAL_DAC_WriteSingle(uint32_t dac_periph, uint32_t channel, uint16_t value);

/**
 * @brief 启动 DAC
 * @param dac_periph DAC 外设编号
 * @param channel DAC 通道
 */
void HAL_DAC_Start(uint32_t dac_periph, uint32_t channel);

#endif /* __HAL_DAC_H */

