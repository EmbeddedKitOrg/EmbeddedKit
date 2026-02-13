#include "../inc/ek_hal_uart.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_uart_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 UART 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_uart_register(ek_hal_uart_t *const dev, const char *name, const ek_uart_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_uart_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    dev->lock = false;
    ek_list_add_tail(&ek_hal_uart_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 UART 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_uart_t *ek_hal_uart_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_uart_head)
    {
        ek_hal_uart_t *dev = ek_list_container(p, ek_hal_uart_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 通过 UART 发送数据（阻塞模式）
 * @param dev 设备实例指针
 * @param txdata 发送数据缓冲区
 * @param size 数据长度
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_uart_write(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    return dev->ops->write(dev, txdata, size);
}

/**
 * @brief 通过 UART 发送数据（DMA 模式）
 * @param dev 设备实例指针
 * @param txdata 发送数据缓冲区
 * @param size 数据长度
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_uart_write_dma(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    return dev->ops->write_dma(dev, txdata, size);
}

/**
 * @brief 启动 UART 接收
 * @param dev 设备实例指针
 */
void ek_hal_uart_read(ek_hal_uart_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->read(dev);
}
