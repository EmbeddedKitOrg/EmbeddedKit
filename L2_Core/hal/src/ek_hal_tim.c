#include "../inc/ek_hal_tim.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_tim_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册定时器设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_tim_register(ek_hal_tim_base_t *const dev, const char *name, const ek_tim_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_tim_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    dev->state = EK_HAL_TIM_STATE_STP;
    ek_list_add_tail(&ek_hal_tim_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的定时器设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_tim_base_t *ek_hal_tim_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_tim_head)
    {
        ek_hal_tim_base_t *dev = ek_list_container(p, ek_hal_tim_base_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 启动定时器
 * @param dev 设备实例指针
 */
void ek_hal_tim_start(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->start(dev);
    dev->state = EK_HAL_TIM_STATE_RUN;
}

/**
 * @brief 停止定时器
 * @param dev 设备实例指针
 */
void ek_hal_tim_stop(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->stop(dev);
    dev->state = EK_HAL_TIM_STATE_STP;
}

/**
 * @brief 获取定时器当前计数值
 * @param dev 设备实例指针
 * @return 当前计数值
 */
uint32_t ek_hal_tim_get(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);

    return dev->ops->get(dev);
}

/**
 * @brief 设置定时器计数值
 * @param dev 设备实例指针
 * @param value 目标计数值
 */
void ek_hal_tim_set(ek_hal_tim_base_t *const dev, uint32_t value)
{
    ek_assert_param(dev != NULL);

    dev->ops->set(dev, value);
}
