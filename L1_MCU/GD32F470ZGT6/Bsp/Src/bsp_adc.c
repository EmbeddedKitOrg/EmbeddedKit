#include "bsp_adc.h"

/**
  * @brief  初始化ADC模块
  * @note   配置ADC时钟、GPIO引脚、ADC工作模式、通道配置和DMA功能
  *         - 配置时钟：使用APB总线120MHz做6分频得到20MHz ADC时钟
  *         - 配置IO：将摇杆X/Y轴和电池检测引脚设置为模拟模式
  *         - 配置ADC：独立模式、连续转换模式、扫描模式、右对齐、12位分辨率
  *         - 配置通道：温度传感器、内部参考电压、摇杆X/Y轴、电池电压共5个通道
  *         - 配置DMA：使能ADC的DMA传输功能
  * @param  无
  * @retval 无
  */
void BSP_ADC_Init(void)
{
    // 使能ADC和GPIO时钟
    rcu_periph_clock_enable(RCU_GPIOA); /* 摇杆X轴 */
    rcu_periph_clock_enable(RCU_GPIOB); /* 电池电压检测 */
    rcu_periph_clock_enable(RCU_GPIOC); /* 摇杆Y轴 */
    rcu_periph_clock_enable(BSP_ADC_RCU); /* ADC0外设时钟 */

    // ADC GPIO配置
    gpio_mode_set(BSP_JSK_X_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BSP_JSK_X_PIN);
    gpio_mode_set(BSP_JSK_Y_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BSP_JSK_Y_PIN);
    gpio_mode_set(BSP_BAT_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, BSP_BAT_PIN);

    adc_clock_config(ADC_ADCCK_PCLK2_DIV6); // 使用APB总线(120M)做6分频 -> 20M

    // ADC CFG
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT); // 独立模式

    adc_special_function_config(BSP_ADC, ADC_CONTINUOUS_MODE, ENABLE); // 连续模式，一次触发后自动转换
    adc_special_function_config(BSP_ADC, ADC_SCAN_MODE, ENABLE); // 开启扫描模式 多个通道自动转换

    adc_data_alignment_config(BSP_ADC, ADC_DATAALIGN_RIGHT); // 数据右对齐
    adc_resolution_config(BSP_ADC, ADC_RESOLUTION_12B); // 12位分辨率

    // 配置规则组
    adc_channel_length_config(BSP_ADC, ADC_ROUTINE_CHANNEL, 5);
    adc_routine_channel_config(BSP_ADC, 0, BSP_TEMP_ADC_CH, ADC_SAMPLETIME_84);
    adc_routine_channel_config(BSP_ADC, 1, BSP_VREF_ADC_CH, ADC_SAMPLETIME_84);
    adc_routine_channel_config(BSP_ADC, 2, BSP_JSK_X_ADC_CH, ADC_SAMPLETIME_84);
    adc_routine_channel_config(BSP_ADC, 3, BSP_JSK_Y_ADC_CH, ADC_SAMPLETIME_84);
    adc_routine_channel_config(BSP_ADC, 4, BSP_BAT_ADC_CH, ADC_SAMPLETIME_84);

    // 内部参考电压传感器
    adc_channel_16_to_18(ADC_TEMP_VREF_CHANNEL_SWITCH, ENABLE);

    // 使用软件触发
    adc_external_trigger_config(BSP_ADC, ADC_ROUTINE_CHANNEL, EXTERNAL_TRIGGER_DISABLE);

    // ADC的DMA配置
    adc_dma_request_after_last_enable(BSP_ADC);
    adc_dma_mode_enable(BSP_ADC);
}
