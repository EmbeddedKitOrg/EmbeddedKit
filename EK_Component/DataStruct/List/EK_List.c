/**
 * @file EK_List.c
 * @brief 双向链表实现文件
 * @details 提供双向链表的创建、插入、删除等基本操作功能
 * @author N1netyNine99
 * @date 2025-09-08
 * @version 1.0
 */

#include "EK_List.h"
/* ========================= 宏定义区 ========================= */
#ifndef LIST_RECURSION_SORT
#define LIST_RECURSION_SORT 1
#endif /* LIST_RECURSION_SORT */

/* ========================= 常数定义区 ========================= */
/**
 * @brief 哨兵节点标识常量
 * @details 用于标识哨兵节点的特殊值
 */
static const uint32_t __DUMMY__ = 0xABCD1234;

/* ========================= 内部函数定义区 ========================= */

/**
 * @brief 获取链表的第一个有效节点
 * 
 * @param list 想要获取节点的链表
 * @return EK_Node_t* 第一个有效节点
 */
static inline EK_Node_t *p_list_get_head(EK_List_t *list)
{
    if (list == NULL) return NULL;

    // 如果首节点和哨兵节点相同 说明没有一个有效节点
    if (list->List_Dummy->Node_Next == list->List_Dummy) return NULL;

    return list->List_Dummy->Node_Next;
}

/**
 * @brief 获取链表的最后一个有效节点
 * 
 * @param list 想要获取节点的链表
 * @return EK_Node_t* 最后一个有效节点
 */
static inline EK_Node_t *p_list_get_end(EK_List_t *list)
{
    if (list == NULL) return NULL;

    // 如果尾节点和哨兵节点相同 说明没有一个有效节点
    if (list->List_Dummy->Node_Prev == list->List_Dummy) return NULL;

    return list->List_Dummy->Node_Prev;
}

/**
 * @brief 初始化一个链表
 * 
 * @param list 想要初始化的链表
 * @return EK_Result_t 操作结果
 */
static EK_Result_t r_list_init(EK_List_t *list)
{
    if (list == NULL || list->List_Dummy == NULL) return EK_NULL_POINTER;

    list->List_Count = 0;
    list->List_Dummy->Node_Next = list->List_Dummy; // 空链表时指向自己
    list->List_Dummy->Node_Prev = list->List_Dummy; // 空链表时指向自己
    list->List_Dummy->Node_Owner = list;
    list->List_Dummy->Node_Data = (void *)&__DUMMY__;

    // 根据链表类型设置哨兵节点的动态标识
    list->List_Dummy->Node_isDynamic = list->List_isDynamic;

    return EK_OK;
}
#if (LIST_RECURSION_SORT != 0)
/**
 * @brief 找到链表的一个中点
 * 
 * @param list 想要找寻的链表
 * @return EK_Node_t* 返回中点
 */
static EK_Node_t *p_find_mid(EK_List_t *list)
{
    if (list == NULL) return NULL;
    if (list->List_Count == 0) return NULL;
    if (list->List_Count == 1) return p_list_get_head(list);

    EK_Node_t *p_fast, *p_slow; //快慢指针
    p_fast = p_slow = p_list_get_head(list);
    uint32_t step_count = 0; // 添加步数计数器防止无限循环

    while (p_fast != list->List_Dummy && p_fast->Node_Next != list->List_Dummy && step_count < list->List_Count)
    {
        // 额外的安全检查
        if (p_slow == NULL || p_fast == NULL || p_slow->Node_Owner != list || p_fast->Node_Owner != list)
        {
            break; // 链表结构可能已损坏
        }

        // 慢指针移动一个节点
        p_slow = p_slow->Node_Next;

        // 检查快指针的下一个节点是否有效
        if (p_fast->Node_Next == NULL || p_fast->Node_Next->Node_Next == NULL)
        {
            break;
        }

        // 快指针移动两个节点
        p_fast = p_fast->Node_Next->Node_Next;

        step_count++; // 增加步数计数器
    }

    return p_slow;
}

/**
 * @brief 将链表从指定节点分开
 * 
 * @param list 要分开的链表
 * @param node 分开的基准节点
 * @param list_left 存储前半段的链表
 * @param list_right 存储后半段的链表
 * @return EK_Result_t 操作结果
 */
static EK_Result_t r_split_list(EK_List_t *list, EK_Node_t *node, EK_List_t *list_left, EK_List_t *list_right)
{
    if (list == NULL || node == NULL || list_left == NULL || list_right == NULL) return EK_NULL_POINTER;
    if (list->List_Count <= 1) return EK_INVALID_PARAM;
    if (node->Node_Owner != list) return EK_NOT_FOUND;

    // 确保目标链表已经有哨兵节点
    if (list_left->List_Dummy == NULL || list_right->List_Dummy == NULL) return EK_NULL_POINTER;

    // 初始化左右链表
    EK_Result_t res = r_list_init(list_left);
    if (res != EK_OK) return res;
    res = r_list_init(list_right);
    if (res != EK_OK) return res;

    // 计算左半部分的节点数量
    uint16_t left_count = 0;
    EK_Node_t *current = p_list_get_head(list);
    uint32_t traverse_count = 0; // 添加遍历计数器防止无限循环

    // 遍历到分割点，计算左半部分节点数
    while (current != node && current != list->List_Dummy && traverse_count < list->List_Count)
    {
        // 额外的安全检查
        if (current == NULL || current->Node_Owner != list)
        {
            return EK_ERROR; // 链表结构可能已损坏
        }

        left_count++;
        current = current->Node_Next;
        traverse_count++; // 增加遍历计数器
    }

    if (current != node) return EK_NOT_FOUND; // 节点不在链表中

    left_count++; // 包含分割节点本身在左半部分

    // 如果分割点是第一个节点，左半部分只有一个节点
    if (left_count == 1)
    {
        // 将第一个节点移到左链表
        EK_Node_t *first_node = p_list_get_head(list);
        EK_Node_t *second_node = first_node->Node_Next;

        // 设置左链表（只有一个节点）
        list_left->List_Dummy->Node_Next = first_node;
        list_left->List_Dummy->Node_Prev = first_node;

        first_node->Node_Next = list_left->List_Dummy;
        first_node->Node_Prev = list_left->List_Dummy;
        first_node->Node_Owner = list_left;

        list_left->List_Count = 1;

        // 设置右链表（剩余节点）
        if (second_node != list->List_Dummy)
        {
            list_right->List_Dummy->Node_Next = second_node;
            list_right->List_Dummy->Node_Prev = p_list_get_end(list);

            second_node->Node_Prev = list_right->List_Dummy;
            list->List_Dummy->Node_Prev->Node_Next = list_right->List_Dummy;

            // 更新右链表所有节点的Owner
            current = second_node;
            list_right->List_Count = list->List_Count - 1;
            while (current != list_right->List_Dummy)
            {
                current->Node_Owner = list_right;
                current = current->Node_Next;
            }
        }
    }
    else
    {
        // 一般情况：在指定节点后分割
        EK_Node_t *left_head = p_list_get_head(list);
        EK_Node_t *left_tail = node;
        EK_Node_t *right_head = node->Node_Next;
        EK_Node_t *right_tail = p_list_get_end(list);

        // 设置左链表
        list_left->List_Dummy->Node_Next = left_head;
        list_left->List_Dummy->Node_Prev = left_tail;

        left_head->Node_Prev = list_left->List_Dummy;
        left_tail->Node_Next = list_left->List_Dummy;
        list_left->List_Count = left_count;

        // 更新左链表所有节点的Owner
        current = left_head;
        while (current != list_left->List_Dummy)
        {
            current->Node_Owner = list_left;
            current = current->Node_Next;
        }

        // 设置右链表（如果右半部分不为空）
        if (right_head != list->List_Dummy)
        {
            list_right->List_Dummy->Node_Next = right_head;
            list_right->List_Dummy->Node_Prev = right_tail;

            right_head->Node_Prev = list_right->List_Dummy;
            right_tail->Node_Next = list_right->List_Dummy;
            list_right->List_Count = list->List_Count - left_count;

            // 更新右链表所有节点的Owner
            current = right_head;
            while (current != list_right->List_Dummy)
            {
                current->Node_Owner = list_right;
                current = current->Node_Next;
            }
        }
    }

    // 清空原链表
    list->List_Count = 0;
    list->List_Dummy->Node_Next = list->List_Dummy;
    list->List_Dummy->Node_Prev = list->List_Dummy;

    return EK_OK;
}

/**
 * @brief 合并两个链表
 * 
 * @param list1 链表1
 * @param list2 链表2
 * @param list_merged 合并过后的链表
 * @param is_descend 是否降序合并
 * @return EK_Result_t 操作结果
 */
static EK_Result_t r_merge_list(EK_List_t *list1, EK_List_t *list2, EK_List_t *list_merged, bool is_descend)
{
    if (list1 == NULL || list2 == NULL || list_merged == NULL) return EK_NULL_POINTER;
    if (list_merged->List_Dummy == NULL) return EK_NULL_POINTER;

    // 初始化合并链表
    EK_Result_t res = r_list_init(list_merged);
    if (res != EK_OK) return res;

    // 如果两个链表都为空
    if (list1->List_Count == 0 && list2->List_Count == 0) return EK_OK;

    // 如果其中一个链表为空，直接复制另一个链表
    if (list1->List_Count == 0)
    {
        // 将list2的所有节点移动到merged链表
        uint32_t transfer_count = 0;
        while (list2->List_Count > 0 && transfer_count < 1000) // 添加安全计数器
        {
            EK_Result_t temp = EK_OK;
            EK_Node_t *node = p_list_get_head(list2);
            if (node == NULL || node == list2->List_Dummy) break; // 安全检查

            temp = EK_rListRemoveNode(list2, node);
            temp = EK_rListInsertEnd(list_merged, node);
            if (temp != EK_OK) return temp;
            transfer_count++;
        }
        return EK_OK;
    }

    if (list2->List_Count == 0)
    {
        // 将list1的所有节点移动到merged链表
        uint32_t transfer_count = 0;
        while (list1->List_Count > 0 && transfer_count < 1000) // 添加安全计数器
        {
            EK_Result_t temp = EK_OK;
            EK_Node_t *node = p_list_get_head(list1);
            if (node == NULL || node == list1->List_Dummy) break; // 安全检查

            temp = EK_rListRemoveNode(list1, node);
            temp = EK_rListInsertEnd(list_merged, node);
            if (temp != EK_OK) return temp;
            transfer_count++;
        }
        return EK_OK;
    }

    // 双链表合并
    EK_Node_t *p1 = p_list_get_head(list1); // list1当前节点
    EK_Node_t *p2 = p_list_get_head(list2); // list2当前节点

    // 直接将两个链表的节点连接到 list_merged
    EK_Node_t *merged_tail = list_merged->List_Dummy;

    while (p1 != list1->List_Dummy && p2 != list2->List_Dummy)
    {
        EK_Node_t *selected_node;
        bool choose_p1;

        if (is_descend)
        {
            // 降序：选择较大的值
            choose_p1 = (p1->Node_Order >= p2->Node_Order);
        }
        else
        {
            // 升序：选择较小的值
            choose_p1 = (p1->Node_Order <= p2->Node_Order);
        }

        if (choose_p1)
        {
            selected_node = p1;
            p1 = p1->Node_Next;
        }
        else
        {
            selected_node = p2;
            p2 = p2->Node_Next;
        }

        // 将选中的节点链接到合并链表的尾部
        merged_tail->Node_Next = selected_node;
        selected_node->Node_Prev = merged_tail;
        selected_node->Node_Owner = list_merged;
        merged_tail = selected_node;
    }

    // 处理剩余的节点
    EK_Node_t *remaining_list_head = (p1 != list1->List_Dummy) ? p1 : p2;
    EK_List_t *remaining_list = (p1 != list1->List_Dummy) ? list1 : list2;

    if (remaining_list_head != remaining_list->List_Dummy)
    {
        merged_tail->Node_Next = remaining_list_head;
        remaining_list_head->Node_Prev = merged_tail;

        EK_Node_t *remaining_list_tail = p_list_get_end(remaining_list);
        list_merged->List_Dummy->Node_Prev = remaining_list_tail;
        remaining_list_tail->Node_Next = list_merged->List_Dummy;

        // 更新剩余所有节点的 owner
        EK_Node_t *current = remaining_list_head;
        while (current != list_merged->List_Dummy)
        {
            current->Node_Owner = list_merged;
            current = current->Node_Next;
        }
    }
    else
    {
        // 如果没有剩余节点，正确闭合链表
        merged_tail->Node_Next = list_merged->List_Dummy;
        list_merged->List_Dummy->Node_Prev = merged_tail;
    }

    // 更新合并后链表的节点总数
    list_merged->List_Count = list1->List_Count + list2->List_Count;

    // 清空原始链表（因为节点已经全部移动）
    r_list_init(list1);
    r_list_init(list2);

    return EK_OK;
}
#endif /* LIST_RECURSION_SORT != 0 */
/* ========================= 公用API函数定义区 ========================= */
/**
 * @brief 获取链表的第一个有效节点
 * 
 * @param list 想要获取节点的链表
 * @return EK_Node_t* 第一个有效节点
 */

EK_Node_t *EK_pListGetHead(EK_List_t *list)
{
    return p_list_get_head(list);
}

/**
 * @brief 获取链表的最后一个有效节点
 * 
 * @param list 想要获取节点的链表
 * @return EK_Node_t* 最后一个有效节点
 */

EK_Node_t *EK_pListGetEnd(EK_List_t *list)
{
    return p_list_get_end(list);
}

/**
 * @brief 动态创建节点
 * @details 动态分配内存并初始化节点数据
 * @param content 节点存储的内容指针
 * @param order 节点序号
 * @return EK_Node_t* 创建的节点指针
 * @retval 非NULL 创建成功，返回节点指针
 * @retval NULL 创建失败（参数为空或内存分配失败）
 */
EK_Node_t *EK_pNodeCreate(void *content, uint16_t order)
{
    if (content == NULL) return NULL;
    EK_Node_t *node = (EK_Node_t *)EK_MALLOC(sizeof(EK_Node_t));

    // 分配失败
    if (node == NULL)
    {
        return NULL;
    }

    node->Node_Data = content;
    node->Node_Order = order;
    node->Node_Next = NULL;
    node->Node_Prev = NULL;
    node->Node_Owner = NULL;
    node->Node_isDynamic = true;

    return node;
}

/**
 * @brief 静态创建节点
 * @details 在已分配的节点内存上初始化节点数据
 * @param node 指向已分配内存的节点指针
 * @param content 节点存储的内容指针
 * @param order 节点序号
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_pNodeCreateStatic(EK_Node_t *node, void *content, uint16_t order)
{
    if (node == NULL || content == NULL) return EK_NULL_POINTER;

    node->Node_Data = content;
    node->Node_Order = order;
    node->Node_Next = NULL;
    node->Node_Prev = NULL;
    node->Node_Owner = NULL;
    node->Node_isDynamic = false;

    return EK_OK;
}

/**
 * @brief 动态创建链表
 * @details 动态分配内存并初始化链表，设置头节点
 * @return EK_List_t* 创建的链表指针
 * @retval 非NULL 创建成功，返回链表指针
 * @retval NULL 创建失败（参数为空或内存分配失败）
 */
EK_List_t *EK_pListCreate(void)
{
    EK_List_t *list = (EK_List_t *)EK_MALLOC(sizeof(EK_List_t));

    if (list == NULL)
    {
        return NULL;
    }

    // 初始化哨兵节点
    list->List_Dummy = (EK_Node_t *)EK_MALLOC(sizeof(EK_Node_t));
    if (list->List_Dummy == NULL)
    {
        EK_FREE(list);
        return NULL;
    }
    list->List_isDynamic = true;

    // 标记哨兵节点为动态分配
    list->List_Dummy->Node_isDynamic = true;

    // 初始化链表
    if (r_list_init(list) != EK_OK)
    {
        EK_FREE(list);
        return NULL;
    }

    return list;
}

/**
 * @brief 静态创建链表
 * @details 在已分配的链表内存上初始化链表
 * @param list 指向已分配内存的链表指针
 * @param dummy_node 哨兵节点
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_pListCreateStatic(EK_List_t *list, EK_Node_t *dummy_node)
{
    if (list == NULL || dummy_node == NULL) return EK_NULL_POINTER;

    // 初始化哨兵节点
    list->List_Dummy = dummy_node;
    list->List_isDynamic = false;

    // 确保哨兵节点被标记为静态
    dummy_node->Node_isDynamic = false;

    return r_list_init(list);
}

/**
 * @brief 在链表尾部插入节点
 * @details 将指定节点插入到链表的尾部位置
 * @param list 目标链表指针
 * @param node 要插入的节点指针
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rListInsertEnd(EK_List_t *list, EK_Node_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    // 空链表
    if (list->List_Count == 0)
    {
        node->Node_Owner = list;
        node->Node_Next = list->List_Dummy;
        node->Node_Prev = list->List_Dummy;
        list->List_Dummy->Node_Next = node;
        list->List_Dummy->Node_Prev = node;
        list->List_Count = 1;
        return EK_OK;
    }

    EK_Node_t *temp = p_list_get_end(list);
    node->Node_Owner = list;
    list->List_Count++;

    node->Node_Prev = temp;
    node->Node_Next = list->List_Dummy;

    temp->Node_Next = node;
    list->List_Dummy->Node_Prev = node; //维护哨兵节点

    return EK_OK;
}

/**
 * @brief 在链表头部插入节点
 * @details 将指定节点插入到链表的头部位置
 * @param list 目标链表指针
 * @param node 要插入的节点指针
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rListInsertHead(EK_List_t *list, EK_Node_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    // 空链表
    if (list->List_Count == 0)
    {
        node->Node_Owner = list;
        node->Node_Next = list->List_Dummy;
        node->Node_Prev = list->List_Dummy;
        list->List_Dummy->Node_Next = node;
        list->List_Dummy->Node_Prev = node;
        list->List_Count = 1;
        return EK_OK;
    }

    EK_Node_t *temp = p_list_get_head(list);
    node->Node_Owner = list;
    list->List_Count++;

    node->Node_Next = temp;
    node->Node_Prev = list->List_Dummy;

    temp->Node_Prev = node;
    list->List_Dummy->Node_Next = node; //维护哨兵节点

    return EK_OK;
}

/**
 * @brief 按序号有序插入节点
 * @details 根据节点的序号值，将节点插入到链表中合适的位置以保持有序
 * @param list 目标链表指针
 * @param node 要插入的节点指针
 * @return EK_Result_t 操作结果
 * @retval DS_SUCCESS 插入成功
 */
EK_Result_t EK_rListInsertOrder(EK_List_t *list, EK_Node_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;

    // 空链表
    if (list->List_Count == 0)
    {
        node->Node_Owner = list;
        node->Node_Next = list->List_Dummy;
        node->Node_Prev = list->List_Dummy;
        list->List_Dummy->Node_Next = node;
        list->List_Dummy->Node_Prev = node;
        list->List_Count = 1;
        return EK_OK;
    }

    // 比首节点都小
    if (node->Node_Order <= p_list_get_head(list)->Node_Order)
    {
        return EK_rListInsertHead(list, node);
    }

    // 比尾节点都大
    if (node->Node_Order >= p_list_get_end(list)->Node_Order)
    {
        return EK_rListInsertEnd(list, node);
    }

    EK_Node_t *p = p_list_get_head(list);

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
            return EK_OK;
        }
        p = p->Node_Next;
    }

    return EK_UNKNOWN;
}

/**
 * @brief 从链表中移除指定节点
 * @details 将指定节点从链表中移除，并重新连接相邻节点
 * @param list 目标链表指针
 * @param node 要移除的节点指针
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rListRemoveNode(EK_List_t *list, EK_Node_t *node)
{
    if (list == NULL || node == NULL) return EK_NULL_POINTER;
    if (list->List_Count == 0) return EK_EMPTY;
    if (node->Node_Owner != list) return EK_NOT_FOUND;

    // 只有一个节点
    if (list->List_Count == 1)
    {
        if (p_list_get_head(list) == p_list_get_end(list))
        {
            if (p_list_get_head(list) == node)
            {
                node->Node_Next = NULL;
                node->Node_Prev = NULL;
                node->Node_Owner = NULL;

                list->List_Dummy->Node_Next = list->List_Dummy;
                list->List_Dummy->Node_Prev = list->List_Dummy;

                list->List_Count = 0;
                return EK_OK;
            }
            return EK_NOT_FOUND;
        }
    }

    // 是头节点
    if (node == p_list_get_head(list))
    {
        // 维护哨兵节点和新头节点的连接
        list->List_Dummy->Node_Next = node->Node_Next;
        node->Node_Next->Node_Prev = list->List_Dummy;

        // 修改 owner 链表的信息
        list->List_Count--;

        // 删除 node 自身的连接
        node->Node_Next = NULL;
        node->Node_Prev = NULL;
        node->Node_Owner = NULL;

        return EK_OK;
    }

    // 是尾节点
    if (node == p_list_get_end(list))
    {
        // 维护哨兵节点和新尾节点的连接
        list->List_Dummy->Node_Prev = node->Node_Prev;
        node->Node_Prev->Node_Next = list->List_Dummy;

        // 修改 owner 链表的信息
        list->List_Count--;

        // 删除 node 自身的连接
        node->Node_Next = NULL;
        node->Node_Prev = NULL;
        node->Node_Owner = NULL;

        return EK_OK;
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

    return EK_OK;
}

/**
 * @brief 将节点从源链表移动到目标链表
 * @details 将指定节点从源链表中移除，并根据order参数插入到目标链表的指定位置
 * @param list_src 源链表指针
 * @param list_dst 目标链表指针
 * @param node 要移动的节点指针
 * @param order 插入方式：0-插入到头部，<0-插入到尾部，>0-按序号插入
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rListMoveNode(EK_List_t *list_src, EK_List_t *list_dst, EK_Node_t *node, int order)
{
    if (list_dst == NULL || list_src == NULL) return EK_NULL_POINTER;
    if (node->Node_Owner != list_src) return EK_NOT_FOUND;

    if (list_src == list_dst) return EK_OK;

    // 从原链表移除节点
    EK_Result_t res = EK_rListRemoveNode(list_src, node);
    if (res != EK_OK) return res;

    if (order == 0) // 插入到头部
    {
        res = EK_rListInsertHead(list_dst, node);
        if (res != EK_OK) return res;
    }
    else if (order < 0) // 插入到尾部
    {
        res = EK_rListInsertEnd(list_dst, node);
        if (res != EK_OK) return res;
    }
    else // 按序号插入
    {
        res = EK_rListInsertOrder(list_dst, node);
        if (res != EK_OK) return res;
    }

    return EK_OK;
}

/**
 * @brief 删除单个节点并释放其内存
 * @details 删除指定节点并释放其动态分配的内存
 * @param node 要删除的节点指针
 * @return EK_Result_t 操作结果
 * @note 此函数只释放节点本身的内存，不释放Node_Data指向的内容内存
 *       如果节点仍在链表中，需要先调用EK_rListRemoveNode移除
 *       静态分配的节点不会被释放，只会被重置
 */
EK_Result_t EK_rNodeDelete(EK_Node_t *node)
{
    if (node == NULL) return EK_NULL_POINTER;

    // 检查节点是否仍在链表中
    if (node->Node_Owner != NULL)
    {
        return EK_ERROR; // 节点仍在链表中，需要先移除
    }

    // 清除节点数据（安全起见）
    node->Node_Data = NULL;
    node->Node_Next = NULL;
    node->Node_Prev = NULL;
    node->Node_Order = 0;

    // 只释放动态分配的节点
    if (node->Node_isDynamic == true)
    {
        EK_FREE(node);
    }

    return EK_OK;
}

/**
 * @brief 删除整个链表并释放所有节点内存
 * @details 遍历链表删除所有节点，并释放动态分配的链表结构和哨兵节点
 * @param list 要删除的链表指针
 * @return EK_Result_t 操作结果
 * @note 此函数会释放所有动态分配的内存，包括：
 *       - 动态分配的节点内存
 *       - 动态分配的哨兵节点内存  
 *       - 动态分配的链表结构内存
 *       节点的Node_Data指向的内存需要用户自行管理
 *       静态分配的节点和链表结构不会被释放，只会被重置为空状态
 *       调用此函数后，动态分配的链表指针将变为无效，不应再使用
 */
EK_Result_t EK_rListDelete(EK_List_t *list)
{
    if (list == NULL) return EK_NULL_POINTER;
    if (list->List_Dummy == NULL) return EK_NULL_POINTER;
    if (list->List_Count == 0) return EK_OK; // 空链表无需删除

    EK_Node_t *current = p_list_get_head(list);
    EK_Node_t *next_node = NULL;
    uint32_t delete_count = 0; // 安全计数器
    uint32_t original_count = list->List_Count; // 保存原始节点数量

    // 遍历删除所有节点
    while (current != list->List_Dummy && delete_count < original_count)
    {
        // 安全检查
        if (current == NULL || current->Node_Owner != list)
        {
            break; // 链表结构可能已损坏
        }

        // 保存下一个节点
        next_node = current->Node_Next;

        // 清除当前节点的连接
        current->Node_Next = NULL;
        current->Node_Prev = NULL;
        current->Node_Owner = NULL;

        // 释放动态分配的节点内存
        if (current->Node_isDynamic == true)
        {
            EK_FREE(current);
        }

        current = next_node;
        delete_count++;
    }

    // 重置链表为空状态
    list->List_Count = 0;
    list->List_Dummy->Node_Next = list->List_Dummy;
    list->List_Dummy->Node_Prev = list->List_Dummy;

    // 释放哨兵节点（如果是动态分配的）
    if (list->List_Dummy->Node_isDynamic == true)
    {
        EK_FREE(list->List_Dummy);
        list->List_Dummy = NULL; // 避免悬挂指针
    }

    // 释放链表结构（如果是动态分配的）
    if (list->List_isDynamic == true)
    {
        EK_FREE(list);
    }

    return EK_OK;
}

/**
 * @brief 将指定链表按照顺序排列（使用递归归并排序）
 * 
 * @param list 想要排序的链表指针
 * @param is_descend 是否为降序 false:升序 true:降序
 * @return EK_Result_t 操作结果
 * @note 如果启用了递归排序，则会在链表节点大于20个之后使用递归归并排序
 *       存在栈溢出的风险!
 */
EK_Result_t EK_rListSort(EK_List_t *list, bool is_descend)
{
    if (list == NULL) return EK_NULL_POINTER;
    if (list->List_Count <= 1) return EK_OK; // 空链表或单节点无需排序

#if (LIST_RECURSION_SORT != 0)
    // 小型链表采用选择排序法
    if (list->List_Count < 5)
    {
#endif /* LIST_RECURSION_SORT != 0 */
        EK_Node_t *current = p_list_get_head(list);
        uint32_t processed_count = 0; // 添加计数器防止无限循环

        while (current != list->List_Dummy && processed_count < list->List_Count)
        {
            EK_Node_t *min_max_node = current;
            EK_Node_t *search_node = current->Node_Next;
            uint32_t search_count = 0; // 内层循环计数器

            // 在剩余节点中找到最小值（升序）或最大值（降序）
            while (search_node != list->List_Dummy && search_count < (list->List_Count - processed_count))
            {
                // 额外的安全检查
                if (search_node == NULL || search_node->Node_Owner != list)
                {
                    return EK_ERROR; // 链表结构可能已损坏，退出
                }

                bool should_select = is_descend ? (search_node->Node_Order > min_max_node->Node_Order)
                                                : (search_node->Node_Order < min_max_node->Node_Order);

                if (should_select)
                {
                    min_max_node = search_node;
                }

                search_node = search_node->Node_Next;
                search_count++; // 增加内层计数器

                // 防止内层无限循环
                if (search_count >= (list->List_Count - processed_count))
                {
                    break; // 已经搜索完所有剩余节点
                }
            }

            // 如果找到了更合适的节点，交换数据
            if (min_max_node != current)
            {
                void *temp_data = current->Node_Data;
                uint32_t temp_order = current->Node_Order;

                current->Node_Data = min_max_node->Node_Data;
                current->Node_Order = min_max_node->Node_Order;

                min_max_node->Node_Data = temp_data;
                min_max_node->Node_Order = temp_order;
            }

            current = current->Node_Next;
            processed_count++; // 增加外层计数器
        }

        return EK_OK;
#if (LIST_RECURSION_SORT != 0)
    }

    else // 大型链表使用递归归并排序
    {
        // 找到链表中点
        EK_Node_t *mid_node = p_find_mid(list);
        if (mid_node == NULL) return EK_ERROR;

        // 创建左右子链表
        EK_List_t *left_list = EK_pListCreate();
        EK_List_t *right_list = EK_pListCreate();
        if (left_list == NULL || right_list == NULL)
        {
            if (left_list)
            {
                EK_FREE(left_list->List_Dummy);
                EK_FREE(left_list);
            }
            if (right_list)
            {
                EK_FREE(right_list->List_Dummy);
                EK_FREE(right_list);
            }
            return EK_NO_MEMORY;
        }

        // 分割链表
        EK_Result_t result = r_split_list(list, mid_node, left_list, right_list);
        if (result != EK_OK)
        {
            EK_FREE(left_list->List_Dummy);
            EK_FREE(left_list);
            EK_FREE(right_list->List_Dummy);
            EK_FREE(right_list);
            return result;
        }

        // 递归排序左右子链表
        result = EK_rListSort(left_list, is_descend);
        if (result != EK_OK)
        {
            // 将节点放回原链表
            while (left_list->List_Count > 0)
            {
                EK_Node_t *node = p_list_get_head(left_list);
                EK_rListRemoveNode(left_list, node);
                EK_rListInsertEnd(list, node);
            }
            while (right_list->List_Count > 0)
            {
                EK_Node_t *node = p_list_get_head(right_list);
                EK_rListRemoveNode(right_list, node);
                EK_rListInsertEnd(list, node);
            }
            EK_FREE(left_list->List_Dummy);
            EK_FREE(left_list);
            EK_FREE(right_list->List_Dummy);
            EK_FREE(right_list);
            return result;
        }

        result = EK_rListSort(right_list, is_descend);
        if (result != EK_OK)
        {
            // 将节点放回原链表
            while (left_list->List_Count > 0)
            {
                EK_Node_t *node = p_list_get_head(left_list);
                EK_rListRemoveNode(left_list, node);
                EK_rListInsertEnd(list, node);
            }
            while (right_list->List_Count > 0)
            {
                EK_Node_t *node = p_list_get_head(right_list);
                EK_rListRemoveNode(right_list, node);
                EK_rListInsertEnd(list, node);
            }
            EK_FREE(left_list->List_Dummy);
            EK_FREE(left_list);
            EK_FREE(right_list->List_Dummy);
            EK_FREE(right_list);
            return result;
        }

        // 合并排序后的左右子链表
        result = r_merge_list(left_list, right_list, list, is_descend);

        // 清理临时链表
        EK_FREE(left_list->List_Dummy);
        EK_FREE(left_list);
        EK_FREE(right_list->List_Dummy);
        EK_FREE(right_list);

        return result;
    }
#endif /* LIST_RECURSION_SORT != 0 */
}
