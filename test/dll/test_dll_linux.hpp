#include <thread>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#define CALL_CDECL    __attribute__((cdecl))
#define CALL_STDCALL  __attribute__((stdcall))
#define CALL_FASTCALL __attribute__((fastcall))

#define API_EXPORT

namespace
{
    char const* DLL_NAME = "libtest_q_ffi_dll.so";

    pid_t pid()
    {
        return getpid();
    }

    std::thread::id tid()
    {
        return std::this_thread::get_id();
    }

    __attribute__((constructor))
    void dllOnLoad()
    {
        std::cout << ">>> Loaded " << DLL_NAME
            << " in process #" << pid() << " (thread #" << tid() << ")." << std::endl;
    }

    __attribute__((destructor))
    void dllOnUnload()
    {
        std::cout << ">>> Unloading " << DLL_NAME
            << " from process #" << pid() << " (thread #" << tid() << ")..." << std::endl;
    }

}//namespace /*anonymous*/
