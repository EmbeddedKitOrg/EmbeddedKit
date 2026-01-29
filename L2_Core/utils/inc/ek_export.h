#ifndef EK_EXPORT_H
#define EK_EXPORT_H

#include "../../../ek_conf.h"

#if (EK_EXPORT_ENABLE == 1)

/*
*    @note 使用这个模块请在对应的 .ld 中 SECTOINS 部分添加下列的部分:
*    .ek_export :
*    {
*        . = ALIGN(4);
*        _ek_export_fn_start = .;
*        KEEP(*(SORT(.ek_export_fn*)))
*        . = ALIGN(4);
*        _ek_export_fn_end = .;
*    } > flash
*/

typedef void (*_ek_export_init_fn_t)(void);

#    define _EK_STR_HELPER(x) #x
#    define _EK_STR(x)        _EK_STR_HELPER(x)

#    define EK_EXPORT(fn, prio) \
        const _ek_export_init_fn_t __init_##fn##prio __attribute__((used, section(".ek_export_fn." _EK_STR(prio)))) = fn

void ek_export_init(void);

#else

#    define EK_EXPORT(fn, prio)

#endif /* EK_EXPORT_ENABLE */

#endif /* EK_EXPORT_H */
