#include "Interrupt.h"

/*SERIAL START*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart == USER_UART)
    {
        Serial_Idle_Handler();
    }
}
/*SERIAL END*/

/*OLED START*/
#if defined(OLED_SPI)

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == OLED_SPI)
    {
        OLED_SPI_TxCplt_Handler();
    }
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi == OLED_SPI)
    {
        OLED_SPI_Error_Handler();
    }
}

#elif defined(OLED_I2C)

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == OLED_I2C)
    {
        OLED_I2C_MasterTxCplt_Handler();
    }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == OLED_I2C)
    {
        OLED_I2C_MemTxCplt_Handler();
    }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c == OLED_I2C)
    {
        OLED_I2C_Error_Handler();
    }
}

#endif
/*OLED END*/