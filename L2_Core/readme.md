# L2_Core (核心与硬件抽象层)

## 1. 简介

L2_Core 是本项目的核心与硬件抽象层，提供基础功能模块和硬件抽象接口。

向下：依赖 L1_MCU 层的厂商库。

向上：为 L3~L5 层提供通用的数据结构、服务功能和硬件抽象接口。

本层包含三个子目录：
- **utils/**：与硬件无关的基础功能（数据结构、内存管理、日志等）
- **hal/**：硬件抽象层（GPIO、UART、I2C、SPI、定时器等）
- **port/**：MCU 移植层（将抽象 HAL 映射到具体硬件）

**OBJECT 库架构**：

本层使用 OBJECT 库模式，所有源文件编译为对象文件后直接参与最终链接。

```cmake
# L2_Core/CMakeLists.txt
add_library(l2_core OBJECT ${L2_SRCS})

# 最终链接
target_link_libraries(${CMAKE_PROJECT_NAME}
    $<TARGET_OBJECTS:l2_core>
    # ...
)
```

OBJECT 库的优势：
- 避免静态库的选择性链接问题
- 所有符号自动包含，无需复杂的链接器选项

## 2. 目录结构

```text
L2_Core/
├── CMakeLists.txt              # 构建脚本
├── readme.md                  # 本文档
│
├── utils/                     # 工具库（硬件无关）
│   ├── inc/                   # 对外接口
│   │   ├── ek_def.h          # 通用定义和编译器宏
│   │   ├── ek_list.h         # 双向循环链表（纯头文件）
│   │   ├── ek_ringbuf.h      # 环形缓冲区
│   │   ├── ek_stack.h        # 栈数据结构
│   │   ├── ek_mem.h          # 动态内存管理（TLSF）
│   │   ├── ek_log.h          # 分级日志系统
│   │   ├── ek_io.h           # 标准 IO（基于 lwprintf）
│   │   ├── ek_vec.h          # 动态数组（纯宏实现）
│   │   ├── ek_str.h          # 动态字符串
│   │   ├── ek_export.h       # 函数自动导出机制
│   │   ├── ek_assert.h       # 断言模块
│   │   └── ek_shell.h        # 命令行接口（letter_shell）
│   └── src/                  # 内部实现
│       ├── ek_ringbuf.c
│       ├── ek_stack.c
│       ├── ek_mem.c
│       ├── ek_log.c
│       ├── ek_io.c
│       ├── ek_str.c
│       ├── ek_export.c
│       └── ek_assert.c
│
├── hal/                       # 硬件抽象层（OOP 设计）
│   ├── inc/                   # HAL 接口（不含厂商头文件）
│   │   ├── ek_hal_gpio.h     # GPIO 抽象
│   │   ├── ek_hal_uart.h     # UART 抽象
│   │   ├── ek_hal_i2c.h      # I2C 抽象
│   │   ├── ek_hal_spi.h      # SPI 抽象
│   │   ├── ek_hal_tick.h     # 系统节拍
│   │   ├── ek_hal_tim.h      # 定时器
│   │   ├── ek_hal_dma2d.h    # DMA2D 硬件加速
│   │   └── ek_hal_ltdc.h     # LTDC 显示控制器
│   └── src/                  # HAL 实现（包含厂商头文件）
│       ├── ek_hal_gpio.c
│       ├── ek_hal_uart.c
│       ├── ek_hal_i2c.c
│       ├── ek_hal_spi.c
│       ├── ek_hal_tick.c
│       ├── ek_hal_tim.c
│       ├── ek_hal_dma2d.c
│       └── ek_hal_ltdc.c
│
├── port/                      # MCU 移植层
│   ├── inc/
│   │   └── st_hal_port.h     # 移植层接口声明
│   └── stm32f429zi/          # STM32F429ZI 移植实现
│       ├── st_gpio_port.c     # GPIO 驱动（132行）
│       ├── st_uart_port.c     # UART 驱动（110行）
│       ├── st_i2c_port.c      # I2C 驱动（138行）
│       ├── st_spi_port.c      # SPI 驱动（97行）
│       ├── st_tick_port.c     # Tick 驱动（50行）
│       ├── st_tim_port.c      # 定时器驱动（86行）
│       ├── st_dma2d_port.c    # DMA2D 驱动（250行）
│       └── st_ltdc_port.c     # LTDC 驱动（92行）
│
└── third_party/               # 第三方库
    ├── tlsf/                  # TLSF 内存分配器
    │   └── tlsf.h
    ├── lwprintf/              # 轻量级 printf
    │   └── inc/
    │       ├── lwprintf.h
    │       ├── lwprintf_opt.h
    │       └── lwprintf_sys.h
    └── letter_shell/          # 命令行 Shell
        └── inc/
            ├── shell.h
            ├── shell_ext.h
            └── shell_cfg.h
```

## 3. 实现状态

### 3.1 代码统计

| 分类 | 头文件 | 源文件 | 总行数 | 状态 |
|------|---------|---------|--------|------|
| **utils/** | 12 | 8 | 774行 | ✅ 100% |
| **hal/** | 8 | 8 | 891行 | ✅ 100% |
| **port/** | 1 | 7 | 955行 | ✅ 100% |
| **第三方库** | 4 | 5 | 4,238行 | ✅ 100% |
| **合计** | 25 | 28+ | 6,858行 | ✅ 100% |

### 3.2 utils/ 子目录状态

| 模块 | 头文件 | 源文件 | 状态 | 说明 |
|------|---------|---------|------|------|
| ek_def.h | ✅ | - | ✅ 完成 | 跨编译器宏定义 |
| ek_list.h | ✅ | - | ✅ 完成 | 双向循环链表（纯头文件） |
| ek_ringbuf | ✅ | ✅ | ✅ 完成 | 环形缓冲区（150行） |
| ek_stack | ✅ | ✅ | ✅ 完成 | 栈数据结构（119行） |
| ek_mem | ✅ | ✅ | ✅ 完成 | 动态内存管理（116行） |
| ek_log | ✅ | ✅ | ✅ 完成 | 分级日志系统（76行） |
| ek_io | ✅ | ✅ | ✅ 完成 | 标准 IO（33行） |
| ek_vec.h | ✅ | - | ✅ 完成 | 动态数组（纯宏实现） |
| ek_str | ✅ | ✅ | ✅ 完成 | 动态字符串（217行） |
| ek_export | ✅ | ✅ | ✅ 完成 | 函数自动导出（34行） |
| ek_assert | ✅ | ✅ | ✅ 完成 | 断言模块（29行） |
| ek_shell.h | ✅ | - | ✅ 完成 | 命令行接口（封装） |

### 3.3 hal/ 子目录状态

| 模块 | 头文件 | 源文件 | 状态 | 说明 |
|------|---------|---------|------|------|
| ek_hal_gpio | ✅ | ✅ | ✅ 完成 | GPIO 抽象（102行） |
| ek_hal_uart | ✅ | ✅ | ✅ 完成 | UART 抽象（95行） |
| ek_hal_i2c | ✅ | ✅ | ✅ 完成 | I2C 抽象（130行） |
| ek_hal_spi | ✅ | ✅ | ✅ 完成 | SPI 抽象（99行） |
| ek_hal_tick | ✅ | ✅ | ✅ 完成 | 系统节拍（79行） |
| ek_hal_tim | ✅ | ✅ | ✅ 完成 | 定时器（104行） |
| ek_hal_dma2d | ✅ | ✅ | ✅ 完成 | DMA2D 硬件加速（139行） |
| ek_hal_ltdc | ✅ | ✅ | ✅ 完成 | LTDC 显示控制器（143行） |

### 3.4 port/ 子目录状态

| 驱动 | 源文件 | 行数 | 状态 |
|------|---------|------|------|
| GPIO 驱动 | st_gpio_port.c | 132行 | ✅ 完成 |
| UART 驱动 | st_uart_port.c | 110行 | ✅ 完成 |
| I2C 驱动 | st_i2c_port.c | 138行 | ✅ 完成 |
| SPI 驱动 | st_spi_port.c | 97行 | ✅ 完成 |
| Tick 驱动 | st_tick_port.c | 50行 | ✅ 完成 |
| 定时器驱动 | st_tim_port.c | 86行 | ✅ 完成 |
| DMA2D 驱动 | st_dma2d_port.c | 250行 | ✅ 完成 |
| LTDC 驱动 | st_ltdc_port.c | 92行 | ✅ 完成 |

### 3.5 第三方库状态

| 库 | 说明 | 状态 |
|----|----|------|
| TLSF | Two-Level Segregated Fit 内存分配器 | ✅ 已集成 |
| lwprintf | 轻量级格式化输出库 | ✅ 已集成 |
| letter_shell | 命令行 Shell | ✅ 已集成 |

## 4. 核心开发原则 (Strict Rules)

为了保证架构的整洁性和可移植性，开发本层时必须遵守以下规则：

**utils/ 子目录（硬件无关）**：

- 禁止包含任何与硬件相关的头文件（如 stm32xxxx.h、gpio.h 等）
- 禁止直接操作寄存器或调用硬件相关函数
- 只使用标准 C 语言库（stdint.h、stdbool.h、string.h 等）

**hal/ 子目录（硬件抽象）**：

- `inc/*.h` 绝对不能包含厂商头文件（stm32xxxx.h）
- 厂商头文件仅允许在 `src/*.c` 中包含
- 提供逻辑到物理的映射（如设备名 → GPIOA PIN_5）
- 所有函数使用 `ek_hal_` 前缀（如 `ek_hal_gpio_set()`）

**port/ 子目录（移植层）**：

- 包含具体的硬件驱动实现
- 可以包含厂商头文件
- 使用设备表驱动模式
- 通过 EK_EXPORT 机制自动注册设备

**内部依赖关系**：

- `hal/` 可以依赖 `utils/`（同层内部允许依赖）
- `port/` 可以依赖 `hal/` 和 `utils/`
- `utils/` 不能依赖 `hal/` 或 `port/`（保持纯软件实现）

## 5. HAL 架构设计（OOP 模式）

本层所有 HAL 模块采用统一的 OOP（面向对象）设计模式：

### 5.1 设备结构体

```c
// 设备结构体
typedef struct ek_hal_xxx_t ek_hal_xxx_t;

struct ek_hal_xxx_t
{
    ek_list_node_t node;        // 链表节点（用于设备管理）
    const char *name;           // 设备名称
    const ek_xxx_ops_t *ops;   // 操作函数集（虚函数表）
    void *dev_info;            // 驱动私有数据
    // ... 设备特定属性
};
```

### 5.2 操作函数集（虚函数表）

```c
// 操作函数集（虚函数表）
typedef struct ek_xxx_ops_t
{
    void (*init)(ek_hal_xxx_t *const dev);
    // ... 其他操作
} ek_xxx_ops_t;
```

### 5.3 设备管理

- 所有设备通过链表管理
- 支持按名称查找设备
- 支持动态注册和注销

```c
// 注册设备
void ek_hal_xxx_register(
    ek_hal_xxx_t *const dev,
    const char *name,
    const ek_xxx_ops_t *ops,
    void *dev_info
);

// 查找设备
ek_hal_xxx_t *ek_hal_xxx_find(const char *name);
```

## 6. 移植层设计（port/）

### 6.1 设备表驱动模式

移植层使用设备表驱动模式，例如 `st_gpio_port.c`：

```c
// 设备表
static const st_gpio_info st_drv_gpio_table[] = {
    {&drv_lcd_cs,    "LCD_CS",    EK_GPIO_MODE_OUTPUT_PP, GPIOC, GPIO_PIN_2  },
    {&drv_lcd_wrx,   "LCD_WRX",   EK_GPIO_MODE_OUTPUT_PP, GPIOD, GPIO_PIN_13 },
    // ...
};

// 注册到 HAL（使用自动导出）
EK_EXPORT_HARDWARE(st_gpio_drv_init);
```

### 6.2 EK_EXPORT 自动注册

使用 `EK_EXPORT` 宏实现自动初始化：

```c
// 硬件初始化（优先级 0）
EK_EXPORT_HARDWARE(st_gpio_drv_init);

// 组件初始化（优先级 1）
EK_EXPORT_COMPONENTS(my_component_init);

// 应用初始化（优先级 2）
EK_EXPORT_APP(my_app_init);
```

## 7. 模块使用指南

### 7.1 使用双向链表（ek_list.h）

```c
#include "ek_list.h"

// 定义链表头
ek_list_node_t my_list;
ek_list_init(&my_list);

// 定义节点结构体
typedef struct {
    ek_list_node_t node;
    int data;
} my_node_t;

// 添加节点
ek_list_add_tail(&my_list, &node->node);

// 遍历链表
ek_list_node_t *pos;
ek_list_iterate(pos, &my_list) {
    my_node_t *n = ek_list_container(pos, my_node_t, node);
    // 处理节点
}

// 移除节点
ek_list_remove(&node->node);
```

### 7.2 使用动态数组（ek_vec.h）

```c
#include "ek_vec.h"

// 定义 int 类型的动态数组
EK_VEC_IMPLEMENT(int);

// 使用动态数组
ek_vec_t(int) my_vec;
ek_vec_init(my_vec);

// 添加元素
ek_vec_append(my_vec, 42);
ek_vec_append(my_vec, 100);

// 遍历
uint32_t i;
ek_vec_iterate(i, my_vec) {
    printf("%d\n", my_vec.items[i]);
}

// 销毁
ek_vec_destroy(my_vec);
```

### 7.3 使用动态字符串（ek_str.h）

```c
#include "ek_str.h"

// 创建字符串
ek_str_t *s = ek_str_create("Hello");

// 追加字符串
ek_str_append(s, " World");
ek_str_append_fmt(s, " %d", 2024);

// 获取 C 风格字符串
const char *cstr = ek_str_get_cstring(s);
printf("%s\n", cstr);  // "Hello World 2024"

// 释放
ek_str_free(s);
```

### 7.4 使用内存管理（ek_mem.h）

```c
#include "ek_mem.h"

// 分配内存
void *ptr = ek_malloc(1024);
if (ptr == NULL) {
    // 处理内存不足
}

// 重新分配
ptr = ek_realloc(ptr, 2048);

// 释放内存
ek_free(ptr);

// 查询内存状态
uint32_t total = ek_heap_total_size();  // 总大小
uint32_t used = ek_heap_used();        // 已使用
uint32_t unused = ek_heap_unused();    // 空闲
```

### 7.5 使用日志系统（ek_log.h）

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

### 7.6 使用 HAL 设备

```c
#include "ek_hal_gpio.h"

// 查找设备
ek_hal_gpio_t *gpio = ek_hal_gpio_find("LED1");
if (gpio == NULL) {
    // 设备不存在
}

// 设置电平
ek_hal_gpio_set(gpio, EK_GPIO_STATUS_SET);

// 翻转
ek_hal_gpio_toggle(gpio);

// 读取状态
ek_gpio_status_t status = ek_hal_gpio_read(gpio);
```

### 7.7 使用自动导出

```c
#include "ek_export.h"

// 定义初始化函数
void my_hardware_init(void)
{
    // 硬件初始化代码
}

// 导出函数（指定优先级）
EK_EXPORT_HARDWARE(my_hardware_init);

// 在主函数中调用
int main(void)
{
    // 自动执行所有导出的函数（按优先级）
    ek_export_init();

    // ...
}
```

## 8. 添加新模块

### 8.1 添加 utils 模块（硬件无关）

#### 步骤 1：定义接口 (utils/inc/ek_xxx.h)

```c
// utils/inc/ek_example.h
#ifndef EK_EXAMPLE_H
#define EK_EXAMPLE_H

#include "ek_def.h"

// API 函数
void ek_example_init(void);
int ek_example_do_something(int value);

#endif
```

#### 步骤 2：实现接口 (utils/src/ek_xxx.c)

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

### 8.2 添加 hal 模块（硬件抽象）

#### 步骤 1：定义接口 (hal/inc/ek_hal_xxx.h)

**重要：头文件中不能包含厂商头文件！**

```c
// hal/inc/ek_hal_xxx.h
#ifndef EK_HAL_XXX_H
#define EK_HAL_XXX_H

#include "ek_def.h"
#include "ek_list.h"

// 操作函数集
typedef struct ek_xxx_ops_t
{
    void (*init)(ek_hal_xxx_t *const dev);
    // ... 其他操作
} ek_xxx_ops_t;

// 设备结构体
typedef struct ek_hal_xxx_t
{
    ek_list_node_t node;
    const char *name;
    const ek_xxx_ops_t *ops;
    void *dev_info;
    // ... 设备特定属性
} ek_hal_xxx_t;

// API 函数
void ek_hal_xxx_register(
    ek_hal_xxx_t *const dev,
    const char *name,
    const ek_xxx_ops_t *ops,
    void *dev_info
);

ek_hal_xxx_t *ek_hal_xxx_find(const char *name);

#endif
```

#### 步骤 2：实现接口 (hal/src/ek_hal_xxx.c)

```c
// hal/src/ek_hal_xxx.c
#include "ek_hal_xxx.h"
#include "stm32f4xx_hal.h"  // 厂商头文件

void ek_hal_xxx_register(
    ek_hal_xxx_t *const dev,
    const char *name,
    const ek_xxx_ops_t *ops,
    void *dev_info)
{
    dev->name = name;
    dev->ops = ops;
    dev->dev_info = dev_info;
    ek_list_add_tail(&ek_hal_xxx_head, &dev->node);
}

ek_hal_xxx_t *ek_hal_xxx_find(const char *name)
{
    ek_list_node_t *pos;
    ek_list_iterate(pos, &ek_hal_xxx_head) {
        ek_hal_xxx_t *dev = ek_list_container(pos, ek_hal_xxx_t, node);
        if (strcmp(dev->name, name) == 0) {
            return dev;
        }
    }
    return NULL;
}
```

#### 步骤 3：实现移植层 (port/stm32f429zi/st_xxx_port.c)

```c
// port/stm32f429zi/st_xxx_port.c
#include "ek_hal_xxx.h"
#include "stm32f4xx_hal.h"

// 设备表
static const st_xxx_info st_drv_xxx_table[] = {
    {&drv_xxx_1, "XXX_1", /* 配置 */},
    // ...
};

// 初始化函数
void st_xxx_drv_init(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(st_drv_xxx_table); i++) {
        // 注册设备
        ek_hal_xxx_register(
            st_drv_xxx_table[i].dev,
            st_drv_xxx_table[i].name,
            &st_xxx_ops,
            (void *)&st_drv_xxx_table[i]
        );
    }
}

// 自动导出
EK_EXPORT_HARDWARE(st_xxx_drv_init);
```

### 8.3 添加移植层

创建新的移植层目录（如 `port/stm32f407/`），参考 `stm32f429zi/` 的实现：

```c
// port/stm32f407/st_gpio_port.c
#include "ek_hal_gpio.h"
#include "stm32f4xx_hal.h"

// 设备表（STM32F407 特定）
static const st_gpio_info st_drv_gpio_table[] = {
    // ... STM32F407 的 GPIO 配置
};

void st_gpio_drv_init(void)
{
    // 注册逻辑
}

EK_EXPORT_HARDWARE(st_gpio_drv_init);
```

## 9. 常见问题

**Q: hal/ 子目录可以直接使用 utils/ 的数据结构吗？**

A: 可以！这是新架构的优势。hal/ 可以使用 utils/ 提供的链表、环形缓冲区等数据结构来管理设备，因为它们属于同一层。

**Q: 为什么 hal 的头文件不能包含厂商头文件？**

A: 保持头文件的纯净性，避免泄露硬件细节到上层。上层应用只看到设备名称，而不需要知道底层是哪个 GPIO 端口。

**Q: 动态内存管理为什么要自己实现？**

A: 嵌入式系统中标准库的 malloc 有很多问题：碎片化、不确定性、占用内存大。本层使用 TLSF（Two-Level Segregated Fit）内存分配器，更适合嵌入式场景。

**Q: 如何添加新的 MCU 支持？**

A: 在 `port/` 下创建新的子目录（如 `port/stm32f407/`），实现相应的移植层代码，然后使用 EK_EXPORT 自动注册设备。

**Q: EK_EXPORT 的优先级是什么意思？**

A: 优先级数字越小越先执行：
- `EK_EXPORT_HARDWARE` (0)：硬件初始化最先执行
- `EK_EXPORT_COMPONENTS` (1)：组件初始化
- `EK_EXPORT_APP` (2)：应用初始化最后执行
- `EK_EXPORT_USER` (3)：用户自定义

**Q: ek_vec.h 为什么使用宏而不是函数？**

A: 宏实现可以提供类型安全，避免 `void*` 的类型不安全问题。同时，宏在编译时展开，没有函数调用开销。
