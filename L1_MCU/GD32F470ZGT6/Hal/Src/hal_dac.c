#include "hal_dac.h"
#include "gd32f4xx_dac.h"

/**
 * @brief 启动DMA音频播放
 *
 * 该函数配置DMA和定时器，实现音频数据的DMA传输播放。通过循环模式不断向DAC输出音频数据，
 * 定时器控制采样率，确保音频播放的时序正确。
 *
 * @param source 音频数据源缓冲区指针
 * @param length 音频数据的长度（字节为单位）
 *
 * @note 函数会启用DMA传输完成中断，每次中断代表一次音频播放周期结束
 * @note 使用循环模式，音频数据会循环播放直到调用HAL_DAC_Stop函数停止
 */
void HAL_DAC_Start_DMA(const uint8_t *source, uint32_t length)
{
    dma_deinit(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH);
    timer_disable(BSP_AUDIO_TIMER);
    timer_counter_value_config(BSP_AUDIO_TIMER, 0); // 先清空计数器

    dma_flag_clear(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INTF_FEEIF);
    dma_flag_clear(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INTF_SDEIF);
    dma_flag_clear(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INTF_TAEIF);
    dma_flag_clear(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INTF_HTFIF);
    dma_flag_clear(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INTF_FTFIF);

    // 配置DMA
    dma_channel_subperipheral_select(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_SUBPERI7);

    dma_single_data_parameter_struct bsp_dma_audio_init_struct;
    dma_single_data_para_struct_init(&bsp_dma_audio_init_struct);

    bsp_dma_audio_init_struct.direction = DMA_MEMORY_TO_PERIPH;
    bsp_dma_audio_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;

    bsp_dma_audio_init_struct.memory0_addr = (uint32_t)source;
    bsp_dma_audio_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

    bsp_dma_audio_init_struct.periph_addr = (uint32_t)&DAC_OUT1_R8DH(BSP_AUDIO_DAC);
    bsp_dma_audio_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;

    bsp_dma_audio_init_struct.number = length;
    bsp_dma_audio_init_struct.priority = DMA_PRIORITY_HIGH;
    bsp_dma_audio_init_struct.circular_mode = DMA_CIRCULAR_MODE_ENABLE; // 打开循环模式

    dma_single_data_mode_init(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, &bsp_dma_audio_init_struct);

    dma_interrupt_flag_clear(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INT_FLAG_FTF);

    // 配置DMA传输完成中断 每一次中断都是代表一次音频播放结束
    dma_interrupt_enable(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH, DMA_INT_FTF);

    nvic_irq_enable(BSP_AUDIO_DMA_IRQ, 2, 0);

    dma_channel_enable(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH);

    // 开启定时器
    timer_enable(BSP_AUDIO_TIMER);
}

/**
 * @brief 停止DMA音频播放
 *
 * 该函数停止定时器和DMA传输，终止音频播放过程。
 * 调用此函数后会立即停止向DAC输出音频数据。
 *
 * @note 此函数与HAL_DAC_Start_DMA函数配合使用，用于控制音频播放的停止
 */
void HAL_DAC_Stop(void)
{
    timer_disable(BSP_AUDIO_TIMER);
    dma_channel_disable(BSP_AUDIO_DMA, BSP_AUDIO_DMA_CH);
}

/**
 * @brief 单次 DAC 写入
 */
void HAL_DAC_WriteSingle(uint32_t dac_periph, uint32_t channel, uint16_t value)
{
    // 写入 12 位右对齐数据
    dac_data_set(dac_periph, channel, DAC_ALIGN_12B_R, value);

    // 软件触发
    dac_software_trigger_enable(dac_periph, channel);
}

/**
 * @brief 启动 DAC
 */
void HAL_DAC_Start(uint32_t dac_periph, uint32_t channel)
{
    dac_enable(dac_periph, channel);
}

