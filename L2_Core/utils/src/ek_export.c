/**
 * @file ek_export.c
 * @brief 函数导出机制实现
 * @author N1netyNine99
 */

#include "../inc/ek_export.h"

#if (EK_EXPORT_ENABLE == 1)

#    include "../inc/ek_assert.h"

extern _ek_export_init_fn_t _ek_export_fn_start;
extern _ek_export_init_fn_t _ek_export_fn_end;

void ek_export_init(void)
{
    _ek_export_init_fn_t *fn_ptr;

    for (fn_ptr = &_ek_export_fn_start; fn_ptr < &_ek_export_fn_end; fn_ptr++)
    {
        ek_assert_param(*fn_ptr != NULL);

        (*fn_ptr)();
    }
}

#else

void ek_export_init(void)
{
}

#endif /* EK_EXPORT_ENABLE */
