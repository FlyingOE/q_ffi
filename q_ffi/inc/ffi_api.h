#pragma once
#include "q_ffi.h"
#include <k_compat.h>

q_ffi_EXTERN q_ffi_API
K K4_DECL version(K /*`2:' requires >=1 args*/);

q_ffi_EXTERN q_ffi_API
K K4_DECL load_fun(K dllSym, K funName, K abi, K ret, K args);

q_ffi_EXTERN q_ffi_API
K K4_DECL get_var(K dllSym, K varName, K typ);

q_ffi_EXTERN q_ffi_API
K K4_DECL set_var(K dllSym, K varName, K typ, K val);

q_ffi_EXTERN q_ffi_API
K K4_DECL size_type(K /*`2:' requires >=1 args*/);

q_ffi_EXTERN q_ffi_API
K K4_DECL ptr_type(K /*`2:' requires >=1 args*/);

q_ffi_EXTERN q_ffi_API
K K4_DECL get_addr(K k);