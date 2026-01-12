#ifndef EK_DEF_H
#define EK_DEF_H

#include <inttypes.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __linux__
#    define CRLF "\n"
#else
#    define CRLF " \r\n"
#endif /* __linux__ */

// gcc
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
// ac6
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
// ac5
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
// unsupport compiler
#else
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

#endif /* EK_DEF_H */
