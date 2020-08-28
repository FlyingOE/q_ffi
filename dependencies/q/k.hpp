#pragma once
// Work-around various standard-compliance issues in <k.h>

#if defined(_MSC_VER)
#	pragma warning( push )
#	pragma warning( disable: 4201 )
#elif defined(__GNUC__)
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wpedantic"
#endif

#include <k.h>

#if defined(_MSC_VER)
#	pragma warning( pop )
#elif defined(__GNUC__)
#	pragma GCC diagnostic pop
#endif
