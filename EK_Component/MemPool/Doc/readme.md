# EK_MemPool 内存池模块

## 1. 简介

`EK_MemPool` 是一个专为嵌入式系统设计的动态内存管理模块。

其设计思想借鉴了 FreeRTOS 的 `heap_4` 实现，旨在提供一个高效、稳定且碎片可控的内存分配方案。它通过管理一个静态分配的大内存块（内存池）来满足系统的动态内存需求，避免了标准 `malloc` 和 `free` 可能带来的不确定性和碎片化问题。

## 2. 核心特性

- **静态内存池**: 基于静态声明的数组 `heap_memory`，大小和对齐方式可配置，便于在编译时确定内存占用。
- **首次适应算法 (First-Fit)**: 遍历空闲链表，返回第一个足够大的内存块，分配效率高。
- **块分割与合并**:
  - **分割**: 当找到的空闲块大于请求大小时，自动将其分割成两部分，剩余部分放回空闲链表，减少内存浪费。
  - **合并**: 释放内存时，自动检查并与前后相邻的空闲块合并，形成更大的连续空闲块，有效对抗内存碎片。
- **双向链表管理**: 使用双向链表 `MemBlock_t` 来组织所有空闲内存块，使得插入、删除和合并操作高效。
- **内存对齐**: 所有分配的内存地址都会根据 `MEMPOOL_ALIGNMENT` 宏进行对齐，满足大多数处理器的访问要求。
- **丰富的统计与诊断**:
  - 提供 `PoolStats_t` 结构体，用于追踪内存池的总大小、当前空闲大小、历史最小空闲大小、分配/释放次数等。
  - 提供完整性检查功能 `EK_bMemPool_CheckIntegrity`，用于调试和验证内存池的健康状态。

## 3. 可配置项

在 `EK_MemPool.c` 文件中，可以修改以下宏定义来调整内存池的行为：

- `MEMPOOL_SIZE`: 定义内存池的总大小（字节）。
- `MEMPOOL_ALIGNMENT`: 定义内存分配的对齐字节数，必须是2的幂。

```c
// 示例
#define MEMPOOL_SIZE (4096)      // 内存池总大小为 4KB
#define MEMPOOL_ALIGNMENT (8)    // 8字节对齐
```

## 4. API 函数参考

| 函数名 | 描述 |
| :--- | :--- |
| `EK_bMemPool_Init()` | 初始化内存池模块。必须在使用其他API前调用。 |
| `EK_vMemPool_Deinit()` | 销毁内存池，清空所有状态和内存。 |
| `EK_pMemPool_Malloc(size)` | 从内存池中分配指定大小的内存。 |
| `EK_bMemPool_Free(ptr)` | 释放之前分配的内存块。 |
| `EK_vMemPool_GetStats(stats)` | 获取内存池的详细统计信息。 |
| `EK_uMemPool_GetFreeSize()` | 获取当前可用的空闲内存字节数。 |
| `EK_bMemPool_CheckIntegrity()`| 检查内存池数据结构的完整性，用于调试。 |

## 5. 使用示例

```c
#include "EK_MemPool.h"
#include <stdio.h>
#include <string.h>

void mempool_example(void) {
    // 1. 初始化内存池
    if (!EK_bMemPool_Init()) {
        printf("Memory pool initialization failed!\n");
        return;
    }
    printf("Memory pool initialized. Free size: %u bytes\n", EK_uMemPool_GetFreeSize());

    // 2. 分配内存
    char *my_buffer = (char *)EK_pMemPool_Malloc(128);
    if (my_buffer == NULL) {
        printf("Malloc failed!\n");
        return;
    }
    printf("Malloc(128) successful. Free size: %u bytes\n", EK_uMemPool_GetFreeSize());

    // 3. 使用内存
    strcpy(my_buffer, "Hello from EK_MemPool!");
    printf("Buffer content: %s\n", my_buffer);

    // 4. 释放内存
    if (EK_bMemPool_Free(my_buffer)) {
        printf("Free successful. Free size: %u bytes\n", EK_uMemPool_GetFreeSize());
    } else {
        printf("Free failed!\n");
    }

    // 5. 获取统计信息
    PoolStats_t stats;
    EK_vMemPool_GetStats(&stats);
    printf("\n--- Pool Stats ---\n");
    printf("Total Size:     %u\n", stats.Pool_TotalSize);
    printf("Free Bytes:     %u\n", stats.Pool_FreeBytes);
    printf("Min Free Bytes: %u\n", stats.Pool_MinFreeBytes);
    printf("Alloc Count:    %u\n", stats.Pool_AllocCount);
    printf("Free Count:     %u\n", stats.Pool_FreeCount);

    // 6. 检查完整性
    if (EK_bMemPool_CheckIntegrity()) {
        printf("Memory pool integrity check passed.\n");
    } else {
        printf("Memory pool integrity check failed!\n");
    }

    // 7. 销毁内存池 (如果不再需要)
    EK_vMemPool_Deinit();
}
