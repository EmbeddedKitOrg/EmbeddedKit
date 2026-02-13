#include "../inc/ek_hal_dma2d.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_dma2d_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 DMA2D 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_dma2d_register(ek_hal_dma2d_t *const dev, const char *name, const ek_dma2d_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_dma2d_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    ek_list_add_tail(&ek_hal_dma2d_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 DMA2D 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_dma2d_t *ek_hal_dma2d_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_dma2d_head)
    {
        ek_hal_dma2d_t *dev = ek_list_container(p, ek_hal_dma2d_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 使用 DMA2D 填充矩形区域（轮询模式）
 * @param dev 设备实例指针
 * @param dst 目标地址
 * @param width 宽度（像素）
 * @param height 高度（像素）
 * @param offset 行偏移（pitch - width）
 * @param color 填充颜色
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_dma2d_fill(
    ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color)
{
    ek_assert_param(dev != NULL);

    return dev->ops->fill(dev, dst, width, height, offset, color);
}

/**
 * @brief 使用 DMA2D 转换颜色格式（轮询模式）
 * @param dev 设备实例指针
 * @param src 源缓冲区
 * @param dst 目标缓冲区
 * @param width 宽度（像素）
 * @param height 高度（像素）
 * @param offset 目标行偏移
 * @param input_mode 输入颜色模式
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_dma2d_convert(ek_hal_dma2d_t *const dev,
                          void *src,
                          void *dst,
                          uint32_t width,
                          uint32_t height,
                          uint32_t offset,
                          ek_dma2d_color_mode_t input_mode)
{
    ek_assert_param(dev != NULL);

    return dev->ops->convert(dev, src, dst, width, height, offset, input_mode);
}

/**
 * @brief 使用 DMA2D 填充矩形区域（中断模式）
 * @param dev 设备实例指针
 * @param dst 目标地址
 * @param width 宽度（像素）
 * @param height 高度（像素）
 * @param offset 行偏移（pitch - width）
 * @param color 填充颜色
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_dma2d_fill_it(
    ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color)
{
    ek_assert_param(dev != NULL);

    return dev->ops->fill_it(dev, dst, width, height, offset, color);
}

/**
 * @brief 使用 DMA2D 转换颜色格式（中断模式）
 * @param dev 设备实例指针
 * @param src 源缓冲区
 * @param dst 目标缓冲区
 * @param width 宽度（像素）
 * @param height 高度（像素）
 * @param offset 目标行偏移
 * @param input_mode 输入颜色模式
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_dma2d_convert_it(ek_hal_dma2d_t *const dev,
                             void *src,
                             void *dst,
                             uint32_t width,
                             uint32_t height,
                             uint32_t offset,
                             ek_dma2d_color_mode_t input_mode)
{
    ek_assert_param(dev != NULL);

    return dev->ops->convert_it(dev, src, dst, width, height, offset, input_mode);
}
