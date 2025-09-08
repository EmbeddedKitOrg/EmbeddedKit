# 数据结构模块

嵌入式系统优化的数据结构实现集合。

## 功能特性

- 🚀 **高性能**: 针对嵌入式环境优化的算法
- 💾 **内存友好**: 最小化内存使用和碎片化
- 🔧 **可配置**: 支持编译时配置各种参数
- 🛡️ **类型安全**: 使用C宏实现泛型数据结构

## 包含的数据结构

### 链表 (Linked List)
- 单向链表
- 双向链表
- 循环链表

### 队列 (Queue)
- 环形队列
- 优先级队列
- 线程安全队列

### 栈 (Stack)
- 动态栈
- 固定大小栈

### 哈希表 (Hash Table)
- 开放寻址法
- 链式哈希
- 自动扩容

## API示例

### 动态数组
```c
#include "dynamic_array.h"

// 创建int类型的动态数组
DECLARE_DYNAMIC_ARRAY(int_array, int);

int main() {
    int_array_t arr;
    int_array_init(&arr, 10);
    
    // 添加元素
    int_array_push(&arr, 42);
    int_array_push(&arr, 24);
    
    // 访问元素
    int value = int_array_get(&arr, 0); // 42
    
    // 清理
    int_array_destroy(&arr);
    return 0;
}
```

### 链表操作
```c
#include "linked_list.h"

typedef struct {
    int data;
    list_node_t node;
} int_node_t;

int main() {
    list_head_t list;
    list_init(&list);
    
    // 添加节点
    int_node_t* node = malloc(sizeof(int_node_t));
    node->data = 100;
    list_add(&list, &node->node);
    
    // 遍历
    list_node_t* pos;
    list_for_each(pos, &list) {
        int_node_t* entry = container_of(pos, int_node_t, node);
        printf("data: %d\n", entry->data);
    }
    
    return 0;
}
```

## 性能特征

| 数据结构 | 插入 | 删除 | 查找 | 空间复杂度 |
|----------|------|------|------|------------|
| 动态数组 | O(1)* | O(n) | O(1) | O(n) |
| 链表 | O(1) | O(1) | O(n) | O(n) |
| 哈希表 | O(1)* | O(1)* | O(1)* | O(n) |
| 优先级队列 | O(log n) | O(log n) | O(1) | O(n) |

*平均情况下的时间复杂度