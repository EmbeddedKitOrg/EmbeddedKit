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

#ifdef __cpluscplus
extern "C"
{
#endif

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

/**
 * @brief 索引类型定义
 */
typedef uint32_t index_t;

/* ========================= 函数声明区 ========================= */

// 内存操作函数
void EK_vMemCpy(void *p_dst, const void *p_src, size_t bytes);
void EK_vMemSet(void *p_dst, uint8_t value, size_t bytes);
int EK_iMemCmp(const void *p_buf1, const void *p_buf2, size_t bytes);

// 字符串操作函数
size_t EK_sStrLen(const char *p_str);
size_t EK_sStrNLen(const char *p_str, size_t max_len);
char *EK_pStrCpy(char *p_dst, const char *p_src);
char *EK_pStrNCpy(char *p_dst, const char *p_src, size_t max_len);
int EK_iStrCmp(const char *p_str1, const char *p_str2);
int EK_iStrNCmp(const char *p_str1, const char *p_str2, size_t max_len);
char *EK_pStrCat(char *p_dst, const char *p_src);
char *EK_pStrChr(const char *p_str, int ch);

// 数值转换函数
char *EK_pItoA(int value, char *p_str, int base);
int EK_iAtoI(const char *p_str);

// 位操作函数
void EK_vSetBit(uint32_t *p_data, uint8_t bit_pos);
void EK_vClearBit(uint32_t *p_data, uint8_t bit_pos);
void EK_vToggleBit(uint32_t *p_data, uint8_t bit_pos);
bool EK_bTestBit(uint32_t data, uint8_t bit_pos);

// 校验函数
uint8_t EK_u8CheckSum(const uint8_t *p_data, size_t length);
uint8_t EK_u8XorCheck(const uint8_t *p_data, size_t length);

// 数学函数
int EK_iAbs(int value);
int EK_iMax(int a, int b);
int EK_iMin(int a, int b);
int EK_iClamp(int value, int min_val, int max_val);

#ifdef __cpluscplus
}
#endif

#endif
