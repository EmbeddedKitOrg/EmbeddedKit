#include "../inc/ek_hal_tick.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_tick_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 Tick 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_tick_register(ek_hal_tick_t *const dev, const char *name, const ek_tick_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_tick_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    ek_list_add_tail(&ek_hal_tick_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 Tick 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_tick_t *ek_hal_tick_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_tick_head)
    {
        ek_hal_tick_t *dev = ek_list_container(p, ek_hal_tick_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 获取当前系统节拍计数
 * @param dev 设备实例指针
 * @return 当前节拍计数值
 */
uint32_t ek_hal_tick_get(ek_hal_tick_t *const dev)
{
    ek_assert_param(dev != NULL);

    return dev->ops->get(dev);
}

/**
 * @brief 延时指定节拍数
 * @param dev 设备实例指针
 * @param xtick 延时节拍数
 */
void ek_hal_tick_delay(ek_hal_tick_t *const dev, uint32_t xtick)
{
    ek_assert_param(dev != NULL);

    dev->ops->delay(dev, xtick);
}
