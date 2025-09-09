/**
 * @file TaskSchedule.h
 * @brief 任务调度器头文件
 * @details 基于双链表+内存池实现的任务调度器接口定义
 * @author N1ntyNine99
 * @date 2025-09-04
 * @version v2.0
 */

#ifndef __TASKSCHEDULE_h
#define __TASKSCHEDULE_h

#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*弱定义宏*/
#ifndef __weak
#if defined(__GNUC__) || defined(__clang__)
#define __weak __attribute__((weak))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __weak __weak
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __weak __weak
#else
#define __weak
#endif
#endif

/*未使用函数宏*/
#ifndef __unused
#if defined(__GNUC__) || defined(__clang__)
#define __unused __attribute__((unused))
#elif defined(__IAR_SYSTEMS_ICC__)
#define __unused __attribute__((unused))
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __unused __attribute__((unused))
#elif defined(_MSC_VER)
#define __unused __pragma(warning(suppress:4505))
#else
#define __unused
#endif
#endif

/*未使用变量宏*/
#if defined(__GNUC__) || defined(__clang__)
#define UNUSED_VAR(x) ((void)(x))
#elif defined(_MSC_VER)
#define UNUSED_VAR(x) ((void)(x))
#else
#define UNUSED_VAR(X) ((void)(x))
#endif

/* ========================= 类型定义区 ========================= */
/**
 * @brief 任务操作结果枚举
 */
typedef enum
{
    TASK_OK = 0, /**< 操作成功 */
    TASK_ERROR_NULL_POINTER, /**< 空指针错误 */
    TASK_ERROR_INVALID_PARAM, /**< 无效参数 */
    TASK_ERROR_MEMORY_ALLOC, /**< 内存分配相关错误 */
    TASK_ERROR_POOL_NOT_INIT, /**< 内存池未初始化 */
    TASK_ERROR_TASK_EXISTS, /**< 任务已存在 */
    TASK_ERROR_TASK_NOT_FOUND, /**< 任务未找到 */
    TASK_ERROR_LIST_FULL, /**< 链表已满 */
    TASK_ERROR_INSERT_FAILED, /**< 插入失败 */
    TASK_ERROR_INIT_FAILED, /**< 初始化失败 */
} TaskRes_t;

/**
 * @brief 任务状态枚举
 */
typedef enum
{
    TASK_STATE_WAITING = 0, /**< 任务在等待链表中 */
    TASK_STATE_RUNNING, /**< 任务在运行链表中 */
    TASK_STATE_UNKNOWN /**< 未知状态 */
} TaskState_t;

typedef struct
{
    uint32_t Task_TrigTime; // 任务倒计时(高16位:设定值, 低16位:当前值)
    uint8_t Task_Info; // 任务状态(bit7:静态创建标志, bit6-1:保留, bit0:激活状态)
    union
    {
        void (*StaticCallBack)(void); // 静态任务的函数指针
        void (**DynamicCallBack)(void); // 动态任务的函数指针（指向内存池中的函数指针）
    } TaskCallBack;
    uint16_t Task_MaxUsed; //任务最高用时
    uint8_t Task_Priority; // 任务优先级
    void *Task_OwnerNode; // 拥有任务的节点
} TaskHandler_t;

typedef struct TaskNode_t
{
    struct TaskNode_t *Next; // 下一个节点
    void *Owner; // 拥有节点的链表
    TaskHandler_t TaskHandler; // 任务句柄
} TaskNode_t;

typedef struct
{
    TaskNode_t *Head; // 链表头指针
    TaskNode_t *Tail; // 链表尾指针
    uint16_t Count; // 节点数量
} TaskSchedule_t;

/**
 * @brief 任务信息结构体
 * @note 用于获取当前任务的详细信息
 */
typedef struct
{
    bool isValid; // 当前是否有有效的任务信息
    bool isActive; // 任务是否激活
    bool isStatic; // 任务是否静态创建
    uint8_t Priority; // 任务优先级
    uint16_t MaxUsedTime; // 任务最大消耗时间(ms)
    size_t Memory; // 任务占用的内存字节数
    TaskState_t state; // 任务当前状态
} TaskInfo_t;

typedef TaskHandler_t *pTaskHandler_t; // 任务句柄指针(aka TaskHandler_t*)
typedef TaskNode_t *pTaskNode_t; // 节点句柄指针(aka TaskNode_t*)
typedef TaskSchedule_t *pTaskSchedule_t; // 调度链表指针(aka TaskSchedule_t*)

/* ========================= 函数声明区 ========================= */
TaskRes_t rTaskInit(void);
pTaskHandler_t pTaskCreate_Static(TaskNode_t *node, TaskHandler_t *static_handler);
TaskRes_t rTaskCreate_Dynamic(void (*pfunc)(void), uint8_t Priority, pTaskHandler_t *task_handler);
TaskRes_t rTaskDelete(pTaskHandler_t task_handler);
TaskRes_t rTaskSuspend(pTaskHandler_t task_handler);
TaskRes_t rTaskResume(pTaskHandler_t task_handler);
TaskRes_t rTaskDelay(uint16_t delay_ms);
TaskRes_t rTaskSetPriority(pTaskHandler_t task_handler, uint8_t Priority);
TaskRes_t rTaskGetInfo(pTaskHandler_t task_handler, TaskInfo_t *task_info);
size_t uTaskGetFreeMemory(void);
void vTaskStart(uint32_t (*tick_get)(void));

#ifdef __cplusplus
}
#endif

#endif
