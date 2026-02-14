#include "hal_spi.h"

/**
  * @brief  SPI同时发送和接收数据
  * @details 该函数通过指定的SPI外设同时发送和接收指定长度的数据。
  *          函数采用全双工通信模式，逐个字节进行数据传输。
  *          每次发送一个字节后立即接收一个字节的数据
  * @param  spi_periph: SPI外设编号 (如SPI0, SPI1等)
  * @param  tx_buffer: 要发送的数据缓冲区指针
  * @param  rx_buffer: 接收数据缓冲区指针
  * @param  size: 要发送和接收的数据长度
  * @param  time_out: 超时时间，单位毫秒
  * @retval 无
  * @note   该函数使用阻塞方式等待传输完成，超时后直接返回
  * @warning tx_buffer和rx_buffer不能为空指针，size必须大于0
  * @example
  * @code
  * uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
  * uint8_t rx_data[10];
  * HAL_SPI_TransmitReceive(SPI0, tx_data, rx_data, 10, 1000); // 使用1000ms超时时间
  * @endcode
  */
void HAL_SPI_TransmitReceive(
    uint32_t spi_periph, uint8_t *tx_buffer, uint8_t *rx_buffer, size_t size, uint32_t time_out)
{
    uint32_t now_tick = GetTick();

    for (uint32_t i = 0; i < size; i++)
    {
        // 等待发送缓冲区为空
        while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE))
        {
            if (GetTick() - now_tick >= time_out) return;
        }
        // 发送数据
        spi_i2s_data_transmit(spi_periph, tx_buffer[i]);

        // 等待接收完成
        while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE))
        {
            if (GetTick() - now_tick >= time_out) return;
        }
        // 读取接收到的数据
        rx_buffer[i] = spi_i2s_data_receive(spi_periph);
    }

    // 等待SPI完全空闲
    while (SET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TRANS))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
}

/**
  * @brief  SPI数据发送函数
  * @note   通过指定的SPI外设发送数据，采用轮询方式等待发送完成
  *         - 逐个字节发送数据，每次发送前等待发送缓冲区为空
  *         - 发送完成后等待SPI总线完全空闲，确保数据传输完成
  * @param  spi_periph: SPI外设编号（如SPI0、SPI1等）
  * @param  tx_buffer: 待发送数据缓冲区指针
  * @param  size: 待发送数据的字节数
  * @param  time_out: 超时时间，单位毫秒
  * @retval 无
  * @note   该函数使用阻塞方式等待发送完成，超时后直接返回
  * @warning tx_buffer不能为空指针，size必须大于0
  * @example
  * @code
  * uint8_t tx_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
  * HAL_SPI_Transmit(SPI0, tx_data, 10, 1000); // 使用1000ms超时时间
  * @endcode
  */
void HAL_SPI_Transmit(uint32_t spi_periph, uint8_t *tx_buffer, size_t size, uint32_t time_out)
{
    uint32_t now_tick = GetTick();

    for (uint32_t i = 0; i < size; i++)
    {
        // 等待发送缓冲区为空
        while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TBE))
        {
            if (GetTick() - now_tick >= time_out) return;
        }
        // 发送数据
        spi_i2s_data_transmit(spi_periph, tx_buffer[i]);
    }

    // 等待SPI完全空闲
    while (SET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TRANS))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
}

/**
  * @brief  SPI数据接收函数
  * @note   通过指定的SPI外设接收数据，采用轮询方式等待接收完成
  *         - 逐个字节接收数据，每次接收前等待接收缓冲区非空
  *         - 接收完成后等待SPI总线完全空闲，确保数据传输完成
  * @param  spi_periph: SPI外设编号（如SPI0、SPI1等）
  * @param  rx_buffer: 接收数据缓冲区指针，用于存储接收到的数据
  * @param  size: 需要接收数据的字节数
  * @param  time_out: 超时时间，单位毫秒
  * @retval 无
  * @note   该函数使用阻塞方式等待接收完成，超时后直接返回
  * @warning rx_buffer不能为空指针，size必须大于0
  * @example
  * @code
  * uint8_t rx_data[10];
  * HAL_SPI_Receive(SPI0, rx_data, 10, 1000); // 使用1000ms超时时间
  * @endcode
  */
void HAL_SPI_Receive(uint32_t spi_periph, uint8_t *rx_buffer, size_t size, uint32_t time_out)
{
    uint32_t now_tick = GetTick();

    for (uint32_t i = 0; i < size; i++)
    {
        // 等待接收完成
        while (RESET == spi_i2s_flag_get(spi_periph, SPI_FLAG_RBNE))
        {
            if (GetTick() - now_tick >= time_out) return;
        }
        // 读取接收到的数据
        rx_buffer[i] = spi_i2s_data_receive(spi_periph);
    }

    // 等待SPI完全空闲
    while (SET == spi_i2s_flag_get(spi_periph, SPI_FLAG_TRANS))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
}