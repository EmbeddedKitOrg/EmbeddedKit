#include "../inc/ek_hal_adc.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_adc_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 ADC 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_adc_register(ek_hal_adc_t *const dev, const char *name, const ek_adc_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_adc_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    dev->lock = false;
    ek_list_add_tail(&ek_hal_adc_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 ADC 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_adc_t *ek_hal_adc_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_adc_head)
    {
        ek_hal_adc_t *dev = ek_list_container(p, ek_hal_adc_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 读取 ADC 值（单次转换）
 * @param dev 设备实例指针
 * @return ADC 转换结果
 */
uint32_t ek_hal_adc_read(ek_hal_adc_t *const dev)
{
    ek_assert_param(dev != NULL);

    return dev->ops->read(dev);
}

/**
 * @brief 通过 DMA 读取 ADC 值
 * @param dev 设备实例指针
 * @param buffer 数据缓冲区
 * @param size 读取数量
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_adc_read_dma(ek_hal_adc_t *const dev, uint32_t *buffer, size_t size)
{
    ek_assert_param(dev != NULL);

    return dev->ops->read_dma(dev, buffer, size);
}

/**
 * @brief 启动 ADC 转换
 * @param dev 设备实例指针
 */
void ek_hal_adc_start(ek_hal_adc_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->start(dev);
}

/**
 * @brief 停止 ADC 转换
 * @param dev 设备实例指针
 */
void ek_hal_adc_stop(ek_hal_adc_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->stop(dev);
}
