/**
 * @file ek_app_stub.c
 * @brief L5_App ek_main() 的弱定义存根
 *
 * 当L5_App未提供 ek_main() 实现时，使用此空实现避免链接错误
 */

__attribute__((weak)) void ek_main(void)
{
    /* 空实现：由L5_App提供实际定义 */
    while (1)
    {
        /* 如果没有L5_App，程序会停在这里 */
    }
}
