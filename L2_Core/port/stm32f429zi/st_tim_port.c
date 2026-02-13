#include "../../hal/inc/ek_hal_tim.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "tim.h"

// 硬件信息结构体
typedef struct
{
    TIM_HandleTypeDef *htim;
} st_tim_info;

// ops 实现
static void _init(ek_hal_tim_base_t *const dev);
static void _tim_start(ek_hal_tim_base_t *const dev);
static void _tim_stop(ek_hal_tim_base_t *const dev);
static uint32_t _get(ek_hal_tim_base_t *const dev);
static void _set(ek_hal_tim_base_t *const dev, uint32_t value);

static const ek_tim_ops_t st_tim_ops = {
    .init = _init,
    .start = _tim_start,
    .stop = _tim_stop,
    .get = _get,
    .set = _set,
};

// 硬件信息
static st_tim_info rtos_dbg_tim_info = {
    .htim = &htim2,
};

// 设备实例
static ek_hal_tim_base_t drv_rtos_dbg_tim = {
    .res = EK_HAL_TIM_RES_32B,
};

// 注册到 HAL
void st_tim_drv_init(void)
{
    ek_hal_tim_register(&drv_rtos_dbg_tim, "RTOS_DBG_TIM", &st_tim_ops, &rtos_dbg_tim_info);
}

EK_EXPORT_HARDWARE(st_tim_drv_init);

// 内部函数
static void _init(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);
    // CubeMX 已完成 TIM 硬件初始化
}

static void _tim_start(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);

    st_tim_info *info = (st_tim_info *)dev->dev_info;

    HAL_TIM_Base_Start(info->htim);
}

static void _tim_stop(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);

    st_tim_info *info = (st_tim_info *)dev->dev_info;

    HAL_TIM_Base_Stop(info->htim);
}

static uint32_t _get(ek_hal_tim_base_t *const dev)
{
    ek_assert_param(dev != NULL);

    st_tim_info *info = (st_tim_info *)dev->dev_info;

    return __HAL_TIM_GET_COUNTER(info->htim);
}

static void _set(ek_hal_tim_base_t *const dev, uint32_t value)
{
    ek_assert_param(dev != NULL);

    st_tim_info *info = (st_tim_info *)dev->dev_info;

    __HAL_TIM_SET_COUNTER(info->htim, value);
}
