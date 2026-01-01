#ifndef EK_MALLOC_H
#define EK_MALLOC_H

#include "ek_def.h"
#include "../third_party/tlsf/tlsf.h"

#define EK_HEAP_SIZE (10 * 1024)

static uint8_t ek_defualt_heap[EK_HEAP_SIZE];
static tlsf_t ek_defualt_tlsf;

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus  */

/**
 * @brief  初始化默认内存堆
 * @note   使用 TLSF 内存分配器创建默认内存池
 *         如果创建失败会阻塞等待直到成功
 */
#define ek_heap_init()                                                          \
    do                                                                          \
    {                                                                           \
        ek_defualt_tlsf = tlsf_create_with_pool(ek_defualt_heap, EK_HEAP_SIZE); \
        while (ek_defualt_tlsf == NULL);                                        \
    } while (0)

/**
 * @brief  销毁默认内存堆
 * @note   释放 TLSF 内存分配器及其管理的所有内存
 */
#define ek_heap_destory() tlsf_destroy(ek_defualt_tlsf)

/**
 * @brief  向默认内存堆添加内存池
 * @param  ptr: 内存池起始地址
 * @param  size: 内存池大小（字节）
 * @note   可以动态扩展内存堆，添加额外的内存区域
 */
#define ek_add_mempool(ptr, size) tlsf_add_pool(ek_defualt_tlsf, ptr, size)

/**
 * @brief  从默认内存堆移除内存池
 * @param  pool: 要移除的内存池指针
 * @note   移除后该内存池不再可用，确保池内无已分配的内存块
 */
#define ek_remove_mempool(pool) tlsf_remove_pool(ek_defualt_tlsf, pool)

/**
 * @brief  从默认内存堆分配内存
 * @param  size: 要分配的内存大小（字节）
 * @retval 分配的内存指针，失败返回 NULL
 */
#define ek_malloc(size) tlsf_malloc(ek_defualt_tlsf, size)

/**
 * @brief  重新分配内存大小
 * @param  ptr: 原内存指针
 * @param  size: 新的内存大小（字节）
 * @retval 重新分配后的内存指针，失败返回 NULL
 * @note   如果 ptr 为 NULL，等同于 ek_malloc
 *         如果 size 为 0，等同于 ek_free
 */
#define ek_realloc(ptr, size) tlsf_realloc(ek_defualt_tlsf, ptr, size)

/**
 * @brief  释放内存到默认内存堆
 * @param  ptr: 要释放的内存指针
 * @note   释放后指针会被置为 NULL，避免悬空指针
 */
#define ek_free(ptr)                     \
    do                                   \
    {                                    \
        tlsf_free(ek_defualt_tlsf, ptr); \
        ptr = NULL;                      \
    } while (0)

#ifdef __cplusplus
}
#endif /* __cplusplus  */

#endif /* EK_MALLOC_H  */
