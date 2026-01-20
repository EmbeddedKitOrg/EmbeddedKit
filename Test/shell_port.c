#if defined(__linux__) || defined(__APPLE__) || defined(WIN32)
// 解决 letter shell 的 ld 报错
unsigned long _shell_command_start = 0;
extern unsigned long _shell_command_end __attribute__((alias("_shell_command_start")));

#endif
