# 基础示例

这里包含EmbedKit的基本使用示例，适合初学者快速上手。

## 示例列表

### 1. Hello World
最简单的EmbedKit使用示例。

```c
#include "embedkit.h"

int main() {
    printf("Hello, EmbedKit!\n");
    return 0;
}
```

### 2. 内存管理示例
演示如何使用内存管理模块。

```c
#include "memory.h"
#include <stdio.h>

int main() {
    // 初始化内存管理
    if (memory_init(1024) != 0) {
        printf("内存初始化失败\n");
        return -1;
    }
    
    printf("内存管理初始化成功\n");
    printf("可用内存: %zu bytes\n", memory_get_free_size());
    
    // 分配内存
    char* buffer = memory_alloc(256);
    if (buffer) {
        printf("分配256字节内存成功\n");
        printf("剩余内存: %zu bytes\n", memory_get_free_size());
        
        // 使用内存
        sprintf(buffer, "这是一个测试字符串");
        printf("写入数据: %s\n", buffer);
        
        // 释放内存
        memory_free(buffer);
        printf("内存释放完成\n");
        printf("剩余内存: %zu bytes\n", memory_get_free_size());
    }
    
    // 清理
    memory_deinit();
    return 0;
}
```

### 3. 链表操作示例
展示链表数据结构的基本用法。

```c
#include "data_structures/linked_list.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int value;
    list_node_t node;
} int_node_t;

int main() {
    list_head_t list;
    list_init(&list);
    
    printf("创建链表并添加节点\n");
    
    // 添加节点
    for (int i = 1; i <= 5; i++) {
        int_node_t* new_node = malloc(sizeof(int_node_t));
        new_node->value = i * 10;
        list_add_tail(&list, &new_node->node);
        printf("添加节点: %d\n", new_node->value);
    }
    
    printf("遍历链表:\n");
    
    // 遍历链表
    list_node_t* pos;
    list_for_each(pos, &list) {
        int_node_t* entry = container_of(pos, int_node_t, node);
        printf("  节点值: %d\n", entry->value);
    }
    
    // 删除节点
    printf("删除第一个节点\n");
    if (!list_empty(&list)) {
        list_node_t* first = list.next;
        int_node_t* entry = container_of(first, int_node_t, node);
        list_del(first);
        free(entry);
    }
    
    printf("删除后的链表:\n");
    list_for_each(pos, &list) {
        int_node_t* entry = container_of(pos, int_node_t, node);
        printf("  节点值: %d\n", entry->value);
    }
    
    return 0;
}
```

### 4. 简单任务调度示例
基础的多任务调度演示。

```c
#include "scheduler.h"
#include <stdio.h>

void led_blink_task(void* arg) {
    int led_id = *(int*)arg;
    
    while (1) {
        printf("LED %d: ON\n", led_id);
        task_delay(500);
        
        printf("LED %d: OFF\n", led_id);
        task_delay(500);
    }
}

void sensor_read_task(void* arg) {
    int count = 0;
    
    while (1) {
        printf("读取传感器数据: %d\n", count++);
        task_delay(2000);
    }
}

int main() {
    printf("启动任务调度器\n");
    
    // 初始化调度器
    scheduler_init();
    
    // 创建LED闪烁任务
    int led1_id = 1;
    int led2_id = 2;
    
    task_t* led1_task = task_create(led_blink_task, &led1_id, 1, 512);
    task_t* led2_task = task_create(led_blink_task, &led2_id, 1, 512);
    
    // 创建传感器读取任务
    task_t* sensor_task = task_create(sensor_read_task, NULL, 2, 512);
    
    // 启动所有任务
    task_start(led1_task);
    task_start(led2_task);
    task_start(sensor_task);
    
    printf("所有任务已启动，开始调度\n");
    
    // 开始调度
    scheduler_start();
    
    return 0;
}
```

## 编译说明

使用以下命令编译示例：

```bash
gcc -I../../include -I../../src basic_example.c -o basic_example
```

## 运行环境

- GCC 4.8+
- 支持POSIX的系统 (Linux, macOS)
- Windows (使用MinGW)