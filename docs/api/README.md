# API 参考手册

## 概述

本文档提供 EmbedKit 库的完整 API 参考。所有函数都遵循统一的命名规范和错误处理机制。

## API 命名规范

```
ek_<module>_<action>_<object>
```

- `ek_` - 统一前缀，避免命名冲突
- `<module>` - 模块名称（如 pool, scheduler, queue）
- `<action>` - 动作（如 create, destroy, push, pop）
- `<object>` - 操作对象（可选）

示例：
- `ek_pool_create()` - 创建内存池
- `ek_queue_push_front()` - 队列前端插入
- `ek_scheduler_start()` - 启动调度器

## 错误处理

### 返回值约定

```c
// 成功/失败返回
#define EK_OK           0   // 成功
#define EK_ERROR       -1   // 一般错误
#define EK_ENOMEM      -2   // 内存不足
#define EK_EINVAL      -3   // 无效参数
#define EK_EBUSY       -4   // 资源忙
#define EK_ETIMEOUT    -5   // 超时
#define EK_ENOTSUP     -6   // 不支持
#define EK_EAGAIN      -7   // 重试

// 布尔返回
typedef enum {
    EK_FALSE = 0,
    EK_TRUE = 1
} ek_bool_t;
```

### 错误处理示例

```c
// 方式1：直接检查返回值
ek_pool_t* pool = ek_pool_create(buffer, size, block_size);
if (pool == NULL) {
    // 处理错误
    return EK_ENOMEM;
}

// 方式2：使用错误码
int result = ek_scheduler_create_task(&task, config);
if (result != EK_OK) {
    switch (result) {
        case EK_ENOMEM:
            // 内存不足
            break;
        case EK_EINVAL:
            // 参数无效
            break;
        default:
            // 其他错误
            break;
    }
}

// 方式3：获取详细错误信息
const char* error_msg = ek_get_error_string(result);
printf("Error: %s\n", error_msg);
```

## 核心 API 模块

### 1. [任务调度器 API](scheduler.md)

任务管理和调度相关接口：

```c
// 初始化和控制
void ek_scheduler_init(void);
void ek_scheduler_start(void);
void ek_scheduler_stop(void);

// 任务管理
ek_task_t* ek_task_create(task_func_t func, void* param, uint32_t period, uint8_t priority);
void ek_task_delete(ek_task_t* task);
void ek_task_suspend(ek_task_t* task);
void ek_task_resume(ek_task_t* task);

// 定时器
ek_timer_t* ek_timer_create(timer_func_t func, void* param, uint32_t period, bool repeat);
void ek_timer_start(ek_timer_t* timer);
void ek_timer_stop(ek_timer_t* timer);
```

[查看完整文档 →](scheduler.md)

### 2. [内存管理 API](memory.md)

内存池和堆管理接口：

```c
// 内存池
ek_pool_t* ek_pool_create(void* buffer, size_t size, size_t block_size);
void* ek_pool_alloc(ek_pool_t* pool);
void ek_pool_free(ek_pool_t* pool, void* block);
void ek_pool_destroy(ek_pool_t* pool);

// 堆管理
void* ek_malloc(size_t size);
void* ek_calloc(size_t num, size_t size);
void* ek_realloc(void* ptr, size_t size);
void ek_free(void* ptr);
```

[查看完整文档 →](memory.md)

### 3. [数据结构 API](data_structures.md)

常用数据结构接口：

```c
// 链表
void ek_list_init(ek_list_t* list);
void ek_list_push_front(ek_list_t* list, ek_list_node_t* node);
void ek_list_push_back(ek_list_t* list, ek_list_node_t* node);
ek_list_node_t* ek_list_pop_front(ek_list_t* list);

// 队列
void ek_queue_init(ek_queue_t* queue, void* buffer, size_t item_size, size_t count);
bool ek_queue_push(ek_queue_t* queue, const void* item);
bool ek_queue_pop(ek_queue_t* queue, void* item);

// 环形缓冲
void ek_ring_init(ek_ring_t* ring, uint8_t* buffer, size_t size);
bool ek_ring_put(ek_ring_t* ring, uint8_t data);
bool ek_ring_get(ek_ring_t* ring, uint8_t* data);
```

[查看完整文档 →](data_structures.md)

### 4. [工具函数 API](utils.md)

实用工具函数：

```c
// CRC 计算
uint16_t ek_crc16(const uint8_t* data, size_t len);
uint32_t ek_crc32(const uint8_t* data, size_t len);

// 调试输出
void ek_debug_print(const char* format, ...);
void ek_debug_dump(const void* data, size_t len);

// 断言
#define EK_ASSERT(expr) ...
#define EK_ASSERT_MSG(expr, msg) ...
```

[查看完整文档 →](utils.md)

## 平台相关 API

### 硬件抽象层 (HAL)

需要移植时实现的接口：

```c
// 系统初始化
void ek_port_init(void);

// 时钟相关
uint32_t ek_port_get_tick(void);
void ek_port_delay_ms(uint32_t ms);

// 临界区
void ek_port_enter_critical(void);
void ek_port_exit_critical(void);

// 低功耗
void ek_port_sleep(void);
void ek_port_deep_sleep(void);
```

[查看移植指南 →](../porting/hal.md)

## 配置宏

通过 `embedkit_config.h` 配置库的行为：

```c
// 功能开关
#define EK_USE_SCHEDULER        1
#define EK_USE_MEMORY_POOL      1
#define EK_USE_HEAP             0
#define EK_USE_DEBUG            1

// 参数配置
#define EK_MAX_TASKS            32
#define EK_TICK_RATE_HZ         1000
#define EK_MIN_STACK_SIZE       128

// 优化选项
#define EK_USE_INLINE           1
#define EK_USE_FAST_CODE        1
#define EK_OPTIMIZE_SIZE        0
```

## 类型定义

### 基础类型

```c
// 标准整数类型
typedef uint8_t  ek_uint8_t;
typedef uint16_t ek_uint16_t;
typedef uint32_t ek_uint32_t;
typedef int8_t   ek_int8_t;
typedef int16_t  ek_int16_t;
typedef int32_t  ek_int32_t;

// 布尔类型
typedef enum {
    EK_FALSE = 0,
    EK_TRUE = 1
} ek_bool_t;

// 大小类型
typedef size_t ek_size_t;
typedef ptrdiff_t ek_ptrdiff_t;

// 时间类型
typedef uint32_t ek_tick_t;
typedef uint32_t ek_time_ms_t;
```

### 句柄类型

```c
// 不透明句柄
typedef struct ek_task ek_task_t;
typedef struct ek_timer ek_timer_t;
typedef struct ek_pool ek_pool_t;
typedef struct ek_queue ek_queue_t;
typedef struct ek_mutex ek_mutex_t;
typedef struct ek_sem ek_sem_t;
```

## 宏定义

### 实用宏

```c
// 获取数组大小
#define EK_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

// 获取结构体成员偏移
#define EK_OFFSET_OF(type, member) ((size_t)&((type*)0)->member)

// 通过成员指针获取结构体指针
#define EK_CONTAINER_OF(ptr, type, member) \
    ((type*)((char*)(ptr) - EK_OFFSET_OF(type, member)))

// 最大/最小值
#define EK_MAX(a, b) ((a) > (b) ? (a) : (b))
#define EK_MIN(a, b) ((a) < (b) ? (a) : (b))

// 对齐
#define EK_ALIGN_UP(x, align) (((x) + (align) - 1) & ~((align) - 1))
#define EK_ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

// 位操作
#define EK_BIT(n) (1U << (n))
#define EK_BIT_SET(x, n) ((x) |= EK_BIT(n))
#define EK_BIT_CLEAR(x, n) ((x) &= ~EK_BIT(n))
#define EK_BIT_TEST(x, n) (((x) & EK_BIT(n)) != 0)
```

### 编译器相关

```c
// 内联
#ifdef __GNUC__
    #define EK_INLINE static inline __attribute__((always_inline))
#else
    #define EK_INLINE static inline
#endif

// 未使用参数
#define EK_UNUSED(x) ((void)(x))

// 弱符号
#define EK_WEAK __attribute__((weak))

// 对齐
#define EK_ALIGNED(x) __attribute__((aligned(x)))

// 打包
#define EK_PACKED __attribute__((packed))

// 节声明
#define EK_SECTION(x) __attribute__((section(x)))

// 不返回
#define EK_NORETURN __attribute__((noreturn))
```

## 使用示例

### 完整应用示例

```c
#include "embedkit.h"

// 配置
#define POOL_SIZE 1024
#define BLOCK_SIZE 64

// 全局资源
static uint8_t g_pool_buffer[POOL_SIZE];
static ek_pool_t* g_pool;

// 任务函数
void sensor_task(void* param) {
    sensor_data_t* data = ek_pool_alloc(g_pool);
    if (data) {
        read_sensor(data);
        process_data(data);
        ek_pool_free(g_pool, data);
    }
}

int main(void) {
    // 初始化硬件
    ek_port_init();
    
    // 创建内存池
    g_pool = ek_pool_create(g_pool_buffer, POOL_SIZE, BLOCK_SIZE);
    
    // 初始化调度器
    ek_scheduler_init();
    
    // 创建任务
    ek_task_create(sensor_task, NULL, 100, TASK_PRIORITY_NORMAL);
    
    // 启动调度器
    ek_scheduler_start();
    
    // 不会到达
    return 0;
}
```

## API 版本兼容性

### 版本宏

```c
#define EK_VERSION_MAJOR 1
#define EK_VERSION_MINOR 0
#define EK_VERSION_PATCH 0

#define EK_VERSION_NUMBER \
    (EK_VERSION_MAJOR * 10000 + \
     EK_VERSION_MINOR * 100 + \
     EK_VERSION_PATCH)

// 版本检查
#if EK_VERSION_NUMBER < 10000
    #error "EmbedKit version 1.0.0 or higher required"
#endif
```

### 废弃 API

```c
// 使用编译器属性标记废弃 API
#define EK_DEPRECATED(msg) __attribute__((deprecated(msg)))

// 示例
EK_DEPRECATED("Use ek_task_create() instead")
ek_task_t* ek_create_task(void);
```

## 调试支持

### 运行时检查

```c
// 启用运行时检查
#define EK_ENABLE_CHECKS 1

// API 参数检查
#if EK_ENABLE_CHECKS
    #define EK_CHECK_PARAM(expr) \
        do { \
            if (!(expr)) { \
                ek_error_handler(__FILE__, __LINE__); \
                return EK_EINVAL; \
            } \
        } while(0)
#else
    #define EK_CHECK_PARAM(expr)
#endif
```

### 统计信息

```c
// 获取运行时统计
typedef struct {
    uint32_t alloc_count;
    uint32_t free_count;
    uint32_t peak_usage;
    uint32_t current_usage;
} ek_stats_t;

void ek_get_stats(ek_stats_t* stats);
void ek_reset_stats(void);
```

## 相关链接

- [快速开始指南](../getting_started/first_program.md)
- [配置选项详解](../getting_started/build_config.md)
- [示例代码](../examples/README.md)
- [最佳实践](../best_practices/README.md)