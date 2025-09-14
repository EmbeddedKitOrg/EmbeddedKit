# STM32F407VGT6 EmbeddedKit Project

This is an STM32F407VGT6 embedded project using the EmbeddedKit framework with CMake build system and ARM GCC toolchain.

## Build Commands

### Configure and Build
```bash
# Configure for Debug build
cmake --preset Debug

# Build Debug version
cmake --build build/Debug

# Configure and build Release version
cmake --preset Release
cmake --build build/Release
```

### Flash and Debug
```bash
# Flash firmware to target (requires probe-rs)
powershell -ExecutionPolicy Bypass -File download.ps1

# Alternative manual flash command
probe-rs download --chip STM32F407VG ./build/Debug/STM32F407VGT6_JLC_TKX.elf
probe-rs reset --chip STM32F407VG
```

## Project Architecture

### Core Structure
- **App/**: Application layer code (user application logic)
- **Bsp/**: Board Support Package (hardware abstraction layer)
- **EK_Component/**: EmbeddedKit framework components
- **Core/**: STM32 HAL and system initialization code
- **Drivers/**: STM32 peripheral drivers
- **Middlewares/**: Third-party middleware components

### EmbeddedKit Framework
The project uses a custom embedded framework (EK_Component) with these key components:

#### Memory Management
- Custom memory pool allocator (`EK_Component/MemPool/`)
- Configured with 4KB pool size and 8-byte alignment
- Use `EK_MALLOC()` and `EK_FREE()` macros instead of standard malloc/free

#### Data Structures
- **List**: Doubly-linked list implementation (`EK_Component/DataStruct/List/`)
- **Queue**: FIFO queue implementation (`EK_Component/DataStruct/Queue/`)
- **Stack**: LIFO stack implementation (`EK_Component/DataStruct/Stack/`)

#### Task Scheduler
- Cooperative task scheduler (`EK_Component/Task/`)
- Priority-based scheduling with timing control
- Supports both static and dynamic task creation

#### Common Utilities
- Unified error handling with `EK_Result_t` enum
- Cross-compiler compatibility macros (`__weak`, `__unused`)
- Memory, string, and bit manipulation utilities
- Mathematical functions and checksum calculations

### Configuration
- **EK_Config.h**: Framework configuration (memory pool size, timeouts, etc.)
- **EK_Common.h**: Global type definitions and utility functions
- **STM32F407VGT6_JLC_TKX.ioc**: STM32CubeMX configuration file

### Hardware Configuration
- Target: STM32F407VGT6 (Cortex-M4 with FPU)
- Hardware floating-point enabled (`-mfloat-abi=hard -mfpu=fpv4-sp-d16`)
- USART1 with DMA support configured
- 168MHz system clock with external crystal oscillators

## Development Notes

### Toolchain Requirements
- ARM GCC toolchain (`arm-none-eabi-gcc`)
- CMake 3.22+
- Ninja build system
- probe-rs for flashing and debugging

### Memory Layout
- Flash: STM32F407XX_FLASH.ld linker script
- RAM: Includes memory pool for dynamic allocation
- FPU support with printf float formatting enabled

### Code Style
- All EmbeddedKit functions prefixed with `EK_`
- Hungarian notation for function names (e.g., `EK_vMemCpy`, `EK_pStrCpy`)
- Comprehensive error handling using `EK_Result_t`