# L2_Core (核心与硬件抽象层)

## 1. 简介
L2_Core 是本项目的核心与硬件抽象层，提供基础功能模块和硬件抽象接口。

向下：依赖 L1_MCU 层的厂商库。

向上：为 L3~L5 层提供通用的数据结构、服务功能和硬件抽象接口。

本层包含两个子目录：
- **utils/**：与硬件无关的基础功能（数据结构、内存管理、日志等）
- **hal/**：硬件抽象层（GPIO、UART、I2C 等）

## 2. 目录结构
```text
L2_Core
├── CMakeLists.txt          # 构建脚本
├── utils/                  # 工具库（硬件无关）
│   ├── inc                 # 对外接口
│   │   ├── ek_def.h       # 通用定义
│   │   ├── ek_list.h      # 双向循环链表
│   │   ├── ek_ringbuf.h   # 环形缓冲区
│   │   ├── ek_stack.h     # 栈数据结构
│   │   ├── ek_mem.h       # 动态内存管理
│   │   ├── ek_log.h       # 日志系统
│   │   └── ek_io.h        # IO 抽象
│   └── src                 # 内部实现
│       ├── ek_ringbuf.c
│       ├── ek_stack.c
│       └── ek_mem.c
├── hal/                    # 硬件抽象层
│   ├── inc                 # HAL 接口（不含厂商头文件）
│   │   ├── hal_gpio.h
│   │   ├── hal_uart.h
│   │   └── hal_i2c.h
│   └── src                 # HAL 实现（包含厂商头文件）
│       ├── hal_gpio.c
│       ├── hal_uart.c
│       └── hal_i2c.c
└── third_party/            # 第三方库
    ├── lwprintf/           # 轻量级 printf
    └── tlsf/               # 内存分配器
```

## 3. 核心开发原则 (Strict Rules)
为了保证架构的整洁性和可移植性，开发本层时必须遵守以下规则：

**utils/ 子目录（硬件无关）**：

禁止包含任何与硬件相关的头文件（如 stm32xxxx.h、gpio.h 等）。

禁止直接操作寄存器或调用硬件相关函数。

只使用标准 C 语言库（stdint.h、stdbool.h、string.h 等）。

**hal/ 子目录（硬件抽象）**：

`inc/*.h` 绝对不能包含厂商头文件（stm32xxxx.h）。

厂商头文件仅允许在 `src/*.c` 中包含。

提供逻辑到物理的映射（如 `HAL_GPIO_1` → `GPIOA PIN_5`）。

所有函数使用 `hal_` 前缀（如 `hal_gpio_write()`）。

**内部依赖关系**：

`hal/` 可以依赖 `utils/`（同层内部允许依赖）。

`utils/` 不能依赖 `hal/`（保持纯软件实现）。

## 4. 开发指南

### 添加 utils 模块（硬件无关）

#### 步骤 1：定义接口 (utils/inc/ek_xxx.h)
定义模块对外的 API，保持接口简洁清晰。

```c
// utils/inc/ek_example.h
#ifndef EK_EXAMPLE_H
#define EK_EXAMPLE_H

#include <stdint.h>
#include <stdbool.h>

// API 函数
void ek_example_init(void);
int ek_example_do_something(int value);

#endif
```

#### 步骤 2：实现接口 (utils/src/ek_xxx.c)
使用纯 C 语言实现功能逻辑。

```c
// utils/src/ek_example.c
#include "ek_example.h"
#include <string.h>

void ek_example_init(void)
{
    // 初始化逻辑
}

int ek_example_do_something(int value)
{
    return value * 2;
}
```

### 添加 hal 模块（硬件抽象）

#### 步骤 1：定义接口 (hal/inc/hal_xxx.h)
**重要：头文件中不能包含厂商头文件！**

```c
// hal/inc/hal_gpio.h
#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>

// 逻辑 GPIO ID
typedef enum {
    HAL_GPIO_0 = 0,
    HAL_GPIO_1,
    // ...
} hal_gpio_t;

// API 函数
void hal_gpio_init(hal_gpio_t gpio);
void hal_gpio_write(hal_gpio_t gpio, bool state);
bool hal_gpio_read(hal_gpio_t gpio);

#endif
```

#### 步骤 2：实现接口 (hal/src/hal_xxx.c)
在此处包含厂商头文件，实现逻辑到物理的映射。

```c
// hal/src/hal_gpio.c
#include "hal_gpio.h"
#include "stm32f4xx_hal.h"  // 厂商头文件

// 逻辑到物理的映射表
static GPIO_TypeDef* const gpio_ports[] = {GPIOA, GPIOB, GPIOC, ...};
static const uint16_t gpio_pins[] = {GPIO_PIN_0, GPIO_PIN_1, ...};

void hal_gpio_write(hal_gpio_t gpio, bool state)
{
    GPIO_TypeDef* port = gpio_ports[gpio >> 4];
    uint16_t pin = gpio_pins[gpio & 0xF];

    HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
```

### 使用日志系统

本层提供了 `ek_log` 模块，支持分级日志输出。

```c
#include "ek_log.h"

void example_function(void)
{
    // 不同级别的日志
    ek_log_debug("调试信息: x=%d", 42);
    ek_log_info("普通信息: 系统启动");
    ek_log_warn("警告信息: 内存不足");
    ek_log_error("错误信息: 打开失败");
}
```

### 使用 lwprintf

本层集成了 `lwprintf` 轻量级格式化输出。

```c
#include "ek_io.h"  // 已包含 lwprintf.h

// 自定义输出函数
static void uart_output(const char *str, uint32_t len)
{
    // 发送到串口（通过 L2_Core/hal 或上层实现）
}

void example_printf(void)
{
    // 注册输出函数
    lwprintf_set_output(uart_output);

    // 使用 lwprintf 输出
    lwprintf("温度: %d.%d C\n", 25, 5);
}
```

## 5. 常见问题

**Q: hal/ 子目录可以直接使用 utils/ 的数据结构吗？**

A: 可以！这是新架构的优势。hal/ 可以使用 utils/ 提供的链表、环形缓冲区等数据结构来管理设备，因为它们属于同一层。

**Q: 为什么 hal 的头文件不能包含厂商头文件？**

A: 保持头文件的纯净性，避免泄露硬件细节到上层。上层应用只看到 `hal_gpio_t` 这样的逻辑 ID，而不需要知道底层是哪个 GPIO 端口。

**Q: 动态内存管理为什么要自己实现？**

A: 嵌入式系统中标准库的 malloc 有很多问题：碎片化、不确定性、占用内存大。本层使用 TLSF（Two-Level Segregated Fit）内存分配器，更适合嵌入式场景。
