# 示例代码

本目录包含各种 EmbedKit 使用示例，从基础到高级应用。

## 📁 示例分类

### 基础示例
- [Hello World](basic/hello_world.c) - 最简单的 EmbedKit 程序
- [任务调度](basic/task_scheduling.c) - 基础任务调度
- [内存池使用](basic/memory_pool.c) - 内存池基本操作
- [队列操作](basic/queue_operations.c) - 队列的使用

### 中级示例
- [多任务协作](intermediate/multi_task.c) - 多任务系统
- [事件驱动](intermediate/event_driven.c) - 事件驱动编程
- [定时器应用](intermediate/timer_usage.c) - 软件定时器
- [生产者消费者](intermediate/producer_consumer.c) - 经典并发模式

### 高级示例
- [实时数据采集](advanced/realtime_sampling.c) - 高速数据采集
- [协议栈实现](advanced/protocol_stack.c) - 简单协议栈
- [状态机框架](advanced/state_machine.c) - 有限状态机
- [电源管理](advanced/power_management.c) - 低功耗设计

### 完整项目
- [智能传感器节点](projects/sensor_node/) - IoT 传感器节点
- [电机控制器](projects/motor_controller/) - 步进电机控制
- [数据记录器](projects/data_logger/) - SD 卡数据记录
- [Modbus 从站](projects/modbus_slave/) - 工业通信

## 🎯 基础示例详解

### 1. Hello World

最简单的 EmbedKit 程序，演示基本框架：

```c
// hello_world.c
#include "embedkit.h"

void hello_task(void* param) {
    static int count = 0;
    printf("Hello EmbedKit! Count: %d\n", ++count);
}

int main(void) {
    // 初始化系统
    ek_system_init();
    
    // 创建任务，每秒执行一次
    ek_task_create(hello_task, NULL, 1000, TASK_PRIORITY_NORMAL);
    
    // 启动调度器
    ek_scheduler_start();
    
    return 0;
}
```

### 2. LED 闪烁

经典的 LED 闪烁示例：

```c
// led_blink.c
#include "embedkit.h"
#include "hal_gpio.h"

#define LED_PIN GPIO_PIN_13
#define BLINK_PERIOD_MS 500

void led_task(void* param) {
    static uint8_t state = 0;
    state = !state;
    hal_gpio_write(LED_PIN, state);
}

int main(void) {
    // 硬件初始化
    hal_init();
    hal_gpio_config(LED_PIN, GPIO_MODE_OUTPUT);
    
    // 初始化 EmbedKit
    ek_system_init();
    
    // 创建 LED 任务
    ek_task_create(led_task, NULL, BLINK_PERIOD_MS, TASK_PRIORITY_LOW);
    
    // 启动调度器
    ek_scheduler_start();
    
    return 0;
}
```

### 3. 按键处理

带消抖的按键输入处理：

```c
// button_handler.c
#include "embedkit.h"

typedef struct {
    uint8_t pin;
    uint8_t state;
    uint8_t last_state;
    uint32_t debounce_time;
    void (*callback)(void);
} button_t;

void button_task(void* param) {
    button_t* btn = (button_t*)param;
    uint8_t current = hal_gpio_read(btn->pin);
    
    if (current != btn->last_state) {
        btn->debounce_time = ek_get_tick();
    }
    
    if ((ek_get_tick() - btn->debounce_time) > 50) {
        if (current != btn->state) {
            btn->state = current;
            if (btn->state == 0 && btn->callback) {  // 按下
                btn->callback();
            }
        }
    }
    
    btn->last_state = current;
}

void on_button_press(void) {
    printf("Button pressed!\n");
}

int main(void) {
    static button_t button = {
        .pin = GPIO_PIN_0,
        .callback = on_button_press
    };
    
    hal_gpio_config(button.pin, GPIO_MODE_INPUT_PULLUP);
    
    ek_system_init();
    ek_task_create(button_task, &button, 10, TASK_PRIORITY_HIGH);
    ek_scheduler_start();
    
    return 0;
}
```

## 🔧 中级示例详解

### 1. 串口通信

异步串口收发示例：

```c
// uart_communication.c
#include "embedkit.h"

#define UART_BUFFER_SIZE 256
#define UART_BAUDRATE 115200

// 环形缓冲区
static uint8_t rx_buffer[UART_BUFFER_SIZE];
static ek_ring_t rx_ring;

// 接收中断处理
void uart_rx_isr(void) {
    uint8_t data = UART_READ_REG();
    ek_ring_put(&rx_ring, data);
}

// 串口处理任务
void uart_task(void* param) {
    uint8_t data;
    static char line_buffer[128];
    static int index = 0;
    
    // 处理接收数据
    while (ek_ring_get(&rx_ring, &data)) {
        if (data == '\n' || data == '\r') {
            if (index > 0) {
                line_buffer[index] = '\0';
                process_command(line_buffer);
                index = 0;
            }
        } else if (index < sizeof(line_buffer) - 1) {
            line_buffer[index++] = data;
        }
    }
}

void process_command(const char* cmd) {
    if (strcmp(cmd, "status") == 0) {
        send_status();
    } else if (strcmp(cmd, "reset") == 0) {
        system_reset();
    } else {
        uart_send("Unknown command\r\n");
    }
}

int main(void) {
    // 初始化硬件
    uart_init(UART_BAUDRATE);
    uart_enable_rx_interrupt();
    
    // 初始化环形缓冲
    ek_ring_init(&rx_ring, rx_buffer, UART_BUFFER_SIZE);
    
    // 创建串口任务
    ek_system_init();
    ek_task_create(uart_task, NULL, 10, TASK_PRIORITY_NORMAL);
    ek_scheduler_start();
    
    return 0;
}
```

### 2. 传感器数据采集

多传感器数据采集和处理：

```c
// sensor_acquisition.c
#include "embedkit.h"

typedef struct {
    float temperature;
    float humidity;
    float pressure;
    uint32_t timestamp;
} sensor_data_t;

// 内存池
#define POOL_SIZE 2048
#define DATA_BLOCK_SIZE sizeof(sensor_data_t)
static uint8_t pool_buffer[POOL_SIZE];
static ek_pool_t* data_pool;

// 数据队列
#define QUEUE_SIZE 16
static sensor_data_t queue_buffer[QUEUE_SIZE];
static ek_queue_t data_queue;

// 传感器读取任务
void sensor_read_task(void* param) {
    sensor_data_t* data = ek_pool_alloc(data_pool);
    if (data) {
        // 读取传感器
        data->temperature = read_temperature();
        data->humidity = read_humidity();
        data->pressure = read_pressure();
        data->timestamp = ek_get_tick();
        
        // 放入队列
        if (!ek_queue_push(&data_queue, data)) {
            ek_pool_free(data_pool, data);
        }
    }
}

// 数据处理任务
void data_process_task(void* param) {
    sensor_data_t* data;
    
    while (ek_queue_pop(&data_queue, &data)) {
        // 处理数据
        if (data->temperature > 50.0) {
            trigger_alarm(ALARM_OVERTEMP);
        }
        
        // 记录数据
        log_sensor_data(data);
        
        // 释放内存
        ek_pool_free(data_pool, data);
    }
}

int main(void) {
    // 初始化传感器
    sensor_init();
    
    // 创建内存池
    data_pool = ek_pool_create(pool_buffer, POOL_SIZE, DATA_BLOCK_SIZE);
    
    // 初始化队列
    ek_queue_init(&data_queue, queue_buffer, 
                  sizeof(sensor_data_t*), QUEUE_SIZE);
    
    // 创建任务
    ek_system_init();
    ek_task_create(sensor_read_task, NULL, 1000, TASK_PRIORITY_HIGH);
    ek_task_create(data_process_task, NULL, 100, TASK_PRIORITY_NORMAL);
    ek_scheduler_start();
    
    return 0;
}
```

## 🚀 高级示例详解

### 1. 状态机实现

通用状态机框架：

```c
// state_machine.c
#include "embedkit.h"

// 状态定义
typedef enum {
    STATE_IDLE,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_ERROR
} state_t;

// 事件定义
typedef enum {
    EVENT_START,
    EVENT_STOP,
    EVENT_PAUSE,
    EVENT_RESUME,
    EVENT_ERROR
} event_t;

// 状态机结构
typedef struct {
    state_t current_state;
    void (*state_handler)(event_t event);
    void* context;
} state_machine_t;

// 状态处理函数
void idle_handler(state_machine_t* sm, event_t event) {
    switch (event) {
        case EVENT_START:
            sm->current_state = STATE_RUNNING;
            printf("Starting...\n");
            break;
        default:
            break;
    }
}

void running_handler(state_machine_t* sm, event_t event) {
    switch (event) {
        case EVENT_PAUSE:
            sm->current_state = STATE_PAUSED;
            printf("Pausing...\n");
            break;
        case EVENT_STOP:
            sm->current_state = STATE_IDLE;
            printf("Stopping...\n");
            break;
        case EVENT_ERROR:
            sm->current_state = STATE_ERROR;
            printf("Error occurred!\n");
            break;
        default:
            break;
    }
}

// 状态机处理
void state_machine_process(state_machine_t* sm, event_t event) {
    switch (sm->current_state) {
        case STATE_IDLE:
            idle_handler(sm, event);
            break;
        case STATE_RUNNING:
            running_handler(sm, event);
            break;
        // ... 其他状态
    }
}
```

### 2. 协议解析器

简单协议解析实现：

```c
// protocol_parser.c
#include "embedkit.h"

// 协议格式: [STX][LEN][CMD][DATA...][CRC][ETX]
#define STX 0x02
#define ETX 0x03
#define MAX_PACKET_SIZE 256

typedef enum {
    PARSE_WAIT_STX,
    PARSE_GET_LEN,
    PARSE_GET_CMD,
    PARSE_GET_DATA,
    PARSE_GET_CRC,
    PARSE_WAIT_ETX
} parse_state_t;

typedef struct {
    parse_state_t state;
    uint8_t buffer[MAX_PACKET_SIZE];
    uint16_t index;
    uint16_t length;
    uint8_t cmd;
    uint16_t crc;
} parser_t;

void parser_init(parser_t* parser) {
    parser->state = PARSE_WAIT_STX;
    parser->index = 0;
}

bool parser_feed(parser_t* parser, uint8_t byte) {
    switch (parser->state) {
        case PARSE_WAIT_STX:
            if (byte == STX) {
                parser->state = PARSE_GET_LEN;
                parser->index = 0;
            }
            break;
            
        case PARSE_GET_LEN:
            parser->length = byte;
            parser->state = PARSE_GET_CMD;
            break;
            
        case PARSE_GET_CMD:
            parser->cmd = byte;
            parser->state = PARSE_GET_DATA;
            break;
            
        case PARSE_GET_DATA:
            parser->buffer[parser->index++] = byte;
            if (parser->index >= parser->length - 1) {
                parser->state = PARSE_GET_CRC;
                parser->index = 0;
            }
            break;
            
        case PARSE_GET_CRC:
            if (parser->index == 0) {
                parser->crc = byte << 8;
                parser->index = 1;
            } else {
                parser->crc |= byte;
                parser->state = PARSE_WAIT_ETX;
            }
            break;
            
        case PARSE_WAIT_ETX:
            if (byte == ETX) {
                // 验证CRC
                uint16_t calc_crc = calculate_crc(parser->buffer, 
                                                  parser->length - 1);
                if (calc_crc == parser->crc) {
                    process_packet(parser->cmd, parser->buffer, 
                                 parser->length - 1);
                    parser_init(parser);
                    return true;
                }
            }
            parser_init(parser);
            break;
    }
    return false;
}
```

## 📦 完整项目示例

### 温度控制器

带 PID 控制的温度调节系统：

```c
// temperature_controller.c
#include "embedkit.h"

// PID 参数
typedef struct {
    float kp, ki, kd;
    float integral;
    float last_error;
    float output_min, output_max;
} pid_controller_t;

// 系统状态
typedef struct {
    float setpoint;
    float current_temp;
    float output;
    pid_controller_t pid;
    bool enabled;
} temp_control_t;

static temp_control_t g_control = {
    .setpoint = 25.0,
    .pid = {
        .kp = 2.0,
        .ki = 0.5,
        .kd = 1.0,
        .output_min = 0,
        .output_max = 100
    }
};

// PID 计算
float pid_calculate(pid_controller_t* pid, float setpoint, float measured) {
    float error = setpoint - measured;
    
    // P项
    float p_term = pid->kp * error;
    
    // I项
    pid->integral += error;
    float i_term = pid->ki * pid->integral;
    
    // D项
    float d_term = pid->kd * (error - pid->last_error);
    pid->last_error = error;
    
    // 总输出
    float output = p_term + i_term + d_term;
    
    // 限幅
    if (output > pid->output_max) {
        output = pid->output_max;
        pid->integral -= error;  // 抗积分饱和
    } else if (output < pid->output_min) {
        output = pid->output_min;
        pid->integral -= error;
    }
    
    return output;
}

// 温度读取任务
void temp_read_task(void* param) {
    g_control.current_temp = read_temperature_sensor();
}

// 控制任务
void control_task(void* param) {
    if (g_control.enabled) {
        g_control.output = pid_calculate(&g_control.pid,
                                        g_control.setpoint,
                                        g_control.current_temp);
        
        // 设置加热器功率
        set_heater_power(g_control.output);
    }
}

// 显示任务
void display_task(void* param) {
    printf("Temp: %.1f°C, Target: %.1f°C, Output: %.1f%%\n",
           g_control.current_temp,
           g_control.setpoint,
           g_control.output);
}

// 命令处理
void command_handler(const char* cmd) {
    float value;
    if (sscanf(cmd, "SET %f", &value) == 1) {
        g_control.setpoint = value;
        printf("Setpoint changed to %.1f°C\n", value);
    } else if (strcmp(cmd, "START") == 0) {
        g_control.enabled = true;
        printf("Control started\n");
    } else if (strcmp(cmd, "STOP") == 0) {
        g_control.enabled = false;
        set_heater_power(0);
        printf("Control stopped\n");
    }
}

int main(void) {
    // 硬件初始化
    hardware_init();
    
    // EmbedKit 初始化
    ek_system_init();
    
    // 创建任务
    ek_task_create(temp_read_task, NULL, 100, TASK_PRIORITY_HIGH);
    ek_task_create(control_task, NULL, 200, TASK_PRIORITY_HIGH);
    ek_task_create(display_task, NULL, 1000, TASK_PRIORITY_LOW);
    
    // 启动系统
    ek_scheduler_start();
    
    return 0;
}
```

## 🎓 学习路径

建议按以下顺序学习示例：

1. **入门级**
   - Hello World → LED 闪烁 → 按键处理

2. **基础级**
   - 任务调度 → 内存池 → 队列操作

3. **进阶级**
   - 串口通信 → 传感器采集 → 定时器应用

4. **高级**
   - 状态机 → 协议栈 → 实时系统

5. **项目级**
   - 选择一个完整项目深入研究

## 📝 编译和运行

### 编译单个示例

```bash
# 编译基础示例
make EXAMPLE=basic/hello_world

# 编译并下载
make EXAMPLE=basic/led_blink flash

# 清理
make clean
```

### 调试示例

```bash
# 使用 GDB 调试
make EXAMPLE=basic/task_scheduling debug

# 使用 J-Link
make EXAMPLE=intermediate/multi_task jlink
```

## 🔗 相关资源

- [API 文档](../api/README.md)
- [配置指南](../getting_started/build_config.md)
- [最佳实践](../best_practices/README.md)
- [移植指南](../porting/overview.md)

## 💡 贡献示例

欢迎贡献新的示例代码！请确保：

1. 代码风格一致
2. 包含详细注释
3. 提供 README 说明
4. 经过实际测试

提交 PR 到 `examples` 分支。