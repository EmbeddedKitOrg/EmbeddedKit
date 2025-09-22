/**
 * @file EK_MemPool.h
 * @brief 内存池管理头文件
 * @details 定义了内存池的数据结构和操作接口
 * @author N1ntyNine99
 * @date 2025-09-22
 * @version 1.1
 */

#ifndef __EK_MEMPOOL_H
#define __EK_MEMPOOL_H

#include "../EK_Config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 类型定义区 ========================= */
/**
 * @brief 内存块节点结构体 (仿照heap4设计)
 * @note 用于管理空闲内存块的双向链表，每个块包含大小和链表指针
 */
typedef struct MemBlock
{
    struct MemBlock *MemPool_NextFree; /**< 指向下一个空闲块 */
    struct MemBlock *MemPool_PrevFree; /**< 指向上一个空闲块 */
    EK_Size_t MemPool_BlockSize;       /**< 块大小，最高位用作分配标记 */
} MemBlock_t;

/**
 * @brief 内存池统计信息结构体
 */
typedef struct
{
    EK_Size_t Pool_TotalSize; /**< 内存池总大小 */
    EK_Size_t Pool_FreeBytes; /**< 当前可用字节数 */
    EK_Size_t Pool_MinFreeBytes; /**< 历史最小可用字节数 */
    EK_Size_t Pool_AllocCount; /**< 分配次数统计 */
    EK_Size_t Pool_FreeCount; /**< 释放次数统计 */
} PoolStats_t;

/* ========================= 函数声明区 ========================= */
bool EK_bMemPool_Init(void);
void EK_vMemPool_Deinit(void);
void *EK_pMemPool_Malloc(EK_Size_t size);
bool EK_bMemPool_Free(void *ptr);
void EK_vMemPool_GetStats(PoolStats_t *stats);
EK_Size_t EK_sMemPool_GetFreeSize(void);
bool EK_bMemPool_CheckIntegrity(void);

#ifdef __cplusplus
}
#endif

#endif
