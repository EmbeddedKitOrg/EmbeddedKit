# EmbeddedKit 协程系统 API 参考手册

## 📖 概述

本手册详细介绍了 EmbeddedKit 协程系统的所有 API 函数，包括任务管理、消息队列、内核控制等功能。每个 API 都包含详细的参数说明、返回值、使用示例和注意事项。

**版本**: 1.0
**作者**: N1ntyNine99
**更新日期**: 2025-10-07

---

## 📋 目录

1. [内核控制 API](#内核控制-api)
2. [任务管理 API](#任务管理-api)
3. [消息队列 API](#消息队列-api)
4. [内存管理 API](#内存管理-api)
5. [错误代码](#错误代码)
6. [数据类型](#数据类型)
7. [使用示例](#使用示例)

---

## 🏛️ 内核控制 API

### EK_vKernelInit

**函数原型**: `void EK_vKernelInit(void)`

**功能描述**: 初始化协程内核，准备调度环境。

**参数**: 无

**返回值**: 无

**使用示例**:
```c
#include "EK_Component/EK_Corotinue/Kernel/Kernel.h"

int main(void) {
    // 硬件初始化
    HAL_Init();
    SystemClock_Config();

    // 初始化内存池
    EK_bMemPool_Init();

    // 初始化协程内核
    EK_vKernelInit();

    // 创建任务...

    // 启动调度器
    EK_vKernelStart();
}
```

**注意事项**:
- 必须在创建任何任务之前调用
- 必须在内存池初始化之后调用
- 只能调用一次

---

### EK_vKernelStart

**函数原型**: `void EK_vKernelStart(void)`

**功能描述**: 启动协程调度器，开始任务调度。

**参数**: 无

**返回值**: 无

**使用示例**:
```c
// 创建任务
EK_CoroHandler_t task1 = EK_pCoroCreate(Task1, NULL, 1, 256);
EK_CoroHandler_t task2 = EK_pCoroCreate(Task2, NULL, 2, 256);

// 启动调度器
EK_vKernelStart();

// 此处代码不会执行
while (1) {}
```

**注意事项**:
- 必须在创建至少一个任务后调用
- 调用后不会返回
- 确保中断向量表已正确配置

---

### EK_vTickHandler

**函数原型**: `void EK_vTickHandler(void)`

**功能描述**: 系统时钟中断处理函数，用于更新系统时间和管理延时任务。

**参数**: 无

**返回值**: 无

**使用示例**:
```c
#include "EK_Component/EK_Corotinue/Kernel/Kernel.h"

void SysTick_Handler(void) {
    // 调用协程时钟处理
    EK_vTickHandler();

    // 如果使用HAL库，还需要调用
    HAL_IncTick();
}
```

**注意事项**:
- 必须在 SysTick 中断中调用
- 调用频率由 `EK_CORO_TICK_RATE_HZ` 决定
- 调用时间应尽可能短

---

### EK_vPendSVHandler

**函数原型**: `void EK_vKernelPendSV_Handler(void)`

**功能描述**: PendSV 中断处理函数，用于协程上下文切换。

**参数**: 无

**返回值**: 无

**使用示例**:
```c
#include "EK_Component/EK_Corotinue/Kernel/Kernel.h"

void PendSV_Handler(void) {
    // 调用协程PendSV处理
    EK_vPendSVHandler();
}

// 或者使用宏定义（推荐）
void PendSV_Handler(void) {
    EK_vPendSVHandler();  // 展开为 __ASM volatile("b EK_vKernelPendSV_Handler")
}
```

**注意事项**:
- 必须在 PendSV 中断中调用
- PendSV 应设置为最低优先级
- 用户不应直接调用此函数

---

## 📋 任务管理 API

### EK_pCoroCreate

**函数原型**: `EK_CoroHandler_t EK_pCoroCreate(EK_CoroFunction_t task_func, void *task_arg, uint16_t priority, EK_Size_t stack_size)`

**功能描述**: 动态创建一个协程任务。

**参数**:
- `task_func`: 任务函数指针
- `task_arg`: 任务参数（可为 NULL）
- `priority`: 任务优先级（数值越小优先级越高）
- `stack_size`: 任务栈大小（字节）

**返回值**:
- 成功: 任务句柄（非 NULL）
- 失败: NULL

**使用示例**:
```c
#include "EK_Component/EK_Corotinue/Task/EK_CoroTask.h"

// 任务函数
void LED_Task(void *arg) {
    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
        EK_vCoroDelay(500);
    }
}

// 创建任务
EK_CoroHandler_t led_task = EK_pCoroCreate(LED_Task, NULL, 1, 256);
if (led_task == NULL) {
    // 创建失败处理
    Error_Handler();
}
```

**注意事项**:
- 任务栈大小必须足够，建议至少 128 字节
- 优先级范围取决于 `EK_CORO_PRIORITY_GROUPS` 配置
- 使用协程专用内存池分配

---

### EK_pCoroCreateStatic

**函数原型**: `EK_CoroStaticHandler_t EK_pCoroCreateStatic(EK_CoroTCB_t *static_tcb, EK_CoroFunction_t task_func, void *task_arg, uint16_t priority, void *stack, EK_Size_t stack_size)`

**功能描述**: 静态创建一个协程任务，使用预分配的内存。

**参数**:
- `static_tcb`: 静态分配的任务控制块
- `task_func`: 任务函数指针
- `task_arg`: 任务参数（可为 NULL）
- `priority`: 任务优先级
- `stack`: 静态分配的栈空间
- `stack_size`: 栈大小（字节）

**返回值**:
- 成功: 任务句柄（非 NULL）
- 失败: NULL

**使用示例**:
```c
#include "EK_Component/EK_Corotinue/Task/EK_CoroTask.h"

// 静态分配的TCB和栈
static EK_CoroTCB_t critical_tcb;
static uint32_t critical_stack[512];

// 任务函数
void Critical_Task(void *arg) {
    while (1) {
        // 关键任务代码
        Process_Critical_Data();
        EK_vCoroDelay(10);
    }
}

// 静态创建任务
EK_CoroStaticHandler_t critical_task = EK_pCoroCreateStatic(
    &critical_tcb, Critical_Task, NULL, 0,
    critical_stack, sizeof(critical_stack)
);
```

**注意事项**:
- 推荐用于关键任务，避免动态内存分配
- 栈空间必须 8 字节对齐
- 任务函数不能返回

---

### EK_rCoroDelete

**函数原型**: `void EK_rCoroDelete(EK_CoroHandler_t task_handle, EK_Result_t *result)`

**功能描述**: 删除指定的协程任务。

**参数**:
- `task_handle`: 要删除的任务句柄
- `result`: 操作结果（可为 NULL）

**返回值**: 无

**使用示例**:
```c
// 创建任务
EK_CoroHandler_t temp_task = EK_pCoroCreate(TempTask, NULL, 5, 256);

// 删除任务
EK_Result_t result;
EK_rCoroDelete(temp_task, &result);

if (result == EK_OK) {
    // 删除成功
} else {
    // 删除失败
    Error_Handler();
}
```

**注意事项**:
- 不能删除当前运行的任务
- 不能删除空闲任务
- 任务删除后句柄无效

---

### EK_rCoroSuspend

**函数原型**: `void EK_rCoroSuspend(EK_CoroHandler_t task_handle, EK_Result_t *result)`

**功能描述**: 挂起指定的协程任务。

**参数**:
- `task_handle`: 要挂起的任务句柄
- `result`: 操作结果（可为 NULL）

**返回值**: 无

**使用示例**:
```c
// 挂起任务
EK_Result_t result;
EK_rCoroSuspend(task_handle, &result);

if (result == EK_OK) {
    // 任务已挂起
}
```

**注意事项**:
- 挂起的任务不会被调度器调度
- 可以挂起自己（自挂起）
- 需要调用 `EK_rCoroResume` 恢复

---

### EK_rCoroResume

**函数原型**: `void EK_rCoroResume(EK_CoroHandler_t task_handle, EK_Result_t *result)`

**功能描述**: 恢复被挂起的协程任务。

**参数**:
- `task_handle`: 要恢复的任务句柄
- `result`: 操作结果（可为 NULL）

**返回值**: 无

**使用示例**:
```c
// 恢复任务
EK_Result_t result;
EK_rCoroResume(task_handle, &result);

if (result == EK_OK) {
    // 任务已恢复
}
```

**注意事项**:
- 只能恢复被挂起的任务
- 恢复后任务变为就绪状态
- 可以恢复自己（在ISR中）

---

### EK_vCoroDelay

**函数原型**: `void EK_vCoroDelay(uint32_t xticks)`

**功能描述**: 使当前任务延时指定的时钟周期。

**参数**:
- `xticks`: 延时的时钟周期数

**返回值**: 无

**使用示例**:
```c
void LED_Task(void *arg) {
    while (1) {
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);

        // 延时500ms（假设时钟频率为1000Hz）
        EK_vCoroDelay(500);
    }
}
```

**注意事项**:
- 延时期间任务进入阻塞状态
- 延时精度由 `EK_CORO_TICK_RATE_HZ` 决定
- 使用 `EK_MAX_DELAY` 表示无限延时

---

### EK_vCoroDelayUntil

**函数原型**: `void EK_vCoroDelayUntil(uint32_t xticks)`

**功能描述**: 使当前任务延时到指定的时钟周期。

**参数**:
- `xticks`: 目标时钟周期数

**返回值**: 无

**使用示例**:
```c
void Periodic_Task(void *arg) {
    uint32_t last_wake_time = EK_CoroKernelTick;

    while (1) {
        // 执行周期性任务
        Process_Periodic_Data();

        // 精确延时到下一个周期
        EK_vCoroDelayUntil(last_wake_time + 100);

        last_wake_time += 100;
    }
}
```

**注意事项**:
- 适用于周期性任务
- 可以补偿执行时间误差
- 保证固定的执行间隔

---

### EK_vCoroYield

**函数原型**: `void EK_vCoroYield(void)`

**功能描述**: 当前任务主动让出 CPU 控制权。

**参数**: 无

**返回值**: 无

**使用示例**:
```c
void Long_Task(void *arg) {
    for (int i = 0; i < 1000; i++) {
        // 处理一部分工作
        Process_Part(i);

        // 定期让出CPU，避免长时间占用
        if (i % 100 == 0) {
            EK_vCoroYield();
        }
    }
}
```

**注意事项**:
- 任务变为就绪状态，可能立即重新调度
- 用于长时间任务的分片处理
- 提高系统响应性

---

### EK_rCoroWakeup

**函数原型**: `EK_Result_t EK_rCoroWakeup(EK_CoroHandler_t task_handle)`

**功能描述**: 唤醒被延时的任务。

**参数**:
- `task_handle`: 要唤醒的任务句柄

**返回值**:
- `EK_OK`: 唤醒成功
- `EK_INVALID_PARAM`: 参数错误
- `EK_ERROR`: 唤醒失败

**使用示例**:
```c
// 唤醒任务
EK_Result_t result = EK_rCoroWakeup(sleeping_task);

if (result == EK_OK) {
    // 任务已唤醒
}
```

**注意事项**:
- 只能唤醒因延时而阻塞的任务
- 不能唤醒因其他原因阻塞的任务
- 通常用于中断服务程序

---

### EK_vCoroSetPriority

**函数原型**: `void EK_vCoroSetPriority(EK_CoroHandler_t task_handle, uint16_t priority, EK_Result_t *result)`

**功能描述**: 设置任务的优先级。

**参数**:
- `task_handle`: 任务句柄
- `priority`: 新的优先级
- `result`: 操作结果（可为 NULL）

**返回值**: 无

**使用示例**:
```c
// 提高任务优先级
EK_Result_t result;
EK_vCoroSetPriority(task_handle, 0, &result);

if (result == EK_OK) {
    // 优先级设置成功
}
```

**注意事项**:
- 优先级范围取决于配置
- 设置后立即生效
- 可能影响任务调度顺序

---

### EK_uCoroGetStack

**函数原型**: `EK_Size_t EK_uCoroGetStack(EK_CoroHandler_t task_handle)`

**功能描述**: 获取任务的栈大小。

**参数**:
- `task_handle`: 任务句柄

**返回值**:
- 成功: 栈大小（字节）
- 失败: 0

**使用示例**:
```c
// 获取任务栈大小
EK_Size_t stack_size = EK_uCoroGetStack(task_handle);

if (stack_size > 0) {
    printf("Task stack size: %d bytes\n", stack_size);
}
```

**注意事项**:
- 返回的是栈的总大小
- 不包括已使用的栈空间

---

### EK_uCoroGetHighWaterMark

**函数原型**: `EK_Size_t EK_uCoroGetHighWaterMark(EK_CoroHandler_t task_handle)`

**功能描述**: 获取任务的栈高水位标记。

**参数**:
- `task_handle`: 任务句柄

**返回值**:
- 成功: 剩余栈空间（字节）
- 失败: 0

**使用示例**:
```c
// 监控栈使用情况
EK_Size_t water_mark = EK_uCoroGetHighWaterMark(task_handle);

if (water_mark < 32) {
    // 栈空间不足
    Warning_Handler("Task %p stack low: %d bytes", task_handle, water_mark);
}
```

**注意事项**:
- 高水位标记表示历史最大使用量
- 需要启用栈溢出检测功能
- 值越小表示栈使用越多

---

## 📨 消息队列 API

### EK_pMsgCreate

**函数原型**: `EK_CoroMsgHanler_t EK_pMsgCreate(EK_Size_t item_size, EK_Size_t item_amount)`

**功能描述**: 动态创建消息队列。

**参数**:
- `item_size`: 每个消息的大小（字节）
- `item_amount`: 消息数量

**返回值**:
- 成功: 消息队列句柄（非 NULL）
- 失败: NULL

**使用示例**:
```c
#include "EK_Component/EK_Corotinue/Message/EK_CoroMessage.h"

// 消息结构
typedef struct {
    uint32_t id;
    uint8_t data[16];
} Message;

// 创建消息队列
EK_CoroMsgHanler_t queue = EK_pMsgCreate(sizeof(Message), 10);
if (queue == NULL) {
    Error_Handler();
}
```

**注意事项**:
- 使用协程专用内存池分配
- 消息大小和数量根据实际需求设置
- 创建失败通常表示内存不足

---

### EK_pMsgCreateStatic

**函数原型**: `EK_CoroMsgStaticHanler_t EK_pMsgCreateStatic(EK_CoroMsg_t *msg, void *buffer, EK_Size_t item_size, EK_Size_t item_amount)`

**功能描述**: 静态创建消息队列，使用预分配的内存。

**参数**:
- `msg`: 静态分配的消息队列结构
- `buffer`: 消息缓冲区
- `item_size`: 每个消息的大小
- `item_amount`: 消息数量

**返回值**:
- 成功: 消息队列句柄（非 NULL）
- 失败: NULL

**使用示例**:
```c
#include "EK_Component/EK_Corotinue/Message/EK_CoroMessage.h"

// 消息结构
typedef struct {
    uint32_t id;
    uint8_t data[16];
} Message;

// 静态分配
static EK_CoroMsg_t static_queue;
static uint8_t msg_buffer[10 * sizeof(Message)];

// 静态创建消息队列
EK_CoroMsgStaticHanler_t queue = EK_pMsgCreateStatic(
    &static_queue, msg_buffer, sizeof(Message), 10
);
```

**注意事项**:
- 推荐用于关键任务的消息队列
- 缓冲区大小必须足够：`item_size * item_amount`
- 缓冲区必须 8 字节对齐

---

### EK_rMsgDelete

**函数原型**: `EK_Result_t EK_rMsgDelete(EK_CoroMsg_t *msg)`

**功能描述**: 删除消息队列。

**参数**:
- `msg`: 消息队列句柄

**返回值**:
- `EK_OK`: 删除成功
- `EK_INVALID_PARAM`: 参数错误
- `EK_ERROR`: 删除失败

**使用示例**:
```c
// 删除消息队列
EK_Result_t result = EK_rMsgDelete(queue);

if (result == EK_OK) {
    // 删除成功
}
```

**注意事项**:
- 删除后所有等待的任务被唤醒
- 静态创建的队列也可以删除
- 删除后句柄无效

---

### EK_rMsgSendToBack

**函数原型**: `EK_Result_t EK_rMsgSendToBack(EK_CoroMsgHanler_t msg, void *tx_buffer, uint32_t timeout)`

**功能描述**: 发送消息到队列。

**参数**:
- `msg`: 消息队列句柄
- `tx_buffer`: 要发送的消息缓冲区
- `timeout`: 超时时间（时钟周期）

**返回值**:
- `EK_OK`: 发送成功
- `EK_TIMEOUT`: 超时
- `EK_INVALID_PARAM`: 参数错误
- `EK_ERROR`: 发送失败

**使用示例**:
```c
// 消息结构
Message message = {1, {0xAA, 0xBB, 0xCC}};

// 发送消息（阻塞）
EK_Result_t result = EK_rMsgSendToBack(queue, &message, 100);

if (result == EK_OK) {
    // 发送成功
} else if (result == EK_TIMEOUT) {
    // 发送超时
}
```

**注意事项**:
- 如果队列满，任务会阻塞
- 使用 `EK_MAX_DELAY` 表示无限等待
- 超时时间为 0 表示非阻塞

---

### EK_rMsgReceive

**函数原型**: `EK_Result_t EK_rMsgReceive(EK_CoroMsgHanler_t msg, void *rx_buffer, uint32_t timeout)`

**功能描述**: 从队列接收消息。

**参数**:
- `msg`: 消息队列句柄
- `rx_buffer`: 接收消息的缓冲区
- `timeout`: 超时时间（时钟周期）

**返回值**:
- `EK_OK`: 接收成功
- `EK_TIMEOUT`: 超时
- `EK_INVALID_PARAM`: 参数错误
- `EK_ERROR`: 接收失败

**使用示例**:
```c
// 接收消息（阻塞）
Message received_msg;
EK_Result_t result = EK_rMsgReceive(queue, &received_msg, EK_MAX_DELAY);

if (result == EK_OK) {
    // 处理接收到的消息
    Process_Message(&received_msg);
}
```

**注意事项**:
- 如果队列空，任务会阻塞
- 接收缓冲区必须足够大
- 使用 `EK_MAX_DELAY` 表示无限等待

---

## 💾 内存管理 API

### EK_Coro_Malloc

**函数原型**: `void *EK_Coro_Malloc(EK_Size_t size)`

**功能描述**: 协程专用内存分配函数。

**参数**:
- `size`: 要分配的内存大小（字节）

**返回值**:
- 成功: 分配的内存指针
- 失败: NULL

**使用示例**:
```c
// 分配内存
void *buffer = EK_Coro_Malloc(1024);
if (buffer == NULL) {
    Error_Handler();
}

// 使用内存...

// 释放内存
EK_Coro_Free(buffer);
```

**注意事项**:
- 线程安全的内存分配
- 使用协程专用的内存池
- 必须使用 `EK_Coro_Free` 释放

---

### EK_Coro_Free

**函数原型**: `void EK_Coro_Free(void *ptr)`

**功能描述**: 协程专用内存释放函数。

**参数**:
- `ptr`: 要释放的内存指针

**返回值**: 无

**使用示例**:
```c
// 释放内存
EK_Coro_Free(ptr);
ptr = NULL;  // 避免悬空指针
```

**注意事项**:
- 只能释放由 `EK_Coro_Malloc` 分配的内存
- 可以安全地传入 NULL
- 释放后指针变为无效

---

## 🚨 错误代码

### EK_Result_t 枚举

```c
typedef enum {
    EK_OK = 0,                    // 操作成功
    EK_ERROR = -1,                // 通用错误
    EK_INVALID_PARAM = -2,        // 参数错误
    EK_TIMEOUT = -3,              // 超时错误
    EK_NO_MEMORY = -4,            // 内存不足
    EK_NOT_INITIALIZED = -5,      // 未初始化
    EK_NOT_FOUND = -6,            // 未找到
    EK_ALREADY_EXISTS = -7,       // 已存在
    EK_FULL = -8,                 // 已满
    EK_EMPTY = -9,                // 已空
    EK_INSUFFICIENT_SPACE = -10,  // 空间不足
    EK_UNKNOWN = -11,             // 未知错误
    EK_NULL_POINTER = -12         // 空指针错误
} EK_Result_t;
```

### 错误处理建议

```c
// 推荐的错误处理模式
EK_Result_t result = EK_rSomeFunction(params);

if (result != EK_OK) {
    switch (result) {
        case EK_INVALID_PARAM:
            Error_Handler("Invalid parameter");
            break;
        case EK_TIMEOUT:
            Error_Handler("Operation timeout");
            break;
        case EK_NO_MEMORY:
            Error_Handler("Out of memory");
            break;
        default:
            Error_Handler("Unknown error: %d", result);
            break;
    }
}
```

---

## 📊 数据类型

### 任务相关类型

```c
// 任务函数指针类型
typedef void (*EK_CoroFunction_t)(void *arg);

// 任务状态枚举
typedef enum {
    EK_CORO_READY = 0,      // 就绪状态
    EK_CORO_BLOCKED,        // 阻塞状态
    EK_CORO_RUNNING,        // 运行状态
    EK_CORO_SUSPENDED       // 挂起状态
} EK_CoroState_t;

// 任务句柄类型
typedef EK_CoroTCB_t *EK_CoroHandler_t;          // 动态任务句柄
typedef EK_CoroTCB_t *EK_CoroStaticHandler_t;     // 静态任务句柄
```

### 消息队列相关类型

```c
// 消息队列句柄类型
typedef EK_CoroMsg_t *EK_CoroMsgHanler_t;         // 动态消息队列句柄
typedef EK_CoroMsg_t *EK_CoroMsgStaticHanler_t;    // 静态消息队列句柄
```

### 事件相关类型

```c
// 事件结果枚举
typedef enum {
    EK_CORO_EVENT_NONE = 0,   // 无事件
    EK_CORO_EVENT_PENDING,     // 事件挂起
    EK_CORO_EVENT_OK,          // 事件成功
    EK_CORO_EVENT_TIMEOUT,     // 事件超时
    EK_CORO_EVENT_DELETED      // 事件已删除
} EK_CoroEventResult_t;
```

---

## 🎯 使用示例

### 完整的任务和消息队列示例

```c
#include "stm32f4xx_hal.h"
#include "EK_Component/EK_Corotinue/Kernel/Kernel.h"
#include "EK_Component/EK_Corotinue/Task/EK_CoroTask.h"
#include "EK_Component/EK_Corotinue/Message/EK_CoroMessage.h"
#include "EK_Component/MemPool/EK_MemPool.h"

// 消息结构
typedef struct {
    uint32_t msg_id;
    float temperature;
    uint32_t timestamp;
} SensorMessage;

// 全局变量
EK_CoroMsgHanler_t sensor_queue;

// 传感器读取任务
void Sensor_Task(void *arg) {
    SensorMessage msg;

    while (1) {
        // 读取传感器数据
        msg.temperature = Read_Temperature();
        msg.timestamp = HAL_GetTick();
        msg.msg_id = 1;

        // 发送消息
        EK_Result_t result = EK_rMsgSendToBack(sensor_queue, &msg, 100);

        if (result != EK_OK) {
            // 发送失败处理
            Error_Handler("Sensor message send failed");
        }

        // 每1秒读取一次
        EK_vCoroDelay(1000);
    }
}

// 数据处理任务
void Process_Task(void *arg) {
    SensorMessage msg;

    while (1) {
        // 接收传感器数据
        EK_Result_t result = EK_rMsgReceive(sensor_queue, &msg, EK_MAX_DELAY);

        if (result == EK_OK) {
            // 处理数据
            if (msg.temperature > 30.0) {
                // 温度过高，触发警报
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
            } else {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
            }

            // 显示数据
            printf("Temperature: %.1f°C, Time: %lu\n",
                   msg.temperature, msg.timestamp);
        }
    }
}

// 监控任务
void Monitor_Task(void *arg) {
    PoolStats_t stats;

    while (1) {
        // 获取内存池统计
        EK_vMemPool_GetStats(&stats);

        // 检查内存使用情况
        if (stats.Pool_FreeBytes < stats.Pool_TotalSize * 0.1) {
            printf("Warning: Low memory (%d bytes free)\n",
                   stats.Pool_FreeBytes);
        }

        // 每5秒监控一次
        EK_vCoroDelay(5000);
    }
}

int main(void) {
    // HAL初始化
    HAL_Init();
    SystemClock_Config();

    // GPIO初始化
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // 内存池初始化
    EK_bMemPool_Init();

    // 创建消息队列
    sensor_queue = EK_pMsgCreate(sizeof(SensorMessage), 10);
    if (sensor_queue == NULL) {
        Error_Handler("Failed to create message queue");
    }

    // 协程内核初始化
    EK_vKernelInit();

    // 创建任务
    EK_pCoroCreate(Sensor_Task, NULL, 1, 512);
    EK_pCoroCreate(Process_Task, NULL, 2, 512);
    EK_pCoroCreate(Monitor_Task, NULL, 3, 256);

    // 启动协程调度器
    EK_vKernelStart();

    while (1) {}
}

// SysTick中断处理
void SysTick_Handler(void) {
    EK_vTickHandler();
    HAL_IncTick();
}

// PendSV中断处理
void PendSV_Handler(void) {
    EK_vPendSVHandler();
}
```

---

## 📝 总结

本手册详细介绍了 EmbeddedKit 协程系统的所有 API 函数，包括：

1. **内核控制 API**: 用于初始化、启动和控制协程系统
2. **任务管理 API**: 用于创建、删除、挂起、恢复任务
3. **消息队列 API**: 用于任务间通信
4. **内存管理 API**: 用于协程专用内存分配
5. **错误代码**: 统一的错误处理机制
6. **数据类型**: 系统使用的各种数据结构
7. **使用示例**: 完整的示例代码

通过合理使用这些 API，您可以构建高效、可靠的嵌入式应用程序。如有任何问题，请参考移植指南或联系技术支持。

**版本**: v1.0
**更新日期**: 2025-10-07