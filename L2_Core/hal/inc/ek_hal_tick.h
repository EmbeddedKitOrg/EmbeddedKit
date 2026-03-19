#ifndef EK_HAL_TICK_H
#define EK_HAL_TICK_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_tick_t ek_hal_tick_t;
typedef struct ek_tick_ops_t ek_tick_ops_t;

/** @brief Tick 操作函数集 */
struct ek_tick_ops_t
{
    void (*init)(ek_hal_tick_t *const dev);
    uint32_t (*get)(ek_hal_tick_t *const dev);
    void (*delay)(ek_hal_tick_t *const dev, uint32_t xtick);
};

/** @brief Tick 系统节拍设备结构体 */
struct ek_hal_tick_t
{
    ek_list_node_t node;
    const char *name;
    const ek_tick_ops_t *ops;
    void *dev_info;

    uint16_t ms_per_tick;
};

extern ek_list_node_t ek_hal_tick_head;

void ek_hal_tick_register(ek_hal_tick_t *const dev, const char *name, const ek_tick_ops_t *ops, void *dev_info);
ek_hal_tick_t *ek_hal_tick_find(const char *name);
uint32_t ek_hal_tick_get(ek_hal_tick_t *const dev);
void ek_hal_tick_delay(ek_hal_tick_t *const dev, uint32_t xtick);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_TICK_H
