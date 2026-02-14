#include "hal_ipa.h"

// 外部 Tick 函数声明（假设在 BSP 或系统层提供）
extern uint32_t GetTick(void);

/**
 * @brief 将简化的像素格式转换为前景层格式
 */
static uint32_t _convert_to_fg_format(hal_ipa_pixel_format_t format)
{
    switch (format)
    {
        case HAL_IPA_PF_ARGB8888:
            return FOREGROUND_PPF_ARGB8888;
        case HAL_IPA_PF_RGB888:
            return FOREGROUND_PPF_RGB888;
        case HAL_IPA_PF_RGB565:
            return FOREGROUND_PPF_RGB565;
        case HAL_IPA_PF_ARGB1555:
            return FOREGROUND_PPF_ARG1555;
        case HAL_IPA_PF_ARGB4444:
            return FOREGROUND_PPF_ARGB4444;
        default:
            return FOREGROUND_PPF_ARGB8888;
    }
}

/**
 * @brief 将简化的像素格式转换为目标层格式
 */
static ipa_dpf_enum _convert_to_dst_format(hal_ipa_pixel_format_t format)
{
    switch (format)
    {
        case HAL_IPA_PF_ARGB8888:
            return IPA_DPF_ARGB8888;
        case HAL_IPA_PF_RGB888:
            return IPA_DPF_RGB888;
        case HAL_IPA_PF_RGB565:
            return IPA_DPF_RGB565;
        case HAL_IPA_PF_ARGB1555:
            return IPA_DPF_ARGB1555;
        case HAL_IPA_PF_ARGB4444:
            return IPA_DPF_ARGB4444;
        default:
            return IPA_DPF_ARGB8888;
    }
}

/**
 * @brief 纯色填充
 */
bool HAL_IPA_Fill(uint32_t *dst_addr,
                  uint32_t width,
                  uint32_t height,
                  uint32_t line_offset,
                  uint32_t color,
                  hal_ipa_pixel_format_t dst_format)
{
    // 去初始化 IPA
    ipa_deinit();

    // 设置为填充模式
    ipa_pixel_format_convert_mode_set(IPA_FILL_UP_DE);

    // 配置目标层
    ipa_destination_parameter_struct dst_struct;
    ipa_destination_struct_para_init(&dst_struct);

    dst_struct.destination_memaddr = (uint32_t)dst_addr;
    dst_struct.destination_lineoff = line_offset;
    dst_struct.destination_pf = _convert_to_dst_format(dst_format);
    dst_struct.image_width = width;
    dst_struct.image_height = height;

    // 提取颜色分量（ARGB8888 格式）
    dst_struct.destination_preblue = (color >> 0) & 0xFF;
    dst_struct.destination_pregreen = (color >> 8) & 0xFF;
    dst_struct.destination_prered = (color >> 16) & 0xFF;
    dst_struct.destination_prealpha = (color >> 24) & 0xFF;

    ipa_destination_init(&dst_struct);

    // 启动传输
    ipa_transfer_enable();

    // 等待完成
    return HAL_IPA_WaitForTransfer(1000);
}

/**
 * @brief 像素格式转换
 */
bool HAL_IPA_Convert(hal_ipa_layer_config_t *fg, hal_ipa_layer_config_t *dst, uint32_t width, uint32_t height)
{
    if (fg == NULL || dst == NULL)
    {
        return false;
    }

    // 去初始化 IPA
    ipa_deinit();

    // 设置转换模式
    ipa_pixel_format_convert_mode_set(IPA_FGTODE_PF_CONVERT);

    // 配置前景层
    ipa_foreground_parameter_struct fg_struct;
    ipa_foreground_struct_para_init(&fg_struct);

    fg_struct.foreground_memaddr = fg->mem_addr;
    fg_struct.foreground_lineoff = fg->line_offset;
    fg_struct.foreground_pf = _convert_to_fg_format(fg->format);
    fg_struct.foreground_prealpha = fg->alpha;

    // 根据 alpha 值选择算法
    if (fg->alpha < 255)
    {
        fg_struct.foreground_alpha_algorithm = IPA_FG_ALPHA_MODE_2; // 使用预定义 alpha 乘以读取的 alpha
    }
    else
    {
        fg_struct.foreground_alpha_algorithm = IPA_FG_ALPHA_MODE_0; // 无效果
    }

    ipa_foreground_init(&fg_struct);

    // 配置目标层
    ipa_destination_parameter_struct dst_struct;
    ipa_destination_struct_para_init(&dst_struct);

    dst_struct.destination_memaddr = dst->mem_addr;
    dst_struct.destination_lineoff = dst->line_offset;
    dst_struct.destination_pf = _convert_to_dst_format(dst->format);
    dst_struct.image_width = width;
    dst_struct.image_height = height;

    ipa_destination_init(&dst_struct);

    // 启动传输
    ipa_transfer_enable();

    // 等待完成
    return HAL_IPA_WaitForTransfer(1000);
}

/**
 * @brief 等待传输完成
 */
bool HAL_IPA_WaitForTransfer(uint32_t timeout_ms)
{
    uint32_t tickstart = GetTick();

    // 等待传输完成标志
    while (!ipa_flag_get(IPA_FLAG_FTF))
    {
        if ((GetTick() - tickstart) > timeout_ms)
        {
            return false; // 超时
        }
    }

    // 清除标志
    ipa_flag_clear(IPA_FLAG_FTF);
    return true;
}

/**
 * @brief 使能 IPA 中断
 */
void HAL_IPA_EnableInterrupt(void)
{
    ipa_interrupt_enable(IPA_INT_FTF);
}

/**
 * @brief 禁用 IPA 中断
 */
void HAL_IPA_DisableInterrupt(void)
{
    ipa_interrupt_disable(IPA_INT_FTF);
}

/**
 * @brief 获取 IPA 传输完成标志
 */
bool HAL_IPA_GetFlag(void)
{
    return (ipa_flag_get(IPA_FLAG_FTF) == SET);
}

/**
 * @brief 清除 IPA 传输完成标志
 */
void HAL_IPA_ClearFlag(void)
{
    ipa_flag_clear(IPA_FLAG_FTF);
}
