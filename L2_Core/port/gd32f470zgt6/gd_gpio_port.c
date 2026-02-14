#include "../../hal/inc/ek_hal_gpio.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "gd32f4xx_gpio.h"

// 具体设备的硬件信息
typedef struct
{
    ek_hal_gpio_t *dev;
    const char *name;
    ek_gpio_mode_t mode;
    uint32_t hw_port;
    uint32_t hw_pin;
} gd_gpio_info;

// 具体设备
static ek_hal_gpio_t drv_led1;
static ek_hal_gpio_t drv_led2;
static ek_hal_gpio_t drv_led3;
static ek_hal_gpio_t drv_led4;
static ek_hal_gpio_t drv_key_l;
static ek_hal_gpio_t drv_key_a;
static ek_hal_gpio_t drv_key_b;
static ek_hal_gpio_t drv_key_r;
static ek_hal_gpio_t drv_flash_cs;
static ek_hal_gpio_t drv_lcd_dc;
static ek_hal_gpio_t drv_lcd_cs;
static ek_hal_gpio_t drv_lcd_reset;

// 设备表
static const gd_gpio_info gd_drv_gpio_table[] = {
    { &drv_led1,       "LED1",       EK_GPIO_MODE_OUTPUT_PP, GPIOE, BIT(3)  },
    { &drv_led2,       "LED2",       EK_GPIO_MODE_OUTPUT_PP, GPIOD, BIT(7)  },
    { &drv_led3,       "LED3",       EK_GPIO_MODE_OUTPUT_PP, GPIOG, BIT(3)  },
    { &drv_led4,       "LED4",       EK_GPIO_MODE_OUTPUT_PP, GPIOA, BIT(5)  },
    { &drv_key_l,      "KEY_L",      EK_GPIO_MODE_INPUT,     GPIOA, BIT(0)  },
    { &drv_key_a,      "KEY_A",      EK_GPIO_MODE_INPUT,     GPIOG, BIT(9)  },
    { &drv_key_b,      "KEY_B",      EK_GPIO_MODE_INPUT,     GPIOB, BIT(15) },
    { &drv_key_r,      "KEY_R",      EK_GPIO_MODE_INPUT,     GPIOB, BIT(2)  },
    { &drv_flash_cs,   "FLASH_CS",   EK_GPIO_MODE_OUTPUT_PP, GPIOF, BIT(6)  },
    { &drv_lcd_dc,     "LCD_DC",     EK_GPIO_MODE_OUTPUT_PP, GPIOA, BIT(6)  },
    { &drv_lcd_cs,     "LCD_CS",     EK_GPIO_MODE_OUTPUT_PP, GPIOA, BIT(4)  },
    { &drv_lcd_reset,  "LCD_RESET",  EK_GPIO_MODE_OUTPUT_PP, GPIOF, BIT(10) }
};

// ops 实现
static void _init(ek_hal_gpio_t *const dev, ek_gpio_mode_t mode);
static void _set(ek_hal_gpio_t *const dev, ek_gpio_status_t status);
static void _toggle(ek_hal_gpio_t *const dev);
static ek_gpio_status_t _read(ek_hal_gpio_t *const dev);

static const ek_gpio_ops_t gd_gpio_ops = {
    .init = _init,
    .set = _set,
    .toggle = _toggle,
    .read = _read,
};

// 注册到hal
void gd_gpio_drv_init(void)
{
    for (uint8_t i = 0; i < sizeof(gd_drv_gpio_table) / sizeof(gd_drv_gpio_table[0]); i++)
    {
        ek_hal_gpio_register(gd_drv_gpio_table[i].dev,
                             gd_drv_gpio_table[i].name,
                             gd_drv_gpio_table[i].mode,
                             &gd_gpio_ops,
                             &gd_drv_gpio_table[i]);
    }
}

// 直接使用自动导出模块导出，如果不使用这个功能则需要显式调用这个初始化函数
EK_EXPORT_HARDWARE(gd_gpio_drv_init);

// 内部函数
static void _init(ek_hal_gpio_t *const dev, ek_gpio_mode_t mode)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(mode < EK_GPIO_MODE_MAX);

    // 用户提到初始化已经实现，这里留空
    // GD32 的 GPIO 初始化已经在 BSP 层完成
}

static void _set(ek_hal_gpio_t *const dev, ek_gpio_status_t status)
{
    ek_assert_param(dev != NULL);

    gd_gpio_info *drv_data = (gd_gpio_info *)dev->dev_info;

    if (status == EK_GPIO_STATUS_SET)
    {
        gpio_bit_set(drv_data->hw_port, drv_data->hw_pin);
    }
    else
    {
        gpio_bit_reset(drv_data->hw_port, drv_data->hw_pin);
    }
}

static void _toggle(ek_hal_gpio_t *const dev)
{
    ek_assert_param(dev != NULL);

    gd_gpio_info *drv_data = (gd_gpio_info *)dev->dev_info;

    gpio_bit_toggle(drv_data->hw_port, drv_data->hw_pin);
}

static ek_gpio_status_t _read(ek_hal_gpio_t *const dev)
{
    ek_assert_param(dev != NULL);

    gd_gpio_info *drv_data = (gd_gpio_info *)dev->dev_info;

    return (ek_gpio_status_t)gpio_input_bit_get(drv_data->hw_port, drv_data->hw_pin);
}
