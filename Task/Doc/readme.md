# EK_Task - 轻量级任务调度器

## 概述

EK_Task 是一个基于双链表和内存池实现的非抢占式任务调度器，专为嵌入式系统设计。采用基于优先级的调度策略，支持任务的创建、删除、挂起、恢复等完整生命周期管理，并提供任务延时、优先级调整等高级功能。

## 核心特性

- **非抢占式调度**：任务主动让出CPU，避免竞争条件
- **优先级调度**：数值越小优先级越高，支持动态优先级调整
- **双队列管理**：运行队列和等待队列分离，高效任务状态转换
- **双内存模式**：支持静态和动态任务创建
- **任务延时**：支持毫秒级任务延时
- **状态监控**：提供完整的任务状态查询和统计信息
- **内存集成**：与EK_MemPool深度集成，统一内存管理

## 系统架构

### 调度队列
```c
typedef struct {
    EK_TaskNode_t *Head;    // 队列头指针
    EK_TaskNode_t *Tail;    // 队列尾指针
    uint16_t Count;         // 节点数量
} EK_TaskSchedule_t;
```

- **RunSchedule**：运行队列，存放就绪待执行的任务
- **WaitSchedule**：等待队列，存放延时或挂起的任务

### 任务状态
```c
typedef enum {
    TASK_STATE_WAITING = 0, // 任务在等待队列中
    TASK_STATE_RUNNING,     // 任务在运行队列中  
    TASK_STATE_UNKNOWN      // 未知状态
} EK_TaskState_t;
```

## 数据结构

### EK_TaskHandler_t - 任务控制块
```c
typedef struct {
    uint32_t Task_TrigTime;  // 任务倒计时(高16位:设定值, 低16位:当前值)
    uint8_t Task_Info;       // 任务状态位(bit7:静态标志, bit0:激活状态)
    union {
        void (*StaticCallBack)(void);      // 静态任务函数指针
        void (**DynamicCallBack)(void);    // 动态任务函数指针
    } TaskCallBack;
    uint16_t Task_MaxUsed;   // 任务最大执行时间(ms)
    uint8_t Task_Priority;   // 任务优先级(0最高)
    void *Task_OwnerNode;    // 所属节点指针
} EK_TaskHandler_t;
```

### EK_TaskNode_t - 任务节点
```c
typedef struct EK_TaskNode_t {
    struct EK_TaskNode_t *Next; // 下一个节点
    void *Owner;                // 所属队列指针
    EK_TaskHandler_t TaskHandler; // 任务控制块
} EK_TaskNode_t;
```

### EK_TaskInfo_t - 任务信息
```c
typedef struct {
    bool isValid;           // 信息是否有效
    bool isActive;          // 任务是否激活
    bool isStatic;          // 是否静态创建
    uint8_t Priority;       // 任务优先级
    uint16_t MaxUsedTime;   // 最大执行时间
    size_t Memory;          // 占用内存大小
    EK_TaskState_t state;   // 当前状态
} EK_TaskInfo_t;
```

## API 接口

### 系统初始化

#### 任务系统初始化
```c
EK_Result_t EK_rTaskInit(void);
```
- **功能**：初始化任务系统和内存池
- **返回值**：初始化结果状态码
- **说明**：必须在创建任务前调用

#### 任务系统启动
```c
void EK_vTaskStart(uint32_t (*tick_get)(void));
```
- **功能**：启动任务调度器主循环
- **参数**：`tick_get` - 系统时钟获取函数指针
- **特性**：
  - 无限循环调度
  - 1ms时基任务延时处理
  - 自动任务状态转换
  - 空闲任务执行

### 任务创建

#### 静态任务创建
```c
EK_pTaskHandler_t EK_pTaskCreate_Static(EK_TaskNode_t *node, EK_TaskHandler_t *static_handler);
```
- **功能**：使用用户提供的内存创建任务
- **参数**：
  - `node`：用户分配的任务节点
  - `static_handler`：用户配置的任务控制块
- **返回值**：任务句柄指针，失败返回NULL
- **优势**：编译时确定内存，无运行时分配开销

#### 动态任务创建
```c
EK_Result_t EK_rTaskCreate_Dynamic(void (*pfunc)(void), uint8_t Priority, EK_pTaskHandler_t *task_handler);
```
- **功能**：使用内存池动态创建任务
- **参数**：
  - `pfunc`：任务函数指针
  - `Priority`：任务优先级(0最高)
  - `task_handler`：返回的任务句柄指针
- **返回值**：创建结果状态码
- **特性**：运行时动态创建，灵活性高

### 任务管理

#### 任务删除
```c
EK_Result_t EK_rTaskDelete(EK_pTaskHandler_t task_handler);
```
- **功能**：删除任务并释放资源
- **参数**：`task_handler` - 任务句柄，NULL表示删除当前任务
- **特性**：
  - 仅支持动态任务删除
  - 自动释放内存池资源
  - 安全删除当前运行任务

#### 任务挂起
```c
EK_Result_t EK_rTaskSuspend(EK_pTaskHandler_t task_handler);
```
- **功能**：挂起任务执行
- **参数**：`task_handler` - 任务句柄，NULL表示挂起当前任务
- **效果**：任务保留在队列中但不被调度

#### 任务恢复
```c
EK_Result_t EK_rTaskResume(EK_pTaskHandler_t task_handler);
```
- **功能**：恢复挂起的任务
- **参数**：`task_handler` - 任务句柄，NULL表示恢复当前任务
- **效果**：任务重新参与调度

### 任务控制

#### 任务延时
```c
EK_Result_t EK_rTaskDelay(uint16_t delay_ms);
```
- **功能**：设置当前任务的执行间隔
- **参数**：`delay_ms` - 延时时间(毫秒)
- **机制**：任务执行完毕后等待指定时间再次就绪

#### 优先级设置
```c
EK_Result_t EK_rTaskSetPriority(EK_pTaskHandler_t task_handler, uint8_t Priority);
```
- **功能**：动态调整任务优先级
- **参数**：
  - `task_handler` - 任务句柄，NULL表示当前任务
  - `Priority` - 新优先级值
- **效果**：自动重新排序任务队列

### 状态查询

#### 任务信息获取
```c
EK_Result_t EK_rTaskGetInfo(EK_pTaskHandler_t task_handler, EK_TaskInfo_t *task_info);
```
- **功能**：获取任务详细信息
- **用途**：性能监控、调试分析

#### 内存使用查询
```c
size_t EK_sTaskGetFreeMemory(void);
```
- **功能**：获取任务系统剩余内存
- **返回值**：内存池剩余字节数

## 使用场景与示例

### 1. 周期性监控任务
```c
void sensor_monitor_task(void) {
    static int init_flag = 0;
    
    if (!init_flag) {
        sensor_init();
        EK_rTaskDelay(1000); // 设置1秒执行周期
        init_flag = 1;
        return;
    }
    
    // 读取传感器数据
    float temperature = read_temperature();
    float humidity = read_humidity();
    
    // 数据处理和上报
    if (temperature > 35.0) {
        trigger_cooling_system();
    }
    
    log_sensor_data(temperature, humidity);
}

void create_monitor_tasks(void) {
    EK_pTaskHandler_t sensor_task;
    EK_rTaskCreate_Dynamic(sensor_monitor_task, 1, &sensor_task);
}
```

### 2. 多任务协调系统
```c
// LED闪烁任务
void led_blink_task(void) {
    static bool led_state = false;
    
    led_state = !led_state;
    set_led(LED1, led_state);
    
    EK_rTaskDelay(500); // 500ms闪烁间隔
}

// 按键扫描任务
void key_scan_task(void) {
    static uint8_t key_state = 0;
    
    uint8_t current_key = scan_keys();
    
    // 检测按键按下
    if (current_key && !key_state) {
        handle_key_press(current_key);
    }
    
    key_state = current_key;
    EK_rTaskDelay(20); // 20ms扫描间隔
}

// 通信处理任务
void comm_process_task(void) {
    if (has_received_data()) {
        uint8_t buffer[64];
        int len = receive_data(buffer, sizeof(buffer));
        process_protocol(buffer, len);
    }
    
    if (has_data_to_send()) {
        send_pending_data();
    }
    
    EK_rTaskDelay(10); // 10ms处理间隔
}

void setup_application_tasks(void) {
    EK_pTaskHandler_t led_task, key_task, comm_task;
    
    // 创建不同优先级的任务
    EK_rTaskCreate_Dynamic(led_blink_task, 3, &led_task);      // 低优先级
    EK_rTaskCreate_Dynamic(key_scan_task, 1, &key_task);       // 高优先级
    EK_rTaskCreate_Dynamic(comm_process_task, 2, &comm_task);  // 中优先级
}
```

### 3. 动态任务管理
```c
typedef struct {
    EK_pTaskHandler_t task_handler;
    bool is_running;
    char name[16];
} TaskManager_t;

static TaskManager_t task_pool[10];
static int task_count = 0;

// 动态创建任务
int create_named_task(void (*func)(void), uint8_t priority, const char *name) {
    if (task_count >= 10) return -1;
    
    EK_pTaskHandler_t handler;
    if (EK_rTaskCreate_Dynamic(func, priority, &handler) == EK_OK) {
        task_pool[task_count].task_handler = handler;
        task_pool[task_count].is_running = true;
        strncpy(task_pool[task_count].name, name, 15);
        task_pool[task_count].name[15] = '\0';
        return task_count++;
    }
    return -1;
}

// 停止任务
void stop_task(int task_id) {
    if (task_id >= 0 && task_id < task_count) {
        EK_rTaskSuspend(task_pool[task_id].task_handler);
        task_pool[task_id].is_running = false;
    }
}

// 重启任务
void restart_task(int task_id) {
    if (task_id >= 0 && task_id < task_count) {
        EK_rTaskResume(task_pool[task_id].task_handler);
        task_pool[task_id].is_running = true;
    }
}

// 任务状态监控
void print_task_status(void) {
    printf("=== Task Status ===\n");
    for (int i = 0; i < task_count; i++) {
        EK_TaskInfo_t info;
        if (EK_rTaskGetInfo(task_pool[i].task_handler, &info) == EK_OK) {
            printf("Task[%d] %s: Priority=%d, MaxTime=%dms, State=%s\n",
                   i, task_pool[i].name, info.Priority, info.MaxUsedTime,
                   info.isActive ? "Active" : "Suspended");
        }
    }
    printf("Free Memory: %zu bytes\n", EK_sTaskGetFreeMemory());
}
```

### 4. 实时性要求任务
```c
// 高优先级实时任务
void realtime_control_task(void) {
    // 关键控制算法
    read_sensor_fast();
    calculate_control_output();
    update_actuators();
    
    EK_rTaskDelay(1); // 1ms控制周期
}

// 中等优先级处理任务
void data_process_task(void) {
    static int process_counter = 0;
    
    // 每10个周期执行一次复杂处理
    if (++process_counter >= 10) {
        complex_data_processing();
        process_counter = 0;
    }
    
    EK_rTaskDelay(5); // 5ms处理周期
}

// 低优先级后台任务
void background_task(void) {
    // 系统维护工作
    garbage_collection();
    system_health_check();
    
    EK_rTaskDelay(1000); // 1秒执行一次
}

void setup_realtime_system(void) {
    EK_pTaskHandler_t rt_task, proc_task, bg_task;
    
    // 按实时性要求分配优先级
    EK_rTaskCreate_Dynamic(realtime_control_task, 0, &rt_task);   // 最高优先级
    EK_rTaskCreate_Dynamic(data_process_task, 5, &proc_task);     // 中优先级  
    EK_rTaskCreate_Dynamic(background_task, 10, &bg_task);        // 最低优先级
}
```

### 5. 静态任务系统
```c
// 静态任务节点和控制块
static EK_TaskNode_t led_node, uart_node, timer_node;
static EK_TaskHandler_t led_handler, uart_handler, timer_handler;

void setup_static_tasks(void) {
    // 配置LED任务
    led_handler.Task_TrigTime = 0;
    led_handler.Task_Info = 0;
    led_handler.TaskCallBack.StaticCallBack = led_blink_task;
    led_handler.Task_Priority = 2;
    led_handler.Task_MaxUsed = 0;
    EK_pTaskCreate_Static(&led_node, &led_handler);
    
    // 配置UART任务
    uart_handler.Task_TrigTime = 0;
    uart_handler.Task_Info = 0;
    uart_handler.TaskCallBack.StaticCallBack = uart_process_task;
    uart_handler.Task_Priority = 1;
    uart_handler.Task_MaxUsed = 0;
    EK_pTaskCreate_Static(&uart_node, &uart_handler);
    
    // 配置定时器任务
    timer_handler.Task_TrigTime = 0;
    timer_handler.Task_Info = 0;
    timer_handler.TaskCallBack.StaticCallBack = timer_service_task;
    timer_handler.Task_Priority = 3;
    timer_handler.Task_MaxUsed = 0;
    EK_pTaskCreate_Static(&timer_node, &timer_handler);
    
    printf("Static task system initialized\n");
}
```

## 系统集成

### 主程序框架
```c
// 系统时钟函数
uint32_t get_system_tick(void) {
    return HAL_GetTick(); // 使用HAL库获取系统时钟
}

// 用户任务初始化（弱定义函数实现）
bool TaskCreation(void) {
    setup_application_tasks();
    return true;
}

// 空闲任务（弱定义函数实现）
void TaskIdle(void) {
    // 进入低功耗模式或执行后台维护
    __WFI(); // ARM Cortex-M 进入等待中断模式
}

int main(void) {
    // 硬件初始化
    system_init();
    
    // 任务系统初始化
    if (EK_rTaskInit() != EK_OK) {
        printf("Task system initialization failed!\n");
        return -1;
    }
    
    // 启动任务调度器
    printf("Starting task scheduler...\n");
    EK_vTaskStart(get_system_tick);
    
    // 永远不会执行到这里
    return 0;
}
```

## 调度算法详解

### 1. 优先级调度
```c
// 任务按优先级插入队列（数值越小优先级越高）
static EK_Result_t _task_insert_node(EK_TaskSchedule_t *list, EK_TaskNode_t *node) {
    // 空队列或优先级最高，插入头部
    if (list->Count == 0 || node->TaskHandler.Task_Priority < list->Head->TaskHandler.Task_Priority) {
        // 插入头部逻辑
    }
    
    // 遍历查找合适位置
    EK_TaskNode_t *p = list->Head;
    while (p->Next != NULL) {
        if (p->Next->TaskHandler.Task_Priority >= node->TaskHandler.Task_Priority) {
            // 找到插入位置
            break;
        }
        p = p->Next;
    }
    // 插入节点
}
```

### 2. 时间管理
```c
// 倒计时处理（每1ms执行一次）
while (p != NULL) {
    uint16_t cur_time = TASK_GET_CUR_TIME(p->TaskHandler.Task_TrigTime);
    if (cur_time > 0) {
        cur_time--;
        p->TaskHandler.Task_TrigTime = TASK_SET_CUR_TIME(p->TaskHandler.Task_TrigTime, cur_time);
    }
    
    // 倒计时到0，移动到运行队列
    if (cur_time == 0) {
        _task_move_node(&WaitSchedule, &RunSchedule, p);
    }
}
```

### 3. 任务执行
```c
// 按优先级顺序执行就绪任务
EK_TaskNode_t *ptr = RunSchedule.Head;
while (ptr != NULL) {
    // 检查任务是否被挂起
    if (!TASK_IS_ACTIVE(ptr->TaskHandler.Task_Info)) {
        // 挂起任务移回等待队列
        continue;
    }
    
    // 记录执行开始时间
    uint32_t start_tick = tick_get();
    _CurTask_Handler = &ptr->TaskHandler;
    
    // 执行任务函数
    if (TASK_IS_STATIC(ptr->TaskHandler.Task_Info)) {
        ptr->TaskHandler.TaskCallBack.StaticCallBack();
    } else {
        (*(ptr->TaskHandler.TaskCallBack.DynamicCallBack))();
    }
    
    // 记录执行时间统计
    uint32_t exec_time = tick_get() - start_tick;
    if (exec_time > ptr->TaskHandler.Task_MaxUsed) {
        ptr->TaskHandler.Task_MaxUsed = exec_time;
    }
    
    // 任务执行完成，移回等待队列
    _task_move_node(&RunSchedule, &WaitSchedule, ptr);
}
```

## 性能特点

### 时间复杂度
- **任务创建**：O(n) - 需要按优先级插入队列
- **任务删除**：O(1) - 利用双向关联快速定位
- **优先级调整**：O(n) - 需要重新排序
- **调度开销**：O(n) - 遍历就绪队列

### 内存开销
- **静态任务**：sizeof(EK_TaskNode_t) ≈ 32字节/任务
- **动态任务**：32字节 + sizeof(void*) ≈ 40字节/任务
- **系统开销**：两个调度队列结构体 ≈ 24字节

### 实时性
- **调度延迟**：取决于当前执行任务的耗时
- **时间精度**：1ms（基于系统时钟）
- **优先级响应**：非抢占，需等待当前任务完成

## 设计优势

1. **简单可靠**：非抢占式避免了复杂的同步问题
2. **资源高效**：双队列设计减少不必要的遍历
3. **灵活配置**：支持静态和动态两种创建方式
4. **完整监控**：提供丰富的状态查询和统计信息
5. **易于调试**：任务执行时间统计便于性能优化

## 注意事项

1. **任务函数设计**：避免长时间阻塞操作，使用EK_rTaskDelay()替代阻塞等待
2. **优先级分配**：合理分配优先级，避免低优先级任务饿死
3. **内存管理**：动态任务记得适时删除，避免内存泄漏
4. **实时性权衡**：非抢占式调度适合对实时性要求不严格的应用
5. **堆栈管理**：任务函数运行在同一堆栈上，注意堆栈大小配置

该任务调度器为嵌入式系统提供了一个轻量级、高效的多任务解决方案，特别适用于资源受限且对实时性要求适中的应用场景。通过合理的任务设计和优先级配置，可以构建出稳定可靠的多任务系统。
