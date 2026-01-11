#include "../inc/ek_ringbuf.h"

#if EK_RINGBUF_ENABLE == 1

#    include "../inc/ek_mem.h"
#    include "../inc/ek_assert.h"

#    if EK_USE_RTOS == 1
#        define EK_LOCKUP(prb)    ((prb)->lock = true)
#        define EK_UNLOCK(prb)    ((prb)->lock = false)
#        define EK_LOCK_TEST(prb) ((prb)->lock == true)
#    else
#        define EK_LOCKUP(prb)
#        define EK_UNLOCK(prb)
#        define EK_LOCK_TEST(prb) (false)
#    endif /* EK_USE_RTOS */

bool ek_ringbuf_full(const ek_ringbuf_t *rb)
{
    ek_assert_param(rb != NULL);
    return rb->item_amount == rb->cap;
}

bool ek_ringbuf_empty(const ek_ringbuf_t *rb)
{
    ek_assert_param(rb != NULL);
    return rb->item_amount == 0;
}

ek_ringbuf_t *ek_ringbuf_create(size_t item_size, uint32_t item_amount)
{
    ek_assert_param(item_amount != 0);
    ek_assert_param(item_size != 0);

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
#    if EK_USE_RTOS == 1
    rb->lock = false;
#    endif /* EK_USE_RTOS */

    return rb;
}

void ek_ringbuf_destroy(ek_ringbuf_t *rb)
{
    ek_assert_param(rb != NULL);

    ek_free(rb->buffer);
    ek_free(rb);
}

bool ek_ringbuf_write(ek_ringbuf_t *rb, const void *item)
{
    ek_assert_param(item != NULL);
    ek_assert_param(rb != NULL);

    if (EK_LOCK_TEST(rb) == true) return false;

    EK_LOCKUP(rb);

    if (ek_ringbuf_full(rb) == true)
    {
        EK_UNLOCK(rb);
        return false;
    }

    uint8_t *target = rb->buffer + (rb->write_idx * rb->item_size);
    memcpy(target, item, rb->item_size);

    rb->write_idx = (rb->write_idx + 1) % rb->cap;
    rb->item_amount++;

    EK_UNLOCK(rb);

    return true;
}

bool ek_ringbuf_read(ek_ringbuf_t *rb, void *item)
{
    ek_assert_param(rb != NULL);

    if (EK_LOCK_TEST(rb) == true) return false;

    EK_LOCKUP(rb);

    if (ek_ringbuf_empty(rb) == true)
    {
        EK_UNLOCK(rb);
        return false;
    }

    if (item != NULL)
    {
        const uint8_t *source = rb->buffer + (rb->read_idx * rb->item_size);
        memcpy(item, source, rb->item_size);
    }

    rb->read_idx = (rb->read_idx + 1) % rb->cap;
    rb->item_amount--;

    EK_UNLOCK(rb);

    return true;
}

bool ek_ringbuf_peek(ek_ringbuf_t *rb, void *item)
{
    ek_assert_param(item != NULL);
    ek_assert_param(rb != NULL);

    if (EK_LOCK_TEST(rb) == true) return false;

    EK_LOCKUP(rb);

    if (ek_ringbuf_empty(rb) == true)
    {
        EK_UNLOCK(rb);
        return false;
    }

    const uint8_t *source = rb->buffer + (rb->read_idx * rb->item_size);
    memcpy(item, source, rb->item_size);

    EK_UNLOCK(rb);

    return true;
}

#endif /* EK_RINGBUF_ENABLE */
