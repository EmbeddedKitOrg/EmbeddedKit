#ifndef __KEY_LED_H
#define __KEY_LED_H

#include "SysConfig.h"

#define LED_TOGGLE() HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin)

uint8_t KeyRead(void);

#endif
