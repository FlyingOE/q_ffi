#pragma once
#include "q_ffi.h"
#include <k_compat.h>

q_ffi_EXTERN q_ffi_API
K K4_DECL version(K /*`2:' requires >=1 args*/);

#pragma region FFI invocation

q_ffi_EXTERN q_ffi_API
K K4_DECL load_fun(K dllSym, K funName, K abi, K ret, K args);

q_ffi_EXTERN q_ffi_API
K K4_DECL get_var(K dllSym, K varName, K typ);

/// Writting to DLL variables is susceptible to values being overwritten during DLL reload.
/// Use with caution!
q_ffi_EXTERN q_ffi_API
K K4_DECL set_var(K dllSym, K varName, K val);

#pragma endregion

#pragma region FFI pointer handling

q_ffi_EXTERN q_ffi_API
K K4_DECL addr_of(K k);

q_ffi_EXTERN q_ffi_API
K K4_DECL get_from_addr(K addr, K typ);

q_ffi_EXTERN q_ffi_API
K K4_DECL set_to_addr(K addr, K val);

#pragma endregion

#pragma region FFI type mapping

q_ffi_EXTERN q_ffi_API
K K4_DECL size_type(K /*`2:' requires >=1 args*/);

q_ffi_EXTERN q_ffi_API
K K4_DECL ptr_type(K /*`2:' requires >=1 args*/);

#pragma endregion
