# L1_HAL (Hardware Abstraction Layer)
## 1. 简介
L1_HAL 是本项目的硬件抽象层。它的核心使命是承上启下：

向下：直接调用 L0_MCU 提供的厂商库（如 STM32 HAL, LL 库）或直接操作寄存器。

向上：向 L2~L5 层提供一套统一、标准、与具体芯片无关的 C 接口。

通过本层，上层业务代码（App）将不再依赖具体的芯片型号。更换 MCU 时，只需重写本层实现，上层代码无需改动。

## 2. 目录结构
```text
L1_HAL
├── CMakeLists.txt      # 构建脚本
├── inc                 # [对外接口] 纯净的头文件，严禁包含厂商头文件
│   ├── hal_gpio.h
│   ├── hal_uart.h
│   └── hal_def.h       # 通用类型定义
└── src                 # [内部实现] 具体的驱动实现，包含厂商头文件
    ├── hal_gpio.c
    └── hal_uart.c
````
## 3. 核心开发原则 (Strict Rules)
为了保证架构的整洁性，开发本层时必须遵守以下规则：

头文件隔离 (Header Isolation)：

禁止 在 inc/*.h 中包含 stm32xxxx.h 或厂商特定的头文件。

厂商头文件只能出现在 src/*.c 中。

如果需要传递句柄或结构体，请使用 void * 指针或自定义的结构体进行封装。

接口标准化 (Standard API)：

函数命名统一使用小写加下划线，例如：hal_driver_action()。

示例：hal_gpio_write(...), hal_uart_send(...)。

无状态推荐 (Stateless)：

尽量设计成简单的透传函数。复杂的状态机逻辑（如协议解析）应放在 L3_Middlewares 或 L4_Components 中。

##4. 开发指南
步骤 1：定义接口 (inc/hal_xxx.h)
定义上层能看到的 API。

```c

// inc/hal_led.h
#ifndef HAL_LED_H
#define HAL_LED_H

#include <stdint.h>
#include <stdbool.h>

// 定义逻辑 ID，不要使用 GPIOA, PIN5 这种物理名称
typedef enum
{
    HAL_LED_1 = 0,
    HAL_LED_2,
    HAL_LED_ERR
} hal_led_t;

void hal_led_init(void);
void hal_led_set(hal_led_t led, bool state);
void hal_led_toggle(hal_led_t led);

#endif
```
步骤 2：实现接口 (src/hal_xxx.c)
在这里引入 L0 层，将通用请求转换为具体硬件操作。

```c

// src/hal_led.c
#include "hal_led.h"
#include "main.h" // 引用 L0 层的厂商定义 (如 GPIOA)

void hal_led_set(hal_led_t led, bool state)
{
    GPIO_TypeDef* port;
    uint16_t pin;

    // 简单的映射表逻辑
    switch(led)
    {
        case HAL_LED_1: port = GPIOA; pin = GPIO_PIN_5; break;
        case HAL_LED_2: port = GPIOB; pin = GPIO_PIN_13; break;
        default: return;
    }

    HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
````
## 5. 常见问题
**Q: 为什么不能在 L1 的头文件里写 include "stm32f4xx_hal.h"?**
A: 如果你这样做了，所有引用 L1 的上层文件（L2, L5）都会间接引用 STM32 的库。一旦将来移植到 GD32 或 ESP32，你需要修改所有上层文件的包含关系，架构解耦就失败了。

**Q: L1 应该包含复杂的逻辑吗?**
A: 不应该。L1 只负责“翻译”和“搬运”数据。比如 UART 驱动，L1 只负责把字节塞进寄存器。至于数据的解析、分包、重组，应该由 L2 或 L3 层处理。
