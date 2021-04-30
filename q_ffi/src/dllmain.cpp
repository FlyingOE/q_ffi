#include "q_ffi.h"
#include <cassert>
#include <mutex>
#ifndef NDEBUG
#   include <cstdio>
#endif
#ifdef _WIN32
#   define WIN32_LEAN_AND_MEAN
#   define NOMINMAX
#   include <windows.h>
#endif
#include <k_compat.h>

using namespace std;

namespace
{
#ifdef _WIN32
#   define DLL_CONSTRUCTOR
#   define DLL_DESTRUCTOR
#else
#   define DLL_CONSTRUCTOR __attribute__((constructor))
#   define DLL_DESTRUCTOR  __attribute__((destructor))
#endif

    DLL_CONSTRUCTOR void dllOnLoad() noexcept
    {
        static once_flag onLoad;
        // @ref https://code.kx.com/q/interfaces/c-client-for-q/#managing-memory-and-reference-counting
        call_once(onLoad, []() {
#       ifndef NDEBUG
            printf("# <q_ffi> loading...\n");
#       endif
            ::setm(1);
        });
    }

    DLL_DESTRUCTOR void dllOnUnload() noexcept
    {
        static once_flag onUnload;
        call_once(onUnload, []() {
#       ifndef NDEBUG
            printf("# <q_ffi> unloading...\n");
#       endif
        });
    }

}//namespace /*anonymous*/

#ifdef _WIN32
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
