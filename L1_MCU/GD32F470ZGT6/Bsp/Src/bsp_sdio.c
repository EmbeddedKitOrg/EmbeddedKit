#include "bsp_sdio.h"

void BSP_SDIO_Init(void)
{
    /* 打开SDIO与相关GPIO时钟 */
    rcu_periph_clock_enable(BSP_SDIO_RCU);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_GPIOD);

    /* 复用功能映射 */
    gpio_af_set(BSP_SDIO_D0_PORT, GPIO_AF_12, BSP_SDIO_D0_PIN);
    gpio_af_set(BSP_SDIO_D1_PORT, GPIO_AF_12, BSP_SDIO_D1_PIN);
    gpio_af_set(BSP_SDIO_D2_PORT, GPIO_AF_12, BSP_SDIO_D2_PIN);
    gpio_af_set(BSP_SDIO_D3_PORT, GPIO_AF_12, BSP_SDIO_D3_PIN);
    gpio_af_set(BSP_SDIO_CLK_PORT, GPIO_AF_12, BSP_SDIO_CLK_PIN);
    gpio_af_set(BSP_SDIO_CMD_PORT, GPIO_AF_12, BSP_SDIO_CMD_PIN);

    /* 数据线上拉，推挽输出 */
    gpio_mode_set(BSP_SDIO_D0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_SDIO_D0_PIN);
    gpio_mode_set(BSP_SDIO_D1_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_SDIO_D1_PIN);
    gpio_mode_set(BSP_SDIO_D2_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_SDIO_D2_PIN);
    gpio_mode_set(BSP_SDIO_D3_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_SDIO_D3_PIN);
    gpio_output_options_set(BSP_SDIO_D0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, BSP_SDIO_D0_PIN);
    gpio_output_options_set(BSP_SDIO_D1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, BSP_SDIO_D1_PIN);
    gpio_output_options_set(BSP_SDIO_D2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, BSP_SDIO_D2_PIN);
    gpio_output_options_set(BSP_SDIO_D3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, BSP_SDIO_D3_PIN);

    /* 时钟线浮空输入，推挽输出 */
    gpio_mode_set(BSP_SDIO_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_SDIO_CLK_PIN);
    gpio_output_options_set(BSP_SDIO_CLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, BSP_SDIO_CLK_PIN);

    /* CMD线上拉 */
    gpio_mode_set(BSP_SDIO_CMD_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_SDIO_CMD_PIN);
    gpio_output_options_set(BSP_SDIO_CMD_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, BSP_SDIO_CMD_PIN);

    /* 复位SDIO外设，保持在默认状态 */
    sdio_deinit();
    sdio_clock_disable();

    nvic_irq_enable(BSP_SDIO_IRQ, 1, 0);
}