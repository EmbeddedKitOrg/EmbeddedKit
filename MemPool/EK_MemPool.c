/**
 * @file EK_MemPool.c
 * @brief 内存池管理模块实现 (仿照FreeRTOS heap4设计思路)
 * @details 实现动态内存分配与回收功能，采用单向链表管理空闲块
 *          支持块分割与合并减少碎片，使用首次适应算法
 * @author N1ntyNine99
 * @date 2025-09-04
 * @version v1.0
 */

/* ========================= 头文件包含区 ========================= */
#include "EK_MemPool.h"

/* ========================= 宏定义区 ========================= */
/** @brief 已分配标记位(最高位) */
#define ALLOCATED_MASK (0x80000000UL)

/** @brief 获取块的实际大小(去除标记位) */
#define GET_SIZE(block_size) ((block_size) & ~ALLOCATED_MASK)

/** @brief 检查块是否已分配 */
#define IS_ALLOCATED(block_size) (((block_size) & ALLOCATED_MASK) != 0)

/** @brief 设置块为已分配 */
#define SET_ALLOCATED(block_size) ((block_size) | ALLOCATED_MASK)

/** @brief 设置块为空闲 */
#define SET_FREE(block_size) ((block_size) & ~ALLOCATED_MASK)

/** @brief 字节对齐宏 */
#define ALIGN_UP(size) (((size) + MEMPOOL_ALIGNMENT - 1) & ~(MEMPOOL_ALIGNMENT - 1))

/** @brief 最小块大小 */
#define MIN_BLOCK_SIZE (sizeof(MemBlock_t))

/* ========================= 全局变量区 ========================= */
/** @brief 静态内存堆数组 */
static uint8_t heap_memory[MEMPOOL_SIZE];

/** @brief 空闲块链表头和尾 */
static MemBlock_t free_list_start, *free_list_end = NULL;

/** @brief 内存池统计信息 */
static PoolStats_t pool_statistics = {0};

/** @brief 初始化标志 */
static bool pool_initialized = false;

/* ========================= 内部函数前置声明区 ========================= */
static void v_init_heap(void);
static void v_insert_free_block(MemBlock_t *block_to_insert);
static MemBlock_t *p_find_suitable_block(size_t wanted_size);
static void v_split_block(MemBlock_t *block, size_t wanted_size);
static void v_merge_blocks(void *ptr);

/* ========================= 内存池初始化区 ========================= */

/**
 * @brief 初始化内存堆
 * @param 无
 * @retval 无
 * @note 设置初始空闲块和链表结构
 */
static void v_init_heap(void)
{
    MemBlock_t *first_block;
    uint8_t *aligned_heap;
    size_t heap_size = MEMPOOL_SIZE;
    size_t total_heap_size;

    /* 确保堆起始地址对齐 */
    aligned_heap = (uint8_t *)ALIGN_UP((size_t)heap_memory);

    /* 调整堆大小以考虑对齐损失 */
    total_heap_size = heap_size - ((size_t)aligned_heap - (size_t)heap_memory);

    /* 创建起始标记块 */
    free_list_start.next_free = (MemBlock_t *)aligned_heap;
    free_list_start.block_size = 0;

    /* 设置结束标记块的位置 */
    free_list_end = (MemBlock_t *)(aligned_heap + total_heap_size - sizeof(MemBlock_t));
    free_list_end->next_free = NULL;
    free_list_end->block_size = 0;
    free_list_end->block_size = SET_ALLOCATED(free_list_end->block_size);

    /* 创建第一个大的空闲块 */
    first_block = (MemBlock_t *)aligned_heap;
    first_block->next_free = free_list_end;
    first_block->block_size = total_heap_size - sizeof(MemBlock_t);

    /* 确保第一个块大小不会与结束标记冲突 */
    first_block->block_size = SET_FREE(first_block->block_size);

    /* 初始化统计信息 */
    pool_statistics.total_size = total_heap_size;
    pool_statistics.free_bytes = first_block->block_size;
    pool_statistics.min_free_bytes = first_block->block_size;
    pool_statistics.alloc_count = 0;
    pool_statistics.free_count = 0;
}

/**
 * @brief 初始化内存池
 * @return 初始化是否成功
 * @retval true 初始化成功
 * @retval false 初始化失败
 */
bool EK_bMemPool_Init(void)
{
    /* 检查是否已经初始化 */
    if (pool_initialized)
    {
        return true;
    }

    /* 清零堆内存 */
    memset(heap_memory, 0, MEMPOOL_SIZE);

    /* 初始化堆结构 */
    v_init_heap();

    pool_initialized = true;
    return true;
}

/**
 * @brief 销毁内存池
 * @param 无
 * @retval 无
 */
void EK_vMemPool_Deinit(void)
{
    pool_initialized = false;
    memset(&pool_statistics, 0, sizeof(pool_statistics));
    memset(heap_memory, 0, MEMPOOL_SIZE);
}

/* ========================= 内部辅助函数区 ========================= */

/**
 * @brief 将空闲块插入到空闲链表中
 * @param block_to_insert 要插入的块
 * @retval 无
 * @note 按地址顺序插入以便后续合并
 */
static void v_insert_free_block(MemBlock_t *block_to_insert)
{
    MemBlock_t *current = &free_list_start;

    /* 在链表中找到合适的插入位置(按地址排序) */
    while (current->next_free != NULL && current->next_free < block_to_insert)
    {
        current = current->next_free;
    }

    /* 插入块到链表中 */
    block_to_insert->next_free = current->next_free;
    current->next_free = block_to_insert;
}

/**
 * @brief 查找合适大小的空闲块
 * @param wanted_size 需要的大小
 * @retval 找到的块指针，NULL表示未找到
 * @note 使用首次适应算法
 */
static MemBlock_t *p_find_suitable_block(size_t wanted_size)
{
    MemBlock_t *current, *prev = &free_list_start;

    /* 遍历空闲链表查找合适的块 */
    current = free_list_start.next_free;
    while (current != NULL && current != free_list_end)
    {
        if (GET_SIZE(current->block_size) >= wanted_size)
        {
            /* 找到合适的块，从空闲链表中移除 */
            prev->next_free = current->next_free;
            return current;
        }
        prev = current;
        current = current->next_free;
    }

    return NULL; /* 未找到合适的块 */
}

/**
 * @brief 分割块
 * @param block 要分割的块
 * @param wanted_size 需要的大小
 * @retval 无
 * @note 如果剩余部分足够大，将其作为新的空闲块
 */
static void v_split_block(MemBlock_t *block, size_t wanted_size)
{
    MemBlock_t *new_block;
    size_t block_size = GET_SIZE(block->block_size);

    /* 检查剩余部分是否足够大以形成新的空闲块 */
    if ((block_size - wanted_size) > MIN_BLOCK_SIZE)
    {
        /* 创建新的空闲块 */
        new_block = (MemBlock_t *)((uint8_t *)block + wanted_size);
        new_block->block_size = block_size - wanted_size;
        new_block->block_size = SET_FREE(new_block->block_size);

        /* 更新原块大小 */
        block->block_size = wanted_size;

        /* 将新块插入空闲链表 */
        v_insert_free_block(new_block);
    }
}

/**
 * @brief 合并相邻的空闲块
 * @param ptr 要释放的内存指针
 * @retval 无
 * @note 检查前后相邻块并进行合并
 */
static void v_merge_blocks(void *ptr)
{
    MemBlock_t *block = (MemBlock_t *)((uint8_t *)ptr - sizeof(MemBlock_t));
    MemBlock_t *current, *prev;
    uint8_t *block_addr = (uint8_t *)block;

    /* 设置为空闲状态 */
    block->block_size = SET_FREE(block->block_size);

    /* 查找插入位置并检查是否可以与相邻块合并 */
    prev = &free_list_start;
    current = free_list_start.next_free;

    while (current != free_list_end && current < block)
    {
        /* 检查是否可以与前一个块合并 */
        if ((uint8_t *)current + GET_SIZE(current->block_size) == block_addr)
        {
            /* 合并与前一个块 */
            current->block_size += GET_SIZE(block->block_size);
            block = current;
            block_addr = (uint8_t *)block;
        }
        else
        {
            prev = current;
        }
        current = current->next_free;
    }

    /* 检查是否可以与后一个块合并 */
    if (block_addr + GET_SIZE(block->block_size) == (uint8_t *)current && current != free_list_end)
    {
        /* 合并与后一个块 */
        if (prev->next_free == current)
        {
            prev->next_free = current->next_free;
        }
        block->block_size += GET_SIZE(current->block_size);
    }

    /* 如果块没有被合并到已存在的空闲块中，插入到链表 */
    if (prev->next_free != block)
    {
        v_insert_free_block(block);
    }
}

/**
 * @brief 从内存池分配指定大小的内存
 * @param size 需要分配的内存大小(字节)
 * @return 分配的内存块指针
 * @retval NULL 分配失败
 * @note 使用首次适应算法查找合适的空闲块
 */
void *EK_pMemPool_Malloc(size_t size)
{
    MemBlock_t *block;
    void *return_ptr = NULL;
    size_t wanted_size;

    /* 检查内存池是否已初始化 */
    if (!pool_initialized)
    {
        return NULL;
    }

    /* 参数检查 */
    if (size == 0)
    {
        return NULL;
    }

    /* 计算实际需要的大小(包含头部信息并对齐) */
    wanted_size = ALIGN_UP(size + sizeof(MemBlock_t));

    /* 确保不小于最小块大小 */
    if (wanted_size < MIN_BLOCK_SIZE)
    {
        wanted_size = MIN_BLOCK_SIZE;
    }

    /* 检查是否有足够的空闲内存 */
    if (wanted_size > pool_statistics.free_bytes)
    {
        return NULL;
    }

    /* 查找合适的空闲块 */
    block = p_find_suitable_block(wanted_size);
    if (block != NULL)
    {
        /* 分割块(如果剩余部分足够大) */
        v_split_block(block, wanted_size);

        /* 标记块为已分配 */
        block->block_size = SET_ALLOCATED(wanted_size);

        /* 更新统计信息 */
        pool_statistics.free_bytes -= wanted_size;
        if (pool_statistics.free_bytes < pool_statistics.min_free_bytes)
        {
            pool_statistics.min_free_bytes = pool_statistics.free_bytes;
        }
        pool_statistics.alloc_count++;

        /* 返回用户可用的内存地址(跳过头部) */
        return_ptr = (void *)((uint8_t *)block + sizeof(MemBlock_t));
    }

    return return_ptr;
}

/**
 * @brief 释放内存块
 * @param ptr 要释放的内存块指针
 * @return 是否成功释放
 * @retval true 释放成功
 * @retval false 释放失败
 * @note 会自动与相邻的空闲块合并
 */
bool EK_bMemPool_Free(void *ptr)
{
    MemBlock_t *block;
    size_t block_size;

    /* 参数检查 */
    if (ptr == NULL || !pool_initialized)
    {
        return false;
    }

    /* 获取块头部地址 */
    block = (MemBlock_t *)((uint8_t *)ptr - sizeof(MemBlock_t));

    /* 检查指针是否在有效范围内 */
    if ((uint8_t *)block < heap_memory || (uint8_t *)block >= (heap_memory + MEMPOOL_SIZE))
    {
        return false;
    }

    /* 检查块是否确实已分配 */
    if (!IS_ALLOCATED(block->block_size))
    {
        return false; /* 重复释放或野指针 */
    }

    /* 获取块大小 */
    block_size = GET_SIZE(block->block_size);

    /* 更新统计信息 */
    pool_statistics.free_bytes += block_size;
    pool_statistics.free_count++;

    /* 合并相邻块并插入空闲链表 */
    v_merge_blocks(ptr);

    return true;
}

/* ========================= 统计和诊断函数区 ========================= */

/**
 * @brief 获取内存池统计信息
 * @param stats 指向统计信息结构体的指针
 * @retval 无
 */
void EK_vMemPool_GetStats(PoolStats_t *stats)
{
    if (stats != NULL && pool_initialized)
    {
        *stats = pool_statistics;
    }
}

/**
 * @brief 获取剩余可用内存大小
 * @param 无
 * @retval 剩余可用字节数
 */
size_t EK_sMemPool_GetFreeSize(void)
{
    return pool_initialized ? pool_statistics.free_bytes : 0;
}

/**
 * @brief 检查内存池完整性
 * @param 无
 * @retval true: 内存池完整, false: 检测到损坏
 * @note 遍历空闲链表检查完整性
 */
bool EK_bMemPool_CheckIntegrity(void)
{
    MemBlock_t *current;
    size_t total_free = 0;
    uint32_t block_count = 0;

    if (!pool_initialized)
    {
        return false;
    }

    /* 遍历空闲链表 */
    current = free_list_start.next_free;
    while (current != free_list_end && block_count < 1000)
    { /* 防止死循环 */
        /* 检查块是否在有效范围内 */
        if ((uint8_t *)current < heap_memory || (uint8_t *)current >= (heap_memory + MEMPOOL_SIZE))
        {
            return false;
        }

        /* 检查块大小是否合理 */
        if (GET_SIZE(current->block_size) < MIN_BLOCK_SIZE)
        {
            return false;
        }

        /* 检查块是否确实是空闲的 */
        if (IS_ALLOCATED(current->block_size))
        {
            return false;
        }

        total_free += GET_SIZE(current->block_size);
        current = current->next_free;
        block_count++;
    }

    /* 检查计算的空闲字节数是否与统计值一致 */
    return (total_free == pool_statistics.free_bytes);
}
