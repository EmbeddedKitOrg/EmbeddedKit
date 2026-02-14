#include "../inc/ek_hal_dma.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_dma_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 DMA 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_dma_register(ek_hal_dma_t *const dev, const char *name, const ek_dma_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_dma_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    dev->lock = false;
    ek_list_add_tail(&ek_hal_dma_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 DMA 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_dma_t *ek_hal_dma_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_dma_head)
    {
        ek_hal_dma_t *dev = ek_list_container(p, ek_hal_dma_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief DMA 传输（阻塞模式）
 * @param dev 设备实例指针
 * @param src 源地址
 * @param dst 目标地址
 * @param size 传输大小
 * @param dir 传输方向
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_dma_transfer(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir)
{
    ek_assert_param(dev != NULL);

    return dev->ops->transfer(dev, src, dst, size, dir);
}

/**
 * @brief DMA 传输（中断模式）
 * @param dev 设备实例指针
 * @param src 源地址
 * @param dst 目标地址
 * @param size 传输大小
 * @param dir 传输方向
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_dma_transfer_it(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir)
{
    ek_assert_param(dev != NULL);

    return dev->ops->transfer_it(dev, src, dst, size, dir);
}

/**
 * @brief 中止 DMA 传输
 * @param dev 设备实例指针
 */
void ek_hal_dma_abort(ek_hal_dma_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->abort(dev);
}
