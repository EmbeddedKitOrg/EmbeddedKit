# EmbeddedKit (EK)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/username/embeddedkit) [![Platform](https://img.shields.io/badge/platform-STM32-green.svg)](https://github.com/username/embeddedkit)

**EmbeddedKit** 是一个现代化的嵌入式组件库，旨在为STM32嵌入式开发提供高质量、可复用的模块化组件。通过统一的API设计和严格的编码规范，EmbeddedKit让嵌入式开发更加高效和可维护。

## ✨ 特性

- 🎯 **统一API设计** - 通过函数前缀明确标识返回类型，提高代码可读性
- 📦 **模块化架构** - 清晰的内部/外部API边界，便于维护和扩展
- 🔒 **类型安全** - 强类型定义和统一的错误处理机制
- 📖 **完善文档** - 详细的API文档和编码规范
- 🔧 **易于集成** - 直接包含到现有STM32项目中
- 🚀 **高性能** - 针对嵌入式系统优化的轻量级实现

## 🚧 项目状态

### 当前版本 (v1.0.x)

- ✅ **核心组件库** - 稳定发布，积极维护
- ✅ **编码规范** - 完整文档，正在使用

### 未来规划 (v2.0.x)

- 🔮 

  EK_HAL硬件抽象层

   \- 架构设计已完成，暂未实施

  > ⚠️ **注意**: EK_HAL目前仅为设计文档，不包含在当前发布版本中。这是我们未来发展的重要方向，将在v2.0版本中正式推出。

## 🚀 快速开始

### 平台支持

- **STM32系列**: F1/F4/F7/H7系列
- **国产32位**: 基于ARM Cortex-M的国产芯片（如GD32、MM32等）

### 集成到项目

1. **添加源文件到项目** 将EmbeddedKit的`Inc/`和`Src/`文件夹添加到你的STM32项目中

2. **配置包含路径** 在IDE中添加EmbeddedKit的头文件路径：

   ```
   ProjectRoot/EmbeddedKit/Inc/
   ```

3. **包含头文件**

   ```c
   #include "EK_Common.h"
   #include "EK_Gpio.h"
   #include "EK_Timer.h"
   ```

4. **配置EK_Config.h** 根据你的项目需求修改配置参数

### 基础使用示例

```c
#include "EK_Led.h"

int main(void) 
{
    // 系统初始化
    SystemClock_Config();
    
    // 初始化LED模块
    if (EK_rInitLed() != EK_OK) {
        // 错误处理
        Error_Handler();
    }
    
    // 添加LED配置 (PC13引脚，高电平有效)
    EK_rAddLed(0, 13, true);
    
    // 主循环 - LED闪烁
    while (1) {
        EK_rToggleLed(0);       // 翻转LED状态
        HAL_Delay(500);         // 延时500ms
    }
}
```

## 📁 项目结构

```
EmbeddedKit/
├── Inc/                    # 头文件 (添加到项目包含路径)
│   ├── EK_Common.h        # 核心定义和错误码
│   ├── EK_Config.h        # 配置文件
│   ├── EK_Gpio.h          # GPIO模块
│   ├── EK_Timer.h         # 定时器模块
│   ├── EK_Uart.h          # UART模块
│   └── EK_Led.h           # LED控制模块
├── Src/                    # 源文件 (添加到项目编译)
│   ├── EK_Gpio.c
│   ├── EK_Timer.c
│   ├── EK_Uart.c
│   ├── EK_Led.c
│   └── EK_Config.c
├── Examples/               # 示例代码
│   ├── STM32F4_Examples/  # STM32F4示例
│   ├── STM32F1_Examples/  # STM32F1示例
│   └── GD32_Examples/     # 国产32示例
├── Docs/                   # 文档
│   ├── CodingStandards.md # 编码规范
│   ├── HAL_Architecture.md# HAL架构设计(未来)
│   └── API/               # API文档
└── Tools/                  # 配置工具和脚本
```

## 📋 API概览

### 核心模块

| 模块      | 功能               | 状态   |
| --------- | ------------------ | ------ |
| EK_Common | 公共定义和错误处理 | ✅ 稳定 |
| EK_Gpio   | GPIO控制和管理     | ✅ 稳定 |
| EK_Timer  | 定时器功能         | ✅ 稳定 |
| EK_Uart   | 串口通信           | ✅ 稳定 |
| EK_Led    | LED控制封装        | ✅ 稳定 |

### 函数命名规范

所有函数使用类型前缀标识返回值：

| 前缀 | 返回类型  | 示例                   |
| ---- | --------- | ---------------------- |
| `v`  | void      | `EK_vInitSystem()`     |
| `r`  | EK_Result | `EK_rConfigureTimer()` |
| `b`  | bool      | `EK_bIsSystemReady()`  |
| `u`  | unsigned  | `EK_uGetSystemClock()` |
| `i`  | int       | `EK_iGetSensorValue()` |
| `p`  | pointer   | `EK_pGetBuffer()`      |

## 🔧 配置

### 基本配置

在`EK_Config.h`中配置基本参数：

```c
// 系统配置
#define EK_SYSTEM_CLOCK_HZ      48000000    // 根据你的系统时钟修改
#define EK_MAX_MODULES          10

// 缓冲区配置
#define EK_MAX_BUFFER_SIZE      1024
#define EK_DEFAULT_TIMEOUT      5000

// 模块使能
#define EK_USE_GPIO_MODULE      1
#define EK_USE_TIMER_MODULE     1
#define EK_USE_UART_MODULE      1
#define EK_USE_LED_MODULE       1

// 调试配置
#define EK_DEBUG_ENABLE         1
#define EK_LOG_LEVEL            EK_LOG_INFO
```



## 💡 使用示例

### GPIO控制

```c
#include "EK_Gpio.h"

// 初始化GPIO
EK_rInitGpio();

// 配置输出引脚
EK_rConfigGpioOutput(GPIOC, GPIO_PIN_13);

// 控制GPIO
EK_vSetGpioHigh(GPIOC, GPIO_PIN_13);
EK_vSetGpioLow(GPIOC, GPIO_PIN_13);
```

### 定时器使用

```c
#include "EK_Timer.h"

// 初始化定时器
EK_rInitTimer();

// 配置1ms定时器
EK_rConfigTimer(TIM2, 1000, timer_callback);

// 启动定时器
EK_rStartTimer(TIM2);
```

### UART通信

```c
#include "EK_Uart.h"

// 初始化UART
EK_rInitUart(USART1, 115200);

// 发送数据
uint8_t data[] = "Hello EmbeddedKit!";
EK_rUartSend(USART1, data, sizeof(data));

// 接收数据
uint8_t recv_buf[64];
size_t recv_len = EK_sUartReceive(USART1, recv_buf, sizeof(recv_buf));
```

## 📖 文档

- 编码规范 - 详细的代码风格和命名规范
- API参考 - 完整的API文档
- EK_HAL架构设计 - 硬件抽象层设计(未来版本)
- 移植指南 - 新平台移植说明

## 🤝 贡献

我们欢迎社区贡献！请参考以下步骤：

1. Fork 本仓库
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 遵循编码规范
4. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
5. 推送分支 (`git push origin feature/AmazingFeature`)
6. 创建 Pull Request

### 贡献指南

- 🔍 **代码质量**: 所有代码必须遵循EK编码规范
- 📝 **文档**: 新功能需要包含相应的文档更新
- 🏷️ **命名规范**: 严格遵循EK命名约定
- ✅ **测试**: 在目标硬件上验证功能正确性

## 👥 团队成员

- **[@zuolan](https://github.com/zuoliangyu)** - 项目维护者
- **[@N1netyNine99](https://github.com/00lllooolll00)** - 核心开发者

## 📄 许可证

本项目采用 MIT 许可证 - 查看 LICENSE 文件了解详情。

## 📞 支持

- 🐛 **问题反馈**: [GitHub Issues](https://github.com/username/embeddedkit/issues)

## ⭐ Star History

<div align="center">

**文档** | **示例** | **贡献指南**

Made with ❤️ for STM32 Developers

</div>