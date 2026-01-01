/**
 * \file            lwprintf_sys.h
 * \brief           操作系统系统函数
 */

#ifndef __LWPRINTF_SYS_H
#define __LWPRINTF_SYS_H

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
#ifndef LWPRINTF_SYS_HDR_H
#define LWPRINTF_SYS_HDR_H

#include <stddef.h>
#include <stdint.h>
#include "lwprintf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if LWPRINTF_CFG_OS || __DOXYGEN__

/**
 * \defgroup        LWPRINTF_SYS 系统函数
 * \brief           与操作系统一起使用时的系统函数
 * \{
 */

/**
 * \brief           创建新的互斥锁并将值赋给句柄
 * \param[out]      m: 用于保存互斥锁句柄的输出变量
 * \return          成功时返回 `1`，否则返回 `0`
 */
uint8_t lwprintf_sys_mutex_create(LWPRINTF_CFG_OS_MUTEX_HANDLE* m);

/**
 * \brief           检查互斥锁句柄是否有效
 * \param[in]       m: 要检查是否有效的互斥锁句柄
 * \return          成功时返回 `1`，否则返回 `0`
 */
uint8_t lwprintf_sys_mutex_isvalid(LWPRINTF_CFG_OS_MUTEX_HANDLE* m);

/**
 * \brief           等待互斥锁直到准备好（无限时间）
 * \param[in]       m: 要等待的互斥锁句柄
 * \return          成功时返回 `1`，否则返回 `0`
 */
uint8_t lwprintf_sys_mutex_wait(LWPRINTF_CFG_OS_MUTEX_HANDLE* m);

/**
 * \brief           释放已锁定的互斥锁
 * \param[in]       m: 要释放的互斥锁句柄
 * \return          成功时返回 `1`，否则返回 `0`
 */
uint8_t lwprintf_sys_mutex_release(LWPRINTF_CFG_OS_MUTEX_HANDLE* m);

/**
 * \}
 */

#endif /* LWPRINTF_CFG_OS || __DOXYGEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LWPRINTF_SYS_HDR_H */


#endif/* __LWPRINTF_SYS_H */
