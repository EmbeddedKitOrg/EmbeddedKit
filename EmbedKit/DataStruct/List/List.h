/**
 * @file List.h
 * @brief 双向链表数据结构头文件
 * @details 定义了双向链表的数据结构和操作接口
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#ifndef __LIST_H
#define __LIST_H

#include "../DataStruct.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct List_t List_t;

typedef struct Node_t
{
    void *Node_Content; /**< 节点存储内容 */
    struct Node_t *Node_Prev; /**< 前一个节点 */
    struct Node_t *Node_Next; /**< 后一个节点 */
    List_t *Node_Owner; /**< 节点所有者 */
    uint32_t Node_Order; /**< 节点序号 */
} Node_t;

typedef struct List_t
{
    Node_t *List_Dummy; /**< 哨兵节点 Prev 指向尾 Next指向头 */
    uint16_t List_Count; /**< 链表存储的节点数目 */
} List_t;

DS_Result_t NodeCreate_Static(Node_t *node, void *content, uint32_t order);
Node_t *NodeCreate_Dynamic(void *content, uint32_t order);
DS_Result_t ListCreate_Static(List_t *list, Node_t *head_node);
List_t *ListCreate_Dynamic(Node_t *head_node);
DS_Result_t ListInsertEnd(List_t *list, Node_t *node);
DS_Result_t ListInsertHead(List_t *list, Node_t *node);
DS_Result_t ListInsertOrder(List_t *list, Node_t *node);
DS_Result_t ListRemoveNode(List_t *list, Node_t *node);
DS_Result_t ListMoveNode(List_t *list_src, List_t *list_dst, Node_t *node, int order);

#ifdef __cplusplus
}
#endif

#endif
