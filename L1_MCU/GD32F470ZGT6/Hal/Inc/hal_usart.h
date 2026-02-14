#ifndef __HAL_USART_H
#define __HAL_USART_H

#include "bsp_interface.h"
#include <stdint.h>
#include <stdbool.h>

void HAL_USART_SendByte(uint32_t usart_periph, uint8_t ch);
void HAL_USART_Transmite(uint32_t usart_periph, uint8_t *tx_buffer, size_t size, uint32_t time_out);
void HAL_USART_Transmite_DMA(uint32_t usart_periph, uint8_t *txbuffer, size_t size);
void HAL_USART_ReceiveToIdle_DMA(uint32_t usart_periph, uint8_t *rxbuffer, size_t size);

/**
 * @brief 轮询接收数据
 * @param usart_periph USART 外设编号 (USART0, USART1, ...)
 * @param data 接收缓冲区
 * @param size 要接收的字节数
 * @param timeout 超时时间（轮询次数）
 * @return 是否成功接收
 */
bool HAL_USART_Receive(uint32_t usart_periph, uint8_t *data, uint16_t size, uint32_t timeout);

#endif /* __HAL_USART_H */

