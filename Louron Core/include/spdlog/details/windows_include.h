#pragma once

#ifndef NOMINMAX
    #define NOMINMAX  // prevent windows redefining min/max
#endif

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#if defined(APIENTRY)
    #undef APIENTRY
#endif
#include <windows.h>
