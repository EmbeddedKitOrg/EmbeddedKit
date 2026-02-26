#ifndef EK_HAL_GPIO_H
#define EK_HAL_GPIO_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_gpio_t ek_hal_gpio_t;
typedef struct ek_gpio_ops_t ek_gpio_ops_t;

/** @brief GPIO 工作模式 */
typedef enum
{
    EK_GPIO_MODE_INPUT = 0,
    EK_GPIO_MODE_INPUT_PULLUP,
    EK_GPIO_MODE_INPUT_PULLDOWN,
    EK_GPIO_MODE_OUTPUT_PP,
    EK_GPIO_MODE_OUTPUT_OD,

    EK_GPIO_MODE_MAX
} ek_gpio_mode_t;

/** @brief GPIO 电平状态 */
typedef enum
{
    EK_GPIO_STATUS_RESET = 0,
    EK_GPIO_STATUS_SET,
} ek_gpio_status_t;

/** @brief GPIO 操作函数集 */
struct ek_gpio_ops_t
{
    void (*init)(ek_hal_gpio_t *const dev, ek_gpio_mode_t mode);
    ek_gpio_status_t (*read)(ek_hal_gpio_t *const dev);
    void (*set)(ek_hal_gpio_t *const dev, ek_gpio_status_t status);
    void (*toggle)(ek_hal_gpio_t *const dev);
};

/** @brief GPIO 设备结构体 */
struct ek_hal_gpio_t
{
    ek_list_node_t node;
    const char *name;
    const ek_gpio_ops_t *ops;
    void *dev_info;

    ek_gpio_mode_t mode;
    ek_gpio_status_t status;
};

extern ek_list_node_t ek_hal_gpio_head;

void ek_hal_gpio_register(
    ek_hal_gpio_t *const dev, const char *name, ek_gpio_mode_t mode, const ek_gpio_ops_t *ops, void *dev_info);
ek_hal_gpio_t *ek_hal_gpio_find(const char *name);
void ek_hal_gpio_set(ek_hal_gpio_t *const dev, ek_gpio_status_t status);
void ek_hal_gpio_toggle(ek_hal_gpio_t *const dev);
ek_gpio_status_t ek_hal_gpio_read(ek_hal_gpio_t *const dev);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_GPIO_H
