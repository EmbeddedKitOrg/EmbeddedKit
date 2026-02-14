#ifndef __HAL_IIC_H
#define __HAL_IIC_H

#include "bsp_interface.h"

void HAL_IIC_Receive(
    uint32_t i2c_periph, uint32_t dev_addr, uint32_t mem_addr, uint8_t *rx_buffer, size_t size, uint32_t time_out);
void HAL_IIC_Transmit(
    uint32_t i2c_periph, uint32_t dev_addr, uint32_t mem_addr, uint8_t *tx_buffer, size_t size, uint32_t time_out);

#endif /* __HAL_IIC_H */
