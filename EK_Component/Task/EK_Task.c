/**
 * @file EK_Task.c
 * @brief 基于双链表+内存池实现的轻量级任务调度器
 * @details 实现了一个非抢占式任务调度器，具有以下特性：
 *          - 基于优先级的任务调度（数值越小优先级越高）
 *          - 支持任务的创建、删除、挂起、恢复等基本操作
 *          - 支持任务延时功能
 *          - 支持静态和动态任务创建
 *          - 使用双链表管理运行队列和等待队列
 *          - 集成内存池进行动态内存管理
 *          - 提供任务状态查询和优先级调整功能
 *          - 支持用户自定义空闲任务和错误处理
 * 
 * @warning 任务函数不能使用阻塞操作，应使用rTaskDelay()进行延时
 * 
 * @author N1ntyNine99
 * @date 2025-09-04
 * @version v2.0
 */
#include "EK_Task.h"
#include "../MemPool/EK_MemPool.h"

/* ========================= 宏定义区 ========================= */
/**
 * @brief 任务倒计时操作宏定义
 * @note Task_TrigTime的高16位存储设定值，低16位存储当前值
 */
#define TASK_SET_TRIG_TIME(set_val, cur_val)  (((uint32_t)(set_val) << 16) | ((uint32_t)(cur_val) & 0xFFFF))
#define TASK_GET_SET_TIME(trig_time)          ((uint16_t)((trig_time) >> 16))
#define TASK_GET_CUR_TIME(trig_time)          ((uint16_t)((trig_time) & 0xFFFF))
#define TASK_SET_CUR_TIME(trig_time, cur_val) (((trig_time) & 0xFFFF0000) | ((uint32_t)(cur_val) & 0xFFFF))
#define TASK_RESET_TIME(trig_time)            (((trig_time) & 0xFFFF0000) | (((trig_time) >> 16) & 0xFFFF))

/**
 * @brief 任务状态操作宏定义
 * @note Task_isActive字段位域定义：
 *       - 最高位(bit7): 是否静态创建 (1=静态, 0=动态)
 *       - bit6-bit1:   保留位
 *       - 最低位(bit0): 是否激活 (1=激活, 0=挂起)
 */
#define TASK_STATE_STATIC_MASK   (0x80) /**< 静态创建标志位掩码 */
#define TASK_STATE_ACTIVE_MASK   (0x01) /**< 激活状态掩码 */
#define TASK_STATE_RESERVED_MASK (0x7E) /**< 保留位掩码 */

/* 静态创建标志操作 */
#define TASK_SET_STATIC(state)  ((state) | TASK_STATE_STATIC_MASK) /**< 设置为静态创建 */
#define TASK_SET_DYNAMIC(state) ((state) & ~TASK_STATE_STATIC_MASK) /**< 设置为动态创建 */
#define TASK_IS_STATIC(state)   (((state) & TASK_STATE_STATIC_MASK) != 0) /**< 检查是否静态创建 */

/* 激活状态操作 */
#define TASK_SET_ACTIVE(state)    ((state) | TASK_STATE_ACTIVE_MASK) /**< 设置为激活状态 */
#define TASK_SET_SUSPENDED(state) ((state) & ~TASK_STATE_ACTIVE_MASK) /**< 设置为挂起状态 */
#define TASK_IS_ACTIVE(state)     (((state) & TASK_STATE_ACTIVE_MASK) != 0) /**< 检查是否激活 */

/* 综合操作 */
#define TASK_INIT_STATE(is_static, is_active) \
    ((is_static) ? TASK_STATE_STATIC_MASK : 0) | ((is_active) ? TASK_STATE_ACTIVE_MASK : 0)

/* 兼容性宏定义 - 用于替换原有的bool操作 */
#define TASK_GET_ACTIVE_STATE(handler) TASK_IS_ACTIVE((handler)->Task_Info)
#define TASK_ACTIVATE(handler)         ((handler)->Task_Info = TASK_SET_ACTIVE((handler)->Task_Info))
#define TASK_SUSPEND(handler)          ((handler)->Task_Info = TASK_SET_SUSPENDED((handler)->Task_Info))

/**
 * @brief 任务调度器错误代码枚举
 */
typedef enum
{
    TASK_ERR_TICK_NULL = 1, /**< tick获取函数为空 */
    TASK_ERR_WAIT_TO_RUN, /**< 等待链表到运行链表移动失败 */
    TASK_ERR_RUN_TO_WAIT, /**< 运行链表到等待链表移动失败 */
    TASK_ERR_MEMORY_CORRUPT, /**< 内存损坏 */
    TASK_ERR_LIST_CORRUPT /**< 链表结构损坏 */
} EK_TaskErrorCode_t;

/**
 * @brief 任务调度器结构体
 * @note 用于管理任务的执行链表和等待链表
 */
typedef struct
{
    EK_TaskEK_Node_t *Head; // 链表头指针
    EK_TaskEK_Node_t *Tail; // 链表尾指针
    uint16_t Count; // 节点数量
} EK_TaskSchedule_t;

typedef EK_TaskSchedule_t *EK_pTaskSchedule_t; // 调度链表指针(aka EK_TaskSchedule_t*)

/* ========================= 内部全局变量区 ========================= */
static EK_TaskSchedule_t RunSchedule; // 任务执行链表
static EK_TaskSchedule_t WaitSchedule; // 任务等待链表
static EK_TaskHandler_t *CurTaskHandler; //当前正在占用CPU的任务的句柄

/* ========================= 弱定义函数区 ========================= */
/**
 * @brief 添加任务初始化
 * @note 需要在别处重新实现 默认是返回错误的
 * 
 * @return  
 */
__weak bool TaskCreation(void)
{
    return false;
}

/**
 * @brief 空闲函数
 * 
 * @note 在调度器空闲的时候会调用的函数
 * 
 * @warning 不要尝试在该函数中进行一些耗费时间的任务!!!
 */

__weak void TaskIdle(void)
{
}

/* ========================= 内部函数定义区 ========================= */
/**
 * @brief 任务调度器出错处理函数
 * @param error_code 错误代码
 * @note 不同的错误代码会进入不同的死循环，便于调试时定位问题
 * 调试时可以在各个循环处打断点来确定具体的错误原因
 */
static void Task_Error(EK_TaskErrorCode_t error_code)
{
    switch (error_code)
    {
        case TASK_ERR_TICK_NULL: // tick获取函数为空
            while (1)
            {
                EK_TaskErrorCode_t code = (EK_TaskErrorCode_t)1;
                UNUSED_VAR(code);
                // 错误1: tick_get函数指针为NULL
                // 检查Task_Start()调用时是否传入了有效的tick获取函数
            }

        case TASK_ERR_WAIT_TO_RUN: // 等待链表到运行链表移动失败
            while (1)
            {
                EK_TaskErrorCode_t code = (EK_TaskErrorCode_t)2;
                UNUSED_VAR(code);
                // 错误2: 任务从等待链表移动到运行链表失败
                // 可能原因: 链表结构被破坏或内存问题
            }

        case TASK_ERR_RUN_TO_WAIT: // 运行链表到等待链表移动失败
            while (1)
            {
                EK_TaskErrorCode_t code = (EK_TaskErrorCode_t)3;
                UNUSED_VAR(code);
                // 错误3: 任务从运行链表移动到等待链表失败
                // 可能原因: 链表结构被破坏或内存问题
            }

        case TASK_ERR_MEMORY_CORRUPT: // 内存损坏
            while (1)
            {
                EK_TaskErrorCode_t code = (EK_TaskErrorCode_t)4;
                UNUSED_VAR(code);
                // 错误4: 内存池或任务节点内存损坏
                // 检查内存池完整性或任务节点是否被意外修改
            }

        case TASK_ERR_LIST_CORRUPT: // 链表结构损坏
            while (1)
            {
                EK_TaskErrorCode_t code = (EK_TaskErrorCode_t)5;
                UNUSED_VAR(code);
                // 错误5: 链表结构损坏
                // 检查链表头、尾指针和计数是否一致
            }

        default: // 未知错误
            while (1)
            {
                EK_TaskErrorCode_t code = (EK_TaskErrorCode_t)0xFF; // 使用有效值替代-1
                UNUSED_VAR(code);
                // 未知错误: 检查调用Task_Error()的地方传入的错误代码
            }
    }
}

/**
 * @brief 将任务节点按优先级插入到链表中
 * @param list 目标链表
 * @param node 要插入的节点
 * @return EK_Result_t 插入结果
 * @note 这是一个通用的内部函数，只负责按优先级排序插入，不负责内存分配
 */
static EK_Result_t r_task_insert_node(EK_TaskSchedule_t *list, EK_TaskEK_Node_t *node)
{
    if (node == NULL || list == NULL) return EK_NULL_POINTER;

    node->Next = NULL;

    // 如果是空链表 直接在表头添加这个任务节点
    if (list->Count == 0)
    {
        list->Head = node;
        node->Next = NULL;
        list->Tail = node;
        list->Count = 1;
        node->Owner = list;
        return EK_OK;
    }

    // 链表遍历指针
    EK_TaskEK_Node_t *p = list->Head;

    // 查看优先级是否高于链表头 越小越高
    if (node->TaskHandler.Task_Priority < list->Head->TaskHandler.Task_Priority)
    {
        node->Next = list->Head;
        list->Head = node;
        list->Count++;
        node->Owner = list;
        return EK_OK;
    }

    while (p->Next != NULL)
    {
        EK_TaskEK_Node_t *p_next = p->Next;
        // 找到优先级恰当的地方
        if (p_next->TaskHandler.Task_Priority >= node->TaskHandler.Task_Priority)
        {
            p->Next = node;
            node->Next = p_next;
            list->Count++;
            node->Owner = list;
            return EK_OK;
        }
        // 当前位置不匹配 移动向下一个节点
        p = p->Next;
    }
    // 没找到大于当前优先级值的位置
    // 将当前节点插入尾部
    p->Next = node;
    node->Next = NULL;
    list->Tail = node;
    list->Count++;
    node->Owner = list;
    return EK_OK;
}

/**
 * @brief 将任务节点从链表中移除
 * @param list 目标链表
 * @param node 要移除的节点
 * @return EK_Result_t 插入结果
 */
static EK_Result_t r_task_remove_node(EK_TaskSchedule_t *list, EK_TaskEK_Node_t *node)
{
    if (node == NULL || list == NULL) return EK_NULL_POINTER;
    if (list->Count == 0) return EK_NOT_FOUND;

    // 只有一个节点
    if (list->Count == 1)
    {
        if (list->Head == node)
        {
            list->Head = NULL;
            list->Tail = NULL;
            node->Next = NULL;
            node->Owner = NULL;
            list->Count = 0;
            return EK_OK;
        }
        return EK_NOT_FOUND;
    }

    // 如果要删除的是头节点
    if (list->Head == node)
    {
        list->Head = node->Next;
        list->Count--;
        node->Next = NULL;
        node->Owner = NULL;
        return EK_OK;
    }

    // 遍历查找要删除的节点
    EK_TaskEK_Node_t *p = list->Head;
    while (p->Next != NULL)
    {
        if (p->Next == node)
        {
            // 找到了要删除的节点
            p->Next = node->Next;

            // 如果删除的是尾节点，更新尾指针
            if (node == list->Tail)
            {
                list->Tail = p;
            }

            list->Count--;
            node->Next = NULL;
            node->Owner = NULL;
            return EK_OK;
        }
        p = p->Next;
    }

    // 没有找到指定节点
    return EK_NOT_FOUND;
}

/**
 * @brief 将某一个节点从 list_src 移动到 list_dst
 * 
 * @param list_src 节点来源
 * @param list_dst 节点目的地
 * @param node 节点
 * @return EK_Result_t 执行情况
 */
static EK_Result_t r_task_move_node(EK_TaskSchedule_t *list_src, EK_TaskSchedule_t *list_dst, EK_TaskEK_Node_t *node)
{
    if (node == NULL || list_src == NULL || list_dst == NULL) return EK_NULL_POINTER;
    if (list_src == list_dst) return EK_INVALID_PARAM;

    EK_Result_t result = r_task_remove_node(list_src, node);
    if (result != EK_OK) return result;

    result = r_task_insert_node(list_dst, node);
    if (result != EK_OK) return result;

    return EK_OK;
}

/**
 * @brief 在链表中查询一个任务
 * 
 * @param list 查询的链表
 * @param task_handler 查询的任务句柄
 * @param node 获取节点的指针的指针
 * @return EK_Result_t 执行情况
 */
static EK_Result_t r_task_search_node(EK_TaskSchedule_t *list, EK_TaskHandler_t *task_handler, EK_pTaskEK_Node_t *node)
{
    *node = NULL; // 节点默认为NULL

    if (task_handler == NULL || list == NULL) return EK_NULL_POINTER;

    // 利用双向关联特性快速验证 - 如果Task_OwnerNode有效，可以直接验证
    if (task_handler->Task_OwnerNode != NULL)
    {
        EK_pTaskEK_Node_t candidate_node = (EK_pTaskEK_Node_t)task_handler->Task_OwnerNode;

        // 验证双向关联的一致性
        if (&(candidate_node->TaskHandler) == task_handler && candidate_node->Owner == list)
        {
            // 双向关联一致，直接返回结果 - O(1)复杂度
            *node = candidate_node;
            return EK_OK;
        }
    }

    // 如果双向关联无效或不一致，回退到链表遍历 - O(n)复杂度
    EK_TaskEK_Node_t *p = list->Head;
    while (p != NULL)
    {
        // 找到对应节点
        if (&(p->TaskHandler) == task_handler)
        {
            *node = p;
            // 发现数据不一致时，修复双向关联
            if (task_handler->Task_OwnerNode != p)
            {
                task_handler->Task_OwnerNode = p; // 修复Task_OwnerNode
            }
            if (p->Owner != list)
            {
                p->Owner = list; // 修复Owner
            }
            return EK_OK;
        }

        p = p->Next;
    }

    return EK_NOT_FOUND;
}
/* ========================= 公用API函数定义区 ========================= */
/**
 * @brief 初始化任务系统
 * @return EK_Result_t 初始化结果
 */
EK_Result_t EK_rTaskInit(void)
{
    // 初始化内存池
    if (EK_bMemPool_Init() == false) return EK_NOT_INITIALIZED;

    // 链表头初始化
    RunSchedule.Head = NULL;
    RunSchedule.Tail = NULL;
    RunSchedule.Count = 0;

    WaitSchedule.Head = NULL;
    WaitSchedule.Tail = NULL;
    WaitSchedule.Count = 0;

    // 添加任务初始化
    if (TaskCreation() == false) return EK_ERROR;

    return EK_OK;
}

/**
 * @brief 静态添加一个任务（节点内存由用户提供）
 * @param node 用户提供的节点内存
 * @param static_handler 用户的静态任务句柄
 * @return EK_pTaskHandler_t 返回节点中的任务句柄指针，失败返回NULL
 * @note 适用于静态分配场景，节点内存和函数指针都由用户管理，不使用内存池
 */
EK_pTaskHandler_t EK_pTaskCreate_Static(EK_TaskEK_Node_t *node, EK_TaskHandler_t *static_handler)
{
    if (node == NULL || static_handler == NULL) return NULL;

    // 先复制用户的句柄内容到节点中
    node->TaskHandler = *static_handler;
    node->Next = NULL;
    node->Owner = &WaitSchedule;

    // 强制覆盖Task_Info值 - 静态创建，激活状态
    node->TaskHandler.Task_Info = TASK_INIT_STATE(true, true);
    node->TaskHandler.Task_OwnerNode = node;

    // 插入到等待队列
    if (r_task_insert_node(&WaitSchedule, node) != EK_OK)
    {
        return NULL;
    }

    // 返回节点中的句柄指针
    return &(node->TaskHandler);
}

/**
 * @brief 动态添加一个任务（使用内存池分配节点）
 * @param pfunc 任务回调函数
 * @param Priority 任务优先级
 * @param task_handler 用于接收任务句柄指针的指针
 * @return EK_Result_t 添加结果
 * @note 适用于动态分配场景，节点内存由内存池管理，需要使用rTaskDelay设置延时
 */
EK_Result_t EK_rTaskCreate_Dynamic(void (*pfunc)(void), uint8_t Priority, EK_pTaskHandler_t *task_handler)
{
    if (pfunc == NULL) return EK_NULL_POINTER;

    EK_TaskEK_Node_t *node = (EK_TaskEK_Node_t *)EK_MALLOC(sizeof(EK_TaskEK_Node_t));
    if (node == NULL)
    {
        return EK_NO_MEMORY;
    }

    // 从内存池分配函数指针空间
    void (**func_ptr)(void) = (void (**)(void))EK_MALLOC(sizeof(void (*)(void)));
    if (func_ptr == NULL)
    {
        EK_FREE(node); // 释放已分配的节点内存
        return EK_NO_MEMORY;
    }

    node->Next = NULL;
    node->Owner = &WaitSchedule;
    // 强制覆盖用户可能设置的Task_Info值 - 动态创建，激活状态
    node->TaskHandler.Task_Info = TASK_INIT_STATE(false, true);
    node->TaskHandler.Task_MaxUsed = 0;
    node->TaskHandler.Task_OwnerNode = node; //任务自值段

    /*用户段*/
    *func_ptr = pfunc; // 将函数指针存储到内存池分配的空间中
    node->TaskHandler.TaskCallBack.DynamicCallBack = func_ptr; // 指向内存池中的函数指针
    node->TaskHandler.Task_TrigTime = 0; // 初始化为0，需要通过rTaskDelay设置
    node->TaskHandler.Task_Priority = Priority;

    EK_Result_t result = r_task_insert_node(&WaitSchedule, node);
    if (result != EK_OK)
    {
        EK_FREE(func_ptr); // 释放函数指针内存
        EK_FREE(node); // 插入失败时释放内存
        return result;
    }

    if (task_handler != NULL)
    {
        *task_handler = &(node->TaskHandler); // 返回指向节点中TaskHandler的指针
    }

    return EK_OK;
}

/**
 * @brief 动态移除一个任务（释放内存池分配的节点）
 * @param task_handler 要移除的任务句柄，如果为NULL则移除当前任务
 * @return EK_Result_t 移除结果
 * @note 适用于动态分配的任务，会释放节点内存和函数指针内存
 */
EK_Result_t EK_rTaskDelete(EK_pTaskHandler_t task_handler)
{
    EK_pTaskHandler_t target_handler = task_handler;

    // 如果传入NULL，则操作当前任务
    if (target_handler == NULL)
    {
        if (CurTaskHandler == NULL) return EK_NULL_POINTER;
        target_handler = CurTaskHandler;
    }

    // 传入的是由静态创建的任务
    if (TASK_IS_STATIC(target_handler->Task_Info) == true)
    {
        // 静态任务只能挂起 不能删除
        TASK_SET_SUSPENDED(target_handler->Task_Info);
        return EK_INVALID_PARAM;
    }

    // 利用Task_OwnerNode直接获取节点，避免链表搜索 - O(1)复杂度
    if (target_handler->Task_OwnerNode == NULL) return EK_NULL_POINTER;

    EK_pTaskEK_Node_t node = (EK_pTaskEK_Node_t)target_handler->Task_OwnerNode;
    EK_pTaskSchedule_t owner_list = (EK_pTaskSchedule_t)node->Owner;

    if (owner_list == NULL) return EK_NULL_POINTER;

    // 直接从对应链表中移除，无需搜索
    EK_Result_t result = r_task_remove_node(owner_list, node);
    if (result == EK_OK)
    {
        // 只有动态分配的任务才释放函数指针占用的内存池空间
        if (!TASK_IS_STATIC(target_handler->Task_Info) && target_handler->TaskCallBack.DynamicCallBack != NULL)
        {
            EK_FREE(target_handler->TaskCallBack.DynamicCallBack);
        }

        EK_FREE(node);
        // 如果移除的是当前任务，清空当前任务句柄
        if (target_handler == CurTaskHandler)
        {
            CurTaskHandler = NULL;
        }
    }

    return result;
}

/**
 * @brief 挂起一个任务
 * @param task_handler 要挂起的任务句柄，如果为NULL则挂起当前任务
 * @return EK_Result_t 挂起结果
 * @note 只是将任务的激活标志位设置为false，任务不会被执行
 */
EK_Result_t EK_rTaskSuspend(EK_pTaskHandler_t task_handler)
{
    EK_pTaskHandler_t target_handler = task_handler;

    // 如果传入NULL，则操作当前任务
    if (target_handler == NULL)
    {
        if (CurTaskHandler == NULL) return EK_NULL_POINTER;
        target_handler = CurTaskHandler;
    }

    target_handler->Task_Info = TASK_SET_SUSPENDED(target_handler->Task_Info);
    return EK_OK;
}

/**
 * @brief 恢复一个任务
 * @param task_handler 要恢复的任务句柄，如果为NULL则恢复当前任务
 * @return EK_Result_t 恢复结果
 * @note 将任务的激活标志位设置为true，任务可以被执行
 */
EK_Result_t EK_rTaskResume(EK_pTaskHandler_t task_handler)
{
    EK_pTaskHandler_t target_handler = task_handler;

    // 如果传入NULL，则操作当前任务
    if (target_handler == NULL)
    {
        if (CurTaskHandler == NULL) return EK_NULL_POINTER;
        target_handler = CurTaskHandler;
    }

    target_handler->Task_Info = TASK_SET_ACTIVE(target_handler->Task_Info);
    return EK_OK;
}

/**
 * @brief 设置任务优先级
 * @param task_handler 要设置优先级的任务句柄，如果为NULL则设置当前任务
 * @param Priority 新的优先级值
 * @return EK_Result_t 设置结果
 * @note 修改任务的优先级，数值越小优先级越高，会自动重新排序链表
 */
EK_Result_t EK_rTaskSetPriority(EK_pTaskHandler_t task_handler, uint8_t Priority)
{
    EK_pTaskHandler_t target_handler = task_handler;

    // 如果传入NULL，则操作当前任务
    if (target_handler == NULL)
    {
        if (CurTaskHandler == NULL) return EK_NULL_POINTER;
        target_handler = CurTaskHandler;
    }

    // 如果优先级没有改变，直接返回
    if (target_handler->Task_Priority == Priority)
    {
        return EK_OK;
    }

    // 利用双向关联特性直接获取节点和所属链表
    if (target_handler->Task_OwnerNode == NULL) return EK_NULL_POINTER;

    EK_pTaskEK_Node_t node = (EK_pTaskEK_Node_t)target_handler->Task_OwnerNode;
    EK_pTaskSchedule_t owner_list = (EK_pTaskSchedule_t)node->Owner;

    if (owner_list == NULL) return EK_NULL_POINTER;

    // 更新优先级
    target_handler->Task_Priority = Priority;

    // 从当前链表移除并重新插入以维持优先级排序
    EK_Result_t result = r_task_remove_node(owner_list, node);
    if (result == EK_OK)
    {
        result = r_task_insert_node(owner_list, node);
    }

    return result;
}
/**
 * @brief 获取任务信息
 * @param task_handler 要获取信息的任务句柄，如果为NULL则获取当前任务信息
 * @param task_info 用于存储任务信息的结构体指针
 * @return EK_Result_t 获取结果
 * @note 获取任务的最大消耗时间、内存占用、状态和优先级等信息
 */
EK_Result_t EK_rTaskGetInfo(EK_pTaskHandler_t task_handler, EK_TaskInfo_t *task_info)
{
    EK_pTaskHandler_t target_handler = task_handler;

    // 参数检查
    if (task_info == NULL)
    {
        return EK_NULL_POINTER;
    }

    // 初始化信息结构体
    task_info->isValid = false;
    task_info->isActive = false;
    task_info->isStatic = false;
    task_info->Priority = 0;
    task_info->MaxUsedTime = 0;
    task_info->Memory = 0;
    task_info->state = TASK_STATE_UNKNOWN;

    // 如果传入NULL，则获取当前任务信息
    if (target_handler == NULL)
    {
        if (CurTaskHandler == NULL)
        {
            return EK_NULL_POINTER; // 当前没有运行的任务
        }
        target_handler = CurTaskHandler;
    }

    // 利用双向关联特性获取任务状态
    EK_TaskState_t task_state = TASK_STATE_UNKNOWN;
    if (target_handler->Task_OwnerNode != NULL)
    {
        EK_pTaskEK_Node_t node = (EK_pTaskEK_Node_t)target_handler->Task_OwnerNode;
        if (node->Owner != NULL)
        {
            EK_pTaskSchedule_t owner_list = (EK_pTaskSchedule_t)node->Owner;
            if (owner_list == &WaitSchedule)
            {
                task_state = TASK_STATE_WAITING;
            }
            else if (owner_list == &RunSchedule)
            {
                task_state = TASK_STATE_RUNNING;
            }
        }
    }

    // 填充任务信息
    task_info->isValid = true;
    task_info->isActive = TASK_IS_ACTIVE(target_handler->Task_Info);
    task_info->isStatic = TASK_IS_STATIC(target_handler->Task_Info);
    task_info->Priority = target_handler->Task_Priority;
    task_info->MaxUsedTime = target_handler->Task_MaxUsed;
    task_info->Memory = sizeof(EK_TaskEK_Node_t); // 每个任务节点占用的内存大小
    task_info->state = task_state;

    return EK_OK;
}

/**
 * @brief 设置任务延时
 * @param delay_ms 延时时间(毫秒)
 * @return EK_Result_t 设置结果
 * @note 设置任务的触发间隔时间，高16位存储设定值，低16位存储当前值
 */
EK_Result_t EK_rTaskDelay(uint16_t delay_ms)
{
    if (CurTaskHandler == NULL) return EK_NULL_POINTER;

    CurTaskHandler->Task_TrigTime = TASK_SET_TRIG_TIME(delay_ms, delay_ms);
    return EK_OK;
}

/**
 * @brief 获取内存池剩余字节数
 * 
 * @return size_t 剩余的可用字节数
 */
size_t EK_sTaskGetFreeMemory(void)
{
    return EK_sMemPool_GetFreeSize();
}

/**
 * @brief 调度器运行
 * 
 * @param tick_get 获得时钟心跳源
 */
void EK_vTaskStart(uint32_t (*tick_get)(void))
{
    if (tick_get == NULL)
    {
        Task_Error(TASK_ERR_TICK_NULL); // tick获取函数不能为空
        return;
    }

    uint32_t last_tick = tick_get(); // 记录上次的时间戳

    while (1)
    {
        uint32_t current_tick = tick_get();

        // 只有当时间间隔达到1ms以上时才处理倒计时
        if (current_tick - last_tick >= 1)
        {
            last_tick = current_tick;

            EK_TaskEK_Node_t *p = WaitSchedule.Head; // 遍历指针

            // 先处理所有等待的任务
            while (p != NULL)
            {
                EK_TaskEK_Node_t *p_next = p->Next; // 保存节点移动前的下一个节点位置

                // 非激活直接跳过当前的节点
                if (TASK_IS_ACTIVE(p->TaskHandler.Task_Info) == false)
                {
                    p = p_next;
                    continue;
                }

                // 获取当前倒计时值
                uint16_t cur_time = TASK_GET_CUR_TIME(p->TaskHandler.Task_TrigTime);

                if (cur_time > 0)
                {
                    cur_time--;
                    // 更新当前倒计时值
                    p->TaskHandler.Task_TrigTime = TASK_SET_CUR_TIME(p->TaskHandler.Task_TrigTime, cur_time);
                }

                // 检查是否需要移动到运行链表（包括初始值为0和倒计时到0的情况）
                if (cur_time == 0)
                {
                    if (r_task_move_node(&WaitSchedule, &RunSchedule, p) != EK_OK)
                    {
                        Task_Error(TASK_ERR_WAIT_TO_RUN); // 等待链表到运行链表移动失败
                    }
                }

                p = p_next; // 节点移动
            }
        }

        //当前没有就绪任务 执行空闲任务
        if (RunSchedule.Count == 0)
        {
            TaskIdle();
            continue;
        }

        // 处理就绪的任务 - 使用临时标记避免重复执行
        EK_TaskEK_Node_t *ptr = RunSchedule.Head;
        while (ptr != NULL)
        {
            EK_TaskEK_Node_t *p_next = ptr->Next; // 保存下一个节点

            uint32_t start_tick = 0; // 在这里定义避免警告
            uint32_t diff_tick = 0; // 在这里定义避免警告

            // 检查任务是否已被挂起
            if (TASK_IS_ACTIVE(ptr->TaskHandler.Task_Info) == false)
            {
                // 任务已被挂起，直接将其移回等待链表，不执行
                ptr->TaskHandler.Task_TrigTime = TASK_RESET_TIME(ptr->TaskHandler.Task_TrigTime);
                if (r_task_move_node(&RunSchedule, &WaitSchedule, ptr) != EK_OK)
                {
                    Task_Error(TASK_ERR_RUN_TO_WAIT);
                }
                ptr = p_next;
                continue;
            }

            CurTaskHandler = &ptr->TaskHandler; // 获得当前占用CPU的任务的句柄
            start_tick = tick_get(); // 任务回调执行开始的tick

            // 根据任务类型调用不同的函数指针
            if (TASK_IS_STATIC(ptr->TaskHandler.Task_Info))
            {
                // 静态分配的任务，直接调用函数指针
                if (ptr->TaskHandler.TaskCallBack.StaticCallBack != NULL)
                {
                    ptr->TaskHandler.TaskCallBack.StaticCallBack();
                }
            }
            else
            {
                // 动态分配的任务，解引用内存池中的函数指针
                if (ptr->TaskHandler.TaskCallBack.DynamicCallBack != NULL &&
                    *(ptr->TaskHandler.TaskCallBack.DynamicCallBack) != NULL)
                {
                    (*(ptr->TaskHandler.TaskCallBack.DynamicCallBack))();
                }
            }

            diff_tick = tick_get() - start_tick; // 任务回调消耗时间 ms

            // 检查任务是否在执行过程中删除了自己
            if (CurTaskHandler == NULL)
            {
                // 当前任务已被删除，直接跳转到下一个任务
                ptr = p_next;
                continue;
            }

            // 利用高效的搜索函数验证 ptr 是否仍然有效（可能在任务执行过程中被删除）
            EK_TaskEK_Node_t *found_node = NULL;
            EK_Result_t search_result = r_task_search_node(&RunSchedule, &ptr->TaskHandler, &found_node);

            if (search_result != EK_OK || found_node != ptr)
            {
                // 当前任务节点已被删除或不在运行链表中，跳过后续处理
                ptr = p_next;
                continue;
            }

            if (diff_tick > ptr->TaskHandler.Task_MaxUsed) // 记录最大用时
            {
                ptr->TaskHandler.Task_MaxUsed = diff_tick;
            }

            // 重置倒计时为设定值
            ptr->TaskHandler.Task_TrigTime = TASK_RESET_TIME(ptr->TaskHandler.Task_TrigTime);

            // 尝试移动节点，如果失败说明节点已经不在运行链表中
            if (r_task_move_node(&RunSchedule, &WaitSchedule, ptr) != EK_OK)
            {
                // 节点移动失败，可能是因为节点已经被删除或移动
                // 不进入错误处理，直接继续下一个任务
            }

            ptr = p_next; // 移动到下一个任务
        }
    }
}
