#include "bsp_usart.h"

/**
  * @brief  初始化USART串口
  * @details 该函数配置并使能USART串口，包括：
  *          - 使能USART和GPIO时钟
  *          - 配置TX和RX引脚为复用功能模式
  *          - 设置串口参数：波特率、8数据位、1停止位、无校验
  *          - 禁用硬件流控制
  *          - 使能发送和接收功能
  * @param  baud_rate: 串口波特率
  * @retval 无
  */
void BSP_USART_Init(uint32_t baud_rate)
{
    // Clock On
    rcu_periph_clock_enable(RCU_GPIOA);  /* USART TX、RX引脚 */
    rcu_periph_clock_enable(RCU_USART0); /* USART0外设时钟 */
    rcu_periph_clock_enable(RCU_DMA1);   /* USART DMA时钟 */

    // GPIO AF Init
    gpio_af_set(BSP_USART_TX_PORT, GPIO_AF_7, BSP_USART_TX_PIN);
    gpio_af_set(BSP_USART_RX_PORT, GPIO_AF_7, BSP_USART_RX_PIN);

    gpio_mode_set(BSP_USART_TX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_USART_TX_PIN);
    gpio_mode_set(BSP_USART_RX_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_USART_RX_PIN);

    gpio_output_options_set(BSP_USART_TX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_USART_TX_PIN);
    gpio_output_options_set(BSP_USART_RX_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_USART_RX_PIN);

    // USART Init
    usart_deinit(BSP_USART); // 复位串口
    usart_baudrate_set(BSP_USART, baud_rate); // 设置波特率
    usart_parity_config(BSP_USART, USART_PM_NONE); // 没有校验位
    usart_word_length_set(BSP_USART, USART_WL_8BIT); // 8位数据位
    usart_stop_bit_set(BSP_USART, USART_STB_1BIT); // 1位停止位
    usart_hardware_flow_rts_config(BSP_USART, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(BSP_USART, USART_CTS_DISABLE);

    // Enable UART
    usart_transmit_config(BSP_USART, USART_TRANSMIT_ENABLE); // 使能串口发送
    usart_receive_config(BSP_USART, USART_RECEIVE_ENABLE); // 使能串口接收

    // DMA
    usart_dma_transmit_config(BSP_USART, USART_TRANSMIT_DMA_ENABLE);
    usart_dma_receive_config(BSP_USART, USART_RECEIVE_DMA_ENABLE);

    // NVIC
    nvic_irq_enable(BSP_USART_IRQ, 7, 0);

    usart_enable(BSP_USART); // 使能串口
}
