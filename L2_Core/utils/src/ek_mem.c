#include "../inc/ek_mem.h"

#if EK_HEAP_NO_TLSF == 0

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

uint8_t ek_default_heap[EK_HEAP_SIZE];
tlsf_t ek_default_tlsf;

static size_t ek_unused_bytes = 0;
static size_t ek_used_bytes = 0;

__WEAK void *_ek_malloc(size_t size)
{
    return tlsf_malloc(ek_default_tlsf, size);
}

__WEAK void *_ek_realloc(void *ptr, size_t size)
{
    return tlsf_realloc(ek_default_tlsf, ptr, size);
}

__WEAK void _ek_free(void *ptr)
{
    tlsf_free(ek_default_tlsf, ptr);
}

// walker 函数：统计空闲内存
static void ek_mem_walker(void *ptr, size_t size, int used, void *user)
{
    __UNUSED(ptr);
    __UNUSED(user);

    if (!used)
    {
        ek_unused_bytes += size;
    }
    else
    {
        ek_used_bytes += size;
    }
}

size_t ek_heap_unused(void)
{
    pool_t pool = tlsf_get_pool(ek_default_tlsf);
    ek_unused_bytes = 0;
    tlsf_walk_pool(pool, ek_mem_walker, NULL);
    return ek_unused_bytes;
}

size_t ek_heap_used(void)
{
    pool_t pool = tlsf_get_pool(ek_default_tlsf);
    ek_used_bytes = 0;
    tlsf_walk_pool(pool, ek_mem_walker, NULL);
    return ek_used_bytes;
}

#endif /* EK_HEAP_NO_TLSF */
