#pragma once

#include <stdexcept>
#include <stdio.h>
#include <string>

#ifdef FLOW_DISABLE_RUNTIME_WARNINGS
#define warn_assert(cond,msg)
#else
#define warn_assert(cond,msg) (cond) || \
    printf("Warning %s:%i, %s\n", __FILE__, __LINE__, (msg))
#endif

#define throw_assert(cond,msg) do { if (not (cond)) { \
    throw_assert_(msg, __FILE__, __LINE__); \
    } } while (0)

inline void throw_assert_(const char* msg, const char* f, int l)
{
    char* e = nullptr;
    asprintf(&e, "Error %s:%i, %s", f, l, msg);
    std::string err(e);
    free(e);
    throw std::logic_error(err);
}
