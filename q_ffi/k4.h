#pragma once
// Work-around various standard-compliance issues in <k.h>

#ifdef _MSC_VER
#	pragma warning( push )
#	pragma warning( disable: 4201 )
#endif

#include <k.h>

#ifdef _MSC_VER
#	pragma warning( pop )
#endif
