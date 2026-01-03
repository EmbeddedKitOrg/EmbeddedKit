#include "../inc/ek_stack.h"
#include "../inc/ek_assert.h"
#include "../inc/ek_mem.h"

bool ek_stack_full(ek_stack_t *sk)
{
    EK_ASSERT(sk != NULL);
    return sk->sp >= sk->capacity;
}

bool ek_stack_empty(ek_stack_t *sk)
{
    EK_ASSERT(sk != NULL);
    return sk->sp == 0;
}

ek_stack_t *ek_stack_create(size_t item_size, uint32_t item_mount)
{
    EK_ASSERT(item_mount != 0);
    EK_ASSERT(item_size != 0);

    ek_stack_t *sk = (ek_stack_t *)ek_malloc(sizeof(ek_stack_t));
    if (sk == NULL)
    {
        return NULL;
    }
    sk->buffer = ek_malloc(item_mount * item_size);
    if (sk->buffer == NULL)
    {
        ek_free(sk);
        return NULL;
    }

    sk->sp = 0;
    sk->capacity = item_mount;
    sk->item_size = item_size;

    return sk;
}

void ek_stack_destroy(ek_stack_t *sk)
{
    EK_ASSERT(sk != NULL);
    
    sk->capacity = 0;
    sk->item_size = 0;
    ek_free(sk->buffer);
    ek_free(sk);
}

bool ek_stack_push(ek_stack_t *sk, const void *item)
{
    EK_ASSERT(sk != NULL);
    EK_ASSERT(item != NULL);

    if(ek_stack_full(sk) == true)return false;
    
    uint8_t *target = sk->buffer + sk->sp * sk->item_size;
    memcpy(target, item, sk->item_size);
    sk->sp ++;

    return true;
}

bool ek_stack_pop(ek_stack_t *sk, void *item)
{
    EK_ASSERT(sk != NULL);
    EK_ASSERT(item != NULL);
    
    if(ek_stack_empty(sk) == true)return false;

    sk->sp --;
    uint8_t *source = sk->buffer + sk->sp * sk->item_size;
    memcpy(item, source, sk->item_size);
    
    return true;
}
