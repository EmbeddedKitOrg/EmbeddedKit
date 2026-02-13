#include "../inc/ek_hal_gpio.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"

ek_list_node_t ek_hal_gpio_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 GPIO 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param mode GPIO的模式
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_gpio_register(
    ek_hal_gpio_t *const dev, const char *name, ek_gpio_mode_t mode, const ek_gpio_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);
    ek_assert_param(mode < EK_GPIO_MODE_MAX);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_gpio_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->mode = mode;
    dev->ops = ops;
    dev->dev_info = dev_info;
    ek_list_add_tail(&ek_hal_gpio_head, &dev->node);

    dev->ops->init(dev, mode);
    dev->status = dev->ops->read(dev);
}

/**
 * @brief 按名称查找已注册的 GPIO 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_gpio_t *ek_hal_gpio_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_gpio_head)
    {
        ek_hal_gpio_t *dev = ek_list_container(p, ek_hal_gpio_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 设置 GPIO 输出电平
 * @param dev 设备实例指针
 * @param status 目标电平状态
 */
void ek_hal_gpio_set(ek_hal_gpio_t *const dev, ek_gpio_status_t status)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(dev->mode == EK_GPIO_MODE_OUTPUT_OD || dev->mode == EK_GPIO_MODE_OUTPUT_PP);

    dev->ops->set(dev, status);
    dev->status = status;
}

/**
 * @brief 翻转 GPIO 输出电平
 * @param dev 设备实例指针
 */
void ek_hal_gpio_toggle(ek_hal_gpio_t *const dev)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(dev->mode == EK_GPIO_MODE_OUTPUT_OD || dev->mode == EK_GPIO_MODE_OUTPUT_PP);

    dev->status = (dev->status == EK_GPIO_STATUS_SET) ? EK_GPIO_STATUS_RESET : EK_GPIO_STATUS_SET;

    dev->ops->toggle(dev);
}

/**
 * @brief 读取 GPIO 当前电平状态
 * @param dev 设备实例指针
 * @return 当前电平状态
 */
ek_gpio_status_t ek_hal_gpio_read(ek_hal_gpio_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->status = dev->ops->read(dev);

    return dev->status;
}
