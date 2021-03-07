#ifndef _LNA_CORE_ASSERT_HPP_
#define _LNA_CORE_ASSERT_HPP_

#include <cstdlib>
#include "core/log.hpp"

#define LNA_STR(x) #x
#define LNA_ASSERT(x) if(!(x)) { lna::log::error("assert: %s (%s::%s::%d)", LNA_STR(x), __FILE__, __PRETTY_FUNCTION__, __LINE__); abort(); }

#endif // _LNA_CORE_ASSERT_HPP_
