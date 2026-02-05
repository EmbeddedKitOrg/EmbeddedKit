#include "../inc/ek_hal_gpio.h"
#include "../../utils/inc/ek_export.h"

ek_list_node_t ek_hal_gpio_head;

static void xxx_pin_read(void);
static void xxx_pin_set(void);
static void xxx_pin_reset(void);
static void xxx_pin_toggle(void);

ek_hal_gpio_t hal_drv_gpio_xxx_pin = {
    .idx = 1,
    .read = xxx_pin_read,
    .reset = xxx_pin_set,
    .set = xxx_pin_reset,
    .toggle = xxx_pin_toggle,
};

static void xxx_pin_read(void)
{
    // gpio设置读取电平的底层
    // e.g. HAL_GPIO_ReadPin(x, x)
}


static void xxx_pin_set(void)
{
    // gpio设置高电平的底层
    // e.g. HAL_GPIO_WritePin(x, x, 1)
}

static void xxx_pin_reset(void)
{
    // gpio设置低电平的底层
    // e.g. HAL_GPIO_WritePin(x, x, 0)
}

static void xxx_pin_toggle(void)
{
    // gpio翻转电平的底层
    // e.g. HAL_GPIO_TogglePin(x, x)
}

void ek_hal_gpio_init(void)
{
    ek_list_init(&ek_hal_gpio_head);
    ek_list_add_tail(&ek_hal_gpio_head, &hal_drv_gpio_xxx_pin.node);
}

EK_EXPORT(ek_hal_gpio_init, 0);
