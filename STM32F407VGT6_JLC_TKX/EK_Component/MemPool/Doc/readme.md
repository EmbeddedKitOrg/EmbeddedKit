# EK_MemPool - 内存池管理模块

## 概述

EK_MemPool 是一个仿照 FreeRTOS heap4 设计思路实现的高效内存池管理模块。采用单向链表管理空闲块，支持动态内存分配与回收，具备块分割与合并功能以减少内存碎片，使用首次适应算法进行内存分配。

## 核心特性

- **动态内存管理**：支持运行时内存分配和释放
- **碎片优化**：自动合并相邻空闲块，减少内存碎片
- **首次适应算法**：快速查找合适大小的内存块
- **块分割机制**：大块内存可分割为所需大小加剩余块
- **完整性检查**：提供内存池完整性验证功能
- **统计信息**：实时监控内存使用情况和历史数据
- **对齐优化**：支持内存对齐以提高访问效率

## 配置参数

### 编译时配置
```c
#ifndef MEMPOOL_SIZE
#define MEMPOOL_SIZE (4096)      // 内存池总大小（字节）
#endif

#ifndef MEMPOOL_ALIGNMENT  
#define MEMPOOL_ALIGNMENT (8)    // 内存对齐大小（字节）
#endif
```

### 可调整参数
在 **EK_Config.h** 中设置：
- **MEMPOOL_SIZE**：内存池总容量，建议至少1KB
- **MEMPOOL_ALIGNMENT**：对齐大小，必须是2的幂次，通常为4或8字节

## 数据结构

### MemBlock_t - 内存块结构体
```c
typedef struct MemBlock
{
    struct MemBlock *MemPool_NextFree; // 指向下一个空闲块
    EK_Size_t MemPool_BlockSize;          // 块大小，最高位用作分配标记
} MemBlock_t;
```

### PoolStats_t - 统计信息结构体
```c
typedef struct
{
    EK_Size_t Pool_TotalSize;      // 内存池总大小
    EK_Size_t Pool_FreeBytes;      // 当前可用字节数
    EK_Size_t Pool_MinFreeBytes;  // 历史最小可用字节数
    EK_Size_t Pool_AllocCount;     // 分配次数统计
    EK_Size_t Pool_FreeCount;      // 释放次数统计
} PoolStats_t;
```

## API 接口

### 内存分配与释放

#### 内存分配
```c
void *EK_pMemPool_Malloc(EK_Size_t size);
```
- **功能**：从内存池分配指定大小的内存
- **参数**：`size` - 需要分配的字节数
- **返回值**：分配的内存指针，失败返回NULL
- **特性**：
  - 自动内存对齐
  - 首次适应算法
  - 支持块分割
  - 更新统计信息

**使用示例**：
```c
// 分配不同大小的内存块
void *ptr1 = EK_pMemPool_Malloc(128);
void *ptr2 = EK_pMemPool_Malloc(256);
void *ptr3 = EK_pMemPool_Malloc(64);

if (ptr1 && ptr2 && ptr3) {
    printf("内存分配成功: %p, %p, %p\n", ptr1, ptr2, ptr3);
}
```

#### 内存释放
```c
bool EK_bMemPool_Free(void *ptr);
```
- **功能**：释放内存块并自动合并相邻空闲块
- **参数**：`ptr` - 要释放的内存指针
- **返回值**：释放成功返回true，失败返回false
- **特性**：
  - 自动合并相邻空闲块
  - 防止重复释放
  - 指针有效性检查

**使用示例**：
```c
// 释放内存
bool free_success = true;
if (ptr1) free_success &= EK_bMemPool_Free(ptr1);
if (ptr2) free_success &= EK_bMemPool_Free(ptr2);
if (ptr3) free_success &= EK_bMemPool_Free(ptr3);

if (free_success) {
    printf("内存释放成功\n");
}
```

### 状态查询

#### 获取剩余内存
```c
EK_Size_t EK_sMemPool_GetFreeSize(void);
```
- **功能**：获取当前可用内存大小
- **返回值**：剩余字节数

**使用示例**：
```c
printf("分配后剩余字节: %u\n", EK_sMemPool_GetFreeSize());
printf("释放后剩余字节: %u\n", EK_sMemPool_GetFreeSize());
```

#### 获取统计信息
```c
void EK_vMemPool_GetStats(PoolStats_t *stats);
```
- **功能**：获取内存池详细统计信息
- **参数**：`stats` - 指向统计信息结构体的指针
- **用途**：性能监控、内存泄漏检测

**使用示例**：
```c
// 获取内存池统计信息
PoolStats_t stats;
EK_vMemPool_GetStats(&stats);

printf("总容量: %u字节\n", (uint32_t)stats.Pool_TotalSize);
printf("可用字节: %u字节\n", (uint32_t)stats.Pool_FreeBytes);
printf("历史最少可用: %u字节\n", (uint32_t)stats.Pool_MinFreeBytes);
printf("分配次数: %u\n", (uint32_t)stats.Pool_AllocCount);
printf("释放次数: %u\n", (uint32_t)stats.Pool_FreeCount);

// 内存使用率计算
float usage = (float)(stats.Pool_TotalSize - stats.Pool_FreeBytes) / stats.Pool_TotalSize * 100;
printf("内存使用率: %.1f%%\n", usage);

// 内存泄漏检测
if (stats.Pool_AllocCount != stats.Pool_FreeCount) {
    printf("警告: 检测到潜在内存泄漏!\n");
    printf("泄漏次数: %u\n", (uint32_t)(stats.Pool_AllocCount - stats.Pool_FreeCount));
}
```

#### 完整性检查
```c
bool EK_bMemPool_CheckIntegrity(void);
```
- **功能**：检查内存池数据结构完整性
- **返回值**：检查通过返回true，失败返回false
- **用途**：调试、故障诊断
- **检查项**：
  - 空闲链表完整性
  - 块大小合理性
  - 统计数据一致性

**使用示例**：
```c
// 检查内存完整性
bool integrity = EK_bMemPool_CheckIntegrity();
if (integrity) {
    printf("内存完整性检查 ✅ - 正常\n");
} else {
    printf("内存完整性检查 ❌ - 异常\n");
}
```

## 完整测试示例

```c
void Test_MemPool(void)
{
    printf("=== 内存池测试 ===\n");

    // 获取内存池统计信息
    PoolStats_t stats;
    EK_vMemPool_GetStats(&stats);

    printf("总容量: %u字节\n", (uint32_t)stats.Pool_TotalSize);
    printf("可用字节: %u字节\n", (uint32_t)stats.Pool_FreeBytes);
    printf("历史最少可用: %u字节\n", (uint32_t)stats.Pool_MinFreeBytes);
    printf("分配次数: %u\n", (uint32_t)stats.Pool_AllocCount);
    printf("释放次数: %u\n", (uint32_t)stats.Pool_FreeCount);

    // 测试内存分配和释放
    void *ptr1 = EK_pMemPool_Malloc(128);
    void *ptr2 = EK_pMemPool_Malloc(256);
    void *ptr3 = EK_pMemPool_Malloc(64);

    if (ptr1 && ptr2 && ptr3) {
        printf("内存分配测试 ✅ - 已分配3个内存块: %p, %p, %p\n", ptr1, ptr2, ptr3);
    } else {
        printf("内存分配测试 ❌ - 分配失败\n");
    }

    printf("分配后剩余字节: %u\n", EK_sMemPool_GetFreeSize());

    // 检查内存完整性
    bool integrity = EK_bMemPool_CheckIntegrity();
    if (integrity) {
        printf("内存完整性检查 ✅ - 正常\n");
    } else {
        printf("内存完整性检查 ❌ - 异常\n");
    }

    // 释放内存
    bool free_success = true;
    if (ptr1) free_success &= EK_bMemPool_Free(ptr1);
    if (ptr2) free_success &= EK_bMemPool_Free(ptr2);
    if (ptr3) free_success &= EK_bMemPool_Free(ptr3);

    if (free_success) {
        printf("内存释放测试 ✅ - 释放成功\n");
    } else {
        printf("内存释放测试 ❌ - 释放失败\n");
    }

    printf("释放后剩余字节: %u\n", EK_sMemPool_GetFreeSize());
    printf("内存池测试完成\n\n");
}
```

## 使用场景与应用

### 1. 动态数据结构
```c
// 动态分配链表节点
typedef struct Node {
    int data;
    struct Node *next;
} EK_Node_t;

EK_Node_t *create_node(int value) {
    EK_Node_t *node = (EK_Node_t*)EK_pMemPool_Malloc(sizeof(EK_Node_t));
    if (node) {
        node->data = value;
        node->next = NULL;
    }
    return node;
}

void destroy_node(EK_Node_t *node) {
    if (node) {
        EK_bMemPool_Free(node);
    }
}
```

### 2. 临时缓冲区管理
```c
// 数据处理缓冲区
void process_data(uint8_t *input, EK_Size_t len) {
    // 分配临时工作缓冲区
    uint8_t *work_buffer = (uint8_t*)EK_pMemPool_Malloc(len * 2);
    if (!work_buffer) {
        printf("Failed to allocate work buffer\n");
        return;
    }
    
    // 数据处理逻辑
    memcpy(work_buffer, input, len);
    // ... 处理数据 ...
    
    // 释放临时缓冲区
    EK_bMemPool_Free(work_buffer);
}
```

### 3. 消息队列
```c
typedef struct Message {
    uint32_t msg_id;
    EK_Size_t data_len;
    uint8_t data[];
} Message_t;

Message_t *create_message(uint32_t id, void *data, EK_Size_t len) {
    Message_t *msg = (Message_t*)EK_pMemPool_Malloc(sizeof(Message_t) + len);
    if (msg) {
        msg->msg_id = id;
        msg->data_len = len;
        memcpy(msg->data, data, len);
    }
    return msg;
}

void destroy_message(Message_t *msg) {
    EK_bMemPool_Free(msg);
}
```

## 性能特点

### 时间复杂度
- **分配操作**：O(n) - 最坏情况需要遍历整个空闲链表
- **释放操作**：O(n) - 需要查找合并位置
- **状态查询**：O(1) - 直接返回统计信息

### 空间开销
- **元数据开销**：每个分配的块额外占用sizeof(MemBlock_t)字节
- **对齐损失**：根据MEMPOOL_ALIGNMENT设置，通常为0-7字节
- **总开销**：约为实际使用内存的3-5%

### 碎片特性
- **内部碎片**：由于对齐要求产生，通常很小
- **外部碎片**：通过块合并算法最小化
- **最坏情况**：交替分配释放可能产生大量小碎片

## 注意事项

1. **线程安全**：模块本身不提供线程同步，多线程环境需要外部保护
2. **内存对齐**：分配的内存地址会按MEMPOOL_ALIGNMENT对齐
3. **大小限制**：单次分配不能超过内存池总大小
4. **指针检查**：释放时会检查指针有效性，防止野指针
5. **统计准确性**：统计信息实时更新，可用于内存泄漏检测
6. **初始化要求**：使用前需要确保内存池已正确初始化

该内存池模块为嵌入式系统提供了高效、可靠的动态内存管理方案，特别适用于需要频繁分配释放小块内存的应用场景。通过合理的参数配置和使用策略，可以显著提高系统的内存利用率和运行效率。