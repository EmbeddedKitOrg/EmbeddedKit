#include "Kernel.h"

#if (EK_CORO_ENABLE == 1)

extern EK_CoroList_t KernelReadyList[EK_CORO_PRIORITY_GROUPS]; // 就绪任务列表
extern EK_CoroList_t KernelBlockList1; // 阻塞任务列表 1
extern EK_CoroList_t KernelBlockList2; // 阻塞任务列表 2
extern EK_CoroList_t KernelSuspendList; // 挂起任务列表
extern volatile EK_BitMap_t KernelReadyBitMap; // 就绪链表位图

/* ========================= 链表函数控制区 ========================= */

/**
 * @brief 判断一个链表是不是在就绪链表中
 * 
 */
#define EK_KERNEL_IS_READY_LIST(X) ((X) >= KernelReadyList && (X) < KernelReadyList + EK_CORO_PRIORITY_GROUPS)

/**
 * @brief 初始化链表。
 * @details
 *  内部函数，用于初始化链表的哨兵节点，建立循环链表结构。
 *  哨兵节点指向自身，表示空链表状态。
 * @param list 要初始化的链表
 */
void EK_vKernelListInit(EK_CoroList_t *list)
{
    list->List_Count = 0;
    // 初始化哨兵节点，使其指向自身，形成循环链表
    list->List_Dummy.CoroNode_Next = (EK_CoroListNode_t *)&list->List_Dummy;
    list->List_Dummy.CoroNode_Prev = (EK_CoroListNode_t *)&list->List_Dummy;
}

/**
 * @brief 从一个链表中移除一个指定的协程节点。
 * @details
 *  此函数使用哨兵节点设计，可以从链表中高效移除节点，无需处理边界条件。
 *  哨兵节点保证了 prev 和 next 永远不为 NULL，大大简化了删除操作。
 *  移除后，节点的归属链表指针会被清空。
 *  如果一个就绪链表因此变为空，对应的就绪位图位也会被清除。
 * @param list 从中移除节点的链表。
 * @param node_to_remove 要移除的协程节点。
 * @return EK_Result_t 操作结果 (EK_OK, EK_NULL_POINTER, EK_NOT_FOUND)。
 */
EK_Result_t EK_rKernelRemove(EK_CoroList_t *list, EK_CoroListNode_t *node_to_remove)
{
    // 入参检查
    if (list->List_Count == 0) return EK_ERROR;
    if (node_to_remove->CoroNode_List != list) return EK_ERROR;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node_to_remove->CoroNode_Owner; // 获取TCB

    EK_ENTER_CRITICAL();

    // 利用哨兵节点的双向链表特性 (O(1) 删除)
    // 哨兵节点保证了 prev 和 next 永远不为 NULL，无需边界检查
    node_to_remove->CoroNode_Prev->CoroNode_Next = node_to_remove->CoroNode_Next;
    node_to_remove->CoroNode_Next->CoroNode_Prev = node_to_remove->CoroNode_Prev;

    // 更新链表信息
    list->List_Count--;

    // 更新节点信息
    node_to_remove->CoroNode_List = NULL;
    node_to_remove->CoroNode_Next = NULL;
    node_to_remove->CoroNode_Prev = NULL;

    // 如果就绪链表变为空，则清除对应的位图位
    if (EK_KERNEL_IS_READY_LIST(list) == true && list->List_Count == 0)
    {
        EK_vClearBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 将一个协程节点按唤醒时间升序插入到链表中。
 * @details
 *  使用哨兵节点设计，按照唤醒时间 `TCB_WakeUpTime` 升序插入节点。
 *  哨兵节点简化了边界条件处理，使插入逻辑更加清晰。
 *  主要用于将任务插入到阻塞链表 `EK_CoroKernelBlockList`。
 * @param list 目标链表。
 * @param node 要插入的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelInsert_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_Owner;

    EK_ENTER_CRITICAL();

    // 使用哨兵节点按时间顺序插入链表
    EK_CoroListNode_t *dummy = EK_pKernelListGetDummy(list);
    EK_CoroListNode_t *current = EK_pKernelListGetFirst(list);

    // 遍历链表找到合适的插入位置（按唤醒时间升序）
    while (current != dummy)
    {
        EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current->CoroNode_Owner;
        if (tcb->TCB_WakeUpTime < current_tcb->TCB_WakeUpTime) break;
        current = current->CoroNode_Next;
    }

    // 在 current 节点之前插入 node
    node->CoroNode_Next = current;
    node->CoroNode_Prev = current->CoroNode_Prev;
    current->CoroNode_Prev->CoroNode_Next = node;
    current->CoroNode_Prev = node;

    node->CoroNode_List = list;
    list->List_Count++;

    // 判断是不是就绪链表
    if (EK_KERNEL_IS_READY_LIST(list) == true)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();
    return EK_OK;
}

/**
 * @brief 将一个协程节点插入到链表的尾部。
 * @details
 *  使用哨兵节点设计，以 O(1) 的时间复杂度将节点添加到链表的末尾。
 *  哨兵节点消除了空链表的边界检查，使代码更简洁高效。
 *  主要用于将任务添加到就绪链表 `KernelReadyList` 或挂起链表 `KernelSuspendList`。
 * @param list 目标链表。
 * @param node 要插入的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelInsert_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_Owner;

    EK_ENTER_CRITICAL();

    // 使用宏进行尾部插入 (O(1) 操作)
    EK_CoroListNode_t *last_node = EK_pKernelListGetLast(list);

    node->CoroNode_Next = last_node->CoroNode_Next;
    node->CoroNode_Prev = last_node;
    last_node->CoroNode_Next->CoroNode_Prev = node;
    last_node->CoroNode_Next = node;

    node->CoroNode_List = list;
    list->List_Count++;

    // 判断是不是就绪链表
    if (EK_KERNEL_IS_READY_LIST(list) == true)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();
    return EK_OK;
}

/**
 * @brief 将一个协程节点插入到链表的头部。
 * @details
 *  使用哨兵节点设计，以 O(1) 的时间复杂度将节点插入到链表的开始位置。
 *  哨兵节点消除了空链表的边界检查，使代码更简洁高效。
 *  常用于需要优先处理某些任务的场景。
 * @param list 目标链表。
 * @param node 要插入的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelInsert_Head(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_Owner;

    EK_ENTER_CRITICAL();

    // 使用哨兵节点进行头部插入 (O(1) 操作)
    // 在哨兵节点之后插入，相当于插入到链表头部
    EK_CoroListNode_t *dummy = EK_pKernelListGetDummy(list);

    node->CoroNode_Next = dummy->CoroNode_Next;
    node->CoroNode_Prev = dummy;
    dummy->CoroNode_Next->CoroNode_Prev = node;
    dummy->CoroNode_Next = node;

    node->CoroNode_List = list;
    list->List_Count++;

    // 判断是不是就绪链表
    if (EK_KERNEL_IS_READY_LIST(list) == true)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();
    return EK_OK;
}

/**
 * @brief 将一个协程节点按优先级升序插入到链表中。
 * @details
 *  使用哨兵节点设计，按照优先级 `TCB_Priority` 升序插入节点（值越小优先级越高）。
 *  哨兵节点简化了边界条件处理，使插入逻辑更加清晰。
 * @param list 目标链表。
 * @param node 要插入的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelInsert_Prio(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)node->CoroNode_Owner;

    EK_ENTER_CRITICAL();

    // 使用哨兵节点按优先级顺序插入链表
    EK_CoroListNode_t *dummy = EK_pKernelListGetDummy(list);
    EK_CoroListNode_t *current = EK_pKernelListGetFirst(list);

    // 遍历链表找到合适的插入位置（按优先级升序，值越小优先级越高）
    while (current != dummy)
    {
        EK_CoroTCB_t *current_tcb = (EK_CoroTCB_t *)current->CoroNode_Owner;
        if (tcb->TCB_Priority < current_tcb->TCB_Priority) break; // 找到了插入点 (新任务优先级更高)
        current = current->CoroNode_Next;
    }

    // 在 current 节点之前插入 node
    node->CoroNode_Next = current;
    node->CoroNode_Prev = current->CoroNode_Prev;
    current->CoroNode_Prev->CoroNode_Next = node;
    current->CoroNode_Prev = node;

    node->CoroNode_List = list;
    list->List_Count++;

    // 判断是不是就绪链表
    if (EK_KERNEL_IS_READY_LIST(list) == true)
    {
        EK_vSetBit(&KernelReadyBitMap, EK_BITMAP_MAX_BIT - tcb->TCB_Priority);
    }

    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 将一个协程节点移动到另一个链表，并按唤醒时间升序排序。
 * @details
 *  此函数将节点从当前链表移除，然后按唤醒时间 `TCB_WakeUpTime` 升序插入到目标链表中。
 *  常用于将任务从就绪链表移动到阻塞链表。
 * @param list 目标链表。
 * @param node 要移动的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelMove_WakeUpTime(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (node->CoroNode_List != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)node->CoroNode_List, node);
        if (remove_res != EK_OK) return remove_res;
    }

    return EK_rKernelInsert_WakeUpTime(list, node);
}

/**
 * @brief 将一个协程节点移动到另一个链表的尾部。
 * @details
 *  此函数将节点从当前链表移除，然后插入到目标链表的末尾。
 *  常用于将任务从阻塞/挂起链表移回到就绪链表。
 * @param list 目标链表。
 * @param node 要移动的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelMove_Tail(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (node->CoroNode_List != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)node->CoroNode_List, node);
        if (remove_res != EK_OK) return remove_res;
    }

    return EK_rKernelInsert_Tail(list, node);
}

/**
 * @brief 将一个协程节点移动到另一个链表，并按优先级升序排序。
 * @details
 *  此函数将节点从当前链表移除，然后按任务优先级 `TCB_Priority` 升序插入到目标链表中。
 * @param list 目标链表。
 * @param node 要移动的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelMove_Prio(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (node->CoroNode_List != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)node->CoroNode_List, node);
        if (remove_res != EK_OK) return remove_res;
    }

    return EK_rKernelInsert_Prio(list, node);
}

/**
 * @brief 将一个协程节点移动到另一个链表的头部。
 * @details
 *  此函数将节点从当前链表移除，然后插入到目标链表的开始位置。
 *  常用于将任务优先插入到就绪链表，使其尽快得到调度。
 * @param list 目标链表。
 * @param node 要移动的协程节点。
 * @return EK_Result_t 操作结果。
 */
EK_Result_t EK_rKernelMove_Head(EK_CoroList_t *list, EK_CoroListNode_t *node)
{
    if (node->CoroNode_List != NULL)
    {
        EK_Result_t remove_res = EK_rKernelRemove((EK_CoroList_t *)node->CoroNode_List, node);
        if (remove_res != EK_OK) return remove_res;
    }

    return EK_rKernelInsert_Head(list, node);
}

#endif /*EK_CORO_ENABLE == 1*/
