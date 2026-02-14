#ifndef HAL_DMA_H
#define HAL_DMA_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    HAL_DMA_DIR_M2M = 0,  // Memory to Memory
    HAL_DMA_DIR_M2P = 1,  // Memory to Peripheral
    HAL_DMA_DIR_P2M = 2,  // Peripheral to Memory
} hal_dma_direction_t;

/**
 * @brief DMA 阻塞传输
 * @param dma_periph DMA 外设编号 (DMA0, DMA1)
 * @param channel DMA 通道号 (DMA_CH0 ~ DMA_CH7)
 * @param src 源地址
 * @param dst 目标地址
 * @param size 传输数据量（字节）
 * @param dir 传输方向
 * @return 传输是否成功
 */
bool HAL_DMA_Transfer(uint32_t dma_periph, uint8_t channel,
                      void *src, void *dst, uint32_t size,
                      hal_dma_direction_t dir);

/**
 * @brief 中止 DMA 传输
 * @param dma_periph DMA 外设编号
 * @param channel DMA 通道号
 */
void HAL_DMA_Abort(uint32_t dma_periph, uint8_t channel);

#endif // HAL_DMA_H
