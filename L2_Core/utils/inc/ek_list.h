/**
 * @file ek_list.h
 * @brief 双向循环链表（Linux 内核风格）
 *
 * 提供轻量级的双向循环链表实现，所有操作为 O(1) 时间复杂度
 */

#ifndef EK_LIST_H
#define EK_LIST_H

#include "../../../ek_conf.h"

#if EK_LIST_ENABLE == 1

#include "ek_def.h"

/**
 * @brief 链表节点结构
 */
typedef struct ek_list_node_t ek_list_node_t;

struct ek_list_node_t
{
    ek_list_node_t *prev; /**< 前驱节点 */
    ek_list_node_t *next; /**< 后继节点 */
};

/* clang-format off */

/**
 * @brief 根据链表节点指针获取包含它的结构体指针
 * @param ptr 链表节点指针
 * @param type 包含该节点的结构体类型
 * @param member 链表节点在结构体中的成员名
 * @return 包含该节点的结构体指针
 *
 * @example
 * struct user_t {
 *     char name[32];
 *     ek_list_node_t list;  // 链表节点成员
 * };
 * ek_list_node_t *node = ...;
 * struct user_t *user = ek_list_container(node, struct user_t, list);
 */
#define ek_list_container(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))

/**
 * @brief 正向遍历链表
 * @param pos 当前节点指针（迭代变量）
 * @param head 链表头节点
 *
 * @warning 遍历过程中不能删除当前节点，否则使用 ek_list_iterate_safe
 */
#define ek_list_iterate(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * @brief 安全遍历链表（允许在遍历时删除节点）
 * @param pos 当前节点指针（迭代变量）
 * @param n 临时节点指针（保存下一个节点）
 * @param head 链表头节点
 */
#define ek_list_iterate_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/* clang-format on */

/**
 * @brief 初始化链表头
 * @param head 链表头节点指针
 *
 * @note 使用前必须初始化，使头节点的 prev 和 next 都指向自己
 */
__STATIC_INLINE void ek_list_init(ek_list_node_t *head)
{
    head->next = head;
    head->prev = head;
}

/**
 * @brief 内部函数：在两个节点之间插入新节点
 * @param new 要插入的新节点
 * @param prev 前一个节点（插入位置之前）
 * @param next 后一个节点（插入位置之后）
 */
__STATIC_INLINE void __ek_list_add(ek_list_node_t *new, ek_list_node_t *prev, ek_list_node_t *next)
{
    prev->next = new;
    new->next = next;
    new->prev = prev;
    next->prev = new;
}

/**
 * @brief 在链表头部添加节点
 * @param head 链表头节点指针
 * @param new 要添加的新节点
 */
__STATIC_INLINE void ek_list_add_head(ek_list_node_t *head, ek_list_node_t *new)
{
    __ek_list_add(new, head, head->next);
}

/**
 * @brief 在链表尾部添加节点
 * @param head 链表头节点指针
 * @param new 要添加的新节点
 */
__STATIC_INLINE void ek_list_add_tail(ek_list_node_t *head, ek_list_node_t *new)
{
    __ek_list_add(new, head->prev, head);
}

/**
 * @brief 内部函数：断开两个节点之间的连接
 * @param prev 前一个节点
 * @param next 后一个节点
 */
__STATIC_INLINE void __ek_list_remove(ek_list_node_t *prev, ek_list_node_t *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * @brief 从链表中移除节点
 * @param remove 要移除的节点指针
 *
 * @note 移除后节点的 prev 和 next 被置为 NULL，可用于检测节点是否在链表中
 */
__STATIC_INLINE void ek_list_remove(ek_list_node_t *remove)
{
    __ek_list_remove(remove->prev, remove->next);
    remove->prev = NULL;
    remove->next = NULL;
}

/**
 * @brief 判断链表是否为空
 * @param head 链表头节点指针
 * @return true 链表为空
 * @return false 链表非空
 */
__STATIC_INLINE bool ek_list_empty(ek_list_node_t *head)
{
    return head->next == head;
}

/**
 * @brief 判断节点是否是链表的最后一个节点
 * @param list 要检查的节点
 * @param head 链表头节点指针
 * @return true 是最后一个节点
 * @return false 不是最后一个节点
 */
__STATIC_INLINE bool ek_list_is_last(ek_list_node_t *list, ek_list_node_t *head)
{
    return list->next == head;
}

/**
 * @brief 获取链表的第一个节点
 * @param head 链表头节点指针
 * @return 第一个节点指针，如果链表为空则返回 head
 */
__STATIC_INLINE ek_list_node_t *ek_list_get_first(ek_list_node_t *head)
{
    return head->next;
}

/**
 * @brief 获取链表的最后一个节点
 * @param head 链表头节点指针
 * @return 最后一个节点指针，如果链表为空则返回 head
 */
__STATIC_INLINE ek_list_node_t *ek_list_get_last(ek_list_node_t *head)
{
    return head->prev;
}

#endif /* EK_LIST_ENABLE */

#endif /* EK_LIST_H */
