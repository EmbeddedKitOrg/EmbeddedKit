#ifndef EK_HAL_DAC_H
#define EK_HAL_DAC_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

typedef struct ek_hal_dac_t ek_hal_dac_t;
typedef struct ek_dac_ops_t ek_dac_ops_t;

/** @brief DAC 操作函数集 */
struct ek_dac_ops_t
{
    void (*init)(ek_hal_dac_t *const dev);
    bool (*write)(ek_hal_dac_t *const dev, uint32_t value);
    bool (*write_dma)(ek_hal_dac_t *const dev, uint32_t *buffer, size_t size);
    void (*start)(ek_hal_dac_t *const dev);
    void (*stop)(ek_hal_dac_t *const dev);
};

/** @brief DAC 设备结构体 */
struct ek_hal_dac_t
{
    ek_list_node_t node;
    const char *name;
    const ek_dac_ops_t *ops;
    void *dev_info;

    uint32_t sample_rate;
    bool lock;
};

extern ek_list_node_t ek_hal_dac_head;

void ek_hal_dac_register(ek_hal_dac_t *const dev, const char *name, const ek_dac_ops_t *ops, void *dev_info);
ek_hal_dac_t *ek_hal_dac_find(const char *name);
bool ek_hal_dac_write(ek_hal_dac_t *const dev, uint32_t value);
bool ek_hal_dac_write_dma(ek_hal_dac_t *const dev, uint32_t *buffer, size_t size);
void ek_hal_dac_start(ek_hal_dac_t *const dev);
void ek_hal_dac_stop(ek_hal_dac_t *const dev);

#endif // EK_HAL_DAC_H
