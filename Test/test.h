#ifndef TEST_H
#define TEST_H

#include <stdlib.h>

#include "ek_io.h"
#include "ek_mem.h"
#include "ek_def.h"
#include "ek_stack.h"
#include "ek_log.h"
#include "ek_ringbuf.h"
#include "ek_vec.h"
#include "ek_assert.h"
#include "ek_str.h"

#define PI (3.141592f)

typedef struct
{
    uint8_t name;
    float score;
} student_t;

void stack_test(void);
void ringbuf_test(void);
void vec_test(void);
void str_test(void);

#endif
