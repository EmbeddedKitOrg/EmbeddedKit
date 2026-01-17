#include <stdio.h>
#include <time.h>
#include "test.h"

EK_LOG_FILE_TAG("main.c");

EK_IO_FPUTC()
{
    fputc(ch, stdout);
}

EK_LOG_GET_TICK()
{
    static uint32_t tick = 0;

    tick++;

    return tick;
}

int main(void)
{
    srand(time(NULL));
    ek_io_init();
    ek_heap_init();

    // ek_assert_full(5 == 0);

    EK_LOG_INFO("tlsf size:%u size_t = (%d) bytes", ek_heap_total_size(), sizeof(size_t));
    EK_LOG_INFO("heap init ok use:%u,unused:%u", ek_heap_used(), ek_heap_unused());
    stack_test();
    ringbuf_test();
    vec_test();

    return 0;
}
