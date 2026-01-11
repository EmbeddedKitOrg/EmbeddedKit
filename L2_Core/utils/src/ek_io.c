#include "../inc/ek_io.h"

#if EK_IO_ENABLE == 1

#include "../inc/ek_def.h"

__WEAK void __ek_io_fputc(int ch)
{
    __UNUSED(ch);
}

static int __ek_io_printf(int ch, lwprintf_t *lwp)
{
    __UNUSED(lwp);
    if (ch != '\0')
    {
        __ek_io_fputc(ch);
    }
    return ch;
}

void ek_io_init(void)
{
    lwprintf_init(__ek_io_printf);
}

#endif /* EK_IO_ENABLE */ 
