/**
 * \file            lwprintf.h
 * \brief           轻量级标准输入输出管理器
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwPRINTF - Lightweight stdio manager library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.1.0
 */
#ifndef LWPRINTF_HDR_H
#define LWPRINTF_HDR_H

#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "lwprintf_opt.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * \defgroup        LWPRINTF 轻量级标准输入输出管理器
 * \brief           轻量级标准输入输出管理器
 * \{
 */

/**
 * \brief           未使用变量宏
 * \param[in]       x: 未使用的变量
 */
#define LWPRINTF_UNUSED(x) ((void)(x))

/**
 * \brief           计算静态分配数组的大小
 * \param[in]       x: 输入数组
 * \return          数组元素的数量
 */
#define LWPRINTF_ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

/**
 * \brief           LwPRINTF 实例的前向声明
 */
struct lwprintf_s;

/**
 * \brief           字符输出的回调函数
 * \param[in]       ch: 要打印的字符
 * \param[in]       lwobj: LwPRINTF 实例
 * \return          成功时返回 `ch`，返回 `0` 则终止后续字符串处理
 */
typedef int (*lwprintf_output_fn)(int ch, struct lwprintf_s *lwobj);

/**
 * \brief           LwPRINTF 实例
 */
typedef struct lwprintf_s
{
    lwprintf_output_fn out_fn; /*!< 用于直接打印操作的输出函数 */
    void *arg; /*!< 自定义用户参数 */
#if LWPRINTF_CFG_OS || __DOXYGEN__
    LWPRINTF_CFG_OS_MUTEX_HANDLE mutex; /*!< 操作系统互斥锁句柄 */
#endif /* LWPRINTF_CFG_OS || __DOXYGEN__ */
} lwprintf_t;

uint8_t lwprintf_init_ex(lwprintf_t *lwobj, lwprintf_output_fn out_fn);
int lwprintf_vprintf_ex(lwprintf_t *const lwobj, const char *format, va_list arg);
int lwprintf_printf_ex(lwprintf_t *const lwobj, const char *format, ...);
int lwprintf_vsnprintf_ex(lwprintf_t *const lwobj, char *s, size_t n, const char *format, va_list arg);
int lwprintf_snprintf_ex(lwprintf_t *const lwobj, char *s, size_t n, const char *format, ...);
uint8_t lwprintf_protect_ex(lwprintf_t *const lwobj);
uint8_t lwprintf_unprotect_ex(lwprintf_t *const lwobj);

/* Argument management */
#define lwprintf_set_arg(lwobj, argval) (lwobj)->arg = (argval)
#define lwprintf_get_arg(lwobj)         ((lwobj)->arg)

/**
 * \brief           将可变参数列表中的格式化数据写入指定大小的缓冲区
 * \param[in,out]   lwobj: LwPRINTF 实例，设置为 `NULL` 则使用默认实例
 * \param[in]       s: 指向存储结果 C 字符串的缓冲区的指针
 *                      缓冲区大小应至少为 `n` 个字符
 * \param[in]       format: C 字符串，包含格式字符串，遵循与 printf 中 format 相同的规范
 * \param[in]       ...: 格式字符串的可选参数
 * \return          本应写入的字符数量，不包括终止空字符
 */
#define lwprintf_sprintf_ex(lwobj, s, format, ...) lwprintf_snprintf_ex((lwobj), (s), SIZE_MAX, (format), ##__VA_ARGS__)

/**
 * \brief           初始化默认的 LwPRINTF 实例
 * \param[in]       out_fn: 用于打印操作的输出函数
 * \return          成功时返回 `1`，否则返回 `0`
 * \sa              lwprintf_init_ex
 */
#define lwprintf_init(out_fn) lwprintf_init_ex(NULL, (out_fn))

/**
 * \brief           使用默认 LwPRINTF 实例将可变参数列表中的格式化数据打印到输出
 * \param[in]       format: C 字符串，包含要写入输出的文本
 * \param[in]       arg: 标识使用 `va_start` 初始化的可变参数列表的值
 *                      `va_list` 是在 `<cstdarg>` 中定义的特殊类型
 * \return          如果 `n` 足够大，本应写入的字符数量，不包括终止空字符
 */
#define lwprintf_vprintf(format, arg) lwprintf_vprintf_ex(NULL, (format), (arg))

/**
 * \brief           使用默认 LwPRINTF 实例将格式化数据打印到输出
 * \param[in]       format: C 字符串，包含要写入输出的文本
 * \param[in]       ...: 格式字符串的可选参数
 * \return          如果 `n` 足够大，本应写入的字符数量，不包括终止空字符
 */
#define lwprintf_printf(format, ...) lwprintf_printf_ex(NULL, (format), ##__VA_ARGS__)

/**
 * \brief           使用默认 LwPRINTF 实例将可变参数列表中的格式化数据写入指定大小的缓冲区
 * \param[in]       s: 指向存储结果 C 字符串的缓冲区的指针
 *                      缓冲区大小应至少为 `n` 个字符
 * \param[in]       n: 缓冲区中使用的最大字节数
 *                      生成的字符串长度最多为 `n - 1`，为额外的终止空字符留出空间
 * \param[in]       format: C 字符串，包含格式字符串，遵循与 printf 中 format 相同的规范
 * \param[in]       arg: 标识使用 `va_start` 初始化的可变参数列表的值
 *                      `va_list` 是在 `<cstdarg>` 中定义的特殊类型
 * \return          如果 `n` 足够大，本应写入的字符数量，不包括终止空字符
 */
#define lwprintf_vsnprintf(s, n, format, arg) lwprintf_vsnprintf_ex(NULL, (s), (n), (format), (arg))

/**
 * \brief           使用默认 LwPRINTF 实例将可变参数列表中的格式化数据写入指定大小的缓冲区
 * \param[in]       s: 指向存储结果 C 字符串的缓冲区的指针
 *                      缓冲区大小应至少为 `n` 个字符
 * \param[in]       n: 缓冲区中使用的最大字节数
 *                      生成的字符串长度最多为 `n - 1`，为额外的终止空字符留出空间
 * \param[in]       format: C 字符串，包含格式字符串，遵循与 printf 中 format 相同的规范
 * \param[in]       ...: 格式字符串的可选参数
 * \return          如果 `n` 足够大，本应写入的字符数量，不包括终止空字符
 */
#define lwprintf_snprintf(s, n, format, ...) lwprintf_snprintf_ex(NULL, (s), (n), (format), ##__VA_ARGS__)

/**
 * \brief           使用默认 LwPRINTF 实例将可变参数列表中的格式化数据写入指定大小的缓冲区
 * \param[in]       s: 指向存储结果 C 字符串的缓冲区的指针
 *                      缓冲区大小应至少为 `n` 个字符
 * \param[in]       format: C 字符串，包含格式字符串，遵循与 printf 中 format 相同的规范
 * \param[in]       ...: 格式字符串的可选参数
 * \return          本应写入的字符数量，不包括终止空字符
 */
#define lwprintf_sprintf(s, format, ...) lwprintf_sprintf_ex(NULL, (s), (format), ##__VA_ARGS__)

/**
 * \brief           手动启用互斥锁
 * \return          如果已保护返回 `1`，否则返回 `0`
 */
#define lwprintf_protect() lwprintf_protect_ex(NULL)

/**
 * \brief           手动禁用互斥锁
 * \return          如果已保护返回 `1`，否则返回 `0`
 */
#define lwprintf_unprotect() lwprintf_unprotect_ex(NULL)

#if LWPRINTF_CFG_ENABLE_SHORTNAMES || __DOXYGEN__

/**
 * \copydoc         lwprintf_printf
 * \note            此函数等同于 \ref lwprintf_printf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_SHORTNAMES 时可用
 */
#    define lwprintf lwprintf_printf

/**
 * \copydoc         lwprintf_vprintf
 * \note            此函数等同于 \ref lwprintf_vprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_SHORTNAMES 时可用
 */
#    define lwvprintf lwprintf_vprintf

/**
 * \copydoc         lwprintf_vsnprintf
 * \note            此函数等同于 \ref lwprintf_vsnprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_SHORTNAMES 时可用
 */
#    define lwvsnprintf lwprintf_vsnprintf

/**
 * \copydoc         lwprintf_snprintf
 * \note            此函数等同于 \ref lwprintf_snprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_SHORTNAMES 时可用
 */
#    define lwsnprintf lwprintf_snprintf

/**
 * \copydoc         lwprintf_sprintf
 * \note            此函数等同于 \ref lwprintf_sprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_SHORTNAMES 时可用
 */
#    define lwsprintf lwprintf_sprintf

#endif /* LWPRINTF_CFG_ENABLE_SHORTNAMES || __DOXYGEN__ */

#if LWPRINTF_CFG_ENABLE_STD_NAMES || __DOXYGEN__

/**
 * \copydoc         lwprintf_printf
 * \note            此函数等同于 \ref lwprintf_printf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_STD_NAMES 时可用
 */
#    define printf lwprintf_printf

/**
 * \copydoc         lwprintf_vprintf
 * \note            此函数等同于 \ref lwprintf_vprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_STD_NAMES 时可用
 */
#    define vprintf lwprintf_vprintf

/**
 * \copydoc         lwprintf_vsnprintf
 * \note            此函数等同于 \ref lwprintf_vsnprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_STD_NAMES 时可用
 */
#    define vsnprintf lwprintf_vsnprintf

/**
 * \copydoc         lwprintf_snprintf
 * \note            此函数等同于 \ref lwprintf_snprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_STD_NAMES 时可用
 */
#    define snprintf lwprintf_snprintf

/**
 * \copydoc         lwprintf_sprintf
 * \note            此函数等同于 \ref lwprintf_sprintf
 *                      仅在启用 \ref LWPRINTF_CFG_ENABLE_STD_NAMES 时可用
 */
#    define sprintf lwprintf_sprintf

#endif /* LWPRINTF_CFG_ENABLE_STD_NAMES || __DOXYGEN__ */

/* Debug module */
#if !defined(NDEBUG)
/**
 * \brief           调试输出函数
 *
 *                  其目的是向定义的输出进行调试打印，
 *                  在发布版本中将被禁用（当定义 NDEBUG 时）。
 *
 * \note            它调用 \ref lwprintf_printf 来执行打印
 * \note            当启用 \ref NDEBUG 时定义为空
 * \param[in]       fmt: 格式化文本
 * \param[in]       ...: 可选的格式化参数
 */
#    define lwprintf_debug(fmt, ...) lwprintf_printf((fmt), ##__VA_ARGS__)
/**
 * \brief           条件调试输出
 *
 *                  仅当条件为真时才打印格式化文本
 *
 *                  其目的是向定义的输出进行调试打印，
 *                  在发布版本中将被禁用（当定义 NDEBUG 时）。
 *
 * \note            它调用 \ref lwprintf_debug 来执行打印
 * \note            当启用 \ref NDEBUG 时定义为空
 * \param[in]       cond: 在输出消息之前要检查的条件
 * \param[in]       fmt: 格式化文本
 * \param[in]       ...: 可选的格式化参数
 */
#    define lwprintf_debug_cond(cond, fmt, ...)      \
        do                                           \
        {                                            \
            if ((cond))                              \
            {                                        \
                lwprintf_debug((fmt), ##__VA_ARGS__) \
            }                                        \
        } while (0)
#else
#    define lwprintf_debug(fmt, ...)            ((void)0)
#    define lwprintf_debug_cond(cond, fmt, ...) ((void)0)
#endif

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWPRINTF_HDR_H */