#include "q_ffi.h"
#include "version.hpp"

#define STR(s)  #s
#define XSTR(s) STR(s)
char const* const q_ffi::version = "v" XSTR(q_ffi_VERSION) "-" XSTR(q_ffi_GIT_HASH) " (" __DATE__ " " __TIME__ ")";