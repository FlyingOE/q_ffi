#include "q_ffi.h"

#include "misc.hpp"

constexpr char const* q_ffi::version()
{
#   define STR(s) #s
    return STR(q_ffi_VERSION) " (" __DATE__ ")";
}