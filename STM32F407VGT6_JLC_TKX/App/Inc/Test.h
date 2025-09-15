#ifndef __TEST_H
#define __TEST_H

#include "SysConfig.h"

typedef struct
{
    const char *Test_Name;
    void (*Test_Fucntion)(void);
} EK_Test_t;

void Test_Queue(void);
void Test_List(void);
void Test_MemPool(void);
void Test_Queue(void);
void Test_TaskSystem(void);
void Test_AllTaskInfo(void);
void Test_Stack(void);

extern EK_Test_t Tst_List;
extern EK_Test_t Tst_MemPool;
extern EK_Test_t Tst_Queue;
extern EK_Test_t Tst_Stack;
extern EK_Test_t Tst_Task1;
extern EK_Test_t Tst_Task2;

#endif
