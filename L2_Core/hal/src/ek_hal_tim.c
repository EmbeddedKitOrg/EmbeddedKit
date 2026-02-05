#include "../inc/ek_hal_tim.h"
#include "../../utils/inc/ek_export.h"

ek_list_node_t ek_hal_tim_head;

static void tim1_start(void);
static void tim1_stop(void);
static uint32_t tim1_get(void);
static void tim1_set(uint32_t value);

ek_hal_tim_base_t hal_drv_tim1 = {
    .idx = 1,
    .res = EK_HAL_TIM_RES_32B,
    .state = EK_HAL_TIM_STATE_STP,

    .start = tim1_start,
    .stop = tim1_stop,
    .get = tim1_get,
    .set = tim1_set,
};

static void tim1_start(void)
{
    // 定时器启动的底层
    // e.g. HAL_TIM_Base_Start(&htim1)
}

static void tim1_stop(void)
{
    // 定时器停止的底层
    // e.g. HAL_TIM_Base_Stop(&htim1)
}

static uint32_t tim1_get(void)
{
    // 获取定时器当前计数值的底层
    // e.g. __HAL_TIM_GET_COUNTER(&htim1)
    return 0;
}

static void tim1_set(uint32_t value)
{
    // 设置定时器计数值的底层
    // e.g. __HAL_TIM_SET_COUNTER(&htim1, value)
}

void ek_hal_tim_init(void)
{
    ek_list_init(&ek_hal_tim_head);
    ek_list_add_tail(&ek_hal_tim_head, &hal_drv_tim1.node);
}

EK_EXPORT(ek_hal_tim_init, 0);
