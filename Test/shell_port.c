// 解决 letter shell 的 ld 报错
#if defined(__linux__) || defined(__APPLE__)

unsigned long _shell_command_start = 0;
extern unsigned long _shell_command_end __attribute__((alias("_shell_command_start")));

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
int _shell_command_start;
int _shell_command_end;

#endif
