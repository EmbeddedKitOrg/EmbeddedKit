#include "hal_dma.h"
#include "gd32f4xx_dma.h"

/**
 * @brief DMA 阻塞传输
 */
bool HAL_DMA_Transfer(uint32_t dma_periph, uint8_t channel,
                      void *src, void *dst, uint32_t size,
                      hal_dma_direction_t dir)
{
    // 禁用通道
    dma_channel_disable(dma_periph, channel);

    // 反初始化通道
    dma_deinit(dma_periph, channel);

    // 配置 DMA 参数
    dma_single_data_parameter_struct dma_init_struct;
    dma_single_data_para_struct_init(&dma_init_struct);

    // 设置传输方向和地址
    switch (dir)
    {
    case HAL_DMA_DIR_M2M:
        dma_init_struct.direction = DMA_MEMORY_TO_MEMORY;
        dma_init_struct.memory0_addr = (uint32_t)src;
        dma_init_struct.periph_addr = (uint32_t)dst;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_ENABLE;
        break;

    case HAL_DMA_DIR_M2P:
        dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
        dma_init_struct.memory0_addr = (uint32_t)src;
        dma_init_struct.periph_addr = (uint32_t)dst;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        break;

    case HAL_DMA_DIR_P2M:
        dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;
        dma_init_struct.periph_addr = (uint32_t)src;
        dma_init_struct.memory0_addr = (uint32_t)dst;
        dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        break;

    default:
        return false;
    }

    // 设置数据宽度和数量
    dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
    dma_init_struct.number = size;
    dma_init_struct.priority = DMA_PRIORITY_HIGH;

    // 初始化 DMA
    dma_single_data_mode_init(dma_periph, channel, &dma_init_struct);

    // 启动 DMA
    dma_channel_enable(dma_periph, channel);

    // 轮询等待传输完成
    uint32_t timeout = 1000000;
    while (!dma_flag_get(dma_periph, channel, DMA_FLAG_FTF) && timeout > 0)
    {
        timeout--;
    }

    // 清除标志
    dma_flag_clear(dma_periph, channel, DMA_FLAG_FTF);

    // 禁用通道
    dma_channel_disable(dma_periph, channel);

    return (timeout > 0);
}

/**
 * @brief 中止 DMA 传输
 */
void HAL_DMA_Abort(uint32_t dma_periph, uint8_t channel)
{
    dma_channel_disable(dma_periph, channel);
}
