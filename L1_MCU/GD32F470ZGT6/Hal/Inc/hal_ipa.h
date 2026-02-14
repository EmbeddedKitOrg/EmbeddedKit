#ifndef HAL_IPA_H
#define HAL_IPA_H

#include <stdint.h>
#include <stdbool.h>
#include "gd32f4xx_ipa.h"

/**
 * @brief IPA 像素格式枚举
 */
typedef enum {
    HAL_IPA_PF_ARGB8888 = 0,    /*!< ARGB8888 格式 */
    HAL_IPA_PF_RGB888   = 1,    /*!< RGB888 格式 */
    HAL_IPA_PF_RGB565   = 2,    /*!< RGB565 格式 */
    HAL_IPA_PF_ARGB1555 = 3,    /*!< ARGB1555 格式 */
    HAL_IPA_PF_ARGB4444 = 4,    /*!< ARGB4444 格式 */
} hal_ipa_pixel_format_t;

/**
 * @brief IPA 层配置结构体
 */
typedef struct {
    uint32_t mem_addr;                  /*!< 内存地址 */
    uint32_t line_offset;               /*!< 行偏移（pitch - width） */
    hal_ipa_pixel_format_t format;      /*!< 像素格式 */
    uint8_t alpha;                      /*!< Alpha 值 (0-255) */
} hal_ipa_layer_config_t;

/**
 * @brief 纯色填充
 * @param dst_addr 目标地址
 * @param width 宽度（像素）
 * @param height 高度（像素）
 * @param line_offset 行偏移（pitch - width）
 * @param color 填充颜色（ARGB8888 格式）
 * @param dst_format 目标像素格式
 * @return 成功返回 true，失败返回 false
 */
bool HAL_IPA_Fill(
    uint32_t *dst_addr,
    uint32_t width,
    uint32_t height,
    uint32_t line_offset,
    uint32_t color,
    hal_ipa_pixel_format_t dst_format
);

/**
 * @brief 像素格式转换
 * @param fg 前景层配置
 * @param dst 目标层配置
 * @param width 宽度（像素）
 * @param height 高度（像素）
 * @return 成功返回 true，失败返回 false
 */
bool HAL_IPA_Convert(
    hal_ipa_layer_config_t *fg,
    hal_ipa_layer_config_t *dst,
    uint32_t width,
    uint32_t height
);

/**
 * @brief 等待传输完成
 * @param timeout_ms 超时时间（毫秒）
 * @return 成功返回 true，超时返回 false
 */
bool HAL_IPA_WaitForTransfer(uint32_t timeout_ms);

/**
 * @brief 使能 IPA 中断
 */
void HAL_IPA_EnableInterrupt(void);

/**
 * @brief 禁用 IPA 中断
 */
void HAL_IPA_DisableInterrupt(void);

/**
 * @brief 获取 IPA 传输完成标志
 * @return 传输完成返回 true，否则返回 false
 */
bool HAL_IPA_GetFlag(void);

/**
 * @brief 清除 IPA 传输完成标志
 */
void HAL_IPA_ClearFlag(void);

#endif // HAL_IPA_H
