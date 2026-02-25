# cmake/EkLinkerScript.cmake
# 根据 MCU 内存参数生成链接脚本的辅助函数
#
# 必选变量（通过 MCU 子目录 CMakeLists.txt 的 PARENT_SCOPE 设置）：
#   EK_FLASH_ORIGIN / EK_FLASH_LENGTH    - Flash 起始地址和大小
#   EK_RAM_ORIGIN   / EK_RAM_LENGTH      - RAM 起始地址和大小
#
# 可选变量（不设置则对应内存区域和段完全省略）：
#   EK_CCMRAM_ORIGIN / EK_CCMRAM_LENGTH  - CCM-RAM 起始地址和大小
#   EK_SDRAM1_ORIGIN / EK_SDRAM1_LENGTH  - SDRAM1 起始地址和大小
#   EK_SDRAM2_ORIGIN / EK_SDRAM2_LENGTH  - SDRAM2 起始地址和大小
#
# 可选变量（有默认值）：
#   EK_MIN_HEAP_SIZE  - 最小堆大小（默认 0x200）
#   EK_MIN_STACK_SIZE - 最小栈大小（默认 0x400）

function(ek_configure_linker_script)

    # --- 必选参数检查 ---
    foreach(_var
        EK_FLASH_ORIGIN  EK_FLASH_LENGTH
        EK_RAM_ORIGIN    EK_RAM_LENGTH
    )
        if(NOT DEFINED ${_var})
            message(FATAL_ERROR
                "ek_configure_linker_script: ${_var} is not set.\n"
                "Please set it in ${MCU_MODEL}/CMakeLists.txt with PARENT_SCOPE.")
        endif()
    endforeach()

    # --- 可选参数默认值 ---
    if(NOT DEFINED EK_MIN_HEAP_SIZE)
        set(EK_MIN_HEAP_SIZE "0x200")
    endif()
    if(NOT DEFINED EK_MIN_STACK_SIZE)
        set(EK_MIN_STACK_SIZE "0x400")
    endif()

    # --- CCMRAM 可选区域 ---
    if(DEFINED EK_CCMRAM_ORIGIN AND DEFINED EK_CCMRAM_LENGTH)
        set(EK_LD_CCMRAM_REGION
            "CCMRAM (xrw) : ORIGIN = ${EK_CCMRAM_ORIGIN}, LENGTH = ${EK_CCMRAM_LENGTH}")
        set(EK_LD_CCMRAM_SECTION
"  _siccmram = LOADADDR(.ccmram);

  /* CCM-RAM section
  *
  * IMPORTANT NOTE!
  * If initialized variables will be placed in this section,
  * the startup code needs to be modified to copy the init-values.
  */
  .ccmram :
  {
    . = ALIGN(4);
    _sccmram = .;       /* create a global symbol at ccmram start */
    *(.ccmram)
    *(.ccmram*)
    *(.tcmram)          /* GD32 TCM-RAM alias */
    *(.tcmram*)

    . = ALIGN(4);
    _eccmram = .;       /* create a global symbol at ccmram end */
  } >CCMRAM AT> FLASH
")
    else()
        set(EK_LD_CCMRAM_REGION "")
        set(EK_LD_CCMRAM_SECTION "")
    endif()
    if(DEFINED EK_SDRAM1_ORIGIN AND DEFINED EK_SDRAM1_LENGTH)
        set(EK_LD_SDRAM1_REGION
            "SDRAM  (xrw) : ORIGIN = ${EK_SDRAM1_ORIGIN}, LENGTH = ${EK_SDRAM1_LENGTH}")
        set(EK_LD_SDRAM1_SECTION
"  /* SDRAM1 数据段（需要外部 SDRAM 初始化后才可用） */
  .sdram1_data (NOLOAD) :
  {
    . = ALIGN(4);
    _sdram1_data_start = .;
    *(.sdram1_data)
    *(.sdram1_data*)
    . = ALIGN(4);
    _sdram1_data_end = .;
  } >SDRAM
")
    else()
        set(EK_LD_SDRAM1_REGION "")
        set(EK_LD_SDRAM1_SECTION "")
    endif()

    # --- SDRAM2 可选区域 ---
    if(DEFINED EK_SDRAM2_ORIGIN AND DEFINED EK_SDRAM2_LENGTH)
        set(EK_LD_SDRAM2_REGION
            "SDRAM2 (xrw) : ORIGIN = ${EK_SDRAM2_ORIGIN}, LENGTH = ${EK_SDRAM2_LENGTH}")
        set(EK_LD_SDRAM2_SECTION
"  /* SDRAM2 数据段 */
  .sdram2_data (NOLOAD) :
  {
    . = ALIGN(4);
    _sdram2_data_start = .;
    *(.sdram2_data)
    *(.sdram2_data*)
    . = ALIGN(4);
    _sdram2_data_end = .;
  } >SDRAM2
")
    else()
        set(EK_LD_SDRAM2_REGION "")
        set(EK_LD_SDRAM2_SECTION "")
    endif()

    # --- 生成链接脚本 ---
    set(_output "${CMAKE_BINARY_DIR}/generated_${MCU_MODEL}.ld")

    configure_file(
        "${CMAKE_SOURCE_DIR}/cmake/ld/ek_generic.ld.in"
        "${_output}"
        @ONLY
    )

    set(EK_GENERATED_LINKER_SCRIPT "${_output}" PARENT_SCOPE)
    message(STATUS "Generated linker script: ${_output}")

endfunction()
