# List 双向链表组件

## 概述

List组件实现了一个高效的双向链表数据结构，支持动态和静态内存管理方式。该组件提供了完整的链表操作接口，包括节点的创建、插入、删除和移动等功能。

## 特性

- **双向链表结构**：支持双向遍历，提高操作效率
- **哨兵节点设计**：简化边界条件处理，减少代码复杂度
- **灵活的内存管理**：支持动态分配和静态分配两种方式
- **按序插入**：支持按节点序号自动排序插入
- **节点所有权管理**：每个节点都记录其所属链表，保证操作安全性
- **统一错误处理**：使用DS_Result_t统一返回码体系

## 数据结构

### Node_t - 链表节点
```c
typedef struct Node_t
{
    void *Node_Content;           /**< 节点存储内容 */
    struct Node_t *Node_Prev;     /**< 前一个节点 */
    struct Node_t *Node_Next;     /**< 后一个节点 */
    List_t *Node_Owner;           /**< 节点所有者 */
    uint32_t Node_Order;          /**< 节点序号 */
} Node_t;
```

### List_t - 链表结构
```c
typedef struct List_t
{
    Node_t *List_Dummy;           /**< 哨兵节点 Prev 指向尾 Next指向头 */
    uint16_t List_Count;          /**< 链表存储的节点数目 */
} List_t;
```

## API接口

### 节点管理
- `DS_Result_t NodeCreate_Static(Node_t *node, void *content, uint32_t order)` - 静态创建节点
- `Node_t *NodeCreate_Dynamic(void *content, uint32_t order)` - 动态创建节点

### 链表管理
- `DS_Result_t ListCreate_Static(List_t *list, Node_t *head_node)` - 静态创建链表
- `List_t *ListCreate_Dynamic(Node_t *head_node)` - 动态创建链表

### 插入操作
- `DS_Result_t ListInsertEnd(List_t *list, Node_t *node)` - 尾部插入
- `DS_Result_t ListInsertHead(List_t *list, Node_t *node)` - 头部插入
- `DS_Result_t ListInsertOrder(List_t *list, Node_t *node)` - 按序插入

### 删除和移动
- `DS_Result_t ListRemoveNode(List_t *list, Node_t *node)` - 删除节点
- `DS_Result_t ListMoveNode(List_t *list_src, List_t *list_dst, Node_t *node, int order)` - 移动节点

## 使用示例

```c
#include "List.h"

// 创建节点内容
int data1 = 10, data2 = 20, data3 = 30;

// 动态创建节点
Node_t *node1 = NodeCreate_Dynamic(&data1, 1);
Node_t *node2 = NodeCreate_Dynamic(&data2, 2);
Node_t *node3 = NodeCreate_Dynamic(&data3, 3);

// 动态创建链表
List_t *list = ListCreate_Dynamic(node1);

// 插入节点
ListInsertEnd(list, node2);
ListInsertOrder(list, node3);

// 移动节点到另一个链表
List_t *list2 = ListCreate_Dynamic(NodeCreate_Dynamic(&data1, 1));
ListMoveNode(list, list2, node2, 0);  // 移动到头部

// 删除节点
ListRemoveNode(list, node3);
```

## 注意事项

1. **内存管理**：动态创建的节点和链表需要手动释放内存
2. **节点所有权**：每个节点只能属于一个链表，移动前会自动处理所有权转移
3. **线程安全**：当前实现不是线程安全的，多线程环境下需要额外同步措施
4. **哨兵节点**：链表使用哨兵节点简化操作，用户不应直接操作哨兵节点

## 错误码

组件使用DS_Result_t统一错误码：
- `DS_SUCCESS` - 操作成功
- `DS_ERROR_NULL_POINTER` - 空指针错误
- `DS_ERROR_MEMORY_ALLOC` - 内存分配失败
- `DS_ERROR_EMPTY` - 链表为空
- `DS_ERROR_NOT_FOUND` - 节点未找到或不属于该链表
- `DS_ERROR_UNKNOWN` - 未知错误

## 版本信息

- **版本**: 1.0
- **作者**: N1ntyNine99
- **日期**: 2025-09-08
