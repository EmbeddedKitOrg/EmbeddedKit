#ifndef __TIMER_H
#define __TIMER_H

#include "bsp_io_define.h"

void BSP_Timer_Tick_Init(void);
void BSP_Timer_LCD_Init(void);
void BSP_Timer_Motor_Init(void);
void BSP_Timer_DAC_Init(void);
void BSP_Timer_Debug_Init(void);

#endif /* __TIMER_H */
