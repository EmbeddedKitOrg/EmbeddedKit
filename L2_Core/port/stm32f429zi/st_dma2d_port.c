#include "../../hal/inc/ek_hal_dma2d.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "dma2d.h"

/* CubeMX 生成的 DMA2D 句柄 */
extern DMA2D_HandleTypeDef hdma2d;

// 硬件信息结构体
typedef struct
{
    DMA2D_HandleTypeDef *hdma2d;
} st_dma2d_info;

/**
 * @brief 将 RGB565 颜色转换为 ARGB8888 格式
 * @note DMA2D 的 COLREG 寄存器（R2M 模式下的颜色源）始终是 ARGB8888 格式
 */
static inline uint32_t rgb565_to_argb8888(uint16_t rgb565)
{
    uint8_t r = (rgb565 >> 11) & 0x1F;
    uint8_t g = (rgb565 >> 5) & 0x3F;
    uint8_t b = rgb565 & 0x1F;

    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);

    return (0xFFu << 24) | (r << 16) | (g << 8) | b;
}

// ops 实现
static void _init(ek_hal_dma2d_t *const dev);
static bool
_fill(ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color);
static bool _convert(ek_hal_dma2d_t *const dev,
                     void *src,
                     void *dst,
                     uint32_t width,
                     uint32_t height,
                     uint32_t offset,
                     ek_dma2d_color_mode_t input_mode);
static bool
_fill_it(ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color);
static bool _convert_it(ek_hal_dma2d_t *const dev,
                        void *src,
                        void *dst,
                        uint32_t width,
                        uint32_t height,
                        uint32_t offset,
                        ek_dma2d_color_mode_t input_mode);

static const ek_dma2d_ops_t st_dma2d_ops = {
    .init = _init,
    .fill = _fill,
    .convert = _convert,
    .fill_it = _fill_it,
    .convert_it = _convert_it,
};

// 硬件信息
static st_dma2d_info dma2d1_info = {
    .hdma2d = &hdma2d,
};

// 设备实例
static ek_hal_dma2d_t drv_dma2d1;

__WEAK void ek_dma2d_transfer_cplt_callback(DMA2D_HandleTypeDef *hdma2d)
{
    __UNUSED(hdma2d);
}

// 注册到 HAL
void st_dma2d_drv_init(void)
{
    ek_hal_dma2d_register(&drv_dma2d1, "DMA2D1", &st_dma2d_ops, &dma2d1_info);
    hdma2d.XferCpltCallback = ek_dma2d_transfer_cplt_callback;
}

EK_EXPORT_HARDWARE(st_dma2d_drv_init);

// 获取 DMA2D 输入颜色模式
static uint32_t _get_dma2d_input_mode(ek_dma2d_color_mode_t input_mode)
{
    switch (input_mode)
    {
        case EK_HAL_DMA2D_ARGB8888:
            return DMA2D_INPUT_ARGB8888;
        case EK_HAL_DMA2D_RGB888:
            return DMA2D_INPUT_RGB888;
        case EK_HAL_DMA2D_RGB565:
            return DMA2D_INPUT_RGB565;
        default:
            return DMA2D_INPUT_RGB565;
    }
}

// 内部函数
static void _init(ek_hal_dma2d_t *const dev)
{
    ek_assert_param(dev != NULL);
    // CubeMX 已完成 DMA2D 硬件初始化
}

static bool
_fill(ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color)
{
    (void)dev;
    HAL_StatusTypeDef status;

    if (HAL_DMA2D_GetState(&hdma2d) != HAL_DMA2D_STATE_READY)
    {
        HAL_DMA2D_Abort(&hdma2d);
    }

    if (hdma2d.Init.Mode != DMA2D_R2M || hdma2d.Init.ColorMode != DMA2D_OUTPUT_RGB565 ||
        hdma2d.Init.OutputOffset != offset)
    {
        hdma2d.Init.Mode = DMA2D_R2M;
        hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
        hdma2d.Init.OutputOffset = offset;

        status = HAL_DMA2D_Init(&hdma2d);
        if (status != HAL_OK) return false;
    }

    status = HAL_DMA2D_Start(&hdma2d, rgb565_to_argb8888((uint16_t)color), (uint32_t)dst, width, height);
    if (status != HAL_OK) return false;

    status = HAL_DMA2D_PollForTransfer(&hdma2d, 1000);
    if (status != HAL_OK) return false;

    return true;
}

static bool _convert(ek_hal_dma2d_t *const dev,
                     void *src,
                     void *dst,
                     uint32_t width,
                     uint32_t height,
                     uint32_t offset,
                     ek_dma2d_color_mode_t input_mode)
{
    (void)dev;
    HAL_StatusTypeDef status;
    uint32_t dma2d_input_mode = _get_dma2d_input_mode(input_mode);

    if (HAL_DMA2D_GetState(&hdma2d) != HAL_DMA2D_STATE_READY)
    {
        HAL_DMA2D_Abort(&hdma2d);
    }

    if (hdma2d.Init.Mode != DMA2D_M2M_PFC || hdma2d.Init.ColorMode != DMA2D_OUTPUT_RGB565 ||
        hdma2d.Init.OutputOffset != offset || hdma2d.LayerCfg[1].InputColorMode != dma2d_input_mode)
    {
        hdma2d.Init.Mode = DMA2D_M2M_PFC;
        hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
        hdma2d.Init.OutputOffset = offset;

        hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[1].InputAlpha = 0xFF;
        hdma2d.LayerCfg[1].InputColorMode = dma2d_input_mode;
        hdma2d.LayerCfg[1].InputOffset = 0;

        status = HAL_DMA2D_Init(&hdma2d);
        if (status != HAL_OK) return false;

        status = HAL_DMA2D_ConfigLayer(&hdma2d, 1);
        if (status != HAL_OK) return false;
    }

    status = HAL_DMA2D_Start(&hdma2d, (uint32_t)src, (uint32_t)dst, width, height);
    if (status != HAL_OK) return false;

    status = HAL_DMA2D_PollForTransfer(&hdma2d, 1000);
    if (status != HAL_OK) return false;

    return true;
}

static bool
_fill_it(ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color)
{
    (void)dev;
    HAL_StatusTypeDef status;

    if (HAL_DMA2D_GetState(&hdma2d) != HAL_DMA2D_STATE_READY)
    {
        return false;
    }

    if (hdma2d.Init.Mode != DMA2D_R2M || hdma2d.Init.ColorMode != DMA2D_OUTPUT_RGB565 ||
        hdma2d.Init.OutputOffset != offset)
    {
        hdma2d.Init.Mode = DMA2D_R2M;
        hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
        hdma2d.Init.OutputOffset = offset;

        status = HAL_DMA2D_Init(&hdma2d);
        if (status != HAL_OK) return false;
    }

    status = HAL_DMA2D_Start_IT(&hdma2d, rgb565_to_argb8888((uint16_t)color), (uint32_t)dst, width, height);
    if (status != HAL_OK) return false;

    return true;
}

static bool _convert_it(ek_hal_dma2d_t *const dev,
                        void *src,
                        void *dst,
                        uint32_t width,
                        uint32_t height,
                        uint32_t offset,
                        ek_dma2d_color_mode_t input_mode)
{
    (void)dev;
    HAL_StatusTypeDef status;
    uint32_t dma2d_input_mode = _get_dma2d_input_mode(input_mode);

    if (HAL_DMA2D_GetState(&hdma2d) != HAL_DMA2D_STATE_READY)
    {
        return false;
    }

    if (hdma2d.Init.Mode != DMA2D_M2M_PFC || hdma2d.Init.ColorMode != DMA2D_OUTPUT_RGB565 ||
        hdma2d.Init.OutputOffset != offset || hdma2d.LayerCfg[1].InputColorMode != dma2d_input_mode)
    {
        hdma2d.Init.Mode = DMA2D_M2M_PFC;
        hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
        hdma2d.Init.OutputOffset = offset;

        hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
        hdma2d.LayerCfg[1].InputAlpha = 0xFF;
        hdma2d.LayerCfg[1].InputColorMode = dma2d_input_mode;
        hdma2d.LayerCfg[1].InputOffset = 0;

        status = HAL_DMA2D_Init(&hdma2d);
        if (status != HAL_OK) return false;

        status = HAL_DMA2D_ConfigLayer(&hdma2d, 1);
        if (status != HAL_OK) return false;
    }

    status = HAL_DMA2D_Start_IT(&hdma2d, (uint32_t)src, (uint32_t)dst, width, height);
    if (status != HAL_OK) return false;

    return true;
}
