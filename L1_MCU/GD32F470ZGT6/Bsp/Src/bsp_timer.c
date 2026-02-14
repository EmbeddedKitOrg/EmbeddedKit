#include "bsp_timer.h"

/**
  * @brief  初始化BSP定时器，1ms中断间隔
  * @details 该函数配置并使能BSP定时器，用于产生1ms周期性中断。
  *          定时器配置如下：
  *          - 时钟预分频：2倍频 (100MHz)
  *          - 预分频器：100-1
  *          - 周期：1000-1 (1ms间隔)
  *          - 向上计数模式
  *          - 边缘对齐
  * @param  无
  * @retval 无
  */
void BSP_Timer_Tick_Init(void)
{
    // 使能定时器时钟
    rcu_periph_clock_enable(BSP_TIMER_TICK_RCU);
    /*
     APB1、APB2总线的定时器的时钟都为2倍频
     APB1 -> 60MHz
     APB2 -> 120MHz
     所以2倍频最可靠(因为所有的定时器不能超过240MHz)
    */
    rcu_timer_clock_prescaler_config(RCU_TIMER_PSC_MUL2); // 二倍频 60 * 2 = 120 MHz

    timer_deinit(BSP_TIMER_TICK);

    timer_parameter_struct bsp_timer_tick_init_struct;

    bsp_timer_tick_init_struct.counterdirection = TIMER_COUNTER_UP; // 向上计数
    bsp_timer_tick_init_struct.alignedmode = TIMER_COUNTER_EDGE; // 边缘对齐
    bsp_timer_tick_init_struct.prescaler = 120 - 1; // 预分频器 120 分频
    bsp_timer_tick_init_struct.period = 1000 - 1; // 计数器 1000 -> 1ms

    /* 在输入捕获的时候使用  数字滤波器使用的采样频率之间的分频比例 */
    bsp_timer_tick_init_struct.clockdivision = TIMER_CKDIV_DIV1; // 分频因子
    /* 只有高级定时器才有 配置为x，就重复x+1次进入中断 */
    bsp_timer_tick_init_struct.repetitioncounter = 0; // 重复计数器 0-255

    timer_init(BSP_TIMER_TICK, &bsp_timer_tick_init_struct);

    timer_interrupt_flag_clear(BSP_TIMER_TICK, TIMER_INT_FLAG_UP);

    timer_interrupt_enable(BSP_TIMER_TICK, TIMER_INT_UP);

    nvic_irq_enable(BSP_TIMER_TICK_IRQ, 0x00UL, 0x00UL);

    // 启动定时器
    timer_enable(BSP_TIMER_TICK);
}

/**
  * @brief  初始化LCD背光PWM控制定时器
  * @details 该函数配置并使能定时器2用于PWM输出，控制LCD背光亮度。
  *          定时器配置如下：
  *          - 时钟源：APB1总线，经2倍频后为120MHz
  *          - 预分频器：12000-1分频，得到10kHz计数频率
  *          - 周期：100-1，即100个计数周期，产生100Hz PWM频率
  *          - PWM模式：PWM模式0（向上计数，占空比=脉冲值/周期）
  *          - 输出极性：高电平有效
  *          - GPIO配置：PA7复用为定时器2通道1功能
  *
  *          PWM参数计算：
  *          - 系统时钟：120MHz（APB1×2）
  *          - PWM频率：10kHz / 100 = 100Hz
  *          - 占空比范围：0-100（对应LCD_SetBlk函数的参数）
  *
  * @param  无
  * @note   该函数初始化的PWM输出由LCD_SetBlk()函数使用
  * @note   PWM输出引脚为PA7（定时器2通道1）
  * @note   初始PWM占空比为0，即背光默认关闭
  * @retval 无
  */
void BSP_Timer_LCD_Init(void)
{
    // 使能定时器和GPIO时钟
    rcu_periph_clock_enable(BSP_PWM_LCD_TIMER_RCU);
    rcu_periph_clock_enable(RCU_GPIOA);

    // 配置PWM输出引脚
    gpio_mode_set(BSP_PWM_LCD_BLK_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_PWM_LCD_BLK_PIN);
    gpio_output_options_set(BSP_PWM_LCD_BLK_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_PWM_LCD_BLK_PIN);
    gpio_af_set(BSP_PWM_LCD_BLK_PORT, GPIO_AF_2, BSP_PWM_LCD_BLK_PIN);

    // timer2 cfg -> APB1总线 二倍频 120M
    timer_deinit(BSP_PWM_LCD_TIMER);
    timer_parameter_struct bsp_pwm_lcd_timer_base_init_struct;
    timer_struct_para_init(&bsp_pwm_lcd_timer_base_init_struct);
    bsp_pwm_lcd_timer_base_init_struct.prescaler = 12000 - 1; // -> 1M
    bsp_pwm_lcd_timer_base_init_struct.period = 100 - 1; // 100Hz
    bsp_pwm_lcd_timer_base_init_struct.alignedmode = TIMER_COUNTER_EDGE;
    bsp_pwm_lcd_timer_base_init_struct.counterdirection = TIMER_COUNTER_UP; // 向上计数

    timer_init(BSP_PWM_LCD_TIMER, &bsp_pwm_lcd_timer_base_init_struct);

    // oc cfg
    timer_oc_parameter_struct bsp_pwm_lcd_timer_oc_init_struct;
    timer_channel_output_struct_para_init(&bsp_pwm_lcd_timer_oc_init_struct);
    bsp_pwm_lcd_timer_oc_init_struct.ocpolarity = TIMER_OC_POLARITY_HIGH;
    bsp_pwm_lcd_timer_oc_init_struct.outputstate = TIMER_CCX_ENABLE;
    timer_channel_output_config(BSP_PWM_LCD_TIMER, BSP_PWM_LCD_BLK_CH, &bsp_pwm_lcd_timer_oc_init_struct);

    // 配置定时器通道输出脉冲值
    timer_channel_output_pulse_value_config(BSP_PWM_LCD_TIMER, BSP_PWM_LCD_BLK_CH, 0);

    // 配置定时器通道输出比较模式
    timer_channel_output_mode_config(BSP_PWM_LCD_TIMER, BSP_PWM_LCD_BLK_CH, TIMER_OC_MODE_PWM0);

    // 配置定时器通道输出影子寄存器
    timer_channel_output_shadow_config(BSP_PWM_LCD_TIMER, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(BSP_PWM_LCD_TIMER);

    timer_enable(BSP_PWM_LCD_TIMER);
}

/**
 * @brief 初始化电机PWM定时器
 * @details 配置TIMER1作为电机驱动PWM信号的定时器。
 *          - 时钟源：APB1总线，120MHz经过2倍频
 *          - 定时器频率：1MHz (120MHz/120)
 *          - PWM频率：100Hz (1MHz/1000)
 *          - 输出通道：TIMER1通道2(PA2引脚)
 *          - PWM模式：边沿对齐PWM模式0
 *          - 初始占空比：0% (电机停止)
 *
 * @note 使用PA2引脚(TIMER1_CH2)输出PWM信号控制电机
 * @note PWM占空比范围：0-1000，对应0%-100%
 * @note 定时器使能自动重装载预装载
 * @note 影子寄存器功能已禁用，立即更新占空比
 * @see Motor_SetDuty() 设置电机PWM占空比函数
 */
void BSP_Timer_Motor_Init(void)
{
    // 使能定时器和GPIO时钟
    rcu_periph_clock_enable(BSP_PWM_MOTOR_TIMER_RCU);
    rcu_periph_clock_enable(RCU_GPIOA);

    // 配置PWM输出引脚
    gpio_mode_set(BSP_PWM_MOTOR_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, BSP_PWM_MOTOR_PIN);
    gpio_output_options_set(BSP_PWM_MOTOR_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_PWM_MOTOR_PIN);
    gpio_af_set(BSP_PWM_MOTOR_PORT, GPIO_AF_1, BSP_PWM_MOTOR_PIN);

    // timer1 cfg -> APB1总线 二倍频 120M
    timer_deinit(BSP_PWM_MOTOR_TIMER);
    timer_parameter_struct bsp_pwm_motor_timer_base_init_struct;
    timer_struct_para_init(&bsp_pwm_motor_timer_base_init_struct);
    bsp_pwm_motor_timer_base_init_struct.prescaler = 120 - 1; // -> 1M
    bsp_pwm_motor_timer_base_init_struct.period = 1000 - 1; // 100Hz
    bsp_pwm_motor_timer_base_init_struct.alignedmode = TIMER_COUNTER_EDGE;
    bsp_pwm_motor_timer_base_init_struct.counterdirection = TIMER_COUNTER_UP; // 向上计数
    timer_init(BSP_PWM_MOTOR_TIMER, &bsp_pwm_motor_timer_base_init_struct);

    // oc cfg
    timer_oc_parameter_struct bsp_pwm_motor_timer_oc_init_struct;
    timer_channel_output_struct_para_init(&bsp_pwm_motor_timer_oc_init_struct);
    bsp_pwm_motor_timer_oc_init_struct.ocpolarity = TIMER_OC_POLARITY_HIGH;
    bsp_pwm_motor_timer_oc_init_struct.outputstate = TIMER_CCX_ENABLE;
    timer_channel_output_config(BSP_PWM_MOTOR_TIMER, BSP_PWM_MOTOR_CH, &bsp_pwm_motor_timer_oc_init_struct);

    // 配置定时器通道输出脉冲值
    timer_channel_output_pulse_value_config(BSP_PWM_MOTOR_TIMER, BSP_PWM_MOTOR_CH, 0);

    // 配置定时器通道输出比较模式
    timer_channel_output_mode_config(BSP_PWM_MOTOR_TIMER, BSP_PWM_MOTOR_CH, TIMER_OC_MODE_PWM0);

    // 配置定时器通道输出影子寄存器
    timer_channel_output_shadow_config(BSP_PWM_MOTOR_TIMER, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);

    /* auto-reload preload enable */
    timer_auto_reload_shadow_enable(BSP_PWM_MOTOR_TIMER);

    timer_enable(BSP_PWM_MOTOR_TIMER);
}

/**
 * @brief  初始化DAC音频定时器
 * @note   配置TIMER7作为DAC的触发源
 *         定时器周期参数将在audio.c中根据实际采样率需求进行设置
 * @param  无
 * @retval 无
 */
void BSP_Timer_DAC_Init(void)
{
    // 使能音频定时器时钟
    rcu_periph_clock_enable(BSP_AUDIO_TIMER_RCU);

    // timer7 cfg -> APB2总线 二倍频 240M
    timer_deinit(BSP_AUDIO_TIMER);
    timer_parameter_struct bsp_dac_timer_base_init_struct;
    timer_struct_para_init(&bsp_dac_timer_base_init_struct);
    bsp_dac_timer_base_init_struct.alignedmode = TIMER_COUNTER_EDGE;
    bsp_dac_timer_base_init_struct.counterdirection = TIMER_COUNTER_UP;
    bsp_dac_timer_base_init_struct.clockdivision = TIMER_CKDIV_DIV1;

    bsp_dac_timer_base_init_struct.prescaler = 2 - 1;
    bsp_dac_timer_base_init_struct.period = 5442 - 1;
    // -> 22050 HZ

    timer_init(BSP_AUDIO_TIMER, &bsp_dac_timer_base_init_struct);

    // 开启影子寄存器
    timer_auto_reload_shadow_enable(BSP_AUDIO_TIMER);

    // 开启定时器溢出事件
    timer_update_event_enable(BSP_AUDIO_TIMER);

    // 触发事件输出
    timer_master_output_trigger_source_select(BSP_AUDIO_TIMER, TIMER_TRI_OUT_SRC_UPDATE);
    timer_master_slave_mode_config(BSP_AUDIO_TIMER, TIMER_MASTER_SLAVE_MODE_ENABLE);
}

/**
 * @brief  初始化Debug使用的定时器
 * @note   配置TIMER4
 *         由FreeRTOS来使用
 * @param  无
 * @retval 无
 */
void BSP_Timer_Debug_Init(void)
{
    rcu_periph_clock_enable(BSP_DEBUG_TIMER_RCU);

    // timer4： APB1 -> 60 * 2 = 120M(32 bit cnt)
    timer_deinit(BSP_DEBUG_TIMER);
    timer_parameter_struct bsp_debug_timer_base_init_struct;
    timer_struct_para_init(&bsp_debug_timer_base_init_struct);
    bsp_debug_timer_base_init_struct.alignedmode = TIMER_COUNTER_EDGE;
    bsp_debug_timer_base_init_struct.counterdirection = TIMER_COUNTER_UP;
    bsp_debug_timer_base_init_struct.clockdivision = TIMER_CKDIV_DIV1;

    bsp_debug_timer_base_init_struct.prescaler = 120 - 1; // -> 1M
    bsp_debug_timer_base_init_struct.period = 0xFFFFFFFF; //最大计数值

    timer_init(BSP_DEBUG_TIMER, &bsp_debug_timer_base_init_struct);

    timer_enable(BSP_DEBUG_TIMER);
}