#ifndef EK_DEF_H
#define EK_DEF_H

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#define EK_ENUM(name, type) \
    typedef type name;      \
    enum

#endif /* EK_DEF_H */
