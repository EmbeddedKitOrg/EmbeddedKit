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

#include <stdarg.h>
#include <stdio.h>

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
#endif /* __weak implementation */
#endif /* __weak */

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
#endif /* __unused implementation */
#endif /* __unused */

/*未使用变量宏*/
#if defined(__GNUC__) || defined(__clang__)
#define UNUSED_VAR(x) ((void)(x))
#elif defined(_MSC_VER)
#define UNUSED_VAR(x) ((void)(x))
#else
#define UNUSED_VAR(X) ((void)(x))
#endif /* UNUSED_VAR macro */

/* 空指针定义 */
#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */

/* bool 定义*/
#ifndef bool
// 定义布尔类型（基于 unsigned char 节省内存）
typedef unsigned char bool;
// 定义假值（0）和真值（1）
#define false (0 == 1)
#define true  (1 == 1)
#endif /* bool */

/* 数据类型定义 */
#ifndef __STDINT_H
#ifndef _STDINT_H
#ifndef _STDINT_H_
#ifndef __STDINT_TYPES_DEFINED__

/* 整数类型限制宏定义 */
#define INT8_MIN   (-0x80)
#define INT8_MAX   (0x7F)
#define UINT8_MAX  (0xFF)

#define INT16_MIN  (-0x8000)
#define INT16_MAX  (0x7FFF)
#define UINT16_MAX (0xFFFF)

#define INT32_MIN  (-0x7FFFFFFF - 1)
#define INT32_MAX  (0x7FFFFFFF)
#define UINT32_MAX (0xFFFFFFFFU)

#define INT64_MIN  (-0x7FFFFFFFFFFFFFFFLL - 1)
#define INT64_MAX  (0x7FFFFFFFFFFFFFFFLL)
#define UINT64_MAX (0xFFFFFFFFFFFFFFFFULL)

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

typedef uint32_t uintptr_t; /*!< 用于指针和整数转换. */

#define _SYS__STDINT_H // 避免GCC报错

#endif /* __STDINT_TYPES_DEFINED__ */
#endif /* _STDINT_H_ */
#endif /* _STDINT_H */
#endif /* __STDINT_H */

#ifdef __cpluscplus
extern "C"
{
#endif /* __cpluscplus */

/**
 * @brief EmbeddedKit 全局统一状态枚举
 * @details 所有模块统一使用此枚举返回操作状态
 */
typedef enum
{
    EK_OK = 0, /*!< 操作成功 */
    EK_ERROR = -1, /*!< 通用错误 */
    EK_INVALID_PARAM = -2, /*!< 参数错误 */
    EK_TIMEOUT = -3, /*!< 超时错误 */
    EK_NO_MEMORY = -4, /*!< 内存不足/内存分配失败 */
    EK_NOT_INITIALIZED = -5, /*!< 未初始化 */
    EK_NOT_FOUND = -6, /*!< 未找到 */
    EK_ALREADY_EXISTS = -7, /*!< 已存在 */
    EK_FULL = -8, /*!< 已满 */
    EK_EMPTY = -9, /*!< 为空 */
    EK_INSUFFICIENT_SPACE = -10, /*!< 空间不足 */
    EK_UNKNOWN = -11, /*!< 未知错误 */
    EK_NULL_POINTER = -12 /*!< 空指针错误 */
} EK_Result_t;

typedef uint32_t EK_Size_t; // size大小类型，修正为数值类型

/* ========================= 函数声明区 ========================= */

// 内存操作函数
void EK_vMemCpy(void *p_dst, const void *p_src, EK_Size_t bytes);
void EK_vMemSet(void *p_dst, uint8_t value, EK_Size_t bytes);
int EK_iMemCmp(const void *p_buf1, const void *p_buf2, EK_Size_t bytes);

// 字符串操作函数
EK_Size_t EK_sStrLen(const char *p_str);
EK_Size_t EK_sStrNLen(const char *p_str, EK_Size_t max_len);
char *EK_pStrCpy(char *p_dst, const char *p_src);
char *EK_pStrNCpy(char *p_dst, const char *p_src, EK_Size_t max_len);
int EK_iStrCmp(const char *p_str1, const char *p_str2);
int EK_iStrNCmp(const char *p_str1, const char *p_str2, EK_Size_t max_len);
char *EK_pStrCat(char *p_dst, const char *p_src);
char *EK_pStrChr(const char *p_str, int ch);

// 数值转换函数
char *EK_pItoA(int value, char *p_str, int base);
int EK_iAtoI(const char *p_str);

// 位操作函数
void EK_vSetBit(void *p_data, uint32_t bit_pos);
void EK_vClearBit(void *p_data, uint32_t bit_pos);
void EK_vToggleBit(void *p_data, uint32_t bit_pos);
bool EK_bTestBit(const void *p_data, uint32_t bit_pos);

// 校验函数
uint8_t EK_u8CheckSum(const uint8_t *p_data, EK_Size_t length);
uint8_t EK_u8XorCheck(const uint8_t *p_data, EK_Size_t length);

// 数学函数
int EK_iAbs(int value);
int EK_iMax(int a, int b);
int EK_iMin(int a, int b);
int EK_iClamp(int value, int min_val, int max_val);

#ifdef __cpluscplus
}
#endif /* __cpluscplus */

#endif /* __EK_COMMON_H */
