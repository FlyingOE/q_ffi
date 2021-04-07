#pragma once
#include "q_ffi.h"
#include <k_compat.h>

q_ffi_EXTERN q_ffi_API
K K4_DECL loadFun(K dllSym, K funName, K abi, K ret, K args);

q_ffi_EXTERN q_ffi_API
K K4_DECL getVar(K dllSym, K varName, K typ);

q_ffi_EXTERN q_ffi_API
K K4_DECL setVar(K dllSym, K varName, K typ, K val);

q_ffi_EXTERN q_ffi_API
K K4_DECL version(K /*2: requires >= 1 arg*/);