#include "bsp_gpio.h"

/**
 * @brief 初始化统一配置的GPIO外设引脚
 * @details 配置需要统一管理的GPIO引脚，包括：
 *          - LCD控制引脚：CS、RESET、DC信号
 * @param None
 * @retval None
 * @note 此函数配置不适合分散到各个外设模块中的GPIO引脚
 * @note 各个外设专用的GPIO引脚已移到对应的外设初始化函数中配置
 */
void BSP_GPIO_Init(void)
{
    // 使能GPIO时钟
    rcu_periph_clock_enable(RCU_GPIOA);  /* LCD控制引脚 */
    rcu_periph_clock_enable(RCU_GPIOF);  /* LCD复位引脚 */

    /*=== LCD控制引脚 ===*/
    // CS
    gpio_mode_set(BSP_SPI_LCD_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BSP_SPI_LCD_CS_PIN);
    gpio_output_options_set(BSP_SPI_LCD_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_LCD_CS_PIN);

    // RESET
    gpio_mode_set(BSP_SPI_LCD_RESET_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BSP_SPI_LCD_RESET_PIN);
    gpio_output_options_set(BSP_SPI_LCD_RESET_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_LCD_RESET_PIN);

    // DC
    gpio_mode_set(BSP_SPI_LCD_DC_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, BSP_SPI_LCD_DC_PIN);
    gpio_output_options_set(BSP_SPI_LCD_DC_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_LCD_DC_PIN);
}