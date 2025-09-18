# EK_Queue - 循环缓冲区队列

## 概述

EK_Queue 是一个基于循环缓冲区实现的FIFO（先进先出）队列数据结构，专为嵌入式系统设计。支持字节流方式存储任意类型数据，提供高效的入队、出队操作，并具备完善的状态检查功能。

## 核心特性

- **循环缓冲区**：基于环形缓冲区实现，内存利用率高
- **字节流存储**：支持任意数据类型，灵活性强
- **双内存模式**：支持静态和动态内存分配
- **状态监控**：提供队列满、空、大小等状态查询
- **数据预览**：支持查看队头数据而不移除
- **空间管理**：自动处理缓冲区空间计算和环形索引
- **跨界处理**：智能处理数据跨越缓冲区边界的情况

## 数据结构

### EK_Queue_t - 队列结构体
```c
typedef struct EK_Queue_t
{
    void *Queue_Buf;              // 队列缓冲区指针
    index_t Queue_Front;          // 队头索引（待出队元素）
    index_t Queue_Rear;           // 队尾索引（下一个入队位置）
    EK_Size_t Queue_Size;            // 当前元素个数（字节数）
    EK_Size_t Queue_Capacity;        // 队列容量（字节数）
    bool Queue_isDynamic;         // 动态创建标志
} EK_Queue_t;
```

### 关键字段说明
- **Queue_Front**：指向第一个待出队元素的索引
- **Queue_Rear**：指向下一个入队位置的索引  
- **Queue_Size**：当前存储的数据字节数
- **Queue_Capacity**：缓冲区总容量
- **Queue_isDynamic**：标识内存分配方式，影响销毁时的内存释放

## API 接口

### 队列创建

#### 动态队列创建
```c
EK_Queue_t *EK_pQueueCreate_Dynamic(EK_Size_t capacity);
```
- **功能**：动态分配内存创建队列
- **参数**：`capacity` - 队列容量（字节数）
- **返回值**：队列指针，失败返回NULL
- **内存管理**：队列结构体和缓冲区都由malloc分配

#### 静态队列创建
```c
EK_Result_t EK_rQueueCreate_Static(EK_Queue_t *queue_handler, void *buffer, const EK_Size_t capacity);
```
- **功能**：使用用户提供的内存初始化队列
- **参数**：
  - `queue_handler`：用户分配的队列结构体
  - `buffer`：用户分配的缓冲区
  - `capacity`：缓冲区大小
- **返回值**：操作结果状态码

### 队列管理

#### 队列清空
```c
EK_Result_t EK_rQueueClean(EK_Queue_t *queue);
```
- **功能**：清空队列中的所有数据
- **特性**：重置队列状态为空，不清零缓冲区内容以提高性能
- **返回值**：操作结果状态码

#### 队列销毁
```c
EK_Result_t EK_rQueueDelete(EK_Queue_t *queue);
```
- **功能**：销毁队列并释放资源
- **参数**：`queue` - 要删除的队列指针
- **返回值**：删除成功返回EK_OK，失败返回对应错误码
- **动态队列**：释放malloc分配的内存（包括缓冲区和队列结构体）
- **静态队列**：清空缓冲区，重置队列状态指针
- **注意**：删除后队列指针将变为无效，不应再使用

### 状态查询

#### 空队列检查
```c
bool EK_bQueueIsEmpty(EK_Queue_t *queue);
```
- **功能**：检查队列是否为空
- **返回值**：空队列返回true

#### 满队列检查
```c
bool EK_bQueueIsFull(EK_Queue_t *queue);
```
- **功能**：检查队列是否已满
- **返回值**：满队列返回true

#### 获取队列大小
```c
EK_Size_t EK_sQueueGetSize(EK_Queue_t *queue);
```
- **功能**：获取当前存储的数据量
- **返回值**：当前数据字节数

#### 获取剩余空间
```c
EK_Size_t EK_sQueueGetRemain(EK_Queue_t *queue);
```
- **功能**：获取剩余可用空间
- **返回值**：剩余字节数

### 数据操作

#### 入队操作
```c
EK_Result_t EK_rQueueEnqueue(EK_Queue_t *queue, void *data, EK_Size_t data_size);
```
- **功能**：向队列尾部添加数据
- **参数**：
  - `queue`：队列指针
  - `data`：要添加的数据指针
  - `data_size`：数据大小（字节）
- **特性**：
  - 自动检查剩余空间
  - 处理环形缓冲区索引
  - 智能处理跨界数据复制
  - 数据完整性保证

#### 出队操作
```c
EK_Result_t EK_rQueueDequeue(EK_Queue_t *queue, void *data_buffer, EK_Size_t data_size);
```
- **功能**：从队列头部取出数据
- **参数**：
  - `queue`：队列指针
  - `data_buffer`：接收数据的缓冲区
  - `data_size`：要取出的数据大小
- **特性**：
  - 数据从队列中完全移除
  - 自动处理环形索引更新
  - 智能处理跨界数据读取

#### 数据预览
```c
EK_Result_t EK_rQueuePeekFront(EK_Queue_t *queue, void *data_buffer, EK_Size_t data_size);
```
- **功能**：查看队头数据但不移除
- **用途**：数据预处理、协议解析等场景
- **特性**：队列状态保持不变

## 使用场景与示例

### 1. 串口数据缓冲
```c
// 创建串口接收队列
EK_Queue_t *uart_rx_queue = EK_pQueueCreate_Dynamic(1024);

// 中断接收数据
void UART_IRQ_Handler(void) {
    uint8_t byte = UART_ReadByte();
    EK_rQueueEnqueue(uart_rx_queue, &byte, 1);
}

// 主循环处理数据
void uart_process_task(void) {
    uint8_t buffer[64];
    if (!EK_bQueueIsEmpty(uart_rx_queue)) {
        EK_Size_t available = EK_sQueueGetSize(uart_rx_queue);
        EK_Size_t read_size = (available > 64) ? 64 : available;
        EK_rQueueDequeue(uart_rx_queue, buffer, read_size);
        // 处理接收到的数据
        process_uart_data(buffer, read_size);
    }
}
```

### 2. 协议数据包处理
```c
// 数据包结构
typedef struct {
    uint16_t length;
    uint8_t data[256];
} Packet_t;

EK_Queue_t *packet_queue = EK_pQueueCreate_Dynamic(4096);

// 接收数据包
void receive_packet(Packet_t *packet) {
    EK_rQueueEnqueue(packet_queue, packet, sizeof(Packet_t));
}

// 预览数据包头
void check_next_packet(void) {
    if (EK_sQueueGetSize(packet_queue) >= sizeof(uint16_t)) {
        uint16_t length;
        EK_rQueuePeekFront(packet_queue, &length, sizeof(length));
        printf("Next packet length: %d\n", length);
    }
}

// 处理完整数据包
void process_packets(void) {
    while (EK_sQueueGetSize(packet_queue) >= sizeof(uint16_t)) {
        uint16_t length;
        EK_rQueuePeekFront(packet_queue, &length, sizeof(length));
        
        // 检查是否有完整数据包
        if (EK_sQueueGetSize(packet_queue) >= sizeof(Packet_t)) {
            Packet_t packet;
            EK_rQueueDequeue(packet_queue, &packet, sizeof(packet));
            handle_packet(&packet);
        } else {
            break; // 等待更多数据
        }
    }
}
```

### 3. 音频数据流处理
```c
#define AUDIO_BUFFER_SIZE 8192
static uint8_t audio_buffer[AUDIO_BUFFER_SIZE];
EK_Queue_t audio_queue;

// 静态创建音频队列
void audio_init(void) {
    EK_rQueueCreate_Static(&audio_queue, audio_buffer, AUDIO_BUFFER_SIZE);
}

// DMA 中断填充数据
void Audio_DMA_Handler(void) {
    int16_t samples[128];
    audio_capture(samples, 128);
    
    // 检查空间是否足够
    if (EK_sQueueGetRemain(&audio_queue) >= sizeof(samples)) {
        EK_rQueueEnqueue(&audio_queue, samples, sizeof(samples));
    } else {
        // 缓冲区满，丢弃旧数据
        int16_t temp[128];
        EK_rQueueDequeue(&audio_queue, temp, sizeof(temp));
        EK_rQueueEnqueue(&audio_queue, samples, sizeof(samples));
    }
}

// 播放任务
void audio_play_task(void) {
    if (EK_sQueueGetSize(&audio_queue) >= 256) { // 确保有足够数据
        int16_t play_buffer[128];
        EK_rQueueDequeue(&audio_queue, play_buffer, sizeof(play_buffer));
        audio_play(play_buffer, 128);
    }
}
```

### 4. 命令队列系统
```c
typedef struct {
    uint8_t cmd_id;
    uint32_t param;
    void (*callback)(uint32_t);
} Command_t;

EK_Queue_t *cmd_queue = EK_pQueueCreate_Dynamic(sizeof(Command_t) * 16);

// 添加命令
void add_command(uint8_t id, uint32_t param, void (*cb)(uint32_t)) {
    Command_t cmd = {.cmd_id = id, .param = param, .callback = cb};
    if (EK_rQueueEnqueue(cmd_queue, &cmd, sizeof(cmd)) != EK_OK) {
        printf("Command queue full!\n");
    }
}

// 执行命令
void execute_commands(void) {
    while (!EK_bQueueIsEmpty(cmd_queue)) {
        Command_t cmd;
        if (EK_rQueueDequeue(cmd_queue, &cmd, sizeof(cmd)) == EK_OK) {
            if (cmd.callback) {
                cmd.callback(cmd.param);
            }
        }
    }
}
```

### 5. 数据流量控制
```c
// 网络数据发送队列
EK_Queue_t *tx_queue = EK_pQueueCreate_Dynamic(4096);

void network_send_data(uint8_t *data, EK_Size_t len) {
    // 检查队列剩余空间
    if (EK_sQueueGetRemain(tx_queue) < len) {
        printf("Send buffer full, applying flow control\n");
        return; // 或等待空间释放
    }
    
    EK_rQueueEnqueue(tx_queue, data, len);
}

void network_tx_task(void) {
    if (!EK_bQueueIsEmpty(tx_queue)) {
        uint8_t tx_buffer[128];
        EK_Size_t send_size = EK_sQueueGetSize(tx_queue);
        if (send_size > 128) send_size = 128;
        
        EK_rQueueDequeue(tx_queue, tx_buffer, send_size);
        network_transmit(tx_buffer, send_size);
    }
}
```

### 6. 传感器数据采集
```c
typedef struct {
    uint32_t timestamp;
    float temperature;
    float humidity;
    uint16_t pressure;
} SensorReading_t;

static uint8_t sensor_buffer[1024];
EK_Queue_t sensor_queue;

void sensor_init(void) {
    EK_rQueueCreate_Static(&sensor_queue, sensor_buffer, sizeof(sensor_buffer));
}

// 定时采集传感器数据
void sensor_sample_task(void) {
    SensorReading_t reading;
    reading.timestamp = HAL_GetTick();
    reading.temperature = read_temperature();
    reading.humidity = read_humidity();
    reading.pressure = read_pressure();
    
    // 如果队列满了，丢弃最老的数据
    if (EK_sQueueGetRemain(&sensor_queue) < sizeof(reading)) {
        SensorReading_t old_reading;
        EK_rQueueDequeue(&sensor_queue, &old_reading, sizeof(old_reading));
    }
    
    EK_rQueueEnqueue(&sensor_queue, &reading, sizeof(reading));
}

// 数据处理任务
void sensor_process_task(void) {
    while (!EK_bQueueIsEmpty(&sensor_queue)) {
        SensorReading_t reading;
        if (EK_rQueueDequeue(&sensor_queue, &reading, sizeof(reading)) == EK_OK) {
            // 处理传感器数据
            process_sensor_data(&reading);
        }
    }
}
```

### 7. 环形日志缓冲区
```c
#define LOG_BUFFER_SIZE 2048
static char log_buffer[LOG_BUFFER_SIZE];
EK_Queue_t log_queue;

void log_init(void) {
    EK_rQueueCreate_Static(&log_queue, log_buffer, LOG_BUFFER_SIZE);
}

void log_message(const char *message) {
    EK_Size_t msg_len = strlen(message);
    
    // 如果消息太长，截断
    if (msg_len > LOG_BUFFER_SIZE / 2) {
        msg_len = LOG_BUFFER_SIZE / 2;
    }
    
    // 确保有足够空间，必要时丢弃旧日志
    while (EK_sQueueGetRemain(&log_queue) < msg_len) {
        char temp[64];
        EK_Size_t discard_size = (EK_sQueueGetSize(&log_queue) > 64) ? 64 : EK_sQueueGetSize(&log_queue);
        EK_rQueueDequeue(&log_queue, temp, discard_size);
    }
    
    EK_rQueueEnqueue(&log_queue, (void*)message, msg_len);
}

void log_dump(void) {
    char buffer[256];
    while (!EK_bQueueIsEmpty(&log_queue)) {
        EK_Size_t read_size = EK_sQueueGetSize(&log_queue);
        if (read_size > sizeof(buffer)) read_size = sizeof(buffer);
        
        EK_rQueueDequeue(&log_queue, buffer, read_size);
        // 输出日志数据
        uart_send(buffer, read_size);
    }
}
```

## 设计优势

### 1. 环形缓冲区优化
- **无内存碎片**：固定大小的连续内存空间
- **高效索引**：通过模运算实现环形访问
- **空间复用**：出队的空间可立即被入队操作重用

### 2. 字节流设计
- **类型无关**：可存储任意数据类型
- **灵活分割**：支持任意大小的数据块
- **协议解析友好**：便于处理变长数据

### 3. 双模式内存管理
- **静态分配**：编译时确定，无运行时开销
- **动态分配**：运行时灵活创建，便于资源管理
- **智能标识**：自动识别内存分配方式，确保正确释放

### 4. 完善的状态管理
- **边界检查**：防止缓冲区溢出
- **状态查询**：便于流量控制和调度决策
- **数据预览**：支持非破坏性数据检查

### 5. 跨界数据处理
- **智能分段**：自动处理跨越缓冲区边界的数据
- **完整性保证**：确保数据读写的原子性
- **性能优化**：减少不必要的内存操作

## 性能特点

- **入队操作**：O(1) 时间复杂度
- **出队操作**：O(1) 时间复杂度
- **状态查询**：O(1) 时间复杂度
- **内存开销**：队列结构体约24字节 + 用户缓冲区
- **线程安全**：需要外部同步保护

## 算法复杂度总结

| 操作类型 | 时间复杂度 | 空间复杂度 | 说明 |
|---------|-----------|-----------|------|
| 创建队列 | O(1) | O(n) | n为队列容量 |
| 销毁队列 | O(1) | O(1) | 释放内存 |
| 入队操作 | O(1) | O(1) | 环形缓冲区优化 |
| 出队操作 | O(1) | O(1) | 直接索引访问 |
| 数据预览 | O(1) | O(1) | 不改变队列状态 |
| 状态查询 | O(1) | O(1) | 直接字段访问 |
| 清空队列 | O(1) | O(1) | 重置指针 |

## 注意事项

### 使用限制
1. **容量规划**：根据数据流量和处理速度合理设置容量
2. **数据对齐**：注意结构体数据的内存对齐问题
3. **错误处理**：检查API返回值，处理队列满/空等异常情况
4. **内存管理**：动态队列记得在不需要时调用删除函数
5. **并发访问**：多线程环境需要适当的同步机制

### 性能优化建议
1. **批量操作**：尽量批量处理数据，减少函数调用开销
2. **容量设置**：根据实际需求设置合适的队列容量
3. **内存对齐**：确保数据结构内存对齐以提高访问效率
4. **预分配**：静态模式下预分配足够的缓冲区空间

### 错误处理
1. **参数验证**：所有API都进行完善的参数有效性检查
2. **边界检查**：防止缓冲区溢出和下溢
3. **状态一致性**：确保队列在异常情况下保持一致状态
4. **内存安全**：动态分配失败时的安全处理

该队列模块为嵌入式系统提供了高效、可靠的数据缓冲解决方案，特别适用于数据流处理、协议解析、任务间通信等场景。通过环形缓冲区设计和智能的跨界处理机制，能够在有限的内存资源下实现高效的数据管理。

## 版本更新

### v1.0 (2025-09-08)
- ✅ 基础循环队列功能
- ✅ 静态/动态内存管理
- ✅ 字节流数据存储
- ✅ 跨界数据处理
- ✅ 完善的状态查询
- ✅ 数据预览功能
- ✅ 队列清空和销毁