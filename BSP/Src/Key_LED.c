#include "Key_LED.h"

uint8_t Key_ReadRaw(void)
{
    uint8_t temp = 0;
    if (HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin) == 1) temp = 1;

    return temp;
}