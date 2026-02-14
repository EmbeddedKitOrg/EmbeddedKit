#include "bsp_spi.h"

/**
  * @brief  初始化SPI外设
  * @details 该函数配置并使能SPI外设，包括：
  *          - 使能SPI和GPIO时钟
  *          - 配置SPI引脚为复用功能模式
  *          - 配置SPI为主机模式，全双工通信
  *          - 设置8位数据帧，MSB先行
  *          - 配置时钟极性和相位
  *          - 设置软件片选，并初始化片选信号为低电平
  * @param  无
  * @retval 无
  */
void BSP_SPI_Init(void)
{
    // 外设时钟开启
    rcu_periph_clock_enable(RCU_GPIOF); /* SPI Flash通信引脚 */
    rcu_periph_clock_enable(BSP_SPI_FLASH_RCU); /* SPI4外设时钟 */

    // 设置复用引脚 LCD 和 Flash 公用一个SPI 所以只用初始化一次
    gpio_af_set(BSP_SPI_FLASH_DI_PORT, GPIO_AF_5, BSP_SPI_FLASH_DI_PIN);
    gpio_af_set(BSP_SPI_FLASH_DO_PORT, GPIO_AF_5, BSP_SPI_FLASH_DO_PIN);
    gpio_af_set(BSP_SPI_FLASH_CK_PORT, GPIO_AF_5, BSP_SPI_FLASH_CK_PIN);

    // 设置IO模式
    gpio_mode_set(BSP_SPI_FLASH_DI_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_SPI_FLASH_DI_PIN);
    gpio_mode_set(BSP_SPI_FLASH_DO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_SPI_FLASH_DO_PIN);
    gpio_mode_set(BSP_SPI_FLASH_CK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_SPI_FLASH_CK_PIN);

    // 设置输出模式
    gpio_output_options_set(BSP_SPI_FLASH_DI_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_FLASH_DI_PIN);
    gpio_output_options_set(BSP_SPI_FLASH_DO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_FLASH_DO_PIN);
    gpio_output_options_set(BSP_SPI_FLASH_CK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_FLASH_CK_PIN);

    // 配置CS片选模式
    gpio_mode_set(BSP_SPI_FLASH_CS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, BSP_SPI_FLASH_CS_PIN);
    gpio_output_options_set(BSP_SPI_FLASH_CS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_SPI_FLASH_CS_PIN);

    // 配置SPI
    spi_parameter_struct bsp_spi_init_struct;
    spi_struct_para_init(&bsp_spi_init_struct);
    bsp_spi_init_struct.device_mode = SPI_MASTER; // 主机
    bsp_spi_init_struct.trans_mode = SPI_TRANSMODE_FULLDUPLEX; // 全双工
    bsp_spi_init_struct.frame_size = SPI_FRAMESIZE_8BIT; // 8位数据
    bsp_spi_init_struct.nss = SPI_NSS_SOFT; // 软件片选
    bsp_spi_init_struct.endian = SPI_ENDIAN_MSB; // MSB
    bsp_spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE; // 极性相位
    bsp_spi_init_struct.prescale = SPI_PSC_2; // 预分频器因数为2

    spi_init(BSP_FLASH_SPI, &bsp_spi_init_struct);
    spi_enable(BSP_FLASH_SPI); // 使能SPI

    gpio_bit_reset(BSP_SPI_FLASH_CS_PORT, BSP_SPI_FLASH_CS_PIN); // 拉低片选
}
