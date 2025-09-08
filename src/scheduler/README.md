# 任务调度模块

轻量级的协作式任务调度器，适用于单线程嵌入式系统。

## 功能特性

- ⚡ **协作式调度**: 任务主动让出CPU，适合嵌入式环境
- 🕐 **定时任务**: 支持周期性和延时任务执行
- 📊 **优先级支持**: 多级优先级任务管理
- 💡 **事件驱动**: 支持事件触发的任务执行
- 🔋 **低功耗**: 无任务时自动进入休眠模式

## 核心概念

### 任务(Task)
最小的可调度执行单元，包含：
- 任务函数指针
- 优先级
- 状态(就绪/运行/挂起/休眠)
- 栈空间

### 调度器(Scheduler)  
管理所有任务的执行顺序和状态转换。

## API接口

### 调度器初始化
```c
int scheduler_init(void);
void scheduler_start(void);
void scheduler_stop(void);
```

### 任务管理
```c
// 创建任务
task_t* task_create(task_func_t func, void* arg, 
                   uint8_t priority, size_t stack_size);

// 任务控制
int task_start(task_t* task);
int task_suspend(task_t* task);
int task_resume(task_t* task);
int task_destroy(task_t* task);

// 主动让出CPU
void task_yield(void);
```

### 定时器功能
```c
// 延时执行
int task_delay(uint32_t ms);

// 周期性任务
timer_t* timer_create(uint32_t period_ms, timer_callback_t callback, void* arg);
int timer_start(timer_t* timer);
int timer_stop(timer_t* timer);
```

## 使用示例

### 基本任务调度
```c
#include "scheduler.h"

void task1_func(void* arg) {
    while (1) {
        printf("Task 1 running\n");
        task_delay(1000); // 延时1秒
    }
}

void task2_func(void* arg) {
    while (1) {
        printf("Task 2 running\n");
        task_yield(); // 主动让出CPU
    }
}

int main() {
    // 初始化调度器
    scheduler_init();
    
    // 创建任务
    task_t* task1 = task_create(task1_func, NULL, 1, 512);
    task_t* task2 = task_create(task2_func, NULL, 2, 512);
    
    // 启动任务
    task_start(task1);
    task_start(task2);
    
    // 开始调度
    scheduler_start();
    
    return 0;
}
```

### 定时器示例
```c
void timer_callback(void* arg) {
    printf("Timer expired: %s\n", (char*)arg);
}

int main() {
    scheduler_init();
    
    // 创建1秒周期定时器
    timer_t* timer = timer_create(1000, timer_callback, "Hello Timer");
    timer_start(timer);
    
    scheduler_start();
    return 0;
}
```

## 配置选项

```c
// 最大任务数量
#define MAX_TASKS 16

// 默认栈大小
#define DEFAULT_STACK_SIZE 512

// 时间片长度(ms)
#define TIME_SLICE_MS 10

// 优先级级数
#define PRIORITY_LEVELS 8
```

## 调度算法

采用**优先级抢占式**调度算法：
1. 高优先级任务优先执行
2. 同优先级任务采用时间片轮转
3. 任务主动让出或阻塞时切换
4. 支持优先级继承，避免优先级反转