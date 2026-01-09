#ifndef EK_IO_H
#define EK_IO_H

#include "../../../ek_conf.h"

#if EK_IO_ENABLE == 1

// 直接使用 lwprintf 作为 stdio 的替代
#    include "../../third_party/lwprintf/inc/lwprintf.h"

#endif /* EK_IO_ENABLE */

#endif
