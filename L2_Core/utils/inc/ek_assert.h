#ifndef EK_ASSERT_H
#define EK_ASSERT_H

#include "../../../ek_conf.h"
#include "ek_def.h"

#ifndef EK_ASSERT_USE_TINY
#    define EK_ASSERT_USE_TINY (1)
#endif /* EK_ASSERT_USE_TINY */

#ifndef EK_ASSERT_WITH_LOG
#    define EK_ASSERT_WITH_LOG (0)
#endif /* EK_ASSERT_WITH_LOG */

#define ek_assert_tiny(expr) while ((expr) == 0)
#define ek_assert_full(expr) (expr) ? (void)0 : ek_assert_fault(__FILE__, __LINE__, #expr)

#if EK_ASSERT_USE_TINY == 1
#    define ek_assert_param(expr) ek_assert_tiny(expr)
#else
#    defiine ek_assert_param(expr) ek_assfuek_assert_full(expr)
#endif /* EK_ASSERT_USE_TINY */

void ek_assert_fault(const char *file, uint32_t line, const char *expr);

#endif /* EK_ASSERT_H */
