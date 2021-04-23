#ifndef LNA_CORE_LNA_ASSERT_H
#define LNA_CORE_LNA_ASSERT_H

#include <stdio.h>
#include <stdlib.h>
#include "core/lna_log.h"

#define LNA_STR(x) #x
#define lna_assert(x) if(!(x)) { lna_log_error("assert: %s (%s::%s::%d)", LNA_STR(x), __FILE__, __PRETTY_FUNCTION__, __LINE__); getchar(); abort(); }

#endif
