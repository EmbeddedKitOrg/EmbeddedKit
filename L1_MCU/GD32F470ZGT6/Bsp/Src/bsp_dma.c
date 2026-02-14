#include "bsp_dma.h"

/**
 * @brief 初始化DMA外设
 * @details 反初始化DMA
 *          - ADC DMA：用于ADC数据的自动传输，减少CPU占用
 *          - USART DMA：用于串口数据的发送和接收，提高通信效率
 *          - DAC DMA：用于搬运音频数据
 * @param None
 * @retval None
 * @note 此函数仅使能DMA时钟，具体的DMA通道配置在各自的外设初始化函数中完成
 *       必须在使用DMA功能之前调用此函数
 */
void BSP_DMA_Init(void)
{
    // 使能DMA时钟
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_DMA1);

    dma_deinit(BSP_ADC_DMA, BSP_ADC_DMA_CH);
    dma_deinit(BSP_USART_DMA_RCU, BSP_USART_DMA_TX_CH);
    dma_deinit(BSP_USART_DMA_RCU, BSP_USART_DMA_RX_CH);
    dma_deinit(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH);
    dma_deinit(BSP_SDIO_DMA, BSP_SDIO_DMA_CH);

    // 配置DMA传输完成中断 每一次中断都是代表一次音频播放结束
    dma_interrupt_enable(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INT_FTF);
    nvic_irq_enable(BSP_AUDIO_DMA_IRQ, 0, 6);
}
