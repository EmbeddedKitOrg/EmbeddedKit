/**
 * @file ek_ringbuf.c
 * @brief 环形缓冲区实现
 * @author N1netyNine99
 */

#include "ek_ringbuf.h"

#if (EK_RINGBUF_ENABLE == 1) || (EK_RINGBUF_SPSC_ENABLE == 1)

#    include "ek_mem.h"
#    include "ek_assert.h"

#    if EK_RINGBUF_ENABLE == 1
#        if EK_USE_RTOS == 1
#            define EK_LOCKUP(prb)    ((prb)->lock = true)
#            define EK_UNLOCK(prb)    ((prb)->lock = false)
#            define EK_LOCK_TEST(prb) ((prb)->lock == true)
#        else
#            define EK_LOCKUP(prb)
#            define EK_UNLOCK(prb)
#            define EK_LOCK_TEST(prb) (false)
#        endif /* EK_USE_RTOS */

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
#        if EK_USE_RTOS == 1
    rb->lock = false;
#        endif /* EK_USE_RTOS */

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
#    endif /* EK_RINGBUF_ENABLE */

#    if EK_RINGBUF_SPSC_ENABLE == 1
__EK_STATIC_INLINE uint32_t _ek_ringbuf_spsc_next_idx(const ek_ringbuf_spsc_t *rb, uint32_t idx)
{
    return (idx + 1U) % rb->cap;
}

bool ek_ringbuf_full_spsc(const ek_ringbuf_spsc_t *rb)
{
    ek_assert_param(rb != NULL);
    return _ek_ringbuf_spsc_next_idx(rb, rb->write_idx) == rb->read_idx;
}

bool ek_ringbuf_empty_spsc(const ek_ringbuf_spsc_t *rb)
{
    ek_assert_param(rb != NULL);
    return rb->read_idx == rb->write_idx;
}

ek_ringbuf_spsc_t *ek_ringbuf_create_spsc(size_t item_size, uint32_t item_amount)
{
    ek_assert_param(item_amount > 1U);
    ek_assert_param(item_size != 0U);

    ek_ringbuf_spsc_t *rb = (ek_ringbuf_spsc_t *)ek_malloc(sizeof(ek_ringbuf_spsc_t));
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
    rb->read_idx = 0U;
    rb->write_idx = 0U;

    return rb;
}

void ek_ringbuf_destroy_spsc(ek_ringbuf_spsc_t *rb)
{
    ek_assert_param(rb != NULL);

    ek_free(rb->buffer);
    ek_free(rb);
}

bool ek_ringbuf_write_spsc(ek_ringbuf_spsc_t *rb, const void *item)
{
    ek_assert_param(rb != NULL);
    ek_assert_param(item != NULL);

    uint32_t next_idx = _ek_ringbuf_spsc_next_idx(rb, rb->write_idx);
    if (next_idx == rb->read_idx)
    {
        return false;
    }

    uint8_t *target = rb->buffer + (rb->write_idx * rb->item_size);
    memcpy(target, item, rb->item_size);
    rb->write_idx = next_idx;

    return true;
}

bool ek_ringbuf_read_spsc(ek_ringbuf_spsc_t *rb, void *item)
{
    ek_assert_param(rb != NULL);

    if (rb->read_idx == rb->write_idx)
    {
        return false;
    }

    if (item != NULL)
    {
        const uint8_t *source = rb->buffer + (rb->read_idx * rb->item_size);
        memcpy(item, source, rb->item_size);
    }

    rb->read_idx = _ek_ringbuf_spsc_next_idx(rb, rb->read_idx);

    return true;
}

bool ek_ringbuf_peek_spsc(ek_ringbuf_spsc_t *rb, void *item)
{
    ek_assert_param(rb != NULL);
    ek_assert_param(item != NULL);

    if (rb->read_idx == rb->write_idx)
    {
        return false;
    }

    const uint8_t *source = rb->buffer + (rb->read_idx * rb->item_size);
    memcpy(item, source, rb->item_size);

    return true;
}
#    endif /* EK_RINGBUF_SPSC_ENABLE */

#endif /* EK_RINGBUF_ENABLE || EK_RINGBUF_SPSC_ENABLE */
