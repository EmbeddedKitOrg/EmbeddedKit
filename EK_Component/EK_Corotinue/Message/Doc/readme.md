# EK_Coroutine 消息队列设计详解

本文档详细阐述了 EK_Coroutine 协程系统中消息队列的设计理念与工作流程。

## 1. 核心设计思想

本消息队列是为非抢占式协程环境量身打造的，其核心在于**将任务的“状态管理”与“事件管理”分离**。这通过在任务控制块 (TCB) 中设置两个独立的链表节点来实现：

- **状态节点 (`TCB_StateNode`)**: 用于将任务链接到内核的全局状态链表，如就绪链表 (`EK_CoroKernelReadyList`) 或延时阻塞链表 (`EK_CoroKernelBlockList`)。它管理任务的生命周期状态（就绪、运行、阻塞）。
- **事件节点 (`TCB_EventNode`)**: 用于将任务链接到其正在等待的特定事件的链表，例如某个消息队列的“发送等待链表”或“接收等待链表”。

这种设计的最大优势在于：
- **职责分离**: 内核的 `SysTick` 时钟中断只需要关心 `EK_CoroKernelBlockList`，处理所有任务的超时逻辑，而无需了解任务阻塞的具体原因（是 `EK_vCoroDelay` 还是在等一个消息）。
- **高效的事件处理**: 当一个事件发生时（如消息入队或出队），可以直接通过事件源（消息队列本身）的等待链表快速找到并唤醒对应的任务，无需遍历一个庞大的、混合了各种阻塞原因的全局链表。

### 关键数据结构

- **`EK_CoroTCB_t` (任务控制块)**
  ```c
  struct EK_CoroTCB_t {
      // ... 其他TCB成员
      EK_CoroListNode_t TCB_StateNode; // 状态节点
      EK_CoroListNode_t TCB_EventNode; // 事件节点
      // ...
  };
  ```

- **`EK_CoroMsg_t` (消息队列控制块)**
  ```c
  typedef struct EK_CoroMsg_t {
      EK_Queue_t *Msg_Queue;        // 底层用于存储消息的环形缓冲区
      EK_Size_t Msg_ItemSize;       // 单个消息的大小
      EK_Size_t Msg_ItemCapacity;   // 队列容量（能存多少个消息）

      EK_CoroList_t Msg_SendWaitList; // 因队列满而阻塞的“发送”任务链表
      EK_CoroList_t Msg_RecvWaitList; // 因队列空而阻塞的“接收”任务链表
  } EK_CoroMsg_t;
  ```

## 2. 工作流程详解

本消息队列严格遵循“先进先出”（FIFO）原则，所有的数据交换**必须**通过队列缓冲区进行，任务之间不发生直接数据拷贝。

### 2.1 发送消息 (`EK_rMsgSend`)

当一个任务（Task A）调用 `EK_rMsgSend` 尝试向一个消息队列发送数据时，流程如下：

1.  **检查队列是否已满**:
    - 系统检查 `Msg_Queue` 是否还有空间。

2.  **情况一：队列未满**
    - **数据入队**: 将 Task A 发送的数据拷贝到 `Msg_Queue` 环形缓冲区中。
    - **唤醒接收者**: 检查 `Msg_RecvWaitList` 是否有任务正在等待接收。如果有，则从链表中取出一个任务（Task B），将其从 `EK_CoroKernelBlockList` 移动到 `EK_CoroKernelReadyList`，并将其 `TCB_EventNode` 从 `Msg_RecvWaitList` 移除。
    - Task A 发送成功，**不阻塞**，立即返回 `EK_OK`。

3.  **情况二：队列已满**
    - **判断是否需要等待**: 检查 `timeout` 参数。
    - 如果 `timeout == 0`，表示不等待。函数立即返回 `EK_INSUFFICIENT_SPACE` 错误。
    - 如果 `timeout > 0`，Task A 需要进入阻塞状态：
        1.  **状态变更**: 调用 `EK_vCoroDelay(timeout)`，此函数会将 Task A 的 `TCB_StateNode` 插入到 `EK_CoroKernelBlockList` 中，并设置好唤醒时间。
        2.  **事件注册**: 将 Task A 的 `TCB_EventNode` 插入到消息队列的 `Msg_SendWaitList` 中。
        3.  **任务切换**: `EK_vCoroDelay` 最终会调用 `EK_vKernelYield()` 触发调度，CPU 执行其他就绪任务。

### 2.2 接收消息 (`EK_rMsgReceive`)

当一个任务（Task C）调用 `EK_rMsgReceive` 尝试从一个消息队列获取数据时，流程如下：

1.  **检查队列是否为空**:
    - 系统检查 `Msg_Queue` 中是否有数据。

2.  **情况一：队列中有数据**
    - **数据出队**: 从 `Msg_Queue` 环形缓冲区中取出一个消息，拷贝到 Task C 的接收缓冲区。
    - **唤醒发送者**: 检查 `Msg_SendWaitList` 是否有任务正在等待发送。如果有，则从链表中取出一个任务（Task D），将其从 `EK_CoroKernelBlockList` 移动到 `EK_CoroKernelReadyList`，并将其 `TCB_EventNode` 从 `Msg_SendWaitList` 移除。被唤醒的 Task D 在轮到它执行时，会再次尝试发送数据（此时队列已有空间）。
    - Task C 接收成功，**不阻塞**，立即返回 `EK_OK`。

3.  **情况二：队列为空**
    - **判断是否需要等待**: 检查 `timeout` 参数。
    - 如果 `timeout == 0`，表示不等待。函数立即返回 `EK_TIMEOUT` 错误。
    - 如果 `timeout > 0`，Task C 需要进入阻塞状态：
        1.  **状态变更**: 调用 `EK_vCoroDelay(timeout)`，将 Task C 的 `TCB_StateNode` 插入 `EK_CoroKernelBlockList`。
        2.  **事件注册**: 将 Task C 的 `TCB_EventNode` 插入到消息队列的 `Msg_RecvWaitList` 中。
        3.  **任务切换**: 触发调度，CPU 执行其他就绪任务。

### 2.3 超时处理 (`EK_vTickHandler`)

`SysTick` 中断服务程序会定期调用 `EK_vTickHandler`，其工作流程非常纯粹：

1.  **遍历延时阻塞链表**: `EK_vTickHandler` 只会遍历全局的 `EK_CoroKernelBlockList`。
2.  **检查超时**: 对链表中的每一个任务，检查其唤醒时间是否已到。
3.  **处理超时任务**:
    - 如果一个任务（例如上面阻塞的 Task A 或 Task C）超时，`EK_vTickHandler` 会将其 `TCB_StateNode` 从 `EK_CoroKernelBlockList` 移到 `EK_CoroKernelReadyList`。
    - **关键一步**: 任务被唤醒后，必须将其 `TCB_EventNode` 从它之前注册的任何事件链表（如 `Msg_SendWaitList` 或 `Msg_RecvWaitList`）中移除。这是为了防止任务在超时后，仍然被后续的事件错误地“唤醒”一次。
    - 任务恢复执行后，其阻塞的API（如 `EK_rMsgSend` 或 `EK_rMsgReceive`）会返回一个超时错误码，应用程序可以据此进行后续处理。

## 3. 流程图示

#### 任务因“队列满”而阻塞发送

```
Task A: EK_rMsgSend()
   |
   +--> Msg Queue is Full? --(Yes)--> Block Task A
   |                                   |
   |                                   +--> TCB_StateNode -> EK_CoroKernelBlockList (for timeout)
   |                                   |
   |                                   +--> TCB_EventNode -> Msg_SendWaitList (for event)
   |
   +--> Msg Queue is NOT Full? --(Yes)--> Copy data to queue, Wakeup Receiver if any, return OK.
```

#### 任务因“队列空”而阻塞接收

```
Task C: EK_rMsgReceive()
   |
   +--> Msg Queue is Empty? --(Yes)--> Block Task C
   |                                   |
   |                                   +--> TCB_StateNode -> EK_CoroKernelBlockList (for timeout)
   |                                   |
   |                                   +--> TCB_EventNode -> Msg_RecvWaitList (for event)
   |
   +--> Msg Queue is NOT Empty? --(Yes)--> Get data from queue, Wakeup Sender if any, return OK.
```

## 4. 总结

该设计通过双节点机制，优雅地实现了通用超时管理和特定事件处理的解耦。通过强制所有数据交换经过统一的队列缓冲区，保证了数据流的严格FIFO顺序和可预测性。它使得内核调度器保持简洁，同时保证了事件响应的高效性，非常适合资源受限且对逻辑清晰度有较高要求的嵌入式协程系统。