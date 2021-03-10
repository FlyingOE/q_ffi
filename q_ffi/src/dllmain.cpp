#include "q_ffi.h"
#include <cassert>
#include <mutex>
#ifndef NDEBUG
#   include <iostream>
#   include <thread>
#   ifdef WIN32
#       define q_ffi_DLL "q_ffi.dll"
#   else
#       define q_ffi_DLL "q_ffi.so"
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
            std::cout << "# <" q_ffi_DLL "> loading..." << std::endl;
#       endif
            ::setm(1);
        });
    }

    DLL_DESTRUCTOR void dllOnUnload() noexcept
    {
        static std::once_flag onUnload;
        std::call_once(onUnload, []() {
#       ifndef NDEBUG
            std::cout << "# <" q_ffi_DLL "> unloading..." << std::endl;
#       endif
        });
    }

    void dllOnThreadEnter() noexcept
    {
        static thread_local std::once_flag onThreadEnter;
        std::call_once(onThreadEnter, []() {
#       ifndef NDEBUG
            std::cout << "# <" q_ffi_DLL "> entering thread "
                << std::this_thread::get_id() << "..." << std::endl;
#       endif
        });
    }

    void dllOnThreadExit() noexcept
    {
        static thread_local std::once_flag onThreadExit;
        // @ref https://code.kx.com/q/interfaces/c-client-for-q/#managing-memory-and-reference-counting
        std::call_once(onThreadExit, []() {
#       ifndef NDEBUG
            std::cout << "# <" q_ffi_DLL "> exiting thread "
                << std::this_thread::get_id() << "..." << std::endl;
#       endif
            ::m9();
        });
    }

}//namespace /*anonymous*/

#if WIN32
BOOL APIENTRY DllMain(HMODULE /*hModule*/,
    DWORD  ul_reason_for_call,
    LPVOID /*lpReserved*/
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        dllOnLoad();
        break;
    case DLL_THREAD_ATTACH:
        dllOnThreadEnter();
        break;
    case DLL_THREAD_DETACH:
        dllOnThreadExit();
        break;
    case DLL_PROCESS_DETACH:
        dllOnUnload();
        break;
    default:
        assert(false);
    }
    return TRUE;
}
#else
namespace
{
    class ThreadMonitor
    {
    public:
        ThreadMonitor() { dllOnThreadEnter(); }
        ~ThreadMonitor() { dllOnThreadExit(); }
    };

    thread_local ThreadMonitor threadMonitor;

}//namespace /*anonymous*/
#endif
