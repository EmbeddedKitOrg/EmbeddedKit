/**
 * @file ek_evoke.h
 * @brief 协作式事件驱动任务调度器
 * @author N1netyNine99
 *
 * 提供轻量级的事件驱动机制，用于非 RTOS 环境下的任务调度和事件处理
 * 支持任务等待事件、延迟发布、ISR 请求队列等功能
 *
 * @note 仅在 EK_USE_RTOS == 0 时可用
 * @note 需要用户实现睡眠和定时器回调的弱函数
 */

#ifndef EK_EVOKE_H
#define EK_EVOKE_H

#include "ek_conf.h"

#if EK_USE_RTOS == 0

#    include "ek_def.h"
#    include "ek_list.h"

/**
 * @brief ISR 请求队列最大容量
 */
#    ifndef EK_EVOKE_MAX_ISR_REQ
#        define EK_EVOKE_MAX_ISR_REQ (10)
#    endif /* EK_EVOKE_MAX_ISR_REQ */

/**
 * @brief 延迟请求池最大容量
 */
#    ifndef EK_EVOKE_MAX_DEFER_REQ
#        define EK_EVOKE_MAX_DEFER_REQ (10)
#    endif /* EK_EVOKE_MAX_DEFER_REQ */

/**
 * @brief 任务结构体（前置声明）
 */
typedef struct ek_evoke_task_t ek_evoke_task_t;

/**
 * @brief 事件结构体（前置声明）
 */
typedef struct ek_evoke_event_t ek_evoke_event_t;

/**
 * @brief 任务句柄类型
 */
typedef ek_evoke_task_t *ek_evoke_task_handle_t;

/**
 * @brief 事件句柄类型
 */
typedef ek_evoke_event_t *ek_evoke_event_handle_t;

/**
 * @brief 任务回调函数类型
 * @param evt 触发的事件
 * @param arg 用户传入的参数
 */
typedef void (*ek_evoke_cb_t)(ek_evoke_event_t *, void *);

/**
 * @brief 任务状态枚举
 */
typedef enum
{
    EK_EVOKE_STATE_IDLE = 0, /**< 空闲状态 */
    EK_EVOKE_STATE_DELAY, /**< 延迟状态 */
    EK_EVOKE_STATE_WAITTING, /**< 等待状态 */
    EK_EVOKE_STATE_READY, /**< 就绪状态 */
    EK_EVOKE_STATE_RUNNING, /**< 运行状态 */

    EK_EVOKE_STATE_MAX, /**< 状态最大值 */
} ek_evoke_state_t;

/**
 * @brief 任务结构体
 */
struct ek_evoke_task_t
{
    ek_evoke_state_t state; /**< 任务状态 */
    ek_list_node_t node; /**< 链表节点 */
    const char *name; /**< 任务名称 */
    ek_evoke_event_t *wait_event; /**< 等待的事件 */
    void *arg; /**< 用户参数 */
    ek_evoke_cb_t cb; /**< 回调函数 */
};

/**
 * @brief 事件结构体
 */
struct ek_evoke_event_t
{
    ek_list_node_t wait_list; /**< 等待该事件的任务链表 */
    const char *name; /**< 事件名称 */
    uint32_t count; /**< 事件计数（用于信号量机制） */
    void *data; /**< 事件携带的数据 */
};

/* ========== 初始化 ========== */

/**
 * @brief 初始化事件调度器
 *
 * @note 使用前必须调用此函数进行初始化
 */
void ek_evoke_init(void);

/* ========== 任务/事件 创建/销毁 ========== */

/**
 * @brief 创建任务
 * @param name 任务名称
 * @param cb 任务回调函数
 * @param arg 用户参数
 * @return 任务句柄
 */
ek_evoke_task_handle_t ek_evoke_task_create(const char *name, ek_evoke_cb_t cb, void *arg);

/**
 * @brief 销毁任务
 * @param tsk 任务句柄
 */
void ek_evoke_task_destroy(ek_evoke_task_handle_t tsk);

/**
 * @brief 创建事件
 * @param name 事件名称
 * @param init 初始计数值
 * @return 事件句柄
 */
ek_evoke_event_handle_t ek_evoke_event_create(const char *name, uint32_t init);

/**
 * @brief 销毁事件
 * @param evt 事件句柄
 *
 * @warning 销毁事件时会将所有等待该事件的任务状态设为 IDLE
 */
void ek_evoke_event_destroy(ek_evoke_event_handle_t evt);

/* ========== 事件订阅/分发 ========== */

/**
 * @brief 任务订阅事件
 * @param tsk 任务句柄
 * @param evt 事件句柄
 * @return true 任务立即就绪（事件已有计数）
 * @return false 任务进入等待状态
 */
bool ek_evoke_event_subscribe(ek_evoke_task_handle_t tsk, ek_evoke_event_handle_t evt);

/**
 * @brief 广播事件（唤醒所有等待任务）
 * @param evt 事件句柄
 * @param payload 事件携带的数据
 *
 * @note 如果没有任务等待，则增加事件计数
 */
void ek_evoke_event_broadcast(ek_evoke_event_handle_t evt, void *payload);

/**
 * @brief 发布事件（唤醒单个等待任务）
 * @param evt 事件句柄
 * @param payload 事件携带的数据
 *
 * @note 如果没有任务等待，则增加事件计数
 */
void ek_evoke_event_publish(ek_evoke_event_handle_t evt, void *payload);

/**
 * @brief 延迟发布事件
 * @param evt 事件句柄
 * @param payload 事件携带的数据
 * @param delay 延迟时间（tick）
 * @param broadcast true 广播模式，false 发布模式
 */
void ek_evoke_event_defer(ek_evoke_event_handle_t evt, void *payload, uint32_t delay, bool broadcast);

/**
 * @brief ISR 中广播事件
 * @param evt 事件句柄
 * @param payload 事件携带的数据
 *
 * @note 请求会被放入 ISR 请求队列，在主循环中处理
 */
void ek_evoke_event_broadcast_from_isr(ek_evoke_event_handle_t evt, void *payload);

/**
 * @brief ISR 中发布事件
 * @param evt 事件句柄
 * @param payload 事件携带的数据
 *
 * @note 请求会被放入 ISR 请求队列，在主循环中处理
 */
void ek_evoke_event_publish_from_isr(ek_evoke_event_handle_t evt, void *payload);

/**
 * @brief ISR 中延迟发布事件
 * @param evt 事件句柄
 * @param payload 事件携带的数据
 * @param delay 延迟时间（tick）
 * @param broadcast true 广播模式，false 发布模式
 *
 * @note 请求会被放入 ISR 请求队列，在主循环中处理
 */
void ek_evoke_event_defer_from_isr(ek_evoke_event_handle_t evt, void *payload, uint32_t delay, bool broadcast);

/* ========== 睡眠锁 ========== */

/**
 * @brief 睡眠锁（禁止深度睡眠）
 *
 * @note 每次调用增加锁计数，需要与 unlock 配对使用
 */
void ek_evoke_sleep_lock(void);

/**
 * @brief 解锁睡眠
 *
 * @note 每次调用减少锁计数，锁计数为 0 时允许深度睡眠
 */
void ek_evoke_sleep_unlock(void);

/* ========== 事件主循环 ========== */

/**
 * @brief 事件主循环
 *
 * @note 此函数永远不会返回，应在 ek_main() 中调用
 */
void ek_evoke_event_loop(void);

/* ========== 回调函数 ========== */

/**
 * @brief 延迟定时器回调函数
 *
 * @note 在定时器中断中调用，用于唤醒延迟事件
 */
void ek_evoke_delay_timer_callback(void);

/* ========== 弱函数（用户实现） ========== */

/**
 * @brief 进入临界区（弱函数）
 *
 * @note 用户需要实现此函数，通常使用关中断或互斥锁
 */
void ek_evoke_enter_critical(void);

/**
 * @brief 退出临界区（弱函数）
 *
 * @note 用户需要实现此函数，通常使用开中断或释放互斥锁
 */
void ek_evoke_exit_critical(void);

/**
 * @brief 设置定时器（弱函数）
 * @param xtick 定时器触发时间（tick）
 *
 * @note 用户需要实现此函数，用于延迟事件的定时唤醒
 */
void ek_evoke_set_timer(uint32_t xtick);

/**
 * @brief 浅睡眠（弱函数）
 *
 * @note 用户需要实现此函数，通常使用 WFI 指令
 */
void ek_evoke_light_sleep(void);

/**
 * @brief 深度睡眠（弱函数）
 *
 * @note 用户需要实现此函数，通常进入低功耗模式
 */
void ek_evoke_deep_sleep(void);

#endif // EK_USE_RTOS

#endif // EK_EVOKE_H
