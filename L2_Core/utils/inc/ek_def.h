/**
 * @file ek_def.h
 * @brief 基础类型定义和编译器相关宏
 * @author N1netyNine99
 *
 * 提供跨编译器的统一宏定义，包括：
 * - 弱符号、打包、对齐等属性宏
 * - 内联函数宏
 * - 汇编指令宏
 * - 平台相关的换行符定义
 *
 * 支持的编译器：GCC、ARM Compiler 6、ARM Compiler 5
 */

#ifndef EK_DEF_H
#define EK_DEF_H

#include <inttypes.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 换行符定义
 * @note Linux 平台使用 \n，其他平台使用 \r\n
 */
#ifdef __linux__
#    define CRLF "\n"
#else
#    define CRLF " \r\n"
#endif /* __linux__ */

/* ========== GCC 编译器宏定义 ========== */
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)

#    define __EK_WEAK          __attribute__((weak))
#    define __EK_PACKED        __attribute__((packed))
#    define __EK_PACKED_STRUCT struct __attribute__((packed))
#    define __EK_ALIGNED(x)    __attribute__((aligned(x)))
#    define __EK_USED          __attribute__((used))
#    define __EK_SECTION(x)    __attribute__((section(x)))
#    define __EK_NO_RETURN     __attribute__((noreturn))
#    define __EK_UNUSED(x)     ((void)(x))
#    define __EK_INLINE        inline
#    define __EK_STATIC_INLINE static inline
#    define __EK_ALWAYS_INLINE __attribute__((always_inline)) static inline
#    define __EK_ASM           __asm
/* ========== ARM Compiler 6 宏定义 ========== */
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)

#    define __EK_WEAK          __attribute__((weak))
#    define __EK_PACKED        __attribute__((packed))
#    define __EK_PACKED_STRUCT struct __attribute__((packed))
#    define __EK_ALIGNED(x)    __attribute__((aligned(x)))
#    define __EK_USED          __attribute__((used))
#    define __EK_SECTION(x)    __attribute__((section(x)))
#    define __EK_NO_RETURN     __attribute__((noreturn))
#    define __EK_UNUSED(x)     ((void)(x))
#    define __EK_INLINE        inline
#    define __EK_STATIC_INLINE static inline
#    define __EK_ALWAYS_INLINE __attribute__((always_inline)) static inline
#    define __EK_ASM           __asm

/* ========== ARM Compiler 5 宏定义 ========== */
#elif defined(__CC_ARM)

#    define __EK_WEAK          __weak
#    define __EK_PACKED        __packed
#    define __EK_PACKED_STRUCT __packed struct
#    define __EK_ALIGNED(x)    __align(x)
#    define __EK_USED          __attribute__((used))
#    define __EK_SECTION(x)    __attribute__((section(x)))
#    define __EK_NO_RETURN     __declspec(noreturn)
#    define __EK_UNUSED(x)     ((void)(x))
#    define __EK_INLINE        __inline
#    define __EK_STATIC_INLINE static __inline
#    define __EK_ALWAYS_INLINE __forceinline
#    define __EK_ASM           __asm
/* ========== 不支持的编译器（空定义） ========== */
#else
#    define __EK_WEAK
#    define __EK_PACKED
#    define __EK_PACKED_STRUCT struct
#    define __EK_ALIGNED(x)
#    define __EK_USED
#    define __EK_SECTION(x)
#    define __EK_NO_RETURN
#    define __EK_UNUSED(x)     ((void)(x))
#    define __EK_INLINE        inline
#    define __EK_STATIC_INLINE static inline
#    define __EK_ALWAYS_INLINE static inline
#    define __EK_ASM           asm
#endif

/* ========== 功能宏 ========== */
#define EK_FREQ_K(x)            ((x) * 1000UL)
#define EK_FREQ_M(x)            ((x) * 1000UL * 1000UL)
#define EK_ARRAY_LEN(x)         (sizeof(x) / (sizeof((x)[0])))
#define EK_CLAMP(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))
#define EK_GET_FILE_NAME(file_path)                            \
    (strrchr((file_path), '/') ? strrchr((file_path), '/') + 1 \
                               : (strrchr((file_path), '\\') ? strrchr((file_path), '\\') + 1 : (file_path)))

#endif /* EK_DEF_H */
