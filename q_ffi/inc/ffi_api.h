#pragma once
#include "q_ffi.h"
#include <k_compat.h>

q_ffi_EXTERN q_ffi_API
K K4_DECL load(K dllSym, K fName, K abi, K ret, K args);

q_ffi_EXTERN q_ffi_API
K K4_DECL version(K /*2: requires >= 1 arg*/);