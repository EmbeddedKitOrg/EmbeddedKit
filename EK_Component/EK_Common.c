/**
 * @file EK_Common.c
 * @brief EmbeddedKit 公共函数实现
 * @details 提供嵌入式系统优化的常用库函数实现
 * @author N1ntyNine99
 * @date 2025-09-14
 * @version 1.0
 */

#include "EK_Common.h"
/* ========================= 内存操作函数 ========================= */
/**
 * @brief 内存拷贝函数
 * @param p_dst 目标地址
 * @param p_src 源地址
 * @param bytes 拷贝字节数
 * @note 针对嵌入式系统优化，支持重叠内存区域
 */
void EK_vMemCpy(void *p_dst, const void *p_src, size_t bytes)
{
    if (p_dst == NULL || p_src == NULL || bytes == 0) return;
    
    uint8_t *temp_dst = (uint8_t *)p_dst;
    const uint8_t *temp_src = (const uint8_t *)p_src;
    
    // 检测内存重叠，选择拷贝方向
    if (temp_dst > temp_src && temp_dst < temp_src + bytes)
    {
        // 从后往前拷贝，防止覆盖
        temp_dst += bytes - 1;
        temp_src += bytes - 1;
        for (size_t i = 0; i < bytes; i++)
        {
            *temp_dst-- = *temp_src--;
        }
    }
    else
    {
        // 从前往后拷贝
        for (size_t i = 0; i < bytes; i++)
        {
            temp_dst[i] = temp_src[i];
        }
    }
}

/**
 * @brief 内存设置函数
 * @param p_dst 目标地址
 * @param value 要设置的值
 * @param bytes 设置字节数
 */
void EK_vMemSet(void *p_dst, uint8_t value, size_t bytes)
{
    if (p_dst == NULL || bytes == 0) return;
    
    uint8_t *temp_dst = (uint8_t *)p_dst;
    for (size_t i = 0; i < bytes; i++)
    {
        temp_dst[i] = value;
    }
}

/**
 * @brief 内存比较函数
 * @param p_buf1 缓冲区1
 * @param p_buf2 缓冲区2
 * @param bytes 比较字节数
 * @return int 比较结果
 * @retval 0 相等
 * @retval >0 buf1 > buf2
 * @retval <0 buf1 < buf2
 */
int EK_iMemCmp(const void *p_buf1, const void *p_buf2, size_t bytes)
{
    if (p_buf1 == NULL || p_buf2 == NULL) return 0;
    
    const uint8_t *temp_buf1 = (const uint8_t *)p_buf1;
    const uint8_t *temp_buf2 = (const uint8_t *)p_buf2;
    
    for (size_t i = 0; i < bytes; i++)
    {
        if (temp_buf1[i] != temp_buf2[i])
        {
            return temp_buf1[i] - temp_buf2[i];
        }
    }
    return 0;
}

/* ========================= 字符串操作函数 ========================= */

/**
 * @brief 字符串长度计算
 * @param p_str 字符串指针
 * @return size_t 字符串长度
 */
size_t EK_sStrLen(const char *p_str)
{
    if (p_str == NULL) return 0;
    
    size_t len = 0;
    while (p_str[len] != '\0')
    {
        len++;
    }
    return len;
}

/**
 * @brief 安全字符串长度计算
 * @param p_str 字符串指针
 * @param max_len 最大长度限制
 * @return size_t 字符串长度
 */
size_t EK_sStrNLen(const char *p_str, size_t max_len)
{
    if (p_str == NULL) return 0;
    
    size_t len = 0;
    while (len < max_len && p_str[len] != '\0')
    {
        len++;
    }
    return len;
}

/**
 * @brief 字符串拷贝
 * @param p_dst 目标字符串
 * @param p_src 源字符串
 * @return char* 目标字符串指针
 */
char *EK_pStrCpy(char *p_dst, const char *p_src)
{
    if (p_dst == NULL || p_src == NULL) return p_dst;
    
    size_t i = 0;
    while ((p_dst[i] = p_src[i]) != '\0')
    {
        i++;
    }
    return p_dst;
}

/**
 * @brief 安全字符串拷贝
 * @param p_dst 目标字符串
 * @param p_src 源字符串
 * @param max_len 最大拷贝长度
 * @return char* 目标字符串指针
 */
char *EK_pStrNCpy(char *p_dst, const char *p_src, size_t max_len)
{
    if (p_dst == NULL || p_src == NULL || max_len == 0) return p_dst;
    
    size_t i;
    for (i = 0; i < max_len - 1 && p_src[i] != '\0'; i++)
    {
        p_dst[i] = p_src[i];
    }
    p_dst[i] = '\0';
    return p_dst;
}

/**
 * @brief 字符串比较
 * @param p_str1 字符串1
 * @param p_str2 字符串2
 * @return int 比较结果
 * @retval 0 相等
 * @retval >0 str1 > str2
 * @retval <0 str1 < str2
 */
int EK_iStrCmp(const char *p_str1, const char *p_str2)
{
    if (p_str1 == NULL || p_str2 == NULL) return 0;
    
    while (*p_str1 && (*p_str1 == *p_str2))
    {
        p_str1++;
        p_str2++;
    }
    return *(const uint8_t*)p_str1 - *(const uint8_t*)p_str2;
}

/**
 * @brief 限长字符串比较
 * @param p_str1 字符串1
 * @param p_str2 字符串2
 * @param max_len 最大比较长度
 * @return int 比较结果
 */
int EK_iStrNCmp(const char *p_str1, const char *p_str2, size_t max_len)
{
    if (p_str1 == NULL || p_str2 == NULL || max_len == 0) return 0;
    
    for (size_t i = 0; i < max_len; i++)
    {
        if (p_str1[i] != p_str2[i] || p_str1[i] == '\0')
        {
            return *(const uint8_t*)&p_str1[i] - *(const uint8_t*)&p_str2[i];
        }
    }
    return 0;
}

/**
 * @brief 字符串连接
 * @param p_dst 目标字符串
 * @param p_src 源字符串
 * @return char* 目标字符串指针
 */
char *EK_pStrCat(char *p_dst, const char *p_src)
{
    if (p_dst == NULL || p_src == NULL) return p_dst;
    
    char *p_temp = p_dst;
    while (*p_temp != '\0')
    {
        p_temp++;
    }
    
    while ((*p_temp++ = *p_src++) != '\0');
    
    return p_dst;
}

/**
 * @brief 查找字符在字符串中的位置
 * @param p_str 字符串
 * @param ch 要查找的字符
 * @return char* 字符位置指针，NULL表示未找到
 */
char *EK_pStrChr(const char *p_str, int ch)
{
    if (p_str == NULL) return NULL;
    
    while (*p_str != '\0')
    {
        if (*p_str == (char)ch)
        {
            return (char*)p_str;
        }
        p_str++;
    }
    
    if (ch == '\0')
    {
        return (char*)p_str;
    }
    
    return NULL;
}

/* ========================= 数值转换函数 ========================= */

/**
 * @brief 整数转字符串
 * @param value 整数值
 * @param p_str 字符串缓冲区
 * @param base 进制(2-36)
 * @return char* 字符串指针
 */
char *EK_pItoA(int value, char *p_str, int base)
{
    if (p_str == NULL || base < 2 || base > 36) return NULL;
    
    char *p_temp = p_str;
    int temp_val = value;
    bool is_negative = false;
    
    if (value < 0 && base == 10)
    {
        is_negative = true;
        temp_val = -value;
    }
    
    // 处理0的特殊情况
    if (temp_val == 0)
    {
        *p_temp++ = '0';
        *p_temp = '\0';
        return p_str;
    }
    
    // 转换数字
    while (temp_val > 0)
    {
        int digit = temp_val % base;
        *p_temp++ = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        temp_val /= base;
    }
    
    if (is_negative)
    {
        *p_temp++ = '-';
    }
    
    *p_temp = '\0';
    
    // 反转字符串
    char *p_start = p_str;
    char *p_end = p_temp - 1;
    while (p_start < p_end)
    {
        char temp_char = *p_start;
        *p_start++ = *p_end;
        *p_end-- = temp_char;
    }
    
    return p_str;
}

/**
 * @brief 字符串转整数
 * @param p_str 字符串
 * @return int 整数值
 */
int EK_iAtoI(const char *p_str)
{
    if (p_str == NULL) return 0;
    
    int result = 0;
    int sign = 1;
    
    // 跳过空白字符
    while (*p_str == ' ' || *p_str == '\t' || *p_str == '\n')
    {
        p_str++;
    }
    
    // 处理符号
    if (*p_str == '-')
    {
        sign = -1;
        p_str++;
    }
    else if (*p_str == '+')
    {
        p_str++;
    }
    
    // 转换数字
    while (*p_str >= '0' && *p_str <= '9')
    {
        result = result * 10 + (*p_str - '0');
        p_str++;
    }
    
    return sign * result;
}

/* ========================= 位操作函数 ========================= */

/**
 * @brief 设置位
 * @param p_data 数据指针
 * @param bit_pos 位位置
 */
void EK_vSetBit(uint32_t *p_data, uint8_t bit_pos)
{
    if (p_data == NULL || bit_pos >= 32) return;
    *p_data |= (1UL << bit_pos);
}

/**
 * @brief 清除位
 * @param p_data 数据指针
 * @param bit_pos 位位置
 */
void EK_vClearBit(uint32_t *p_data, uint8_t bit_pos)
{
    if (p_data == NULL || bit_pos >= 32) return;
    *p_data &= ~(1UL << bit_pos);
}

/**
 * @brief 翻转位
 * @param p_data 数据指针
 * @param bit_pos 位位置
 */
void EK_vToggleBit(uint32_t *p_data, uint8_t bit_pos)
{
    if (p_data == NULL || bit_pos >= 32) return;
    *p_data ^= (1UL << bit_pos);
}

/**
 * @brief 测试位
 * @param data 数据
 * @param bit_pos 位位置
 * @return bool 位状态
 * @retval true 位为1
 * @retval false 位为0
 */
bool EK_bTestBit(uint32_t data, uint8_t bit_pos)
{
    if (bit_pos >= 32) return false;
    return (data & (1UL << bit_pos)) != 0;
}

/* ========================= 校验函数 ========================= */

/**
 * @brief 计算8位校验和
 * @param p_data 数据指针
 * @param length 数据长度
 * @return uint8_t 校验和
 */
uint8_t EK_u8CheckSum(const uint8_t *p_data, size_t length)
{
    if (p_data == NULL) return 0;
    
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++)
    {
        checksum += p_data[i];
    }
    return checksum;
}

/**
 * @brief 计算异或校验
 * @param p_data 数据指针
 * @param length 数据长度
 * @return uint8_t 异或校验值
 */
uint8_t EK_u8XorCheck(const uint8_t *p_data, size_t length)
{
    if (p_data == NULL) return 0;
    
    uint8_t xor_result = 0;
    for (size_t i = 0; i < length; i++)
    {
        xor_result ^= p_data[i];
    }
    return xor_result;
}

/* ========================= 数学函数 ========================= */

/**
 * @brief 求绝对值
 * @param value 输入值
 * @return int 绝对值
 */
int EK_iAbs(int value)
{
    return (value < 0) ? -value : value;
}

/**
 * @brief 求最大值
 * @param a 值1
 * @param b 值2
 * @return int 最大值
 */
int EK_iMax(int a, int b)
{
    return (a > b) ? a : b;
}

/**
 * @brief 求最小值
 * @param a 值1
 * @param b 值2
 * @return int 最小值
 */
int EK_iMin(int a, int b)
{
    return (a < b) ? a : b;
}

/**
 * @brief 值限制在范围内
 * @param value 输入值
 * @param min_val 最小值
 * @param max_val 最大值
 * @return int 限制后的值
 */
int EK_iClamp(int value, int min_val, int max_val)
{
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}