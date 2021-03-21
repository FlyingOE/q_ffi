#include <cassert>
#include <iostream>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define CALL_CDECL    __cdecl
#define CALL_STDCALL  __stdcall
#define CALL_FASTCALL __fastcall

#define API_EXPORT __declspec(dllexport)

namespace
{
    char const* DLL_NAME = "test_q_ffi_dll.dll";
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    auto const pid = ::GetCurrentProcessId();
    auto const tid = std::this_thread::get_id();
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        std::cout << ">>> Loaded " << DLL_NAME
            << " in process #" << pid << " (thread #" << tid << ")." << std::endl;
        break;
    case DLL_THREAD_ATTACH:
        std::cout << ">>> Attached " << DLL_NAME
            << " to (process #" << pid << ") thread #" << tid << "." << std::endl;
        break;
    case DLL_THREAD_DETACH:
        std::cout << ">>> Detaching " << DLL_NAME
            << " from (process #" << pid << ") thread #" << tid << "..." << std::endl;
        break;
    case DLL_PROCESS_DETACH:
        std::cout << ">>> Unloading " << DLL_NAME
            << " from process #" << pid << " (thread #" << tid << ")..." << std::endl;
        break;
    default:
        assert(!"Unexpected ul_reason_for_call in DllMain(...)");
    }
    return TRUE;
}