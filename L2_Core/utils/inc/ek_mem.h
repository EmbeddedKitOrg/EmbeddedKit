/**
 * @file ek_mem.h
 * @brief 内存管理接口
 * @author N1netyNine99
 *
 * 提供基于 TLSF 算法的动态内存分配功能
 */

#ifndef EK_MEM_H
#define EK_MEM_H

#include "ek_def.h"
#include "../../../ek_conf.h"

#ifndef EK_HEAP_NO_TLSF
#    define EK_HEAP_NO_TLSF 0 /* 默认使用 TLSF 内存分配器 */
#endif

#ifndef EK_HEAP_SIZE
#    define EK_HEAP_SIZE (10 * 1024)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief  底层内存分配函数（弱定义）
 * @note   默认使用 TLSF 实现，如果不使用 TLSF（EK_HEAP_NO_TLSF=1），
 *         需要用户自行实现这三个函数的强定义版本
 * @warning 这些函数采用弱定义方式，用户可以覆盖默认实现
 */
void *_ek_malloc(size_t size);
void _ek_free(void *ptr);
void *_ek_realloc(void *ptr, size_t size);

/**
 * @brief  从默认内存堆分配内存
 * @param  size: 要分配的内存大小（字节）
 * @retval 分配的内存指针，失败返回 NULL
 */
#ifndef ek_malloc
#    define ek_malloc(size) _ek_malloc(size)
#endif

/**
 * @brief  重新分配内存大小
 * @param  ptr: 原内存指针
 * @param  size: 新的内存大小（字节）
 * @retval 重新分配后的内存指针，失败返回 NULL
 */
#ifndef ek_realloc
#    define ek_realloc(ptr, size) _ek_realloc(ptr, size)
#endif

/**
 * @brief  释放内存到默认内存堆
 * @param  ptr: 要释放的内存指针
 * @note   释放后指针会被置为 NULL，避免悬空指针
 */
#ifndef ek_free
#    define ek_free(ptr)   \
        do                 \
        {                  \
            _ek_free(ptr); \
            ptr = NULL;    \
        } while (0)
#endif

#if EK_HEAP_NO_TLSF == 0

#    include "../../third_party/tlsf/tlsf.h"

extern uint8_t ek_default_heap[];
extern tlsf_t ek_default_tlsf;

/**
 * @brief  初始化默认内存堆
 * @note   使用 TLSF 内存分配器创建默认内存池
 */
#    ifndef ek_heap_init
#        define ek_heap_init()                                                          \
            do                                                                          \
            {                                                                           \
                ek_default_tlsf = tlsf_create_with_pool(ek_default_heap, EK_HEAP_SIZE); \
                while (ek_default_tlsf == NULL);                                        \
            } while (0)
#    endif

/**
 * @brief  获取内存池总大小
 * @retval 内存池总字节数
 */
#    ifndef ek_heap_total_size
#        define ek_heap_total_size() (EK_HEAP_SIZE - tlsf_size())
#    endif

/**
 * @brief  销毁默认内存堆
 */
#    ifndef ek_heap_destory
#        define ek_heap_destory() tlsf_destroy(ek_default_tlsf)
#    endif

/**
 * @brief  向默认内存堆添加内存池
 * @param  ptr: 内存池起始地址
 * @param  size: 内存池大小（字节）
 */
#    ifndef ek_heap_add_pool
#        define ek_heap_add_pool(ptr, size) tlsf_add_pool(ek_default_tlsf, ptr, size)
#    endif

/**
 * @brief  从默认内存堆移除内存池
 * @param  pool: 要移除的内存池指针
 */
#    ifndef ek_heap_remove_pool
#        define ek_heap_remove_pool(pool) tlsf_remove_pool(ek_default_tlsf, pool)
#    endif

/**
 * @brief  获取当前空闲内存大小
 * @retval 空闲字节数
 */
size_t ek_heap_unused(void);

/**
 * @brief  获取当前已使用的内存大小
 * @retval 使用的字节数
 */
size_t ek_heap_used(void);

#else

/* ========================================================================
 * EK_HEAP_NO_TLSF == 1: 用户自定义内存分配器
 * 用户需要:
 *   定义 _ek_malloc, _ek_free, _ek_realloc 函数
 * ======================================================================== */

/**
 * @brief  初始化内存堆 (用户需自行实现或定义为空)
 */
#    ifndef ek_heap_init
#        define ek_heap_init() ((void)0)
#    endif

#endif /* EK_HEAP_NO_TLSF */

#ifdef __cplusplus
}
#endif

#endif /* EK_MEM_H */
