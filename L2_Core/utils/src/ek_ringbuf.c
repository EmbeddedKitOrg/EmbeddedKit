#include "../inc/ek_ringbuf.h"

#if EK_RINGBUF_ENABLE == 1

#    include "../inc/ek_mem.h"
#    include "../inc/ek_assert.h"

bool ek_ringbuf_full(const ek_ringbuf_t *rb)
{
    EK_ASSERT(rb != NULL);
    return rb->item_amount == rb->cap;
}

bool ek_ringbuf_empty(const ek_ringbuf_t *rb)
{
    EK_ASSERT(rb != NULL);
    return rb->item_amount == 0;
}

ek_ringbuf_t *ek_ringbuf_create(size_t item_size, uint32_t item_amount)
{
    EK_ASSERT(item_amount != 0);
    EK_ASSERT(item_size != 0);

    ek_ringbuf_t *rb = (ek_ringbuf_t *)ek_malloc(sizeof(ek_ringbuf_t));
    if (rb == NULL)
    {
        return NULL;
    }
    rb->buffer = (uint8_t *)ek_malloc(item_amount * item_size);
    if (rb->buffer == NULL)
    {
        ek_free(rb);
        return NULL;
    }

    rb->cap = item_amount;
    rb->item_size = item_size;
    rb->read_idx = 0;
    rb->write_idx = 0;
    rb->item_amount = 0;

    return rb;
}

void ek_ringbuf_destroy(ek_ringbuf_t *rb)
{
    EK_ASSERT(rb != NULL);

    ek_free(rb->buffer);
    ek_free(rb);
}

bool ek_ringbuf_write(ek_ringbuf_t *rb, const void *item)
{
    EK_ASSERT(item != NULL);
    EK_ASSERT(rb != NULL);

    if (ek_ringbuf_full(rb) == true) return false;

    uint8_t *target = rb->buffer + (rb->write_idx * rb->item_size);
    memcpy(target, item, rb->item_size);

    rb->write_idx = (rb->write_idx + 1) % rb->cap;
    rb->item_amount++;

    return true;
}

bool ek_ringbuf_read(ek_ringbuf_t *rb, void *item)
{
    EK_ASSERT(rb != NULL);

    if (ek_ringbuf_empty(rb) == true)
    {
        return false;
    }

    if (item != NULL)
    {
        const uint8_t *source = rb->buffer + (rb->read_idx * rb->item_size);
        memcpy(item, source, rb->item_size);
    }

    rb->read_idx = (rb->read_idx + 1) % rb->cap;
    rb->item_amount--;

    return true;
}

bool ek_ringbuf_peek(const ek_ringbuf_t *rb, void *item)
{
    EK_ASSERT(item != NULL);
    EK_ASSERT(rb != NULL);

    if (ek_ringbuf_empty(rb) == true)
    {
        return false;
    }

    const uint8_t *source = rb->buffer + (rb->read_idx * rb->item_size);
    memcpy(item, source, rb->item_size);

    return true;
}

#endif /* EK_RINGBUF_ENABLE */
