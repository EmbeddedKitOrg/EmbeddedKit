#ifndef EK_HAL_DMA_H
#define EK_HAL_DMA_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_dma_t ek_hal_dma_t;
typedef struct ek_dma_ops_t ek_dma_ops_t;

/** @brief DMA 传输方向枚举 */
typedef enum {
    EK_HAL_DMA_DIR_M2M,  // Memory to Memory
    EK_HAL_DMA_DIR_M2P,  // Memory to Peripheral
    EK_HAL_DMA_DIR_P2M,  // Peripheral to Memory
} ek_dma_direction_t;

/** @brief DMA 操作函数集 */
struct ek_dma_ops_t
{
    void (*init)(ek_hal_dma_t *const dev);
    bool (*transfer)(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir);
    bool (*transfer_it)(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir);
    void (*abort)(ek_hal_dma_t *const dev);
};

/** @brief DMA 设备结构体 */
struct ek_hal_dma_t
{
    ek_list_node_t node;
    const char *name;
    const ek_dma_ops_t *ops;
    void *dev_info;

    bool lock;
};

extern ek_list_node_t ek_hal_dma_head;

void ek_hal_dma_register(ek_hal_dma_t *const dev, const char *name, const ek_dma_ops_t *ops, void *dev_info);
ek_hal_dma_t *ek_hal_dma_find(const char *name);
bool ek_hal_dma_transfer(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir);
bool ek_hal_dma_transfer_it(ek_hal_dma_t *const dev, void *src, void *dst, size_t size, ek_dma_direction_t dir);
void ek_hal_dma_abort(ek_hal_dma_t *const dev);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_DMA_H
