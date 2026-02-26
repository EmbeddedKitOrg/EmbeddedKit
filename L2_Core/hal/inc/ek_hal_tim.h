#ifndef EK_HAL_TIMER_H
#define EK_HAL_TIMER_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_tim_base_t ek_hal_tim_base_t;
typedef struct ek_tim_ops_t ek_tim_ops_t;

/** @brief 定时器运行状态 */
typedef enum
{
    EK_HAL_TIM_STATE_RUN,
    EK_HAL_TIM_STATE_STP
} ek_tim_state_t;

/** @brief 定时器计数器位宽 */
typedef enum
{
    EK_HAL_TIM_RES_8B,
    EK_HAL_TIM_RES_16B,
    EK_HAL_TIM_RES_32B,
} ek_tim_res_t;

/** @brief 定时器操作函数集 */
struct ek_tim_ops_t
{
    void (*init)(ek_hal_tim_base_t *const dev);
    void (*start)(ek_hal_tim_base_t *const dev);
    void (*stop)(ek_hal_tim_base_t *const dev);
    uint32_t (*get)(ek_hal_tim_base_t *const dev);
    void (*set)(ek_hal_tim_base_t *const dev, uint32_t value);
};

/** @brief 定时器设备结构体 */
struct ek_hal_tim_base_t
{
    ek_list_node_t node;
    const char *name;
    const ek_tim_ops_t *ops;
    void *dev_info;

    ek_tim_state_t state;
    ek_tim_res_t res;
};

extern ek_list_node_t ek_hal_tim_head;

void ek_hal_tim_register(ek_hal_tim_base_t *const dev, const char *name, const ek_tim_ops_t *ops, void *dev_info);
ek_hal_tim_base_t *ek_hal_tim_find(const char *name);
void ek_hal_tim_start(ek_hal_tim_base_t *const dev);
void ek_hal_tim_stop(ek_hal_tim_base_t *const dev);
uint32_t ek_hal_tim_get(ek_hal_tim_base_t *const dev);
void ek_hal_tim_set(ek_hal_tim_base_t *const dev, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_TIMER_H
