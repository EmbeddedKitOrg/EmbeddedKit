# L5_App (应用业务逻辑层)

## 1. 简介
L5_App 是本项目的应用业务逻辑层，实现系统的具体功能和业务逻辑。

向下：调用 L2~L4 层提供的服务（如组件驱动、中间件、核心库）。

向上：不向上提供服务，本层是整个系统的最高层。

本层的核心职责是实现**业务逻辑封装**和**模块化设计**，将复杂功能分解为独立的模块。

## 2. 目录结构
```text
L5_App
├── CMakeLists.txt          # 构建脚本
├── inc                     # [对外接口] 应用层头文件
│   ├── app.h              # 应用入口声明
│   ├── app_menu.h         # 菜单模块
│   ├── app_sensor.h       # 传感器采集模块
│   └── app_display.h      # 显示控制模块
└── src                     # [内部实现] 应用源文件
    ├── app.c              # 主程序入口
    ├── app_menu.c
    ├── app_sensor.c
    └── app_display.c
```

## 3. 核心开发原则 (Strict Rules)
为了保证代码的可维护性和可测试性，开发本层时必须遵守以下规则：

**业务逻辑封装**：

每个功能模块应该独立封装，对外提供清晰的接口。

避免跨模块直接访问内部变量。

**模块化设计**：

将复杂功能分解为多个小模块，每个模块负责单一职责。

模块之间通过接口通信，减少耦合。

**与 L0 的入口对接**：

必须实现 `ek_main()` 函数作为应用层入口。

L0_MCU 的 main.c 负责调用此函数。

## 4. 开发指南

### 步骤 1：定义应用入口 (inc/app.h)
```c
// inc/app.h
#ifndef APP_H
#define APP_H

#include <stdint.h>

// 应用层入口函数（由 L0 层调用）
// 返回值：0表示正常退出，非0表示错误码
int ek_main(void);

// 系统初始化
void app_init(void);

// 主循环
void app_loop(void);

#endif
```

### 步骤 2：实现 ek_main 入口 (src/app.c)
```c
// src/app.c
#include "app.h"
#include "dev_oled.h"
#include "app_sensor.h"
#include "app_display.h"
#include "ek_log.h"

// 各模块对象
static OLED_Object_t g_oled;
static Sensor_t g_sensor;

// 系统初始化
void app_init(void)
{
    ek_log_info("System initializing...");

    // 初始化显示模块
    app_display_init(&g_oled);

    // 初始化传感器模块
    app_sensor_init(&g_sensor);

    ek_log_info("System ready");
}

// 主循环
void app_loop(void)
{
    // 读取传感器
    float temp = app_sensor_read_temperature(&g_sensor);

    // 更新显示
    app_display_update(&g_oled, temp);

    // 延时
    hal_delay_ms(1000);
}

// 应用层入口
int ek_main(void)
{
    // 1. 系统初始化
    app_init();

    // 2. 主循环
    while(1) {
        app_loop();
    }

    return 0;
}
```

### 步骤 3：创建功能模块
将功能分解为独立的模块：

```c
// inc/app_sensor.h
#ifndef APP_SENSOR_H
#define APP_SENSOR_H

typedef struct
{
    uint32_t raw_value;
    float temperature;
    uint8_t unit;  // 0=Celsius, 1=Fahrenheit
} Sensor_t;

void app_sensor_init(Sensor_t *self);
float app_sensor_read_temperature(Sensor_t *self);

#endif
```

```c
// src/app_sensor.c
#include "app_sensor.h"
#include "dev_adc.h"    // L4 层的 ADC 组件
#include "ek_log.h"

void app_sensor_init(Sensor_t *self)
{
    self->unit = 0;  // 默认摄氏度
    dev_adc_init();   // 初始化 ADC 硬件
    ek_log_info("Sensor initialized");
}

float app_sensor_read_temperature(Sensor_t *self)
{
    // 读取 ADC 原始值
    self->raw_value = dev_adc_read_channel(ADC_CHANNEL_TEMP);

    // 转换为温度值
    self->temperature = (self->raw_value * 3.3f / 4096.0f) * 100.0f;

    // 单位转换
    if (self->unit == 1)
    {
        self->temperature = self->temperature * 9.0f / 5.0f + 32.0f;
    }

    return self->temperature;
}
```

### 步骤 4：模块间通信
模块之间应该通过接口通信，而不是直接访问：

```c
// inc/app_display.h
#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include "dev_oled.h"

// 初始化显示模块
void app_display_init(OLED_Object_t *oled);

// 更新显示（传入数据，而不是直接访问传感器）
void app_display_update(OLED_Object_t *oled, float temperature);

#endif
```

```c
// src/app_display.c
#include "app_display.h"
#include "dev_oled.h"
#include <stdio.h>

void app_display_init(OLED_Object_t *oled)
{
    OLED_Clear(oled);
    OLED_DisplayString(oled, 0, 0, "System Ready");
    OLED_Update(oled);
}

void app_display_update(OLED_Object_t *oled, float temperature)
{
    char buffer[32];

    OLED_Clear(oled);

    sprintf(buffer, "Temp: %.1f C", temperature);
    OLED_DisplayString(oled, 0, 0, buffer);

    sprintf(buffer, "Raw: %lu", (uint32_t)temperature);
    OLED_DisplayString(oled, 0, 16, buffer);

    OLED_Update(oled);
}
```

### 步骤 5：状态机设计（复杂应用）
对于复杂的应用，使用状态机管理：

```c
// inc/app_menu.h
#ifndef APP_MENU_H
#define APP_MENU_H

typedef enum
{
    MENU_STATE_IDLE,
    MENU_STATE_SCAN,
    MENU_STATE_CONNECT,
    MENU_STATE_CONFIG,
} menu_state_t;

typedef struct
{
    menu_state_t state;
    uint8_t current_item;
} Menu_t;

void app_menu_init(Menu_t *menu);
void app_menu_process(Menu_t *menu, uint8_t key);
menu_state_t app_menu_get_state(const Menu_t *menu);

#endif
```

```c
// src/app_menu.c
#include "app_menu.h"

void app_menu_init(Menu_t *menu)
{
    menu->state = MENU_STATE_IDLE;
    menu->current_item = 0;
}

void app_menu_process(Menu_t *menu, uint8_t key)
{
    switch(menu->state)
    {
        case MENU_STATE_IDLE:
            if (key == KEY_OK)
            {
                menu->state = MENU_STATE_SCAN;
            }
            break;

        case MENU_STATE_SCAN:
            if (key == KEY_BACK)
            {
                menu->state = MENU_STATE_IDLE;
            } else if (key == KEY_OK)
            {
                menu->state = MENU_STATE_CONNECT;
            }
            break;

        case MENU_STATE_CONNECT:
            // 连接逻辑
            if (key == KEY_BACK)
            {
                menu->state = MENU_STATE_SCAN;
            }
            break;

        default:
            menu->state = MENU_STATE_IDLE;
            break;
    }
}
```

## 5. 常见问题

**Q: ek_main 和 main 有什么区别？**

A: `main()` 是 C 语言的标准入口，由 L0_MCU 层实现，负责硬件初始化（时钟、中断等）。`ek_main()` 是本项目自定义的应用层入口，由 `main()` 调用，专注于业务逻辑。这样分离后，业务代码与硬件初始化解耦。

**Q: 全局变量应该放在哪里？**

A: 尽量避免使用全局变量。如果必须使用，建议：
- 模块内部的全局变量使用 `static` 修饰（私有化）
- 跨模块共享的变量定义在对应模块的 `.c` 文件中，通过 getter/setter 函数访问

**Q: 如何处理多个任务的调度？**

A: 如果项目使用 FreeRTOS（L3 层），每个模块可以创建独立的任务：

```c
void sensor_task(void *param)
{
    while(1)
     {
        app_sensor_read();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void display_task(void *param)
{
    while(1)
    {
        app_display_update();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int ek_main(void)
{
    app_init();

    xTaskCreate(sensor_task, "Sensor", 256, NULL, 2, NULL);
    xTaskCreate(display_task, "Display", 256, NULL, 1, NULL);

    vTaskStartScheduler();
    return 0;
}
```

如果不使用 RTOS，则在 `app_loop()` 中手动调度各模块。

**Q: L5 层可以直接调用 L1_HAL 吗？**

A: 不建议。L5 层应该通过 L4 组件访问硬件，而不是直接调用 L1。这样保持层次清晰，便于维护和测试。例如，需要控制 LED 时，应该使用 L4 的 `dev_led` 组件，而不是直接调用 `hal_gpio_write`。
