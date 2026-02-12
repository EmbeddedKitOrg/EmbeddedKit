#include "../inc/ek_hal_ltdc.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_ltdc_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 LTDC 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_ltdc_register(ek_hal_ltdc_t *const dev, const char *name, const ek_ltdc_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_ltdc_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    ek_list_add_tail(&ek_hal_ltdc_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 LTDC 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_ltdc_t *ek_hal_ltdc_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_ltdc_head)
    {
        ek_hal_ltdc_t *dev = ek_list_container(p, ek_hal_ltdc_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 设置 LTDC 图层帧缓冲地址
 * @param dev 设备实例指针
 * @param layer_idx 图层索引
 * @param address 帧缓冲地址
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_ltdc_set_address(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint32_t address)
{
    ek_assert_param(dev != NULL);

    return dev->ops->set_address(dev, layer_idx, address);
}

/**
 * @brief 设置 LTDC 图层透明度
 * @param dev 设备实例指针
 * @param layer_idx 图层索引
 * @param alpha 透明度值（0-255）
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_ltdc_set_alpha(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint8_t alpha)
{
    ek_assert_param(dev != NULL);

    return dev->ops->set_alpha(dev, layer_idx, alpha);
}

/**
 * @brief 使能 LTDC 图层
 * @param dev 设备实例指针
 * @param layer_idx 图层索引
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_ltdc_enable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx)
{
    ek_assert_param(dev != NULL);

    return dev->ops->enable_layer(dev, layer_idx);
}

/**
 * @brief 禁用 LTDC 图层
 * @param dev 设备实例指针
 * @param layer_idx 图层索引
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_ltdc_disable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx)
{
    ek_assert_param(dev != NULL);

    return dev->ops->disable_layer(dev, layer_idx);
}

/**
 * @brief 重新加载 LTDC 配置
 * @param dev 设备实例指针
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_ltdc_reload_config(ek_hal_ltdc_t *const dev)
{
    ek_assert_param(dev != NULL);

    return dev->ops->reload_config(dev);
}

/**
 * @brief 开启 LTDC 显示输出
 * @param dev 设备实例指针
 */
void ek_hal_ltdc_display_on(ek_hal_ltdc_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->display_on(dev);
}

/**
 * @brief 关闭 LTDC 显示输出
 * @param dev 设备实例指针
 */
void ek_hal_ltdc_display_off(ek_hal_ltdc_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->display_off(dev);
}
