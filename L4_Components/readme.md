# L4_Components (硬件驱动组件层)

## 1. 简介
L4_Components 是本项目的硬件驱动组件层，基于 L1_HAL 实现各种外设的驱动程序。

向下：调用 L1_HAL 提供的统一硬件接口（如 hal_gpio_write、hal_uart_send）。

向上：为 L5_App 层提供完整的设备驱动功能（如 OLED 显示、Flash 存储、传感器采集等）。

本层的核心设计理念是**OOP 面向对象编程**，通过结构体和函数指针模拟类和虚函数，实现接口与实现分离、依赖倒置等设计模式。

## 2. 目录结构
```text
L4_Components
├── CMakeLists.txt          # 构建脚本
├── inc                     # [对外接口] 组件头文件
│   ├── dev_oled.h         # OLED 显示驱动
│   ├── dev_flash.h        # Flash 存储驱动
│   ├── dev_uart_ring.h    # UART 环形缓冲驱动
│   └── dev_adc.h          # ADC 采集驱动
└── src                     # [内部实现] 组件源文件
    ├── dev_oled.c
    ├── dev_flash.c
    ├── dev_uart_ring.c
    └── dev_adc.c
```

## 3. 核心开发原则 (Strict Rules)
为了保证代码的可维护性和可扩展性，开发本层时必须遵守以下规则：

**OOP 编程风格 (Object-Oriented Programming)**：

使用结构体模拟"类"（Class），包含属性和方法。

使用函数指针模拟"虚函数"（Virtual Function），实现多态。

每个组件方法的第一个参数必须是对象指针（self/this）。

**接口与实现分离**：

定义抽象的接口层（如 IO 接口），与具体硬件实现解耦。

组件不直接依赖 L1_HAL 的具体函数，而是通过接口调用。

**依赖倒置原则 (Dependency Inversion)**：

高层模块不应依赖低层模块，两者都应依赖抽象。

抽象不应依赖细节，细节应依赖抽象。

## 4. 开发指南

### 完整示例：OLED 驱动的 OOP 实现

#### 步骤 1：定义 IO 接口（抽象层）
首先定义一个与具体硬件无关的 IO 接口：

```c
// inc/dev_oled.h

#ifndef DEV_OLED_H
#define DEV_OLED_H

#include <stdint.h>
#include <stdbool.h>

/* ============================================================
 * 1. 定义接口类 (Interface) - 模拟虚函数表 (vtable)
 * 这也实现了"依赖倒置"：OLED不依赖具体的I2C，而是依赖这个接口
 * ============================================================ */
typedef struct
{
    // 这是一个回调函数，用于写命令
    void (*WriteCommand)(uint8_t cmd);

    // 这是一个回调函数，用于写数据
    void (*WriteData)(uint8_t *data, uint16_t len);

    // 这是一个回调函数，用于延时 (可选)
    void (*DelayMs)(uint32_t ms);

} OLED_IO_Interface_t;

/* ============================================================
 * 2. 定义对象类 (Class/Object)
 * ============================================================ */
typedef struct
{
    // [属性] Private Data
    uint16_t width;
    uint16_t height;
    uint8_t *frame_buffer;     // 显存指针
    uint16_t buffer_size;

    // [接口] 关联到底层硬件
    const OLED_IO_Interface_t *io;

} OLED_Object_t;

/* ============================================================
 * 3. 方法声明 (Methods) - 第一个参数永远是对象指针 (self)
 * ============================================================ */

// 构造/初始化函数
void OLED_Init(OLED_Object_t *self,
               const OLED_IO_Interface_t *io,
               uint16_t width,
               uint16_t height);

// 绘图方法
void OLED_Clear(OLED_Object_t *self);
void OLED_DrawPixel(OLED_Object_t *self, uint16_t x, uint16_t y, uint8_t color);
void OLED_Update(OLED_Object_t *self);   // 将 buffer 刷新到屏幕
void OLED_DisplayString(OLED_Object_t *self, uint16_t x, uint16_t y, const char *str);

#endif
```

#### 步骤 2：实现 OLED 组件（src/dev_oled.c）
```c
// src/dev_oled.c
#include "dev_oled.h"
#include <string.h>

// 初始化 OLED 对象
void OLED_Init(OLED_Object_t *self,
               const OLED_IO_Interface_t *io,
               uint16_t width,
               uint16_t height)
{
    // 保存配置
    self->width = width;
    self->height = height;
    self->io = io;

    // 分配显存
    self->buffer_size = (width * height) / 8;
    // self->frame_buffer = ek_mem_alloc(self->buffer_size);

    // 发送初始化命令
    self->io->WriteCommand(0xAE);    // 显示关闭
    self->io->WriteCommand(0xD5);    // 设置时钟分频
    self->io->WriteCommand(0x80);
    // ... 更多初始化命令

    self->io->DelayMs(10);
    self->io->WriteCommand(0xAF);    // 显示开启
}

// 清屏
void OLED_Clear(OLED_Object_t *self)
{
    if (self->frame_buffer != NULL)
    {
        memset(self->frame_buffer, 0, self->buffer_size);
    }
}

// 画点
void OLED_DrawPixel(OLED_Object_t *self, uint16_t x, uint16_t y, uint8_t color)
{
    if (x >= self->width || y >= self->height) return;

    uint16_t index = x + (y / 8) * self->width;
    uint8_t bit = y % 8;

    if (color)
    {
        self->frame_buffer[index] |= (1 << bit);
    } else
    {
        self->frame_buffer[index] &= ~(1 << bit);
    }
}

// 刷新屏幕
void OLED_Update(OLED_Object_t *self)
{
    // 通过 IO 接口发送数据
    self->io->WriteCommand(0xB0);    // 设置页地址

    for (uint16_t i = 0; i < self->buffer_size; i++)
    {
        self->io->WriteData(&self->frame_buffer[i], 1);
    }
}

// 显示字符串
void OLED_DisplayString(OLED_Object_t *self, uint16_t x, uint16_t y, const char *str)
{
    // 实现字符串显示逻辑
    while (*str)
    {
        // 画字模...
        str++;
    }
}
```

#### 步骤 3：实现 I2C 适配层（连接 L1_HAL）
```c
// L4_Components/src/oled_i2c_adapter.c
#include "dev_oled.h"
#include "hal_i2c.h"
#include "hal_delay.h"

// OLED 地址定义
#define OLED_I2C_ADDR    0x3C

// 实现 IO 接口的写命令函数
static void I2C_WriteCommand(uint8_t cmd)
{
    uint8_t data[2] = {0x00, cmd};    // 0x00 表示命令
    hal_i2c_master_transmit(OLED_I2C_ADDR, data, 2, 1000);
}

// 实现 IO 接口的写数据函数
static void I2C_WriteData(uint8_t *data, uint16_t len)
{
    uint8_t *buffer = ek_mem_alloc(len + 1);
    buffer[0] = 0x40;    // 0x40 表示数据
    memcpy(&buffer[1], data, len);

    hal_i2c_master_transmit(OLED_I2C_ADDR, buffer, len + 1, 1000);
    ek_mem_free(buffer);
}

// 实现 IO 接口的延时函数
static void I2C_DelayMs(uint32_t ms)
{
    hal_delay_ms(ms);
}

// 定义 IO 接口实例（静态常量，只读）
const OLED_IO_Interface_t g_oled_i2c_interface =
{
    .WriteCommand = I2C_WriteCommand,
    .WriteData    = I2C_WriteData,
    .DelayMs      = I2C_DelayMs,
};
```

#### 步骤 4：在应用层使用（L5_App）
```c
// L5_App/app.c
#include "dev_oled.h"
#include "oled_i2c_adapter.h"

// 定义 OLED 对象（全局或静态）
static OLED_Object_t g_oled;

int ek_main(void)
{
    // 1. 构造 OLED 对象
    OLED_Init(&g_oled, &g_oled_i2c_interface, 128, 64);

    // 2. 使用 OLED
    OLED_Clear(&g_oled);
    OLED_DisplayString(&g_oled, 0, 0, "Hello World!");
    OLED_DrawPixel(&g_oled, 10, 10, 1);
    OLED_Update(&g_oled);

    while(1)
    {
        // 业务逻辑
    }
}
```

### 步骤 5：支持多种硬件（SPI 接口）
由于使用了接口抽象，更换硬件非常简单：

```c
// L4_Components/src/oled_spi_adapter.c
#include "dev_oled.h"
#include "hal_spi.h"
#include "hal_gpio.h"

static void SPI_WriteCommand(uint8_t cmd)
{
    hal_gpio_write_pin(HAL_GPIO_DC, 0);    // DC 拉低表示命令
    hal_spi_transmit(&cmd, 1);
}

static void SPI_WriteData(uint8_t *data, uint16_t len)
{
    hal_gpio_write_pin(HAL_GPIO_DC, 1);    // DC 拉高表示数据
    hal_spi_transmit(data, len);
}

// 定义 SPI 接口实例
const OLED_IO_Interface_t g_oled_spi_interface =
{
    .WriteCommand = SPI_WriteCommand,
    .WriteData    = SPI_WriteData,
    .DelayMs      = hal_delay_ms,
};

// 应用层只需修改初始化参数
// OLED_Init(&g_oled, &g_oled_spi_interface, 128, 64);
```
```c
OLED_Object_t oled1, oled2;
OLED_Init(&oled1, &g_oled_i2c_interface, 128, 64);
OLED_Init(&oled2, &g_oled_spi_interface, 128, 64);

// 独立操作
OLED_Clear(&oled1);
OLED_Clear(&oled2);
```

## 5. 常见问题

**Q: 为什么不直接在 OLED 组件中调用 hal_i2c_send？**

A: 如果直接调用，OLED 组件就与 I2C 硬件强耦合了。使用接口抽象后，OLED 组件只关心"如何发送数据"，不关心"用什么硬件发送"。这样更换硬件（如从 I2C 换到 SPI）时，OLED 组件代码完全不需要修改。

**Q: 函数指针会增加开销吗？**

A: 函数指针的调用开销很小（一次间接跳转），在现代 MCU 上可以忽略不计。换来的架构清晰度和代码可维护性远远大于这个微小开销。如果确实需要优化，可以将关键接口声明为 `inline` 或使用宏。

**Q: 每个 C 文件都要实现一套接口吗？**

A: 不一定。如果多个组件使用相同的接口（如多个 I2C 设备），可以实现一个通用的适配器，所有组件共享。

**Q: 如何处理组件的多个实例？**

A: 每个实例对应一个 `OLED_Object_t` 结构体。如果有两个 OLED 屏幕（一个用 I2C，一个用 SPI），只需创建两个对象，分别初始化不同的 IO 接口即可：

