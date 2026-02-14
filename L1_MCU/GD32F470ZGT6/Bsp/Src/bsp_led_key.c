#include "bsp_led_key.h"

/**
  * @brief  初始化LED和按键GPIO
  * @details 该函数初始化所有LED和按键相关的GPIO引脚，包括：
  *          - 使能GPIOA、GPIOB、GPIOD、GPIOE、GPIOG时钟
  *          - 配置LED1-LED4为输出模式，下拉电阻
  *          - 配置按键L、A、B、R为输入模式，上拉电阻
  *          - 设置LED输出为推挽输出，50MHz速度
  *          - 初始化所有LED为熄灭状态
  * @param  无
  * @retval 无
  */
void BSP_LED_Key_Init(void)
{
    // 使能GPIO时钟
    rcu_periph_clock_enable(RCU_GPIOA);  /* LED4、Key L */
    rcu_periph_clock_enable(RCU_GPIOB);  /* Key A、Key B */
    rcu_periph_clock_enable(RCU_GPIOD);  /* LED2 */
    rcu_periph_clock_enable(RCU_GPIOE);  /* LED1 */
    rcu_periph_clock_enable(RCU_GPIOG);  /* LED3、Key A */

    // LED GPIO配置
    gpio_mode_set(BSP_LED1_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, BSP_LED1_PIN);
    gpio_mode_set(BSP_LED2_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, BSP_LED2_PIN);
    gpio_mode_set(BSP_LED3_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, BSP_LED3_PIN);
    gpio_mode_set(BSP_LED4_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, BSP_LED4_PIN);

    gpio_output_options_set(BSP_LED1_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_LED1_PIN);
    gpio_output_options_set(BSP_LED2_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_LED2_PIN);
    gpio_output_options_set(BSP_LED3_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_LED3_PIN);
    gpio_output_options_set(BSP_LED4_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, BSP_LED4_PIN);

    // Key GPIO配置
    gpio_mode_set(BSP_KEY_L_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, BSP_KEY_L_PIN);
    gpio_mode_set(BSP_KEY_A_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, BSP_KEY_A_PIN);
    gpio_mode_set(BSP_KEY_B_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, BSP_KEY_B_PIN);
    gpio_mode_set(BSP_KEY_R_PORT, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, BSP_KEY_R_PIN);

    // LED Off
    gpio_bit_reset(BSP_LED1_PORT, BSP_LED1_PIN);
    gpio_bit_reset(BSP_LED2_PORT, BSP_LED2_PIN);
    gpio_bit_reset(BSP_LED3_PORT, BSP_LED3_PIN);
    gpio_bit_reset(BSP_LED4_PORT, BSP_LED4_PIN);
}

/**
  * @brief  读取按键状态
  * @details 该函数读取所有按键的状态并返回按键值。
  *          按键优先级：L > R > A > B
  *          当多个按键同时按下时，返回优先级最高的按键值
  * @param  无
  * @retval KeyVal_t 按键值，枚举类型：
  *          - KEY_VAL_NONE: 无按键按下
  *          - KEY_VAL_L: 左键按下
  *          - KEY_VAL_R: 右键按下
  *          - KEY_VAL_A: A键按下
  *          - KEY_VAL_B: B键按下
  */
KeyVal_t BSP_KeyRead(void)
{
    KeyVal_t temp = KEY_VAL_NONE;
    if (gpio_input_bit_get(BSP_KEY_L_PORT, BSP_KEY_L_PIN) == 1) temp = KEY_VAL_L;
    if (gpio_input_bit_get(BSP_KEY_R_PORT, BSP_KEY_R_PIN) == 1) temp = KEY_VAL_R;
    if (gpio_input_bit_get(BSP_KEY_A_PORT, BSP_KEY_A_PIN) == 1) temp = KEY_VAL_A;
    if (gpio_input_bit_get(BSP_KEY_B_PORT, BSP_KEY_B_PIN) == 1) temp = KEY_VAL_B;

    return temp;
}