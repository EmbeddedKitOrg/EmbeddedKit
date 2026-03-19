#ifndef EK_HAL_DMA2D_H
#define EK_HAL_DMA2D_H

#include "../../utils/inc/ek_def.h"
#include "../../utils/inc/ek_list.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct ek_hal_dma2d_t ek_hal_dma2d_t;
typedef struct ek_dma2d_ops_t ek_dma2d_ops_t;

/** @brief DMA2D 颜色模式 */
typedef enum
{
    EK_HAL_DMA2D_ARGB8888 = 0,
    EK_HAL_DMA2D_RGB888 = 1,
    EK_HAL_DMA2D_RGB565 = 2,
} ek_dma2d_color_mode_t;

/** @brief DMA2D 操作函数集 */
struct ek_dma2d_ops_t
{
    void (*init)(ek_hal_dma2d_t *const dev);
    bool (*fill)(
        ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color);
    bool (*convert)(ek_hal_dma2d_t *const dev,
                    void *src,
                    void *dst,
                    uint32_t width,
                    uint32_t height,
                    uint32_t offset,
                    ek_dma2d_color_mode_t input_mode);
    bool (*fill_it)(
        ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color);
    bool (*convert_it)(ek_hal_dma2d_t *const dev,
                       void *src,
                       void *dst,
                       uint32_t width,
                       uint32_t height,
                       uint32_t offset,
                       ek_dma2d_color_mode_t input_mode);
};

/** @brief DMA2D 设备结构体 */
struct ek_hal_dma2d_t
{
    ek_list_node_t node;
    const char *name;
    const ek_dma2d_ops_t *ops;
    void *dev_info;
};

extern ek_list_node_t ek_hal_dma2d_head;

void ek_hal_dma2d_register(ek_hal_dma2d_t *const dev, const char *name, const ek_dma2d_ops_t *ops, void *dev_info);
ek_hal_dma2d_t *ek_hal_dma2d_find(const char *name);
bool ek_hal_dma2d_fill(
    ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color);
bool ek_hal_dma2d_convert(ek_hal_dma2d_t *const dev,
                          void *src,
                          void *dst,
                          uint32_t width,
                          uint32_t height,
                          uint32_t offset,
                          ek_dma2d_color_mode_t input_mode);
bool ek_hal_dma2d_fill_it(
    ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color);
bool ek_hal_dma2d_convert_it(ek_hal_dma2d_t *const dev,
                             void *src,
                             void *dst,
                             uint32_t width,
                             uint32_t height,
                             uint32_t offset,
                             ek_dma2d_color_mode_t input_mode);

#ifdef __cplusplus
}
#endif

#endif // EK_HAL_DMA2D_H
