# MemPool 内存池组件

## 概述

MemPool组件实现了一个高效的动态内存池管理系统，采用类似FreeRTOS heap4的设计思路。该组件通过单向链表管理空闲内存块，支持块分割与合并，有效减少内存碎片化，提供稳定可靠的内存分配服务。

## 特性

- **碎片化管理**：支持空闲块自动合并，减少内存碎片
- **块分割**：大块可按需分割，提高内存利用率
- **单向链表管理**：采用高效的单向链表管理空闲块
- **内存对齐**：支持可配置的内存对齐，优化访问性能
- **统计功能**：提供详细的内存使用统计信息
- **完整性检查**：内置内存池完整性验证功能
- **线程安全设计**：为多线程环境预留扩展接口

## 设计原理

### 内存块结构
```c
typedef struct MemBlock
{
    struct MemBlock *next_free; /**< 指向下一个空闲块 */
    size_t block_size;          /**< 块大小，最高位用作分配标记 */
} MemBlock_t;
```

### 分配标记机制
- 块大小的最高位用作分配标记位
- 0表示空闲块，1表示已分配块
- 实际块大小通过掩码操作获取

### 内存合并算法
- 释放时检查相邻块是否为空闲状态
- 自动合并连续的空闲块
- 维护按地址排序的空闲块链表

## 数据结构

### MemBlock_t - 内存块节点
```c
typedef struct MemBlock
{
    struct MemBlock *next_free; /**< 指向下一个空闲块 */
    size_t block_size;          /**< 块大小，最高位用作分配标记 */
} MemBlock_t;
```

### PoolStats_t - 统计信息结构
```c
typedef struct
{
    size_t total_size;      /**< 内存池总大小 */
    size_t free_bytes;      /**< 当前可用字节数 */
    size_t min_free_bytes;  /**< 历史最小可用字节数 */
    size_t alloc_count;     /**< 分配次数统计 */
    size_t free_count;      /**< 释放次数统计 */
} PoolStats_t;
```

## 配置选项

### 编译时配置
```c
// 内存池总大小 (默认4KB)
#define MEMPOOL_SIZE (4096)

// 内存对齐大小 (默认8字节)
#define MEMPOOL_ALIGNMENT (8)
```

## API接口

### 初始化和清理
- `bool MemPool_Init(void)` - 初始化内存池
- `void MemPool_Deinit(void)` - 清理内存池

### 内存分配和释放
- `void *MemPool_Malloc(size_t size)` - 分配内存
- `bool MemPool_Free(void *ptr)` - 释放内存

### 状态查询
- `void MemPool_GetStats(PoolStats_t *stats)` - 获取统计信息
- `size_t MemPool_GetFreeSize(void)` - 获取可用内存大小
- `bool MemPool_CheckIntegrity(void)` - 检查内存池完整性

## 使用示例

### 基本使用
```c
#include "MemPool.h"

int main()
{
    // 初始化内存池
    if (!MemPool_Init()) {
        printf("Memory pool initialization failed\n");
        return -1;
    }
    
    // 分配内存
    void *ptr1 = MemPool_Malloc(128);
    void *ptr2 = MemPool_Malloc(256);
    
    if (ptr1 == NULL || ptr2 == NULL) {
        printf("Memory allocation failed\n");
        return -1;
    }
    
    // 使用内存
    memset(ptr1, 0x55, 128);
    memset(ptr2, 0xAA, 256);
    
    // 释放内存
    MemPool_Free(ptr1);
    MemPool_Free(ptr2);
    
    // 清理内存池
    MemPool_Deinit();
    
    return 0;
}
```

### 统计信息查询
```c
#include "MemPool.h"

void print_memory_stats()
{
    PoolStats_t stats;
    MemPool_GetStats(&stats);
    
    printf("Memory Pool Statistics:\n");
    printf("  Total Size: %zu bytes\n", stats.total_size);
    printf("  Free Bytes: %zu bytes\n", stats.free_bytes);
    printf("  Min Free Bytes: %zu bytes\n", stats.min_free_bytes);
    printf("  Allocations: %zu\n", stats.alloc_count);
    printf("  Deallocations: %zu\n", stats.free_count);
    printf("  Utilization: %.2f%%\n", 
           100.0 * (stats.total_size - stats.free_bytes) / stats.total_size);
}
```

### 完整性检查
```c
#include "MemPool.h"

void memory_health_check()
{
    if (MemPool_CheckIntegrity()) {
        printf("Memory pool integrity: OK\n");
    } else {
        printf("Memory pool integrity: CORRUPTED\n");
        // 进行错误处理或系统重启
    }
}
```

## 内存分配策略

### 首次适配算法
1. 遍历空闲块链表
2. 找到第一个满足大小要求的块
3. 如果块过大，进行分割操作
4. 更新链表结构

### 块合并策略
1. 释放时检查前后相邻块
2. 如果相邻块为空闲状态，进行合并
3. 保持空闲块按地址排序
4. 减少碎片化程度

## 性能特性

- **分配时间复杂度**：O(n)，n为空闲块数量
- **释放时间复杂度**：O(n)，包含合并操作
- **空间开销**：每个块8字节的管理开销（64位系统）
- **内存对齐**：可配置对齐大小，优化访问性能

## 注意事项

1. **初始化顺序**：使用前必须调用MemPool_Init()
2. **双重释放**：避免对同一指针多次释放
3. **野指针释放**：只能释放由本内存池分配的指针
4. **内存泄漏**：确保所有分配的内存都被正确释放
5. **线程安全**：当前版本不是线程安全的，多线程环境需要额外保护

## 调试和诊断

### 常见问题诊断
- **分配失败**：检查剩余内存和碎片化程度
- **释放失败**：验证指针有效性和重复释放
- **内存泄漏**：对比分配和释放次数统计
- **内存踩踏**：使用完整性检查功能定期验证

### 性能调优
- **内存池大小**：根据应用需求调整MEMPOOL_SIZE
- **对齐大小**：根据目标平台优化MEMPOOL_ALIGNMENT
- **分配策略**：考虑实现最佳适配或快速适配算法

## 版本信息

- **版本**: 1.0
- **作者**: N1netyNine99
- **日期**: 2025-09-04
- **设计参考**: FreeRTOS heap4算法
