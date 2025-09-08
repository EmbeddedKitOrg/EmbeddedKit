# EmbedKit 代码审查检查清单

## 🎯 审查总则
- 专注于代码质量和嵌入式特性
- 关注架构设计和可维护性
- 确保性能和资源使用合理
- 验证硬件兼容性和安全性

## 📐 架构设计审查

### ✅ 代码结构
- [ ] **文件行数**: C文件不超过250行
- [ ] **文件夹结构**: 每个文件夹不超过8个文件
- [ ] **模块职责**: 单一职责原则，功能边界清晰
- [ ] **依赖关系**: 避免循环依赖，依赖方向合理

### ✅ 接口设计
- [ ] **API一致性**: 命名风格统一，参数顺序合理
- [ ] **错误处理**: 返回值明确，错误代码定义清晰
- [ ] **可扩展性**: 预留扩展点，版本兼容性考虑
- [ ] **文档完整性**: API注释完整，使用示例提供

### ✅ 设计模式应用
- [ ] **策略模式**: 算法可替换性
- [ ] **工厂模式**: 对象创建抽象
- [ ] **观察者模式**: 事件通知机制
- [ ] **状态机**: 复杂状态管理

## 💾 内存管理审查

### ✅ 内存分配
```c
// 检查项示例
void* ptr = malloc(size);
if (ptr == NULL) {
    return ERROR_NO_MEMORY;  // ✅ 分配失败检查
}
// ... 使用ptr
free(ptr);                   // ✅ 对应的释放
ptr = NULL;                  // ✅ 防止悬挂指针
```

### ✅ 栈使用管理
- [ ] **函数栈帧**: 局部变量大小合理（<1KB）
- [ ] **递归调用**: 避免深度递归或提供深度限制
- [ ] **栈监控**: 关键函数添加栈使用注释
- [ ] **编译检查**: 启用 `-Wstack-usage` 警告

### ✅ 缓冲区安全
```c
// 边界检查示例
void safe_copy(char* dest, const char* src, size_t dest_size) {
    if (dest == NULL || src == NULL || dest_size == 0) {
        return;  // ✅ 参数验证
    }
    
    size_t len = strlen(src);
    if (len >= dest_size) {
        len = dest_size - 1;  // ✅ 防止溢出
    }
    
    memcpy(dest, src, len);
    dest[len] = '\0';  // ✅ 确保字符串结束
}
```

## ⚡ 性能审查

### ✅ 算法复杂度
- [ ] **时间复杂度**: 关键路径算法复杂度合理
- [ ] **空间复杂度**: 辅助空间使用最小化
- [ ] **循环优化**: 避免不必要的嵌套循环
- [ ] **查找优化**: 频繁查找使用哈希表或二分查找

### ✅ 内存访问模式
```c
// 缓存友好的访问模式
// ✅ 好的访问模式 - 顺序访问
for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
        matrix[i][j] = process(data[i * cols + j]);
    }
}

// ❌ 不好的访问模式 - 跳跃访问
for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows; i++) {
        matrix[i][j] = process(data[i * cols + j]);
    }
}
```

### ✅ 编译器优化
- [ ] **内联函数**: 小函数适当使用inline关键字
- [ ] **常量折叠**: 编译时计算的表达式使用const
- [ ] **循环展开**: 关键循环考虑手动展开
- [ ] **分支预测**: 频繁分支使用likely/unlikely宏

## 🔌 硬件兼容性审查

### ✅ 数据类型和对齐
```c
// 平台无关的数据类型
#include <stdint.h>

typedef struct {
    uint8_t  status;      // 1字节
    uint8_t  padding[3];  // ✅ 显式填充，确保对齐
    uint32_t timestamp;   // 4字节，4字节对齐
    uint16_t data_len;    // 2字节
    uint8_t  data[];      // 变长数据
} __attribute__((packed)) protocol_header_t;  // ✅ 指定打包属性
```

### ✅ 字节序处理
```c
// 字节序转换
static inline uint32_t host_to_be32(uint32_t value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap32(value);  // ✅ 使用编译器内建函数
#else
    return value;
#endif
}
```

### ✅ 中断和并发
- [ ] **原子操作**: 共享数据使用原子操作保护
- [ ] **中断安全**: 中断处理函数简短高效
- [ ] **临界区**: 关中断区域最小化
- [ ] **死锁预防**: 锁的获取顺序一致

## 🧪 测试覆盖审查

### ✅ 单元测试
- [ ] **边界条件**: 测试参数边界值
- [ ] **错误路径**: 测试各种错误情况
- [ ] **资源限制**: 测试内存不足等情况
- [ ] **性能基准**: 关键函数性能测试

### ✅ 集成测试
- [ ] **模块协作**: 多模块协同工作测试
- [ ] **硬件集成**: 实际硬件平台验证
- [ ] **长时间运行**: 稳定性和内存泄漏测试
- [ ] **边界压力**: 系统负载极限测试

## 📝 代码质量审查

### ✅ 命名规范
```c
// ✅ 良好的命名示例
typedef enum {
    UART_STATUS_IDLE,
    UART_STATUS_TRANSMITTING,
    UART_STATUS_RECEIVING,
    UART_STATUS_ERROR
} uart_status_t;

static uart_status_t s_uart_status = UART_STATUS_IDLE;

int uart_send_data(const uint8_t* data, size_t len) {
    if (data == NULL || len == 0) {
        return -1;
    }
    // 实现...
}
```

### ✅ 注释质量
```c
/**
 * @brief 配置UART参数
 * 
 * @param[in] uart_id    UART设备ID (0-2)
 * @param[in] baudrate   波特率 (1200-3000000)
 * @param[in] config     配置参数
 * 
 * @return 0: 成功, <0: 错误码
 * 
 * @note 调用此函数前必须先初始化时钟
 * @warning 配置过程中会暂时禁用UART
 * 
 * @par 使用示例:
 * @code
 * uart_config_t config = {
 *     .data_bits = 8,
 *     .parity = UART_PARITY_NONE,
 *     .stop_bits = 1
 * };
 * int ret = uart_configure(0, 115200, &config);
 * @endcode
 */
```

### ✅ 错误处理
```c
// ✅ 完善的错误处理
typedef enum {
    ERROR_OK = 0,
    ERROR_INVALID_PARAM = -1,
    ERROR_NO_MEMORY = -2,
    ERROR_TIMEOUT = -3,
    ERROR_HARDWARE = -4
} error_code_t;

static const char* error_strings[] = {
    [0] = "Success",
    [-ERROR_INVALID_PARAM] = "Invalid parameter",
    [-ERROR_NO_MEMORY] = "Out of memory",
    [-ERROR_TIMEOUT] = "Operation timeout",
    [-ERROR_HARDWARE] = "Hardware error"
};
```

## 🔍 审查流程

### 1. 预审查准备
- [ ] 检查PR模板填写完整性
- [ ] 确认测试用例覆盖度
- [ ] 验证编译无警告
- [ ] 检查静态分析报告

### 2. 代码走查
- [ ] 逐行检查关键逻辑
- [ ] 验证算法正确性
- [ ] 检查资源管理
- [ ] 确认异常处理

### 3. 架构审查
- [ ] 评估设计合理性
- [ ] 检查模块边界
- [ ] 验证接口一致性
- [ ] 确认扩展性

### 4. 性能评估
- [ ] 分析时间复杂度
- [ ] 评估空间使用
- [ ] 检查关键路径
- [ ] 验证实时性需求

### 5. 审查反馈
- [ ] 明确指出问题
- [ ] 提供改进建议
- [ ] 讨论替代方案
- [ ] 确认修改计划

## 📋 审查报告模板

```markdown
## 代码审查报告

### 基本信息
- PR编号: #123
- 审查者: @reviewer
- 审查时间: 2024-01-15
- 代码变更: +150 -50 lines

### 审查结果
- [ ] ✅ 通过 - 可以合并
- [ ] ⚠️  有条件通过 - 需要小修改
- [ ] ❌ 不通过 - 需要重大修改

### 发现的问题
1. **内存管理**: line 45, malloc未检查返回值
2. **性能优化**: line 78-85, 可优化循环结构
3. **命名规范**: 函数名不符合项目约定

### 建议改进
1. 添加内存分配失败检查
2. 使用更高效的算法实现
3. 统一命名风格

### 总体评价
代码功能正确，但在内存安全和性能方面需要改进。
建议修改后重新审查。
```

## 🎯 审查重点提醒
- **安全第一**: 内存安全、缓冲区溢出、并发安全
- **性能关键**: 时间复杂度、空间使用、实时性保证  
- **可维护性**: 代码清晰、文档完整、测试充分
- **硬件适配**: 平台兼容、资源限制、硬件特性