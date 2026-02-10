#include "main.h"

extern void ek_main(void);

int main(void)
{
    // NVIC Group
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    SysTick_Init();

    ek_main();

    while (1)
    {
    }
}

void Error_Handler(void)
{
    while (1)
    {
    }
}
