#include <gtest/gtest.h>
#include <random>
#include "ffi_api.h"
#include "kpointer.hpp"
#include "ktype_traits.hpp"
#include "type_convert.hpp"

using namespace q;
using namespace std;

#if defined(_WIN32)
#   define PLATFORM_WINDOWS
#   ifdef _WIN64
#       define PLATFORM_X86_64
#   else
#       define PLATFORM_X86
#   endif
#elif defined(__linux__) && !defined(__ANDROID__)
#   define PLATFORM_LINUX
#   ifdef __x86_64__
#       define PLATFORM_X86_64
#   else
#       define PLATFORM_X86
#   endif
#   include <unistd.h>
#else
#   error FIXME: add unit test for this platform...
#endif

namespace q_ffi
{
#ifdef PLATFORM_LINUX
    static string getTestDllPath()
    {
        char path[PATH_MAX + 1]{};
        auto n = readlink("/proc/self/exe", path, PATH_MAX);
        assert(n <= PATH_MAX);

        char* p = strrchr(path, '/');
        assert(p != nullptr && p <= path + PATH_MAX);

        auto const DLL_NAME = "libtest_q_ffi_dll.so";
        strncpy(p + 1, DLL_NAME, PATH_MAX - strlen(path));
        return path;
    }
#endif

    class FFCallTests : public ::testing::Test
    {
    private:
        K_ptr DLL_NAME;

    protected:
        random_device rd_;  // seed generator

    protected:
        void SetUp() override {
            using symbol_type = TypeTraits<kSymbol>;
            DLL_NAME.reset(
#       if defined(PLATFORM_WINDOWS)
                symbol_type::atom("test_q_ffi_dll.dll")
#       elif defined(PLATFORM_LINUX)
                symbol_type::atom(getTestDllPath().c_str())
#       else
#           error FIXME: add unit test for this platform...
#       endif
            );
        }

        void invoke(::K func, ::K retType, ::K argTypes, ::K abi,
            K_ptr& result, K_ptr const& params)
        {
            K_ptr functor{ load(DLL_NAME.get(), func, retType, argTypes, abi) };
            ASSERT_TRUE(functor);
            result.reset(::dot(functor.get(), params.get()));
        }
    };

    TEST_F(FFCallTests, add_cdecl)
    {
#   ifdef PLATFORM_X86_64
        K_ptr abi{ TypeTraits<kSymbol>::atom("") };
        K_ptr func{ TypeTraits<kSymbol>::atom("add_double_cdecl") };
#   else
        K_ptr abi{ TypeTraits<kSymbol>::atom("cdecl") };
        K_ptr func{ TypeTraits<kSymbol>::atom("add_double_cdecl") };
#   endif
        K_ptr retType{ TypeTraits<kChar>::atom('f') };
        K_ptr argTypes{ TypeTraits<kChar>::list("ff") };
        {
            SCOPED_TRACE("__cdecl: homogeneous arguments, matching types");
            using value_type = TypeTraits<kFloat>::value_type;
            uniform_real_distribution<value_type> dist;
            vector<value_type> pars{ dist(this->rd_), dist(this->rd_) };
            K_ptr params{ TypeTraits<kFloat>::list(pars.cbegin(), pars.cend()) };
            K_ptr result{};

/*
            this->invoke(func.get(), retType.get(), argTypes.get(), abi.get(),
                result, params);
            EXPECT_NE(result.get(), nullptr);
            EXPECT_FLOAT_EQ(q2Real(result.get()), pars[0] + pars[1]);
*/
        }
    }

}//namespace q_ffi