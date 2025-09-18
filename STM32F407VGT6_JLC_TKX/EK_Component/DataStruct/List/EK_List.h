/**
 * @file EK_List.h
 * @brief 双向链表数据结构头文件
 * @details 定义了双向链表的数据结构和操作接口
 * @author N1ntyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#ifndef __EK_LIST_H
#define __EK_LIST_H

#include "../../EK_Config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 类型定义区 ========================= */
typedef struct EK_List_t EK_List_t;

typedef struct EK_Node_t
{
    void *Node_Data; /**< 节点存储内容 */
    struct EK_Node_t *Node_Prev; /**< 前一个节点 */
    struct EK_Node_t *Node_Next; /**< 后一个节点 */
    EK_List_t *Node_Owner; /**< 节点所有者 */
    uint16_t Node_Order; /**< 节点序号 */
    bool Node_isDynamic; /**< 是否来自动态分配 */
} EK_Node_t;

typedef struct EK_List_t
{
    EK_Node_t *List_Dummy; /**< 哨兵节点 Prev 指向尾 Next指向头 */
    uint16_t List_Count; /**< 链表存储的节点数目 */
    bool List_isDynamic; /**< 是否来自动态分配 */
} EK_List_t;

/* ========================= 函数声明区 ========================= */
EK_Node_t *EK_pListGetHead(EK_List_t *list);
EK_Node_t *EK_pListGetEnd(EK_List_t *list);
EK_Result_t EK_rNodeCreate_Static(EK_Node_t *node, void *content, uint16_t order);
EK_Node_t *EK_pNodeCreate_Dynamic(void *content, uint16_t order);
EK_Result_t EK_rListCreate_Static(EK_List_t *list, EK_Node_t *dummy_node);
EK_List_t *EK_pListCreate_Dynamic(void);
EK_Result_t EK_rListInsertEnd(EK_List_t *list, EK_Node_t *node);
EK_Result_t EK_rListInsertHead(EK_List_t *list, EK_Node_t *node);
EK_Result_t EK_rListInsertOrder(EK_List_t *list, EK_Node_t *node);
EK_Result_t EK_rListRemoveNode(EK_List_t *list, EK_Node_t *node);
EK_Result_t EK_rListMoveNode(EK_List_t *list_src, EK_List_t *list_dst, EK_Node_t *node, int order);
EK_Result_t EK_rNodeDelete(EK_Node_t *node);
EK_Result_t EK_rListDelete(EK_List_t *list);
EK_Result_t EK_rListSort(EK_List_t *list, bool is_descend);

#ifdef __cplusplus
}
#endif

#endif
