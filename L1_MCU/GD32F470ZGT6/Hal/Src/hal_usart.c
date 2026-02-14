#include "hal_usart.h"
#include "gd32f4xx_usart.h"

/**
  * @brief  USART发送单个字节
  * @details 该函数通过指定的USART外设发送一个字节数据。
  *          函数会等待发送缓冲区为空，然后发送数据，
  *          最后等待发送完成标志置位
  * @param  usart_periph: USART外设编号 (如USART0, USART1等)
  * @param  ch: 要发送的字节数据
  * @retval 无
  */
void HAL_USART_SendByte(uint32_t usart_periph, uint8_t ch)
{
    usart_data_transmit(usart_periph, ch);
    while (RESET == usart_flag_get(usart_periph, USART_FLAG_TBE));
}

/**
  * @brief  USART发送数据数组
  * @details 该函数通过指定的USART外设发送指定长度的数据数组。
  *          函数内部调用USART_SendByte逐个字节发送数据
  * @param  usart_periph: USART外设编号 (如USART0, USART1等)
  * @param  tx_buffer: 要发送的数据缓冲区指针
  * @param  size: 要发送的数据长度
  * @param  time_out: 阻塞时间
  * @retval 无
  */
void HAL_USART_Transmite(uint32_t usart_periph, uint8_t *tx_buffer, size_t size, uint32_t time_out)
{
    usart_interrupt_disable(BSP_USART, USART_INT_TC); // 关闭传输完成中断
    uint32_t tick_now = GetTick();
    for (uint32_t i = 0; i < size; i++)
    {
        usart_data_transmit(usart_periph, tx_buffer[i]);
        while (RESET == usart_flag_get(usart_periph, USART_FLAG_TBE))
        {
            if (GetTick() - tick_now >= time_out) return;
        }
    }
}

/**
  * @brief  USART通过DMA方式发送数据
  * @details 该函数通过DMA方式发送指定长度的数据，实现高效的数据传输。
  *          函数会配置DMA通道参数，将数据从内存传输到USART数据寄存器，
  *          配置完成后启动DMA传输，无需CPU干预即可完成数据发送
  * @param  usart_periph: USART外设编号 (如USART0, USART1等)
  * @param  txbuffer: 要发送的数据缓冲区指针
  * @param  size: 要发送的数据长度
  * @retval 无
  * @note   DMA传输完成后会触发中断，可在中断中处理传输完成事件
  */
void HAL_USART_Transmite_DMA(uint32_t usart_periph, uint8_t *txbuffer, size_t size)
{
    // 禁用DMA通道
    dma_channel_disable(BSP_USART_DMA, BSP_USART_DMA_TX_CH);

    dma_deinit(BSP_USART_DMA, BSP_USART_DMA_TX_CH);

    dma_channel_subperipheral_select(BSP_USART_DMA, BSP_USART_DMA_TX_CH, DMA_SUBPERI4);

    // tx
    dma_single_data_parameter_struct bsp_usart_dma_tx_init_struct;
    bsp_usart_dma_tx_init_struct.periph_addr = (uint32_t)&USART_DATA(BSP_USART);
    bsp_usart_dma_tx_init_struct.memory0_addr = (uint32_t)txbuffer; // 使用时需要设置
    bsp_usart_dma_tx_init_struct.direction = DMA_MEMORY_TO_PERIPH; // 内存到外设
    bsp_usart_dma_tx_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT; // 字节宽度
    bsp_usart_dma_tx_init_struct.priority = DMA_PRIORITY_MEDIUM; // DMA优先级
    bsp_usart_dma_tx_init_struct.number = size; // 使用时需要设置
    bsp_usart_dma_tx_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    bsp_usart_dma_tx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

    dma_single_data_mode_init(BSP_USART_DMA, BSP_USART_DMA_TX_CH, &bsp_usart_dma_tx_init_struct);

    usart_interrupt_enable(BSP_USART, USART_INT_TC); // 传输完成中断
    usart_interrupt_flag_clear(BSP_USART, USART_INT_FLAG_TC);

    // 启用DMA通道
    dma_channel_enable(BSP_USART_DMA, BSP_USART_DMA_TX_CH);
}

/**
  * @brief  USART通过DMA方式接收数据直到空闲状态
  * @details 该函数通过DMA方式接收数据，支持空闲中断检测。
  *          函数会配置DMA通道参数，将USART数据寄存器的数据传输到内存缓冲区，
  *          当串口总线空闲时会触发空闲中断，可用于判断一帧数据的结束
  * @param  usart_periph: USART外设编号 (如USART0, USART1等)
  * @param  rxbuffer: 接收数据缓冲区指针
  * @param  size: 接收缓冲区大小
  * @retval 无
  * @note   需配合空闲中断使用，当检测到空闲中断时说明一帧数据接收完成
  */
void HAL_USART_ReceiveToIdle_DMA(uint32_t usart_periph, uint8_t *rxbuffer, size_t size)
{
    // 禁用DMA通道
    dma_channel_disable(BSP_USART_DMA, BSP_USART_DMA_RX_CH);

    dma_deinit(BSP_USART_DMA, BSP_USART_DMA_RX_CH);

    dma_channel_subperipheral_select(BSP_USART_DMA, BSP_USART_DMA_RX_CH, DMA_SUBPERI4);
    // rx
    dma_single_data_parameter_struct bsp_usart_dma_rx_init_struct;
    bsp_usart_dma_rx_init_struct.periph_addr = (uint32_t)&USART_DATA(BSP_USART);
    bsp_usart_dma_rx_init_struct.memory0_addr = (uint32_t)rxbuffer; // 使用时需要设置
    bsp_usart_dma_rx_init_struct.direction = DMA_PERIPH_TO_MEMORY; // 外设到内存
    bsp_usart_dma_rx_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT; // 字节宽度
    bsp_usart_dma_rx_init_struct.priority = DMA_PRIORITY_MEDIUM; // DMA优先级
    bsp_usart_dma_rx_init_struct.number = size; // 使用时需要设置
    bsp_usart_dma_rx_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    bsp_usart_dma_rx_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;

    dma_single_data_mode_init(BSP_USART_DMA, BSP_USART_DMA_RX_CH, &bsp_usart_dma_rx_init_struct);
    dma_circulation_disable(BSP_USART_DMA, BSP_USART_DMA_RX_CH);

    usart_interrupt_enable(BSP_USART, USART_INT_IDLE); // 使能空闲中断

    // 启用DMA通道
    dma_channel_enable(BSP_USART_DMA, BSP_USART_DMA_RX_CH);
}

/**
 * @brief 轮询接收数据
 */
bool HAL_USART_Receive(uint32_t usart_periph, uint8_t *data, uint16_t size, uint32_t timeout)
{
    for (uint16_t i = 0; i < size; i++)
    {
        uint32_t count = timeout;
        while (RESET == usart_flag_get(usart_periph, USART_FLAG_RBNE) && count > 0)
        {
            count--;
        }

        if (count == 0)
        {
            return false; // 超时
        }

        data[i] = usart_data_receive(usart_periph);
    }

    return true;
}
