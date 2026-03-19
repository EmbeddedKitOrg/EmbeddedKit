#ifndef EK_HAL_PWM_H
#define EK_HAL_PWM_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_pwm_t ek_hal_pwm_t;
typedef struct ek_pwm_ops_t ek_pwm_ops_t;

/** @brief PWM 操作函数集 */
struct ek_pwm_ops_t
{
    void (*init)(ek_hal_pwm_t *const dev);
    void (*start)(ek_hal_pwm_t *const dev);
    void (*stop)(ek_hal_pwm_t *const dev);
    void (*set_duty)(ek_hal_pwm_t *const dev, uint32_t duty);
    void (*set_freq)(ek_hal_pwm_t *const dev, uint32_t freq);
    uint32_t (*get_duty)(ek_hal_pwm_t *const dev);
    uint32_t (*get_freq)(ek_hal_pwm_t *const dev);
};

/** @brief PWM 设备结构体 */
struct ek_hal_pwm_t
{
    ek_list_node_t node;
    const char *name;
    const ek_pwm_ops_t *ops;
    void *dev_info;

    uint32_t frequency;  // PWM 频率 (Hz)
    uint32_t duty_cycle; // 占空比 (0-10000, 表示 0.00% - 100.00%)
    bool lock;
};

extern ek_list_node_t ek_hal_pwm_head;

void ek_hal_pwm_register(ek_hal_pwm_t *const dev, const char *name, const ek_pwm_ops_t *ops, void *dev_info);
ek_hal_pwm_t *ek_hal_pwm_find(const char *name);
void ek_hal_pwm_start(ek_hal_pwm_t *const dev);
void ek_hal_pwm_stop(ek_hal_pwm_t *const dev);
void ek_hal_pwm_set_duty(ek_hal_pwm_t *const dev, uint32_t duty);
void ek_hal_pwm_set_freq(ek_hal_pwm_t *const dev, uint32_t freq);
uint32_t ek_hal_pwm_get_duty(ek_hal_pwm_t *const dev);
uint32_t ek_hal_pwm_get_freq(ek_hal_pwm_t *const dev);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_PWM_H
