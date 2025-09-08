# 内存管理模块

轻量级的内存管理工具，适用于嵌入式系统。

## 功能特性

- 📦 **内存池管理**: 预分配内存池，避免碎片化
- 🔒 **线程安全**: 支持多线程环境下的内存操作  
- 📊 **使用统计**: 内置内存使用情况监控
- ⚡ **高性能**: 优化的分配算法，适合实时系统

## API接口

### 初始化
```c
int memory_init(size_t pool_size);
void memory_deinit(void);
```

### 内存分配
```c
void* memory_alloc(size_t size);
void memory_free(void* ptr);
void* memory_realloc(void* ptr, size_t new_size);
```

### 状态查询
```c
size_t memory_get_free_size(void);
size_t memory_get_used_size(void);
int memory_get_fragment_count(void);
```

## 使用示例

```c
#include "memory.h"

int main() {
    // 初始化1MB内存池
    if (memory_init(1024 * 1024) != 0) {
        return -1;
    }
    
    // 分配内存
    char* buffer = memory_alloc(256);
    if (buffer != NULL) {
        // 使用内存...
        memory_free(buffer);
    }
    
    // 清理资源
    memory_deinit();
    return 0;
}
```

## 配置选项

可通过宏定义配置模块行为：

- `MEMORY_POOL_SIZE`: 默认内存池大小
- `MEMORY_ALIGNMENT`: 内存对齐字节数 
- `MEMORY_DEBUG`: 启用调试模式
- `MEMORY_THREAD_SAFE`: 启用线程安全