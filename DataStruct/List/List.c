/**
 * @file List.c
 * @brief 双向链表实现文件
 * @details 提供双向链表的创建、插入、删除等基本操作功能
 * @author N1netyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#include "List.h"

/* ========================= 宏定义区 ========================= */
/**
 * @brief 获取链表第一个节点的宏定义
 * @param X 链表指针
 * @return 链表的第一个节点指针
 */
#define GET_FIRST_NODE(X) (X->List_Dummy->Node_Next)

/**
 * @brief 获取链表最后一个节点的宏定义
 * @param X 链表指针
 * @return 链表的最后一个节点指针
 */
#define GET_LAST_NODE(X) (X->List_Dummy->Node_Prev)

/* ========================= 常数定义区 ========================= */
/**
 * @brief 哨兵节点标识常量
 * @details 用于标识哨兵节点的特殊值
 */
static const uint32_t __DUMMY = 0xABCD1234;

/* ========================= 公用API函数定义区 ========================= */
/**
 * @brief 静态创建节点
 * @details 在已分配的节点内存上初始化节点数据
 * @param node 指向已分配内存的节点指针
 * @param content 节点存储的内容指针
 * @param order 节点序号
 * @return ListResult_t 操作结果
 */
ListResult_t NodeCreate_Static(Node_t *node, void *content, uint32_t order)
{
    if (node == NULL || content == NULL) return LIST_ERROR_NULL_POINTER;

    node->Node_Content = content;
    node->Node_Order = order;
    node->Node_Next = NULL;
    node->Node_Prev = NULL;
    node->Node_Owner = NULL;

    return LIST_SUCCESS;
}

/**
 * @brief 动态创建节点
 * @details 动态分配内存并初始化节点数据
 * @param content 节点存储的内容指针
 * @param order 节点序号
 * @return Node_t* 创建的节点指针
 * @retval 非NULL 创建成功，返回节点指针
 * @retval NULL 创建失败（参数为空或内存分配失败）
 */
Node_t *NodeCreate_Dynamic(void *content, uint32_t order)
{
    if (content == NULL) return NULL;
    Node_t *node = (Node_t *)_MALLOC(sizeof(Node_t));

    // 分配失败
    if (node == NULL)
    {
        return NULL;
    }

    node->Node_Content = content;
    node->Node_Order = order;
    node->Node_Next = NULL;
    node->Node_Prev = NULL;
    node->Node_Owner = NULL;

    return node;
}

/**
 * @brief 静态创建链表
 * @details 在已分配的链表内存上初始化链表，并设置头节点
 * @param list 指向已分配内存的链表指针
 * @param head_node 链表的头节点
 * @return ListResult_t 操作结果
 */
ListResult_t ListCreate_Static(List_t *list, Node_t *head_node)
{
    if (list == NULL || head_node == NULL) return LIST_ERROR_NULL_POINTER;

    // 初始化哨兵节点
    list->List_Dummy = (Node_t *)_MALLOC(sizeof(Node_t));
    if (list->List_Dummy == NULL) return LIST_ERROR_MEMORY_ALLOC;

    list->List_Count = 1;
    head_node->Node_Owner = list;
    GET_FIRST_NODE(list) = head_node;
    GET_LAST_NODE(list) = head_node;
    list->List_Dummy->Node_Owner = list;
    list->List_Dummy->Node_Content = (void *)&__DUMMY;

    return LIST_SUCCESS;
}

/**
 * @brief 动态创建链表
 * @details 动态分配内存并初始化链表，设置头节点
 * @param head_node 链表的头节点
 * @return List_t* 创建的链表指针
 * @retval 非NULL 创建成功，返回链表指针
 * @retval NULL 创建失败（参数为空或内存分配失败）
 */
List_t *ListCreate_Dynamic(Node_t *head_node)
{
    if (head_node == NULL) return NULL;

    List_t *list = (List_t *)_MALLOC(sizeof(List_t));

    if (list == NULL)
    {
        return NULL;
    }

    // 初始化哨兵节点
    list->List_Dummy = (Node_t *)_MALLOC(sizeof(Node_t));
    if (list->List_Dummy == NULL)
    {
        _FREE(list);
        return NULL;
    }

    list->List_Count = 1;
    head_node->Node_Owner = list;
    GET_FIRST_NODE(list) = head_node;
    GET_LAST_NODE(list) = head_node;
    list->List_Dummy->Node_Owner = list;
    list->List_Dummy->Node_Content = (void *)&__DUMMY;

    return list;
}

/**
 * @brief 在链表尾部插入节点
 * @details 将指定节点插入到链表的尾部位置
 * @param list 目标链表指针
 * @param node 要插入的节点指针
 * @return ListResult_t 操作结果
 */
ListResult_t ListInsertEnd(List_t *list, Node_t *node)
{
    if (list == NULL || node == NULL) return LIST_ERROR_NULL_POINTER;

    // 空链表
    if (list->List_Count == 0)
    {
        node->Node_Owner = list;
        GET_FIRST_NODE(list) = node;
        GET_LAST_NODE(list) = node;
        list->List_Count = 1;
        return LIST_SUCCESS;
    }

    Node_t *temp = GET_LAST_NODE(list);
    node->Node_Owner = list;
    list->List_Count++;

    node->Node_Prev = temp;
    node->Node_Next = list->List_Dummy;

    temp->Node_Next = node;
    list->List_Dummy->Node_Prev = node; //维护哨兵节点
    GET_LAST_NODE(list) = node;

    return LIST_SUCCESS;
}

/**
 * @brief 在链表头部插入节点
 * @details 将指定节点插入到链表的头部位置
 * @param list 目标链表指针
 * @param node 要插入的节点指针
 * @return ListResult_t 操作结果
 */
ListResult_t ListInsertHead(List_t *list, Node_t *node)
{
    if (list == NULL || node == NULL) return LIST_ERROR_NULL_POINTER;

    // 空链表
    if (list->List_Count == 0)
    {
        node->Node_Owner = list;
        GET_FIRST_NODE(list) = node;
        GET_LAST_NODE(list) = node;
        list->List_Count = 1;
        return LIST_SUCCESS;
    }

    Node_t *temp = GET_FIRST_NODE(list);
    node->Node_Owner = list;
    list->List_Count++;

    node->Node_Next = temp;
    node->Node_Prev = list->List_Dummy;

    temp->Node_Prev = node;
    list->List_Dummy->Node_Next = node; //维护哨兵节点
    GET_FIRST_NODE(list) = node;

    return LIST_SUCCESS;
}

/**
 * @brief 按序号有序插入节点
 * @details 根据节点的序号值，将节点插入到链表中合适的位置以保持有序
 * @param list 目标链表指针
 * @param node 要插入的节点指针
 * @return ListResult_t 操作结果
 * @retval LIST_SUCCESS 插入成功
 */
ListResult_t ListInsertOrder(List_t *list, Node_t *node)
{
    if (list == NULL || node == NULL) return LIST_ERROR_NULL_POINTER;

    // 空链表
    if (list->List_Count == 0)
    {
        node->Node_Owner = list;
        GET_FIRST_NODE(list) = node;
        GET_LAST_NODE(list) = node;
        list->List_Count = 1;
        return LIST_SUCCESS;
    }

    // 比首节点都小
    if (node->Node_Order <= GET_FIRST_NODE(list)->Node_Order)
    {
        return ListInsertHead(list, node);
    }

    // 比尾节点都大
    if (node->Node_Order >= GET_LAST_NODE(list)->Node_Order)
    {
        return ListInsertEnd(list, node);
    }

    Node_t *p = GET_FIRST_NODE(list);

    // 遍历连表查询
    while (p->Node_Next != list->List_Dummy)
    {
        if (p->Node_Next->Node_Order > node->Node_Order)
        {
            // node 节点连接 p 和 p 本来的下个节点
            node->Node_Prev = p;
            node->Node_Next = p->Node_Next;

            // p 本来的下个节点连接 node 节点
            p->Node_Next->Node_Prev = node;

            // p 连接 node 节点
            p->Node_Next = node;

            node->Node_Owner = list;
            list->List_Count++;
            return LIST_SUCCESS;
        }
        p = p->Node_Next;
    }

    return LIST_ERROR_UNKNOWN;
}

/**
 * @brief 从链表中移除指定节点
 * @details 将指定节点从链表中移除，并重新连接相邻节点
 * @param list 目标链表指针
 * @param node 要移除的节点指针
 * @return ListResult_t 操作结果
 */
ListResult_t ListRemoveNode(List_t *list, Node_t *node)
{
    if (list == NULL || node == NULL) return LIST_ERROR_NULL_POINTER;
    if (list->List_Count == 0) return LIST_ERROR_EMPTY_LIST;
    if (node->Node_Owner != list) return LIST_ERROR_NODE_NOT_OWNER;

    // 只有一个节点
    if (list->List_Count == 1)
    {
        if (GET_FIRST_NODE(list) == GET_LAST_NODE(list))
        {
            if (GET_FIRST_NODE(list) == node)
            {
                node->Node_Next = NULL;
                node->Node_Prev = NULL;
                node->Node_Owner = NULL;

                list->List_Dummy->Node_Next = list->List_Dummy;
                list->List_Dummy->Node_Prev = list->List_Dummy;

                list->List_Count = 0;
                return LIST_SUCCESS;
            }
            return LIST_ERROR_NODE_NOT_FOUND;
        }
    }

    // 是头节点
    if (node == GET_FIRST_NODE(list))
    {
        // 修改原本头节点
        GET_FIRST_NODE(list) = node->Node_Next;

        // 维护哨兵节点和新头节点的连接
        list->List_Dummy->Node_Next = node->Node_Next;
        node->Node_Next->Node_Prev = list->List_Dummy;

        // 修改 owner 链表的信息
        list->List_Count--;

        // 删除 node 自身的连接
        node->Node_Next = NULL;
        node->Node_Prev = NULL;
        node->Node_Owner = NULL;

        return LIST_SUCCESS;
    }

    // 是尾节点
    if (node == GET_LAST_NODE(list))
    {
        // 修改原本尾节点
        GET_LAST_NODE(list) = node->Node_Prev;

        // 维护哨兵节点和新尾节点的连接
        list->List_Dummy->Node_Prev = node->Node_Prev;
        node->Node_Prev->Node_Next = list->List_Dummy;

        // 修改 owner 链表的信息
        list->List_Count--;

        // 删除 node 自身的连接
        node->Node_Next = NULL;
        node->Node_Prev = NULL;
        node->Node_Owner = NULL;

        return LIST_SUCCESS;
    }

    // 将 node 前后节点相互连接
    node->Node_Prev->Node_Next = node->Node_Next;
    node->Node_Next->Node_Prev = node->Node_Prev;

    // 修改 owner 链表的信息
    list->List_Count--;

    // 删除 node 自身的连接
    node->Node_Next = NULL;
    node->Node_Prev = NULL;
    node->Node_Owner = NULL;

    return LIST_SUCCESS;
}

/**
 * @brief 将节点从源链表移动到目标链表
 * @details 将指定节点从源链表中移除，并根据order参数插入到目标链表的指定位置
 * @param list_src 源链表指针
 * @param list_dst 目标链表指针
 * @param node 要移动的节点指针
 * @param order 插入方式：0-插入到头部，<0-插入到尾部，>0-按序号插入
 * @return ListResult_t 操作结果
 */
ListResult_t ListMoveNode(List_t *list_src, List_t *list_dst, Node_t *node, int order)
{
    if (list_dst == NULL || list_src == NULL) return LIST_ERROR_NULL_POINTER;
    if (node->Node_Owner != list_src) return LIST_ERROR_NODE_NOT_OWNER;

    if (list_src == list_dst) return LIST_SUCCESS;

    // 从原链表移除节点
    ListResult_t res = ListRemoveNode(list_src, node);
    if (res != LIST_SUCCESS) return res;

    if (order == 0) // 插入到头部
    {
        res = ListInsertHead(list_dst, node);
        if (res != LIST_SUCCESS) return res;
    }
    else if (order < 0) // 插入到尾部
    {
        res = ListInsertEnd(list_dst, node);
        if (res != LIST_SUCCESS) return res;
    }
    else // 按序号插入
    {
        res = ListInsertOrder(list_dst, node);
        if (res != LIST_SUCCESS) return res;
    }

    return LIST_SUCCESS;
}