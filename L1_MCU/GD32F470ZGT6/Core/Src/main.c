#include "main.h"
#include "bsp_interface.h"
#include "hal_interface.h"

extern void ek_main(void);

int main(void)
{
    // NVIC Group
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    //
    BSP_Timer_Tick_Init();

    BSP_SDIO_Init();
    BSP_GPIO_Init();
    BSP_DMA_Init();
    BSP_Timer_LCD_Init();
    BSP_Timer_Motor_Init();
    BSP_Timer_DAC_Init();
    BSP_SPI_Init();
    BSP_IIC_Init();
    BSP_LED_Key_Init();
    BSP_EXMC_Init();
    BSP_USART_Init(115200);
    BSP_ADC_Init();
    BSP_DAC_Init();
    
    ek_main();

    while (1)
    {
    }
}

void Error_Handler(void)
{
    while (1)
    {
    }
}
