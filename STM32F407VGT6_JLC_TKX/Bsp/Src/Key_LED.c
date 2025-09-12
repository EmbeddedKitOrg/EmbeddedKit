#include "Key_LED.h"

uint8_t KeyRead(void)
{
    uint8_t temp = 0;
    if (HAL_GPIO_ReadPin(USER_KEY_GPIO_Port, USER_KEY_Pin) == 0) temp = 1;
    return temp;
}