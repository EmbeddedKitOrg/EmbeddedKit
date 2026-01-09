/**
 * @file lv_conf.h
 * Configuration file for LVGL v9.x
 *
 * EmbeddedKit 项目定制配置
 * 目标平台: STM32F429VGT6
 * 显示: RGB565 (16位色深)
 */

/* clang-format off */
#if 1 /*Set it to "1" to enable content*/

#ifndef LV_CONF_H
#define LV_CONF_H

/* 注意: EK_USE_RTOS 由 L2_Core 的 ek_def.h 定义
 * 由于 LVGL 配置文件需要在更早阶段被包含，
 * 这里直接使用宏判断，依赖项目构建配置传递 */

/*====================
   COLOR SETTINGS
 *====================*/

/*Color depth: 1 (I1), 8 (L8), 16 (RGB565), 24 (RGB888), 32 (XRGB8888)*/
#define LV_COLOR_DEPTH 16

/*=========================
   STDLIB WRAPPER SETTINGS
 *=========================*/

/* 使用内置内存管理器 */
#define LV_USE_STDLIB_MALLOC    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_STRING    LV_STDLIB_BUILTIN
#define LV_USE_STDLIB_SPRINTF   LV_STDLIB_BUILTIN

#define LV_STDINT_INCLUDE       <stdint.h>
#define LV_STDDEF_INCLUDE       <stddef.h>
#define LV_STDBOOL_INCLUDE      <stdbool.h>
#define LV_INTTYPES_INCLUDE     <inttypes.h>
#define LV_LIMITS_INCLUDE       <limits.h>
#define LV_STDARG_INCLUDE       <stdarg.h>

#if LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN
    /*Size of the memory available for `lv_malloc()` in bytes (>= 2kB)*/
    #define LV_MEM_SIZE (128 * 1024U)          /*[bytes]*/

    /*Size of the memory expand for `lv_malloc()` in bytes*/
    #define LV_MEM_POOL_EXPAND_SIZE 0

    /*Set an address for the memory pool instead of allocating it as a normal array. Can be in external SRAM too.*/
    #define LV_MEM_ADR 0     /*0: unused*/
    /*Instead of an address give a memory allocator that will be called to get a memory pool for LVGL. E.g. my_malloc*/
    #if LV_MEM_ADR == 0
        #undef LV_MEM_POOL_INCLUDE
        #undef LV_MEM_POOL_ALLOC
    #endif
#endif  /*LV_USE_STDLIB_MALLOC == LV_STDLIB_BUILTIN*/

/*====================
   HAL SETTINGS
 *====================*/

/*Default display refresh, input device read and animation step period.*/
#define LV_DEF_REFR_PERIOD  33      /*[ms]*/

/*Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 *(Not so important, you can adjust it to modify default sizes and spaces)*/
#define LV_DPI_DEF 130     /*[px/inch]*/

/*=================
 * OPERATING SYSTEM
 *=================*/
/*操作系统选择 - 可通过 CMake 的 LVGL_USE_FREERTOS 变量配置*/
#if defined(LVGL_USE_FREERTOS) && (LVGL_USE_FREERTOS == 1)
    #define LV_USE_OS   LV_OS_FREERTOS
    #define LV_USE_FREERTOS_TASK_NOTIFY 1
#else
    #define LV_USE_OS   LV_OS_NONE
#endif

/*========================
 * RENDERING CONFIGURATION
 *========================*/

/*Align the stride of all layers and images to this bytes*/
#define LV_DRAW_BUF_STRIDE_ALIGN                1

/*Align the start address of draw_buf addresses to this bytes*/
#define LV_DRAW_BUF_ALIGN                       4

/*Enable matrix support for transformations*/
#define LV_USE_FLOAT                            1
#define LV_USE_MATRIX                           1

/*Using matrix for transformations.
 *Requirements:
    `LV_USE_MATRIX = 1`.
    The rendering engine needs to support 3x3 matrix transformations.*/
#define LV_DRAW_TRANSFORM_USE_MATRIX            1

/*The target buffer size for simple layer chunks.*/
#define LV_DRAW_LAYER_SIMPLE_BUF_SIZE    (24 * 1024)   /*[bytes]*/

/* The stack size of the drawing thread.
 * NOTE: If FreeType or ThorVG is enabled, it is recommended to set it to 32KB or more.
 */
#define LV_DRAW_THREAD_STACK_SIZE    (32 * 1024)   /*[bytes] - 增大以支持 ThorVG*/

#define LV_USE_DRAW_SW 1
#if LV_USE_DRAW_SW == 1
	#define LV_DRAW_SW_SUPPORT_RGB565		1
	#define LV_DRAW_SW_SUPPORT_RGB565A8		1
	#define LV_DRAW_SW_SUPPORT_RGB888		1
	#define LV_DRAW_SW_SUPPORT_XRGB8888		1
	#define LV_DRAW_SW_SUPPORT_ARGB8888		1
	#define LV_DRAW_SW_SUPPORT_L8			1
	#define LV_DRAW_SW_SUPPORT_AL88			1
	#define LV_DRAW_SW_SUPPORT_A8			1
	#define LV_DRAW_SW_SUPPORT_I1			1

    #define LV_DRAW_SW_DRAW_UNIT_CNT    1

    /* Use Arm-2D to accelerate the sw render */
    #define LV_USE_DRAW_ARM2D_SYNC      0

    /* Enable native helium assembly to be compiled */
    #define LV_USE_NATIVE_HELIUM_ASM    0

    /* 0: use a simple renderer capable of drawing only simple rectangles with gradient, images, texts, and straight lines only
     * 1: use a complex renderer capable of drawing rounded corners, shadow, skew lines, and arcs too */
    #define LV_DRAW_SW_COMPLEX          1

    #if LV_DRAW_SW_COMPLEX == 1
        #define LV_DRAW_SW_SHADOW_CACHE_SIZE 0
        #define LV_DRAW_SW_CIRCLE_CACHE_SIZE 4
    #endif

    #define  LV_USE_DRAW_SW_ASM     LV_DRAW_SW_ASM_NONE

    /* Enable drawing complex gradients in software: linear at an angle, radial or conical */
    #define LV_USE_DRAW_SW_COMPLEX_GRADIENTS    0
#endif

/* GPU 加速 - STM32F429 不支持 */
#define LV_USE_DRAW_VGLITE 0
#define LV_USE_DRAW_PPG 0
#define LV_USE_DRAW_SDL 0
#define LV_USE_DRAW_OPENGLES 0

/*=================
 * WIDGET USAGE
 *====================*/

/* 基础控件 */
#define LV_USE_ANIMIMAGE    1
#define LV_USE_ARC          1
#define LV_USE_BAR          1
#define LV_USE_BUTTON       1
#define LV_USE_BUTTONMATRIX 1
#define LV_USE_CALENDAR     1
#define LV_USE_CANVAS       1
#define LV_USE_CHART        1
#define LV_USE_CHECKBOX     1
#define LV_USE_DROPDOWN     1
#define LV_USE_IMAGE        1
#define LV_USE_IMAGEBUTTON  1
#define LV_USE_KEYBOARD     1
#define LV_USE_LABEL        1
#define LV_USE_LED          1
#define LV_USE_LINE         1
#define LV_USE_LIST         1
#define LV_USE_LOTTIE       1
#define LV_USE_MENU         1
#define LV_USE_MSGBOX       1
#define LV_USE_ROLLER       1
#define LV_USE_SCALE        1
#define LV_USE_SLIDER       1
#define LV_USE_SPAN         1
#define LV_USE_SPINBOX      1
#define LV_USE_SPINNER      1
#define LV_USE_SWITCH       1
#define LV_USE_TABLE        1
#define LV_USE_TABVIEW      1
#define LV_USE_TEXTAREA     1
#define LV_USE_TILEVIEW     1
#define LV_USE_WIN          1

/*==================
 * LAYOUTS
 *==================*/

#define LV_USE_FLEX 1
#define LV_USE_GRID 1

/*==================
 * THEMES
 *==================*/

#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    #define LV_THEME_DEFAULT_DARK 0
#endif

#define LV_USE_THEME_SIMPLE 1

/*==================
 * OTHERS
 *==================*/

/*1: Enable API to take snapshot for object*/
#define LV_USE_SNAPSHOT 1

/*1: Enable system monitor component*/
#define LV_USE_SYSMON   1
#if LV_USE_SYSMON
    /*1: Show CPU usage and FPS count*/
    #define LV_USE_PERF_MONITOR 0
    /*1: Show the used memory and the memory fragmentation*/
    #define LV_USE_MEM_MONITOR 0
#endif

/*1: Enable the runtime performance profiler*/
#define LV_USE_PROFILER 0

/*1: Enable monkey test*/
#define LV_USE_MONKEY 0

/*1: Enable grid navigation*/
#define LV_USE_GRIDNAV 0

/*1: Enable lv_obj fragment*/
#define LV_USE_FRAGMENT 0

/*1: Support using images as font in label or span widgets */
#define LV_USE_IMGFONT 0

/*1: Enable an observer pattern implementation and API*/
#define LV_USE_OBSERVER 1

/*1: Enable Pinyin input method*/
#define LV_USE_IME_PINYIN 0

/*1: Enable file explorer*/
#define LV_USE_FILE_EXPLORER 0

/*==================
 * LIBRARIES
 *==================*/

/*PNG decoder library*/
#define LV_USE_PNG 1

/*BMP decoder library*/
#define LV_USE_BMP 1

/*JPG + split JPG decoder library*/
#define LV_USE_SJPG 0

/*GIF decoder library*/
#define LV_USE_GIF 0

/*QR code library*/
#define LV_USE_QRCODE 1

/*Barcode code library*/
#define LV_USE_BARCODE 0

/*FreeType library*/
#define LV_USE_FREETYPE 0

/* Built-in TTF decoder */
#define LV_USE_TINY_TTF 0

/*Rlottie library*/
#define LV_USE_RLOTTIE 0

/*Enable Vector Graphic APIs
 *Requires `LV_USE_MATRIX = 1`*/
#define LV_USE_VECTOR_GRAPHIC  1

/* Enable ThorVG (vector graphics library) from the src/libs folder
 * 由 CMake 变量 USE_LVGL_THORVG 控制
 * 如果定义了 LV_CONF_BUILD_DISABLE_THORVG_INTERNAL 则禁用 */
#if defined(LV_CONF_BUILD_DISABLE_THORVG_INTERNAL) && (LV_CONF_BUILD_DISABLE_THORVG_INTERNAL == 1)
    #define LV_USE_THORVG_INTERNAL 0
#else
    #define LV_USE_THORVG_INTERNAL 1
#endif

/*FFmpeg library for image decoding and playing videos*/
#define LV_USE_FFMPEG 0

/*==================
 * FONT SUPPORT
 *==================*/

/*Montserrat fonts with various sizes and character sets*/
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 0
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Demonstrate special features*/
#define LV_FONT_MONTSERRAT_28_COMPRESSED 0  /* [12KB] compressed pixels */
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0  /* [21KB] Hebrew, Arabic, Perisan letters */
#define LV_FONT_SIMSUN_16_CJK            0  /* [2MB] CJK characters */

/*==================
 * TEXT ENCODING
 *==================*/

/*Select a character encoding.
 * - LV_TEXT_ENC_UTF8
 * - LV_TEXT_ENC_ASCII
 * */
#define LV_TEXT_ENC LV_TEXT_ENC_UTF8

/*==================
 * BIDI SETTINGS
 *==================*/

/*Configure the supported Bidirectional languages.
 * - LV_BASE_DIR_LTR: Left-to-Right language
 * - LV_BASE_DIR_RTL: Right-to-Left language
 * - LV_BASE_DIR_AUTO: Detects the direction automatically
 * */
#define LV_BASE_DIR_LTR LV_DIR_BASE_DIR_LTR

/*==================
 * USE CUSTOM CONFIG HEADER
 *==================*/

/* Include custom configuration header if needed */
/* #define LV_CONF_PATH "path/to/custom/lv_conf.h" */

/*==================
 * END OF CONFIGURATION
 *==================*/

#endif /*LV_CONF_H*/

#endif /*End of "Content enable"*/
