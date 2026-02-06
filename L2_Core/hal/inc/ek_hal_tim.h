#ifndef EK_HAL_TIMER_H
#define EK_HAL_TIMER_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_tim_base_t ek_hal_tim_base_t;

typedef enum
{
    EK_HAL_TIM_STATE_RUN,
    EK_HAL_TIM_STATE_STP
} ek_hal_tim_state_t;

typedef enum
{
    EK_HAL_TIM_RES_8B,
    EK_HAL_TIM_RES_16B,
    EK_HAL_TIM_RES_32B,
} ek_hal_tim_res_t;

struct ek_hal_tim_base_t
{
    uint8_t idx;
    ek_list_node_t node;

    ek_hal_tim_state_t state;
    ek_hal_tim_res_t res;

    void (*start)(void);
    void (*stop)(void);
    uint32_t (*get)(void);
    void (*set)(uint32_t value);
};

extern ek_list_node_t ek_hal_tim_head;
extern ek_hal_tim_base_t hal_drv_tim1;

#endif // EK_HAL_TIMER_H
