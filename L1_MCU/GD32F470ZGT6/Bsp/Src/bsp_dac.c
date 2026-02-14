#include "bsp_dac.h"

/**
 * @brief  初始化DAC模块
 * @details 配置DAC用于音频输出，包括：
 *          - 启用DAC时钟
 *          - 设置触发源为TIMER7
 *          - 禁用波形生成模式
 *          - 启用输出缓存
 *          - 配置8位右对齐数据格式
 *          - 使能DMA请求
 *          - 使能DAC通道
 * @note   该函数配置DAC用于音频播放，使用外部定时器触发
 * @param  无
 * @retval 无
 */
void BSP_DAC_Init(void)
{
    // 使能DAC和GPIO时钟
    rcu_periph_clock_enable(RCU_GPIOA); /* DAC音频输出引脚 */
    rcu_periph_clock_enable(BSP_AUDIO_DAC_RCU); /* DAC外设时钟 */

    // DAC GPIO配置
    gpio_mode_set(BSP_DAC_AUDIO_OUT_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BSP_DAC_AUDIO_OUT_PIN);

    dac_deinit(BSP_AUDIO_DAC);

    // 配置触发源 使用 TIMER7 触发
    dac_trigger_source_config(BSP_AUDIO_DAC, BSP_DAC_AUDIO_OUT, DAC_TRIGGER_T7_TRGO);

    // 使能触发
    dac_trigger_enable(BSP_AUDIO_DAC, BSP_DAC_AUDIO_OUT);

    // 关闭波形模式
    dac_wave_mode_config(BSP_AUDIO_DAC, BSP_DAC_AUDIO_OUT, DAC_WAVE_DISABLE);

    // 开启输出缓存
    dac_output_buffer_enable(BSP_AUDIO_DAC, BSP_DAC_AUDIO_OUT);

    // 允许DMA请求
    dac_dma_enable(BSP_AUDIO_DAC, BSP_DAC_AUDIO_OUT);

    // 使能DAC
    dac_enable(BSP_AUDIO_DAC, BSP_DAC_AUDIO_OUT);
}