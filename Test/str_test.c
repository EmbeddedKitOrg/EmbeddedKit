#include "test.h"

EK_LOG_FILE_TAG("str_test.c");

void str_test(void)
{
    EK_LOG_INFO("str test start");

    EK_LOG_INFO("make an empty str");
    ek_str_t *s = ek_str_create(NULL);
    EK_LOG_INFO("s:%s", ek_str_get_cstring(s));
    EK_LOG_INFO("before free:%zu used:%zu", ek_heap_unused(), ek_heap_used());
    ek_str_free(s);
    EK_LOG_INFO("after free:%zu, used:%zu", ek_heap_unused(), ek_heap_used());

    const char *i_am_a_string = "i am a string";
    EK_LOG_INFO("make a str:%s", i_am_a_string);
    s = ek_str_create(i_am_a_string);
    EK_LOG_INFO("s:%s", ek_str_get_cstring(s));
    EK_LOG_INFO("before free:%zu used:%zu", ek_heap_unused(), ek_heap_used());
    ek_str_free(s);
    EK_LOG_INFO("after free:%zu, used:%zu", ek_heap_unused(), ek_heap_used());

    const char *hello = "hello";
    EK_LOG_INFO("make a str:%s", hello);
    s = ek_str_create(hello);
    EK_LOG_INFO("s:%s", ek_str_get_cstring(s));
    EK_LOG_INFO("str append");
    ek_str_append(s, " world!");
    EK_LOG_INFO("s:%s", ek_str_get_cstring(s));

    const char *embeddekit = "EmbeddedKit";
    EK_LOG_INFO("before append:%s", ek_str_get_cstring(s));
    EK_LOG_INFO("append:%s", embeddekit);
    ek_str_append_fmt(s, "%s", embeddekit);
    EK_LOG_INFO("after append:%s", ek_str_get_cstring(s));

    EK_LOG_INFO("slice \"%s\",at [12:%u] ", ek_str_get_cstring(s), ek_str_get_len(s));
    ek_str_t *slice = ek_str_slice(s, 12, ek_str_get_len(s));
    EK_LOG_INFO("slice:%s", ek_str_get_cstring(slice));
    ek_str_free(slice);

    const char *n1netynine99 = " N1netyNine99";
    EK_LOG_INFO("cat test");
    EK_LOG_INFO("make a new string:%s", n1netynine99);
    ek_str_t *s1 = ek_str_create(n1netynine99);
    EK_LOG_INFO("string:%s", ek_str_get_cstring(s));
    EK_LOG_INFO("cat start");
    ek_str_cat(s, s1);
    EK_LOG_INFO("cat result:%s", ek_str_get_cstring(s));

    ek_str_free(s);
    ek_str_free(s1);
    EK_LOG_INFO("string finished,unused heap:%zu", ek_heap_unused());
    EK_LOG_INFO("string finished,used heap:%zu", ek_heap_used());
}
