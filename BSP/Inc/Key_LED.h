#ifndef __KEY_LED_H
#define __KEY_LED_H

#include "Common.h"

#define LED_JLC_TOGGLE() HAL_GPIO_TogglePin(LED_JLC_GPIO_Port, LED_JLC_Pin)

uint8_t Key_ReadRaw(void);

#endif/* __KEY_LED_H */
