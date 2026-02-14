# EmbeddedKit (EK)

A professional 6-layer embedded firmware framework designed for maximum modularity and hardware portability. Uses a CMake-based build system with support for multiple toolchains.

[中文版](README.md)

---

## Architecture

EmbeddedKit follows a strict bottom-up dependency model where upper layers depend only on lower layers. Circular dependencies are strictly prohibited.

```
L5_App (Application Layer)
  └─> Business logic, state machines
L4_Components (Device Driver Component Layer)
  └─> Hardware drivers (OLED, Flash, sensors) - Using OOP pattern
L3_Middlewares (Third-party Middleware Layer)
  └─> FreeRTOS, FatFs, LVGL, etc.
L2_Core (Core & Hardware Abstraction Layer)
  ├─> utils/ (Data structures, memory management, logging)
  └─> hal/ (Hardware abstraction: GPIO/UART/I2C/etc.)
L1_MCU (MCU Vendor Library Layer)
  └─> Chip-specific vendor code (STM32 HAL, CMSIS)
L0_Assets (Resource File Layer)
  └─> Static resource data (images, fonts, configs, etc.)
```

### Key Architectural Rules

- **L0_Assets (Lowest Layer)**: Stores static resource data embedded at compile time (images, fonts, configs, etc.). Has no dependencies on other layers and can be accessed by any layer.

- **L1_MCU (MCU Vendor Library Layer)**: Each MCU model has an independent subdirectory containing vendor HAL/LL libraries, startup code, and `main.c`. The `main.c` calls `ek_main()` from L5_App after hardware initialization. No sharing of vendor libraries across different MCUs.

- **L2_Core**:
  - **utils/**: Pure software implementations - data structures, memory management, logging
  - **hal/**: Hardware abstraction layer providing logical-to-physical mapping
    - `inc/*.h` must NOT include vendor headers (stm32xxxx.h)
    - Vendor headers only allowed in `src/*.c`
    - All functions use `hal_` prefix (e.g., `hal_gpio_write()`)
  - Internal dependency: hal/ may depend on utils/ (allowed within same layer)
  - Naming convention: `ek_` prefix (utils) and `hal_` prefix (hal)

- **L3_Middlewares**: Each middleware has an independent subdirectory with its own CMakeLists.txt. Can depend on L2_Core/hal for hardware adaptation.

- **L4_Components**: **Strict OOP Pattern** - Use function pointers for polymorphism. The first parameter of methods must be the object pointer (`self`). Define abstract interfaces to implement dependency inversion for hardware. **Implemented by users based on actual hardware.**

- **L5_App**: Implements `ek_main()` as the application entry point. Should call L4 components, not L1 directly. Encapsulate business logic into modules.

---

## Features

- **6-Layer Architecture**: Clear separation of concerns with strict dependency rules
- **OBJECT Library Build Mode**: Avoids static library selective linking issues
- **Hardware Portability**: Replace MCU by only changing the L1 layer
- **OOP Design Pattern**: Interface abstraction in L4 for dependency inversion
- **Flexible Build System**: Support for multiple toolchains (GCC, ARM Compiler 6, Clang)
- **Conditional Compilation**: Enable/disable features via CMake options
- **Rich Data Structures**: Linked list, ring buffer, stack, dynamic vector (included)
- **Memory Management**: TLSF-based dynamic memory allocator
- **Logging System**: Multi-level logging with color support
- **RTOS Support**: FreeRTOS integration ready
- **Resource Management**: Independent resource file layer supporting static data embedding

---

## Quick Start

### Prerequisites

- CMake 3.20 or higher
- ARM toolchain (gcc-arm-none-eabi or STARM Clang)
- STM32CubeMX or GD32 Eclipse (for generating initialization code)
- [just](https://github.com/casey/just) (optional, provides simpler build commands)

### Build

**Method 1: Using just (Recommended)**

```bash
# Build STM32F407VGT6 (GCC ARM)
just build

# Build GD32F470ZGT6 (GCC ARM)
just build-gd

# Build STM32F429ZIT6 (STARM Clang)
just build-starm

# Clean build directory
just clean

# Run unit tests
just test
```

**Method 2: Using CMake**

```bash
# Configure build (select MCU model via cache variable)
cmake -B build -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake \
  -DMCU_MODEL=STM32F407VGT6_GCC \
  -DUSE_FREERTOS=OFF \
  -DUSE_FATFS=OFF \
  -DUSE_LVGL=OFF

# Build
cmake --build build
```

### Available Toolchains

- `cmake/gcc-arm-none-eabi.cmake` - GCC ARM toolchain
- `cmake/starm-clang.cmake` - STARM Clang toolchain

**Note**: If you need to use ARM Compiler 6, please write your own toolchain file.

---

## Directory Structure

```
EmbeddedKit/
├── CMakeLists.txt              # Main build script
├── ek_conf.h                   # Global configuration file
├── CLAUDE.md                   # Project documentation and changelog
├── LICENSE                     # MIT License
├── cmake/                      # Toolchain files
│   ├── gcc-arm-none-eabi.cmake
│   ├── arm-compile6.cmake
│   └── starm-clang.cmake
├── L0_Assets/                  # Layer 0: Resource files
├── L1_MCU/                     # Layer 1: MCU vendor libraries
│   └── STM32F429ZIT6_GCC/      # STM32F429 support
├── L2_Core/                    # Layer 2: Core & HAL
│   ├── utils/                  # Pure software utilities
│   ├── hal/                    # Hardware abstraction layer
│   └── third_party/            # Third-party libraries
├── L3_Middlewares/             # Layer 3: Middleware
│   ├── FreeRTOS/               # Real-time operating system
│   ├── FatFS/                  # File system
│   └── LVGL/                   # Graphics library
├── L4_Components/              # Layer 4: Device drivers (OOP)
└── L5_App/                     # Layer 5: Application logic
```

---

## Layers Description

### L0_Assets (Resource File Layer)

Stores static resource data embedded at compile time, with no dependencies on other layers.

**Purpose**: Images, fonts, configuration data, and other static resources

**Features**:
- Resources provided as C arrays or const data
- Directly linked to final firmware
- Accessible by any layer

### L1_MCU (MCU Vendor Library Layer)

Contains vendor-provided official libraries and startup code. Each MCU model has an independent subdirectory. The `main.c` is responsible for calling `ek_main()` after hardware initialization.

**Currently Supported**: STM32F407VGT6_GCC, STM32F429ZIT6_STARM, GD32F470ZGT6

**Build Mode**: OBJECT library, linked via `$<TARGET_OBJECTS:l1_mcu>`

### L2_Core (Core & Hardware Abstraction Layer)

**utils/ (Hardware-independent)**:
- `ek_def.h` - Common definitions and cross-compiler macros
- `ek_list.h` - Doubly linked list
- `ek_ringbuf.h` - Ring buffer with element counting
- `ek_stack.h` - Stack data structure
- `ek_vec.h` - Dynamic vector (array)
- `ek_mem.h` - TLSF-based dynamic memory management
- `ek_log.h` - Multi-level logging system
- `ek_assert.h` - Assertion module
- `ek_export.h` - Export macros (auto-initialization)
- `ek_io.h` - IO module (based on lwprintf)
- `ek_shell.h` - Shell module
- `ek_str.h` - String processing utilities

**hal/ (Hardware Abstraction)**: Being implemented. Will provide logical-to-physical mapping (e.g., `HAL_GPIO_1` → `GPIOA PIN_5`).
- GPIO, UART, SPI, I2C, Tick

**third_party/**:
- `lwprintf` - Lightweight formatted output library
- `tlsf` - Two-Level Segregated Fit memory allocator

### L3_Middlewares (Third-party Middleware Layer)

Each middleware has an independent subdirectory and CMakeLists.txt.

**Integrated Middleware**:

1. **FreeRTOS** - Real-time operating system (✅ Fully Supported)
   - Port: GCC_ARM_CM4F (for STM32F429)
   - Heap implementation: heap_4.c
   - Configuration: 168MHz CPU, 1000Hz Tick Rate, 32KB heap
   - Enable via: `-DUSE_FREERTOS=ON`

2. **FatFS** - File system (✅ Fully Supported)
   - Enable via: `-DUSE_FATFS=ON`

3. **LVGL** - Lightweight graphics library (✅ Fully Supported)
   - Version: v8.3.11
   - Configuration: RGB565, 128KB memory pool
   - Includes: Official examples and demos
   - Enable via: `-DUSE_LVGL=ON`
   - **Note**: LVGL links to L1_MCU for direct hardware access

### L4_Components (Hardware Driver Component Layer)

**Core Design - OOP Pattern**:
- Interface abstraction using function pointers
- Object encapsulation with properties and methods
- Dependency inversion - components depend on abstract interfaces
- Polymorphism support - same component for multiple hardware types

**Implementation**: **Implemented by users based on actual hardware**

### L5_App (Application Layer)

Implements `ek_main()` as the application entry point. Should call L2~L4 layer services, not L1 directly.

**Current Implementation**:
```c
void ek_main(void)
{
    ek_heap_init();  // Initialize memory heap

    while (1)
    {
        // Business logic
    }
}
```

---

## Configuration

The global configuration file `ek_conf.h` in the root directory manages framework-wide settings:

```c
// RTOS Configuration
#define EK_USE_RTOS (1)

// Memory Management
#define EK_HEAP_NO_TLSF (0)
#define EK_HEAP_SIZE    (30 * 1024)

// IO Lib Management
#define EK_IO_NO_LWPRTF   (0)


// Module Enable/Disable
#define EK_EXPORT_ENABLE  (0)
#define EK_STR_ENABLE     (1)
#define EK_LOG_ENABLE     (1)
#define EK_LIST_ENABLE    (1)
#define EK_VEC_ENABLE     (1)
#define EK_RINGBUF_ENABLE (1)
#define EK_STACK_ENABLE   (1)
#define EK_SHELL_ENABLE   (1)

// Logging Configuration
#define EK_LOG_DEBUG_ENABLE (1)
#define EK_LOG_COLOR_ENABLE (1)
#define EK_LOG_BUFFER_SIZE  (256)

// Assertion Configuration
#define EK_ASSERT_USE_TINY (1)
#define EK_ASSERT_WITH_LOG (1)
```

---

## Build System

### OBJECT Library Architecture

This project uses **OBJECT Library** mode instead of traditional static libraries, offering the following advantages:

**Why use OBJECT libraries?**

Traditional static libraries have selective linking issues:
- Linker only extracts object files that resolve current undefined references
- "Skipped" symbols are not included in the final firmware
- Requires complex options like `--whole-archive`, `--undefined` to force symbol inclusion

OBJECT library advantages:
- Object files participate directly in final linking, no selective linking issues
- No need for `--whole-archive`, `--undefined` linker options
- Simpler build system, more reliable symbol resolution

**CMake Implementation:**

```cmake
# Define each layer as OBJECT library
add_library(l0_assets OBJECT)
add_library(l1_mcu OBJECT)
add_library(l2_core OBJECT)
add_library(l4_components OBJECT)
add_library(l5_app OBJECT)

# L3 remains INTERFACE library (aggregation layer)
add_library(l3_middlewares INTERFACE)

# Final linking uses $<TARGET_OBJECTS:>
target_link_libraries(${CMAKE_PROJECT_NAME}
    $<TARGET_OBJECTS:l5_app>
    $<TARGET_OBJECTS:l4_components>
    l3_middlewares                      # INTERFACE library links normally
    $<TARGET_OBJECTS:l2_core>
    $<TARGET_OBJECTS:l1_mcu>
    $<TARGET_OBJECTS:l0_assets>
)
```

---

## Project Status

### Completed
- ✅ Complete 6-layer architecture design
- ✅ CMake build system configuration
- ✅ just command-line tool support
- ✅ L0_Assets: Resource file layer framework ready
- ✅ L1_MCU: STM32F407VGT6_GCC, STM32F429ZIT6_STARM, GD32F470ZGT6 fully supported
- ✅ L2_Core/utils: All utility modules implemented (12 modules including export, io, shell, str)
- ✅ L2_Core/third_party: lwprintf and tlsf integrated
- ✅ L2_Core/hal: GPIO, UART, SPI, I2C, Tick being implemented
- ✅ L3_Middlewares: FreeRTOS, FatFS, LVGL v8.3.11 fully supported (includes examples and demos)
- ✅ L5_App: Basic entry implementation
- ✅ Global configuration file `ek_conf.h`
- ✅ Detailed documentation

### To Be Implemented
- ⏳ L2_Core/hal: Complete remaining peripherals (DAC, ADC, RTC, etc.)
- ⏳ L4_Components: Device driver components implemented by users based on actual hardware
- ⏳ L5_App: Specific business logic applications

---

## Contributing

Contributions are welcome! Please follow the architectural rules and coding standards described in this document.

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.	
