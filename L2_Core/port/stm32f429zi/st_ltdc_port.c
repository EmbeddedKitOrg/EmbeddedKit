#include "../../hal/inc/ek_hal_ltdc.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "ltdc.h"

// ops 实现
static void _init(ek_hal_ltdc_t *const dev);
static bool _set_address(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint32_t address);
static bool _set_alpha(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint8_t alpha);
static bool _enable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx);
static bool _disable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx);
static bool _reload_config(ek_hal_ltdc_t *const dev);
static void _display_on(ek_hal_ltdc_t *const dev);
static void _display_off(ek_hal_ltdc_t *const dev);

static const ek_ltdc_ops_t st_ltdc_ops = {
    .init = _init,
    .set_address = _set_address,
    .set_alpha = _set_alpha,
    .enable_layer = _enable_layer,
    .disable_layer = _disable_layer,
    .reload_config = _reload_config,
    .display_on = _display_on,
    .display_off = _display_off,
};

// 设备实例
static ek_hal_ltdc_t drv_ltdc1;

// 注册到 HAL
void st_ltdc_drv_init(void)
{
    ek_hal_ltdc_register(&drv_ltdc1, "LTDC1", &st_ltdc_ops, NULL);
}

EK_EXPORT_HARDWARE(st_ltdc_drv_init);

// 内部函数
static void _init(ek_hal_ltdc_t *const dev)
{
    ek_assert_param(dev != NULL);
    // LTDC 硬件已由 CubeMX 初始化
}

static bool _set_address(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint32_t address)
{
    (void)dev;
    HAL_LTDC_SetAddress(&hltdc, address, layer_idx);
    return true;
}

static bool _set_alpha(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint8_t alpha)
{
    (void)dev;
    HAL_LTDC_SetAlpha(&hltdc, alpha, layer_idx);
    return true;
}

static bool _enable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx)
{
    (void)dev;
    __HAL_LTDC_LAYER_ENABLE(&hltdc, layer_idx);
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
    return true;
}

static bool _disable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx)
{
    (void)dev;
    __HAL_LTDC_LAYER_DISABLE(&hltdc, layer_idx);
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
    return true;
}

static bool _reload_config(ek_hal_ltdc_t *const dev)
{
    (void)dev;
    __HAL_LTDC_RELOAD_CONFIG(&hltdc);
    return true;
}

static void _display_on(ek_hal_ltdc_t *const dev)
{
    (void)dev;
    __HAL_LTDC_ENABLE(&hltdc);
}

static void _display_off(ek_hal_ltdc_t *const dev)
{
    (void)dev;
    __HAL_LTDC_DISABLE(&hltdc);
}
