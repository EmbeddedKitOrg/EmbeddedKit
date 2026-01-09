/*
 * @note 内存管理都来自于 tlsf(../third_party/tlsf/tlsf.c),对于不同大小的内存需求管理，
 * 建议修改下面两个参数来抉择一个合适的内存管理的开销(去上述的源码中修改) 
 * 这里的两个参数决定了内存分配器的管理开销 (Overhead) 和 运行效率。
 * 管理结构体大小(字节) ≈ FL_INDEX_MAX * (2 ^ SL_INDEX_COUNT_LOG2) * 4
 * 我们设置的默认值如下
 * FL_INDEX_MAX = 24 ; SL_INDEX_COUNT_LOG2 = 3;
 * 管理结构体大小 = 24 * 2^3 * 4 Bytes = 768 Bytes
 * 最大可以管理 = 2^24 = 16M byes
 * @para1: FL_INDEX_MAX (First Level Index)
 * ----------------------------------------------------------------------------------
 * 含义: 决定内存池能管理的最大连续内存块大小 (2的N次方)。
 * 策略: 根据你的 最大内存池大小 (Pool Size) 查表填入。
 * (填入值必须 >= log2(Pool Size))
 *
 * | 内存池大小 (Pool Size) | 建议填入值 (FL_INDEX_MAX) | 备注                    |
 * | :--------------------- | :-----------------------: | :---------------------- |
 * | < 16 KB                | 14                        | 适用于极小 RAM (M0/M3)  |
 * | < 32 KB                | 15                        |                         |
 * | < 64 KB                | 16                        | 适用于 50KB 典型场景    |
 * | < 128 KB               | 17                        |                         |
 * | < 512 KB               | 19                        |                         |
 * | < 16 MB                | 24                        | 通用嵌入式配置          |
 * | < 1 GB                 | 30                        | 桌面级/Linux配置        |
 *
 * @para2: SL_INDEX_COUNT_LOG2 (Second Level Index Log2)
 * ----------------------------------------------------------------------------------
 * 含义: 决定空闲块的分类细致度 (2^N 份)。
 * 影响: 值越大，碎片率越低(分配越准)，但内存开销(RAM)呈倍数增长。
 *
 * | 填入值 | 分割份数 | 32位系统下每级开销 | 适用场景建议 (Trade-off)               |
 * | :----: | :------: | :----------------: | :------------------------------------- |
 * |   5    | 32 份    | 128 字节/级        | [高性能PC] 追求极低碎片，内存充足      |
 * |   4    | 16 份    | 64 字节/级         | [折中方案] 适合 >1MB 的内存池          |
 * |   3    | 8 份     | 32 字节/级         | [嵌入式推荐] 适合 10KB-100KB，性价比高 |
 * |   2    | 4 份     | 16 字节/级         | [极限压缩]  RAM极度紧缺(<5KB)时使用   |
 *
 * ==================================================================================
 * @example
 * [开销计算示例 - 帮助您抉择] (基于32位系统指针)
 * 场景 A: 10KB ~ 50KB 内存池 (推荐配置)
 * -> FL_INDEX_MAX = 16 (支持到64KB)
 * -> SL_INDEX_COUNT_LOG2 = 3 (切8份)
 * -> 固定开销 ≈ 16 * 8 * 4 = 【512 字节】 (非常节省)
 *
 * 场景 B: 标准默认配置 (不建议用于小内存)
 * -> FL_INDEX_MAX = 30 (支持到1GB)
 * -> SL_INDEX_COUNT_LOG2 = 5 (切32份)
 * -> 固定开销 ≈ 30 * 32 * 4 = 【3840 字节】 (对小内存池是巨大浪费)
 * ==================================================================================
 */
#ifndef EK_MEM_H
#define EK_MEM_H

#include "ek_def.h"
// 使用 tlsf 来作为 malloc free
#include "../../third_party/tlsf/tlsf.h"
#include "../../../ek_conf.h"

#ifndef EK_HEAP_SIZE
#    define EK_HEAP_SIZE (10 * 1024)
#endif /* EK_HEAP_SIZE  */

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus  */

/**
 * @brief  获取内存池总大小
 * @retval 内存池总字节数
 */
#ifndef ek_heap_total_size
#    define ek_heap_total_size() (EK_HEAP_SIZE - tlsf_size())
#endif /* ek_heap_total_size */

/**
 * @brief  初始化默认内存堆
 * @note   使用 TLSF 内存分配器创建默认内存池
 *         如果创建失败会阻塞等待直到成功
 */
#ifndef ek_heap_init
#    define ek_heap_init()                                                          \
        do                                                                          \
        {                                                                           \
            ek_default_tlsf = tlsf_create_with_pool(ek_default_heap, EK_HEAP_SIZE); \
            while (ek_default_tlsf == NULL);                                        \
        } while (0)
#endif /* ek_heap_init */

/**
 * @brief  销毁默认内存堆
 * @note   释放 TLSF 内存分配器及其管理的所有内存
 */
#ifndef ek_heap_destory
#    define ek_heap_destory() tlsf_destroy(ek_default_tlsf)
#endif /* ek_heap_destory */

/**
 * @brief  向默认内存堆添加内存池
 * @param  ptr: 内存池起始地址
 * @param  size: 内存池大小（字节）
 * @note   可以动态扩展内存堆，添加额外的内存区域
 */
#ifndef ek_heap_add_pool
#    define ek_heap_add_pool(ptr, size) tlsf_add_pool(ek_default_tlsf, ptr, size)
#endif /* ek_heap_add_pool */

/**
 * @brief  从默认内存堆移除内存池
 * @param  pool: 要移除的内存池指针
 * @note   移除后该内存池不再可用，确保池内无已分配的内存块
 */
#ifndef ek_heap_remove_pool
#    define ek_heap_remove_pool(pool) tlsf_remove_pool(ek_default_tlsf, pool)
#endif /* ek_heap_remove_pool */

/**
 * @brief  从默认内存堆分配内存
 * @param  size: 要分配的内存大小（字节）
 * @retval 分配的内存指针，失败返回 NULL
 */
#ifndef ek_malloc
#    define ek_malloc(size) tlsf_malloc(ek_default_tlsf, size)
#endif /* ek_malloc */

/**
 * @brief  重新分配内存大小
 * @param  ptr: 原内存指针
 * @param  size: 新的内存大小（字节）
 * @retval 重新分配后的内存指针，失败返回 NULL
 * @note   如果 ptr 为 NULL，等同于 ek_malloc
 *         如果 size 为 0，等同于 ek_free
 */
#ifndef ek_realloc
#    define ek_realloc(ptr, size) tlsf_realloc(ek_default_tlsf, ptr, size)
#endif /* ek_realloc */

/**
 * @brief  释放内存到默认内存堆
 * @param  ptr: 要释放的内存指针
 * @note   释放后指针会被置为 NULL，避免悬空指针
 */
#ifndef ek_free
#    define ek_free(ptr)                     \
        do                                   \
        {                                    \
            tlsf_free(ek_default_tlsf, ptr); \
            ptr = NULL;                      \
        } while (0)
#endif /* ek_free */

#if EK_HEAP_NO_TLSF == 0

extern uint8_t ek_default_heap[];
extern tlsf_t ek_default_tlsf;

/**
 * @brief  获取当前空闲内存大小
 * @retval 空闲字节数
 * @note   每次调用都会遍历内存池统计，有一定开销
 */
size_t ek_heap_unused(void);

/**
 * @brief  获取当使用的内存大小
 * @retval 使用的字节数
 * @note   每次调用都会遍历内存池统计，有一定开销
 */
size_t ek_heap_used(void);

#endif /* EK_HEAP_NO_TLSF */

#ifdef __cplusplus
}
#endif /* __cplusplus  */

#endif /* EK_MEM_H  */
