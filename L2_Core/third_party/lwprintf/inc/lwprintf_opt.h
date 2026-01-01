/**
 * \file            lwprintf_opt.h
 * \brief           LwPRINTF 选项配置不要删除！
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
#ifndef LWPRINTF_OPT_HDR_H
#define LWPRINTF_OPT_HDR_H

// 取消这里的注释可以创建自己的lwprintf的配置
// #define LWPRINTF_IGNORE_USER_OPTS

/* Include application options */
#ifndef LWPRINTF_IGNORE_USER_OPTS
#    include "ek_lwprintf_opt.h"
#endif /* LWPRINTF_IGNORE_USER_OPTS */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * \defgroup        LWPRINTF_OPT 配置
 * \brief           LwPRINTF 选项配置
 * \{
 */

/**
 * \brief           启用 `1` 或禁用 `0` 库中的操作系统支持
 *
 * \note            当启用 `LWPRINTF_CFG_OS` 时，用户必须实现 \ref LWPRINTF_SYS 组中的函数。
 */
#ifndef LWPRINTF_CFG_OS
#    define LWPRINTF_CFG_OS 0
#endif

/**
 * \brief           互斥锁句柄类型
 *
 * \note            如果 \ref LWPRINTF_CFG_OS 设置为 `1`，则必须设置此值。
 *                  如果编译器不知道数据类型，请在定义句柄类型之前包含定义的头文件
 */
#ifndef LWPRINTF_CFG_OS_MUTEX_HANDLE
#    define LWPRINTF_CFG_OS_MUTEX_HANDLE void *
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 手动互斥锁。
 *
 * 当启用此功能时，结合 \ref LWPRINTF_CFG_OS，行为如下：
 * - 系统互斥锁在初始化阶段保持创建状态
 * - 对直接打印函数的调用默认不再是线程安全的
 * - 对 sprintf（缓冲区函数）的调用保持线程安全
 * - 用户必须手动调用 \ref lwprintf_protect 或 \ref lwprintf_protect_ex 函数来保护直接打印操作
 * - 用户必须手动调用 \ref lwprintf_unprotect 或 \ref lwprintf_unprotect_ex 函数来退出保护区域
 *
 * \note            如果您希望完全禁用此库的锁定机制，
 *                  请关闭 \ref LWPRINTF_CFG_OS 并完全手动处理非重入函数的互斥
 */
#ifndef LWPRINTF_CFG_OS_MANUAL_PROTECT
#    define LWPRINTF_CFG_OS_MANUAL_PROTECT 0
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 对有符号或无符号 `long long int` 类型的支持。
 *
 */
#ifndef LWPRINTF_CFG_SUPPORT_LONG_LONG
#    define LWPRINTF_CFG_SUPPORT_LONG_LONG 1
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 对接受任何整数类型的说明符的支持。
 *                  这将启用 `%d, %b, %u, %o, %i, %x` 说明符
 *
 */
#ifndef LWPRINTF_CFG_SUPPORT_TYPE_INT
#    define LWPRINTF_CFG_SUPPORT_TYPE_INT 1
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 对 `%p` 指针打印类型的支持
 *
 * 启用时，架构必须支持 `uintptr_t` 类型，通常在 C11 标准中可用
 */
#ifndef LWPRINTF_CFG_SUPPORT_TYPE_POINTER
#    define LWPRINTF_CFG_SUPPORT_TYPE_POINTER 1
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 对 `%f` 浮点类型的支持
 *
 */
#ifndef LWPRINTF_CFG_SUPPORT_TYPE_FLOAT
#    define LWPRINTF_CFG_SUPPORT_TYPE_FLOAT 1
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 对浮点数的 `%e` 工程输出类型的支持
 *
 * \note            必须启用 \ref LWPRINTF_CFG_SUPPORT_TYPE_FLOAT 才能使用此功能
 *
 */
#ifndef LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
#    define LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING 1
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 对 `%s` 字符串输出的支持
 *
 */
#ifndef LWPRINTF_CFG_SUPPORT_TYPE_STRING
#    define LWPRINTF_CFG_SUPPORT_TYPE_STRING 1
#endif

/**
 * \brief           启用 `1` 或禁用 `0` 对 `%k` 十六进制字节数组输出的支持
 *
 */
#ifndef LWPRINTF_CFG_SUPPORT_TYPE_BYTE_ARRAY
#    define LWPRINTF_CFG_SUPPORT_TYPE_BYTE_ARRAY 1
#endif

/**
 * \brief           指定浮点数的默认精度数
 *
 * 如果说明符本身没有设置精度，则表示逗号后要使用的数字位数
 *
 */
#ifndef LWPRINTF_CFG_FLOAT_DEFAULT_PRECISION
#    define LWPRINTF_CFG_FLOAT_DEFAULT_PRECISION 6
#endif

/**
 * \brief           启用 `1` 或禁用 `0` LwPRINTF API 函数的可选短名称。
 *
 * 它为默认实例添加函数：`lwprintf`、`lwsnprintf` 等
 */
#ifndef LWPRINTF_CFG_ENABLE_SHORTNAMES
#    define LWPRINTF_CFG_ENABLE_SHORTNAMES 0
#endif /* LWPRINTF_CFG_ENABLE_SHORTNAMES */

/**
 * \brief           启用 `1` 或禁用 `0` C 标准 API 名称
 *
 * 默认禁用以免干扰编译器实现。
 * 应用程序可能需要从链接中移除标准 C STDIO 库，
 * 才能在启用此选项时正确编译 LwPRINTF
 */
#ifndef LWPRINTF_CFG_ENABLE_STD_NAMES
#    define LWPRINTF_CFG_ENABLE_STD_NAMES 0
#endif /* LWPRINTF_CFG_ENABLE_SHORTNAMES */

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWPRINTF_OPT_HDR_H */
