#include "hal_adc.h"
#include "gd32f4xx_adc.h"

/**
  * @brief  启动ADC的DMA转换
  * @note   配置DMA通道并启动ADC转换，使用循环模式持续采集数据
  *         - 配置DMA为16位数据宽度，外设到内存方向
  *         - 设置外设地址为ADC数据寄存器，内存地址为用户提供的数据缓冲区
  *         - 使能DMA循环模式，自动重新开始采集
  *         - 启动ADC并执行校准，然后开始软件触发转换
  * @param  buffer: 数据缓冲区指针，用于存储ADC转换结果
  * @param  size: 缓冲区大小，指定DMA传输的数据个数
  * @retval 无
  */
void HAL_ADC_Start_DMA(uint16_t *buffer, uint16_t size)
{
    dma_channel_disable(BSP_ADC_DMA, BSP_ADC_DMA_CH);

    // 反初始化通道
    dma_deinit(BSP_ADC_DMA, BSP_ADC_DMA_CH);

    // DMA配置
    dma_single_data_parameter_struct bsp_adc_dma_init_struct;
    dma_single_data_para_struct_init(&bsp_adc_dma_init_struct);

    bsp_adc_dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
    bsp_adc_dma_init_struct.direction = DMA_PERIPH_TO_MEMORY;

    bsp_adc_dma_init_struct.periph_addr = (uint32_t)(&ADC_RDATA(BSP_ADC));
    bsp_adc_dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;

    bsp_adc_dma_init_struct.memory0_addr = (uint32_t)buffer;
    bsp_adc_dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

    bsp_adc_dma_init_struct.number = size;
    bsp_adc_dma_init_struct.priority = DMA_PRIORITY_HIGH;

    dma_single_data_mode_init(BSP_ADC_DMA, BSP_ADC_DMA_CH, &bsp_adc_dma_init_struct);
    dma_channel_subperipheral_select(BSP_ADC_DMA, BSP_ADC_DMA_CH, DMA_SUBPERI0);

    dma_circulation_enable(BSP_ADC_DMA, BSP_ADC_DMA_CH);

    dma_channel_enable(BSP_ADC_DMA, BSP_ADC_DMA_CH);

    // 开启ADC
    adc_enable(BSP_ADC);

    Delay(1);

    // 校准ADC
    adc_calibration_enable(BSP_ADC);

    // 开启ADC转换
    adc_software_trigger_enable(BSP_ADC, ADC_ROUTINE_CHANNEL);
}

/**
 * @brief 单次 ADC 读取
 */
uint16_t HAL_ADC_ReadSingle(uint32_t adc_periph, uint8_t channel)
{
    // 配置通道
    adc_routine_channel_config(adc_periph, 0, channel, ADC_SAMPLETIME_15);

    // 启动软件触发
    adc_software_trigger_enable(adc_periph, ADC_ROUTINE_CHANNEL);

    // 轮询等待转换完成
    uint32_t timeout = 100000;
    while (!adc_flag_get(adc_periph, ADC_FLAG_EOC) && timeout > 0)
    {
        timeout--;
    }

    // 读取数据
    uint16_t value = adc_routine_data_read(adc_periph);

    // 清除标志
    adc_flag_clear(adc_periph, ADC_FLAG_EOC);

    return value;
}

/**
 * @brief 启动 ADC
 */
void HAL_ADC_Start(uint32_t adc_periph)
{
    adc_enable(adc_periph);
    Delay(1);
    adc_calibration_enable(adc_periph);
    adc_software_trigger_enable(adc_periph, ADC_ROUTINE_CHANNEL);
}

/**
 * @brief 停止 ADC
 */
void HAL_ADC_Stop(uint32_t adc_periph)
{
    adc_disable(adc_periph);
}
