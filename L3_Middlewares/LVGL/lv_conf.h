/**
 * @file lv_conf.h
 * LVGL v8.3.11 配置文件
 */

#if 1 /*设置为"1"启用内容*/

#    ifndef LV_CONF_H
#        define LV_CONF_H

#        include <stdint.h>

/*====================
   颜色设置
 *====================*/

/*颜色深度：1（1字节/像素），8（RGB332），16（RGB565），32（ARGB8888）*/
#        define LV_COLOR_DEPTH 16

/*交换RGB565颜色的2个字节。如果显示器使用8位接口（如SPI）则有用*/
#        define LV_COLOR_16_SWAP 0

/*启用在透明背景上绘制功能
 *如果使用了透明度和transform_*样式属性，则需要启用
 *也可用于UI叠加在另一层之上，如OSD菜单或视频播放器*/
#        define LV_COLOR_SCREEN_TRANSP 0

/*调整颜色混合函数的舍入方式。GPU可能以不同方式计算颜色混合（混合）
 * 0: 向下取整，64: 从x.75向上取整，128: 从半值向上取整，192: 从x.25向上取整，254: 向上取整*/
#        define LV_COLOR_MIX_ROUND_OFS 0

/*具有此颜色的图像像素将不会被绘制（如果使用了色度键）*/
#        define LV_COLOR_CHROMA_KEY lv_color_hex(0x00ff00) /*纯绿色*/

/*=========================
   内存设置
 *=========================*/

/*1：使用自定义malloc/free，0：使用内置的`lv_mem_alloc()`和`lv_mem_free()`*/
#        define LV_MEM_CUSTOM 0
#        if LV_MEM_CUSTOM == 0
/*`lv_mem_alloc()`可用的内存大小（字节）（>= 2kB）*/
#            define LV_MEM_SIZE (48U * 1024U) /*[字节]*/

/*为内存池设置地址，而不是将其分配为普通数组。也可以在外部SRAM中。*/
#            define LV_MEM_ADR 0 /*0：未使用*/
/*代替地址，提供一个将被调用以获取LVGL内存池的内存分配器。例如my_malloc*/
#            if LV_MEM_ADR == 0
#                undef LV_MEM_POOL_INCLUDE
#                undef LV_MEM_POOL_ALLOC
#            endif

#        else /*LV_MEM_CUSTOM*/
#            define LV_MEM_CUSTOM_INCLUDE <stdlib.h> /*动态内存函数的头文件*/
#            define LV_MEM_CUSTOM_ALLOC   malloc
#            define LV_MEM_CUSTOM_FREE    free
#            define LV_MEM_CUSTOM_REALLOC realloc
#        endif /*LV_MEM_CUSTOM*/

/*渲染和其他内部处理机制中使用的中间内存缓冲区数量
 *如果没有足够的缓冲区，您将看到错误日志消息。*/
#        define LV_MEM_BUF_MAX_NUM 16

/*使用标准的`memcpy`和`memset`而不是LVGL自己的函数。（可能更快也可能不快）*/
#        define LV_MEMCPY_MEMSET_STD 0

/*====================
   HAL设置
 *====================*/

/*默认显示刷新周期。LVG将以此周期时间重绘更改区域*/
#        define LV_DISP_DEF_REFR_PERIOD 30 /*[毫秒]*/

/*输入设备读取周期（毫秒）*/
#        define LV_INDEV_DEF_READ_PERIOD 30 /*[毫秒]*/

/*使用自定义时钟源来告知经过的时间（毫秒）
 *它消除了手动使用`lv_tick_inc()`更新时钟的需要*/
#        define LV_TICK_CUSTOM 0
#        if LV_TICK_CUSTOM
#            define LV_TICK_CUSTOM_INCLUDE       "Arduino.h" /*系统时间函数的头文件*/
#            define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis()) /*计算当前系统时间的表达式（毫秒）*/
/*如果将lvgl用作ESP32组件*/
// #define LV_TICK_CUSTOM_INCLUDE "esp_timer.h"
// #define LV_TICK_CUSTOM_SYS_TIME_EXPR ((esp_timer_get_time() / 1000LL))
#        endif /*LV_TICK_CUSTOM*/

/*默认每英寸点数。用于初始化默认大小，如控件大小、样式内边距
 *（不是很重要，您可以通过修改它来调整默认大小和间距）*/
#        define LV_DPI_DEF 130 /*[像素/英寸]*/

/*=======================
 * 功能配置
 *=======================*/

/*-------------
 * 绘制
 *-----------*/

/*启用复杂绘制引擎
 *绘制阴影、渐变、圆角、圆形、弧线、斜线、图像变换或任何遮罩都需要*/
#        define LV_DRAW_COMPLEX 1
#        if LV_DRAW_COMPLEX != 0

/*允许缓冲一些阴影计算
     *LV_SHADOW_CACHE_SIZE是缓冲的最大阴影大小，其中阴影大小是`shadow_width + radius`
     *缓存的RAM成本是LV_SHADOW_CACHE_SIZE^2*/
#            define LV_SHADOW_CACHE_SIZE 0

/*设置最大缓存的圆形数据数量
     *保存1/4圆的周长用于抗锯齿
     *每个圆使用radius * 4字节（最常使用的半径被保存）
     * 0：禁用缓存*/
#            define LV_CIRCLE_CACHE_SIZE 4
#        endif /*LV_DRAW_COMPLEX*/

/**
 * "简单图层"用于当控件具有`style_opa < 255`时，将控件缓冲到图层中
 * 并以给定的不透明度将其作为图像混合
 * 注意`bg_opa`、`text_opa`等不需要缓冲到图层）
 * 控件可以分块缓冲到较小的缓冲区中以避免使用大缓冲区
 *
 * - LV_LAYER_SIMPLE_BUF_SIZE：[字节]最佳目标缓冲区大小。LVGL将尝试分配它
 * - LV_LAYER_SIMPLE_FALLBACK_BUF_SIZE：[字节]如果无法分配`LV_LAYER_SIMPLE_BUF_SIZE`时使用
 *
 * 两个缓冲区大小都以字节为单位
 * "变换图层"（使用了transform_angle/zoom属性的地方）使用更大的缓冲区
 * 并且不能分块绘制。所以这些设置只影响具有不透明度的控件
 */
#        define LV_LAYER_SIMPLE_BUF_SIZE          (24 * 1024)
#        define LV_LAYER_SIMPLE_FALLBACK_BUF_SIZE (3 * 1024)

/*默认图像缓存大小。图像缓存保持图像打开状态
 *如果只使用内置图像格式，则缓存没有真正优势。（即如果没有添加新的图像解码器）
 *使用复杂的图像解码器（如PNG或JPG），缓存可以节省连续打开/解码图像
 *然而打开的图像可能会消耗额外的RAM
 * 0：禁用缓存*/
#        define LV_IMG_CACHE_DEF_SIZE 0

/*每个渐变允许的停止点数。增加此值以允许更多停止点
 *这会为每个额外的停止点增加(sizeof(lv_color_t) + 1)字节*/
#        define LV_GRADIENT_MAX_STOPS 2

/*默认渐变缓冲区大小
 *当LVGL计算渐变"映射"时，可以将它们保存到缓存中以避免再次计算
 *LV_GRAD_CACHE_DEF_SIZE设置此缓存的大小（字节）
 *如果缓存太小，映射仅在绘制需要时分配
 * 0表示没有缓存*/
#        define LV_GRAD_CACHE_DEF_SIZE 0

/*允许对渐变进行抖动（在有限颜色深度显示器上实现视觉上平滑的颜色渐变）
 * LV_DITHER_GRADIENT意味着为对象的渲染表面分配一行或两行额外的内容
 *内存消耗的增加是（32位 * 对象宽度）加上如果使用误差扩散时的24位 * 对象宽度*/
#        define LV_DITHER_GRADIENT 0
#        if LV_DITHER_GRADIENT
/*添加对误差扩散抖动的支持
     *误差扩散抖动获得更好的视觉效果，但意味着在绘制时更多的CPU消耗和内存
     *内存消耗的增加是（24位 * 对象的宽度）*/
#            define LV_DITHER_ERROR_DIFFUSION 0
#        endif

/*旋转分配的最大缓冲区大小
 *仅在显示驱动程序中启用软件旋转时使用*/
#        define LV_DISP_ROT_MAX_BUF (10 * 1024)

/*-------------
 * GPU
 *-----------*/

/*使用Arm的2D加速库Arm-2D*/
#        define LV_USE_GPU_ARM2D 0

/*使用STM32的DMA2D（又名Chrom Art）GPU*/
#        define LV_USE_GPU_STM32_DMA2D 0
#        if LV_USE_GPU_STM32_DMA2D
/*必须定义以包含目标处理器的CMSIS头文件路径
     例如"stm32f7xx.h"或"stm32f4xx.h"*/
#            define LV_GPU_DMA2D_CMSIS_INCLUDE
#        endif

/*启用RA6M3 G2D GPU*/
#        define LV_USE_GPU_RA6M3_G2D 0
#        if LV_USE_GPU_RA6M3_G2D
/*目标处理器的包含路径
     例如"hal_data.h"*/
#            define LV_GPU_RA6M3_G2D_INCLUDE "hal_data.h"
#        endif

/*使用SWM341的DMA2D GPU*/
#        define LV_USE_GPU_SWM341_DMA2D 0
#        if LV_USE_GPU_SWM341_DMA2D
#            define LV_GPU_SWM341_DMA2D_INCLUDE "SWM341.h"
#        endif

/*使用NXP的PXP GPU iMX RTxxx平台*/
#        define LV_USE_GPU_NXP_PXP 0
#        if LV_USE_GPU_NXP_PXP
/*1：为PXP添加默认的裸机和FreeRTOS中断处理程序（lv_gpu_nxp_pxp_osa.c）
     *   并在lv_init()期间自动调用lv_gpu_nxp_pxp_init()。注意必须定义符号SDK_OS_FREE_RTOS
     *   才能使用FreeRTOS OSA，否则选择裸机实现
     * 0：需要在lv_init()之前手动调用lv_gpu_nxp_pxp_init()
     */
#            define LV_USE_GPU_NXP_PXP_AUTO_INIT 0
#        endif

/*使用NXP的VG-Lite GPU iMX RTxxx平台*/
#        define LV_USE_GPU_NXP_VG_LITE 0

/*使用SDL渲染器API*/
#        define LV_USE_GPU_SDL 0
#        if LV_USE_GPU_SDL
#            define LV_GPU_SDL_INCLUDE_PATH <SDL2/SDL.h>
/*纹理缓存大小，默认8MB*/
#            define LV_GPU_SDL_LRU_SIZE (1024 * 1024 * 8)
/*遮罩绘制的自定义混合模式，如果需要与较旧的SDL2库链接则禁用*/
#            define LV_GPU_SDL_CUSTOM_BLEND_MODE (SDL_VERSION_ATLEAST(2, 0, 6))
#        endif

/*-------------
 * 日志
 *-----------*/

/*启用日志模块*/
#        define LV_USE_LOG 0
#        if LV_USE_LOG

/*日志的重要性级别：
     * LV_LOG_LEVEL_TRACE       大量日志以提供详细信息
     * LV_LOG_LEVEL_INFO        记录重要事件
     * LV_LOG_LEVEL_WARN        记录发生的不想要但未导致问题的事件
     * LV_LOG_LEVEL_ERROR       仅关键问题，系统可能失败
     * LV_LOG_LEVEL_USER        仅用户添加的日志
     * LV_LOG_LEVEL_NONE        不记录任何内容*/
#            define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/*1：使用'printf'打印日志
     * 0：用户需要使用`lv_log_register_print_cb()`注册回调*/
#            define LV_LOG_PRINTF 0

/*在产生大量日志的模块中启用/禁用LV_LOG_TRACE*/
#            define LV_LOG_TRACE_MEM        1
#            define LV_LOG_TRACE_TIMER      1
#            define LV_LOG_TRACE_INDEV      1
#            define LV_LOG_TRACE_DISP_REFR  1
#            define LV_LOG_TRACE_EVENT      1
#            define LV_LOG_TRACE_OBJ_CREATE 1
#            define LV_LOG_TRACE_LAYOUT     1
#            define LV_LOG_TRACE_ANIM       1

#        endif /*LV_USE_LOG*/

/*-------------
 * 断言
 *-----------*/

/*如果操作失败或发现无效数据，则启用断言
 *如果启用了LV_USE_LOG，失败时将打印错误消息*/
#        define LV_USE_ASSERT_NULL          1 /*检查参数是否为NULL（非常快，推荐）*/
#        define LV_USE_ASSERT_MALLOC        1 /*检查内存是否成功分配（非常快，推荐）*/
#        define LV_USE_ASSERT_STYLE         0 /*检查样式是否正确初始化（非常快，推荐）*/
#        define LV_USE_ASSERT_MEM_INTEGRITY 0 /*在关键操作后检查`lv_mem`的完整性（慢）*/
#        define LV_USE_ASSERT_OBJ           0 /*检查对象的类型和存在性（例如，未删除）（慢）*/

/*在断言发生时添加自定义处理程序，例如重启MCU*/
#        define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#        define LV_ASSERT_HANDLER         while (1); /*默认停止*/

/*-------------
 * 其他
 *-----------*/

/*1：显示CPU使用率和FPS计数*/
#        define LV_USE_PERF_MONITOR 0
#        if LV_USE_PERF_MONITOR
#            define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
#        endif

/*1：显示已使用的内存和内存碎片
 * 需要LV_MEM_CUSTOM = 0*/
#        define LV_USE_MEM_MONITOR 0
#        if LV_USE_MEM_MONITOR
#            define LV_USE_MEM_MONITOR_POS LV_ALIGN_BOTTOM_LEFT
#        endif

/*1：在重绘区域上绘制随机彩色矩形*/
#        define LV_USE_REFR_DEBUG 0

/*更改内置的(v)snprintf函数*/
#        define LV_SPRINTF_CUSTOM 0
#        if LV_SPRINTF_CUSTOM
#            define LV_SPRINTF_INCLUDE <stdio.h>
#            define lv_snprintf        snprintf
#            define lv_vsnprintf       vsnprintf
#        else /*LV_SPRINTF_CUSTOM*/
#            define LV_SPRINTF_USE_FLOAT 0
#        endif /*LV_SPRINTF_CUSTOM*/

#        define LV_USE_USER_DATA 1

/*垃圾收集器设置
 *如果lvgl绑定到更高级别的语言并且内存由该语言管理，则使用*/
#        define LV_ENABLE_GC 0
#        if LV_ENABLE_GC != 0
#            define LV_GC_INCLUDE "gc.h" /*包含垃圾收集器相关内容*/
#        endif /*LV_ENABLE_GC*/

/*=====================
 *  编译器设置
 *====================*/

/*对于大端系统设置为1*/
#        define LV_BIG_ENDIAN_SYSTEM 0

/*为`lv_tick_inc`函数定义自定义属性*/
#        define LV_ATTRIBUTE_TICK_INC

/*为`lv_timer_handler`函数定义自定义属性*/
#        define LV_ATTRIBUTE_TIMER_HANDLER

/*为`lv_disp_flush_ready`函数定义自定义属性*/
#        define LV_ATTRIBUTE_FLUSH_READY

/*缓冲区所需的对齐大小*/
#        define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1

/*将在需要对齐内存的地方添加（使用-Os时数据可能默认不对齐到边界）
 * 例如__attribute__((aligned(4)))*/
#        define LV_ATTRIBUTE_MEM_ALIGN

/*标记大常数数组的自定义属性，例如字体的位图*/
#        define LV_ATTRIBUTE_LARGE_CONST

/*RAM中大数组声明的编译器前缀*/
#        define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/*将性能关键函数放在更快的内存中（例如RAM）*/
#        define LV_ATTRIBUTE_FAST_MEM

/*用于GPU加速操作的前缀变量，通常这些需要放在可DMA访问的RAM部分中*/
#        define LV_ATTRIBUTE_DMA

/*将整数常量导出到绑定。此宏用于LV_<CONST>形式的常量
 *也应该出现在LVGL绑定API中，如Micropython*/
#        define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning /*默认值只是防止GCC警告*/

/*通过使用int32_t作为坐标而不是int16_t来扩展默认的-32k..32k坐标范围到-4M..4M*/
#        define LV_USE_LARGE_COORD 0

/*==================
 *   字体使用
 *===================*/

/*带有ASCII范围和一些符号的Montserrat字体，使用bpp = 4
 *https://fonts.google.com/specimen/Montserrat*/
#        define LV_FONT_MONTSERRAT_8  0
#        define LV_FONT_MONTSERRAT_10 0
#        define LV_FONT_MONTSERRAT_12 0
#        define LV_FONT_MONTSERRAT_14 1
#        define LV_FONT_MONTSERRAT_16 0
#        define LV_FONT_MONTSERRAT_18 0
#        define LV_FONT_MONTSERRAT_20 0
#        define LV_FONT_MONTSERRAT_22 0
#        define LV_FONT_MONTSERRAT_24 0
#        define LV_FONT_MONTSERRAT_26 0
#        define LV_FONT_MONTSERRAT_28 0
#        define LV_FONT_MONTSERRAT_30 0
#        define LV_FONT_MONTSERRAT_32 0
#        define LV_FONT_MONTSERRAT_34 0
#        define LV_FONT_MONTSERRAT_36 0
#        define LV_FONT_MONTSERRAT_38 0
#        define LV_FONT_MONTSERRAT_40 0
#        define LV_FONT_MONTSERRAT_42 0
#        define LV_FONT_MONTSERRAT_44 0
#        define LV_FONT_MONTSERRAT_46 0
#        define LV_FONT_MONTSERRAT_48 0

/*演示特殊功能*/
#        define LV_FONT_MONTSERRAT_12_SUBPX      0
#        define LV_FONT_MONTSERRAT_28_COMPRESSED 0 /*bpp = 3*/
#        define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0 /*希伯来语、阿拉伯语、波斯语字母及其所有形式*/
#        define LV_FONT_SIMSUN_16_CJK            0 /*1000个最常见的CJK部首*/

/*像素完美的等宽字体*/
#        define LV_FONT_UNSCII_8  0
#        define LV_FONT_UNSCII_16 0

/*在此处声明自定义字体
 *您可以将这些字体用作默认字体，它们将全局可用
 *例如 #define LV_FONT_CUSTOM_DECLARE   LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2)*/
#        define LV_FONT_CUSTOM_DECLARE

/*始终设置默认字体*/
#        define LV_FONT_DEFAULT &lv_font_montserrat_14

/*启用处理大字体和/或具有大量字符的字体
 *限制取决于字体大小、字体字体和bpp
 *如果字体需要它，将触发编译器错误*/
#        define LV_FONT_FMT_TXT_LARGE 0

/*启用/禁用压缩字体支持*/
#        define LV_USE_FONT_COMPRESSED 0

/*启用子像素渲染*/
#        define LV_USE_FONT_SUBPX 0
#        if LV_USE_FONT_SUBPX
/*设置显示器的像素顺序。RGB通道的物理顺序。"正常"字体不重要*/
#            define LV_FONT_SUBPX_BGR 0 /*0：RGB顺序；1：BGR顺序*/
#        endif

/*在找不到字形dsc时启用绘制占位符*/
#        define LV_USE_FONT_PLACEHOLDER 1

/*=================
 *  文本设置
 *=================*/

/**
 * 选择字符串的字符编码
 * 您的IDE或编辑器应该具有相同的字符编码
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
#        define LV_TXT_ENC LV_TXT_ENC_UTF8

/*可以在这些字符上断开（换行）文本*/
#        define LV_TXT_BREAK_CHARS " ,.;:-_"

/*如果单词至少这么长，无论"最漂亮"在哪里都会断开
 *要禁用，设置为<= 0的值*/
#        define LV_TXT_LINE_BREAK_LONG_LEN 0

/*长单词在一行上放置的最少字符数，然后才断开
 *取决于LV_TXT_LINE_BREAK_LONG_LEN*/
#        define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/*长单词在断开后在一行上放置的最少字符数
 *取决于LV_TXT_LINE_BREAK_LONG_LEN*/
#        define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/*用于发信号通知文本重新着色的控制字符*/
#        define LV_TXT_COLOR_CMD "#"

/*支持双向文本。允许混合从左到右和从右到左的文本
 *方向将根据Unicode双向算法处理：
 *https://www.w3.org/International/articles/inline-bidi-markup/uba-basics*/
#        define LV_USE_BIDI 0
#        if LV_USE_BIDI
/*设置默认方向。支持的值：
     *`LV_BASE_DIR_LTR`从左到右
     *`LV_BASE_DIR_RTL`从右到左
     *`LV_BASE_DIR_AUTO`检测文本基本方向*/
#            define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#        endif

/*启用阿拉伯语/波斯语处理
 *在这些语言中，字符应根据其在文本中的位置替换为其他形式*/
#        define LV_USE_ARABIC_PERSIAN_CHARS 0

/*==================
 *  控件使用
 *==================*/

/*控件文档：https://docs.lvgl.io/latest/en/html/widgets/index.html*/

#        define LV_USE_ARC       1

#        define LV_USE_BAR       1

#        define LV_USE_BTN       1

#        define LV_USE_BTNMATRIX 1

#        define LV_USE_CANVAS    1

#        define LV_USE_CHECKBOX  1

#        define LV_USE_DROPDOWN  1 /*需要：lv_label*/

#        define LV_USE_IMG       1 /*需要：lv_label*/

#        define LV_USE_LABEL     1
#        if LV_USE_LABEL
#            define LV_LABEL_TEXT_SELECTION 1 /*启用选择标签的文本*/
#            define LV_LABEL_LONG_TXT_HINT  1 /*在标签中存储一些额外信息以加速绘制非常长的文本*/
#        endif

#        define LV_USE_LINE   1

#        define LV_USE_ROLLER 1 /*需要：lv_label*/
#        if LV_USE_ROLLER
#            define LV_ROLLER_INF_PAGES 7 /*滚轮无限时的额外"页"数*/
#        endif

#        define LV_USE_SLIDER   1 /*需要：lv_bar*/

#        define LV_USE_SWITCH   1

#        define LV_USE_TEXTAREA 1 /*需要：lv_label*/
#        if LV_USE_TEXTAREA != 0
#            define LV_TEXTAREA_DEF_PWD_SHOW_TIME 1500 /*毫秒*/
#        endif

#        define LV_USE_TABLE 1

/*==================
 * 扩展组件
 *==================*/

/*-----------
 * 控件
 *----------*/
#        define LV_USE_ANIMIMG  1

#        define LV_USE_CALENDAR 1
#        if LV_USE_CALENDAR
#            define LV_CALENDAR_WEEK_STARTS_MONDAY 0
#            if LV_CALENDAR_WEEK_STARTS_MONDAY
#                define LV_CALENDAR_DEFAULT_DAY_NAMES            \
                    {                                            \
                        "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" \
                    }
#            else
#                define LV_CALENDAR_DEFAULT_DAY_NAMES            \
                    {                                            \
                        "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" \
                    }
#            endif

#            define LV_CALENDAR_DEFAULT_MONTH_NAMES                                                                   \
                {                                                                                                     \
                    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", \
                        "November", "December"                                                                        \
                }
#            define LV_USE_CALENDAR_HEADER_ARROW    1
#            define LV_USE_CALENDAR_HEADER_DROPDOWN 1
#        endif /*LV_USE_CALENDAR*/

#        define LV_USE_CHART      1

#        define LV_USE_COLORWHEEL 1

#        define LV_USE_IMGBTN     1

#        define LV_USE_KEYBOARD   1

#        define LV_USE_LED        1

#        define LV_USE_LIST       1

#        define LV_USE_MENU       1

#        define LV_USE_METER      1

#        define LV_USE_MSGBOX     1

#        define LV_USE_SPAN       1
#        if LV_USE_SPAN
/*一行文本可以包含的最大span描述符数量*/
#            define LV_SPAN_SNIPPET_STACK_SIZE 64
#        endif

#        define LV_USE_SPINBOX  1

#        define LV_USE_SPINNER  1

#        define LV_USE_TABVIEW  1

#        define LV_USE_TILEVIEW 1

#        define LV_USE_WIN      1

/*-----------
 * 主题
 *----------*/

/*一个简单、令人印象深刻且非常完整的主题*/
#        define LV_USE_THEME_DEFAULT 1
#        if LV_USE_THEME_DEFAULT

/*0：亮色模式；1：暗色模式*/
#            define LV_THEME_DEFAULT_DARK 0

/*1：启用按压时增长*/
#            define LV_THEME_DEFAULT_GROW 1

/*默认过渡时间[毫秒]*/
#            define LV_THEME_DEFAULT_TRANSITION_TIME 80
#        endif /*LV_USE_THEME_DEFAULT*/

/*一个非常简单的主题，是自定义主题的良好起点*/
#        define LV_USE_THEME_BASIC 1

/*为单色显示器设计的主题*/
#        define LV_USE_THEME_MONO 1

/*-----------
 * 布局
 *----------*/

/*类似于CSS中的Flexbox布局*/
#        define LV_USE_FLEX 1

/*类似于CSS中的Grid布局*/
#        define LV_USE_GRID 1

/*---------------------
 * 3rd party libraries
 *--------------------*/

/*常用API的文件系统接口*/

/*fopen、fread等的API*/
#        define LV_USE_FS_STDIO 0
#        if LV_USE_FS_STDIO
#            define LV_FS_STDIO_LETTER     '\0' /*设置一个大写字母，驱动程序将通过该字母访问（例如'A'）*/
#            define LV_FS_STDIO_PATH       "" /*设置工作目录。文件/目录路径将附加到它。*/
#            define LV_FS_STDIO_CACHE_SIZE 0 /*> 0在lv_fs_read()中缓存此字节数*/
#        endif

/*open、read等的API*/
#        define LV_USE_FS_POSIX 0
#        if LV_USE_FS_POSIX
#            define LV_FS_POSIX_LETTER     '\0' /*设置一个大写字母，驱动程序将通过该字母访问（例如'A'）*/
#            define LV_FS_POSIX_PATH       "" /*设置工作目录。文件/目录路径将附加到它。*/
#            define LV_FS_POSIX_CACHE_SIZE 0 /*> 0在lv_fs_read()中缓存此字节数*/
#        endif

/*CreateFile、ReadFile等的API*/
#        define LV_USE_FS_WIN32 0
#        if LV_USE_FS_WIN32
#            define LV_FS_WIN32_LETTER     '\0' /*设置一个大写字母，驱动程序将通过该字母访问（例如'A'）*/
#            define LV_FS_WIN32_PATH       "" /*设置工作目录。文件/目录路径将附加到它。*/
#            define LV_FS_WIN32_CACHE_SIZE 0 /*> 0在lv_fs_read()中缓存此字节数*/
#        endif

/*FATFS的API（需要单独添加）。使用f_open、f_read等*/
#        define LV_USE_FS_FATFS 0
#        if LV_USE_FS_FATFS
#            define LV_FS_FATFS_LETTER     '\0' /*设置一个大写字母，驱动程序将通过该字母访问（例如'A'）*/
#            define LV_FS_FATFS_CACHE_SIZE 0 /*> 0在lv_fs_read()中缓存此字节数*/
#        endif

/*LittleFS的API（需要单独添加库）。使用lfs_file_open、lfs_file_read等*/
#        define LV_USE_FS_LITTLEFS 0
#        if LV_USE_FS_LITTLEFS
#            define LV_FS_LITTLEFS_LETTER     '\0' /*设置一个大写字母，驱动程序将通过该字母访问（例如'A'）*/
#            define LV_FS_LITTLEFS_CACHE_SIZE 0 /*> 0在lv_fs_read()中缓存此字节数*/
#        endif

/*PNG解码器库*/
#        define LV_USE_PNG 0

/*BMP解码器库*/
#        define LV_USE_BMP 0

/*JPG + 分割JPG解码器库
 * 分割JPG是为嵌入式系统优化的自定义格式*/
#        define LV_USE_SJPG 0

/*GIF解码器库*/
#        define LV_USE_GIF 0

/*二维码库*/
#        define LV_USE_QRCODE 0

/*FreeType库*/
#        define LV_USE_FREETYPE 0
#        if LV_USE_FREETYPE
/*FreeType用于缓存字符的内存[字节]（-1：无缓存）*/
#            define LV_FREETYPE_CACHE_SIZE (16 * 1024)
#            if LV_FREETYPE_CACHE_SIZE >= 0
/* 1：位图缓存使用sbit缓存，0：位图缓存使用图像缓存*/
/* sbit缓存：对于小位图（字体大小< 256）更节省内存*/
/* 如果字体大小>= 256，必须配置为图像缓存*/
#                define LV_FREETYPE_SBIT_CACHE 0
/*由此缓存实例管理的最大打开FT_Face/FT_Size对象数*/
/* （0：使用系统默认值）*/
#                define LV_FREETYPE_CACHE_FT_FACES 0
#                define LV_FREETYPE_CACHE_FT_SIZES 0
#            endif
#        endif

/*Tiny TTF库*/
#        define LV_USE_TINY_TTF 0
#        if LV_USE_TINY_TTF
/*从文件加载TTF数据*/
#            define LV_TINY_TTF_FILE_SUPPORT 0
#        endif

/*Rlottie库*/
#        define LV_USE_RLOTTIE 0

/*FFmpeg库用于图像解码和播放视频
 *支持所有主要图像格式，所以不要同时启用其他图像解码器*/
#        define LV_USE_FFMPEG 0
#        if LV_USE_FFMPEG
/*将输入信息转储到stderr*/
#            define LV_FFMPEG_DUMP_FORMAT 0
#        endif

/*-----------
 * 其他
 *----------*/

/*1：启用API来获取对象的快照*/
#        define LV_USE_SNAPSHOT 0

/*1：启用Monkey测试*/
#        define LV_USE_MONKEY 0

/*1：启用网格导航*/
#        define LV_USE_GRIDNAV 0

/*1：启用lv_obj片段*/
#        define LV_USE_FRAGMENT 0

/*1：支持在标签或span控件中使用图像作为字体*/
#        define LV_USE_IMGFONT 0

/*1：启用基于发布者订阅者的消息传递系统*/
#        define LV_USE_MSG 0

/*1：启用拼音输入法*/
/*需要：lv_keyboard*/
#        define LV_USE_IME_PINYIN 0
#        if LV_USE_IME_PINYIN
/*1：使用默认词库*/
/*如果不使用默认词库，请确保在设置词库后使用`lv_ime_pinyin`*/
#            define LV_IME_PINYIN_USE_DEFAULT_DICT 1
/*设置可以显示的候选面板的最大数量*/
/*这需要根据屏幕的大小进行调整*/
#            define LV_IME_PINYIN_CAND_TEXT_NUM 6

/*使用9键输入(k9)*/
#            define LV_IME_PINYIN_USE_K9_MODE 1
#            if LV_IME_PINYIN_USE_K9_MODE == 1
#                define LV_IME_PINYIN_K9_CAND_TEXT_NUM 3
#            endif // LV_IME_PINYIN_USE_K9_MODE
#        endif

/*==================
* 示例
*==================*/

/*启用与库一起构建的示例*/
#        define LV_BUILD_EXAMPLES 1

/*===================
 * 演示使用
 ====================*/

/*显示一些控件。可能需要增加`LV_MEM_SIZE`*/
#        define LV_USE_DEMO_WIDGETS 0
#        if LV_USE_DEMO_WIDGETS
#            define LV_DEMO_WIDGETS_SLIDESHOW 0
#        endif

/*演示编码器和键盘的使用*/
#        define LV_USE_DEMO_KEYPAD_AND_ENCODER 0

/*系统基准测试*/
#        define LV_USE_DEMO_BENCHMARK 0
#        if LV_USE_DEMO_BENCHMARK
/*使用16位颜色深度的RGB565A8图像，而不是ARGB8565*/
#            define LV_DEMO_BENCHMARK_RGB565A8 0
#        endif

/*LVGL压力测试*/
#        define LV_USE_DEMO_STRESS 0

/*音乐播放器演示*/
#        define LV_USE_DEMO_MUSIC 0
#        if LV_USE_DEMO_MUSIC
#            define LV_DEMO_MUSIC_SQUARE    0
#            define LV_DEMO_MUSIC_LANDSCAPE 0
#            define LV_DEMO_MUSIC_ROUND     0
#            define LV_DEMO_MUSIC_LARGE     0
#            define LV_DEMO_MUSIC_AUTO_PLAY 0
#        endif

/*--LV_CONF_H 结束--*/

#    endif /*LV_CONF_H*/

#endif /*"内容启用"结束*/
