/**
 * @file ek_evoke.c
 * @brief 协作式事件驱动任务调度器实现
 * @author N1netyNine99
 */

#include "ek_evoke.h"

#if EK_USE_RTOS == 0

#    include "ek_ringbuf.h"
#    include "ek_mem.h"
#    include "ek_export.h"
#    include "ek_log.h"
#    include "ek_assert.h"

EK_LOG_FILE_TAG("ek_evoke.c");

#    define ISR_REQ_PUBLISH       (0x01)
#    define ISR_REQ_PUBLISH_DEALY (0x02)
#    define ISR_REQ_BROADCAST     (0x04)

typedef uint8_t _isr_req_type_t;

typedef struct
{
    _isr_req_type_t type;
    ek_evoke_event_t *evt;
    void *payload;
    uint32_t delay;
} _isr_req_t;

typedef struct
{
    ek_list_node_t node;
    ek_evoke_event_t *evt;
    void *payload;
    uint32_t wakeup_tick;
    bool broadcast;
} _defer_req_t;

static volatile uint32_t _sleep_lock;
static volatile uint32_t _event_tick_base;
static volatile uint32_t _event_tick_diff;
static volatile uint32_t _defer_earilest_tick;
static volatile bool _defer_evt_wakeup;

static _defer_req_t _defer_req_pool[EK_EVOKE_MAX_DEFER_REQ];
static ek_list_node_t _defer_pool_free_list;
static ek_ringbuf_spsc_t *_isr_fifo;

static ek_list_node_t _ready_task_list;
static ek_list_node_t _defer_evt_list;

static _defer_req_t *_defer_req_malloc(void);
static void _defer_req_free(_defer_req_t *req);

__EK_STATIC_INLINE void _ek_evoke_set_timer(uint32_t xtick)
{
    ek_evoke_set_timer(xtick);
    _event_tick_diff = xtick;
}

void ek_evoke_sleep_lock(void)
{
    _sleep_lock++;
}

void ek_evoke_sleep_unlock(void)
{
    if (_sleep_lock) _sleep_lock--;
    else EK_LOG_WARN("the sleep lock is 0, fail to decrease the sleep lock");
}

void ek_evoke_delay_timer_callback(void)
{
    _defer_evt_wakeup = true;
    _event_tick_base += _event_tick_diff;
    _event_tick_diff = 0;
}

void ek_evoke_init(void)
{
    ek_list_init(&_ready_task_list);
    ek_list_init(&_defer_evt_list);
    ek_list_init(&_defer_pool_free_list);

    for (size_t i = 0; i < EK_EVOKE_MAX_DEFER_REQ; i++)
    {
        ek_list_insert_tail(&_defer_pool_free_list, &_defer_req_pool[i].node);
    }

    _isr_fifo = ek_ringbuf_create_spsc(sizeof(_isr_req_t), EK_EVOKE_MAX_ISR_REQ);
    ek_assert_param(_isr_fifo != NULL);

    _defer_earilest_tick = UINT32_MAX;
    _event_tick_base = 0;
    _sleep_lock = 0;
    _defer_evt_wakeup = false;
}

EK_EXPORT_COMPONENTS(ek_evoke_init);

ek_evoke_task_handle_t ek_evoke_task_create(const char *name, ek_evoke_cb_t cb, void *arg)
{
    ek_assert_param(cb != NULL);

    ek_evoke_task_t *tsk = ek_malloc(sizeof(*tsk));
    ek_assert_param(tsk != NULL);

    ek_list_init(&tsk->node);
    tsk->name = name;
    tsk->cb = cb;
    tsk->arg = arg;
    tsk->state = EK_EVOKE_STATE_IDLE;
    tsk->wait_event = NULL;

    return tsk;
}

void ek_evoke_task_destroy(ek_evoke_task_handle_t tsk)
{
    ek_assert_param(tsk != NULL);
    if (tsk->state != EK_EVOKE_STATE_IDLE)
    {
        ek_list_remove(&tsk->node);
    }
    ek_free(tsk);
}

ek_evoke_event_handle_t ek_evoke_event_create(const char *name, uint32_t init)
{
    ek_evoke_event_t *evt = ek_malloc(sizeof(*evt));
    ek_assert_param(evt != NULL);

    ek_list_init(&evt->wait_list);
    evt->name = name;
    evt->count = init;
    evt->data = NULL;

    return evt;
}

void ek_evoke_event_destroy(ek_evoke_event_handle_t evt)
{
    ek_assert_param(evt != NULL);
    while (!ek_list_is_empty(&evt->wait_list))
    {
        ek_list_node_t *node = ek_list_get_first(&evt->wait_list);
        ek_evoke_task_t *tsk = ek_list_container(node, ek_evoke_task_t, node);
        ek_list_remove(&tsk->node);
        tsk->wait_event = NULL;
        tsk->state = EK_EVOKE_STATE_IDLE;
    }
    ek_free(evt);
}

bool ek_evoke_event_subscribe(ek_evoke_task_handle_t tsk, ek_evoke_event_handle_t evt)
{
    ek_assert_param(tsk != NULL);
    ek_assert_param(evt != NULL);

    tsk->wait_event = evt;
    // 等待的事件中有计数量
    // 且没有其他任务已经在等待这个事件
    // 则可以直接减少计数量并且运行任务
    if (evt->count && ek_list_is_empty(&evt->wait_list))
    {
        evt->count--;
        ek_list_insert_tail(&_ready_task_list, &tsk->node);
        tsk->state = EK_EVOKE_STATE_READY;

        return true;
    }

    // 否则说明需要等待事件发布
    ek_list_insert_tail(&evt->wait_list, &tsk->node);
    tsk->state = EK_EVOKE_STATE_WAITTING;

    return false;
}

void ek_evoke_event_broadcast(ek_evoke_event_handle_t evt, void *payload)
{
    ek_assert_param(evt != NULL);

    evt->data = payload;
    // 没有任务等待这个事件
    // 增加计数值
    if (ek_list_is_empty(&evt->wait_list))
    {
        evt->count++;
        return;
    }

    // 唤醒所有等待该事件的任务
    while (!ek_list_is_empty(&evt->wait_list))
    {
        ek_list_node_t *node = ek_list_get_first(&evt->wait_list);
        ek_evoke_task_t *tsk = ek_list_container(node, ek_evoke_task_t, node);
        ek_list_remove(node);
        ek_list_insert_tail(&_ready_task_list, node);
        tsk->state = EK_EVOKE_STATE_READY;
    }
}

void ek_evoke_event_publish(ek_evoke_event_handle_t evt, void *payload)
{
    ek_assert_param(evt != NULL);

    evt->data = payload;
    // 只唤醒第一个等待该事件的任务
    if (!ek_list_is_empty(&evt->wait_list))
    {
        ek_list_node_t *node = ek_list_get_first(&evt->wait_list);
        ek_evoke_task_t *tsk = ek_list_container(node, ek_evoke_task_t, node);
        ek_list_remove(node);
        ek_list_insert_tail(&_ready_task_list, node);
        tsk->state = EK_EVOKE_STATE_READY;

        return;
    }

    // 如果没有任务等待，就增加计数值
    evt->count++;
}

void ek_evoke_event_defer(ek_evoke_event_handle_t evt, void *payload, uint32_t delay, bool broadcast)
{
    ek_assert_param(evt != NULL);

    if (!delay)
    {
        if (broadcast) return ek_evoke_event_broadcast(evt, payload);
        else return ek_evoke_event_publish(evt, payload);
    }

    _defer_req_t *req = _defer_req_malloc();
    if (req == NULL)
    {
        EK_LOG_WARN("the defer request pool is empty, fail to create a defer request");
        return;
    }
    uint32_t wakeup_tick = delay + _event_tick_base;

    req->evt = evt;
    req->broadcast = broadcast;
    req->payload = payload;
    req->wakeup_tick = wakeup_tick;

    ek_list_node_t *pos;
    ek_list_foreach(pos, &_defer_evt_list)
    {
        _defer_req_t *pos_req = ek_list_container(pos, _defer_req_t, node);
        if (pos_req->wakeup_tick >= wakeup_tick) break;
    }
    ek_list_insert_before(pos, &req->node);
}

void ek_evoke_event_broadcast_from_isr(ek_evoke_event_handle_t evt, void *payload)
{
    ek_assert_param(evt != NULL);

    ek_evoke_enter_critical();

    _isr_req_t req = {
        .type = ISR_REQ_BROADCAST,
        .evt = evt,
        .payload = payload,
    };
    ek_ringbuf_write_spsc(_isr_fifo, &req);

    ek_evoke_exit_critical();
}

void ek_evoke_event_publish_from_isr(ek_evoke_event_handle_t evt, void *payload)
{
    ek_assert_param(evt != NULL);

    ek_evoke_enter_critical();

    _isr_req_t req = {
        .type = ISR_REQ_PUBLISH,
        .evt = evt,
        .payload = payload,
    };
    ek_ringbuf_write_spsc(_isr_fifo, &req);

    ek_evoke_exit_critical();
}

void ek_evoke_event_defer_from_isr(ek_evoke_event_handle_t evt, void *payload, uint32_t delay, bool broadcast)
{
    ek_assert_param(evt != NULL);

    ek_evoke_enter_critical();

    _isr_req_t req = {
        .type = (broadcast == true) ? (ISR_REQ_PUBLISH_DEALY | ISR_REQ_BROADCAST)
                                    : (ISR_REQ_PUBLISH_DEALY | ISR_REQ_PUBLISH),
        .evt = evt,
        .payload = payload,
        .delay = delay,
    };
    ek_ringbuf_write_spsc(_isr_fifo, &req);

    ek_evoke_exit_critical();
}

void ek_evoke_event_loop(void)
{
    while (1)
    {
        // 先处理是否有来自中断的请求
        // 从中断请求fifo中读取
        while (!ek_ringbuf_empty_spsc(_isr_fifo))
        {
            _isr_req_t req = { 0 };
            if (ek_ringbuf_read_spsc(_isr_fifo, &req))
            {
                if (req.type & ISR_REQ_PUBLISH)
                {
                    if (req.type & ISR_REQ_PUBLISH_DEALY) ek_evoke_event_defer(req.evt, req.payload, req.delay, false);
                    else ek_evoke_event_publish(req.evt, req.payload);
                }
                else if (req.type & ISR_REQ_BROADCAST)
                {
                    if (req.type & ISR_REQ_PUBLISH_DEALY) ek_evoke_event_defer(req.evt, req.payload, req.delay, true);
                    else ek_evoke_event_broadcast(req.evt, req.payload);
                }
            }
        }

        // 检查延时链表
        // 如果有延时的事件，则取出最早的唤醒事件然后设置中断时间
        if (!ek_list_is_empty(&_defer_evt_list))
        {
            ek_list_node_t *node = ek_list_get_first(&_defer_evt_list);
            _defer_req_t *req = ek_list_container(node, _defer_req_t, node);
            if (req->wakeup_tick != _defer_earilest_tick)
            {
                _defer_earilest_tick = req->wakeup_tick;
                _ek_evoke_set_timer(_defer_earilest_tick - _event_tick_base);
            }
        }

        // 首先检查就绪链表是否为空
        // 如果不为空则执行所有到期任务
        // 如果为空就直接去检查延时链表
        while (!ek_list_is_empty(&_ready_task_list))
        {
            ek_list_node_t *node = ek_list_get_first(&_ready_task_list);
            ek_evoke_task_t *tsk = ek_list_container(node, ek_evoke_task_t, node);
            ek_list_remove(&tsk->node);
            tsk->state = EK_EVOKE_STATE_RUNNING;
            tsk->cb(tsk->wait_event, tsk->arg);
            ek_list_insert_tail(&tsk->wait_event->wait_list, &tsk->node);
            tsk->state = EK_EVOKE_STATE_WAITTING;
        }

        // 检查睡眠锁，根据锁的状态来执行不同的睡眠状态
        // 如果有锁没有释放，则去浅睡眠 WFI
        // 如果所有的锁都释放了，则进行深度睡眠
        if (_sleep_lock) ek_evoke_light_sleep();
        else ek_evoke_deep_sleep();

        // 中断唤醒，要去检查是否有延时事件被唤醒
        // 如果有，则依次唤醒
        // 并且要去检查是否有其他同样唤醒时间的事件
        // 如果有，一并发布
        if (_defer_evt_wakeup)
        {
            _defer_evt_wakeup = false;

            while (!ek_list_is_empty(&_defer_evt_list))
            {
                ek_list_node_t *node = ek_list_get_first(&_defer_evt_list);
                _defer_req_t *req = ek_list_container(node, _defer_req_t, node);

                if (req->wakeup_tick > _defer_earilest_tick) break;

                ek_list_remove(node);
                if (req->broadcast) ek_evoke_event_broadcast(req->evt, req->payload);
                else ek_evoke_event_publish(req->evt, req->payload);
                _defer_req_free(req);
            }
            _defer_earilest_tick = UINT32_MAX;
        }
    }
}

static _defer_req_t *_defer_req_malloc(void)
{
    if (!ek_list_is_empty(&_defer_pool_free_list))
    {
        ek_list_node_t *node = ek_list_get_first(&_defer_pool_free_list);
        ek_list_remove(node);
        _defer_req_t *req = ek_list_container(node, _defer_req_t, node);
        return req;
    }
    return NULL;
}

static void _defer_req_free(_defer_req_t *req)
{
    ek_list_insert_tail(&_defer_pool_free_list, &req->node);
}

__EK_WEAK void ek_evoke_enter_critical(void)
{
}

__EK_WEAK void ek_evoke_exit_critical(void)
{
}

__EK_WEAK void ek_evoke_light_sleep(void)
{
}

__EK_WEAK void ek_evoke_deep_sleep(void)
{
}

__EK_WEAK void ek_evoke_set_timer(uint32_t xtick)
{
    __EK_UNUSED(xtick);
}

#endif // EK_USE_RTOS
