#ifndef __MAIN_H
#define __MAIN_H

#include "gd32f4xx.h"
#include "systick.h"
#include "gd32f4xx_libopt.h"

/* _assert */
#define _assert(X) while ((X) == 0)
#define MAX_DELAY  UINT32_MAX

void Error_Handler(void);

#endif /* __MAIN_H */
