# =============================================================================
# 基础系统配置 (一般不用改)
# =============================================================================
set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)
set(CMAKE_C_COMPILER_ID             GNU)
set(CMAKE_CXX_COMPILER_ID           GNU)

# =============================================================================
# 工具链路径配置
# =============================================================================
# [需修改] 如果你的编译器没加环境变量，要在下面手动指定绝对路径
set(TOOLCHAIN_PREFIX                arm-none-eabi-)

set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

# 关键：跳过编译可执行文件测试，防止配置阶段报错
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)

# =============================================================================
# 链接脚本配置
# =============================================================================
#  定义变量 LINKER_SCRIPT
#    CACHE FILEPATH: 表示这是一个缓存变量，类型为文件路径，可以在命令行被 -D 覆盖
#    默认值: 设置为你原本的路径
set(LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/L1_MCU/STM32F429ZIT6/STM32F429XX_FLASH.ld"
    CACHE FILEPATH "The path to the linker script")

if(NOT CMAKE_SOURCE_DIR MATCHES "CMakeScratch")
    if(NOT EXISTS "${LINKER_SCRIPT}")
        message(WARNING "Linker script not found at: ${LINKER_SCRIPT}")
    endif()
endif()

# =============================================================================
# 芯片架构参数 (针对 STM32F429)
# =============================================================================
# [需修改] 核心参数：STM32F429 是 M4 核，带硬件浮点
# -mcpu=cortex-m4       : F4 系列核心
# -mfpu=fpv4-sp-d16     : F4 的浮点单元版本
# -mfloat-abi=hard      : 开启硬件浮点 (若用 FreeRTOS 需确保配置匹配)
set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard")

# =============================================================================
# 编译器选项 (Compiler Flags)
# =============================================================================
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
# -fdata-sections -ffunction-sections : 为每个函数生成独立段，方便最后删减体积
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

# 调试/发布模式预设
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

# C++ 特有优化 (禁用异常和RTTI，节省空间)
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

# =============================================================================
# 链接器选项 (Linker Flags)
# =============================================================================
set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")

# 指定链接脚本 (.ld) 的路径
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${LINKER_SCRIPT}\"")

# 使用 newlib-nano (嵌入式专用微型库)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")

# 生成 Map 文件并删除未使用的函数
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
# 打印内存占用 (Flash/RAM)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")

set(TOOLCHAIN_LINK_LIBRARIES "m")
