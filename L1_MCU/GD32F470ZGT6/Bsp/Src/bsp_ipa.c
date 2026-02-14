#include "bsp_ipa.h"

/**
 * @brief 初始化 IPA 外设
 * @details 使能 IPA 时钟，配置中断（可选）
 *          - IPA（图像像素加速器）：用于图形加速，支持像素格式转换、Alpha 混合、纯色填充等
 * @param None
 * @retval None
 * @note 必须在使用 IPA 功能之前调用此函数
 */
void BSP_IPA_Init(void)
{
    // 使能 IPA 时钟
    rcu_periph_clock_enable(RCU_IPA);

    // 反初始化 IPA（复位到默认状态）
    ipa_deinit();

    // 如果需要使用中断模式，配置 NVIC
    // nvic_irq_enable(IPA_IRQn, 2, 0);
}
