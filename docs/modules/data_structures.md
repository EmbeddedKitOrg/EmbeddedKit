# 数据结构模块

## 概述

EmbedKit提供了一套针对嵌入式系统优化的基础数据结构，包括链表、队列、栈和环形缓冲区。所有数据结构都采用静态内存分配，具有可预测的执行时间。

## 模块特性

- ✅ **零动态分配** - 所有内存静态分配
- ✅ **中断安全** - 支持在中断中使用
- ✅ **线程安全选项** - 可选的互斥保护
- ✅ **最小化内存占用** - 紧凑的数据结构
- ✅ **类型安全** - 泛型宏实现
- ✅ **侵入式设计** - 高效的内存利用

## 链表 (List)

### 单向链表

```c
#include "embedkit/data_structures/list.h"

// 定义节点结构
typedef struct {
    ek_slist_node_t node;  // 链表节点（必须是第一个成员）
    int data;
    char name[32];
} my_node_t;

// 创建链表
ek_slist_t list;
ek_slist_init(&list);

// 添加节点
my_node_t node1 = { .data = 100, .name = "Node1" };
ek_slist_push_front(&list, &node1.node);

my_node_t node2 = { .data = 200, .name = "Node2" };
ek_slist_push_back(&list, &node2.node);

// 遍历链表
ek_slist_node_t* iter;
ek_slist_foreach(&list, iter) {
    my_node_t* item = EK_CONTAINER_OF(iter, my_node_t, node);
    printf("Data: %d, Name: %s\n", item->data, item->name);
}

// 查找节点
my_node_t* find_node(ek_slist_t* list, int data) {
    ek_slist_node_t* iter;
    ek_slist_foreach(list, iter) {
        my_node_t* item = EK_CONTAINER_OF(iter, my_node_t, node);
        if (item->data == data) {
            return item;
        }
    }
    return NULL;
}

// 删除节点
my_node_t* target = find_node(&list, 100);
if (target) {
    ek_slist_remove(&list, &target->node);
}
```

### 双向链表

```c
// 双向链表节点
typedef struct {
    ek_dlist_node_t node;
    uint32_t id;
    void* payload;
} task_node_t;

// 初始化
ek_dlist_t task_list;
ek_dlist_init(&task_list);

// 插入操作
task_node_t task1, task2, task3;
ek_dlist_push_front(&task_list, &task1.node);
ek_dlist_push_back(&task_list, &task2.node);
ek_dlist_insert_after(&task1.node, &task3.node);

// 双向遍历
ek_dlist_node_t* node;

// 正向遍历
ek_dlist_foreach(&task_list, node) {
    task_node_t* task = EK_CONTAINER_OF(node, task_node_t, node);
    process_task(task);
}

// 反向遍历
ek_dlist_foreach_reverse(&task_list, node) {
    task_node_t* task = EK_CONTAINER_OF(node, task_node_t, node);
    process_task_reverse(task);
}

// 安全遍历（允许删除当前节点）
ek_dlist_node_t* next;
ek_dlist_foreach_safe(&task_list, node, next) {
    task_node_t* task = EK_CONTAINER_OF(node, task_node_t, node);
    if (should_remove(task)) {
        ek_dlist_remove(node);
        free_task(task);
    }
}
```

### 循环链表

```c
// 创建循环链表（用于轮询调度）
typedef struct {
    ek_dlist_node_t node;
    void (*handler)(void);
    uint32_t period;
    uint32_t next_run;
} scheduler_entry_t;

ek_dlist_t scheduler;
ek_dlist_init(&scheduler);

// 添加调度项
scheduler_entry_t entries[5];
for (int i = 0; i < 5; i++) {
    entries[i].handler = handlers[i];
    entries[i].period = periods[i];
    ek_dlist_push_back(&scheduler, &entries[i].node);
}

// 使链表循环
ek_dlist_make_circular(&scheduler);

// 轮询执行
ek_dlist_node_t* current = scheduler.head;
while (1) {
    scheduler_entry_t* entry = EK_CONTAINER_OF(current, scheduler_entry_t, node);
    
    if (get_tick() >= entry->next_run) {
        entry->handler();
        entry->next_run = get_tick() + entry->period;
    }
    
    current = current->next;
    if (current == scheduler.head) {
        // 完成一轮，可以进入低功耗
        enter_sleep();
    }
}
```

## 队列 (Queue)

### 基础FIFO队列

```c
#include "embedkit/data_structures/queue.h"

// 定义队列和缓冲区
#define QUEUE_SIZE 32
typedef struct {
    uint8_t cmd;
    uint16_t param;
} command_t;

static command_t queue_buffer[QUEUE_SIZE];
static ek_queue_t cmd_queue;

// 初始化队列
ek_queue_init(&cmd_queue, queue_buffer, sizeof(command_t), QUEUE_SIZE);

// 生产者（如中断处理）
void uart_isr(void) {
    command_t cmd = {
        .cmd = UART_RX_REG,
        .param = get_timestamp()
    };
    
    if (!ek_queue_push(&cmd_queue, &cmd)) {
        // 队列满，处理溢出
        queue_overflow_count++;
    }
}

// 消费者（主循环）
void process_commands(void) {
    command_t cmd;
    
    while (ek_queue_pop(&cmd_queue, &cmd)) {
        switch (cmd.cmd) {
            case CMD_LED:
                toggle_led(cmd.param);
                break;
            case CMD_MOTOR:
                set_motor_speed(cmd.param);
                break;
            default:
                break;
        }
    }
}
```

### 优先级队列

```c
// 优先级队列节点
typedef struct {
    uint8_t priority;
    void* data;
    size_t size;
} prio_item_t;

// 比较函数
int prio_compare(const void* a, const void* b) {
    const prio_item_t* item_a = (const prio_item_t*)a;
    const prio_item_t* item_b = (const prio_item_t*)b;
    return item_b->priority - item_a->priority;  // 高优先级优先
}

// 创建优先级队列
static prio_item_t prio_buffer[16];
static ek_pqueue_t prio_queue;

ek_pqueue_init(&prio_queue, prio_buffer, sizeof(prio_item_t), 16, prio_compare);

// 插入不同优先级的项
prio_item_t high_prio = { .priority = 10, .data = &high_task };
prio_item_t low_prio = { .priority = 1, .data = &low_task };

ek_pqueue_push(&prio_queue, &high_prio);
ek_pqueue_push(&prio_queue, &low_prio);

// 按优先级处理
prio_item_t item;
while (ek_pqueue_pop(&prio_queue, &item)) {
    process_by_priority(item.data, item.priority);
}
```

### 线程安全队列

```c
// 带互斥保护的队列
typedef struct {
    ek_queue_t queue;
    ek_mutex_t mutex;
    ek_sem_t items_available;
} safe_queue_t;

// 初始化线程安全队列
void safe_queue_init(safe_queue_t* sq, void* buffer, size_t item_size, size_t count) {
    ek_queue_init(&sq->queue, buffer, item_size, count);
    ek_mutex_init(&sq->mutex);
    ek_sem_init(&sq->items_available, 0);
}

// 线程安全的入队
bool safe_queue_push(safe_queue_t* sq, const void* item) {
    bool result;
    
    ek_mutex_lock(&sq->mutex);
    result = ek_queue_push(&sq->queue, item);
    ek_mutex_unlock(&sq->mutex);
    
    if (result) {
        ek_sem_post(&sq->items_available);
    }
    
    return result;
}

// 线程安全的出队（带超时）
bool safe_queue_pop(safe_queue_t* sq, void* item, uint32_t timeout_ms) {
    if (ek_sem_wait(&sq->items_available, timeout_ms) != EK_OK) {
        return false;  // 超时
    }
    
    ek_mutex_lock(&sq->mutex);
    bool result = ek_queue_pop(&sq->queue, item);
    ek_mutex_unlock(&sq->mutex);
    
    return result;
}
```

## 栈 (Stack)

### 基础栈操作

```c
#include "embedkit/data_structures/stack.h"

// 定义栈
#define STACK_SIZE 64
static int stack_buffer[STACK_SIZE];
static ek_stack_t stack;

// 初始化
ek_stack_init(&stack, stack_buffer, sizeof(int), STACK_SIZE);

// 压栈
for (int i = 0; i < 10; i++) {
    if (!ek_stack_push(&stack, &i)) {
        printf("Stack overflow at %d\n", i);
        break;
    }
}

// 出栈
int value;
while (ek_stack_pop(&stack, &value)) {
    printf("Popped: %d\n", value);
}

// 查看栈顶（不弹出）
if (ek_stack_peek(&stack, &value)) {
    printf("Top element: %d\n", value);
}
```

### 表达式求值栈

```c
// 使用栈实现后缀表达式求值
typedef struct {
    enum { NUMBER, OPERATOR } type;
    union {
        double number;
        char op;
    } value;
} token_t;

double evaluate_postfix(token_t* tokens, int count) {
    static double stack_buffer[32];
    ek_stack_t stack;
    ek_stack_init(&stack, stack_buffer, sizeof(double), 32);
    
    for (int i = 0; i < count; i++) {
        if (tokens[i].type == NUMBER) {
            ek_stack_push(&stack, &tokens[i].value.number);
        } else {
            double b, a;
            ek_stack_pop(&stack, &b);
            ek_stack_pop(&stack, &a);
            
            double result;
            switch (tokens[i].value.op) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': result = a / b; break;
            }
            
            ek_stack_push(&stack, &result);
        }
    }
    
    double final_result;
    ek_stack_pop(&stack, &final_result);
    return final_result;
}
```

## 环形缓冲区 (Ring Buffer)

### 基础环形缓冲

```c
#include "embedkit/data_structures/ring_buffer.h"

// 定义环形缓冲区
#define RING_SIZE 256
static uint8_t ring_buffer[RING_SIZE];
static ek_ring_t uart_ring;

// 初始化
ek_ring_init(&uart_ring, ring_buffer, RING_SIZE);

// UART接收中断
void uart_rx_isr(void) {
    uint8_t data = UART_RX_REG;
    if (!ek_ring_put(&uart_ring, data)) {
        // 缓冲区满
        rx_overflow_flag = 1;
    }
}

// 主循环处理
void process_uart_data(void) {
    uint8_t data;
    while (ek_ring_get(&uart_ring, &data)) {
        process_byte(data);
    }
}

// 批量操作
void process_uart_burst(void) {
    uint8_t buffer[64];
    size_t count = ek_ring_get_multiple(&uart_ring, buffer, 64);
    if (count > 0) {
        process_bytes(buffer, count);
    }
}
```

### 无锁环形缓冲（单生产者-单消费者）

```c
// 无锁环形缓冲区（SPSC）
typedef struct {
    uint8_t* buffer;
    size_t size;
    volatile size_t head;  // 生产者写
    volatile size_t tail;  // 消费者写
} spsc_ring_t;

// 生产者函数（中断中调用）
bool spsc_put(spsc_ring_t* ring, uint8_t data) {
    size_t next_head = (ring->head + 1) % ring->size;
    
    if (next_head == ring->tail) {
        return false;  // 缓冲区满
    }
    
    ring->buffer[ring->head] = data;
    
    // 内存屏障，确保数据写入后才更新head
    __DMB();
    
    ring->head = next_head;
    return true;
}

// 消费者函数（主循环调用）
bool spsc_get(spsc_ring_t* ring, uint8_t* data) {
    if (ring->tail == ring->head) {
        return false;  // 缓冲区空
    }
    
    *data = ring->buffer[ring->tail];
    
    // 内存屏障
    __DMB();
    
    ring->tail = (ring->tail + 1) % ring->size;
    return true;
}
```

### DMA友好的环形缓冲

```c
// DMA环形缓冲配置
typedef struct {
    uint8_t* buffer;
    size_t size;
    volatile size_t write_idx;  // DMA写指针
    size_t read_idx;           // CPU读指针
    DMA_HandleTypeDef* hdma;
} dma_ring_t;

// 初始化DMA环形缓冲
void dma_ring_init(dma_ring_t* ring, uint8_t* buffer, size_t size) {
    ring->buffer = buffer;
    ring->size = size;
    ring->write_idx = 0;
    ring->read_idx = 0;
    
    // 配置DMA循环模式
    HAL_DMA_Start_IT(ring->hdma, 
                     (uint32_t)&UART->DR,
                     (uint32_t)ring->buffer,
                     ring->size);
}

// 读取DMA缓冲区数据
size_t dma_ring_read(dma_ring_t* ring, uint8_t* dest, size_t max_len) {
    // 获取DMA当前写位置
    ring->write_idx = ring->size - __HAL_DMA_GET_COUNTER(ring->hdma);
    
    size_t available = 0;
    if (ring->write_idx >= ring->read_idx) {
        available = ring->write_idx - ring->read_idx;
    } else {
        available = ring->size - ring->read_idx + ring->write_idx;
    }
    
    size_t to_read = (available < max_len) ? available : max_len;
    
    for (size_t i = 0; i < to_read; i++) {
        dest[i] = ring->buffer[ring->read_idx];
        ring->read_idx = (ring->read_idx + 1) % ring->size;
    }
    
    return to_read;
}
```

## 高级数据结构

### 位图 (Bitmap)

```c
// 位图实现
typedef struct {
    uint32_t* bits;
    size_t size;  // 位数
} ek_bitmap_t;

// 初始化位图
void ek_bitmap_init(ek_bitmap_t* bm, uint32_t* buffer, size_t bits) {
    bm->bits = buffer;
    bm->size = bits;
    memset(buffer, 0, (bits + 31) / 32 * sizeof(uint32_t));
}

// 设置位
void ek_bitmap_set(ek_bitmap_t* bm, size_t pos) {
    if (pos < bm->size) {
        bm->bits[pos / 32] |= (1U << (pos % 32));
    }
}

// 清除位
void ek_bitmap_clear(ek_bitmap_t* bm, size_t pos) {
    if (pos < bm->size) {
        bm->bits[pos / 32] &= ~(1U << (pos % 32));
    }
}

// 测试位
bool ek_bitmap_test(ek_bitmap_t* bm, size_t pos) {
    if (pos < bm->size) {
        return (bm->bits[pos / 32] & (1U << (pos % 32))) != 0;
    }
    return false;
}

// 查找第一个置位的位
int ek_bitmap_find_first_set(ek_bitmap_t* bm) {
    for (size_t i = 0; i < (bm->size + 31) / 32; i++) {
        if (bm->bits[i] != 0) {
            return i * 32 + __builtin_ctz(bm->bits[i]);
        }
    }
    return -1;
}
```

### 哈希表 (简单实现)

```c
// 简单哈希表（链式解决冲突）
typedef struct hash_node {
    uint32_t key;
    void* value;
    struct hash_node* next;
} hash_node_t;

typedef struct {
    hash_node_t** buckets;
    size_t bucket_count;
    hash_node_t* node_pool;
    size_t pool_size;
    size_t pool_used;
} ek_hashtable_t;

// 哈希函数
static uint32_t hash_func(uint32_t key, size_t bucket_count) {
    return (key * 2654435761U) % bucket_count;
}

// 插入键值对
bool ek_hashtable_insert(ek_hashtable_t* ht, uint32_t key, void* value) {
    uint32_t index = hash_func(key, ht->bucket_count);
    
    // 检查是否已存在
    hash_node_t* node = ht->buckets[index];
    while (node) {
        if (node->key == key) {
            node->value = value;  // 更新值
            return true;
        }
        node = node->next;
    }
    
    // 分配新节点
    if (ht->pool_used >= ht->pool_size) {
        return false;  // 池已满
    }
    
    hash_node_t* new_node = &ht->node_pool[ht->pool_used++];
    new_node->key = key;
    new_node->value = value;
    new_node->next = ht->buckets[index];
    ht->buckets[index] = new_node;
    
    return true;
}

// 查找值
void* ek_hashtable_find(ek_hashtable_t* ht, uint32_t key) {
    uint32_t index = hash_func(key, ht->bucket_count);
    hash_node_t* node = ht->buckets[index];
    
    while (node) {
        if (node->key == key) {
            return node->value;
        }
        node = node->next;
    }
    
    return NULL;
}
```

## 性能对比

| 数据结构 | 插入 | 删除 | 查找 | 遍历 | 内存开销 |
|----------|------|------|------|------|----------|
| 单向链表 | O(1)* | O(n) | O(n) | O(n) | 1指针/节点 |
| 双向链表 | O(1)* | O(1)** | O(n) | O(n) | 2指针/节点 |
| 队列 | O(1) | O(1) | N/A | O(n) | 固定数组 |
| 栈 | O(1) | O(1) | N/A | O(n) | 固定数组 |
| 环形缓冲 | O(1) | O(1) | N/A | O(n) | 固定数组 |
| 位图 | O(1) | O(1) | O(1) | O(n) | 1位/元素 |

*在头部插入  
**已知节点指针

## 使用建议

### 选择合适的数据结构

1. **频繁头部操作** → 单向链表
2. **需要双向遍历** → 双向链表
3. **FIFO场景** → 队列或环形缓冲
4. **LIFO场景** → 栈
5. **中断与主循环通信** → 环形缓冲（无锁）
6. **资源管理** → 位图
7. **快速查找** → 哈希表

### 内存布局优化

```c
// 缓存友好的节点布局
typedef struct {
    // 热数据（频繁访问）
    struct {
        void* next;
        uint32_t key;
        uint8_t flags;
    } __attribute__((packed)) hot;
    
    // 冷数据（不常访问）
    struct {
        uint32_t created_time;
        uint32_t modified_time;
        char description[64];
    } cold;
} optimized_node_t;
```

## 配置选项

```c
// embedkit_config.h

// 数据结构配置
#define EK_DS_USE_INLINE        1   // 使用内联函数
#define EK_DS_USE_THREAD_SAFE   0   // 线程安全（增加开销）
#define EK_DS_USE_STATISTICS    1   // 启用统计
#define EK_DS_USE_ASSERTIONS    1   // 启用断言检查

// 类型安全宏
#define EK_DS_TYPE_SAFE         1   // 类型安全检查

// 性能优化
#define EK_DS_CACHE_ALIGN       1   // 缓存行对齐
#define EK_DS_PREFETCH          1   // 启用预取指令
```

## 相关链接

- [数据结构API参考](../api/data_structures.md)
- [内存管理](memory.md)
- [性能优化](../design/optimization.md)
- [示例代码](../examples/data_structures_examples.md)