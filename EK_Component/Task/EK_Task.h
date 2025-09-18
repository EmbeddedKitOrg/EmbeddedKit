/**
 * @file EK_Task.h
 * @brief 任务调度器头文件
 * @details 基于双链表+内存池实现的任务调度器接口定义
 * @author N1ntyNine99
 * @date 2025-09-04
 * @version v2.0
 */

#ifndef __EK_TASK_H
#define __EK_TASK_H

#include "../EK_Config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================= 类型定义区 ========================= */

/**
 * @brief 任务状态枚举
 */
typedef enum
{
    TASK_STATE_WAITING = 0, /**< 任务在等待链表中 */
    TASK_STATE_RUNNING, /**< 任务在运行链表中 */
    TASK_STATE_UNKNOWN /**< 未知状态 */
} EK_TaskState_t;

typedef struct
{
    uint16_t Task_TrigTime; /**< 任务倒计时值(毫秒) */
    uint8_t Task_Info; /**< 任务状态(bit7:静态创建标志, bit6-1:保留, bit0:激活状态) */
    union
    {
        void (*StaticCallBack)(void); /**< 静态任务的函数指针 */
        void (**DynamicCallBack)(void); /**< 动态任务的函数指针（指向内存池中的函数指针） */
    } TaskCallBack;
    uint16_t Task_MaxUsed; /**< 任务最高用时 */
    uint8_t Task_Priority; /**< 任务优先级 */
    void *Task_OwnerNode; /**< 拥有任务的节点 */
} EK_TaskHandler_t;

typedef struct EK_TaskEK_Node_t
{
    struct EK_TaskEK_Node_t *Next; /**< 下一个节点 */
    void *Owner; /**< 拥有节点的链表 */
    EK_TaskHandler_t TaskHandler; /**< 任务句柄 */
} EK_TaskEK_Node_t;

/**
 * @brief 任务信息结构体
 * @note 用于获取当前任务的详细信息
 */
typedef struct
{
    bool isValid; /**< 当前是否有有效的任务信息 */
    bool isActive; /**< 任务是否激活 */
    bool isStatic; /**< 任务是否静态创建 */
    uint8_t Priority; /**< 任务优先级 */
    uint16_t MaxUsedTime; /**< 任务最大消耗时间(ms) */
    EK_Size_t Memory; /**< 任务占用的内存字节数 */
    EK_TaskState_t state; /**< 任务当前状态 */
} EK_TaskInfo_t;

typedef EK_TaskHandler_t *EK_pTaskHandler_t; /**< 任务句柄指针(aka EK_TaskHandler_t*) */
typedef EK_TaskEK_Node_t *EK_pTaskEK_Node_t; /**< 节点句柄指针(aka EK_TaskEK_Node_t*) */

/* ========================= 函数声明区 ========================= */
EK_Result_t EK_rTaskInit(void);
EK_pTaskHandler_t EK_pTaskCreate_Static(EK_TaskEK_Node_t *node, EK_TaskHandler_t *static_handler);
EK_Result_t EK_rTaskCreate_Dynamic(void (*pfunc)(void), uint8_t Priority, EK_pTaskHandler_t *task_handler);
EK_Result_t EK_rTaskDelete(EK_pTaskHandler_t task_handler);
EK_Result_t EK_rTaskSuspend(EK_pTaskHandler_t task_handler);
EK_Result_t EK_rTaskResume(EK_pTaskHandler_t task_handler);
EK_Result_t EK_rTaskDelay(uint16_t delay_ms);
EK_Result_t EK_rTaskSetPriority(EK_pTaskHandler_t task_handler, uint8_t Priority);
EK_Result_t EK_rTaskGetInfo(EK_pTaskHandler_t task_handler, EK_TaskInfo_t *task_info);
EK_Size_t EK_sTaskGetFreeMemory(void);
void EK_vTaskStart(uint32_t (*tick_get)(void));

#ifdef __cplusplus
}
#endif

#endif
