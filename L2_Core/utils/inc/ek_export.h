/**
 * @file ek_export.h
 * @brief 函数导出机制
 * @author N1netyNine99
 *
 * 提供基于链接器段的函数自动导出和初始化机制
 *
 * @note 使用前需在链接脚本 (.ld) 中添加以下段定义：
 * @code
 * .ek_export :
 * {
 *     . = ALIGN(4);
 *     _ek_export_fn_start = .;
 *     KEEP(*(SORT(.ek_export_fn*)))
 *     . = ALIGN(4);
 *     _ek_export_fn_end = .;
 * } > flash
 * @endcode
 */

#ifndef EK_EXPORT_H
#define EK_EXPORT_H

#include "../../../ek_conf.h"

#if (EK_EXPORT_ENABLE == 1)

/**
 * @brief 导出初始化函数类型
 */
typedef void (*_ek_export_init_fn_t)(void);

/**
 * @brief 字符串化辅助宏（内部使用）
 */
#    define _EK_STR_HELPER(x) #x

/**
 * @brief 字符串化宏（内部使用）
 */
#    define _EK_STR(x) _EK_STR_HELPER(x)

/**
 * @brief 导出函数宏
 * @param fn 要导出的函数指针
 * @param prio 优先级（数字越小越先执行）
 *
 * @note 导出的函数会在 ek_export_init() 中按优先级顺序自动调用
 *
 * @example
 * void my_init(void)
 * {
 *     // 初始化代码
 * }
 * EK_EXPORT(my_init, 10);  // 优先级 10
 */
#    define EK_EXPORT(fn, prio) \
        const _ek_export_init_fn_t __init_##fn##prio __attribute__((used, section(".ek_export_fn." _EK_STR(prio)))) = fn

#    define EK_EXPORT_HARDWARE(fn)   EK_EXPORT(fn, 0)
#    define EK_EXPORT_COMPONENTS(fn) EK_EXPORT(fn, 1)
#    define EK_EXPORT_APP(fn)        EK_EXPORT(fn, 2)
#    define EK_EXPORT_USER(fn)       EK_EXPORT(fn, 3)

#else

#    define EK_EXPORT(fn, prio)
#    define EK_EXPORT_HARDWARE(fn)
#    define EK_EXPORT_COMPONENTS(fn)
#    define EK_EXPORT_APP(fn)
#    define EK_EXPORT_USER(fn)

#endif /* EK_EXPORT_ENABLE */

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 执行所有导出的初始化函数
 * @note 按优先级顺序调用所有通过 EK_EXPORT 导出的函数
 */
void ek_export_init(void);

#ifdef __cplusplus
}
#endif

#endif /* EK_EXPORT_H */
