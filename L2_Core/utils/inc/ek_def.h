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
#include <stdarg.h>
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

#    define __WEAK          __attribute__((weak))
#    define __PACKED        __attribute__((packed))
#    define __PACKED_STRUCT struct __attribute__((packed))
#    define __ALIGNED(x)    __attribute__((aligned(x)))
#    define __USED          __attribute__((used))
#    define __SECTION(x)    __attribute__((section(x)))
#    define __NO_RETURN     __attribute__((noreturn))
#    define __UNUSED(x)     ((void)(x))
#    define __INLINE        inline
#    define __STATIC_INLINE static inline
#    define __ALWAYS_INLINE __attribute__((always_inline)) static inline
#    define __ASM           __asm
/* ========== ARM Compiler 6 宏定义 ========== */
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)

#    define __WEAK          __attribute__((weak))
#    define __PACKED        __attribute__((packed))
#    define __PACKED_STRUCT struct __attribute__((packed))
#    define __ALIGNED(x)    __attribute__((aligned(x)))
#    define __USED          __attribute__((used))
#    define __SECTION(x)    __attribute__((section(x)))
#    define __NO_RETURN     __attribute__((noreturn))
#    define __UNUSED(x)     ((void)(x))
#    define __INLINE        inline
#    define __STATIC_INLINE static inline
#    define __ALWAYS_INLINE __attribute__((always_inline)) static inline
#    define __ASM           __asm

/* ========== ARM Compiler 5 宏定义 ========== */
#elif defined(__CC_ARM)

#    define __WEAK          __weak
#    define __PACKED        __packed
#    define __PACKED_STRUCT __packed struct
#    define __ALIGNED(x)    __align(x)
#    define __USED          __attribute__((used))
#    define __SECTION(x)    __attribute__((section(x)))
#    define __NO_RETURN     __declspec(noreturn)
#    define __UNUSED(x)     ((void)(x))
#    define __INLINE        __inline
#    define __STATIC_INLINE static __inline
#    define __ALWAYS_INLINE __forceinline
#    define __ASM           __asm
/* ========== 不支持的编译器（空定义） ========== */
#else

#    if 1
#        error "unsupport compiler.please fixup the marcos below."
#    endif

#    define __WEAK
#    define __PACKED
#    define __PACKED_STRUCT struct
#    define __ALIGNED(x)
#    define __USED
#    define __SECTION(x)
#    define __NO_RETURN
#    define __UNUSED(x)     ((void)(x))
#    define __INLINE        inline
#    define __STATIC_INLINE static inline
#    define __ALWAYS_INLINE static inline
#    define __ASM           asm
#endif

/* ========== 功能宏 ========== */
#define EK_ARRAY_LEN(x)         (sizeof(x) / (sizeof((x)[0])))
#define EK_CLAMP(val, min, max) (((val) < (min)) ? (min) : (((val) > (max)) ? (max) : (val)))
#define EK_GET_FILE_NAME(file_path)                             \
    (strrchr((file_path), '/') ? strrchr((file_path), '/') + 1  \
                               : (strrchr((file_path), '\\') ? strrchr((file_path), '\\') + 1 : (file_path)))

#endif /* EK_DEF_H */
