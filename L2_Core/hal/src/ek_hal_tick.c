#include "../inc/ek_hal_tick.h"

static uint32_t defualt_get_tick(void);
static void default_delay(uint32_t xticks);

ek_hal_tick_t ek_default_ticker = {
    .idx = 1,
    .ms_per_tick = 1,
    .tick_to_wait = 0,
    .get = defualt_get_tick,
    .delay = default_delay,
};

static uint32_t defualt_get_tick(void)
{
    // 这里放置一个系统返回函数 e.g. HAL_GetTick()
    return 0;
}

static void default_delay(uint32_t xticks)
{
    // 延时等待函数底层
    // e.g. while((HAL_GetTick() - tick) < wait)

    ek_default_ticker.tick_to_wait = ek_default_ticker.get() + xticks;

    while (ek_default_ticker.tick_to_wait >= ek_default_ticker.get());
}
