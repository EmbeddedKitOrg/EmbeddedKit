# EK_TasksRun - 自动任务注册与执行模块文档

## 📖 文档说明

欢迎使用 `EK_TasksRun` 模块！本文档旨在帮助您理解其核心设计思想，并指导您如何将其无缝集成到您的嵌入式项目中，以实现高效、解耦的模块化开发。

* ### ✨ 核心优势：传统方式 vs EK_TasksRun

  `EK_TasksRun` 的核心价值在于**自动化**和**解耦**。下面通过一个具体的协程任务创建场景，来直观展示其优势。

  ### 传统开发模式 (高耦合)

  在传统模式下，`main` 函数需要了解所有需要创建的协程任务，并手动调用它们的创建函数 `EK_pCoroCreate`。

  ```c
  // main.c
  #include "sys.h"
  #include "Kernel.h"
  #include "EK_CoroTask.h"
  
  // 任务1 和 任务2 的具体实现...
  void Task1(void* arg){/*...*/}
  void Task2(void* arg){/*...*/}
  
  int main(void)
  {
  	// 初始化
  	DebugInit();
  	EK_vKernelInit();
  
  	// 手动创建每一个任务
  	// main函数必须知道Task1和Task2的存在
  	EK_pCoroCreate(Task1, NULL, 0, 1024);
  	EK_pCoroCreate(Task2, NULL, 1, 1024);
  
  	// 启动调度器
  	EK_vKernelStart();
  
  	while (1) {}
  }
  ```

  - **缺点**：`main` 函数与 `Task1`、`Task2` 等具体任务紧密耦合。如果想新增一个 `Task3` 或者禁用 `Task2`，都必须直接修改 `main.c` 文件，这在中大型项目中会变得难以维护。

  ### EK_TasksRun 模式 (低耦合)

  `EK_TasksRun` 允许将任务的创建逻辑封装在各自的模块中，`main` 函数只负责触发执行，无需关心任何具体任务的实现。

  **模块内部 (`Task_Example.c`)**

  ```c
  // Task_Example.c
  #include "Kernel.h"
  #include "EK_CoroTask.h"
  #include "EK_TasksRun.h"
  
  // 任务1 和 任务2 的具体实现...
  void Task1(void* arg){/*...*/}
  void Task2(void* arg){/*...*/}
  
  // 定义一个专门用于创建该模块任务的入口函数
  void Task_Entry(void)
  {
      EK_CoroHandler_t handler = NULL;
      handler = EK_pCoroCreate(Task1, NULL, 1, 256);
      if (handler == NULL) {
          printf("[Task1] Create Task1 failed!!!\r\n");
      }
  	// 将handler传递给Task2
      handler = EK_pCoroCreate(Task2, (void*)handler, 2, 256);
      if (handler == NULL) {
          printf("[Task2] Create Task2 failed!!!\r\n");
      }
  }
  
  // 模块自我注册任务创建入口，main函数对此完全无感知
  EK_vTaskRegister(Task_Entry);
  ```

  **主函数 (`main.c`)**

  C

  ```c
  // main.c
  #include "sys.h"
  #include "Kernel.h"
  #include "EK_CoroTask.h"
  #include "EK_TasksRun.h"
  
  int main(void)
  {
  	// 初始化
  	DebugInit();
  	EK_vKernelInit();
  
  	// 一键执行所有模块已注册的任务创建入口
  	EK_vTasksRun();
  
  	// 启动调度器
  	EK_vKernelStart();
  
  	while (1) {}
  }
  ```

  - **优点**：任务模块（如 `Task_Example.c`）可以作为独立的单元被添加到项目中或从项目中移除。`main` 函数始终保持干净、稳定，无需任何改动，真正实现了模块的“即插即用”，极大提升了项目的可维护性和扩展性。

---

## 🛠️ 移植与配置指南

`EK_TasksRun` 的实现依赖于 **链接器脚本** 。您必须正确配置它，否则模块将无法工作。

### 系统架构

本模块通过 `__attribute__((section(".EK_TaskEntry")))` 将所有任务函数的指针收集到同一个数据段。链接器负责将这些分散的指针聚合成一个连续的数组。`EK_vTasksRun` 函数则通过链接器生成的边界符号（如 `Image$$...$$Base/Limit`）来遍历这个数组。

### 关键配置 (ARM Compiler 6)

在您的 `.sct` 分散加载文件中，添加一个用于存放任务指针的执行域：

**代码段**

```plaintext
LR_IROM1 0x08000000 0x00010000 {
	ER_IROM1 0x08000000 0x00010000 {
		*.o (RESET, +First) 
		*(InRoot$$Sections) 
		.ANY (+RO) 
		.ANY (+XO) 
	}

	; ====> 将所有 .EK_TaskEntry 段聚合到这里 <====
    EK_TASK_ENTRIES +0
    {
        *(.EK_TaskEntry)
    }
    ; ===========================================

	RW_IRAM1 0x20000000 0x00005000 {
		.ANY (+RW +ZI) 
	}
}
```

### 兼容性与未来规划

* **当前已验证平台** :
* **工具链** : ARM Compiler 6 (AC6)
* **硬件** : STM32F1 系列
* **未来适配计划** :
* **ARM Compiler 5 (AC5)** : 验证 `.sct` 文件兼容性。
* **GCC for ARM** : 编写 `.ld` 链接脚本，并修改C代码以适配 GCC 。

---

## 📖 API 参考手册

### 数据类型

#### `EK_TaskEntry_t`

```c
typedef void (*EK_TaskEntry_t)(void);
```

**描述** : 定义了任务入口函数的标准原型。所有被注册的任务入口都必须是无参数、无返回值的函数。

### 宏定义

#### `EK_vTaskRegister(func)`

```c
#define EK_vTaskRegister(func) EK_TASK_ADD(func)
```

* **描述** : 注册一个任务入口函数，使其可以被 `EK_vTasksRun` 执行。这是模块最核心的 API。
* **参数** :
  * `func`: `[in]` 要注册的入口函数名，其类型必须符合 `EK_TaskEntry_t`。

* **使用示例** :

```c
  void Task_Entry(void) { /* ... */ }
  EK_vTaskRegister(Task_Entry);
```

### 函数

#### `EK_vTasksRun(void)`

```c
void EK_vTasksRun(void);
```

* **描述** : 执行所有已通过 `EK_vTaskRegister` 注册的任务入口。该函数通常在系统时钟等基础配置完成后，主循环开始前调用。
* **返回值** : 无。
* **注意** : 调用此函数前，必须确保链接器脚本已正确配置。

---

## 🐛 常见问题解答

**暂无**

---

## 🔄 版本信息

* **当前版本** : v1.0
* **更新日期** : 2025-10-22
* **编译器支持** : AC6
* **目标架构** : ARM Cortex-M

---

祝您使用愉快！🚀
