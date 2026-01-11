#ifndef EK_IO_H
#define EK_IO_H

#include "../../../ek_conf.h"

#if EK_IO_ENABLE == 1

#    include "../../third_party/lwprintf/inc/lwprintf.h"

#    define EK_IO_FPUTC() void __ek_io_fputc(int ch)

void ek_io_init(void);

#endif /* EK_IO_ENABLE */

#endif
