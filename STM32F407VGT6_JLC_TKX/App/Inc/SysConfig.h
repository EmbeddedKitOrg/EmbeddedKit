#ifndef __SysConfig_h
#define __SysConfig_h

/*STD C*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

/*Math*/
#include <math.h>
#include "arm_math.h"

/*EK Component*/
#include "EK_Task.h"
#include "EK_MemPool.h"
#include "EK_List.h"
#include "EK_Queue.h"
#include "EK_Stack.h"

/*Hardware*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/*BSP*/
#include "Serial.h"
#include "Key_LED.h"

/*APP*/
#include "Test.h"
#include "Task_App.h"

#endif
