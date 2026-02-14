#include "systick.h"

volatile uint32_t uwTick = 0;
volatile uint32_t delayTick = 0;

void SysTick_Init(void)
{
    /* setup systick timer for 1000Hz interrupts */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        /* capture error */
        while (1)
        {
        }
    }
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
}

/**
 * @brief 阻塞指定时间
 *
 * @param xms 阻塞的事件
 */
void Delay(uint32_t xms)
{
    delayTick = xms;

    while (delayTick)
    {
    }
}

uint32_t GetTick(void)
{
    return uwTick;
}

void Tick_Inc(void)
{
    uwTick++;
    if (delayTick)
    {
        delayTick--;
    }
}