/**
 * @file EK_Common.h
 * @brief EmbeddedKit 公共头文件
 * @details 包含全局类型定义、宏定义和统一错误码枚举
 * @author N1ntyNine99
 * @version 1.0
 * @date 2025-01-09
 */

#ifndef __EK_COMMON_H
#define __EK_COMMON_H

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "EK_Config.h"

/*弱定义宏*/
#ifndef __weak
#if defined(__GNUC__) || defined(__clang__)
#define __weak __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __weak __weak
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __weak __weak
#else
#define __weak
#endif
#endif

/*未使用函数宏*/
#ifndef __unused
#if defined(__GNUC__) || defined(__clang__)
#define __unused __attribute__((unused))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __unused __attribute__((unused))
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __unused __attribute__((unused))
#elif defined(_MSC_VER)
#define __unused __pragma(warning(suppress:4505))
#else
#define __unused
#endif
#endif

/*未使用变量宏*/
#if defined(__GNUC__) || defined(__clang__)
#define UNUSED_VAR(x) ((void)(x))
#elif defined(_MSC_VER)
#define UNUSED_VAR(x) ((void)(x))
#else
#define UNUSED_VAR(X) ((void)(x))
#endif

/**
 * @brief 内存分配宏定义
 * @details 函数签名要求是 void* xxx(size_t)
 */
#define _MALLOC(x) malloc(x)

/**
 * @brief 内存释放宏定义
 * @details 函数入参要求是 void*
 */
#define _FREE(X) free(X)

#ifdef __cpluscplus
extern "C"{
#endif

/**
 * @brief EmbeddedKit 全局统一状态枚举
 * @details 所有模块统一使用此枚举返回操作状态
 */
typedef enum {
    EK_OK = 0,              /*!< 操作成功 */
    EK_ERROR = -1,          /*!< 通用错误 */
    EK_INVALID_PARAM = -2,  /*!< 参数错误 */
    EK_TIMEOUT = -3,        /*!< 超时错误 */
    EK_NO_MEMORY = -4,      /*!< 内存不足/内存分配失败 */
    EK_NOT_INITIALIZED = -5,/*!< 未初始化 */
    EK_NOT_FOUND = -6,      /*!< 未找到 */
    EK_ALREADY_EXISTS = -7, /*!< 已存在 */
    EK_FULL = -8,           /*!< 已满 */
    EK_EMPTY = -9,          /*!< 为空 */
    EK_INSUFFICIENT_SPACE = -10, /*!< 空间不足 */
    EK_UNKNOWN = -11,       /*!< 未知错误 */
    EK_NULL_POINTER = -12   /*!< 空指针错误 */
} EK_Result_t;

/**
 * @brief 索引类型定义
 */
typedef uint32_t index_t;


#ifdef __cpluscplus
}
#endif

#endif
