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
static const uint32_t __DUMMY__ = 0xABCD1234;

/* ========================= 内部函数定义区 ========================= */

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
    if (list->List_Count == 1) return GET_FIRST_NODE(list);

    EK_Node_t *p_fast, *p_slow; //快慢指针
    p_fast = p_slow = GET_FIRST_NODE(list);

    while (p_fast != list->List_Dummy && p_fast->Node_Next != list->List_Dummy)
    {
        // 慢指针移动一个节点
        p_slow = p_slow->Node_Next;
        // 快指针移动两个节点
        p_fast = p_fast->Node_Next->Node_Next;
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
    EK_Node_t *current = GET_FIRST_NODE(list);

    // 遍历到分割点，计算左半部分节点数
    while (current != node && current != list->List_Dummy)
    {
        left_count++;
        current = current->Node_Next;
    }

    if (current != node) return EK_NOT_FOUND; // 节点不在链表中

    left_count++; // 包含分割节点本身在左半部分

    // 如果分割点是第一个节点，左半部分只有一个节点
    if (left_count == 1)
    {
        // 将第一个节点移到左链表
        EK_Node_t *first_node = GET_FIRST_NODE(list);
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
            list_right->List_Dummy->Node_Prev = GET_LAST_NODE(list);

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
        EK_Node_t *left_head = GET_FIRST_NODE(list);
        EK_Node_t *left_tail = node;
        EK_Node_t *right_head = node->Node_Next;
        EK_Node_t *right_tail = GET_LAST_NODE(list);

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
        while (list2->List_Count > 0)
        {
            EK_Result_t temp = EK_OK;
            EK_Node_t *node = GET_FIRST_NODE(list2);
            temp = EK_rListRemoveNode(list2, node);
            temp = EK_rListInsertEnd(list_merged, node);
            if (temp != EK_OK) return temp;
        }
        return EK_OK;
    }

    if (list2->List_Count == 0)
    {
        // 将list1的所有节点移动到merged链表
        while (list1->List_Count > 0)
        {
            EK_Result_t temp = EK_OK;
            EK_Node_t *node = GET_FIRST_NODE(list1);
            temp = EK_rListRemoveNode(list1, node);
            temp = EK_rListInsertEnd(list_merged, node);
            if (temp != EK_OK) return temp;
        }
        return EK_OK;
    }

    // 双链表合并
    EK_Node_t *p1 = GET_FIRST_NODE(list1); // list1当前节点
    EK_Node_t *p2 = GET_FIRST_NODE(list2); // list2当前节点

    while (p1 != list1->List_Dummy && p2 != list2->List_Dummy)
    {
        EK_Node_t *selected_node = NULL;
        bool choose_p1 = false;

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
            p1 = p1->Node_Next; // 移动到下一个节点
            EK_rListRemoveNode(list1, selected_node);
            // 检查p1是否仍然有效
            if (list1->List_Count == 0) p1 = list1->List_Dummy;
        }
        else
        {
            selected_node = p2;
            p2 = p2->Node_Next; // 移动到下一个节点
            EK_rListRemoveNode(list2, selected_node);
            // 检查p2是否仍然有效
            if (list2->List_Count == 0) p2 = list2->List_Dummy;
        }

        // 将选中的节点插入到合并链表的尾部
        EK_rListInsertEnd(list_merged, selected_node);
    }

    // 处理剩余节点
    // 如果list1还有剩余节点
    while (list1->List_Count > 0)
    {
        EK_Result_t temp = EK_OK;
        EK_Node_t *node = GET_FIRST_NODE(list1);
        temp = EK_rListRemoveNode(list1, node);
        temp = EK_rListInsertEnd(list_merged, node);
        if (temp != EK_OK) return temp;
    }

    // 如果list2还有剩余节点
    while (list2->List_Count > 0)
    {
        EK_Result_t temp = EK_OK;
        EK_Node_t *node = GET_FIRST_NODE(list2);
        temp = EK_rListRemoveNode(list2, node);
        temp = EK_rListInsertEnd(list_merged, node);
        if (temp != EK_OK) return temp;
    }

    return EK_OK;
}
#endif
/* ========================= 公用API函数定义区 ========================= */
/**
 * @brief 静态创建节点
 * @details 在已分配的节点内存上初始化节点数据
 * @param node 指向已分配内存的节点指针
 * @param content 节点存储的内容指针
 * @param order 节点序号
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rNodeCreate_Static(EK_Node_t *node, void *content, uint32_t order)
{
    if (node == NULL || content == NULL) return EK_NULL_POINTER;

    node->Node_Data = content;
    node->Node_Order = order;
    node->Node_Next = NULL;
    node->Node_Prev = NULL;
    node->Node_Owner = NULL;

    return EK_OK;
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
EK_Node_t *EK_pNodeCreate_Dynamic(void *content, uint32_t order)
{
    if (content == NULL) return NULL;
    EK_Node_t *node = (EK_Node_t *)_MALLOC(sizeof(EK_Node_t));

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

    return node;
}

/**
 * @brief 静态创建链表
 * @details 在已分配的链表内存上初始化链表
 * @param list 指向已分配内存的链表指针
 * @param dummy_node 哨兵节点
 * @return EK_Result_t 操作结果
 */
EK_Result_t EK_rListCreate_Static(EK_List_t *list, EK_Node_t *dummy_node)
{
    if (list == NULL) return EK_NULL_POINTER;

    // 初始化哨兵节点
    list->List_Dummy = dummy_node;

    return r_list_init(list);
}

/**
 * @brief 动态创建链表
 * @details 动态分配内存并初始化链表，设置头节点
 * @return EK_List_t* 创建的链表指针
 * @retval 非NULL 创建成功，返回链表指针
 * @retval NULL 创建失败（参数为空或内存分配失败）
 */
EK_List_t *EK_pListCreate_Dynamic(void)
{
    EK_List_t *list = (EK_List_t *)_MALLOC(sizeof(EK_List_t));

    if (list == NULL)
    {
        return NULL;
    }

    // 初始化哨兵节点
    list->List_Dummy = (EK_Node_t *)_MALLOC(sizeof(EK_Node_t));
    if (list->List_Dummy == NULL)
    {
        _FREE(list);
        return NULL;
    }

    // 初始化链表
    if (r_list_init(list) != EK_OK) return NULL;

    return list;
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

    EK_Node_t *temp = GET_LAST_NODE(list);
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

    EK_Node_t *temp = GET_FIRST_NODE(list);
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
    if (node->Node_Order <= GET_FIRST_NODE(list)->Node_Order)
    {
        return EK_rListInsertHead(list, node);
    }

    // 比尾节点都大
    if (node->Node_Order >= GET_LAST_NODE(list)->Node_Order)
    {
        return EK_rListInsertEnd(list, node);
    }

    EK_Node_t *p = GET_FIRST_NODE(list);

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
                return EK_OK;
            }
            return EK_NOT_FOUND;
        }
    }

    // 是头节点
    if (node == GET_FIRST_NODE(list))
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
    if (node == GET_LAST_NODE(list))
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
    if (list->List_Count < 20)
    {
#endif
        EK_Node_t *current = GET_FIRST_NODE(list);

        while (current != list->List_Dummy && current->Node_Next != list->List_Dummy)
        {
            EK_Node_t *min_max_node = current;
            EK_Node_t *search_node = current->Node_Next;

            // 在剩余节点中找到最小值（升序）或最大值（降序）
            while (search_node != list->List_Dummy)
            {
                bool should_select = is_descend ? (search_node->Node_Order > min_max_node->Node_Order)
                                                : (search_node->Node_Order < min_max_node->Node_Order);

                if (should_select)
                {
                    min_max_node = search_node;
                }
                search_node = search_node->Node_Next;
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
        EK_List_t *left_list = EK_pListCreate_Dynamic();
        EK_List_t *right_list = EK_pListCreate_Dynamic();
        if (left_list == NULL || right_list == NULL)
        {
            if (left_list)
            {
                _FREE(left_list->List_Dummy);
                _FREE(left_list);
            }
            if (right_list)
            {
                _FREE(right_list->List_Dummy);
                _FREE(right_list);
            }
            return EK_NO_MEMORY;
        }

        // 分割链表
        EK_Result_t result = r_split_list(list, mid_node, left_list, right_list);
        if (result != EK_OK)
        {
            _FREE(left_list->List_Dummy);
            _FREE(left_list);
            _FREE(right_list->List_Dummy);
            _FREE(right_list);
            return result;
        }

        // 递归排序左右子链表
        result = EK_rListSort(left_list, is_descend);
        if (result != EK_OK)
        {
            // 将节点放回原链表
            while (left_list->List_Count > 0)
            {
                EK_Node_t *node = GET_FIRST_NODE(left_list);
                EK_rListRemoveNode(left_list, node);
                EK_rListInsertEnd(list, node);
            }
            while (right_list->List_Count > 0)
            {
                EK_Node_t *node = GET_FIRST_NODE(right_list);
                EK_rListRemoveNode(right_list, node);
                EK_rListInsertEnd(list, node);
            }
            _FREE(left_list->List_Dummy);
            _FREE(left_list);
            _FREE(right_list->List_Dummy);
            _FREE(right_list);
            return result;
        }

        result = EK_rListSort(right_list, is_descend);
        if (result != EK_OK)
        {
            // 将节点放回原链表
            while (left_list->List_Count > 0)
            {
                EK_Node_t *node = GET_FIRST_NODE(left_list);
                EK_rListRemoveNode(left_list, node);
                EK_rListInsertEnd(list, node);
            }
            while (right_list->List_Count > 0)
            {
                EK_Node_t *node = GET_FIRST_NODE(right_list);
                EK_rListRemoveNode(right_list, node);
                EK_rListInsertEnd(list, node);
            }
            _FREE(left_list->List_Dummy);
            _FREE(left_list);
            _FREE(right_list->List_Dummy);
            _FREE(right_list);
            return result;
        }

        // 合并排序后的左右子链表
        result = r_merge_list(left_list, right_list, list, is_descend);

        // 清理临时链表
        _FREE(left_list->List_Dummy);
        _FREE(left_list);
        _FREE(right_list->List_Dummy);
        _FREE(right_list);

        return result;
    }
#endif
}
