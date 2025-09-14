# EK_MemPool - 内存池管理模块

## 概述

EK_MemPool 是一个仿照 FreeRTOS heap4 设计思路实现的高效内存池管理模块。采用单向链表管理空闲块，支持动态内存分配与回收，具备块分割与合并功能以减少内存碎片，使用首次适应算法进行内存分配。

## 核心特性

- **动态内存管理**：支持运行时内存分配和释放
- **碎片优化**：自动合并相邻空闲块，减少内存碎片
- **首次适应算法**：快速查找合适大小的内存块
- **块分割机制**：大块内存可分割为所需大小加剩余块
- **完整性检查**：提供内存池完整性验证功能
- **统计信息**：实时监控内存使用情况
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
在"**EK_Config.h**中设置"
- **MEMPOOL_SIZE**：内存池总容量，建议至少1KB
- **MEMPOOL_ALIGNMENT**：对齐大小，必须是2的幂次，通常为4或8字节

## 数据结构

### MemBlock_t - 内存块结构体
```c
typedef struct MemBlock
{
    struct MemBlock *next_free; // 指向下一个空闲块
    size_t block_size;          // 块大小，最高位用作分配标记
} MemBlock_t;
```

### PoolStats_t - 统计信息结构体
```c
typedef struct
{
    size_t total_size;      // 内存池总大小
    size_t free_bytes;      // 当前可用字节数
    size_t min_free_bytes;  // 历史最小可用字节数
    size_t alloc_count;     // 分配次数统计
    size_t free_count;      // 释放次数统计
} PoolStats_t;
```

## API 接口

### 初始化与销毁

#### 内存池初始化
```c
bool EK_bMemPool_Init(void);
```
- **功能**：初始化内存池，创建初始空闲块
- **返回值**：初始化成功返回true
- **说明**：必须在使用其他API前调用

#### 内存池销毁
```c
void EK_vMemPool_Deinit(void);
```
- **功能**：销毁内存池，清空所有数据
- **用途**：系统重启或模块卸载时调用

### 内存分配与释放

#### 内存分配
```c
void *EK_pMemPool_Malloc(size_t size);
```
- **功能**：从内存池分配指定大小的内存
- **参数**：`size` - 需要分配的字节数
- **返回值**：分配的内存指针，失败返回NULL
- **特性**：
  - 自动内存对齐
  - 首次适应算法
  - 支持块分割
  - 更新统计信息

#### 内存释放
```c
bool EK_bMemPool_Free(void *ptr);
```
- **功能**：释放内存块并自动合并相邻空闲块
- **参数**：`ptr` - 要释放的内存指针
- **返回值**：释放成功返回true
- **特性**：
  - 自动合并相邻空闲块
  - 防止重复释放
  - 指针有效性检查

### 状态查询

#### 获取剩余内存
```c
size_t EK_sMemPool_GetFreeSize(void);
```
- **功能**：获取当前可用内存大小
- **返回值**：剩余字节数

#### 获取统计信息
```c
void EK_vMemPool_GetStats(PoolStats_t *stats);
```
- **功能**：获取内存池详细统计信息
- **用途**：性能监控、内存泄漏检测

#### 完整性检查
```c
bool EK_bMemPool_CheckIntegrity(void);
```
- **功能**：检查内存池数据结构完整性
- **用途**：调试、故障诊断
- **检查项**：
  - 空闲链表完整性
  - 块大小合理性
  - 统计数据一致性

## 使用场景与示例

### 1. 动态数据结构
```c
// 初始化内存池
if (!EK_bMemPool_Init()) {
    printf("Memory pool initialization failed!\n");
    return;
}

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
void process_data(uint8_t *input, size_t len) {
    // 分配临时工作缓冲区
    uint8_t *work_buffer = (uint8_t*)EK_pMemPool_Malloc(len * 2);
    if (!work_buffer) {
        printf("Failed to allocate work buffer\n");
        return;
    }
    
    // 数据处理逻辑
    EK_vMemCpy(work_buffer, input, len);
    // ... 处理数据 ...
    
    // 释放临时缓冲区
    EK_bMemPool_Free(work_buffer);
}
```

### 3. 消息队列
```c
typedef struct Message {
    uint32_t msg_id;
    size_t data_len;
    uint8_t data[];
} Message_t;

Message_t *create_message(uint32_t id, void *data, size_t len) {
    Message_t *msg = (Message_t*)EK_pMemPool_Malloc(sizeof(Message_t) + len);
    if (msg) {
        msg->msg_id = id;
        msg->data_len = len;
        EK_vMemCpy(msg->data, data, len);
    }
    return msg;
}

void destroy_message(Message_t *msg) {
    EK_bMemPool_Free(msg);
}

void message_handler(void) {
    // 创建消息
    uint8_t data[] = {1, 2, 3, 4, 5};
    Message_t *msg = create_message(0x100, data, sizeof(data));
    
    // 处理消息
    if (msg) {
        printf("Message ID: 0x%X, Length: %zu\n", msg->msg_id, msg->data_len);
        // ... 处理消息内容 ...
        destroy_message(msg);
    }
}
```

### 4. 内存监控系统
```c
void memory_monitor_task(void) {
    PoolStats_t stats;
    EK_vMemPool_GetStats(&stats);
    
    printf("=== Memory Pool Statistics ===\n");
    printf("Total Size: %zu bytes\n", stats.total_size);
    printf("Free Bytes: %zu bytes\n", stats.free_bytes);
    printf("Min Free: %zu bytes\n", stats.min_free_bytes);
    printf("Usage: %.1f%%\n", 
           (float)(stats.total_size - stats.free_bytes) / stats.total_size * 100);
    printf("Alloc Count: %zu\n", stats.alloc_count);
    printf("Free Count: %zu\n", stats.free_count);
    
    // 内存泄漏检测
    if (stats.alloc_count != stats.free_count) {
        printf("WARNING: Potential memory leak detected!\n");
        printf("Leak Count: %zu\n", stats.alloc_count - stats.free_count);
    }
    
    // 完整性检查
    if (!EK_bMemPool_CheckIntegrity()) {
        printf("ERROR: Memory pool integrity check failed!\n");
    }
}
```

### 5. 智能指针实现
```c
typedef struct SmartPtr {
    void *ptr;
    size_t size;
    int ref_count;
} SmartPtr_t;

SmartPtr_t *smart_malloc(size_t size) {
    SmartPtr_t *smart = (SmartPtr_t*)EK_pMemPool_Malloc(sizeof(SmartPtr_t));
    if (smart) {
        smart->ptr = EK_pMemPool_Malloc(size);
        if (smart->ptr) {
            smart->size = size;
            smart->ref_count = 1;
        } else {
            EK_bMemPool_Free(smart);
            smart = NULL;
        }
    }
    return smart;
}

void smart_free(SmartPtr_t *smart) {
    if (smart && --smart->ref_count == 0) {
        EK_bMemPool_Free(smart->ptr);
        EK_bMemPool_Free(smart);
    }
}
```

## 算法实现详解

### 1. 首次适应算法
```c
// 遍历空闲链表，找到第一个大小满足要求的块
static MemBlock_t *_find_suitable_block(size_t wanted_size) {
    MemBlock_t *current = free_list_start.next_free;
    while (current != free_list_end) {
        if (GET_SIZE(current->block_size) >= wanted_size) {
            return current;  // 找到合适的块
        }
        current = current->next_free;
    }
    return NULL;  // 未找到
}
```

### 2. 块分割机制
```c
// 如果找到的块比需要的大，分割成两个块
static void _split_block(MemBlock_t *block, size_t wanted_size) {
    size_t block_size = GET_SIZE(block->block_size);
    if ((block_size - wanted_size) > MIN_BLOCK_SIZE) {
        // 创建新的空闲块
        MemBlock_t *new_block = (MemBlock_t*)((uint8_t*)block + wanted_size);
        new_block->block_size = block_size - wanted_size;
        // 插入到空闲链表
        _insert_free_block(new_block);
        // 更新原块大小
        block->block_size = wanted_size;
    }
}
```

### 3. 块合并算法
```c
// 释放时合并相邻的空闲块
static void _merge_blocks(void *ptr) {
    MemBlock_t *block = (MemBlock_t*)((uint8_t*)ptr - sizeof(MemBlock_t));
    
    // 与前一个块合并
    if (/* 前一个块是空闲的 */) {
        prev_block->block_size += GET_SIZE(block->block_size);
        block = prev_block;
    }
    
    // 与后一个块合并  
    if (/* 后一个块是空闲的 */) {
        block->block_size += GET_SIZE(next_block->block_size);
        // 从空闲链表移除后一个块
    }
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

## 优化策略

### 1. 内存池大小调优
```c
// 根据应用特点调整内存池大小
#define MEMPOOL_SIZE (8192)  // 增加到8KB适合大型应用
```

### 2. 对齐优化
```c
// 64位系统使用8字节对齐
#define MEMPOOL_ALIGNMENT (8)
// 32位系统使用4字节对齐  
#define MEMPOOL_ALIGNMENT (4)
```

### 3. 预分配策略
```c
// 系统启动时预分配常用大小的块
void prealloc_common_blocks(void) {
    void *blocks[10];
    for (int i = 0; i < 10; i++) {
        blocks[i] = EK_pMemPool_Malloc(64);  // 预分配64字节块
    }
    // 立即释放，形成合适大小的空闲块
    for (int i = 0; i < 10; i++) {
        EK_bMemPool_Free(blocks[i]);
    }
}
```

## 注意事项

1. **线程安全**：模块本身不提供线程同步，多线程环境需要外部保护
2. **内存对齐**：分配的内存地址会按MEMPOOL_ALIGNMENT对齐
3. **大小限制**：单次分配不能超过内存池总大小
4. **指针检查**：释放时会检查指针有效性，防止野指针
5. **统计准确性**：统计信息实时更新，可用于内存泄漏检测

该内存池模块为嵌入式系统提供了高效、可靠的动态内存管理方案，特别适用于需要频繁分配释放小块内存的应用场景。通过合理的参数配置和使用策略，可以显著提高系统的内存利用率和运行效率。
