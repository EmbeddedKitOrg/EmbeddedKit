#ifndef __HAL_SPI_H
#define __HAL_SPI_H

#include "bsp_interface.h"

void HAL_SPI_TransmitReceive(
    uint32_t spi_periph, uint8_t *tx_buffer, uint8_t *rx_buffer, size_t size, uint32_t time_out);
void HAL_SPI_Transmit(uint32_t spi_periph, uint8_t *tx_buffer, size_t size, uint32_t time_out);
void HAL_SPI_Receive(uint32_t spi_periph, uint8_t *rx_buffer, size_t size, uint32_t time_out);

#endif /* __HAL_SPI_H */
