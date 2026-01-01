# L2_Core (核心库层)

## 1. 简介
L2_Core 是本项目的核心库层，提供与硬件无关的基础功能模块。

向下：不依赖任何硬件层（L0、L1），纯软件实现。

向上：为 L3~L5 层提供通用的数据结构和服务功能（如日志系统、轻量级 printf、动态内存管理等）。

本层的设计目标是**高度可移植**，所有代码都可以独立运行，无需任何硬件支持。

## 2. 目录结构
```text
L2_Core
├── CMakeLists.txt          # 构建脚本
├── inc                     # [对外接口] 纯净的头文件
│   ├── ek_log.h           # 日志系统
│   ├── ek_lwprintf.h      # 轻量级 printf
│   ├── ek_mem.h           # 动态内存管理
│   └── ek_dlist.h         # 双向链表
└── src                     # [内部实现]
    ├── ek_log.c
    ├── ek_lwprintf.c
    ├── ek_mem.c
    └── ek_dlist.c
```

## 3. 核心开发原则 (Strict Rules)
为了保证架构的整洁性和可移植性，开发本层时必须遵守以下规则：

**硬件无关性 (Hardware Independent)** ：

禁止在本层中包含任何与硬件相关的头文件（如 stm32xxxx.h、gpio.h 等）。

禁止直接操作寄存器或调用硬件相关函数。

**可移植性优先 (Portability First)** ：

只使用标准 C 语言库（stdint.h、stdbool.h、string.h 等）。

避免使用编译器特定的扩展语法。

**纯 C 语言实现**：

不依赖 C++ 特性，不使用外部第三方库。

代码应能在任何支持 C99 的编译器上编译通过。

## 4. 开发指南

### 步骤 1：定义接口 (inc/ek_xxx.h)
定义模块对外的 API，保持接口简洁清晰。

```c
// inc/ek_dlist.h
#ifndef __README_H
#define __README_H

#include <stdint.h>
#include <stdbool.h>

// 双向链表节点
typedef struct ek_dlist_node
{
    struct ek_dlist_node *prev;
    struct ek_dlist_node *next;
} ek_dlist_node_t;

// 双向链表头
typedef struct
{
    ek_dlist_node_t *head;
    ek_dlist_node_t *tail;
    uint32_t size;
} ek_dlist_t;

// API 函数
void ek_dlist_init(ek_dlist_t *list);
void ek_dlist_push_back(ek_dlist_t *list, ek_dlist_node_t *node);
void ek_dlist_push_front(ek_dlist_t *list, ek_dlist_node_t *node);
ek_dlist_node_t* ek_dlist_pop_front(ek_dlist_t *list);
bool ek_dlist_is_empty(const ek_dlist_t *list);

#endif/* __README_H */
```

### 步骤 2：实现接口 (src/ek_xxx.c)
使用纯 C 语言实现功能逻辑。

```c
// src/ek_dlist.c
#include "ek_dlist.h"
#include <string.h>

void ek_dlist_init(ek_dlist_t *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void ek_dlist_push_back(ek_dlist_t *list, ek_dlist_node_t *node)
{
    if (list->tail == NULL)
    {
        // 空链表
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else
    {
        // 插入尾部
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}
```

### 步骤 3：使用日志系统
本层提供了 `ek_log` 模块，支持分级日志输出。

```c
#include "ek_log.h"

void example_function(void)
{
    // 不同级别的日志
    ek_log_debug("调试信息: x=%d", 42);
    ek_log_info("普通信息: 系统启动");
    ek_log_warn("警告信息: 内存不足");
    ek_log_error("错误信息: 打开失败");
}
```

### 步骤 4：使用 lwprintf
本层提供了 `ek_lwprintf` 轻量级格式化输出，可作为标准 printf 的替代。

```c
#include "ek_lwprintf.h"

// 自定义输出函数（如发送到 UART）
static void uart_output(const char *str, uint32_t len)
{
    // 发送到串口
    hal_uart_send((uint8_t*)str, len);
}

void example_printf(void)
{
    // 注册输出函数
    ek_lwprintf_set_output(uart_output);

    // 使用 lwprintf 输出
    ek_lwprintf("温度: %d.%d C\n", 25, 5);
}
```

## 5. 常见问题

**Q: 为什么日志系统不直接调用 HAL_UART 发送？**

A: L2 层不能依赖 L1 层的硬件驱动。正确的做法是：L2 只提供日志接口，由上层（L3 或 L4）注册具体的输出函数。这样实现了依赖倒置。

**Q: 动态内存管理为什么要自己实现，不用标准库 malloc？**

A: 嵌入式系统中标准库的 malloc 有很多问题：碎片化、不确定性、占用内存大。自己实现可以更好地控制内存分配策略，甚至可以使用静态内存池替代堆内存。

**Q: 可以在 L2 层使用 C++ STL 吗？**

A: 不建议。STL 会带来较大的代码体积开销和运行时开销。嵌入式系统中，手写简单的数据结构通常更高效。
