#include "test.h"

EK_LOG_FILE_TAG("stack_test.c");

void stack_test(void)
{
    EK_LOG_INFO("start creating a stack");
    ek_stack_t *stack = ek_stack_create(sizeof(student_t), 5);

    if (stack == NULL)
    {
        EK_LOG_ERROR("creating stack fail");
        exit(1);
    }

    EK_LOG_INFO("create successfully");

    EK_LOG_INFO("start creating items to push in stack");

    for (uint8_t i = 0; i < 10; i++)
    {
        student_t *new = malloc(sizeof(*new));
        if (new == NULL)
        {
            EK_LOG_ERROR("creating item fail,loop works:%d", i);
            exit(1);
        }

        new->name = i;
        new->score = i *PI;

        bool ret;
        ret = ek_stack_push(stack, new);
        if (ret == true)
        {
            EK_LOG("i:%d push item{name:%d,score:%.2f} sp:%u", i, new->name, new->score, stack->sp);
        }
        else
        {
            EK_LOG_ERROR("stack full:%u", stack->sp);
        }
    }

    EK_LOG_INFO("push finished.mem remain:%u", ek_heap_unused());
    EK_LOG_INFO("statr poping form stack");

    for (uint8_t i = 0; i < 10; i++)
    {
        student_t item;

        bool ret;
        ret = ek_stack_pop(stack, &item);
        if (ret == true)
        {
            EK_LOG("i:%d pop item{name:%d,score:%.2f} sp:%u", i, item.name, item.score, stack->sp);
        }
        else
        {
            EK_LOG_ERROR("stack empty:%u", stack->sp);
        }
    }
    ek_stack_destroy(stack);
    EK_LOG_INFO("test finised.mem remain:%u", ek_heap_unused());
}
