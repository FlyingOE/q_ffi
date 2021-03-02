#pragma once
/* vim: set et noai ts=4 sw=4 syntax=cpp: */

// C++/C name mangling
#ifdef __cplusplus
#   define q_ffi_EXTERN  extern "C"
#else
#   define q_ffi_EXTERN
#endif

// DLL export nuance
#ifdef _WIN32
#   ifdef q_ffi_EXPORTS
#   define q_ffi_API  __declspec(dllexport)
#   else
#   define q_ffi_API  __declspec(dllimport)
#   endif
#   define K4_DECL  __cdecl
#else
#   define q_ffi_API
#   define K4_DECL
#endif

// Platform-specific headers
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <windows.h>
#endif
