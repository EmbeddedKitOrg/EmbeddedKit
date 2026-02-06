#ifndef EK_HAL_GPIO_H
#define EK_HAL_GPIO_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_gpio_t ek_hal_gpio_t;

struct ek_hal_gpio_t
{
    uint8_t idx;
    ek_list_node_t node;

    uint8_t (*read)(void);
    void (*set)(void);
    void (*reset)(void);
    void (*toggle)(void);
};

extern ek_list_node_t ek_hal_gpio_head;

extern ek_hal_gpio_t hal_drv_gpio_xxx_pin;

void ek_hal_gpio_init(void);

#endif // EK_HAL_GPIO_H
