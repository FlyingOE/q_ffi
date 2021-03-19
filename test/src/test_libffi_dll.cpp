#ifdef _WIN32

#include <cassert>
#include <iostream>
#include <thread>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace
{
    char const* DLL_NAME = "test_libffi_dll.dll";
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

extern "C"
{

#   define DEFINE_TEST_FUNCTION(Abi, Type)  \
        __declspec(dllexport)   \
        Type Abi add_##Type##Abi(Type a, Type b) {   \
			auto const c = a + b;   \
			std::cout << "In add_" #Type #Abi " with "   \
                << '(' << a << ") + (" << b << ") = (" << c << ')' << std::endl;    \
            return c;   \
        }

    DEFINE_TEST_FUNCTION(__cdecl, char);
    DEFINE_TEST_FUNCTION(__cdecl, int16_t);
    DEFINE_TEST_FUNCTION(__cdecl, int32_t);
    DEFINE_TEST_FUNCTION(__cdecl, int64_t);
    DEFINE_TEST_FUNCTION(__cdecl, float);
    DEFINE_TEST_FUNCTION(__cdecl, double);

    DEFINE_TEST_FUNCTION(__stdcall, char);
    DEFINE_TEST_FUNCTION(__stdcall, int16_t);
    DEFINE_TEST_FUNCTION(__stdcall, int32_t);
    DEFINE_TEST_FUNCTION(__stdcall, int64_t);
    DEFINE_TEST_FUNCTION(__stdcall, float);
    DEFINE_TEST_FUNCTION(__stdcall, double);

    DEFINE_TEST_FUNCTION(__fastcall, char);
    DEFINE_TEST_FUNCTION(__fastcall, int16_t);
    DEFINE_TEST_FUNCTION(__fastcall, int32_t);
    DEFINE_TEST_FUNCTION(__fastcall, int64_t);
    DEFINE_TEST_FUNCTION(__fastcall, float);
    DEFINE_TEST_FUNCTION(__fastcall, double);

}//extern "C"

#endif