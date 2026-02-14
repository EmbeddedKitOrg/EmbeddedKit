#include "../inc/ek_hal_pwm.h"
#include "../../utils/inc/ek_assert.h"

ek_list_node_t ek_hal_pwm_head;
static bool _ek_init_flag = false;

/**
 * @brief 注册 PWM 设备到 HAL 管理链表
 * @param dev 设备实例指针
 * @param name 设备名称
 * @param ops 操作函数集
 * @param dev_info 驱动私有数据
 */
void ek_hal_pwm_register(ek_hal_pwm_t *const dev, const char *name, const ek_pwm_ops_t *ops, void *dev_info)
{
    ek_assert_param(dev != NULL);
    ek_assert_param(name != NULL);
    ek_assert_param(ops != NULL);

    if (_ek_init_flag == false)
    {
        ek_list_init(&ek_hal_pwm_head);
        _ek_init_flag = true;
    }

    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    dev->lock = false;
    ek_list_add_tail(&ek_hal_pwm_head, &dev->node);

    dev->ops->init(dev);
}

/**
 * @brief 按名称查找已注册的 PWM 设备
 * @param name 设备名称
 * @return 找到返回设备指针，未找到返回 NULL
 */
ek_hal_pwm_t *ek_hal_pwm_find(const char *name)
{
    ek_assert_param(name != NULL);

    ek_list_node_t *p;
    ek_list_iterate(p, &ek_hal_pwm_head)
    {
        ek_hal_pwm_t *dev = ek_list_container(p, ek_hal_pwm_t, node);

        if (strcmp(dev->name, name) == 0)
        {
            return dev;
        }
    }
    return NULL;
}

/**
 * @brief 启动 PWM 输出
 * @param dev 设备实例指针
 */
void ek_hal_pwm_start(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->start(dev);
}

/**
 * @brief 停止 PWM 输出
 * @param dev 设备实例指针
 */
void ek_hal_pwm_stop(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);

    dev->ops->stop(dev);
}

/**
 * @brief 设置 PWM 占空比
 * @param dev 设备实例指针
 * @param duty 占空比 (0-10000, 表示 0.00% - 100.00%)
 */
void ek_hal_pwm_set_duty(ek_hal_pwm_t *const dev, uint32_t duty)
{
    ek_assert_param(dev != NULL);

    dev->ops->set_duty(dev, duty);
}

/**
 * @brief 设置 PWM 频率
 * @param dev 设备实例指针
 * @param freq 频率 (Hz)
 */
void ek_hal_pwm_set_freq(ek_hal_pwm_t *const dev, uint32_t freq)
{
    ek_assert_param(dev != NULL);

    dev->ops->set_freq(dev, freq);
}

/**
 * @brief 获取 PWM 占空比
 * @param dev 设备实例指针
 * @return 占空比 (0-10000)
 */
uint32_t ek_hal_pwm_get_duty(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);

    return dev->ops->get_duty(dev);
}

/**
 * @brief 获取 PWM 频率
 * @param dev 设备实例指针
 * @return 频率 (Hz)
 */
uint32_t ek_hal_pwm_get_freq(ek_hal_pwm_t *const dev)
{
    ek_assert_param(dev != NULL);

    return dev->ops->get_freq(dev);
}
