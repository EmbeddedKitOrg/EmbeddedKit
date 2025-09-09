# TaskSchedule 任务调度器组件

## 概述

TaskSchedule组件实现了一个基于双链表和内存池的轻量级**定时任务调度器**，支持多优先级任务管理。该调度器采用**运行到完成（Run-to-Completion）**的调度模式，每个任务函数执行完毕后自动返回调度器，通过设置延时时间控制任务的下次执行时机。

## 特性

- **双链表管理**：分离等待任务和就绪任务，提高调度效率
- **定时调度**：基于时间触发的任务执行机制
- **多优先级支持**：支持256级优先级，数值越小优先级越高
- **内存池集成**：与MemPool组件深度集成，提供高效内存管理
- **动态/静态任务**：支持动态创建和静态创建两种任务模式
- **任务状态管理**：完整的任务生命周期管理（创建、挂起、恢复、删除）
- **性能监控**：内置任务执行时间统计和内存使用监控
- **运行到完成**：任务函数执行期间不会被中断，执行完毕后自动调度下一个任务

## 调度原理

### 核心调度机制
这是一个**基于时间触发的任务调度器**，不是传统意义上的实时操作系统：

1. **两个链表管理**：
   - **等待链表（WaitSchedule）**：存放所有等待执行的任务
   - **运行链表（RunSchedule）**：存放当前可以执行的就绪任务

2. **调度循环**：
   ```
   每1ms检查一次:
   等待链表 → 倒计时减1 → 倒计时为0 → 移至运行链表
                                    ↓
   运行链表 → 按优先级执行 → 执行完毕 → 重置倒计时 → 移回等待链表
   ```

3. **任务执行模型**：
   - 任务函数是**普通函数调用**，不是死循环
   - 任务函数执行完毕后自动返回调度器
   - `rTaskDelay()`只是设置下次执行的延时时间，不会中断当前任务
   - 任务在延时期间处于等待状态，不占用CPU

### 优先级调度
- 数值越小优先级越高（0为最高优先级）
- 同一调度周期内，高优先级任务先执行
- 相同优先级任务按照在链表中的顺序执行
- 支持运行时动态调整优先级

## 设计架构

### 任务调度模型
```
         调度器主循环
              ↓
    ┌─────────────────────┐
    │   检查时间递减        │
    └─────────┬───────────┘
              ↓
    ┌─────────────────────┐
    │ 等待链表(倒计时>0)    │   倒计时为0
    │ [Task1:1000ms]      │ ──────────→ ┌─────────────────────┐
    │ [Task2:500ms]       │             │   运行链表(就绪)      │
    │ [Task3:2000ms]      │             │   [Task4:优先级1]    │
    └─────────────────────┘             │   [Task5:优先级2]    │
              ↑                         └─────────┬───────────┘
              │                                   ↓
              │                         ┌─────────────────────┐
              │ 执行完毕,重置倒计时        │  按优先级顺序执行      │
              └─────────────────────────│   task4() → task5() │
                                        └─────────────────────┘
```

### 任务生命周期
```
创建 → 等待(倒计时) → 就绪 → 执行 → 完成 → 等待(倒计时) → ...
  ↓                    ↓      ↑              ↑
 删除                 挂起    恢复            性能统计
```

## 数据结构

### TaskHandler_t - 任务控制块
```c
typedef struct
{
    uint32_t Task_TrigTime;    /**< 任务倒计时(高16位:设定值, 低16位:当前值) */
    uint8_t Task_Info;         /**< 任务状态(bit7:静态标志, bit0:激活状态) */
    union
    {
        void (*StaticCallBack)(void);      /**< 静态任务函数指针 */
        void (**DynamicCallBack)(void);    /**< 动态任务函数指针 */
    } TaskCallBack;
    uint16_t Task_MaxUsed;     /**< 任务最高用时 */
    uint8_t Task_Priority;     /**< 任务优先级 */
    void *Task_OwnerNode;      /**< 拥有任务的节点 */
} TaskHandler_t;
```

### TaskNode_t - 任务节点
```c
typedef struct TaskNode_t
{
    struct TaskNode_t *Next;   /**< 下一个节点 */
    void *Owner;               /**< 拥有节点的链表 */
    TaskHandler_t TaskHandler; /**< 任务句柄 */
} TaskNode_t;
```

### TaskSchedule_t - 调度器链表
```c
typedef struct
{
    TaskNode_t *Head;          /**< 链表头指针 */
    TaskNode_t *Tail;          /**< 链表尾指针 */
    uint16_t Count;            /**< 节点数量 */
} TaskSchedule_t;
```

### TaskInfo_t - 任务信息
```c
typedef struct
{
    bool isValid;              /**< 当前是否有有效的任务信息 */
    bool isActive;             /**< 任务是否激活 */
    bool isStatic;             /**< 任务是否静态创建 */
    uint8_t Priority;          /**< 任务优先级 */
    uint16_t MaxUsedTime;      /**< 任务最大消耗时间(ms) */
    size_t Memory;             /**< 任务占用的内存字节数 */
    TaskState_t state;         /**< 任务当前状态 */
} TaskInfo_t;
```

## API接口

### 调度器管理
- `TaskRes_t rTaskInit(void)` - 初始化任务调度器
- `void vTaskStart(uint32_t (*tick_get)(void))` - 启动任务调度器

### 任务创建和删除
- `pTaskHandler_t pTaskCreate_Static(TaskNode_t *node, TaskHandler_t *static_handler)` - 创建静态任务
- `TaskRes_t rTaskCreate_Dynamic(void (*pfunc)(void), uint8_t Priority, pTaskHandler_t *task_handler)` - 创建动态任务
- `TaskRes_t rTaskDelete(pTaskHandler_t task_handler)` - 删除任务

### 任务控制
- `TaskRes_t rTaskSuspend(pTaskHandler_t task_handler)` - 挂起任务
- `TaskRes_t rTaskResume(pTaskHandler_t task_handler)` - 恢复任务
- `TaskRes_t rTaskDelay(uint16_t delay_ms)` - 任务延时

### 任务配置和查询
- `TaskRes_t rTaskSetPriority(pTaskHandler_t task_handler, uint8_t Priority)` - 设置任务优先级
- `TaskRes_t rTaskGetInfo(pTaskHandler_t task_handler, TaskInfo_t *task_info)` - 获取任务信息
- `size_t uTaskGetFreeMemory(void)` - 获取空闲内存大小

## 使用示例

### 初始化和启动调度器
```c
#include "TaskSchedule.h"

// 系统时钟获取函数
uint32_t get_system_tick(void)
{
    return HAL_GetTick(); // 或其他系统时钟函数
}

int main()
{
    // 初始化调度器
    if (rTaskInit() != TASK_OK) {
        printf("Task scheduler initialization failed\n");
        return -1;
    }
    
    // 启动调度器（此函数不会返回）
    vTaskStart(get_system_tick);
    
    return 0;
}
```

### 创建定时任务
```c
#include "TaskSchedule.h"

// LED闪烁任务 - 每500ms执行一次
void led_blink_task(void)
{
    static bool led_state = false;
    led_state = !led_state;
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, led_state);
    
    printf("LED state: %s\n", led_state ? "ON" : "OFF");
    
    // 设置500ms后再次执行
    rTaskDelay(500);
}

// 传感器读取任务 - 每1秒执行一次
void sensor_read_task(void)
{
    float temperature = read_temperature_sensor();
    float humidity = read_humidity_sensor();
    
    printf("Temperature: %.2f°C, Humidity: %.2f%%\n", temperature, humidity);
    
    // 设置1000ms后再次执行
    rTaskDelay(1000);
}

// 数据处理任务 - 每2秒执行一次
void data_process_task(void)
{
    process_sensor_data();
    printf("Data processing completed\n");
    
    // 设置2000ms后再次执行
    rTaskDelay(2000);
}

// 创建任务的函数（在TaskCreation中调用）
_bool TaskCreation(void)
{
    pTaskHandler_t led_task = NULL;
    pTaskHandler_t sensor_task = NULL;
    pTaskHandler_t data_task = NULL;
    
    // 创建LED闪烁任务 (最高优先级)
    if (rTaskCreate_Dynamic(led_blink_task, 0, &led_task) != TASK_OK) {
        return false;
    }
    
    // 创建传感器读取任务 (中等优先级)
    if (rTaskCreate_Dynamic(sensor_read_task, 1, &sensor_task) != TASK_OK) {
        return false;
    }
    
    // 创建数据处理任务 (低优先级)
    if (rTaskCreate_Dynamic(data_process_task, 2, &data_task) != TASK_OK) {
        return false;
    }
    
    return true;
}
```

### 静态任务创建示例
```c
#include "TaskSchedule.h"

// 静态任务节点和句柄
TaskNode_t heartbeat_task_node;
TaskHandler_t heartbeat_task_handler;

// 心跳任务 - 每秒输出一次
void heartbeat_task_function(void)
{
    static uint32_t counter = 0;
    printf("System heartbeat: %lu\n", ++counter);
    
    // 设置1000ms后再次执行
    rTaskDelay(1000);
}

void create_static_heartbeat_task(void)
{
    // 初始化静态任务句柄
    heartbeat_task_handler.Task_TrigTime = 0;  // 立即执行
    heartbeat_task_handler.Task_Info = 0x80;   // 静态标志位
    heartbeat_task_handler.TaskCallBack.StaticCallBack = heartbeat_task_function;
    heartbeat_task_handler.Task_Priority = 5;  // 低优先级
    
    // 创建静态任务
    pTaskHandler_t task_handler = pTaskCreate_Static(&heartbeat_task_node, &heartbeat_task_handler);
    if (task_handler == NULL) {
        printf("Failed to create heartbeat task\n");
    } else {
        printf("Heartbeat task created successfully\n");
    }
}
```

### 任务控制示例
```c
#include "TaskSchedule.h"

pTaskHandler_t global_sensor_task = NULL;

void task_control_example(void)
{
    TaskInfo_t task_info;
    
    // 获取任务信息
    if (rTaskGetInfo(global_sensor_task, &task_info) == TASK_OK) {
        printf("Task Info:\n");
        printf("  Active: %s\n", task_info.isActive ? "Yes" : "No");
        printf("  Static: %s\n", task_info.isStatic ? "Yes" : "No");
        printf("  Priority: %d\n", task_info.Priority);
        printf("  Max Used Time: %d ms\n", task_info.MaxUsedTime);
        printf("  Memory Usage: %zu bytes\n", task_info.Memory);
    }
    
    // 挂起任务
    rTaskSuspend(global_sensor_task);
    printf("Sensor task suspended\n");
    
    // 等待5秒后恢复任务
    HAL_Delay(5000);
    
    rTaskResume(global_sensor_task);
    printf("Sensor task resumed\n");
    
    // 修改任务优先级
    rTaskSetPriority(global_sensor_task, 0);
    printf("Sensor task priority changed to highest\n");
}
```

### 一次性任务示例
```c
// 初始化任务 - 只执行一次
void init_peripherals_task(void)
{
    printf("Initializing peripherals...\n");
    
    // 初始化各种外设
    init_uart();
    init_spi();
    init_i2c();
    
    printf("Peripheral initialization completed\n");
    
    // 删除自己，因为只需要执行一次
    rTaskDelete(NULL);  // NULL表示删除当前任务
}

// 创建一次性任务
void create_init_task(void)
{
    pTaskHandler_t init_task = NULL;
    rTaskCreate_Dynamic(init_peripherals_task, 0, &init_task);
}
```

## 调度算法详解

### 运行到完成模型
这不是传统的抢占式或协作式调度器，而是**运行到完成（Run-to-Completion）**模型：

1. **任务函数不是死循环**：每个任务函数执行完毕后自然返回
2. **自动重新调度**：任务执行完毕后，调度器自动将其移回等待链表
3. **延时机制**：`rTaskDelay(ms)`设置任务下次执行的延时时间
4. **时间驱动**：调度器每1ms检查一次，将延时到期的任务移至就绪队列

### 调度流程
```
┌─────────────────┐
│   调度器启动      │
└─────────┬───────┘
          ↓
┌─────────────────┐
│ 每1ms时间检查     │ ←─────────┐
└─────────┬───────┘           │
          ↓                   │
┌─────────────────┐           │
│ 遍历等待链表      │           │
│ 倒计时-1         │           │
└─────────┬───────┘           │
          ↓                   │
┌─────────────────┐           │
│ 倒计时为0？       │──No───────┤
└─────────┬───────┘           │
          │Yes                │
          ↓                   │
┌─────────────────┐           │
│ 移至运行链表      │           │
└─────────┬───────┘           │
          ↓                   │
┌─────────────────┐           │
│ 按优先级执行      │           │
│ task_function() │           │
└─────────┬───────┘           │
          ↓                   │
┌─────────────────┐           │
│ 重置倒计时        │           │
│ 移回等待链表      │───────────┘
└─────────────────┘
```

### 任务状态转换
```
创建 → 等待链表(倒计时>0) → 运行链表(倒计时=0) → 执行 → 等待链表(重置倒计时)
  ↓           ↑                    ↓                ↑              ↓
删除        挂起                  挂起              统计性能        循环执行
```

### 重要特性说明
1. **非阻塞执行**：任务函数执行期间，调度器等待其完成
2. **自动循环**：任务执行完毕后自动重新调度，实现周期性执行
3. **精确延时**：基于系统tick的精确时间控制
4. **优先级调度**：同一时刻多个任务就绪时，按优先级顺序执行

## 性能特性

## 性能特性

- **调度开销**：O(1)时间复杂度的任务切换
- **内存占用**：每个任务约32字节（根据平台而定）
- **最大任务数**：受内存池大小限制
- **优先级范围**：0-255（0为最高优先级）
- **时间精度**：1ms精度的延时控制
- **调度方式**：运行到完成，任务执行完毕后自动调度
- **并发模型**：单线程顺序执行，无需考虑线程安全问题

## 注意事项

## 注意事项

1. **初始化顺序**：必须先初始化内存池，再初始化任务调度器
2. **任务函数设计**：
   - 任务函数不应是死循环，应该执行完毕后自然返回
   - 任务函数末尾必须调用`rTaskDelay()`设置下次执行时间
   - 如果任务不调用`rTaskDelay()`，该任务将不会再次执行
3. **执行模型**：任务函数是普通的函数调用，不是并发执行
4. **避免阻塞操作**：任务函数中不应使用会阻塞很长时间的系统调用
5. **栈深度**：任务函数调用层次不应过深，避免栈溢出
6. **临界区保护**：在中断服务程序中操作任务时需要关闭中断
7. **内存管理**：动态创建的任务会自动管理内存，静态任务需要用户管理节点内存
8. **一次性任务**：如果任务只需要执行一次，可以在任务末尾调用`rTaskDelete(NULL)`删除自己

## 错误码说明

```c
typedef enum
{
    TASK_OK = 0,                  /**< 操作成功 */
    TASK_ERROR_NULL_POINTER,      /**< 空指针错误 */
    TASK_ERROR_INVALID_PARAM,     /**< 无效参数 */
    TASK_ERROR_MEMORY_ALLOC,      /**< 内存分配相关错误 */
    TASK_ERROR_POOL_NOT_INIT,     /**< 内存池未初始化 */
    TASK_ERROR_TASK_EXISTS,       /**< 任务已存在 */
    TASK_ERROR_TASK_NOT_FOUND,    /**< 任务未找到 */
    TASK_ERROR_LIST_FULL,         /**< 链表已满 */
    TASK_ERROR_INSERT_FAILED,     /**< 插入失败 */
    TASK_ERROR_INIT_FAILED,       /**< 初始化失败 */
} TaskRes_t;
```

## 适用场景

这个调度器特别适合以下场景：

1. **嵌入式系统**：资源受限环境下的周期性任务管理
2. **传感器数据采集**：定时读取各种传感器数据
3. **设备状态监控**：定期检查系统状态和健康度
4. **通信任务**：定时发送心跳包、状态报告等
5. **用户界面更新**：定期刷新显示内容
6. **数据处理管道**：按时间间隔处理数据批次
7. **系统维护任务**：定期清理缓存、日志轮转等

## 与传统RTOS的区别

| 特性 | TaskSchedule | 传统RTOS |
|------|-------------|----------|
| 调度方式 | 运行到完成 | 抢占式/协作式 |
| 并发模型 | 顺序执行 | 真正并发 |
| 任务函数 | 普通函数 | 无限循环 |
| 上下文切换 | 无 | 有 |
| 栈需求 | 共享栈 | 每任务独立栈 |
| 同步机制 | 不需要 | 信号量/互斥锁 |
| 复杂度 | 低 | 高 |
| 内存开销 | 小 | 大 |
| 实时性 | 软实时 | 硬实时 |

## 版本信息

- **版本**: 2.0
- **作者**: N1ntyNine99
- **日期**: 2025-09-04
- **依赖**: MemPool组件
