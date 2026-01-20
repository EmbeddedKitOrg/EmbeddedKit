#ifndef EK_SHELL_H
#define EK_SHELL_H

#include "ek_conf.h"

#if EK_SHELL_ENABLE == 1
/*
在 `.ld` 文件中添加以下内容
_shell_command_start = .;
KEEP (*(shellCommand))
_shell_command_end = .;
*/

#include "../../third_party/letter_shell/inc/shell.h"

#define EK_SHELL_EXPORT_VAR_INT(var, variable, desc)   SHELL_EXPORT_VAR_INT(var, variable, desc)
#define EK_SHELL_EXPORT_VAR_SHORT(var, variable, desc) SHELL_EXPORT_VAR_SHORT(var, variable, desc)
#define EK_SHELL_EXPORT_VAR_CHAR(var, variable, desc)  SHELL_EXPORT_VAR_CHAR(var, variable, desc)
#define EK_SHELL_EXPORT_VAR_PTR(var, variable, desc)   SHELL_EXPORT_VAR_POINTER(var, variable, desc)
#define EK_SHELL_EXPORT_VAR(var, variable, desc)       SHELL_EXPORT_VAL(val, value, desc)

#endif /* EK_SHELL_ENABLE */

#endif /* EK_SHELL_H */
