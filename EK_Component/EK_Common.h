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
#include <stdint.h>

/* ========================= 函数属性宏 ========================= */

/*弱定义宏*/
#ifndef __weak
#if defined(__GNUC__) || defined(__clang__)
#define __weak __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __weak __weak
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __weak __attribute__((weak))
#elif defined(__C51__)
#define __weak
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
#elif defined(__C51__)
#define __unused
#else
#define __unused
#endif /* __unused implementation */
#endif /* __unused */

/*内部内联宏*/
#ifndef STATIC_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define STATIC_INLINE static inline
#elif defined(__IAR_SYSTEMS_ICC__)
#define STATIC_INLINE static inline
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define STATIC_INLINE static __inline
#elif defined(__C51__)
#define STATIC_INLINE static __inline
#else
#define STATIC_INLINE static
#endif /* STATIC_INLINE implementation */
#endif /* STATIC_INLINE */

/*强内联宏*/
#ifndef ALWAYS_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define ALWAYS_INLINE STATIC_INLINE __attribute__((always_inline))
#elif defined(__IAR_SYSTEMS_ICC__)
#define ALWAYS_INLINE _Pragma("inline=forced") STATIC_INLINE
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define ALWAYS_INLINE __forceinline
#elif defined(__C51__)
#define ALWAYS_INLINE STATIC_INLINE
#else
#define ALWAYS_INLINE STATIC_INLINE
#endif /* ALWAYS_INLINE implementation */
#endif /* ALWAYS_INLINE */

/*强制内部内联宏*/
#ifndef ALWAYS_STATIC_INLINE
#if defined(__GNUC__) || defined(__clang__)
#define ALWAYS_STATIC_INLINE STATIC_INLINE __attribute__((always_inline))
#elif defined(__IAR_SYSTEMS_ICC__)
#define ALWAYS_STATIC_INLINE _Pragma("inline=forced") STATIC_INLINE
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define ALWAYS_STATIC_INLINE static __forceinline
#elif defined(__C51__)
#define ALWAYS_STATIC_INLINE STATIC_INLINE
#else
#define ALWAYS_STATIC_INLINE STATIC_INLINE
#endif /* ALWAYS_STATIC_INLINE implementation */
#endif /* ALWAYS_STATIC_INLINE */

/*未使用变量宏*/
#if defined(__GNUC__) || defined(__clang__) || defined(__IAR_SYSTEMS_ICC__) || defined(__CC_ARM) || \
    defined(__ARMCC_VERSION)
#define UNUSED_VAR(x) ((void)(x))
#elif defined(__C51__)
#define UNUSED_VAR(x) ((void)(x))
#else
#define UNUSED_VAR(x) ((void)(x))
#endif /* UNUSED_VAR macro */

/*裸函数宏*/
#ifndef __naked
#if defined(__GNUC__) || defined(__clang__)
#define __naked __attribute__((naked))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __naked __naked
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __naked __declspec(naked)
#elif defined(__C51__)
#define __naked
#else
#define __naked
#endif /* __naked implementation */
#endif /* __naked */

/* ========================= 数据类型宏 ========================= */

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

/* ========================= 工具宏 ========================= */

/*
 * 类型推导宏
 * - C++ 使用 decltype
 * - GCC/Clang/IAR/Armclang 使用 __typeof__
 * - Arm Compiler 5 (armcc) 使用 typeof 关键字
 * 其余编译器需自行扩展，否则会在编译期抛出错误提示
 */
#if defined(__cplusplus)
#define EK_TYPE_OF(__expression__) decltype(__expression__)
#elif defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
#define EK_TYPE_OF(__expression__) __typeof__(__expression__)
#elif defined(__ICCARM__)
#define EK_TYPE_OF(__expression__) __typeof__(__expression__)
#elif defined(__CC_ARM)
#define EK_TYPE_OF(__expression__) typeof(__expression__)
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#define EK_TYPE_OF(__expression__) __typeof__(__expression__)
#else
#error "EK_TYPE_OF is not supported by this compiler. Please provide an implementation."
#endif /* EK_TYPE_OF */

/**
 * @brief 获取结构体成员中的偏移量
 * @param __type__ 结构体数据类型
 * @param __mem__ 成员名
 * 
 */
#define EK_GET_OFFSET_OF(__type__, __mem__) ((size_t)&((__type__ *)0)->__mem__)

/**
 * @brief 通过一个结构体成员的地址反推得到整个结构体的地址
 * @param __ptr__ 获取地址的指针
 * @param __type__ 结构体数据类型
 * @param __mem__ 成员名 
 * 
 */
#define EK_GET_OWNER_OF(__ptr__, __type__, __mem__) \
    ((__type__ *)(char *)(__ptr__) - EK_GET_OFFSET_OF(__type__, __mem__))

/**
 * @brief 限制表达式值在指定范围内的宏
 * @param __expression__ 要限制的表达式
 * @param __min__        最大值（下界）
 * @param __max__        最小值（上界）
 * @return 限制后的值，确保在 [__min__, __max__] 范围内
 * @warning 使用 ++/-- 操作符到时候 务必保证: ++/-- expression 否则宏不会奏效
 *
 */
#define EK_CLAMP(__expression__, __min__, __max__)                        \
    ({                                                                    \
        EK_TYPE_OF(__expression__) _val = (__expression__);               \
        EK_TYPE_OF(__min__) _min_val = (__min__);                         \
        EK_TYPE_OF(__max__) _max_val = (__max__);                         \
        _val < _min_val ? _min_val : (_val > _max_val ? _max_val : _val); \
    })

#ifdef __cpluscplus
extern "C"
{
#endif /* __cpluscplus */

/* ========================= 通用数据类型声明区 ========================= */

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

/* ========================= 位操作内联函数实现 ========================= */

/**
 * @brief 设置位图中的指定位为1
 * @param p_data 指向位图内存的指针 (void*)
 * @param bit_pos 要操作的位的位置 (从0开始)
 * @note 该函数是类型和对齐安全的。
 */
ALWAYS_INLINE void EK_vSetBit(void *p_data, uint32_t bit_pos)
{
    if (p_data == NULL) return;

    // 以字节方式访问内存，避免对齐问题
    uint8_t *p_map = (uint8_t *)p_data;
    uint32_t byte_index = bit_pos / 8;
    uint8_t bit_in_byte = bit_pos % 8;

    p_map[byte_index] |= (1U << bit_in_byte);
}

/**
 * @brief 清除位图中的指定位为0
 * @param p_data 指向位图内存的指针 (void*)
 * @param bit_pos 要操作的位的位置 (从0开始)
 * @note 该函数是类型和对齐安全的。
 */
ALWAYS_INLINE void EK_vClearBit(void *p_data, uint32_t bit_pos)
{
    if (p_data == NULL) return;

    uint8_t *p_map = (uint8_t *)p_data;
    uint32_t byte_index = bit_pos / 8;
    uint8_t bit_in_byte = bit_pos % 8;

    p_map[byte_index] &= ~(1U << bit_in_byte);
}

/**
 * @brief 翻转位图中的指定位
 * @param p_data 指向位图内存的指针 (void*)
 * @param bit_pos 要操作的位的位置 (从0开始)
 * @note 该函数是类型和对齐安全的。
 */
ALWAYS_INLINE void EK_vToggleBit(void *p_data, uint32_t bit_pos)
{
    if (p_data == NULL) return;

    uint8_t *p_map = (uint8_t *)p_data;
    uint32_t byte_index = bit_pos / 8;
    uint8_t bit_in_byte = bit_pos % 8;

    p_map[byte_index] ^= (1U << bit_in_byte);
}

/**
 * @brief 测试位图中的指定位是否为1
 * @param p_data 指向位图内存的指针 (const void*)
 * @param bit_pos 要测试的位的位置 (从0开始)
 * @return bool 位状态
 * @retval true 位为1
 * @retval false 位为0
 * @note 该函数是类型和对齐安全的。
 */
ALWAYS_INLINE bool EK_bTestBit(const void *p_data, uint32_t bit_pos)
{
    if (p_data == NULL) return false;

    const uint8_t *p_map = (const uint8_t *)p_data;
    uint32_t byte_index = bit_pos / 8;
    uint8_t bit_in_byte = bit_pos % 8;

    return (p_map[byte_index] & (1U << bit_in_byte)) != 0;
}

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
ALWAYS_INLINE void EK_vSetBit(void *p_data, uint32_t bit_pos);
ALWAYS_INLINE void EK_vClearBit(void *p_data, uint32_t bit_pos);
ALWAYS_INLINE void EK_vToggleBit(void *p_data, uint32_t bit_pos);
ALWAYS_INLINE bool EK_bTestBit(const void *p_data, uint32_t bit_pos);

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
