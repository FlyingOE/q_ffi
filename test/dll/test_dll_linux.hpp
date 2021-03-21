#include <thread>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include "gtest/gtest.h"

#define CALL_CDECL    __attribute__((cdecl))
#define CALL_STDCALL  __attribute__((stdcall))
#define CALL_FASTCALL __attribute__((fastcall))

namespace
{
    char const* DLL_NAME = "libtest_q_ffi_dll.so";
}

class DllEcho
{
private:
    pid_t pid_;

    std::thread::id tid()
    {
        return std::this_thread::get_id();
    }

public:
    DllEcho() : pid_{ getpid() }
    {
        std::cout << ">>> Loaded " << DLL_NAME
            << " in process #" << pid_ << " (thread #" << tid() << ")." << std::endl;
    }

    ~DllEcho()
    {
        std::cout << ">>> Unloading " << DLL_NAME
            << " from process #" << pid_ << " (thread #" << tid() << ")..." << std::endl;
    }
};

namespace
{

    ::testing::Environment* const init_dll =
        ::testing::AddGlobalTestEnvironment(new DllEcho);

}
