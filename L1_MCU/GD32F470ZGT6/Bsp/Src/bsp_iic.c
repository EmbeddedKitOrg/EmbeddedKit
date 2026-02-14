#include "bsp_iic.h"

/**
 * @brief I2C外设初始化函数
 * @details 初始化EEPROM使用的I2C外设，包括GPIO配置和I2C参数设置
 *
 * @param None
 * @retval None
 * @note 该函数配置I2C速率为400KHz，使用开漏输出模式，GPIO设置为复用功能4
 * @warning 调用此函数前确保相关GPIO和I2C外设未被其他模块占用
 *
 * 配置详情：
 * - I2C速率：400KHz
 * - GPIO模式：复用功能，上拉电阻，开漏输出
 * - GPIO速度：50MHz
 * - I2C地址格式：7位地址模式
 * - 主机地址：0x01
 *
 * @example
 * @code
 * // 在系统初始化时调用
 * BSP_IIC_Init();
 * @endcode
 */
void BSP_IIC_Init(void)
{
    // 使能I2C和GPIO时钟
    rcu_periph_clock_enable(RCU_GPIOB);  /* I2C SDA、SCL引脚 */
    rcu_periph_clock_enable(BSP_EEPROM_IIC_RCU);   /* I2C0外设时钟 */

    // I2C GPIO配置
    gpio_af_set(BSP_IIC_EEPROM_SDA_PORT, GPIO_AF_4, BSP_IIC_EEPROM_SDA_PIN);
    gpio_af_set(BSP_IIC_EEPROM_SCL_PORT, GPIO_AF_4, BSP_IIC_EEPROM_SCL_PIN);

    // SDA
    gpio_mode_set(BSP_IIC_EEPROM_SDA_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_IIC_EEPROM_SDA_PIN);
    gpio_output_options_set(BSP_IIC_EEPROM_SDA_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, BSP_IIC_EEPROM_SDA_PIN);

    // SCL
    gpio_mode_set(BSP_IIC_EEPROM_SCL_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_IIC_EEPROM_SCL_PIN);
    gpio_output_options_set(BSP_IIC_EEPROM_SCL_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, BSP_IIC_EEPROM_SCL_PIN);

    // iic
    i2c_deinit(BSP_EEPROM_IIC);
    i2c_clock_config(BSP_EEPROM_IIC, 400000, I2C_DTCY_2); // 配置iic速率400K
    i2c_mode_addr_config(I2C0, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, 0x01); // 设置主机地址
    i2c_ack_config(BSP_EEPROM_IIC, I2C_ACK_ENABLE); // 启动iic应答

    i2c_enable(BSP_EEPROM_IIC);
}
