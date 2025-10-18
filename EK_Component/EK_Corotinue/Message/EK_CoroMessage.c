/**
 * @file EK_CoroMessage.c
 * @brief 协程消息队列模块实现文件
 * @details 实现协程任务间的消息传递机制，包含动态和静态消息队列的创建、删除、
 *          发送和接收功能。支持任务阻塞与唤醒机制，提供汇合操作和超时控制。
 *          使用双链表管理等待队列，实现高效的任务调度和资源管理。
 * @author N1ntyNine99
 * @date 2025-09-30
 * @version v1.0
 */

/* ========================= 头文件包含区 ========================= */
#include "EK_CoroMessage.h"

#if (EK_CORO_ENABLE == 1)
#if (EK_CORO_MESSAGE_QUEUE_ENABLE == 1)

#include "../Task/EK_CoroTask.h"

typedef EK_Result_t (*msg_send_t)(EK_CoroMsg_t *, void *, EK_Size_t); // 写消息队列的函数指针

/* ========================= 内部函数 ========================= */

/**
 * @brief 将当前任务阻塞，并根据是发送还是接收操作，将其放入相应的等待列表。
 * 
 * @param msg 指向消息队列的指针。
 * @param tcb 指向要阻塞的任务控制块（TCB）的指针。
 * @param timeout 阻塞的超时时间。如果为 EK_MAX_DELAY，则永久阻塞。
 * @param isRecv 一个布尔值，如果任务因接收而阻塞，则为 true；如果因发送而阻塞，则为 false。
 */
ALWAYS_STATIC_INLINE void v_msg_delay(EK_CoroMsg_t *msg, EK_CoroTCB_t *tcb, uint32_t timeout, bool isRecv)
{
    if (msg == NULL || tcb == NULL || timeout == 0) return;

    // 原子化操作：将任务状态设置和事件节点添加在同一临界区内完成
    // 将TCB的状态设置为阻塞态
    tcb->TCB_State = EK_CORO_BLOCKED;

    // 根据标志位插入事件链表
    if (isRecv == false) EK_rKernelMove_Prio(&msg->Msg_SendWaitList, &tcb->TCB_EventNode);
    else EK_rKernelMove_Prio(&msg->Msg_RecvWaitList, &tcb->TCB_EventNode);

    // 进入阻塞的延时
    EK_vCoroDelay(timeout);
}

/**
 * @brief 从等待列表中取出一个等待任务
 * @details
 *  用于统一处理发送/接收等待链表的节点获取与移除逻辑。
 *  返回的TCB已从等待链表中安全移除，可进一步拷贝数据或唤醒。
 * @param wait_list 目标等待链表指针。
 * @return EK_CoroTCB_t* 成功返回等待的任务指针，链表为空或参数无效返回NULL。
 */
ALWAYS_STATIC_INLINE EK_CoroTCB_t *p_msg_take_waiter(EK_CoroList_t *wait_list)
{
    if (wait_list == NULL || wait_list->List_Count == 0) return NULL;

    EK_CoroTCB_t *tcb = (EK_CoroTCB_t *)EK_pKernelListGetFirst(wait_list)->CoroNode_Owner;
    EK_rKernelRemove(wait_list, &tcb->TCB_EventNode);

    return tcb;
}

/**
 * @brief 内部唤醒等待消息队列的任务
 * @details
 *  设置事件结果并将任务重新放入就绪链表。
 *  需配合 p_msg_take_waiter 一起使用，确保任务已从等待链表移除。
 * @param tcb 要唤醒的协程TCB。
 * @param result 唤醒原因，对应 EK_CoroEventResult_t 枚举。
 */
ALWAYS_STATIC_INLINE void v_msg_wake(EK_CoroTCB_t *tcb, EK_CoroEventResult_t result)
{
    if (tcb == NULL) return;

    tcb->TCB_WakeUpTime = EK_uKernelGetTick();
    tcb->TCB_EventResult = result;
    tcb->TCB_State = EK_CORO_READY;
    EK_rKernelMove_Head(EK_pKernelGetReadyList(tcb->TCB_Priority), &tcb->TCB_StateNode);
}

/**
 * @brief 向消息队列尾部添加数据（入队操作）
 * @param msg 消息队列指针
 * @param data 要添加的数据指针
 * @param data_size 数据大小（字节数）
 * @return EK_Result_t 操作结果
 */
ALWAYS_STATIC_INLINE EK_Result_t v_msg_queue_send_to_back(EK_CoroMsg_t *msg, void *data, EK_Size_t data_size)
{
    // 参数有效性检查
    if (msg == NULL || data == NULL || data_size == 0) return EK_INVALID_PARAM;

    // 检查队列是否满了
    if (EK_bMsgIsFull(msg)) return EK_FULL;

    // 检测剩余空间是否足够
    if (EK_uMsgGetFree(msg) * msg->Msg_ItemSize < data_size) return EK_INSUFFICIENT_SPACE;

    // 获取缓冲区指针
    uint8_t *buffer = EK_MSG_GET_BUFFER(msg);

    // 计算写入位置
    uint8_t *write_addr = buffer + msg->Msg_Rear;

    // 检查是否需要分段复制（跨越缓冲区边界）
    if (msg->Msg_Rear + data_size <= msg->Msg_BufferSize)
    {
        // 数据不跨界，直接复制
        EK_vMemCpy(write_addr, data, data_size);
    }
    else
    {
        // 数据跨界，分两段复制
        EK_Size_t first_part = msg->Msg_BufferSize - msg->Msg_Rear;
        EK_Size_t second_part = data_size - first_part;

        EK_vMemCpy(write_addr, data, first_part);
        EK_vMemCpy(buffer, (uint8_t *)data + first_part, second_part);
    }

    // 更新写指针和大小
    msg->Msg_Rear = (msg->Msg_Rear + data_size) % msg->Msg_BufferSize;
    msg->Msg_Size += data_size;

    return EK_OK;
}

/**
 * @brief 向消息队列尾部覆写数据（强制入队操作）
 * @details 当队列已满时，会自动覆盖最旧的数据以腾出空间
 * @param msg 消息队列指针
 * @param data 要写入的数据指针
 * @param data_size 数据大小（字节数）
 * @return EK_Result_t 覆写结果
 */
ALWAYS_STATIC_INLINE EK_Result_t v_msg_queue_over_write(EK_CoroMsg_t *msg, void *data, EK_Size_t data_size)
{
    // 参数有效性检查
    if (msg == NULL || data == NULL || data_size == 0) return EK_INVALID_PARAM;

    // 检查数据大小是否超过队列容量
    if (data_size > msg->Msg_BufferSize) return EK_INSUFFICIENT_SPACE;

    // 检查剩余空间是否足够，不足时丢弃最旧的数据
    while (EK_uMsgGetFree(msg) * msg->Msg_ItemSize < data_size)
    {
        // 计算需要丢弃的数据大小
        EK_Size_t discard_size = EK_uMsgGetCount(msg) * msg->Msg_ItemSize;
        if (discard_size > data_size)
        {
            discard_size = data_size;
        }

        // 更新读指针，相当于丢弃最旧的数据
        msg->Msg_Front = (msg->Msg_Front + discard_size) % msg->Msg_BufferSize;
        msg->Msg_Size -= discard_size;
    }

    // 获取缓冲区指针
    uint8_t *buffer = EK_MSG_GET_BUFFER(msg);

    // 计算写入位置
    uint8_t *write_addr = buffer + msg->Msg_Rear;

    // 检查是否需要分段复制（跨越缓冲区边界）
    if (msg->Msg_Rear + data_size <= msg->Msg_BufferSize)
    {
        // 数据不跨界，直接复制
        EK_vMemCpy(write_addr, data, data_size);
    }
    else
    {
        // 数据跨界，分两段复制
        EK_Size_t first_part = msg->Msg_BufferSize - msg->Msg_Rear;
        EK_Size_t second_part = data_size - first_part;

        EK_vMemCpy(write_addr, data, first_part);
        EK_vMemCpy(buffer, (uint8_t *)data + first_part, second_part);
    }

    // 更新写指针和大小
    msg->Msg_Rear = (msg->Msg_Rear + data_size) % msg->Msg_BufferSize;
    msg->Msg_Size += data_size;

    return EK_OK;
}

/**
 * @brief 从消息队列头部取出数据（出队操作）
 * @param msg 消息队列指针
 * @param data_buffer 用于接收出队数据的缓冲区指针
 * @param data_size 要取出的数据大小（字节数）
 * @return EK_Result_t 操作结果
 */
ALWAYS_STATIC_INLINE EK_Result_t v_msg_queue_receive(EK_CoroMsg_t *msg, void *data_buffer, EK_Size_t data_size)
{
    // 参数有效性检查
    if (msg == NULL || data_buffer == NULL || data_size == 0) return EK_INVALID_PARAM;

    // 检查队列是否为空
    if (EK_bMsgIsEmpty(msg)) return EK_EMPTY;

    // 检查队列是否有所需的数据数目
    if (EK_uMsgGetCount(msg) * msg->Msg_ItemSize < data_size) return EK_INSUFFICIENT_SPACE;

    // 获取缓冲区指针
    uint8_t *buffer = EK_MSG_GET_BUFFER(msg);

    // 计算读取位置
    uint8_t *read_addr = buffer + msg->Msg_Front;

    // 检查是否需要分段读取（跨越缓冲区边界）
    if (msg->Msg_Front + data_size <= msg->Msg_BufferSize)
    {
        // 数据不跨界，直接复制
        EK_vMemCpy(data_buffer, read_addr, data_size);
    }
    else
    {
        // 数据跨界，分两段读取
        EK_Size_t first_part = msg->Msg_BufferSize - msg->Msg_Front;
        EK_Size_t second_part = data_size - first_part;

        EK_vMemCpy(data_buffer, read_addr, first_part);
        EK_vMemCpy((uint8_t *)data_buffer + first_part, buffer, second_part);
    }

    // 更新读指针和大小
    msg->Msg_Front = (msg->Msg_Front + data_size) % msg->Msg_BufferSize;
    msg->Msg_Size -= data_size;

    return EK_OK;
}

/**
 * @brief 查看消息队列头部数据但不移除（窥视操作）
 * @param msg 消息队列指针
 * @param data_buffer 用于接收数据的缓冲区指针
 * @param data_size 要查看的数据大小（字节数）
 * @return EK_Result_t 操作结果
 */
ALWAYS_STATIC_INLINE EK_Result_t v_msg_queue_peek(EK_CoroMsg_t *msg, void *data_buffer, EK_Size_t data_size)
{
    // 参数有效性检查
    if (msg == NULL || data_buffer == NULL || data_size == 0) return EK_INVALID_PARAM;

    // 检查队列是否为空
    if (EK_bMsgIsEmpty(msg)) return EK_EMPTY;

    // 检查队列是否有所需的数据数目
    if (EK_uMsgGetCount(msg) * msg->Msg_ItemSize < data_size) return EK_INSUFFICIENT_SPACE;

    // 获取缓冲区指针
    uint8_t *buffer = EK_MSG_GET_BUFFER(msg);

    // 计算读取位置
    uint8_t *read_addr = buffer + msg->Msg_Front;

    // 检查是否需要分段读取（跨越缓冲区边界）
    if (msg->Msg_Front + data_size <= msg->Msg_BufferSize)
    {
        // 数据不跨界，直接复制
        EK_vMemCpy(data_buffer, read_addr, data_size);
    }
    else
    {
        // 数据跨界，分两段读取
        EK_Size_t first_part = msg->Msg_BufferSize - msg->Msg_Front;
        EK_Size_t second_part = data_size - first_part;

        EK_vMemCpy(data_buffer, read_addr, first_part);
        EK_vMemCpy((uint8_t *)data_buffer + first_part, buffer, second_part);
    }

    return EK_OK;
}

/**
 * @brief 清空消息队列中的全部数据
 * @param msg 消息队列指针
 * @return EK_Result_t 操作结果
 */
ALWAYS_STATIC_INLINE EK_Result_t v_msg_queue_clean(EK_CoroMsg_t *msg)
{
    // 参数有效性检查
    if (msg == NULL) return EK_NULL_POINTER;

    // 重置队列状态为空
    msg->Msg_Front = 0;
    msg->Msg_Rear = 0;
    msg->Msg_Size = 0;

    return EK_OK;
}

/* ========================= 公开API ========================= */
/**
 * @brief 使用动态内存创建一个消息队列。
 * @details 此函数会动态分配消息队列控制块和用于存储消息的缓冲区。
 * 
 * @param item_size 队列中单个消息项的大小（以字节为单位）。
 * @param item_amount 队列中单个消息项的个数。
 * 
 * @return EK_CoroMsgHanler_t 成功时返回消息队列的句柄，如果内存分配失败则返回 NULL。
 */
EK_CoroMsgHanler_t EK_pMsgCreate(EK_Size_t item_size, EK_Size_t item_amount)
{
    if (item_size == 0 || item_amount == 0) return NULL;

    // 为结构体分配内存
    EK_CoroMsg_t *msg = (EK_CoroMsg_t *)EK_CORO_MALLOC(sizeof(EK_CoroMsg_t));
    if (msg == NULL) return NULL;

    // 为消息缓冲区分配内存
    EK_Size_t buffer_size = item_size * item_amount;
    uint8_t *dynamic_buffer = (uint8_t *)EK_CORO_MALLOC(buffer_size);
    if (dynamic_buffer == NULL)
    {
        EK_CORO_FREE(msg);
        return NULL;
    }
    msg->Msg_Buffer = dynamic_buffer; // 设置缓冲区指针

    // 初始化环形缓冲区控制字段
    msg->Msg_BufferSize = buffer_size; // 缓冲区总容量
    msg->Msg_Front = 0; // 读指针位置
    msg->Msg_Rear = 0; // 写指针位置
    msg->Msg_Size = 0; // 当前数据大小

    msg->Msg_ItemSize = item_size; // 每条消息的字节大小

    // 初始化等待发送链表
    EK_vKernelListInit(&msg->Msg_SendWaitList);

    // 初始化等待接收链表
    EK_vKernelListInit(&msg->Msg_RecvWaitList);

    msg->Msg_isDynamic = true; // 来源于动态创建

    return (EK_CoroMsgHanler_t)msg;
}

/**
 * @brief 使用静态分配的内存来初始化一个消息队列。
 * @details 此函数使用用户提供的缓冲区和消息队列控制块进行初始化，完全避免了动态内存分配。
 *          使用内嵌的队列结构体，确保真正的静态创建。
 *
 * @param msg 指向静态消息队列结构体 `EK_CoroMsg_t` 的指针。
 * @param buffer 指向用于消息存储的静态缓冲区的指针。
 * @param item_size 队列中单个消息项的大小（以字节为单位）。
 * @param item_amount 队列中单个消息项的个数。
 *
 * @return EK_CoroMsgStaticHanler_t 成功时返回消息队列的句柄，如果参数无效则返回 NULL。
 */
EK_CoroMsgStaticHanler_t
EK_pMsgCreateStatic(EK_CoroMsg_t *msg, void *buffer, EK_Size_t item_size, EK_Size_t item_amount)
{
    if (msg == NULL || buffer == NULL) return NULL;
    if (item_size == 0 || item_amount == 0) return NULL;

    // 直接使用用户提供的缓冲区，完全避免动态分配
    uint8_t *static_buffer = (uint8_t *)buffer;
    EK_Size_t buffer_size = item_size * item_amount;

    // 设置消息队列属性
    msg->Msg_BufferSize = buffer_size; // 缓冲区总容量
    msg->Msg_Front = 0; // 读指针位置
    msg->Msg_Rear = 0; // 写指针位置
    msg->Msg_Size = 0; // 当前数据大小
    msg->Msg_ItemSize = item_size; // 每条消息的字节大小

    // 注意：静态创建时不需要设置缓冲区指针，因为缓冲区在联合体的第二个成员中
    // 柔性数组会直接使用消息队列结构体后面的内存空间

    // 初始化等待发送链表
    EK_vKernelListInit(&msg->Msg_SendWaitList);

    // 初始化等待接收链表
    EK_vKernelListInit(&msg->Msg_RecvWaitList);

    msg->Msg_isDynamic = false; // 来源于静态创建

    return (EK_CoroMsgStaticHanler_t)msg;
}

/**
 * @brief 删除一个消息队列。
 * @details 在删除队列之前，此函数会唤醒所有等待该队列的任务（包括发送和接收），
 *          并将它们移回就绪列表。同时，它会将消息队列本身从内核的等待列表中移除。
 *          对于动态创建的队列，它会释放其所有分配的内存。
 *          静态创建的队列不会被释放内存，但等待其上的任务仍会被唤醒。
 * 
 * @param msg 指向要删除的消息队列 `EK_CoroMsg_t` 的指针。
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功删除。
 *         - EK_NULL_POINTER: 如果传入的 msg 指针为空。
 */
EK_Result_t EK_rMsgDelete(EK_CoroMsg_t *msg)
{
    if (msg == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    // 唤醒所有等待发送的任务
    EK_CoroTCB_t *tcb_waiter = NULL;
    while ((tcb_waiter = p_msg_take_waiter(&msg->Msg_SendWaitList)) != NULL)
    {
        v_msg_wake(tcb_waiter, EK_CORO_EVENT_DELETED);
    }

    // 唤醒所有等待接收的任务
    while ((tcb_waiter = p_msg_take_waiter(&msg->Msg_RecvWaitList)) != NULL)
    {
        v_msg_wake(tcb_waiter, EK_CORO_EVENT_DELETED);
    }

    // 释放缓冲区内存
    if (msg->Msg_isDynamic)
    {
        // 动态创建：释放底层缓冲区和消息队列控制块
        uint8_t *buffer = EK_MSG_GET_BUFFER(msg); // 安全获取缓冲区指针
        if (buffer != NULL)
        {
            EK_CORO_FREE(buffer); // 释放底层缓冲区
        }
        EK_CORO_FREE(msg); // 释放消息队列控制块
    }
    else
    {
        // 静态创建：不需要释放任何内存
        // 内嵌的缓冲区是消息队列结构体的一部分，由用户管理其生命周期
        // 用户提供的缓冲区也由用户自己管理
        // 所以这里什么都不用做
    }

    EK_EXIT_CRITICAL();

    return EK_OK;
}

/**
 * @brief 向消息队列发送消息
 * @details 此函数是消息发送的核心实现，支持FIFO和覆盖两种模式。
 *          发送流程：
 *          1. 检查参数有效性，防止空指针和非法调用
 *          2. 根据over_write参数选择发送策略：
 *             - FIFO模式：队列满时阻塞等待
 *             - 覆盖模式：直接覆盖最旧消息
 *          3. 如果有任务等待接收，直接唤醒任务并传递数据
 *          4. 否则将消息加入队列，根据队列状态决定是否阻塞
 *          5. 阻塞唤醒后检查唤醒原因，处理超时等情况
 *
 *          线程安全性：此函数使用临界区保护，确保多任务环境下的安全访问
 *          阻塞机制：当队列满且非覆盖模式时，当前任务会被加入发送等待列表
 *          唤醒策略：优先唤醒等待时间最长的接收任务
 *
 * @param msg 消息队列句柄，必须为有效的消息队列指针
 * @param tx_buffer 指向要发送数据的缓冲区，数据将被复制到队列中
 * @param timeout 队列满时的等待超时时间（毫秒）：
 *                - 0：不等待，队列满时立即返回EK_FULL
 *                - EK_MAX_DELAY：永久等待直到队列有空位
 *                - 其他值：等待指定时间，超时返回EK_TIMEOUT
 * @param over_write 发送模式选择：
 *                   - false：FIFO模式，队列满时阻塞
 *                   - true：覆盖模式，队列满时覆盖最旧消息
 *
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 消息发送成功
 *         - EK_NULL_POINTER: msg或tx_buffer为空
 *         - EK_ERROR: 在中断上下文中调用或当前任务为空闲任务
 *         - EK_FULL: 队列满且timeout为0（仅FIFO模式）
 *         - EK_TIMEOUT: 等待超时（仅FIFO模式）
 *
 * @note 此函数不能在中断服务程序中调用
 * @note 覆盖模式下不会阻塞，timeout参数被忽略
 * @note 发送操作会将当前任务从就绪列表移至阻塞列表（如果需要等待）
 * @see EK_rMsgSendToBack() FIFO模式发送宏
 * @see EK_rMsgOverWrite() 覆盖模式发送宏
 * @see EK_rMsgReceive() 消息接收函数
 */
EK_Result_t EK_rMsgSend(EK_CoroMsgHanler_t msg, void *tx_buffer, uint32_t timeout, bool over_write)
{
    if (msg == NULL || tx_buffer == NULL) return EK_NULL_POINTER;
    if (EK_pKernelGetCurrentTCB() == EK_pKernelGetIdleHandler()) return EK_ERROR;
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 获取当前任务TCB

    // 写消息队列模式
    msg_send_t r_msg_send_queue = (over_write == true) ? v_msg_queue_over_write : v_msg_queue_send_to_back;

    while (1)
    {
        EK_ENTER_CRITICAL();

        // 当前的队列未满
        if (EK_bMsgIsFull(msg) == false || over_write == true)
        {
            bool need_yield = false;

            // 当前的等待接收的列表中有等待任务
            if (msg->Msg_RecvWaitList.List_Count > 0)
            {
                // 从等待链表中找到等待的TCB
                EK_CoroTCB_t *waiter_tcb = p_msg_take_waiter(&msg->Msg_RecvWaitList);

                // 将其唤醒
                v_msg_wake(waiter_tcb, EK_CORO_EVENT_OK);

                // 如果唤醒的任务的优先级比当前任务的优先级高 则申请一次调度
                if (waiter_tcb->TCB_Priority < current_tcb->TCB_Priority) need_yield = true;
            }

            // 将消息直接入队
            EK_Result_t send_result = r_msg_send_queue(msg, tx_buffer, msg->Msg_ItemSize);

            EK_EXIT_CRITICAL();

            if (need_yield == true) EK_vCoroYield();

            return send_result;
        }

        // 剩余空间不够

        EK_EXIT_CRITICAL();

        // 不阻塞的话 直接退出
        if (timeout == 0)
        {
            return EK_FULL;
        }

        // 阻塞
        v_msg_delay(msg, current_tcb, timeout, false);

        // --从这里唤醒--
        // 超时了 如果现在还是没有空间 就直接退出 有空间就入队
        if (current_tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT && EK_bMsgIsFull(msg) == true)
        {
            return EK_TIMEOUT;
        }
        // 否则就重试
    }
}

/**
 * @brief 从消息队列接收一条消息。
 * @details 如果有任务因队列满而等待发送，则直接从等待时间最长的任务接收数据。
 *          如果没有任务等待发送，并且队列中有消息，则从队列中取出一个消息。
 *          如果队列为空，则根据指定的超时时间，当前任务将被阻塞。
 * 
 * @param msg 消息队列的句柄。
 * @param rx_buffer 指向用于存储接收数据的缓冲区的指针。
 * @param timeout 如果队列为空，等待接收的超时时间。设置为 0 表示不等待，设置为 EK_MAX_DELAY 表示永久等待。
 * 
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 消息成功接收或任务已进入等待状态。
 *         - EK_NULL_POINTER: msg 或 rx_buffer 为空。
 *         - EK_ERROR: 无法获取当前任务的 TCB。
 *         - EK_TIMEOUT: 当 timeout 为 0 且队列为空时返回。
 */
EK_Result_t EK_rMsgReceive(EK_CoroMsgHanler_t msg, void *rx_buffer, uint32_t timeout)
{
    if (msg == NULL || rx_buffer == NULL) return EK_NULL_POINTER;
    if (EK_pKernelGetCurrentTCB() == EK_pKernelGetIdleHandler()) return EK_ERROR;
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 获取当前任务TCB

    while (1)
    {
        EK_ENTER_CRITICAL();

        // 当前队列不为空 有消息可以读
        if (EK_bMsgIsEmpty(msg) != true)
        {
            bool need_yield = false;

            // 读取当前的消息队列
            EK_Result_t op_res = v_msg_queue_receive(msg, rx_buffer, msg->Msg_ItemSize);

            // 当前的等待发送的列表中有等待任务
            if (msg->Msg_SendWaitList.List_Count > 0)
            {
                // 从等待链表中找到等待的TCB
                EK_CoroTCB_t *waiter_tcb = p_msg_take_waiter(&msg->Msg_SendWaitList);

                // 将其唤醒
                v_msg_wake(waiter_tcb, EK_CORO_EVENT_OK);

                // 如果唤醒的任务的优先级比当前任务的优先级高 则申请一次调度
                if (waiter_tcb->TCB_Priority < current_tcb->TCB_Priority) need_yield = true;
            }
            EK_EXIT_CRITICAL();

            if (need_yield == true) EK_vCoroYield();

            return op_res;
        }

        EK_EXIT_CRITICAL();

        // 没有消息可读
        // 是否阻塞
        // 不阻塞 直接退出
        if (timeout == 0)
        {
            return EK_EMPTY;
        }

        // 准备阻塞
        v_msg_delay(msg, current_tcb, timeout, true);

        //--从这里唤醒

        // 超时了 如果现在还是为空 就直接退出 有空间就入队
        if (current_tcb->TCB_EventResult == EK_CORO_EVENT_TIMEOUT && EK_bMsgIsEmpty(msg) == true)
        {
            return EK_TIMEOUT;
        }
        // 否则就重试
    }
}

/**
 * @brief 查看消息队列头部的消息但不移除
 * @details
 *  该函数用于在不改变队列内容的情况下读取队列头部的消息。
 *  即使队列为空也不会阻塞，调用者需要检查返回值。
 * @param msg 消息队列的句柄。
 * @param rx_buffer 指向用于接收数据的缓冲区的指针。
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功将消息复制到缓冲区。
 *         - EK_NULL_POINTER: msg 或 rx_buffer 为空。
 *         - EK_EMPTY: 队列为空，无可读取的数据。
 */
EK_Result_t EK_rMsgPeek(EK_CoroMsgHanler_t msg, void *rx_buffer)
{
    if (msg == NULL || rx_buffer == NULL) return EK_NULL_POINTER;

    EK_ENTER_CRITICAL();

    EK_Result_t op_res;

    // 如果队列中没有完整的一条消息则返回空
    if (EK_bMsgIsEmpty(msg) == true)
    {
        op_res = EK_EMPTY;
    }
    else
    {
        // 调用内部Peek接口复制但不移除队列内容
        op_res = v_msg_queue_peek(msg, rx_buffer, msg->Msg_ItemSize);
    }

    EK_EXIT_CRITICAL();

    return op_res;
}

/**
 * @brief 清空消息队列中的全部数据
 * @details
 *  将队列重置为空状态，并唤醒所有因队列满而等待发送的任务。
 *  接收等待列表保持不变，调用者可根据需要继续发送新消息。
 * @param msg 消息队列的句柄。
 * @return EK_Result_t 操作结果：
 *         - EK_OK: 成功清空队列。
 *         - EK_NULL_POINTER: msg 为空。
 *         - EK_ERROR: 在中断上下文中调用。
 */
EK_Result_t EK_rMsgClean(EK_CoroMsgHanler_t msg)
{
    if (EK_IS_IN_INTERRUPT() == true) return EK_ERROR;
    if (msg == NULL) return EK_NULL_POINTER;

    bool need_yield = false;

    EK_ENTER_CRITICAL();

    // 重置消息队列读写指针
    EK_Result_t op_res = v_msg_queue_clean(msg);

    if (op_res != EK_OK)
    {
        EK_EXIT_CRITICAL();
        return op_res;
    }

    // 唤醒所有等待发送的任务，让它们重新尝试
    EK_CoroTCB_t *tcb_waiter = NULL;
    while ((tcb_waiter = p_msg_take_waiter(&msg->Msg_SendWaitList)) != NULL)
    {
        v_msg_wake(tcb_waiter, EK_CORO_EVENT_OK);
        need_yield = true;
    }

    EK_EXIT_CRITICAL();

    if (need_yield)
    {
        EK_vKernelYield();
    }

    return EK_OK;
}

/**
 * @brief 向消息队列发送消息（中断服务程序版本）
 * @details 此函数专用于中断服务程序中向消息队列发送消息，支持FIFO和覆盖两种模式。
 *          与普通版本的区别：
 *          1. 只能在中断上下文中调用，非中断环境会返回失败
 *          2. 不会阻塞调用者，队列满时根据over_write参数决定行为
 *          3. 通过higher_prio_wake参数通知是否需要进行任务切换
 *
 *          发送流程：
 *          1. 验证中断上下文和参数有效性
 *          2. 进入临界区保护共享数据
 *          3. 根据over_write参数选择发送策略：
 *             - FIFO模式：队列满时失败返回
 *             - 覆盖模式：直接覆盖最旧消息
 *          4. 如果有任务等待接收，直接唤醒最高优先级任务
 *          5. 否则将消息加入队列
 *          6. 更新higher_prio_wake标志指示是否需要调度
 *
 *          中断安全性：此函数使用临界区保护，确保中断环境下的安全访问
 *          非阻塞特性：中断中不能阻塞，队列满时直接返回失败或覆盖
 *          调度提示：通过higher_prio_wake提示内核是否需要立即进行任务切换
 *
 * @param msg 消息队列句柄，必须为有效的消息队列指针
 * @param tx_buffer 指向要发送数据的缓冲区，数据将被复制到队列中
 * @param higher_prio_wake 输出参数，用于指示是否唤醒了更高优先级的任务：
 *                        - true: 唤醒了更高优先级任务，建议在中断退出后进行任务切换
 *                        - false: 未唤醒更高优先级任务，无需特殊处理
 * @param over_write 发送模式选择：
 *                   - false：FIFO模式，队列满时返回false
 *                   - true：覆盖模式，队列满时覆盖最旧消息
 *
 * @return bool 操作结果：
 *         - true: 消息发送成功
 *         - false: 发送失败（非中断上下文、msg为空、队列满且非覆盖模式）
 *
 * @note 此函数只能在中断服务程序中调用
 * @note 调用者需要检查higher_prio_wake参数，如果为true则应该触发任务切换
 * @note 覆盖模式下不会失败（除非参数无效），timeout概念在中断中不适用
 * @note 此函数不会阻塞调用者，适合在中断上下文中使用
 *
 * @warning 不要在非中断上下文中调用此函数
 * @warning 确保tx_buffer指向的数据在中断处理期间保持有效
 * @warning higher_prio_wake参数必须指向有效的bool变量
 *
 * @see EK_rMsgSend() 普通版本的消息发送函数
 * @see EK_rMsgSendToBack() FIFO模式发送宏
 * @see EK_rMsgOverWrite() 覆盖模式发送宏
 * @see EK_rMsgReceive() 消息接收函数
 */
bool EK_bMsgSend_FromISR(EK_CoroMsgHanler_t msg, void *tx_buffer, bool *higher_prio_wake, bool over_write)
{
    if (EK_IS_IN_INTERRUPT() == false) return false;
    if (msg == NULL) return false;

    EK_ENTER_CRITICAL();

    EK_CoroTCB_t *current_tcb = EK_pKernelGetCurrentTCB(); // 获取当前任务TCB

    bool need_yield = false;

    // 写消息队列模式
    msg_send_t r_msg_send_queue = (over_write == true) ? v_msg_queue_over_write : v_msg_queue_send_to_back;

    // 如果消息队列未满 或者是覆写模式  正常将数据写进去 并且唤醒一个任务
    if (EK_bMsgIsFull(msg) != true || over_write == true)
    {
        // 当前的等待接收的列表中有等待任务
        if (msg->Msg_RecvWaitList.List_Count > 0)
        {
            // 从等待链表中找到等待的TCB
            EK_CoroTCB_t *waiter_tcb = p_msg_take_waiter(&msg->Msg_RecvWaitList);

            // 将其唤醒
            v_msg_wake(waiter_tcb, EK_CORO_EVENT_OK);

            // 如果唤醒的任务的优先级比当前任务的优先级高 则申请一次调度
            if (waiter_tcb->TCB_Priority < current_tcb->TCB_Priority) need_yield = true;
        }

        // 将消息直接入队
        r_msg_send_queue(msg, tx_buffer, msg->Msg_ItemSize);
    }

    *higher_prio_wake |= need_yield;

    EK_EXIT_CRITICAL();

    return need_yield;
}

#endif /* EK_CORO_MESSAGE_QUEUE_ENABLE == 1 */
#endif /* EK_CORO_ENABLE == 1 */
