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

### EK_Node_t - ## 注意事项

1. **内存管理**：动态创建的节点需要手动释放内存
2. **节点重用**：节点从链表移除后可以重新插入其他链表
3. **线程安全**：模块本身不提供线程同步，多线程环境需要外部保护
4. **节点内容**：Node_Data 只存储指针，不负责内容内存的管理
5. **排序限制**：
   - 排序依据是 `Node_Order` 字段，确保该字段已正确设置
   - 大链表排序可能消耗较多栈空间，注意栈大小限制
   - 排序过程中会创建临时链表，确保有足够内存空间
   可以在**EK_Config.h**中设置**LIST_RECURSION_SORT**宏定义的值来决定是否启用递归归并排序
6. **性能考虑**：
   - 频繁排序时考虑使用按序插入维护有序性
   - 大数据集排序前评估内存和时间开销
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

### 链表排序

#### 链表排序
```c
EK_Result_t EK_rListSort(EK_List_t *list, bool is_descend);
```
- **功能**：对链表进行排序，支持升序和降序
- **参数**：
  - `list`：要排序的链表指针
  - `is_descend`：排序方向（false=升序，true=降序）
- **算法策略**：
  - 小链表（< 20节点）：选择排序算法，时间复杂度 O(n²)
  - 大链表（≥ 20节点）：递归归并排序算法，时间复杂度 O(n log n)
- **排序依据**：按节点的 `Node_Order` 字段进行排序
- **稳定性**：保持相同序号节点的相对顺序不变
- **返回值**：操作结果状态码

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

### 5. 数据排序处理
```c
// 创建包含随机数据的链表
EK_List_t *data_list = EK_pListCreate_Dynamic();

// 添加一些数据节点
int values[] = {50, 20, 80, 10, 60, 30};
for (int i = 0; i < 6; i++) {
    EK_Node_t *node = EK_pNodeCreate_Dynamic(&values[i], values[i]);
    EK_rListInsertEnd(data_list, node);
}

// 升序排序
EK_rListSort(data_list, false);  // 结果：10, 20, 30, 50, 60, 80

// 降序排序
EK_rListSort(data_list, true);   // 结果：80, 60, 50, 30, 20, 10
```

### 6. 优先级队列动态调整
```c
// 任务优先级发生变化后重新排序
typedef struct {
    uint32_t task_id;
    uint32_t priority;
    void *task_context;
} Task_t;

// 动态调整任务优先级
Task_t *task = (Task_t *)task_node->Node_Data;
task->priority = new_priority;
task_node->Node_Order = new_priority;

// 重新排序整个任务队列（按优先级升序）
EK_rListSort(task_queue, false);
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

### 5. 智能排序算法
- **混合排序策略**：根据数据规模自动选择最优算法
- **小数据集优化**：少于20个节点时使用选择排序，避免递归开销
- **大数据集优化**：20个或更多节点时使用归并排序，保证O(n log n)性能
- **内存效率**：利用现有的分割和合并函数，最大化代码复用
- **稳定排序**：保持相同序号元素的相对顺序
- **双向支持**：同时支持升序和降序排列

### 6. 高级功能支持
- **跨链表操作**：支持节点在不同链表间安全移动
- **批量排序**：一次性对整个链表进行排序
- **错误恢复**：排序过程中如遇错误，能够恢复到原始状态

## 排序功能详解

### 算法选择策略

EK_List 采用混合排序策略，根据链表规模智能选择最适合的排序算法：

#### 小规模数据（< 20个节点）- 选择排序
- **优点**：实现简单，无递归开销，内存占用极少
- **适用场景**：小型任务队列、配置参数列表
- **特点**：原地排序，通过交换节点数据内容实现

#### 大规模数据（≥ 20个节点）- 递归归并排序  
- **优点**：稳定的O(n log n)性能，适合大数据集
- **适用场景**：大型事件队列、批量数据处理
- **特点**：利用已有的链表分割和合并函数，代码复用率高

### 排序过程详解

```c
// 排序流程示例
EK_List_t *data_list = create_test_list();

// 1. 算法选择阶段
if (data_list->List_Count < 20) {
    // 使用选择排序：找出最值，交换到正确位置
    // 时间复杂度：O(n²)，空间复杂度：O(1)
} else {
    // 使用递归归并排序：
    // Step 1: 找中点 - p_find_mid()
    // Step 2: 分割链表 - r_split_list() 
    // Step 3: 递归排序子链表
    // Step 4: 合并有序子链表 - r_merge_list()
}
```

### 排序稳定性保证

- **稳定排序**：相同序号的节点保持原有相对顺序
- **数据一致性**：排序过程中节点的所有者关系保持正确
- **错误恢复**：排序失败时能够恢复链表到可用状态

### 性能优化特点

1. **边界优化**：特殊处理已排序、逆序等特殊情况
2. **内存复用**：最大化利用现有的链表操作函数
3. **递归优化**：合理的递归深度控制，避免栈溢出
4. **错误处理**：完善的内存分配失败处理机制

## 注意事项

1. **内存管理**：动态创建的节点需要手动释放内存
2. **节点重用**：节点从链表移除后可以重新插入其他链表
3. **线程安全**：模块本身不提供线程同步，多线程环境需要外部保护
4. **节点内容**：Node_Data 只存储指针，不负责内容内存的管理

## 性能特点

- **插入操作**：头尾插入 O(1)，按序插入 O(n)
- **删除操作**：已知节点位置时 O(1)
- **移动操作**：跨链表移动 O(1)（不考虑目标位置查找）
- **排序操作**：
  - 小链表（< 20节点）：选择排序 O(n²)，但常数因子小
  - 大链表（≥ 20节点）：归并排序 O(n log n)，稳定高效
  - 最坏情况：O(n log n)
  - 最好情况：O(n)（已排序数据）
- **查找操作**：线性查找 O(n)
- **内存占用**：
  - 每节点：约32字节（64位系统）
  - 排序时：额外O(log n)栈空间 + O(n)临时空间
  - 哨兵节点：每链表额外一个节点开销

## 算法复杂度总结

| 操作类型 | 时间复杂度 | 空间复杂度 | 说明 |
|---------|-----------|-----------|------|
| 头部插入 | O(1) | O(1) | 直接操作 |
| 尾部插入 | O(1) | O(1) | 通过哨兵节点优化 |
| 按序插入 | O(n) | O(1) | 需要查找插入位置 |
| 删除节点 | O(1) | O(1) | 已知节点位置 |
| 节点移动 | O(1) | O(1) | 不考虑查找目标位置 |
| 小链表排序 | O(n²) | O(1) | 选择排序，原地排序 |
| 大链表排序 | O(n log n) | O(log n) | 递归归并排序 |
| 链表遍历 | O(n) | O(1) | 线性访问 |

该双向链表模块为嵌入式系统提供了一个高效、灵活的数据管理解决方案，特别适用于需要动态插入删除、优先级管理和数据排序的应用场景。新增的智能排序功能进一步增强了模块的实用性，能够满足从简单任务调度到复杂数据处理的各种需求。

## 版本更新

### v1.1 (2025-01-10)
- ✅ 新增 `EK_rListSort()` 排序功能
- ✅ 实现混合排序策略（选择排序 + 递归归并排序）
- ✅ 支持升序和降序排列
- ✅ 优化大数据集处理性能
- ✅ 增强错误处理和内存管理

### v1.0 (2025-09-08)
- ✅ 基础双向链表功能
- ✅ 哨兵节点设计
- ✅ 静态/动态内存管理
- ✅ 节点所有权机制
