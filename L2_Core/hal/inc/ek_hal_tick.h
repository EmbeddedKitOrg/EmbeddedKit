#ifndef EK_HAL_TICK_H
#define EK_HAL_TICK_H

#include "../../utils/inc/ek_def.h"

typedef struct ek_hal_tick_t ek_hal_tick_t;

struct ek_hal_tick_t
{
    uint8_t idx;
    uint16_t ms_per_tick;
    uint32_t tick_to_wait;

    uint32_t (*get)(void);
    void (*delay)(uint32_t xtick);
};

extern ek_hal_tick_t ek_default_ticker;

#endif // EK_HAL_TICK_H
