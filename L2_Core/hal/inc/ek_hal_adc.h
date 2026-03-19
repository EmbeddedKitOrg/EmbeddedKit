#ifndef EK_HAL_ADC_H
#define EK_HAL_ADC_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_adc_t ek_hal_adc_t;
typedef struct ek_adc_ops_t ek_adc_ops_t;

/** @brief ADC 分辨率枚举 */
typedef enum
{
    EK_HAL_ADC_RES_8B,
    EK_HAL_ADC_RES_10B,
    EK_HAL_ADC_RES_12B,
    EK_HAL_ADC_RES_16B,
} ek_adc_resolution_t;

/** @brief ADC 操作函数集 */
struct ek_adc_ops_t
{
    void (*init)(ek_hal_adc_t *const dev);
    uint32_t (*read)(ek_hal_adc_t *const dev);
    bool (*read_dma)(ek_hal_adc_t *const dev, uint32_t *buffer, size_t size);
    void (*start)(ek_hal_adc_t *const dev);
    void (*stop)(ek_hal_adc_t *const dev);
};

/** @brief ADC 设备结构体 */
struct ek_hal_adc_t
{
    ek_list_node_t node;
    const char *name;
    const ek_adc_ops_t *ops;
    void *dev_info;

    uint32_t sample_rate;
    ek_adc_resolution_t resolution;
    bool lock;
};

extern ek_list_node_t ek_hal_adc_head;

void ek_hal_adc_register(ek_hal_adc_t *const dev, const char *name, const ek_adc_ops_t *ops, void *dev_info);
ek_hal_adc_t *ek_hal_adc_find(const char *name);
uint32_t ek_hal_adc_read(ek_hal_adc_t *const dev);
bool ek_hal_adc_read_dma(ek_hal_adc_t *const dev, uint32_t *buffer, size_t size);
void ek_hal_adc_start(ek_hal_adc_t *const dev);
void ek_hal_adc_stop(ek_hal_adc_t *const dev);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_ADC_H
