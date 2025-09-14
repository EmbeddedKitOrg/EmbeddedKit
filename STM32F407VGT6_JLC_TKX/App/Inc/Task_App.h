#ifndef __Task_App_h
#define __Task_App_h

#include "SysConfig.h"

extern EK_pTaskHandler_t TskHandler1;
extern EK_pTaskHandler_t TskHandler2;
extern EK_pTaskHandler_t TskHandler3;
extern EK_pTaskHandler_t TskHandler_LargeListSort;
extern EK_pSeiralQueue_t TestQueue;

// 测试数据结构
extern EK_List_t *test_list;
extern EK_Queue_t *test_queue;
extern uint8_t queue_buffer[100];
extern EK_Queue_t static_queue;
extern EK_Stack_t *test_stack;
extern uint8_t stack_buffer[100];
extern EK_Stack_t static_stack;

// 测试计数器
extern uint32_t test_counter;
// 测试结果统计
extern uint32_t test_success_count;
extern uint32_t test_failure_count;

#endif
