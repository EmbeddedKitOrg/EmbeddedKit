#include "../inc/ek_mem.h"

#if EK_HEAP_NO_TLSF == 0

uint8_t ek_default_heap[EK_HEAP_SIZE];
tlsf_t ek_default_tlsf;

static size_t ek_unused_bytes = 0;
static size_t ek_used_bytes = 0;

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
