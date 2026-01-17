#include "test.h"

EK_LOG_FILE_TAG("vec_test.c");

void vec_test(void)
{
    EK_VEC_IMPLEMENT(float);

    ek_vec_t(float) v;

    ek_vec_init(v);

    EK_LOG_INFO("append vec");
    for (int i = 0; i < 10; i++)
    {
        ek_vec_append(v, i * 5);
        EK_LOG("cap:%u", v.cap);
    }

    EK_LOG_INFO("walk vec");
    uint32_t i;
    ek_vec_iterate(i, v)
    {
        EK_LOG("vec[%d] = %f", i, v.items[i]);
    }

    EK_LOG_INFO("walk vec randomly");
    for (i = 0; i < 10; i++)
    {
        uint32_t temp = rand() % v.amount;
        EK_LOG("vec[%d] = %f", temp, v.items[temp]);
    }

    EK_LOG_INFO("vec remove index 2");
    ek_vec_remove(v, 2);

    ek_vec_iterate(i, v)
    {
        EK_LOG("vec[%d] = %f,cap = %u", i, v.items[i], v.cap);
    }

    EK_LOG_INFO("remove items until vec`s amount is 4");

    while (v.amount > 4)
    {
        ek_vec_remove(v, 0);
    }

    ek_vec_iterate(i, v)
    {
        EK_LOG("vec[%d] = %f,cap = %u", i, v.items[i], v.cap);
    }

    EK_LOG_INFO("vec shrink");
    ek_vec_shrink(v);
    ek_vec_iterate(i, v)
    {
        EK_LOG("vec[%d] = %f,cap = %u", i, v.items[i], v.cap);
    }

    EK_LOG_INFO("vec clear");
    ek_vec_clear(v);
    ek_vec_iterate(i, v)
    {
        EK_LOG("vec[%d] = %f,cap = %u", i, v.items[i], v.cap);
    }

    EK_LOG_INFO("append vec");

    for (int i = 0; i < 10; i++)
    {
        ek_vec_append(v, i * 5);
    }

    ek_vec_iterate(i, v)
    {
        EK_LOG("vec[%d] = %f,cap = %u", i, v.items[i], v.cap);
    }

    EK_LOG_INFO("O(1) remove");

    ek_vec_remove_unorder(v, 2);

    ek_vec_iterate(i, v)
    {
        EK_LOG("vec[%d] = %f,cap = %u", i, v.items[i], v.cap);
    }

    EK_LOG_INFO("free vec");
    EK_LOG_INFO("before free used:%u unused:%u", ek_heap_used(), ek_heap_unused());
    EK_LOG_INFO("vec cap:%u take size %u bytes", v.cap, v.cap * sizeof(int));
    ek_vec_destroy(v);
    EK_LOG_INFO("after free used:%u unused:%u", ek_heap_used(), ek_heap_unused());
}
