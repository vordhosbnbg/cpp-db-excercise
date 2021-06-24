#pragma once

#ifdef __GNUG__
#include <cxxabi.h>
#define DEMANGLE(x) abi::__cxa_demangle(x, NULL, NULL, NULL)
#else
#define DEMANGLE(x) x
#endif
