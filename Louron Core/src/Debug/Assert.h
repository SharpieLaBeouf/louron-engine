#pragma once

#include <ostream>

#ifdef NDEBUG

	#define L_CORE_ASSERT(condition, message) ((void)0)

#else

    #define L_CORE_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "[L20] Assertion failed: " << message << " in file " << __FILE__ << " on line " << __LINE__ << std::endl; \
                std::terminate(); \
            } \
        } while (false)

#endif