#include "bsp_exmc.h"

/* W9825G6KH-6 关键时序参数（单位：ns，基于数据手册） */
#define SDRAM_TMRD_CYCLES 2U /* 模式寄存器写入间隔 tMRD >= 2 个时钟周期 */
#define SDRAM_TXSR_NS     70U /* 自刷新唤醒时间 tXSR >= 70ns */
#define SDRAM_TRAS_NS     42U /* 行有效保持时间 tRAS >= 42ns */
#define SDRAM_TRFC_NS     66U /* 自动刷新周期 tRFC >= 66ns */
#define SDRAM_TWR_NS      14U /* 写恢复时间 tWR  >= 14ns */
#define SDRAM_TRP_NS      18U /* 行预充电时间 tRP  >= 18ns */
#define SDRAM_TRCD_NS     18U /* 行到列延迟 tRCD >= 18ns */

/* EXMC 时序寄存器允许的最大计数值（对应 16 个时钟周期） */
#define SDRAM_TIMING_MAX_CYCLES 16U

/* 根据 SDCLK 频率把纳秒换算为 EXMC 需要的时钟周期数（向上取整） */
static uint32_t sdram_ns_to_cycles(uint32_t time_ns, uint32_t sdclk_hz)
{
    if ((0U == time_ns) || (0U == sdclk_hz))
    {
        return 1U;
    }

    uint64_t cycles = (uint64_t)time_ns * (uint64_t)sdclk_hz + (1000000000ULL - 1ULL);
    cycles /= 1000000000ULL;

    if (cycles < 1ULL)
    {
        cycles = 1ULL;
    }
    else if (cycles > SDRAM_TIMING_MAX_CYCLES)
    {
        cycles = SDRAM_TIMING_MAX_CYCLES;
    }

    return (uint32_t)cycles;
}

/**
 * @brief  初始化 EXMC 控制器以驱动 W9825G6KH-6 SDRAM。
 * @note   该函数只负责时序寄存器和控制寄存器配置，真正的初始化命令在 SDRAM_Init 中完成。
 */
void BSP_EXMC_Init(void)
{
    // 使能EXMC和相关GPIO时钟
    rcu_periph_clock_enable(BSP_SDRAM_EXMC_RCU); /* EXMC SDRAM控制器 */
    rcu_periph_clock_enable(RCU_GPIOC); /* EXMC SDRAM控制信号 */
    rcu_periph_clock_enable(RCU_GPIOD); /* EXMC SDRAM数据线 */
    rcu_periph_clock_enable(RCU_GPIOE); /* EXMC SDRAM数据线 */
    rcu_periph_clock_enable(RCU_GPIOF); /* EXMC SDRAM地址线 */
    rcu_periph_clock_enable(RCU_GPIOG); /* EXMC SDRAM地址线和控制信号 */

    // EXMC SDRAM GPIO配置
    // 地址线配置
    gpio_af_set(BSP_EXMC_SDRAM_A0_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A0_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A1_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A1_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A2_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A2_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A3_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A3_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A4_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A4_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A5_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A5_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A6_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A6_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A7_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A7_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A8_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A8_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A9_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A9_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A10_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A10_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A11_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A11_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_A12_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_A12_PIN);

    gpio_mode_set(BSP_EXMC_SDRAM_A0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A0_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A1_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A1_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A2_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A2_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A3_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A3_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A4_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A4_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A5_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A5_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A6_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A6_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A7_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A7_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A8_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A8_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A9_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A9_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A10_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A10_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A11_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A11_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_A12_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_A12_PIN);

    gpio_output_options_set(BSP_EXMC_SDRAM_A0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A0_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A1_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A2_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A3_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A4_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A4_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A5_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A5_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A6_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A6_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A7_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A7_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A8_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A8_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A9_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A9_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A10_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A10_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A11_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A11_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_A12_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_A12_PIN);

    // 数据线配置
    gpio_af_set(BSP_EXMC_SDRAM_D0_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D0_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D1_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D1_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D2_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D2_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D3_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D3_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D4_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D4_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D5_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D5_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D6_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D6_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D7_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D7_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D8_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D8_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D9_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D9_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D10_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D10_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D11_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D11_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D12_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D12_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D13_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D13_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D14_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D14_PIN);
    gpio_af_set(BSP_EXMC_SDRAM_D15_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_D15_PIN);

    gpio_mode_set(BSP_EXMC_SDRAM_D0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D0_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D1_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D1_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D2_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D2_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D3_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D3_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D4_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D4_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D5_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D5_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D6_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D6_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D7_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D7_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D8_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D8_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D9_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D9_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D10_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D10_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D11_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D11_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D12_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D12_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D13_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D13_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D14_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D14_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_D15_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_D15_PIN);

    gpio_output_options_set(BSP_EXMC_SDRAM_D0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D0_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D1_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D2_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D3_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D4_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D4_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D5_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D5_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D6_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D6_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D7_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D7_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D8_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D8_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D9_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D9_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D10_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D10_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D11_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D11_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D12_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D12_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D13_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D13_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D14_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D14_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_D15_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_D15_PIN);

    // 控制信号配置
    gpio_af_set(BSP_EXMC_SDRAM_NBL0_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_NBL0_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_NBL0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_NBL0_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_NBL0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_NBL0_PIN);

    gpio_af_set(BSP_EXMC_SDRAM_NBL1_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_NBL1_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_NBL1_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_NBL1_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_NBL1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_NBL1_PIN);

    // 时钟使能
    gpio_af_set(BSP_EXMC_SDRAM_CEK0_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_CEK0_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_CEK0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_CEK0_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_CEK0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_CEK0_PIN);

    // bank 地址
    gpio_af_set(BSP_EXMC_SDRAM_BA0_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_BA0_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_BA0_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_BA0_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_BA0_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_BA0_PIN);

    gpio_af_set(BSP_EXMC_SDRAM_BA1_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_BA1_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_BA1_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_BA1_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_BA1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_BA1_PIN);

    // 时钟信号线
    gpio_af_set(BSP_EXMC_SDRAM_CLK_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_CLK_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_CLK_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_CLK_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_CLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_CLK_PIN);

    // 列地址选通
    gpio_af_set(BSP_EXMC_SDRAM_NCAS_PORT, GPIO_AF_12, BSP_FMSC_SDR_NCAS_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_NCAS_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_FMSC_SDR_NCAS_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_NCAS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_FMSC_SDR_NCAS_PIN);

    // 行地址选通
    gpio_af_set(BSP_EXMC_SDRAM_NRAS_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_NRAS_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_NRAS_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_NRAS_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_NRAS_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_NRAS_PIN);

    // 输出使能
    gpio_af_set(BSP_EXMC_SDRAM_NEO_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_NEO_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_NEO_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_NEO_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_NEO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_NEO_PIN);

    // 写使能
    gpio_af_set(BSP_EXMC_SDRAM_NWE_PORT, GPIO_AF_12, BSP_EXMC_SDRAM_NWE_PIN);
    gpio_mode_set(BSP_EXMC_SDRAM_NWE_PORT, GPIO_MODE_AF, GPIO_PUPD_PULLUP, BSP_EXMC_SDRAM_NWE_PIN);
    gpio_output_options_set(BSP_EXMC_SDRAM_NWE_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_EXMC_SDRAM_NWE_PIN);

    exmc_sdram_timing_parameter_struct sdram_timing_init_struct;
    exmc_sdram_parameter_struct sdram_init_struct;

    /* 当前工程中 EXMC 时钟配置为 SDCLK = HCLK / 2 */
    uint32_t hclk_hz = rcu_clock_freq_get(CK_AHB);
    uint32_t sdclk_hz = hclk_hz / 2U;
    if (0U == sdclk_hz)
    {
        sdclk_hz = 120000000U; /* 兜底使用 120MHz，避免除零导致的配置错误 */
    }

    /*--------------- 步骤1：根据芯片手册计算时序寄存器 ----------------*/
    sdram_timing_init_struct.load_mode_register_delay = SDRAM_TMRD_CYCLES; /* tMRD */
    sdram_timing_init_struct.exit_selfrefresh_delay = sdram_ns_to_cycles(SDRAM_TXSR_NS, sdclk_hz); /* tXSR */
    sdram_timing_init_struct.row_address_select_delay = sdram_ns_to_cycles(SDRAM_TRAS_NS, sdclk_hz); /* tRAS */
    sdram_timing_init_struct.auto_refresh_delay = sdram_ns_to_cycles(SDRAM_TRFC_NS, sdclk_hz); /* tRFC */
    sdram_timing_init_struct.write_recovery_delay = sdram_ns_to_cycles(SDRAM_TWR_NS, sdclk_hz); /* tWR  */
    sdram_timing_init_struct.row_precharge_delay = sdram_ns_to_cycles(SDRAM_TRP_NS, sdclk_hz); /* tRP  */
    sdram_timing_init_struct.row_to_column_delay = sdram_ns_to_cycles(SDRAM_TRCD_NS, sdclk_hz); /* tRCD */

    /*--------------- 步骤2：配置 SDRAM 控制寄存器 -----------------------*/
    sdram_init_struct.sdram_device = BSP_SDRAM_EXMC; /* 仅使用设备 0 */
    sdram_init_struct.column_address_width = EXMC_SDRAM_COW_ADDRESS_9; /* 9 位列地址 */
    sdram_init_struct.row_address_width = EXMC_SDRAM_ROW_ADDRESS_13; /* 13 位行地址 */
    sdram_init_struct.data_width = EXMC_SDRAM_DATABUS_WIDTH_16B; /* 16 位数据总线 */
    sdram_init_struct.internal_bank_number = EXMC_SDRAM_4_INTER_BANK; /* 4 个内部 bank */
    sdram_init_struct.cas_latency = EXMC_CAS_LATENCY_3_SDCLK; /* 使用 CAS = 3 满足 166MHz 要求 */
    sdram_init_struct.write_protection = DISABLE; /* 允许写入 */
    sdram_init_struct.sdclock_config = EXMC_SDCLK_PERIODS_2_HCLK; /* SDCLK = HCLK / 2 */
    sdram_init_struct.burst_read_switch = ENABLE; /* 启用突发读 */
    sdram_init_struct.pipeline_read_delay = EXMC_PIPELINE_DELAY_1_HCLK; /* 1 个 HCLK 延迟 */
    sdram_init_struct.timing = &sdram_timing_init_struct; /* 绑定时序结构体 */

    /*--------------- 步骤3：写入硬件寄存器 -------------------------------*/
    exmc_sdram_init(&sdram_init_struct);
}
