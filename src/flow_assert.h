#pragma once
#include <exception>

#ifdef FLOW_DISABLE_RUNTIME_WARNINGS
#define warn_assert(cond,msg)
#else
#define warn_assert(cond,msg) (cond) || \
    printf("Warning %s:%i, %s\n", __FILE__, __LINE__, (msg))
#endif

#define throw_assert(cond,msg) do { if (not (cond)) { \
    printf("Error %s:%i, %s\n", __FILE__, __LINE__, (msg)); \
    throw std::logic_error(msg); \
    } } while (0)