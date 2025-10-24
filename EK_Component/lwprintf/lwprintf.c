/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwPRINTF - Lightweight stdio manager library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.1.0
 */
#include "Inc/lwprintf.h"
#include <float.h>
#include <limits.h>
#include <stdint.h>

/* ========================= 条件编译 ========================= */
#if (EK_LWPRINTF_ENABLE == 1)

#if LWPRINTF_CFG_OS
#include "system/lwprintf_sys.h"
#endif /* LWPRINTF_CFG_OS */

/* ========================= 静态检查 ========================= */
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING && !LWPRINTF_CFG_SUPPORT_TYPE_FLOAT
#error "不能在没有浮点支持的情况下使用工程类型!"
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING && !LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */
#if !LWPRINTF_CFG_OS && LWPRINTF_CFG_OS_MANUAL_PROTECT
#error "LWPRINTF_CFG_OS_MANUAL_PROTECT 只能在 LWPRINTF_CFG_OS 启用时使用"
#endif /* !LWPRINTF_CFG_OS && LWPRINTF_CFG_OS_MANUAL_PROTECT */

#define CHARISNUM(x)     ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)     ((x) - '0')
#define IS_PRINT_MODE(p) ((p)->out_fn == prv_out_fn_print)

/* Define custom types */
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
typedef long long int float_long_t;
typedef unsigned long long int uint_maxtype_t;
typedef long long int int_maxtype_t;
#else
typedef long int float_long_t;
typedef unsigned long int uint_maxtype_t;
typedef long int int_maxtype_t;
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */

/**
 * @brief          浮点数按部分分割的结构体
 */
typedef struct
{
    float_long_t integer_part; /**< 双精度数的整数部分 */
    double decimal_part_dbl; /**< 双精度数的小数部分乘以10^精度 */
    float_long_t decimal_part; /**< 双精度数小数部分的整数格式 */
    double diff; /**< 小数部分之间的差值（双精度 - 整数） */

    short digits_cnt_integer_part; /**< 整数部分的位数 */
    short digits_cnt_decimal_part; /**< 小数部分的位数 */
    short digits_cnt_decimal_part_useful; /**< 要打印的有效位数 */
} float_num_t;

#if LWPRINTF_CFG_SUPPORT_TYPE_FLOAT
/* Powers of 10 from beginning up to precision level */
static const float_long_t powers_of_10[] = {
    (float_long_t)1E00, (float_long_t)1E01, (float_long_t)1E02, (float_long_t)1E03, (float_long_t)1E04,
    (float_long_t)1E05, (float_long_t)1E06, (float_long_t)1E07, (float_long_t)1E08, (float_long_t)1E09,
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
    (float_long_t)1E10, (float_long_t)1E11, (float_long_t)1E12, (float_long_t)1E13, (float_long_t)1E14,
    (float_long_t)1E15, (float_long_t)1E16, (float_long_t)1E17, (float_long_t)1E18,
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
};
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */
#define FLOAT_MAX_B_ENG (powers_of_10[LWPRINTF_ARRAYSIZE(powers_of_10) - 1])

/**
 * @brief          在输出有符号整数之前检查负数输入
 * @param[in]      pp: 解析对象
 * @param[in]      nnum: 要检查的数字
 */
#define SIGNED_CHECK_NEGATIVE(pp, nnum)    \
    {                                      \
        if ((nnum) < 0)                    \
        {                                  \
            (pp)->m.flags.is_negative = 1; \
            (nnum) = -(nnum);              \
        }                                  \
    }

/**
 * @brief           前向声明
 */
struct lwprintf_int;

/**
 * @brief           私有输出函数声明
 * @param[in]       lwi: 内部工作结构体
 * @param[in]       chr: 要打印的字符
 */
typedef int (*prv_output_fn)(struct lwprintf_int *lwi, const char chr);

/**
 * @brief           内部结构体
 */
typedef struct lwprintf_int
{
    lwprintf_t *lwobj; /*!< 实例句柄 */
    const char *fmt; /*!< 格式字符串 */
    char *const buff; /*!< 不使用打印选项时的缓冲区指针 */
    const size_t buff_size; /*!< 输入缓冲区的大小（使用时） */
    size_t n_len; /*!< 格式化文本的总长度 */
    prv_output_fn out_fn; /*!< 内部输出函数 */
    uint8_t is_print_cancelled; /*!< 打印是否应该被取消的状态 */

    /* 每次检测到新的 % 时必须重置这些内容 */
    struct
    {
        struct
        {
            uint8_t left_align:1; /*!< 减号用于左对齐 */
            uint8_t plus:1; /*!< 为输出中的正数前面添加 + */
            uint8_t space:1; /*!< 前置空格。不与 plus 修饰符一起使用 */
            uint8_t zero:1; /*!< 零填充标志检测，如果数字长度小于宽度修饰符则添加零 */
            uint8_t thousands:1; /*!< 千位已应用分组 */
            uint8_t alt:1; /*!< 使用 # 的替代形式 */
            uint8_t precision:1; /*!< 已使用精度标志 */

            /* 长度修饰符标志 */
            uint8_t longlong:2; /*!< 指示长长整数的标志，用于 'l' (1) 或 'll' (2) 模式 */
            uint8_t char_short:2; /*!< 用于 'h' (1 = short) 或 'hh' (2 = char) 长度修饰符 */
            uint8_t sz_t:1; /*!< size_t 长度整数类型的状态 */
            uint8_t umax_t:1; /*!< uintmax_z 长度整数类型的状态 */

            uint8_t uc:1; /*!< 大写标志 */
            uint8_t is_negative:1; /*!< 数字为负数的状态 */
            uint8_t is_num_zero:1; /*!< 输入数字为零的状态 */
        } flags; /*!< 标志列表 */

        int precision; /*!< 选定的精度 */
        int width; /*!< 文本宽度指示器 */
        uint8_t base; /*!< 数字格式输出的基数 */
        char type; /*!< 格式类型 */
    } m; /*!< 每次格式开始时重置的块 */
} lwprintf_int_t;

/**
 * @brief           根据用户输入获取 LwPRINTF 实例
 * @param[in]       ptr: LwPRINTF 实例。
 *                      设置为 `NULL` 使用默认实例
 */
#define LWPRINTF_GET_LWOBJ(ptr) ((ptr) != NULL ? (ptr) : (&lwprintf_default))

/**
 * @brief           应用程序使用的 LwPRINTF 默认结构体
 */
static lwprintf_t lwprintf_default;

/**
 * @brief           打印数据的输出函数
 * @param[in]       lwi: LwPRINTF 内部实例
 * @param[in]       chr: 要打印的字符
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_out_fn_print(lwprintf_int_t *lwi, const char chr)
{
    if (lwi->is_print_cancelled)
    {
        return 0;
    }

    /* 发送字符到输出 */
    if (!lwi->lwobj->out_fn(chr, lwi->lwobj))
    {
        lwi->is_print_cancelled = 1;
    }
    if (chr != '\0' && !lwi->is_print_cancelled)
    {
        ++lwi->n_len;
    }
    return 1;
}

/**
 * @brief           生成缓冲区数据的输出函数
 * @param[in]       lwi: LwPRINTF 内部实例
 * @param[in]       chr: 要写入的字符
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_out_fn_write_buff(lwprintf_int_t *lwi, const char chr)
{
    if (lwi->buff_size > 0 && lwi->n_len < (lwi->buff_size - 1) && lwi->buff != NULL)
    {
        lwi->buff[lwi->n_len] = chr;
        if (chr != '\0')
        {
            lwi->buff[lwi->n_len + 1] = '\0';
        }
    }
    if (chr != '\0')
    {
        ++lwi->n_len;
    }
    return 1;
}

/**
 * @brief           从输入字符串解析数字
 * @param[in,out]   format: 要处理的输入文本
 * @return          解析的数字
 */
static int prv_parse_num(const char **format)
{
    int num = 0;

    for (; CHARISNUM(**format); ++(*format))
    {
        num = (int)10 * num + CHARTONUM(**format);
    }
    return num;
}

/**
 * @brief           格式化在实际值之前打印的数据
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in]       buff_size: 输出字符串的预期缓冲区大小
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_out_str_before(lwprintf_int_t *lwi, size_t buff_size)
{
    /* 检查宽度 */
    if (lwi->m.width > 0
        /* 如果数字为负数，添加负号；如果为正数且强制使用加号标志 */
        && (lwi->m.flags.is_negative || lwi->m.flags.plus))
    {
        --lwi->m.width;
    }

    /* 检查替代模式 */
    if (lwi->m.flags.alt && !lwi->m.flags.is_num_zero)
    {
        if (lwi->m.base == 8)
        {
            if (lwi->m.width > 0)
            {
                --lwi->m.width;
            }
        }
        else if (lwi->m.base == 16 || lwi->m.base == 2)
        {
            if (lwi->m.width >= 2)
            {
                lwi->m.width -= 2;
            }
            else
            {
                lwi->m.width = 0;
            }
        }
    }

    /* 当使用零填充宽度时，在此处添加负号（或在 + 标志情况下添加正号，或在空格标志情况下添加空格） */
    if (lwi->m.flags.zero)
    {
        if (lwi->m.flags.is_negative)
        {
            lwi->out_fn(lwi, '-');
        }
        else if (lwi->m.flags.plus)
        {
            lwi->out_fn(lwi, '+');
        }
        else if (lwi->m.flags.space)
        {
            lwi->out_fn(lwi, ' ');
        }
    }

    /* 检查标志输出 */
    if (lwi->m.flags.alt && !lwi->m.flags.is_num_zero)
    {
        if (lwi->m.base == 8)
        {
            lwi->out_fn(lwi, '0');
        }
        else if (lwi->m.base == 16)
        {
            lwi->out_fn(lwi, '0');
            lwi->out_fn(lwi, lwi->m.flags.uc ? 'X' : 'x');
        }
        else if (lwi->m.base == 2)
        {
            lwi->out_fn(lwi, '0');
            lwi->out_fn(lwi, lwi->m.flags.uc ? 'B' : 'b');
        }
    }

    /* 右对齐，空格或零填充 */
    if (!lwi->m.flags.left_align && lwi->m.width > 0)
    {
        for (size_t idx = buff_size; !lwi->m.flags.left_align && idx < (size_t)lwi->m.width; ++idx)
        {
            lwi->out_fn(lwi, lwi->m.flags.zero ? '0' : ' ');
        }
    }

    /* 当使用空格填充宽度时，在此处添加负号 */
    if (!lwi->m.flags.zero)
    {
        if (lwi->m.flags.is_negative)
        {
            lwi->out_fn(lwi, '-');
        }
        else if (lwi->m.flags.plus)
        {
            lwi->out_fn(lwi, '+');
        }
        else if (lwi->m.flags.space && buff_size >= (size_t)lwi->m.width)
        {
            lwi->out_fn(lwi, ' ');
        }
    }

    return 1;
}

/**
 * @brief           格式化在实际值之后打印的数据
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in]       buff_size: 输出字符串的预期缓冲区大小
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_out_str_after(lwprintf_int_t *lwi, size_t buff_size)
{
    /* 左对齐，但仅使用空格 */
    if (lwi->m.flags.left_align)
    {
        for (size_t idx = buff_size; idx < (size_t)lwi->m.width; ++idx)
        {
            lwi->out_fn(lwi, ' ');
        }
    }
    return 1;
}

/**
 * @brief           输出原始字符串而不进行任何格式化
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in]       buff: 缓冲区字符串
 * @param[in]       buff_size: 要输出的缓冲区长度
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_out_str_raw(lwprintf_int_t *lwi, const char *buff, size_t buff_size)
{
    for (size_t idx = 0; idx < buff_size; ++idx)
    {
        lwi->out_fn(lwi, buff[idx]);
    }
    return 1;
}

/**
 * @brief           输出从数字/数字生成的字符串
 * 在此阶段应用前后的填充
 *
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in]       buff: 缓冲区字符串
 * @param[in]       buff_size: 要输出的缓冲区长度
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_out_str(lwprintf_int_t *lwi, const char *buff, size_t buff_size)
{
    prv_out_str_before(lwi, buff_size); /* 实现预格式化 */
    prv_out_str_raw(lwi, buff, buff_size); /* 打印实际字符串 */
    prv_out_str_after(lwi, buff_size); /* 实现后格式化 */

    return 1;
}

/**
 * @brief           将 `unsigned int` 转换为字符串
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in]       num: 要转换为字符串的数字
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_longest_unsigned_int_to_str(lwprintf_int_t *lwi, uint_maxtype_t num)
{
    /* 从数字长度开始，支持整数的二进制，即 32 位最大宽度 */
    char num_buf[33], *num_buf_ptr = &num_buf[sizeof(num_buf)];
    char adder_ch = (lwi->m.flags.uc ? 'A' : 'a') - 10;
    size_t len = 0;

    /* 检查数字是否为零 */
    lwi->m.flags.is_num_zero = num == 0;

    /* 向后填充缓冲区 */
    *--num_buf_ptr = '\0';
    do
    {
        int digit = num % lwi->m.base;
        num /= lwi->m.base;
        *--num_buf_ptr = (char)digit + (char)(digit >= 10 ? adder_ch : '0');
    } while (num > 0);

    /* 计算并生成输出 */
    len = sizeof(num_buf) - (size_t)((uintptr_t)num_buf_ptr - (uintptr_t)num_buf) - 1;
    prv_out_str_before(lwi, len);
    for (; *num_buf_ptr;)
    {
        lwi->out_fn(lwi, *num_buf_ptr++);
    }
    prv_out_str_after(lwi, len);
    return 1;
}

/**
 * @brief           将有符号长整数转换为字符串
 * @param[in,out]   lwi: LwPRINTF 实例
 * @param[in]       num: 要转换为字符串的数字
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_longest_signed_int_to_str(lwprintf_int_t *lwi, int_maxtype_t num)
{
    SIGNED_CHECK_NEGATIVE(lwi, num);
    return prv_longest_unsigned_int_to_str(lwi, (uint_maxtype_t)num);
}

/**
 * @brief           计算字符串长度，限制到最大值。
 *
 * @note            使用自定义实现以支持潜在的 `< C11` 版本
 *
 * @param[in]       str: 要计算的字符串
 * @param[in]       max_n: 长度被截断的最大字节数
 * @return          字符串的字节长度
 */
size_t prv_strnlen(const char *str, size_t max_n)
{
    size_t length = 0;

    for (; *str != '\0' && length < max_n; ++length, ++str)
    {
    }
    return length;
}

#if LWPRINTF_CFG_SUPPORT_TYPE_FLOAT

/**
 * @brief           为输入数字计算必要的参数
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in,out]   n: 浮点数实例
 * @param[in]       num: 输入数字
 * @param[in]       type: 格式类型
 */
static void prv_calculate_dbl_num_data(lwprintf_int_t *lwi, float_num_t *n, double num, const char type)
{
    memset(n, 0x00, sizeof(*n));

    if (lwi->m.precision >= (int)LWPRINTF_ARRAYSIZE(powers_of_10))
    {
        lwi->m.precision = (int)LWPRINTF_ARRAYSIZE(powers_of_10) - 1;
    }

    /*
     * Get integer and decimal parts, both in integer formats
     *
     * As an example, with input number of 12.345678 and precision digits set as 4, then result is the following:
     *
     * integer_part = 12            -> Actual integer part of the double number
     * decimal_part_dbl = 3456.78   -> Decimal part multiplied by 10^precision, keeping it in double format
     * decimal_part = 3456          -> Integer part of decimal number
     * diff = 0.78                  -> Difference between actual decimal and integer part of decimal
     *                                  This is used for rounding of last digit (if necessary)
     */
    num += 0.000000000000005;
    n->integer_part = (float_long_t)num;
    n->decimal_part_dbl = (num - (double)n->integer_part) * (double)powers_of_10[lwi->m.precision];
    n->decimal_part = (float_long_t)n->decimal_part_dbl;
    n->diff = n->decimal_part_dbl - (double)((float_long_t)n->decimal_part);

    /* Rounding check of last digit */
    if (n->diff > 0.5)
    {
        ++n->decimal_part;
        if (n->decimal_part >= powers_of_10[lwi->m.precision])
        {
            n->decimal_part = 0;
            ++n->integer_part;
        }
    }
    else if (n->diff < 0.5)
    {
        /* Used in separate if, since comparing float to == will certainly result to false */
    }
    else
    {
        /* Difference is exactly 0.5 */
        if (n->decimal_part == 0)
        {
            ++n->integer_part;
        }
        else
        {
            ++n->decimal_part;
        }
    }

    /* Calculate number of digits for integer and decimal parts */
    if (n->integer_part == 0)
    {
        n->digits_cnt_integer_part = 1;
    }
    else
    {
        float_long_t tmp;
        for (n->digits_cnt_integer_part = 0, tmp = n->integer_part; tmp > 0; ++n->digits_cnt_integer_part, tmp /= 10)
        {
        }
    }
    n->digits_cnt_decimal_part = (short)lwi->m.precision;

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Calculate minimum useful digits for decimal (excl last useless zeros) */
    if (type == 'g')
    {
        float_long_t tmp = n->decimal_part;
        short adder, i;

        for (adder = 0, i = 0; tmp > 0 || i < (short)lwi->m.precision;
             tmp /= 10, n->digits_cnt_decimal_part_useful += adder, ++i)
        {
            if (adder == 0 && (tmp % 10) > 0)
            {
                adder = 1;
            }
        }
    }
    else
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    {
        n->digits_cnt_decimal_part_useful = (short)lwi->m.precision;
    }
}

/**
 * @brief           将双精度数字转换为字符串
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in]       num: 要转换为字符串的数字
 * @return          成功时返回 `1`，否则返回 `0`
 */
static int prv_double_to_str(lwprintf_int_t *lwi, double in_num)
{
    float_num_t dblnum;
    double orig_num = in_num;
    int digits_cnt, chosen_precision, i;
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    int exp_cnt = 0;
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    char def_type = lwi->m.type;
    char str[LWPRINTF_CFG_SUPPORT_LONG_LONG ? 22 : 11];

    /*
     * Check for corner cases
     *
     * - Print "nan" if number is not valid
     * - Print negative infinity if number is less than absolute minimum
     * - Print negative infinity if number is less than -FLOAT_MAX_B_ENG and engineering mode is disabled
     * - Print positive infinity if number is greater than absolute minimum
     * - Print positive infinity if number is greater than FLOAT_MAX_B_ENG and engineering mode is disabled
     * - Go to engineering mode if it is enabled and `in_num < -FLOAT_MAX_B_ENG` or `in_num > FLOAT_MAX_B_ENG`
     */
    if (in_num != in_num)
    {
        return prv_out_str(lwi, lwi->m.flags.uc ? "NAN" : "nan", 3);
    }
    else if (in_num < -DBL_MAX
#if !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
             || in_num < -FLOAT_MAX_B_ENG
#endif /* !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    )
    {
        return prv_out_str(lwi, lwi->m.flags.uc ? "-INF" : "-inf", 4);
    }
    else if (in_num > DBL_MAX
#if !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
             || in_num > FLOAT_MAX_B_ENG
#endif /* !LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    )
    {
        char str[5], *s_ptr = str;
        if (lwi->m.flags.plus)
        {
            *s_ptr++ = '+';
        }
        strcpy(s_ptr, lwi->m.flags.uc ? "INF" : "inf");
        return prv_out_str(lwi, str, lwi->m.flags.plus ? 4 : 3);
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    }
    else if ((in_num < -FLOAT_MAX_B_ENG || in_num > FLOAT_MAX_B_ENG) && def_type != 'g')
    {
        lwi->m.type = def_type = 'e'; /* Go to engineering mode */
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    }

    /* Check sign of the number */
    SIGNED_CHECK_NEGATIVE(lwi, in_num);
    orig_num = in_num;

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Engineering mode check for number of exponents */
    if (def_type == 'e' || def_type == 'g' || in_num > (double)(powers_of_10[LWPRINTF_ARRAYSIZE(powers_of_10) - 1]))
    { /* More vs what float can hold */
        if (lwi->m.type != 'g')
        {
            lwi->m.type = 'e';
        }

        /* Normalize number to be between 0 and 1 and count decimals for exponent */
        if (in_num < 1)
        {
            for (exp_cnt = 0; in_num < 1 && in_num > 0; in_num *= 10, --exp_cnt)
            {
            }
        }
        else
        {
            for (exp_cnt = 0; in_num >= 10; in_num /= 10, ++exp_cnt)
            {
            }
        }
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */

    /* Check precision data */
    chosen_precision = lwi->m.precision; /* This is default value coming from app */
    if (lwi->m.precision >= (int)LWPRINTF_ARRAYSIZE(powers_of_10))
    {
        lwi->m.precision = (int)LWPRINTF_ARRAYSIZE(powers_of_10) - 1; /* Limit to maximum precision */
        /*
         * Precision is lower than the one selected by app (or user).
         * It means that we have to append ending zeros for precision when printing data
         */
    }
    else if (!lwi->m.flags.precision)
    {
        lwi->m.precision = LWPRINTF_CFG_FLOAT_DEFAULT_PRECISION; /* Default precision when not used */
        chosen_precision = lwi->m.precision; /* There was no precision, update chosen precision */
    }
    else if (lwi->m.flags.precision && lwi->m.precision == 0)
    {
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
        /* Precision must be set to 1 if set to 0 by default */
        if (def_type == 'g')
        {
            lwi->m.precision = 1;
        }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    }

    /* Check if type is g and decide if final output should be 'f' or 'e' */
    /*
     * For 'g/G' specifier
     *
     * A double argument representing a floating-point number is converted
     * in style 'f' or 'e' (or in style 'F' or 'E' in the case of a 'G' conversion specifier),
     * depending on the value converted and the precision.
     * Let 'P' equal the precision if nonzero, '6' if the precision is omitted, or '1' if the precision is zero.
     * Then, if a conversion with style 'E' would have an exponent of 'X':
     *
     * if 'P > X >= -4', the conversion is with style 'f' (or 'F') and precision 'P - (X + 1)'.
     * otherwise, the conversion is with style 'e' (or 'E') and precision 'P - 1'.
     *
     * Finally, unless the '#' flag is used,
     * any trailing zeros are removed from the fractional portion of the result
     * and the decimal-point character is removed if there is no fractional portion remaining.
     *
     * A double argument representing an infinity or 'NaN' is converted in the style of an 'f' or 'F' conversion specifier.
     */

    /* Calculate data for number */
    prv_calculate_dbl_num_data(lwi, &dblnum, def_type == 'e' ? in_num : orig_num, def_type);

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Set type G */
    if (def_type == 'g')
    {
        /* As per standard to decide level of precision */
        if (exp_cnt >= -4 && exp_cnt < lwi->m.precision)
        {
            lwi->m.precision -= exp_cnt + 1;
            chosen_precision -= exp_cnt + 1;
            lwi->m.type = 'f';
            in_num = orig_num;
        }
        else
        {
            lwi->m.type = 'e';
            if (lwi->m.precision > 0)
            {
                --lwi->m.precision;
                --chosen_precision;
            }
        }
        prv_calculate_dbl_num_data(lwi, &dblnum, in_num, def_type);
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */

    /* Set number of digits to display */
    digits_cnt = dblnum.digits_cnt_integer_part;
    if (0)
    {
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    }
    else if (def_type == 'g' && lwi->m.precision > 0)
    {
        digits_cnt += dblnum.digits_cnt_decimal_part_useful;
        if (dblnum.digits_cnt_decimal_part_useful > 0)
        {
            ++digits_cnt;
        }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    }
    else
    {
        if (chosen_precision > 0 && lwi->m.flags.precision)
        {
            /* Add precision digits + dot separator */
            digits_cnt += chosen_precision + 1;
        }
    }

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Increase number of digits to display */
    if (lwi->m.type == 'e')
    {
        /* Format is +Exxx, so add 4 or 5 characters (max is 307, min is 00 for exponent) */
        digits_cnt += 4 + (exp_cnt >= 100 || exp_cnt <= -100);
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */

    /* Output strings */
    prv_out_str_before(lwi, digits_cnt);

    /* Output integer part of number */
    if (dblnum.integer_part == 0)
    {
        lwi->out_fn(lwi, '0');
    }
    else
    {
        for (i = 0; dblnum.integer_part > 0; dblnum.integer_part /= 10, ++i)
        {
            str[i] = (char)'0' + (char)(dblnum.integer_part % 10);
        }
        for (; i > 0; --i)
        {
            lwi->out_fn(lwi, str[i - 1]);
        }
    }

    /* Output decimal part */
    if (lwi->m.precision > 0)
    {
        int x;
        if (dblnum.digits_cnt_decimal_part_useful > 0)
        {
            lwi->out_fn(lwi, '.');
        }
        for (i = 0; dblnum.decimal_part > 0; dblnum.decimal_part /= 10, ++i)
        {
            str[i] = (char)'0' + (char)(dblnum.decimal_part % 10);
        }

        /* Output relevant zeros first, string to print is opposite way */
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
        if (def_type == 'g')
        {
            /* TODO: This is to be checked */
            for (x = 0; x < (lwi->m.precision - i) && dblnum.digits_cnt_decimal_part_useful > 0;
                 ++x, --dblnum.digits_cnt_decimal_part_useful)
            {
                lwi->out_fn(lwi, '0');
            }
        }
        else
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
        {
            for (x = i; x < lwi->m.precision; ++x)
            {
                lwi->out_fn(lwi, '0');
            }
        }

        /* Now print string itself */
        for (; i > 0; --i)
        {
            lwi->out_fn(lwi, str[i - 1]);
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
            if (def_type == 'g' && --dblnum.digits_cnt_decimal_part_useful == 0)
            {
                break;
            }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
        }

        /* Print ending zeros if selected precision is bigger than maximum supported */
        if (def_type != 'g')
        {
            for (; x < chosen_precision; ++x)
            {
                lwi->out_fn(lwi, '0');
            }
        }
    }

#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
    /* Engineering mode output, add exponent part */
    if (lwi->m.type == 'e')
    {
        lwi->out_fn(lwi, lwi->m.flags.uc ? 'E' : 'e');
        lwi->out_fn(lwi, exp_cnt >= 0 ? '+' : '-');
        if (exp_cnt < 0)
        {
            exp_cnt = -exp_cnt;
        }
        if (exp_cnt >= 100)
        {
            lwi->out_fn(lwi, (char)'0' + (char)(exp_cnt / 100));
            exp_cnt /= 100;
        }
        lwi->out_fn(lwi, (char)'0' + (char)(exp_cnt / 10));
        lwi->out_fn(lwi, (char)'0' + (char)(exp_cnt % 10));
    }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
    prv_out_str_after(lwi, digits_cnt);

    return 1;
}

#endif /* LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */

/**
 * @brief           处理格式字符串并解析可变参数
 * @param[in,out]   lwi: LwPRINTF 内部实例
 * @param[in]       arg: 可变参数列表
 * @return          成功时返回 `1`，否则返回 `0`
 */
static uint8_t prv_format(lwprintf_int_t *lwi, va_list arg)
{
    uint8_t detected = 0;
    const char *fmt = lwi->fmt;

#if LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT
    if (IS_PRINT_MODE(lwi) && /* OS protection only for print */
        (!lwprintf_sys_mutex_isvalid(&lwi->lwobj->mutex) /* Invalid mutex handle */
         || !lwprintf_sys_mutex_wait(&lwi->lwobj->mutex)))
    { /* Cannot acquire mutex */
        return 0;
    }
#endif /* LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT */

    while (fmt != NULL && *fmt != '\0')
    {
        /* Check if we should stop processing */
        if (lwi->is_print_cancelled)
        {
            break;
        }

        /* Detect beginning */
        if (*fmt != '%')
        {
            lwi->out_fn(lwi, *fmt); /* Output character */
            ++fmt;
            continue;
        }
        ++fmt;
        memset(&lwi->m, 0x00, sizeof(lwi->m)); /* Reset structure */

        /* Parse format */
        /* %[flags][width][.precision][length]type */
        /* Go to https://docs.majerle.eu for more info about supported features */

        /* Check [flags] */
        /* It can have multiple flags in any order */
        detected = 1;
        do
        {
            switch (*fmt)
            {
                case '-':
                    lwi->m.flags.left_align = 1;
                    break;
                case '+':
                    lwi->m.flags.plus = 1;
                    break;
                case ' ':
                    lwi->m.flags.space = 1;
                    break;
                case '0':
                    lwi->m.flags.zero = 1;
                    break;
                case '\'':
                    lwi->m.flags.thousands = 1;
                    break;
                case '#':
                    lwi->m.flags.alt = 1;
                    break;
                default:
                    detected = 0;
                    break;
            }
            if (detected)
            {
                ++fmt;
            }
        } while (detected);

        /* Check [width] */
        lwi->m.width = 0;
        if (CHARISNUM(*fmt))
        { /* Fixed width check */
            /* If number is negative, it has been captured from previous step (left align) */
            lwi->m.width = prv_parse_num(&fmt); /* Number from string directly */
        }
        else if (*fmt == '*')
        { /* Or variable check */
            const int w = (int)va_arg(arg, int);
            if (w < 0)
            {
                lwi->m.flags.left_align = 1; /* Negative width means left aligned */
                lwi->m.width = -w;
            }
            else
            {
                lwi->m.width = w;
            }
            ++fmt;
        }

        /* Check [.precision] */
        lwi->m.precision = 0;
        if (*fmt == '.')
        { /* Precision flag is detected */
            lwi->m.flags.precision = 1;
            if (*++fmt == '*')
            { /* Variable check */
                const int pr = (int)va_arg(arg, int);
                lwi->m.precision = pr > 0 ? pr : 0;
                ++fmt;
            }
            else if (CHARISNUM(*fmt))
            { /* Directly in the string */
                lwi->m.precision = prv_parse_num(&fmt);
            }
        }

        /* Check [length] */
        detected = 1;
        switch (*fmt)
        {
            case 'h':
                lwi->m.flags.char_short = 1; /* Single h detected */
                if (*++fmt == 'h')
                { /* Does it follow by another h? */
                    lwi->m.flags.char_short = 2; /* Second h detected */
                    ++fmt;
                }
                break;
            case 'l':
                lwi->m.flags.longlong = 1; /* Single l detected */
                if (*++fmt == 'l')
                { /* Does it follow by another l? */
                    lwi->m.flags.longlong = 2; /* Second l detected */
                    ++fmt;
                }
                break;
            case 'L':
                break;
            case 'z':
                lwi->m.flags.sz_t = 1; /* Size T flag */
                ++fmt;
                break;
            case 'j':
                lwi->m.flags.umax_t = 1; /* uintmax_t flag */
                ++fmt;
                break;
            case 't':
                break;
            default:
                detected = 0;
        }

        /* Check type */
        lwi->m.type = *fmt + (char)((*fmt >= 'A' && *fmt <= 'Z') ? 0x20 : 0x00);
        if (*fmt >= 'A' && *fmt <= 'Z')
        {
            lwi->m.flags.uc = 1;
        }
        switch (*fmt)
        {
            case 'a':
            case 'A':
                /* Double in hexadecimal notation */
                (void)va_arg(arg, double); /* Read argument to ignore it and move to next one */
                prv_out_str_raw(lwi, "NaN", 3); /* Print string */
                break;
            case 'c':
                lwi->out_fn(lwi, (char)va_arg(arg, int));
                break;
#if LWPRINTF_CFG_SUPPORT_TYPE_INT
            case 'd':
            case 'i':
                {
                    /* Check for different length parameters */
                    lwi->m.base = 10;
                    if (lwi->m.flags.longlong == 0)
                    {
                        prv_longest_signed_int_to_str(lwi, (int_maxtype_t)va_arg(arg, signed int));
                    }
                    else if (lwi->m.flags.longlong == 1)
                    {
                        prv_longest_signed_int_to_str(lwi, (int_maxtype_t)va_arg(arg, signed long int));
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
                    }
                    else if (lwi->m.flags.longlong == 2)
                    {
                        prv_longest_signed_int_to_str(lwi, (int_maxtype_t)va_arg(arg, signed long long int));
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
                    }
                    break;
                }
            case 'b':
            case 'B':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
                if (*fmt == 'b' || *fmt == 'B')
                {
                    lwi->m.base = 2;
                }
                else if (*fmt == 'o')
                {
                    lwi->m.base = 8;
                }
                else if (*fmt == 'u')
                {
                    lwi->m.base = 10;
                }
                else if (*fmt == 'x' || *fmt == 'X')
                {
                    lwi->m.base = 16;
                }
                lwi->m.flags.space = 0; /* Space flag has no meaning here */

                /* Check for different length parameters */
                if (0)
                {
                }
                else if (lwi->m.flags.sz_t)
                {
                    prv_longest_unsigned_int_to_str(lwi, (uint_maxtype_t)va_arg(arg, size_t));
                }
                else if (lwi->m.flags.umax_t)
                {
                    prv_longest_unsigned_int_to_str(lwi, (uint_maxtype_t)va_arg(arg, uintmax_t));
                }
                else if (lwi->m.flags.longlong == 0 || lwi->m.base == 2)
                {
                    uint_maxtype_t v = va_arg(arg, unsigned int);
                    switch (lwi->m.flags.char_short)
                    {
                        case 2:
                            v = (uint_maxtype_t)((unsigned char)v);
                            break;
                        case 1:
                            v = (uint_maxtype_t)((unsigned short int)v);
                            break;
                        default:
                            v = (uint_maxtype_t)((unsigned int)v);
                            break;
                    }
                    prv_longest_unsigned_int_to_str(lwi, v);
                }
                else if (lwi->m.flags.longlong == 1)
                {
                    prv_longest_unsigned_int_to_str(lwi, (uint_maxtype_t)va_arg(arg, unsigned long int));
#if LWPRINTF_CFG_SUPPORT_LONG_LONG
                }
                else if (lwi->m.flags.longlong == 2)
                {
                    prv_longest_unsigned_int_to_str(lwi, (uint_maxtype_t)va_arg(arg, unsigned long long int));
#endif /* LWPRINTF_CFG_SUPPORT_LONG_LONG */
                }
                break;
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_INT */
#if LWPRINTF_CFG_SUPPORT_TYPE_STRING
            case 's':
                {
                    const char *b = va_arg(arg, const char *);
                    if (b == NULL)
                    {
                        b = "(null)";
                    }

                    /* Output string up to maximum buffer. If user provides lower buffer size, write will not write to it
                    but it will still calculate "would be" length */
                    prv_out_str(lwi, b, prv_strnlen(b, lwi->m.flags.precision ? (size_t)lwi->m.precision : (SIZE_MAX)));
                    break;
                }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_STRING */
#if LWPRINTF_CFG_SUPPORT_TYPE_POINTER
            case 'p':
                {
                    lwi->m.base = 16; /* Go to hex format */
                    lwi->m.flags.uc = 0; /* Uppercase characters */
                    lwi->m.flags.zero = 1; /* Zero padding */
                    lwi->m.width =
                        sizeof(uintptr_t) * 2; /* Number is in hex format and byte is represented with 2 letters */

                    prv_longest_unsigned_int_to_str(lwi, (uint_maxtype_t)va_arg(arg, uintptr_t));
                    break;
                }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_POINTER */
#if LWPRINTF_CFG_SUPPORT_TYPE_FLOAT
            case 'f':
            case 'F':
#if LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING
            case 'e':
            case 'E':
            case 'g':
            case 'G':
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_ENGINEERING */
                /* Double number in different format. Final output depends on type of format */
                prv_double_to_str(lwi, (double)va_arg(arg, double));
                break;
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_FLOAT */
            case 'n':
                {
                    int *ptr = (void *)va_arg(arg, int *);
                    *ptr = (int)lwi->n_len; /* Write current length */

                    break;
                }
            case '%':
                lwi->out_fn(lwi, '%');
                break;
#if LWPRINTF_CFG_SUPPORT_TYPE_BYTE_ARRAY
            /*
             * This is to print unsigned-char formatted pointer in hex string
             *
             * char arr[] = {0, 1, 2, 3, 255};
             * "%5K" would produce 00010203FF
             */
            case 'k':
            case 'K':
                {
                    unsigned char *ptr =
                        (void *)va_arg(arg, unsigned char *); /* Get input parameter as unsigned char pointer */
                    int len = lwi->m.width, full_width;
                    uint8_t is_space = lwi->m.flags.space == 1;

                    if (ptr == NULL || len == 0)
                    {
                        break;
                    }

                    lwi->m.flags.zero = 1; /* Prepend with zeros if necessary */
                    lwi->m.width = 0; /* No width parameter */
                    lwi->m.base = 16; /* Hex format */
                    lwi->m.flags.space = 0; /* Delete any flag for space */

                    /* Full width of digits to print */
                    full_width = len * (2 + (int)is_space);
                    if (is_space && full_width > 0)
                    {
                        --full_width; /* Remove space after last number */
                    }

                    /* Output byte by byte w/o hex prefix */
                    prv_out_str_before(lwi, full_width);
                    for (int i = 0; i < len; ++i, ++ptr)
                    {
                        uint8_t d;

                        d = (*ptr >> 0x04) & 0x0F; /* Print MSB */
                        lwi->out_fn(lwi, (char)(d) + (char)(d >= 10 ? ((lwi->m.flags.uc ? 'A' : 'a') - 10) : '0'));
                        d = *ptr & 0x0F; /* Print LSB */
                        lwi->out_fn(lwi, (char)(d) + (char)(d >= 10 ? ((lwi->m.flags.uc ? 'A' : 'a') - 10) : '0'));

                        if (is_space && i < (len - 1))
                        {
                            lwi->out_fn(lwi, ' '); /* Generate space between numbers */
                        }
                    }
                    prv_out_str_after(lwi, full_width);
                    break;
                }
#endif /* LWPRINTF_CFG_SUPPORT_TYPE_BYTE_ARRAY */
            default:
                lwi->out_fn(lwi, *fmt);
        }
        ++fmt;
    }
    lwi->out_fn(lwi, '\0'); /* Output last zero number */
#if LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT
    if (IS_PRINT_MODE(lwi))
    { /* Mutex only for print operation */
        lwprintf_sys_mutex_release(&lwi->lwobj->mutex);
    }
#endif /* LWPRINTF_CFG_OS && !LWPRINTF_CFG_OS_MANUAL_PROTECT */
    return 1;
}

/**
 * @brief           初始化 LwPRINTF 实例
 * @param[in,out]   lwobj: LwPRINTF 工作实例
 * @param[in]       out_fn: 用于打印操作的输出函数。
 *                      当设置为 `NULL` 时，直接打印到流函数将无法工作
 *                      并且如果被应用程序调用将返回错误。
 *                      此外，此特定实例的系统互斥锁将不会被调用，
 *                      因为不需要系统互斥锁。所有格式化函数（打印除外）
 *                      都是线程安全的。库使用基于栈的变量
 * @return          成功时返回 `1`，否则返回 `0`
 */
uint8_t lwprintf_init_ex(lwprintf_t *lwobj, lwprintf_output_fn out_fn)
{
    LWPRINTF_GET_LWOBJ(lwobj)->out_fn = out_fn;

#if LWPRINTF_CFG_OS
    /* Create system mutex, but only if user selected to ever use print mode */
    if (out_fn != NULL && (lwprintf_sys_mutex_isvalid(&LWPRINTF_GET_LWOBJ(lwobj)->mutex) ||
                           !lwprintf_sys_mutex_create(&LWPRINTF_GET_LWOBJ(lwobj)->mutex)))
    {
        return 0;
    }
#endif /* LWPRINTF_CFG_OS */
    return 1;
}

/**
 * @brief           将格式化数据从可变参数列表打印到输出
 * @param[in,out]   lwobj: LwPRINTF 实例。设置为 `NULL` 使用默认实例
 * @param[in]       format: 包含要写入输出的文本的 C 字符串
 * @param[in]       arg: 识别用 `va_start` 初始化的可变参数列表的值。
 *                      `va_list` 是在 `<cstdarg>` 中定义的特殊类型。
 * @return          如果 `n` 足够大，本应写入的字符数，
 *                      不计算终止空字符。
 */
int lwprintf_vprintf_ex(lwprintf_t *const lwobj, const char *format, va_list arg)
{
    lwprintf_int_t fobj = {
        .lwobj = LWPRINTF_GET_LWOBJ(lwobj),
        .out_fn = prv_out_fn_print,
        .fmt = format,
        .buff = NULL,
        .buff_size = 0,
    };
    /* For direct print, output function must be set by user */
    if (fobj.lwobj->out_fn == NULL)
    {
        return 0;
    }
    if (prv_format(&fobj, arg))
    {
        return (int)fobj.n_len;
    }
    return 0;
}

/**
 * @brief           将格式化数据打印到输出
 * @param[in,out]   lwobj: LwPRINTF 实例。设置为 `NULL` 使用默认实例
 * @param[in]       format: 包含要写入输出的文本的 C 字符串
 * @param[in]       ...: 格式字符串的可选参数
 * @return          如果 `n` 足够大，本应写入的字符数，
 *                      不计算终止空字符。
 */
int lwprintf_printf_ex(lwprintf_t *const lwobj, const char *format, ...)
{
    va_list valist;
    int n_len;

    va_start(valist, format);
    n_len = lwprintf_vprintf_ex(lwobj, format, valist);
    va_end(valist);

    return n_len;
}

/**
 * @brief           将格式化数据从可变参数列表写入到指定大小的缓冲区
 * @param[in,out]   lwobj: LwPRINTF 实例。设置为 `NULL` 使用默认实例
 * @param[in]       s_out: 指向存储结果 C 字符串的缓冲区的指针。
 *                      缓冲区的大小应至少为 `n` 个字符
 * @param[in]       n_maxlen: 缓冲区中要使用的最大字节数。
 *                      生成的字符串长度最多为 `n - 1`，
 *                      为额外的终止空字符留出空间
 * @param[in]       format: 包含格式字符串的 C 字符串，遵循与 printf 中 format 相同的规范
 * @param[in]       arg: 识别用 `va_start` 初始化的可变参数列表的值。
 *                      `va_list` 是在 `<cstdarg>` 中定义的特殊类型。
 * @return          如果 `n` 足够大，本应写入的字符数，
 *                      不计算终止空字符。
 */
int lwprintf_vsnprintf_ex(lwprintf_t *const lwobj, char *s_out, size_t n_maxlen, const char *format, va_list arg)
{
    lwprintf_int_t fobj = {
        .lwobj = LWPRINTF_GET_LWOBJ(lwobj),
        .out_fn = prv_out_fn_write_buff,
        .fmt = format,
        .buff = s_out,
        .buff_size = n_maxlen,
    };
    if (s_out != NULL && n_maxlen > 0)
    {
        *s_out = '\0';
    }
    if (prv_format(&fobj, arg))
    {
        return (int)fobj.n_len;
    }
    return 0;
}

/**
 * @brief           将格式化数据从可变参数列表写入到指定大小的缓冲区
 * @param[in,out]   lwobj: LwPRINTF 实例。设置为 `NULL` 使用默认实例
 * @param[in]       s_out: 指向存储结果 C 字符串的缓冲区的指针。
 *                      缓冲区的大小应至少为 `n` 个字符
 * @param[in]       n_maxlen: 缓冲区中要使用的最大字节数。
 *                      生成的字符串长度最多为 `n - 1`，
 *                      为额外的终止空字符留出空间
 * @param[in]       format: 包含格式字符串的 C 字符串，遵循与 printf 中 format 相同的规范
 * @param[in]       ...: 格式字符串的可选参数
 * @return          如果 `n` 足够大，本应写入的字符数，
 *                      不计算终止空字符。
 */
int lwprintf_snprintf_ex(lwprintf_t *const lwobj, char *s_out, size_t n_maxlen, const char *format, ...)
{
    va_list valist;
    int len;

    va_start(valist, format);
    len = lwprintf_vsnprintf_ex(lwobj, s_out, n_maxlen, format, valist);
    va_end(valist);

    return len;
}

#if LWPRINTF_CFG_OS_MANUAL_PROTECT || __DOXYGEN__

/**
 * @brief           手动启用互斥
 * @param[in,out]   lwobj: LwPRINTF 实例。设置为 `NULL` 使用默认实例
 * @return          如果受保护返回 `1`，否则返回 `0`
 */
uint8_t lwprintf_protect_ex(lwprintf_t *const lwobj)
{
    lwprintf_t *obj = LWPRINTF_GET_LWOBJ(lwobj);
    return obj->out_fn != NULL && lwprintf_sys_mutex_isvalid(&obj->mutex) && lwprintf_sys_mutex_wait(&obj->mutex);
}

/**
 * @brief           手动禁用互斥
 * @param[in,out]   lwobj: LwPRINTF 实例。设置为 `NULL` 使用默认实例
 * @return          如果保护已禁用返回 `1`，否则返回 `0`
 */
uint8_t lwprintf_unprotect_ex(lwprintf_t *const lwobj)
{
    lwprintf_t *obj = LWPRINTF_GET_LWOBJ(lwobj);
    return obj->out_fn != NULL && lwprintf_sys_mutex_release(&obj->mutex);
}

#endif /* LWPRINTF_CFG_OS_MANUAL_PROTECT || __DOXYGEN__ */

#endif /* EK_LWPRINTF_ENABLE */
