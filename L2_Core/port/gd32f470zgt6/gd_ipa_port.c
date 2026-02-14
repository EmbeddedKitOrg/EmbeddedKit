#include "../../hal/inc/ek_hal_dma2d.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "hal_ipa.h"

// 硬件信息结构体
typedef struct
{
    bool initialized; // IPA 是全局单例，无需存储外设编号
} gd_ipa_info;

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

static const ek_dma2d_ops_t gd_ipa_ops = {
    .init = _init,
    .fill = _fill,
    .convert = _convert,
    .fill_it = _fill_it,
    .convert_it = _convert_it,
};

// 硬件信息
static gd_ipa_info ipa_info = {
    .initialized = false,
};

// 设备实例
static ek_hal_dma2d_t drv_ipa;

// 注册到 HAL
void gd_ipa_drv_init(void)
{
    ek_hal_dma2d_register(&drv_ipa, "IPA", &gd_ipa_ops, &ipa_info);
}

EK_EXPORT_HARDWARE(gd_ipa_drv_init);

// 内部函数

/**
 * @brief 颜色模式转换
 */
static hal_ipa_pixel_format_t _convert_color_mode(ek_dma2d_color_mode_t mode)
{
    switch (mode)
    {
        case EK_HAL_DMA2D_ARGB8888:
            return HAL_IPA_PF_ARGB8888;
        case EK_HAL_DMA2D_RGB888:
            return HAL_IPA_PF_RGB888;
        case EK_HAL_DMA2D_RGB565:
            return HAL_IPA_PF_RGB565;
        default:
            return HAL_IPA_PF_ARGB8888;
    }
}

static void _init(ek_hal_dma2d_t *const dev)
{
    ek_assert_param(dev != NULL);
    gd_ipa_info *info = (gd_ipa_info *)dev->dev_info;
    info->initialized = true;
}

static bool
_fill(ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color)
{
    ek_assert_param(dev != NULL);

    // IPA 是单例硬件，不需要 lock
    hal_ipa_pixel_format_t format = HAL_IPA_PF_ARGB8888; // 默认格式

    return HAL_IPA_Fill(dst, width, height, offset, color, format);
}

static bool _convert(ek_hal_dma2d_t *const dev,
                     void *src,
                     void *dst,
                     uint32_t width,
                     uint32_t height,
                     uint32_t offset,
                     ek_dma2d_color_mode_t input_mode)
{
    ek_assert_param(dev != NULL);

    // 配置前景层
    hal_ipa_layer_config_t fg = {
        .mem_addr = (uint32_t)src,
        .line_offset = offset,
        .format = _convert_color_mode(input_mode),
        .alpha = 255, // 不透明
    };

    // 配置目标层
    hal_ipa_layer_config_t dst_layer = {
        .mem_addr = (uint32_t)dst,
        .line_offset = offset,
        .format = HAL_IPA_PF_ARGB8888, // 默认输出格式
        .alpha = 255,
    };

    return HAL_IPA_Convert(&fg, &dst_layer, width, height);
}

static bool
_fill_it(ek_hal_dma2d_t *const dev, uint32_t *dst, uint32_t width, uint32_t height, uint32_t offset, uint32_t color)
{
    ek_assert_param(dev != NULL);

    // 启用中断
    HAL_IPA_EnableInterrupt();

    // 启动填充（不等待）
    hal_ipa_pixel_format_t format = HAL_IPA_PF_ARGB8888;

    // 配置并启动（不等待完成）
    // 注意：这里简化实现，实际应该在 HAL_IPA_Fill 中添加非阻塞版本
    bool result = HAL_IPA_Fill(dst, width, height, offset, color, format);

    return result;
}

static bool _convert_it(ek_hal_dma2d_t *const dev,
                        void *src,
                        void *dst,
                        uint32_t width,
                        uint32_t height,
                        uint32_t offset,
                        ek_dma2d_color_mode_t input_mode)
{
    ek_assert_param(dev != NULL);

    // 启用中断
    HAL_IPA_EnableInterrupt();

    // 配置前景层
    hal_ipa_layer_config_t fg = {
        .mem_addr = (uint32_t)src,
        .line_offset = offset,
        .format = _convert_color_mode(input_mode),
        .alpha = 255,
    };

    // 配置目标层
    hal_ipa_layer_config_t dst_layer = {
        .mem_addr = (uint32_t)dst,
        .line_offset = offset,
        .format = HAL_IPA_PF_ARGB8888,
        .alpha = 255,
    };

    // 启动转换（不等待）
    // 注意：这里简化实现，实际应该在 HAL_IPA_Convert 中添加非阻塞版本
    bool result = HAL_IPA_Convert(&fg, &dst_layer, width, height);

    return result;
}
