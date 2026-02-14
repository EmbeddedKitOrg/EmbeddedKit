#include "hal_iic.h"

/**
 * @brief I2C主机接收数据函数
 * @details 从指定I2C设备的指定内存地址读取指定长度的数据
 *
 * @param i2c_periph I2C外设基地址 (如 I2C0, I2C1)
 * @param dev_addr 设备地址 (7位地址，需左移1位)
 * @param mem_addr 要读取的内存地址
 * @param rx_buffer 接收数据缓冲区指针
 * @param size 要接收的数据字节数
 * @param time_out 超时时间，单位毫秒
 * @retval None
 * @note 该函数使用标准I2C协议，先发送内存地址再读取数据，使用阻塞方式等待接收完成，超时后直接返回
 * @warning rx_buffer不能为空指针，size必须大于0
 *
 * @example
 * @code
 * uint8_t data[10];
 * HAL_IIC_Receive(I2C0, 0xA0, 0x00, data, 10, 1000); // 从地址0xA0的设备内存0x00读取10字节数据，使用1000ms超时
 * @endcode
 */
void HAL_IIC_Receive(
    uint32_t i2c_periph, uint32_t dev_addr, uint32_t mem_addr, uint8_t *rx_buffer, size_t size, uint32_t time_out)
{
    uint32_t now_tick = GetTick();

    /* 发送要通讯的地址 */
    i2c_start_on_bus(i2c_periph);
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    // 发送器件地址加写操作
    i2c_master_addressing(i2c_periph, dev_addr, I2C_TRANSMITTER);
    // 等待器件地址发送成功
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    while (!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    // 发送要读取的地址
    i2c_data_transmit(i2c_periph, mem_addr);

    while (!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    // 发送停止信号
    i2c_stop_on_bus(i2c_periph);

    /* 读取数据 */
    i2c_start_on_bus(i2c_periph);
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    // 发送器件地址加读操作
    i2c_master_addressing(i2c_periph, dev_addr, I2C_RECEIVER);
    // 等待器件地址发送成功
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    // 清除地址发送成功标志位
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    // 使能自动发送应答
    i2c_ack_config(I2C0, I2C_ACK_ENABLE);

    // 等待数据寄存器非空标志位置一
    for (uint32_t i = 0UL; i < size; i++)
    {
        /* 在接收最后一个字节前禁用ACK */
        if (i == (size - 1U))
        {
            i2c_ack_config(i2c_periph, I2C_ACK_DISABLE);
        }

        while (!i2c_flag_get(i2c_periph, I2C_FLAG_RBNE))
        {
            if (GetTick() - now_tick >= time_out) return;
        }
        // 接收数据
        rx_buffer[i] = i2c_data_receive(i2c_periph);
    }

    /* 重新启用ACK为下次传输做准备 */
    i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);
    i2c_stop_on_bus(i2c_periph);
}

/**
 * @brief I2C主机发送数据函数
 * @details 向指定I2C设备的指定内存地址写入指定长度的数据
 *
 * @param i2c_periph I2C外设基地址 (如 I2C0, I2C1)
 * @param dev_addr 设备地址 (7位地址，需左移1位)
 * @param mem_addr 要写入的内存地址
 * @param tx_buffer 发送数据缓冲区指针
 * @param size 要发送的数据字节数
 * @param time_out 超时时间，单位毫秒
 * @retval None
 * @note 该函数使用标准I2C协议，先发送内存地址再写入数据，使用阻塞方式等待发送完成，超时后直接返回
 * @warning tx_buffer不能为空指针，size必须大于0
 *
 * @example
 * @code
 * uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
 * HAL_IIC_Transmit(I2C0, 0xA0, 0x00, data, 4, 1000); // 向地址0xA0的设备内存0x00写入4字节数据，使用1000ms超时
 * @endcode
 */
void HAL_IIC_Transmit(
    uint32_t i2c_periph, uint32_t dev_addr, uint32_t mem_addr, uint8_t *tx_buffer, size_t size, uint32_t time_out)
{
    uint32_t now_tick = GetTick();

    /* 发送要通讯的地址 */
    i2c_start_on_bus(i2c_periph);
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    // 发送器件地址加写操作
    i2c_master_addressing(i2c_periph, dev_addr, I2C_TRANSMITTER);
    // 等待器件地址发送成功
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);

    while (!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
    {
        if (GetTick() - now_tick >= time_out) return;
    }
    // 发送要写入的内存地址
    i2c_data_transmit(i2c_periph, mem_addr);
    while (!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
    {
        if (GetTick() - now_tick >= time_out) return;
    }

    for (uint32_t i = 0UL; i < size; i++)
    {
        while (!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
        {
            if (GetTick() - now_tick >= time_out) return;
        }
        // 发送数据
        i2c_data_transmit(i2c_periph, tx_buffer[i]);
        while (!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
        {
            if (GetTick() - now_tick >= time_out) return;
        }
    }

    // 发送停止信号
    i2c_stop_on_bus(i2c_periph);
}
