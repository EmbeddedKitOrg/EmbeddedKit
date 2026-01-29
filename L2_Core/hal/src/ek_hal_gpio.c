#include "../inc/ek_hal_gpio.h"
#include "../../utils/inc/ek_export.h"

ek_list_node_t ek_hal_gpio_head;

static void xxx_pin_set(vopd);
static void xxx_pin_reset(vopd);
static void xxx_pin_toggle(vopd);

ek_hal_gpio_t hal_drv_gpio_xxx_pin = {
    .idx = 1,
    .reset = xxx_pin_set,
    .set = xxx_pin_reset,
    .toggle = xxx_pin_toggle,
};

static void xxx_pin_set(vopd)
{
}

static void xxx_pin_reset(vopd)
{
}

static void xxx_pin_toggle(vopd)
{
}

void ek_hal_gpio_init(void)
{
    ek_list_init(&ek_hal_gpio_head);
    ek_list_add_tail(&ek_hal_gpio_head, &hal_drv_gpio_xxx_pin.node);
}

EK_EXPORT(ek_hal_gpio_init, 0);
