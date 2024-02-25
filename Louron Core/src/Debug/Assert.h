#pragma once

// Louron Core Headers
#include "../Core/Logging.h"

// C++ Standard Library Headers
#include <iostream>

// External Vendor Library Headers

#ifdef NDEBUG

	#define L_CORE_ASSERT(condition, message) ((void)0)

#else

    #define L_CORE_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                L_CORE_FATAL("Assertion Failure Msg: {0}", message); \
                L_CORE_FATAL("Assertion Failure File: {0}", __FILE__); \
                L_CORE_FATAL("Assertion Failure Line: {0}", __LINE__); \
                std::terminate(); \
            } \
        } while (false)

#endif