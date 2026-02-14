#ifndef __LED_KEY_H
#define __LED_KEY_H

#include "bsp_io_define.h"

// key enum
typedef enum
{
    KEY_VAL_NONE = 0,
    KEY_VAL_L,
    KEY_VAL_A,
    KEY_VAL_B,
    KEY_VAL_R
} KeyVal_t;

// LED
#define LED1_TOGGLE() gpio_bit_toggle(BSP_LED1_PORT, BSP_LED1_PIN)
#define LED2_TOGGLE() gpio_bit_toggle(BSP_LED2_PORT, BSP_LED2_PIN)
#define LED3_TOGGLE() gpio_bit_toggle(BSP_LED3_PORT, BSP_LED3_PIN)
#define LED4_TOGGLE() gpio_bit_toggle(BSP_LED4_PORT, BSP_LED4_PIN)

#define LED1_VALUE(X) gpio_bit_write(BSP_LED1_PORT, BSP_LED1_PIN, (bit_status)(X))
#define LED2_VALUE(X) gpio_bit_write(BSP_LED2_PORT, BSP_LED2_PIN, (bit_status)(X))
#define LED3_VALUE(X) gpio_bit_write(BSP_LED3_PORT, BSP_LED3_PIN, (bit_status)(X))
#define LED4_VALUE(X) gpio_bit_write(BSP_LED4_PORT, BSP_LED4_PIN, (bit_status)(X))

void BSP_LED_Key_Init(void);
KeyVal_t BSP_KeyRead(void);

#endif /* __LED_KEY_H */
