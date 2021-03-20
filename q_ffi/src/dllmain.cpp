#include "q_ffi.h"
#include <cassert>
#include <mutex>
#ifndef NDEBUG
#   include <cstdio>
#   ifdef WIN32
#       define q_ffi_DLL "q_ffi.dll"
#   else
#       define q_ffi_DLL "libq_ffi.so"
#   endif
#endif
#include <k_compat.h>

namespace
{
#   if WIN32
#       define DLL_CONSTRUCTOR
#       define DLL_DESTRUCTOR
#   else
#       define DLL_CONSTRUCTOR __attribute__((constructor))
#       define DLL_DESTRUCTOR  __attribute__((destructor))
#   endif

    DLL_CONSTRUCTOR void dllOnLoad() noexcept
    {
        static std::once_flag onLoad;
        // @ref https://code.kx.com/q/interfaces/c-client-for-q/#managing-memory-and-reference-counting
        std::call_once(onLoad, []() {
#       ifndef NDEBUG
            std::printf("# <" q_ffi_DLL "> loading...\n");
#       endif
            ::setm(1);
        });
    }

    DLL_DESTRUCTOR void dllOnUnload() noexcept
    {
        static std::once_flag onUnload;
        std::call_once(onUnload, []() {
#       ifndef NDEBUG
            std::printf("# <" q_ffi_DLL "> unloading...\n");
#       endif
        });
    }

}//namespace /*anonymous*/

#if WIN32
BOOL APIENTRY DllMain(HMODULE /*hModule*/,
    DWORD  ul_reason_for_call,
    LPVOID /*lpReserved*/
) {
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        dllOnLoad();
        break;
    case DLL_THREAD_ATTACH:
        /*fallthrough*/
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        dllOnUnload();
        break;
    default:
        assert(!"Unexpected ul_reason_for_call in DllMain");
    }
    return TRUE;
}
#endif
