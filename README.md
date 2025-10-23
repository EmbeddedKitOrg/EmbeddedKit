---
author: 左岚
---

# EmbeddedKit (EK) 代码规范

## 概述

本文档定义了EmbeddedKit嵌入式组件库的代码规范，旨在确保代码的一致性、可读性和可维护性。

## CMake 集成指南

为了方便在其他项目中复用 EmbeddedKit，本仓库提供了标准的 CMake 构建脚本和安装信息，用户可以通过以下两种方式集成：

### 方式一：作为子目录直接引用

1. 将仓库克隆或以子模块形式放入现有工程（例如放在 `external/EmbeddedKit`）。
2. 在主工程的 `CMakeLists.txt` 中添加子目录并链接目标：

    ```cmake
    add_subdirectory(external/EmbeddedKit)
    target_link_libraries(MyApp PRIVATE EmbeddedKit::EmbeddedKit)
    ```

    目标 `EmbeddedKit::EmbeddedKit` 会自动暴露所有头文件目录，无需手动设置 `include_directories`。

### 方式二：安装后通过 find_package 引用

1. 在 EmbeddedKit 根目录配置并安装（安装前可根据需要调整前缀）：

    ```pwsh
    cmake -S . -B build -DCMAKE_INSTALL_PREFIX="${Env:USERPROFILE}/.local"
    cmake --build build --config Release
    cmake --install build --config Release
    ```

2. 在消费工程中查找并链接库：

    ```cmake
    find_package(EmbeddedKit CONFIG REQUIRED)
    target_link_libraries(MyApp PRIVATE EmbeddedKit::EmbeddedKit)
    ```

    调用 `find_package` 后还会得到变量 `EmbeddedKit_INCLUDE_DIR` 指向安装的头文件根目录。

### 核心设计原则

- **类型安全**：通过函数名前缀明确标识返回类型
- **状态统一**：使用全局统一的EK_Result枚举处理函数执行状态
- **命名清晰**：通过命名约定快速识别函数用途和作用域
- **模块化**：清晰的内部/外部API边界

### 返回类型前缀快速参考

| 前缀 | 返回类型                  | 说明             | 示例                   |
| ---- | ------------------------- | ---------------- | ---------------------- |
| `v_` | void                      | 无返回值         | `EK_vInitSystem()`     |
| `r_` | EK_Result                 | 全局统一状态枚举 | `EK_rConfigureTimer()` |
| `i_` | int                       | 有符号整数       | `EK_iGetSensorValue()` |
| `u_` | uint32_t/uint16_t/uint8_t | 无符号整数       | `EK_uGetSystemClock()` |
| `c_` | char                      | 字符类型         | `EK_cGetStatusChar()`  |
| `b_` | bool                      | 布尔类型         | `EK_bIsSystemReady()`  |
| `p_` | pointer                   | 指针类型         | `EK_pGetBuffer()`      |
| `s_` | size_t                    | 大小/长度类型    | `EK_sGetBufferSize()`  |

## 文件命名规范

### C/H文件命名

- **规则**：使用`EK_`前缀 + 大驼峰命名法（PascalCase）
- C文件示例
  - `EK_Timer.c`
  - `EK_Gpio.c`
  - `EK_Uart.c`
  - `EK_Config.c`
- 头文件示例
  - `EK_Timer.h`
  - `EK_Gpio.h`
  - `EK_Uart.h`
  - `EK_Config.h`

## 工程结构规范

### STM32嵌入式项目目录结构

```
EmbeddedKit/
├── Inc/                    # 头文件目录
│   ├── EK_Common.h        # 公共定义
│   ├── EK_Config.h        # 配置文件
│   ├── EK_Timer.h         # 定时器模块
│   ├── EK_Gpio.h          # GPIO模块
│   ├── EK_Uart.h          # UART模块
│   └── EK_Spi.h           # SPI模块
├── Src/                    # 源文件目录
│   ├── EK_Timer.c
│   ├── EK_Gpio.c
│   ├── EK_Uart.c
│   ├── EK_Spi.c
│   └── EK_Config.c
├── Driver/                 # STM32底层驱动
│   ├── CMSIS/             # CMSIS文件
│   └── HAL/               # STM32 HAL库文件
├── Examples/               # 示例代码
│   ├── Basic/
│   │   ├── GPIO_Example/
│   │   ├── UART_Example/
│   │   └── Timer_Example/
│   └── Advanced/
│       ├── Communication/
│       └── Control/
├── Docs/                   # 文档
│   ├── API/
│   └── UserGuide/
├── Tools/                  # 工具和脚本
│   ├── Scripts/
│   └── Config/
└── Makefile               # 构建文件
```

## 命名约定

### API命名规范

#### 公开API

- **规则**：使用`EK_`前缀 + 大驼峰命名法（PascalCase）

- **适用范围**：对外暴露的接口、函数、结构体等

- 示例

  ```c
  // 公开API函数void EK_vInitSystem(void);EK_Result EK_rConfigureTimer(uint32_t frequency);bool EK_bCheckSensorStatus(void);// 公开结构体typedef struct {    uint32_t frequency;    uint8_t mode;} EK_TimerConfig;// 公开枚举typedef enum {    EK_STATUS_OK = 0,    EK_STATUS_ERROR,    EK_STATUS_TIMEOUT} EK_Status;
  ```

#### 全局变量和静态变量

- **规则**：使用类型前缀 + 大驼峰命名法（PascalCase）

- 示例

  ```c
  // 全局变量 - 仅使用类型前缀volatile uint32_t uSystemClock;bool bIsInitialized;uint8_t* pGlobalBuffer;// 静态变量 - 仅使用类型前缀static uint32_t uInternalCounter;static const char* pVersionString;static bool bModuleReady;
  ```

#### 内部变量和函数

- **规则**：使用小写加下划线命名法（snake_case），无EK前缀

- 示例

  ```c
  // 内部函数 - 无EK前缀static void v_internal_init(void);static int r_calculate_checksum(uint8_t* data, size_t len);static uint8_t* p_allocate_buffer(size_t size);// 内部变量 - 使用类型前缀static uint32_t u_buffer_size;static bool b_is_ready;static int i_error_count;
  ```

### 函数返回类型命名约定

函数名的首字母表示返回类型：

#### v_ 开头 - void类型

```c
// 公开API
void EK_vInitGpio(uint32_t pin, uint32_t mode);
void EK_vResetSystem(void);
// 内部函数
static void v_clear_buffer(void);
static void v_update_status(void);
```

#### r_ 开头 - result类型（通常为int、EK_Result等状态返回值）

```c
// 公开API
EK_Result EK_rConfigureUart(uint32_t baudrate);
int EK_rCheckConnection(void);
// 内部函数
static int r_validate_data(uint8_t* data);
static bool r_is_valid_pin(uint32_t pin);
```

#### p_ 开头 - pointer类型

```c
// 公开API
uint8_t* EK_pGetBuffer(size_t size);
char* EK_pGetVersionString(void);
// 内部函数
static uint32_t* p_get_register_address(uint32_t offset);
static void* p_allocate_memory(size_t size);
```

#### u_ 开头 - unsigned类型

```c
// 公开API
uint32_t EK_uGetSystemClock(void);
uint16_t EK_uReadADC(uint8_t channel);
// 内部函数
static uint32_t u_calculate_frequency(void);
static uint8_t u_get_status_register(void);
```

#### b_ 开头 - bool类型

```c
// 公开API
bool EK_bIsSystemReady(void);
bool EK_bCheckSensorStatus(void);
// 内部函数
static bool b_validate_config(void);
static bool b_is_timeout(uint32_t start_time);
```

## 代码结构规范

### 头文件保护

```c
#ifndef EK_MODULE_NAME_H
#define EK_MODULE_NAME_H
// 头文件内容
#endif /* EK_MODULE_NAME_H */
```

### 包含文件顺序

```c
// 1. 标准C库头文件
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
// 2. 平台相关头文件
#include "platform.h"
// 3. EK公共头文件
#include "EK_Common.h"
// 4. 当前模块相关头文件
#include "EK_Timer.h"
```

### 常量和宏定义

```c
// 使用大写字母和下划线
#define EK_MAX_BUFFER_SIZE      1024
#define EK_DEFAULT_TIMEOUT      5000
#define EK_TIMER_FREQ_MAX       100000
// 复杂宏使用do-while包装
#define EK_SAFE_FREE(ptr) do { \
    if (ptr != NULL) { \
        free(ptr); \
        ptr = NULL; \
    } \
} while(0)
```

### 枚举类型

```c
typedef enum {
    EK_OK = 0,
    EK_ERROR,
    EK_TIMEOUT,
    EK_INVALID_PARAM,
    EK_NO_MEMORY
} EK_Result;
typedef enum {
    EK_GPIO_MODE_INPUT = 0,
    EK_GPIO_MODE_OUTPUT,
    EK_GPIO_MODE_ALTERNATE
} EK_GpioMode;
```

## 代码格式规范

### 缩进和空格

- 使用4个空格缩进，不使用Tab
- 函数参数过长时换行对齐
- 运算符前后加空格

```c
// 正确示例
EK_Result EK_RConfigureTimer(uint32_t frequency, 
                            uint8_t mode, 
                            bool enable_interrupt);
if (frequency > EK_TIMER_FREQ_MAX) {
    return EK_INVALID_PARAM;
}
uint32_t uResult = (uValue1 + uValue2) * uMultiplier;
```

### 大括号风格

```c
// 函数定义
void EK_VInitSystem(void) 
{
    // 函数体
}
// 控制结构
if (bCondition) {
    // 代码块
} else {
    // 代码块
}
for (int i = 0; i < iCount; i++) {
    // 循环体
}
```

## 注释规范

### 文件头注释

```c
/**
 * @file    EK_Timer.c
 * @brief   EmbeddedKit 定时器模块实现
 * @author  EK Team
 */
```

### 函数注释

```c
/**
 * @brief  配置定时器参数
 * @param  frequency 定时器频率 (Hz)
 * @param  mode      定时器模式
 * @retval EK_OK 配置成功
 * @retval EK_ERROR 配置失败
 * @note   频率范围：1Hz - 100kHz
 */
EK_Result EK_RConfigureTimer(uint32_t frequency, uint8_t mode);
```

### 结构体注释

```c
/**
 * @brief 定时器配置结构体
 */
typedef struct {
    uint32_t frequency;     /*!< 定时器频率 */
    uint8_t mode;          /*!< 工作模式 */
    bool auto_reload;      /*!< 是否自动重载 */
} EK_TimerConfig;
```

## 错误处理

### 错误码定义

```c
typedef enum {
    EK_OK = 0,              /*!< 操作成功 */
    EK_ERROR = -1,          /*!< 通用错误 */
    EK_INVALID_PARAM = -2,  /*!< 参数错误 */
    EK_TIMEOUT = -3,        /*!< 超时错误 */
    EK_NO_MEMORY = -4,      /*!< 内存不足 */
    EK_NOT_INITIALIZED = -5 /*!< 未初始化 */
} EK_Result;
```

### 错误检查模式

```c
// 参数检查
EK_Result EK_RConfigureTimer(uint32_t frequency, uint8_t mode) 
{
    // 参数有效性检查
    if (frequency == 0 || frequency > EK_TIMER_FREQ_MAX) {
        return EK_INVALID_PARAM;
    }
    if (mode >= EK_TIMER_MODE_COUNT) {
        return EK_INVALID_PARAM;
    }
    // 实际配置逻辑
    // ...
    return EK_OK;
}
```

## 示例代码

### 完整模块示例

```c
/**
 * @file    EK_Led.c
 * @brief   EmbeddedKit LED控制模块
 */
#include <stdint.h>
#include <stdbool.h>
#include "EK_Led.h"
// 全局状态变量 - 使用类型前缀
static bool bLedInitialized = false;
static uint32_t uLedCount = 0;
// 内部函数声明
static EK_Result r_validate_led_id(uint32_t led_id);
static void v_hardware_init(void);
/**
 * @brief  初始化LED模块
 * @retval EK_OK 初始化成功
 */
EK_Result EK_RInitLed(void) 
{
    if (bLedInitialized) {
        return EK_OK;
    }
    v_hardware_init();
    bLedInitialized = true;
    return EK_OK;
}
/**
 * @brief  控制LED状态
 * @param  led_id LED编号
 * @param  state  LED状态 (true=亮, false=灭)
 * @retval EK_OK 操作成功
 * @retval EK_INVALID_PARAM 参数错误
 */
EK_Result EK_RSetLedState(uint32_t led_id, bool state) 
{
    if (r_validate_led_id(led_id) != EK_OK) {
        return EK_INVALID_PARAM;
    }
    // 硬件控制逻辑
    // ...
    return EK_OK;
}
/**
 * @brief  获取LED状态指针
 * @param  led_id LED编号
 * @retval LED状态指针，失败返回NULL
 */
bool* EK_PGetLedState(uint32_t led_id) 
{
    if (r_validate_led_id(led_id) != EK_OK) {
        return NULL;
    }
    // 返回状态指针
    // ...
    return NULL;
}
// 内部函数实现
static EK_Result r_validate_led_id(uint32_t led_id) 
{
    return (led_id < uLedCount) ? EK_OK : EK_INVALID_PARAM;
}
static void v_hardware_init(void) 
{
    // 硬件初始化逻辑
    // ...
}
```