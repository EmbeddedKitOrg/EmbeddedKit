/**
 * @file Pool.h
 * @brief 内存池管理模块 (仿照FreeRTOS heap4设计思路)
 * @details 实现动态内存分配功能，支持内存合并，减少碎片化
 *          采用单向链表管理空闲块，支持块分割与合并
 * @author N1netyNine99
 * @date 2025-09-04
 * @version v1.0
 */

#ifndef __POOL_h
#define __POOL_h

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 宏定义区 ========================= */
/**
 * @brief 内存池总大小 (字节)
 * @note 可根据系统资源调整，建议至少1KB
 */
#ifndef MEMPOOL_SIZE
#define MEMPOOL_SIZE (4096)
#endif

/**
 * @brief 内存对齐大小 (字节)
 * @note 必须是2的幂次，通常为4或8字节
 */
#ifndef MEMPOOL_ALIGNMENT
#define MEMPOOL_ALIGNMENT (8)
#endif

/* ========================= 类型定义区 ========================= */
/**
 * @brief 内存块节点结构体 (仿照heap4设计)
 * @note 用于管理空闲内存块的单向链表，每个块包含大小和链表指针
 */
typedef struct MemBlock
{
    struct MemBlock *next_free; /**< 指向下一个空闲块 */
    size_t block_size; /**< 块大小，最高位用作分配标记 */
} MemBlock_t;

/**
 * @brief 内存池统计信息结构体
 */
typedef struct
{
    size_t total_size; /**< 内存池总大小 */
    size_t free_bytes; /**< 当前可用字节数 */
    size_t min_free_bytes; /**< 历史最小可用字节数 */
    size_t alloc_count; /**< 分配次数统计 */
    size_t free_count; /**< 释放次数统计 */
} PoolStats_t;

/* ========================= 函数声明区 ========================= */
bool MemPool_Init(void);
void MemPool_Deinit(void);
void *MemPool_Malloc(size_t size);
bool MemPool_Free(void *ptr);
void MemPool_GetStats(PoolStats_t *stats);
size_t MemPool_GetFreeSize(void);
bool MemPool_CheckIntegrity(void);

#ifdef __cplusplus
}
#endif

#endif
