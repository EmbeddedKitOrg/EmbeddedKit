#include "test.h"

EK_LOG_FILE_TAG("ringbuf_test.c")

void ringbuf_test()
{
    EK_LOG_INFO("start creating a rb");
    ek_ringbuf_t *rb = ek_ringbuf_create(sizeof(student_t), 5);

    if (rb == NULL)
    {
        EK_LOG_ERROR("creating rb fail");
        exit(1);
    }

    EK_LOG_INFO("create successfully");

    EK_LOG_INFO("start creating items to push in rb");

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
        ret = ek_ringbuf_write(rb, new);
        if (ret == true)
        {
            EK_LOG_INFO("i:%d push item{name:%d,score:%.2f} ridx:%u widx:%u",
                        i,
                        new->name,
                        new->score,
                        rb->read_idx,
                        rb->write_idx);
        }
        else
        {
            EK_LOG_WARN("full ridx:%u widx:%u", rb->read_idx, rb->write_idx);
        }
    }

    EK_LOG_INFO("push finished.mem remain:%u", ek_heap_unused());
    EK_LOG_INFO("statr poping form rb");

    for (uint8_t i = 0; i < 10; i++)
    {
        student_t item;

        bool ret;
        ret = ek_ringbuf_read(rb, &item);
        if (ret == true)
        {
            EK_LOG_INFO("i:%d pop item{name:%d,score:%.2f} ridx:%u widx:%u",
                        i,
                        item.name,
                        item.score,
                        rb->read_idx,
                        rb->write_idx);
        }
        else
        {
            EK_LOG_WARN("empty ridx:%u widx:%u", rb->read_idx, rb->write_idx);
        }
    }
    ek_ringbuf_destroy(rb);
    EK_LOG_INFO("test finised.mem remain:%u", ek_heap_unused());
}
