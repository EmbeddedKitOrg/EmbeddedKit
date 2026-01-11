#ifndef EK_HAL_BASE_H
#define EK_HAL_BASE_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef enum
{
    // 通信类
    HAL_PERIPH_UART = 0,
    HAL_PERIPH_USART,
    HAL_PERIPH_SPI,
    HAL_PERIPH_I2C,
    HAL_PERIPH_I2S,
    HAL_PERIPH_CAN,
    HAL_PERIPH_USB,
    HAL_PERIPH_SDIO,

    // 定时器类
    HAL_PERIPH_TIM,
    HAL_PERIPH_PWM,
    HAL_PERIPH_WWDG,
    HAL_PERIPH_IWDG,
    HAL_PERIPH_RTC,

    // 模拟类
    HAL_PERIPH_ADC,
    HAL_PERIPH_DAC,

    // GPIO和中断
    HAL_PERIPH_GPIO,
    HAL_PERIPH_EXTI,

    // DMA
    HAL_PERIPH_DMA,

    // 存储
    HAL_PERIPH_FLASH,

    // 系统类
    HAL_PERIPH_PWR,
    HAL_PERIPH_RCC,

    // 特殊功能
    HAL_PERIPH_RNG,

    HAL_PERIPH_MAX,
} ek_hal_periph_type_t;

typedef struct ek_hal_base_t ek_hal_base_t;

struct ek_hal_base_t
{
    uint8_t idx;
    ek_hal_periph_type_t type;
    ek_list_node_t node;
};

#endif /* EK_HAL_BASE_H */
