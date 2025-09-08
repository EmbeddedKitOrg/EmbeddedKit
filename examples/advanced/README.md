# 高级示例

这里包含EmbedKit的高级使用场景和复杂应用示例。

## 示例列表

### 1. 多传感器数据采集系统
演示如何构建一个完整的多传感器数据采集系统。

**功能特性:**
- 多个传感器并发采集
- 数据缓存和过滤
- 异常检测和处理
- 低功耗模式管理

### 2. 实时通信系统
展示实时数据通信的实现方案。

**功能特性:**
- UART/SPI/I2C通信协议栈
- 数据包封装和校验
- 重传机制和错误恢复
- 流量控制

### 3. 状态机框架应用
使用状态机处理复杂的系统状态转换。

**功能特性:**
- 层次化状态机
- 状态转换表
- 事件驱动处理
- 状态持久化

### 4. 嵌入式Web服务器
轻量级HTTP服务器实现。

**功能特性:**
- HTTP/1.1协议支持
- 静态文件服务
- RESTful API
- 内存优化

## 多传感器数据采集系统

```c
#include "embedkit.h"
#include "sensor_manager.h"
#include "data_processor.h"

// 传感器配置
typedef struct {
    uint8_t sensor_id;
    uint32_t sample_rate;  // Hz
    sensor_type_t type;
    bool enabled;
} sensor_config_t;

// 数据采集任务
void sensor_acquisition_task(void* arg) {
    sensor_config_t* config = (sensor_config_t*)arg;
    sensor_data_t data;
    
    while (1) {
        if (config->enabled) {
            // 读取传感器数据
            if (sensor_read(config->sensor_id, &data) == 0) {
                // 数据预处理
                if (data_validate(&data)) {
                    data_filter(&data);
                    
                    // 存储到缓存
                    data_cache_put(&data);
                    
                    // 异常检测
                    if (data_anomaly_detect(&data)) {
                        printf("传感器 %d 检测到异常值\n", config->sensor_id);
                        // 触发异常处理
                        event_post(EVENT_SENSOR_ANOMALY, &data);
                    }
                }
            }
        }
        
        // 根据采样率延时
        task_delay(1000 / config->sample_rate);
    }
}

// 数据处理任务
void data_processing_task(void* arg) {
    sensor_data_t data;
    processed_data_t result;
    
    while (1) {
        // 从缓存获取数据
        if (data_cache_get(&data, 100) == 0) {  // 100ms超时
            // 数据融合和分析
            if (data_fusion(&data, &result) == 0) {
                // 发送处理结果
                communication_send(&result);
                
                // 更新系统状态
                system_state_update(&result);
            }
        }
        
        task_yield();
    }
}

// 系统监控任务
void system_monitor_task(void* arg) {
    system_status_t status;
    
    while (1) {
        // 检查系统资源使用情况
        status.cpu_usage = cpu_usage_get();
        status.memory_usage = memory_get_used_size();
        status.task_count = scheduler_get_task_count();
        
        // 检查电池电量
        status.battery_level = battery_get_level();
        
        if (status.battery_level < BATTERY_LOW_THRESHOLD) {
            printf("电池电量低，进入省电模式\n");
            
            // 降低采样率
            for (int i = 0; i < sensor_count; i++) {
                sensors[i].sample_rate /= 2;
            }
            
            // 降低CPU频率
            cpu_frequency_scale(0.5);
        }
        
        // 每5秒检查一次
        task_delay(5000);
    }
}

int main() {
    printf("启动多传感器数据采集系统\n");
    
    // 初始化各个模块
    scheduler_init();
    sensor_manager_init();
    data_processor_init();
    communication_init();
    
    // 配置传感器
    sensor_config_t sensors[] = {
        {1, 10, SENSOR_TEMPERATURE, true},  // 温度传感器，10Hz
        {2, 50, SENSOR_ACCELEROMETER, true}, // 加速度计，50Hz
        {3, 1,  SENSOR_PRESSURE, true},     // 气压传感器，1Hz
        {4, 5,  SENSOR_HUMIDITY, true}      // 湿度传感器，5Hz
    };
    
    // 创建传感器采集任务
    for (int i = 0; i < 4; i++) {
        task_t* sensor_task = task_create(
            sensor_acquisition_task, 
            &sensors[i], 
            PRIORITY_NORMAL, 
            1024
        );
        task_start(sensor_task);
    }
    
    // 创建数据处理任务
    task_t* processing_task = task_create(
        data_processing_task, 
        NULL, 
        PRIORITY_HIGH, 
        2048
    );
    task_start(processing_task);
    
    // 创建系统监控任务
    task_t* monitor_task = task_create(
        system_monitor_task, 
        NULL, 
        PRIORITY_LOW, 
        1024
    );
    task_start(monitor_task);
    
    printf("所有任务已启动\n");
    
    // 启动调度器
    scheduler_start();
    
    return 0;
}
```

## 实时通信系统

```c
#include "embedkit.h"
#include "communication/protocol.h"

// 通信协议栈
typedef struct {
    uint8_t header[4];      // 协议头
    uint16_t seq_num;       // 序列号
    uint8_t cmd;            // 命令类型
    uint8_t length;         // 数据长度
    uint8_t data[256];      // 数据载荷
    uint16_t checksum;      // 校验和
} protocol_packet_t;

// 接收任务
void uart_receive_task(void* arg) {
    uint8_t rx_buffer[512];
    protocol_packet_t packet;
    
    while (1) {
        // 接收数据
        int bytes = uart_receive(rx_buffer, sizeof(rx_buffer), 1000);
        
        if (bytes > 0) {
            // 解析协议包
            if (protocol_parse(rx_buffer, bytes, &packet) == 0) {
                // 验证校验和
                if (protocol_verify_checksum(&packet)) {
                    // 处理命令
                    protocol_handle_command(&packet);
                } else {
                    printf("校验和错误，请求重传\n");
                    protocol_send_nack(packet.seq_num);
                }
            }
        }
        
        task_yield();
    }
}

// 发送任务
void uart_transmit_task(void* arg) {
    protocol_packet_t packet;
    
    while (1) {
        // 从发送队列获取数据
        if (tx_queue_get(&packet, 100) == 0) {
            // 计算校验和
            packet.checksum = protocol_calc_checksum(&packet);
            
            // 发送数据包
            uart_send((uint8_t*)&packet, sizeof(packet));
            
            // 等待ACK
            if (protocol_wait_ack(packet.seq_num, 1000) != 0) {
                // 重传机制
                printf("ACK超时，重传数据包 %d\n", packet.seq_num);
                protocol_retransmit(&packet);
            }
        }
        
        task_yield();
    }
}

int main() {
    // 初始化通信系统
    scheduler_init();
    uart_init(115200);
    protocol_init();
    
    // 创建收发任务
    task_t* rx_task = task_create(uart_receive_task, NULL, PRIORITY_HIGH, 1024);
    task_t* tx_task = task_create(uart_transmit_task, NULL, PRIORITY_HIGH, 1024);
    
    task_start(rx_task);
    task_start(tx_task);
    
    scheduler_start();
    return 0;
}
```

## 编译和运行

### 编译命令
```bash
# 编译多传感器系统
gcc -I../../include -I../../src \
    multi_sensor_system.c \
    -lembedkit -lsensor -lcomm \
    -o multi_sensor_system

# 编译通信系统    
gcc -I../../include -I../../src \
    communication_system.c \
    -lembedkit -luart -lprotocol \
    -o communication_system
```

### 硬件要求
- ARM Cortex-M3/M4 处理器
- 32KB+ RAM
- 128KB+ Flash
- UART/SPI/I2C接口

### 性能指标
- CPU使用率: < 70%
- 内存使用: < 24KB
- 响应时间: < 10ms
- 数据吞吐: 1Mbps+