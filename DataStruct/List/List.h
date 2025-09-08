#ifndef __LIST_H
#define __LIST_H

#include "../DataStruct.h"

#ifdef __cplusplus
extern "C"
{
#endif

// 前向声明
typedef struct List_t List_t;

typedef struct Node_t
{
    void *Node_Content; // 节点存储内容
    struct Node_t *Node_Prev; // 前一个节点
    struct Node_t *Node_Next; // 后一个节点
    List_t *Node_Owner; // 节点所有者
    uint32_t Node_Order; // 节点序号
} Node_t;

struct List_t
{
    Node_t *List_Dummy; // 哨兵节点 Prev 指向尾 Next指向头
    uint16_t List_Count; // 链表存储的节点数目
};

// 链表操作结果枚举
typedef enum
{
    LIST_SUCCESS = 0, // 操作成功
    LIST_ERROR_NULL_POINTER, // 空指针错误
    LIST_ERROR_INVALID_PARAM, // 无效参数
    LIST_ERROR_MEMORY_ALLOC, // 内存分配失败
    LIST_ERROR_EMPTY_LIST, // 链表为空
    LIST_ERROR_NODE_NOT_FOUND, // 节点未找到
    LIST_ERROR_NODE_NOT_OWNER, // 节点不属于该链表
    LIST_ERROR_UNKNOWN // 未知错误
} ListResult_t;

// 函数声明
ListResult_t NodeCreate_Static(Node_t *node, void *content, uint32_t order);
Node_t *NodeCreate_Dynamic(void *content, uint32_t order);
ListResult_t ListCreate_Static(List_t *list, Node_t *head_node);
List_t *ListCreate_Dynamic(Node_t *head_node);
ListResult_t ListInsertEnd(List_t *list, Node_t *node);
ListResult_t ListInsertHead(List_t *list, Node_t *node);
ListResult_t ListInsertOrder(List_t *list, Node_t *node);
ListResult_t ListRemoveNode(List_t *list, Node_t *node);
ListResult_t ListMoveNode(List_t *list_src, List_t *list_dst, Node_t *node, int order);

#ifdef __cplusplus
}
#endif

#endif
