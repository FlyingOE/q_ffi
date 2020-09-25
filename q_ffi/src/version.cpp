#include "q_ffi_config.h"
#include "q_ffi.h"
#include "version.hpp"

#define STRINGIZE(s) #s
#define XSTR(s)      STRINGIZE(s)

char const* const q_ffi::version =
    "v" XSTR(q_ffi_VERSION) " #" XSTR(q_ffi_GIT_HASH) " (" __DATE__ " " __TIME__ ")";