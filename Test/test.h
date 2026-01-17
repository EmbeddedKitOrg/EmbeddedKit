#ifndef TEST_H
#define TEST_H

#include <stdlib.h>

#include "../L2_Core/utils/inc/ek_io.h"
#include "../L2_Core/utils/inc/ek_mem.h"
#include "../L2_Core/utils/inc/ek_def.h"
#include "../L2_Core/utils/inc/ek_stack.h"
#include "../L2_Core/utils/inc/ek_log.h"
#include "../L2_Core/utils/inc/ek_ringbuf.h"
#include "../L2_Core/utils/inc/ek_vec.h"
#include "../L2_Core/utils/inc/ek_assert.h"

#define PI (3.141592f)

typedef struct
{
    uint8_t name;
    float score;
} student_t;

void stack_test(void);
void ringbuf_test(void);
void vec_test(void);

#endif
