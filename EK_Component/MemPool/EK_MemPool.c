/**
 * @file EK_MemPool.c
 * @brief 内存池管理模块实现 (仿照FreeRTOS heap4设计思路)
 * @details 实现动态内存分配与回收功能，采用双向链表管理空闲块
 *          支持块分割与合并减少碎片，使用首次适应算法
 * @author N1ntyNine99
 * @date 2025-09-22
 * @version v1.1
 */

/* ========================= 头文件包含区 ========================= */
#include "EK_MemPool.h"

/* ========================= 宏定义区 ========================= */
/**
 * @brief 内存池总大小 (字节)
 * @note 可根据系统资源调整，建议至少1KB
 */
#ifndef MEMPOOL_SIZE
#define MEMPOOL_SIZE (4096)
#endif /* MEMPOOL_SIZE */

/**
 * @brief 内存对齐大小 (字节)
 * @note 必须是2的幂次，通常为4或8字节
 */
#ifndef MEMPOOL_ALIGNMENT
#define MEMPOOL_ALIGNMENT (8)
#endif /* MEMPOOL_ALIGNMENT */

/** @brief 已分配标记位(最高位) */
#define ALLOCATED_MASK (0x80000000UL)

/** @brief 获取块的实际大小(去除标记位) */
#define GET_SIZE(MemPool_BlockSize) ((MemPool_BlockSize) & ~ALLOCATED_MASK)

/** @brief 检查块是否已分配 */
#define IS_ALLOCATED(MemPool_BlockSize) (((MemPool_BlockSize) & ALLOCATED_MASK) != 0)

/** @brief 设置块为已分配 */
#define SET_ALLOCATED(MemPool_BlockSize) ((MemPool_BlockSize) | ALLOCATED_MASK)

/** @brief 设置块为空闲 */
#define SET_FREE(MemPool_BlockSize) ((MemPool_BlockSize) & ~ALLOCATED_MASK)

/** @brief 字节对齐宏 */
#define ALIGN_UP(size) (((size) + MEMPOOL_ALIGNMENT - 1) & ~(MEMPOOL_ALIGNMENT - 1))

/** @brief 最小块大小 */
#define MIN_BLOCK_SIZE (sizeof(MemBlock_t))

/* ========================= 全局变量区 ========================= */
/** @brief 静态内存堆数组 */
static uint8_t heap_memory[MEMPOOL_SIZE];

/** @brief 空闲块链表头和尾 */
static MemBlock_t free_list_head, *free_list_end = NULL;

/** @brief 内存池统计信息 */
static PoolStats_t pool_statistics = {0};

/** @brief 初始化标志 */
static bool pool_initialized = false;

/* ========================= 内部函数前置声明区 ========================= */
static void v_init_heap(void);
static void v_insert_free_block(MemBlock_t *block_to_insert);
static MemBlock_t *p_find_suitable_block(EK_Size_t wanted_size);
static void v_split_block(MemBlock_t *block, EK_Size_t wanted_size);
static void v_merge_blocks(void *ptr);

/* ========================= 内部函数定义区 ========================= */
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
    EK_Size_t heap_size = MEMPOOL_SIZE;
    EK_Size_t total_heap_size;

    // 确保堆起始地址对齐
    aligned_heap = (uint8_t *)ALIGN_UP((EK_Size_t)heap_memory);

    // 调整堆大小以考虑对齐损失
    total_heap_size = heap_size - ((EK_Size_t)aligned_heap - (EK_Size_t)heap_memory);

    // 创建起始标记块
    free_list_head.MemPool_NextFree = (MemBlock_t *)aligned_heap;
    free_list_head.MemPool_PrevFree = NULL;
    free_list_head.MemPool_BlockSize = 0;

    // 设置结束标记块的位置
    free_list_end = (MemBlock_t *)(aligned_heap + total_heap_size - sizeof(MemBlock_t));
    free_list_end->MemPool_NextFree = NULL;
    free_list_end->MemPool_PrevFree = (MemBlock_t *)aligned_heap; // Initially points to the first block
    free_list_end->MemPool_BlockSize = 0;
    free_list_end->MemPool_BlockSize = SET_ALLOCATED(free_list_end->MemPool_BlockSize);

    // 创建第一个大的空闲块
    first_block = (MemBlock_t *)aligned_heap;
    first_block->MemPool_PrevFree = &free_list_head;
    first_block->MemPool_NextFree = free_list_end;
    first_block->MemPool_BlockSize = total_heap_size - sizeof(MemBlock_t);

    // 确保第一个块大小不会与结束标记冲突
    first_block->MemPool_BlockSize = SET_FREE(first_block->MemPool_BlockSize);

    // 初始化统计信息
    pool_statistics.Pool_TotalSize = total_heap_size;
    pool_statistics.Pool_FreeBytes = first_block->MemPool_BlockSize;
    pool_statistics.Pool_MinFreeBytes = first_block->MemPool_BlockSize;
    pool_statistics.Pool_AllocCount = 0;
    pool_statistics.Pool_FreeCount = 0;
}

/**
 * @brief 初始化内存池
 * @return 初始化是否成功
 * @retval true 初始化成功
 * @retval false 初始化失败
 */
bool EK_bMemPool_Init(void)
{
    // 检查是否已经初始化
    if (pool_initialized)
    {
        return true;
    }

    // 清零堆内存
    EK_vMemSet(heap_memory, 0, MEMPOOL_SIZE);

    // 初始化堆结构
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
    EK_vMemSet(&pool_statistics, 0, sizeof(pool_statistics));
    EK_vMemSet(heap_memory, 0, MEMPOOL_SIZE);
}

/* ========================= 内部辅助函数区 ========================= */

/**
 * @brief 将空闲块插入到空闲链表中
 * @param block_to_insert 要插入的块
 * @retval 无
 * @note 简单插入到链表头部
 */
STATIC_INLINE void v_insert_free_block(MemBlock_t *block_to_insert)
{
    // 插入到链表头部
    block_to_insert->MemPool_NextFree = free_list_head.MemPool_NextFree;
    block_to_insert->MemPool_PrevFree = &free_list_head;
    free_list_head.MemPool_NextFree->MemPool_PrevFree = block_to_insert;
    free_list_head.MemPool_NextFree = block_to_insert;
}

/**
 * @brief 查找合适大小的空闲块
 * @param wanted_size 需要的大小
 * @retval 找到的块指针，NULL表示未找到
 * @note 使用首次适应算法
 */
STATIC_INLINE MemBlock_t *p_find_suitable_block(EK_Size_t wanted_size)
{
    MemBlock_t *current;

    // 遍历空闲链表查找合适的块
    for (current = free_list_head.MemPool_NextFree; current != free_list_end; current = current->MemPool_NextFree)
    {
        if (GET_SIZE(current->MemPool_BlockSize) >= wanted_size)
        {
            // 找到合适的块，从空闲链表中移除
            current->MemPool_PrevFree->MemPool_NextFree = current->MemPool_NextFree;
            current->MemPool_NextFree->MemPool_PrevFree = current->MemPool_PrevFree;
            return current;
        }
    }

    return NULL; // 未找到合适的块
}

/**
 * @brief 分割块
 * @param block 要分割的块
 * @param wanted_size 需要的大小
 * @retval 无
 * @note 如果剩余部分足够大，将其作为新的空闲块
 */
STATIC_INLINE void v_split_block(MemBlock_t *block, EK_Size_t wanted_size)
{
    MemBlock_t *new_block;
    EK_Size_t MemPool_BlockSize = GET_SIZE(block->MemPool_BlockSize);

    // 检查剩余部分是否足够大以形成新的空闲块
    if ((MemPool_BlockSize - wanted_size) > MIN_BLOCK_SIZE)
    {
        // 创建新的空闲块
        new_block = (MemBlock_t *)((uint8_t *)block + wanted_size);
        new_block->MemPool_BlockSize = MemPool_BlockSize - wanted_size;
        new_block->MemPool_BlockSize = SET_FREE(new_block->MemPool_BlockSize);

        // 更新原块大小
        block->MemPool_BlockSize = wanted_size;

        // 将新块插入空闲链表
        v_insert_free_block(new_block);
    }
}

/**
 * @brief 合并相邻的空闲块
 * @param ptr 要释放的内存指针
 * @retval 无
 * @note 检查前后相邻块并进行合并
 */
STATIC_INLINE void v_merge_blocks(void *ptr)
{
    MemBlock_t *block = (MemBlock_t *)((uint8_t *)ptr - sizeof(MemBlock_t));
    MemBlock_t *current;
    EK_Size_t MemPool_BlockSize;

    // 设置为空闲状态
    block->MemPool_BlockSize = SET_FREE(block->MemPool_BlockSize);
    MemPool_BlockSize = GET_SIZE(block->MemPool_BlockSize);

    // 尝试与后面的物理块合并
    MemBlock_t *next_block = (MemBlock_t *)((uint8_t *)block + MemPool_BlockSize);
    if (next_block < free_list_end && !IS_ALLOCATED(next_block->MemPool_BlockSize))
    {
        // 从空闲链表中移除 next_block (O(1) 操作)
        next_block->MemPool_PrevFree->MemPool_NextFree = next_block->MemPool_NextFree;
        next_block->MemPool_NextFree->MemPool_PrevFree = next_block->MemPool_PrevFree;

        // 合并块大小
        block->MemPool_BlockSize += GET_SIZE(next_block->MemPool_BlockSize);
        MemPool_BlockSize = GET_SIZE(block->MemPool_BlockSize);
    }

    // 尝试与前面的物理块合并
    // 遍历空闲链表寻找可能的前驱块 (此部分仍然是 O(N) )
    for (current = free_list_head.MemPool_NextFree; current != free_list_end; current = current->MemPool_NextFree)
    {
        EK_Size_t current_size = GET_SIZE(current->MemPool_BlockSize);
        MemBlock_t *current_next_physical = (MemBlock_t *)((uint8_t *)current + current_size);

        if (current_next_physical == block)
        {
            // 从空闲链表中移除 current (O(1) 操作)
            current->MemPool_PrevFree->MemPool_NextFree = current->MemPool_NextFree;
            current->MemPool_NextFree->MemPool_PrevFree = current->MemPool_PrevFree;

            // 合并到前驱块
            current->MemPool_BlockSize += MemPool_BlockSize;
            block = current; // 更新块指针为合并后的块
            break; // 找到后即可退出循环
        }
    }

    // 将最终的合并块插入到空闲链表
    v_insert_free_block(block);
}

/* ========================= 公用API函数定义区 ========================= */
/**
 * @brief 从内存池分配指定大小的内存
 * @param size 需要分配的内存大小(字节)
 * @return 分配的内存块指针
 * @retval NULL 分配失败
 * @note 使用首次适应算法查找合适的空闲块
 */
void *EK_pMemPool_Malloc(EK_Size_t size)
{
    MemBlock_t *block;
    void *return_ptr = NULL;
    EK_Size_t wanted_size;

    // 检查内存池是否已初始化
    if (!pool_initialized)
    {
        return NULL;
    }

    // 参数检查
    if (size == 0)
    {
        return NULL;
    }

    // 计算实际需要的大小(包含头部信息并对齐)
    wanted_size = ALIGN_UP(size + sizeof(MemBlock_t));

    // 确保不小于最小块大小
    if (wanted_size < MIN_BLOCK_SIZE)
    {
        wanted_size = MIN_BLOCK_SIZE;
    }

    // 检查是否有足够的空闲内存
    if (wanted_size > pool_statistics.Pool_FreeBytes)
    {
        return NULL;
    }

    // 查找合适的空闲块
    block = p_find_suitable_block(wanted_size);
    if (block != NULL)
    {
        // 分割块(如果剩余部分足够大)
        v_split_block(block, wanted_size);

        // 标记块为已分配
        block->MemPool_BlockSize = SET_ALLOCATED(wanted_size);

        // 更新统计信息
        pool_statistics.Pool_FreeBytes -= wanted_size;
        if (pool_statistics.Pool_FreeBytes < pool_statistics.Pool_MinFreeBytes)
        {
            pool_statistics.Pool_MinFreeBytes = pool_statistics.Pool_FreeBytes;
        }
        pool_statistics.Pool_AllocCount++;

        // 返回用户可用的内存地址(跳过头部)
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
    EK_Size_t MemPool_BlockSize;

    // 参数检查
    if (ptr == NULL || !pool_initialized)
    {
        return false;
    }

    // 获取块头部地址
    block = (MemBlock_t *)((uint8_t *)ptr - sizeof(MemBlock_t));

    // 简化检查：只检查块是否已分配
    if (!IS_ALLOCATED(block->MemPool_BlockSize))
    {
        return false; // 重复释放或野指针
    }

    // 获取块大小
    MemPool_BlockSize = GET_SIZE(block->MemPool_BlockSize);

    // 更新统计信息
    pool_statistics.Pool_FreeBytes += MemPool_BlockSize;
    pool_statistics.Pool_FreeCount++;

    // 合并相邻块并插入空闲链表
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
EK_Size_t EK_uMemPool_GetFreeSize(void)
{
    return pool_initialized ? pool_statistics.Pool_FreeBytes : 0;
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
    EK_Size_t total_free = 0;
    uint32_t block_count = 0;

    if (!pool_initialized)
    {
        return false;
    }

    // 遍历空闲链表
    for (current = free_list_head.MemPool_NextFree; current != free_list_end && block_count < 10000;
         current = current->MemPool_NextFree)
    { // 防止死循环
        // 检查块大小
        if (GET_SIZE(current->MemPool_BlockSize) < MIN_BLOCK_SIZE)
        {
            return false;
        }

        // 检查块是否确实是空闲的
        if (IS_ALLOCATED(current->MemPool_BlockSize))
        {
            return false;
        }

        // 检查双向链表指针的完整性
        if (current->MemPool_PrevFree->MemPool_NextFree != current)
        {
            return false;
        }
        if (current->MemPool_NextFree->MemPool_PrevFree != current)
        {
            return false;
        }

        total_free += GET_SIZE(current->MemPool_BlockSize);
        block_count++;
    }

    // 检查计算的空闲字节数是否与统计值一致
    return (total_free == pool_statistics.Pool_FreeBytes);
}