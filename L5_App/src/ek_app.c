#include "ek_def.h"
#include "ek_mem.h"
#include "ek_io.h"
#include "ek_log.h"
#include "ek_hal_tick.h"

EK_LOG_FILE_TAG("ek_app.c");

EK_IO_FPUTC()
{
    // 在这里放置传输一个字符的函数
    // e.g. fputc(ch, stdout);
}

EK_LOG_GET_TICK()
{
    return ek_default_ticker.get();
}

void ek_main(void)
{
    ek_heap_init();
    ek_io_init();

    while (1)
    {
    }
}
