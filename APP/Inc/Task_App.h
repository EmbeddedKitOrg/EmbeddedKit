#ifndef __TASK_APP_H
#define __TASK_APP_H

#include "Common.h"

#define USER_HUART &huart1

typedef struct
{
    char *buf;
    uint16_t len;
} Message_t;

void TaskCreation(void);

#endif /* __TASK_APP_H */

