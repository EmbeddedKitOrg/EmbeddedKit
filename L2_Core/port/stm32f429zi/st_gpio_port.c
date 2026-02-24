#include "../../hal/inc/ek_hal_gpio.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "gpio.h"

// 具体设备的硬件信息
typedef struct
{
    ek_hal_gpio_t *dev;
    const char *name;
    ek_gpio_mode_t mode;
    GPIO_TypeDef *hw_port;
    uint16_t hw_pin;
} st_gpio_info;

// 具体设备
static ek_hal_gpio_t drv_lcd_cs;
static ek_hal_gpio_t drv_lcd_wrx;
static ek_hal_gpio_t drv_lcd_rdx;
static ek_hal_gpio_t drv_gyro_cs;
static ek_hal_gpio_t drv_led_green;
static ek_hal_gpio_t drv_led_red;
static ek_hal_gpio_t drv_key;

// 设备表
static const st_gpio_info st_drv_gpio_table[] = {
    { &drv_lcd_cs,    "LCD_CS",    EK_GPIO_MODE_OUTPUT_PP, GPIOC, GPIO_PIN_2  },
    { &drv_lcd_wrx,   "LCD_WRX",   EK_GPIO_MODE_OUTPUT_PP, GPIOD, GPIO_PIN_13 },
    { &drv_lcd_rdx,   "LCD_RDX",   EK_GPIO_MODE_OUTPUT_PP, GPIOD, GPIO_PIN_12 },
    { &drv_gyro_cs,   "GYRO_CS",   EK_GPIO_MODE_OUTPUT_PP, GPIOC, GPIO_PIN_1  },
    { &drv_led_green, "LED GREEN", EK_GPIO_MODE_OUTPUT_PP, GPIOG, GPIO_PIN_13 },
    { &drv_led_red,   "LED RED",   EK_GPIO_MODE_OUTPUT_PP, GPIOG, GPIO_PIN_14 },
    { &drv_key,       "KEY",       EK_GPIO_MODE_INPUT,     GPIOA, GPIO_PIN_0  }
};

// ops 实现
static void _init(ek_hal_gpio_t *const dev, ek_gpio_mode_t mode);
static void _set(ek_hal_gpio_t *const dev, ek_gpio_status_t status);
static void _toggle(ek_hal_gpio_t *const dev);
static ek_gpio_status_t _read(ek_hal_gpio_t *const dev);

static const ek_gpio_ops_t st_gpio_ops = {
    .init = _init,
    .set = _set,
    .toggle = _toggle,
    .read = _read,
};

// 注册到hal
void st_gpio_drv_init(void)
{
    for (uint8_t i = 0; i < EK_ARRAY_LEN(st_drv_gpio_table); i++)
    {
        ek_hal_gpio_register(st_drv_gpio_table[i].dev,
                             st_drv_gpio_table[i].name,
                             st_drv_gpio_table[i].mode,
                             &st_gpio_ops,
                             &st_drv_gpio_table[i]);
    }
}

// 直接使用自动导出模块导出，如果不使用这个功能则需要显式调用这个初始化函数
EK_EXPORT_HARDWARE(st_gpio_drv_init);

// 内部函数
static void _init(ek_hal_gpio_t *const dev, ek_gpio_mode_t mode)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(mode < EK_GPIO_MODE_MAX);

    // CubeMX已经做了初始化
    // 如果是其他芯片可以在这里补充初始化逻辑
    // @exmpale
    // st_gpio_info *drv_data = (st_gpio_info *)dev->dev_info;
    // GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    // if (mode == EK_GPIO_MODE_INPUT)
    // {
    //     GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    //     GPIO_InitStruct.Pull = GPIO_NOPULL;
    // }
    // else if (mode == EK_GPIO_MODE_INPUT_PULLUP)
    // {
    //     GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    //     GPIO_InitStruct.Pull = GPIO_PULLUP;
    // }
    // else if (mode == EK_GPIO_MODE_INPUT_PULLDOWN)
    // {
    //     GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    //     GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    // }
    // else if (mode == EK_GPIO_MODE_OUTPUT_PP)
    // {
    //     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    //     GPIO_InitStruct.Pull = GPIO_NOPULL;
    // }
    // else if (mode == EK_GPIO_MODE_OUTPUT_OD)
    // {
    //     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    //     GPIO_InitStruct.Pull = GPIO_PULLUP;
    // }
    // GPIO_InitStruct.Pin = drv_data->hw_pin;
    // GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    // HAL_GPIO_Init(drv_data->hw_port, &GPIO_InitStruct);
}

static void _set(ek_hal_gpio_t *const dev, ek_gpio_status_t status)
{
    ek_assert_param(dev != NULL);

    st_gpio_info *drv_data = (st_gpio_info *)dev->dev_info;
    GPIO_PinState state = (GPIO_PinState)status;

    HAL_GPIO_WritePin(drv_data->hw_port, drv_data->hw_pin, state);
}

static void _toggle(ek_hal_gpio_t *const dev)
{
    ek_assert_param(dev != NULL);

    st_gpio_info *drv_data = (st_gpio_info *)dev->dev_info;

    HAL_GPIO_TogglePin(drv_data->hw_port, drv_data->hw_pin);
}

static ek_gpio_status_t _read(ek_hal_gpio_t *const dev)
{
    ek_assert_param(dev != NULL);

    st_gpio_info *drv_data = (st_gpio_info *)dev->dev_info;

    return (ek_gpio_status_t)HAL_GPIO_ReadPin(drv_data->hw_port, drv_data->hw_pin);
}
