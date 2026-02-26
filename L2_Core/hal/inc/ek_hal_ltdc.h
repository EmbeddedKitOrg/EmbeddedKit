#ifndef EK_HAL_LTDC_H
#define EK_HAL_LTDC_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_ltdc_t ek_hal_ltdc_t;
typedef struct ek_ltdc_ops_t ek_ltdc_ops_t;

/** @brief LTDC 操作函数集 */
struct ek_ltdc_ops_t
{
    void (*init)(ek_hal_ltdc_t *const dev);
    bool (*set_address)(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint32_t address);
    bool (*set_alpha)(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint8_t alpha);
    bool (*enable_layer)(ek_hal_ltdc_t *const dev, uint32_t layer_idx);
    bool (*disable_layer)(ek_hal_ltdc_t *const dev, uint32_t layer_idx);
    bool (*reload_config)(ek_hal_ltdc_t *const dev);
    void (*display_on)(ek_hal_ltdc_t *const dev);
    void (*display_off)(ek_hal_ltdc_t *const dev);
};

/** @brief LTDC 显示控制器设备结构体 */
struct ek_hal_ltdc_t
{
    ek_list_node_t node;
    const char *name;
    const ek_ltdc_ops_t *ops;
    void *dev_info;
};

extern ek_list_node_t ek_hal_ltdc_head;

void ek_hal_ltdc_register(ek_hal_ltdc_t *const dev, const char *name, const ek_ltdc_ops_t *ops, void *dev_info);
ek_hal_ltdc_t *ek_hal_ltdc_find(const char *name);
bool ek_hal_ltdc_set_address(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint32_t address);
bool ek_hal_ltdc_set_alpha(ek_hal_ltdc_t *const dev, uint32_t layer_idx, uint8_t alpha);
bool ek_hal_ltdc_enable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx);
bool ek_hal_ltdc_disable_layer(ek_hal_ltdc_t *const dev, uint32_t layer_idx);
bool ek_hal_ltdc_reload_config(ek_hal_ltdc_t *const dev);
void ek_hal_ltdc_display_on(ek_hal_ltdc_t *const dev);
void ek_hal_ltdc_display_off(ek_hal_ltdc_t *const dev);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_LTDC_H
