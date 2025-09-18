# EK_Serial - 串口队列管理组件

## 概述

EK_Serial 是一个基于队列和链表实现的串口数据缓冲和定时发送管理组件，专为嵌入式系统设计。该组件提供统一的串口数据发送管理机制，支持多串口实例管理、优先级调度、定时发送和流量控制等功能。

## 核心特性

- **多串口管理**：支持管理多个串口实例，每个实例独立缓冲和发送
- **优先级调度**：基于链表的优先级排序，高优先级串口优先处理
- **定时发送机制**：可配置的发送超时时间，避免数据积压
- **流量控制**：支持队列满时的数据处理策略（丢弃新数据或覆盖旧数据）
- **格式化输出**：类似printf的格式化数据输出接口
- **双内存模式**：支持静态和动态内存分配方式
- **回调机制**：每个串口实例可绑定独立的发送回调函数

## 数据结构

### EK_SeiralQueue_t - 串口队列结构体
```c
typedef struct
{
    EK_Node_t *Serial_Owner;                     // 拥有该结构的链表节点
    EK_Queue_t *Serial_Queue;                    // 数据缓冲队列
    uint8_t Serial_Timer;                        // 定时器
    void (*Serial_SendCallBack)(void *, EK_Size_t); // 数据发送回调函数
} EK_SeiralQueue_t;
```

### 关键字段说明
- **Serial_Owner**：指向管理链表中的节点，用于优先级排序和实例管理
- **Serial_Queue**：数据缓冲队列，存储待发送的数据
- **Serial_Timer**：发送定时器，控制数据发送时机
- **Serial_SendCallBack**：硬件发送回调函数，执行实际的数据传输

## 配置宏定义

### 缓冲区配置
```c
#define SERIAL_TX_BUFFER 256        // 发送缓冲区大小（字节）
#define SERIAL_MAX_SEND_SIZE 128    // 单次发送最大字节数
```

### 时间配置
```c
#define SERIAL_OVER_TIME 50         // 发送超时时间（ms）
#define SERIAL_POLL_INTERVAL 5      // 轮询间隔（ms）
```

### 策略配置
```c
#define SERIAL_FULL_STRATEGY 1      // 队列满处理策略
                                   // 0: 丢弃新数据
                                   // 1: 丢弃最老数据
```

## API 接口

### 系统初始化

#### 动态初始化
```c
EK_Result_t EK_rSerialInit_Dynamic(void);
```
- **功能**：动态分配内存初始化串口管理系统
- **返回值**：操作结果状态码
- **特性**：全局只能初始化一次，创建管理链表

#### 静态初始化
```c
EK_Result_t EK_rSerialInit_Static(void);
```
- **功能**：使用静态内存初始化串口管理系统
- **返回值**：操作结果状态码
- **特性**：适用于内存受限的嵌入式环境

### 串口实例创建

#### 动态创建串口队列
```c
EK_Result_t EK_rSerialCreateQueue_Dyanmic(EK_pSeiralQueue_t *serial_fifo,
                                          void (*send_func)(void *, EK_Size_t),
                                          uint16_t priority,
                                          EK_Size_t capacity);
```
- **功能**：动态创建串口队列实例
- **参数**：
  - `serial_fifo`：串口队列句柄指针的指针
  - `send_func`：硬件发送回调函数
  - `priority`：优先级（数值越小优先级越高）
  - `capacity`：队列容量（字节数）
- **特性**：自动分配所有必需内存，按优先级插入管理链表

#### 静态创建串口队列
```c
EK_Result_t EK_rSerialCreateQueue_Static(EK_pSeiralQueue_t serial_fifo, 
                                        void *buffer, 
                                        void (*send_func)(void *, EK_Size_t), 
                                        uint16_t priority, 
                                        EK_Size_t capacity);
```
- **功能**：使用用户提供的内存创建串口队列
- **参数**：
  - `serial_fifo`：用户分配的串口队列结构体
  - `buffer`：用户分配的缓冲区
  - `send_func`：硬件发送回调函数
  - `priority`：优先级
  - `capacity`：缓冲区大小
- **特性**：无动态内存分配，适合静态内存管理

### 数据发送

#### 格式化数据发送
```c
EK_Result_t EK_rSerialPrintf(EK_pSeiralQueue_t serial_fifo, const char *format, ...);
```
- **功能**：向指定串口队列发送格式化数据
- **参数**：
  - `serial_fifo`：目标串口队列句柄
  - `format`：格式化字符串
  - `...`：可变参数
- **特性**：
  - 类似printf的格式化输出
  - 支持数据截断处理
  - 队列满时可选择处理策略
  - 自动启动发送定时器

### 系统轮询

#### 串口服务轮询
```c
EK_Result_t EK_rSerialPoll(uint32_t (*get_tick)(void));
```
- **功能**：串口服务轮询函数，处理所有串口实例的数据发送
- **参数**：`get_tick` - 获取系统时间的函数指针
- **特性**：
  - 按优先级顺序处理串口实例
  - 定时发送机制
  - 限制单次发送数据量
  - 自动内存管理

## 使用场景与示例

### 1. 多串口调试输出系统
```c
// 系统初始化
EK_rSerialInit_Dynamic();

// 创建不同优先级的串口实例
EK_pSeiralQueue_t debug_uart;    // 调试串口
EK_pSeiralQueue_t log_uart;      // 日志串口
EK_pSeiralQueue_t cmd_uart;      // 命令串口

// 硬件发送回调函数
void debug_uart_send(void *data, EK_Size_t len) {
    HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
}

void log_uart_send(void *data, EK_Size_t len) {
    HAL_UART_Transmit(&huart2, (uint8_t*)data, len, HAL_MAX_DELAY);
}

void cmd_uart_send(void *data, EK_Size_t len) {
    HAL_UART_Transmit(&huart3, (uint8_t*)data, len, HAL_MAX_DELAY);
}

// 创建串口实例
EK_rSerialCreateQueue_Dyanmic(&debug_uart, debug_uart_send, 10, 512);  // 低优先级
EK_rSerialCreateQueue_Dyanmic(&log_uart, log_uart_send, 5, 1024);      // 中优先级
EK_rSerialCreateQueue_Dyanmic(&cmd_uart, cmd_uart_send, 1, 256);       // 高优先级

// 发送数据
EK_rSerialPrintf(debug_uart, "Debug: sensor value = %d\n", sensor_val);
EK_rSerialPrintf(log_uart, "Log: [%s] System started\n", get_timestamp());
EK_rSerialPrintf(cmd_uart, "CMD> Ready for commands\n");

// 主循环轮询
while(1) {
    EK_rSerialPoll(HAL_GetTick);
    // 其他任务
}
```

### 2. 静态内存管理示例
```c
// 静态分配内存
static EK_SeiralQueue_t uart1_instance;
static uint8_t uart1_buffer[512];
static EK_SeiralQueue_t uart2_instance;
static uint8_t uart2_buffer[256];

// 系统初始化
void system_init(void) {
    // 静态初始化串口管理系统
    EK_rSerialInit_Static();
    
    // 创建静态串口实例
    EK_rSerialCreateQueue_Static(&uart1_instance, uart1_buffer, 
                                uart1_send_callback, 1, sizeof(uart1_buffer));
    EK_rSerialCreateQueue_Static(&uart2_instance, uart2_buffer, 
                                uart2_send_callback, 2, sizeof(uart2_buffer));
}

// 使用示例
void application_task(void) {
    static uint32_t counter = 0;
    
    EK_rSerialPrintf(&uart1_instance, "Counter: %lu\n", counter++);
    EK_rSerialPrintf(&uart2_instance, "Status: OK\n");
}
```

### 3. 实时数据监控系统
```c
// 传感器数据结构
typedef struct {
    float temperature;
    float humidity;
    uint16_t pressure;
    uint32_t timestamp;
} SensorData_t;

EK_pSeiralQueue_t monitor_uart;

// 创建监控串口
void monitor_init(void) {
    EK_rSerialInit_Dynamic();
    EK_rSerialCreateQueue_Dyanmic(&monitor_uart, monitor_send, 1, 2048);
}

// 发送传感器数据
void send_sensor_data(SensorData_t *data) {
    EK_rSerialPrintf(monitor_uart, 
                    "SENSOR,%.2f,%.2f,%u,%lu\n",
                    data->temperature,
                    data->humidity, 
                    data->pressure,
                    data->timestamp);
}

// 定时器中断处理
void TIM_IRQHandler(void) {
    static SensorData_t sensor_data;
    
    // 读取传感器数据
    read_sensors(&sensor_data);
    sensor_data.timestamp = HAL_GetTick();
    
    // 发送数据
    send_sensor_data(&sensor_data);
}
```

### 4. 命令行接口系统
```c
EK_pSeiralQueue_t cli_uart;

// CLI初始化
void cli_init(void) {
    EK_rSerialCreateQueue_Dyanmic(&cli_uart, cli_send, 1, 1024);
    
    // 发送欢迎信息
    EK_rSerialPrintf(cli_uart, "\n=== System CLI v1.0 ===\n");
    EK_rSerialPrintf(cli_uart, "Type 'help' for commands\n");
    EK_rSerialPrintf(cli_uart, "CLI> ");
}

// 命令处理函数
void process_command(char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        EK_rSerialPrintf(cli_uart, "Available commands:\n");
        EK_rSerialPrintf(cli_uart, "  status  - Show system status\n");
        EK_rSerialPrintf(cli_uart, "  reset   - Reset system\n");
        EK_rSerialPrintf(cli_uart, "  version - Show version\n");
    }
    else if (strcmp(cmd, "status") == 0) {
        EK_rSerialPrintf(cli_uart, "System Status: Running\n");
        EK_rSerialPrintf(cli_uart, "Uptime: %lu ms\n", HAL_GetTick());
        EK_rSerialPrintf(cli_uart, "Free Memory: %u bytes\n", get_free_memory());
    }
    else if (strcmp(cmd, "version") == 0) {
        EK_rSerialPrintf(cli_uart, "Firmware Version: 1.2.3\n");
        EK_rSerialPrintf(cli_uart, "Build Date: %s %s\n", __DATE__, __TIME__);
    }
    else {
        EK_rSerialPrintf(cli_uart, "Unknown command: %s\n", cmd);
    }
    
    EK_rSerialPrintf(cli_uart, "CLI> ");
}
```

### 5. 数据记录和分析系统
```c
EK_pSeiralQueue_t data_logger;

// 数据记录器初始化
void logger_init(void) {
    EK_rSerialCreateQueue_Dyanmic(&data_logger, logger_send, 3, 4096);
}

// 记录系统事件
void log_event(const char *level, const char *module, const char *message) {
    uint32_t timestamp = HAL_GetTick();
    EK_rSerialPrintf(data_logger, "[%lu][%s][%s] %s\n", 
                    timestamp, level, module, message);
}

// 记录数据点
void log_data_point(const char *name, float value, const char *unit) {
    EK_rSerialPrintf(data_logger, "DATA,%s,%.3f,%s,%lu\n", 
                    name, value, unit, HAL_GetTick());
}

// 使用示例
void application_example(void) {
    log_event("INFO", "MAIN", "System initialization complete");
    log_data_point("CPU_TEMP", 45.2, "C");
    log_data_point("VOLTAGE", 3.31, "V");
    log_event("WARN", "ADC", "Voltage below threshold");
}
```

## 设计优势

### 1. 统一管理机制
- **集中调度**：所有串口实例通过单一管理链表统一调度
- **优先级控制**：高优先级串口优先处理，确保重要数据及时发送
- **资源共享**：轮询机制避免多个定时器占用资源

### 2. 灵活的内存管理
- **双模式支持**：同时支持静态和动态内存分配
- **按需分配**：动态模式下根据实际需求分配内存
- **内存安全**：完善的内存分配失败处理机制

### 3. 智能流量控制
- **队列满处理**：可配置的队列满处理策略
- **数据截断**：超长数据自动截断，避免缓冲区溢出
- **限流机制**：限制单次发送数据量，保证系统响应性

### 4. 定时发送优化
- **超时机制**：避免数据长时间积压在队列中
- **快速处理**：队列有剩余数据时缩短发送间隔
- **自适应调整**：根据队列状态动态调整发送策略

### 5. 错误处理和恢复
- **参数验证**：完善的输入参数检查
- **异常恢复**：内存分配失败时的自动清理机制
- **状态一致性**：确保系统在异常情况下保持一致状态

## 性能特点

- **初始化开销**：O(1) 时间复杂度，一次性初始化
- **实例创建**：O(n) 时间复杂度（n为现有实例数，用于优先级排序）
- **数据发送**：O(1) 时间复杂度，直接入队操作
- **轮询处理**：O(n) 时间复杂度（n为串口实例数）
- **内存占用**：
  - 管理结构：约64字节（链表 + 全局变量）
  - 每个实例：约48字节（结构体 + 节点） + 用户缓冲区
  - 临时缓冲区：发送时动态分配，用完即释放

## 注意事项

### 使用限制
1. **全局初始化**：系统只能初始化一次，重复初始化会返回错误
2. **线程安全**：模块本身不提供线程同步，多线程环境需要外部保护
3. **回调函数**：发送回调函数应尽快返回，避免阻塞轮询机制
4. **内存管理**：动态创建的实例需要手动释放（当前版本未提供删除接口）

### 配置建议
1. **缓冲区大小**：根据数据流量和发送频率合理配置
2. **轮询间隔**：平衡系统响应性和CPU占用率
3. **发送超时**：根据应用实时性要求设置合适的超时时间
4. **优先级分配**：合理分配优先级，避免低优先级串口饿死

### 性能优化
1. **批量发送**：适当增加单次发送数据量可提高效率
2. **缓冲区预分配**：静态模式下预分配足够的缓冲区
3. **回调优化**：使用DMA等硬件加速减少回调函数执行时间
4. **数据格式**：避免频繁的小数据发送，尽量批量处理

该串口管理组件为嵌入式系统提供了一个高效、灵活的串口数据管理解决方案，特别适用于多串口、多优先级的复杂通信场景。通过统一的管理机制和智能的调度策略，能够显著简化串口通信的开发工作，提高系统的可靠性和可维护性。

## 版本更新

### v1.0 (2025-09-13)
- ✅ 基础串口队列管理功能
- ✅ 多串口实例支持
- ✅ 优先级调度机制
- ✅ 定时发送功能
- ✅ 格式化数据输出
- ✅ 静态/动态内存管理
- ✅ 流量控制策略