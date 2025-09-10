# EK_List - 双向链表数据结构

## 概述

EK_List 是一个高效的双向链表数据结构实现，采用哨兵节点设计，提供完整的链表操作功能。该模块特别适用于嵌入式系统中需要动态数据管理的场景。

## 核心特性

- **双向链表结构**：每个节点都有前驱和后继指针，支持双向遍历
- **哨兵节点设计**：使用dummy节点简化边界条件处理，提高操作效率
- **节点所有权管理**：每个节点都记录其所属链表，支持跨链表操作
- **按序插入**：支持按节点序号自动排序插入
- **内存管理**：支持静态和动态两种内存分配方式

## 数据结构

### EK_Node_t - 节点结构体
```c
typedef struct EK_Node_t
{
    void *Node_Data;      // 节点存储内容指针
    struct EK_Node_t *Node_Prev; // 前一个节点指针
    struct EK_Node_t *Node_Next; // 后一个节点指针
    EK_List_t *Node_Owner;      // 节点所有者链表
    uint32_t Node_Order;     // 节点序号
} EK_Node_t;
```

### EK_List_t - 链表结构体
```c
typedef struct EK_List_t
{
    EK_Node_t *List_Dummy;      // 哨兵节点（Prev指向尾，Next指向头）
    uint16_t List_Count;     // 链表节点数量
} EK_List_t;
```

## API 接口

### 节点管理

#### 静态节点创建
```c
EK_Result_t EK_rNodeCreate_Static(EK_Node_t *node, void *content, uint32_t order);
```
- **功能**：在用户提供的内存上初始化节点
- **参数**：
  - `node`：已分配的节点内存指针
  - `content`：节点存储的内容指针
  - `order`：节点序号
- **返回值**：操作结果状态码

#### 动态节点创建
```c
EK_Node_t *EK_pNodeCreate_Dynamic(void *content, uint32_t order);
```
- **功能**：动态分配内存并创建节点
- **参数**：
  - `content`：节点存储的内容指针
  - `order`：节点序号
- **返回值**：创建的节点指针，失败返回NULL

### 链表管理

#### 静态链表创建
```c
EK_Result_t EK_rListCreate_Static(EK_List_t *list, EK_Node_t *head_node);
```
- **功能**：在用户提供的内存上初始化链表
- **参数**：
  - `list`：已分配的链表内存指针
  - `head_node`：初始头节点
- **返回值**：操作结果状态码

#### 动态链表创建
```c
EK_List_t *EK_pListCreate_Dynamic(EK_Node_t *head_node);
```
- **功能**：动态分配内存并创建链表
- **参数**：
  - `head_node`：初始头节点
- **返回值**：创建的链表指针，失败返回NULL

### 节点插入

#### 尾部插入
```c
EK_Result_t EK_rListInsertEnd(EK_List_t *list, EK_Node_t *node);
```
- **功能**：在链表尾部插入节点
- **时间复杂度**：O(1)

#### 头部插入
```c
EK_Result_t EK_rListInsertHead(EK_List_t *list, EK_Node_t *node);
```
- **功能**：在链表头部插入节点
- **时间复杂度**：O(1)

#### 按序插入
```c
EK_Result_t EK_rListInsertOrder(EK_List_t *list, EK_Node_t *node);
```
- **功能**：按节点序号有序插入
- **排序规则**：序号小的在前
- **时间复杂度**：O(n)

### 节点删除和移动

#### 节点删除
```c
EK_Result_t EK_rListRemoveNode(EK_List_t *list, EK_Node_t *node);
```
- **功能**：从链表中移除指定节点
- **特性**：自动处理头尾节点特殊情况

#### 节点移动
```c
EK_Result_t EK_rListMoveNode(EK_List_t *list_src, EK_List_t *list_dst, EK_Node_t *node, int order);
```
- **功能**：将节点从源链表移动到目标链表
- **参数**：
  - `order`：插入方式（0=头部，<0=尾部，>0=按序插入）

## 使用场景

### 1. 任务调度队列
```c
// 创建任务队列
EK_List_t *ready_queue = EK_pListCreate_Dynamic(first_task_node);
EK_List_t *wait_queue = EK_pListCreate_Dynamic(wait_task_node);

// 任务状态切换
EK_rListMoveNode(wait_queue, ready_queue, task_node, -1); // 移到就绪队列尾部
```

### 2. 优先级管理
```c
// 创建优先级节点
EK_Node_t *high_priority_node = EK_pNodeCreate_Dynamic(task_data, 1);    // 高优先级
EK_Node_t *low_priority_node = EK_pNodeCreate_Dynamic(task_data, 10);    // 低优先级

// 按优先级插入
EK_rListInsertOrder(priority_list, high_priority_node);
EK_rListInsertOrder(priority_list, low_priority_node);
```

### 3. 事件管理系统
```c
// 事件节点
typedef struct {
    uint32_t event_id;
    uint32_t timestamp;
    void *event_data;
} Event_t;

Event_t event = {.event_id = 1, .timestamp = get_tick(), .event_data = data};
EK_Node_t *event_node = EK_pNodeCreate_Dynamic(&event, event.timestamp);

// 按时间戳有序插入事件队列
EK_rListInsertOrder(event_queue, event_node);
```

### 4. 资源池管理
```c
// 空闲资源链表
EK_List_t *free_buffers;
EK_List_t *used_buffers;

// 分配资源：从空闲链表移到使用链表
EK_rListMoveNode(free_buffers, used_buffers, buffer_node, -1);

// 释放资源：从使用链表移回空闲链表
EK_rListMoveNode(used_buffers, free_buffers, buffer_node, 0);
```

## 设计亮点

### 1. 哨兵节点优化
- 消除了空链表和边界条件的特殊处理
- 头尾操作统一，代码简洁
- Prev指向尾节点，Next指向头节点，形成环形结构

### 2. 节点所有权机制
- 每个节点记录所属链表，防止误操作
- 支持安全的跨链表移动
- 便于调试和问题定位

### 3. 灵活的内存管理
- 支持静态分配（编译时确定大小）
- 支持动态分配（运行时按需分配）
- 用户可选择最适合的内存管理方式

### 4. 高效的插入策略
- 头尾插入：O(1) 时间复杂度
- 按序插入：智能判断边界条件，减少遍历次数
- 自动维护排序顺序

## 注意事项

1. **内存管理**：动态创建的节点需要手动释放内存
2. **节点重用**：节点从链表移除后可以重新插入其他链表
3. **线程安全**：模块本身不提供线程同步，多线程环境需要外部保护
4. **节点内容**：Node_Data 只存储指针，不负责内容内存的管理

## 性能特点

- **插入操作**：头尾插入 O(1)，按序插入 O(n)
- **删除操作**：已知节点位置时 O(1)
- **查找操作**：线性查找 O(n)
- **内存占用**：每节点额外占用约32字节（64位系统）

该双向链表模块为嵌入式系统提供了一个高效、灵活的数据管理解决方案，特别适用于需要动态插入删除和优先级管理的应用场景。
