#pragma once
/* vim: set et noai ts=4 sw=4: */

// Silence various standard-compliance issues in <k.h>
#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4201 )
#elif defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wpedantic"
#endif

#define KXVER 3
#include <k.h>

#if defined(_MSC_VER)
#	pragma warning( pop )
#elif defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif