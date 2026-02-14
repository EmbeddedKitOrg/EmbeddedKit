#include "../inc/ek_hal_i2c.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_i2c_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 I2C 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_i2c_register(ek_hal_i2c_t *const dev, const char *name, const ek_i2c_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_i2c_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    dev->lock = false;
    ek_list_add_tail(&ek_hal_i2c_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 I2C 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_i2c_t *ek_hal_i2c_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_i2c_head)
    {
        ek_hal_i2c_t *dev = ek_list_container(p, ek_hal_i2c_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 通过 I2C 发送数据
 * @param dev 设备实例指针
 * @param dev_addr 目标设备地址
 * @param txdata 发送数据缓冲区
 * @param size 数据长度
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_i2c_write(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    return dev->ops->write(dev, dev_addr, txdata, size);
}

/**
 * @brief 通过 I2C 接收数据
 * @param dev 设备实例指针
 * @param dev_addr 目标设备地址
 * @param rxdata 接收数据缓冲区
 * @param size 数据长度
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_i2c_read(ek_hal_i2c_t *const dev, uint16_t dev_addr, uint8_t *rxdata, size_t size)
{
    ek_assert_param(dev != NULL);

    return dev->ops->read(dev, dev_addr, rxdata, size);
}

/**
 * @brief 通过 I2C 写入指定寄存器
 * @param dev 设备实例指针
 * @param dev_addr 目标设备地址
 * @param mem_addr 寄存器地址
 * @param mem_size 寄存器地址宽度（8位/16位）
 * @param txdata 发送数据缓冲区
 * @param size 数据长度
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_i2c_mem_write(ek_hal_i2c_t *const dev,
                          uint16_t dev_addr,
                          uint16_t mem_addr,
                          ek_hal_i2c_size_t mem_size,
                          uint8_t *txdata,
                          size_t size)
{
    ek_assert_param(dev != NULL);

    return dev->ops->mem_write(dev, dev_addr, mem_addr, mem_size, txdata, size);
}

/**
 * @brief 通过 I2C 读取指定寄存器
 * @param dev 设备实例指针
 * @param dev_addr 目标设备地址
 * @param mem_addr 寄存器地址
 * @param mem_size 寄存器地址宽度（8位/16位）
 * @param rxdata 接收数据缓冲区
 * @param size 数据长度
 * @return 成功返回 true，失败返回 false
 */
bool ek_hal_i2c_mem_read(ek_hal_i2c_t *const dev,
                         uint16_t dev_addr,
                         uint16_t mem_addr,
                         ek_hal_i2c_size_t mem_size,
                         uint8_t *rxdata,
                         size_t size)
{
    ek_assert_param(dev != NULL);

    return dev->ops->mem_read(dev, dev_addr, mem_addr, mem_size, rxdata, size);
}
