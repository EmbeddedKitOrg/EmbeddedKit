/**
 * @file ek_io.c
 * @brief 标准输入输出实现
 * @author N1netyNine99
 */

#include "../inc/ek_io.h"

#if EK_IO_ENABLE == 1

#    include "../inc/ek_def.h"

__WEAK void _ek_io_fputc(int ch)
{
    __UNUSED(ch);
}

static int _ek_io_printf(int ch, lwprintf_t *lwp)
{
    __UNUSED(lwp);
    if (ch != '\0')
    {
        _ek_io_fputc(ch);
    }
    return ch;
}

void ek_io_init(void)
{
    lwprintf_init(_ek_io_printf);
}

#endif /* EK_IO_ENABLE */
