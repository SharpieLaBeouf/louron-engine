#pragma once

// Louron Core Headers

// C++ Standard Library Headers
#include <iostream>

// External Vendor Library Headers

#ifdef NDEBUG

	#define L_CORE_ASSERT(condition, message) ((void)0)

#else

    #define L_CORE_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "[L20] Assertion Failure Msg: " << message << std::endl; \
                std::cerr << "[L20] Assertion Failure File: " << __FILE__ << std::endl; \
                std::cerr << "[L20] Assertion Failure Line: " << __LINE__ << std::endl; \
                std::terminate(); \
            } \
        } while (false)

#endif