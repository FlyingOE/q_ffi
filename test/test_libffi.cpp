#include <gtest/gtest.h>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <random>
#include <dlfcn.h>
#include <ffi.h>

#if defined(_WIN64) || defined(__x86_64__)
#   define PLATFORM_64BIT
#else
#   define PLATFORM_32BIT
#endif

namespace
{
    class DLLLoader
    {
        void *dll_{ nullptr };

    public:
        DLLLoader(char const* filename)
        {
            assert(nullptr == dll_);
            std::cout << "Loading DLL `" << filename << "'..." << std::endl;
            dll_ = dlopen(filename, RTLD_LAZY);

            if (nullptr == dll_) {
                std::ostringstream buffer;
                buffer << "Failed to load DLL `" << filename << "' (error = " << dlerror() << ')';
                throw std::runtime_error(buffer.str());
            }
        }

        template<typename ProcType>
        ProcType getProc(char const* funcName) const
        {
            checkState();
            return (ProcType)dlsym(dll_, funcName);
        }

        ~DLLLoader()
        {
            if (nullptr != dll_) {
                dlclose(dll_);
                dll_ = nullptr;
            }
        }

    private:
        void checkState() const noexcept(false)
        {
            if (nullptr == dll_)
                throw std::runtime_error("DLL not loaded yet");
        }
    };

}//namespace /*anonymous*/

/////////////////////////////////////////////////////////////////////////////
#if defined(_WIN32)
#   include "test_libffi_win.hpp"
#elif defined(__linux__) && !defined(__ANDROID__)
#   include "test_libffi_linux.hpp"
#else
#   error FIXME: add base case tests for this platform...
#endif
/////////////////////////////////////////////////////////////////////////////

#pragma region Test all supported call conventions

namespace
{
#   pragma region Map C++ types to ffi_type structs

    template<typename T>
    struct ffi_type_info;

#   define DEFINE_FFI_TYPE_INFO(Type, Info) \
        template<>  \
        struct ffi_type_info<Type>  \
        {   \
            static constexpr ffi_type& type_info = (Info);  \
            static constexpr char const* type_name = #Type; \
        }

    DEFINE_FFI_TYPE_INFO(char, ffi_type_schar);
    DEFINE_FFI_TYPE_INFO(int16_t, ffi_type_sint16);
    DEFINE_FFI_TYPE_INFO(int32_t, ffi_type_sint32);
    DEFINE_FFI_TYPE_INFO(int64_t, ffi_type_sint64);
    DEFINE_FFI_TYPE_INFO(float, ffi_type_float);
    DEFINE_FFI_TYPE_INFO(double, ffi_type_double);

#   pragma endregion

#   pragma region Function pointer traits

    template<class F>
    struct function_traits;

    template<class R, class... Args>
#   ifdef PLATFORM_64BIT
    struct function_traits<R(*)(Args...)>
#   else
    struct function_traits<R(CALL_CDECL *)(Args...)>
#   endif
        : public function_traits<R(Args...)>
    {
        static constexpr ffi_abi abi_type =
#       ifdef PLATFORM_64BIT
            FFI_DEFAULT_ABI;
#       else
            FFI_MS_CDECL;
#       endif

        template<typename Type>
        static std::string mangle_name(char const* funcName, char const* typeName)
        {
            std::ostringstream buffer;
            buffer << funcName << '_' << typeName << "_" "cdecl";
            return buffer.str();
        }
    };

#ifndef PLATFORM_64BIT

    template<class R, class... Args>
    struct function_traits<R(CALL_STDCALL *)(Args...)>
        : public function_traits<R(Args...)>
    {
        static constexpr ffi_abi abi_type = FFI_STDCALL;

        template<typename Type>
        static std::string mangle_name(char const* funcName, char const* typeName)
        {
            std::ostringstream buffer;
            buffer << '_'
                << funcName << '_' << typeName << "_" "stdcall"
                << '@' << std::max<std::size_t>(8, sizeof(Type) * 2);
            return buffer.str();
        }
    };

    template<class R, class... Args>
    struct function_traits<R(CALL_FASTCALL *)(Args...)>
        : public function_traits<R(Args...)>
    {
        static constexpr ffi_abi abi_type = FFI_FASTCALL;

        template<typename Type>
        static std::string mangle_name(char const* funcName, char const* typeName)
        {
            std::ostringstream buffer;
            buffer << '@'
                << funcName << '_' << typeName << "_" "fastcall"
                << '@' << std::max<std::size_t>(8, sizeof(Type) * 2);
            return buffer.str();
        }
    };

#endif

    template<class R, class... Args>
    struct function_traits<R(Args...)>
    {
        using return_type = R;

        static constexpr std::size_t arity = sizeof...(Args);

        template <std::size_t N>
        struct argument
        {
            static_assert(N < arity, "error: invalid parameter index.");
            using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
        };
    };

#   pragma endregion
}//namespace /*anonymous*/

template<typename Signature>
struct LibffiTestCase
{
    using function_type = Signature;
};

#ifdef PLATFORM_64BIT
#   define ADD_FFI_TEST_CASES(Type)   \
        Type (*)(Type, Type)
#else
#   define ADD_FFI_TEST_CASES(Type)   \
        Type (CALL_CDECL    *)(Type, Type), \
        Type (CALL_STDCALL  *)(Type, Type), \
        Type (CALL_FASTCALL *)(Type, Type)
#endif

using LibffiTestTypes = ::testing::Types <
    ADD_FFI_TEST_CASES(char),
    ADD_FFI_TEST_CASES(int16_t),
    ADD_FFI_TEST_CASES(int32_t),
    ADD_FFI_TEST_CASES(int64_t),
    ADD_FFI_TEST_CASES(float),
    ADD_FFI_TEST_CASES(double)
>;

template<typename Signature>
class LibffiTests : public ::testing::Test
{
protected:
    using fun_type = Signature;
    using fun_traits = function_traits<fun_type>;
    static_assert(fun_traits::arity == 2, "Expecting a diadic test function");

    using data_type = typename fun_traits::return_type;
    using return_type = std::conditional_t<
        sizeof(data_type) < sizeof(ffi_sarg),
        ffi_sarg,
        data_type>;

private:
    using distribution_type = std::conditional_t<
        std::is_floating_point_v<data_type>,
        std::uniform_real_distribution<data_type>,
        std::conditional_t<
            std::is_same_v<data_type, char>,
            std::uniform_int_distribution<int16_t>,
            std::uniform_int_distribution<data_type>>>;

protected:
    void test_invoke(char const* dllname, char const* func)
    {
        ASSERT_NE(dllname, nullptr);
        ASSERT_NE(func, nullptr);

        DLLLoader dll{ dllname };

        auto const mangled = fun_traits::template mangle_name<data_type>(
            func, ffi_type_info<data_type>::type_name);
        SCOPED_TRACE("FFI invocation of " + mangled);

        auto const fp = dll.getProc<void(*)()>(mangled.c_str());
        ASSERT_NE(fp, nullptr) << "Look up for `" << mangled << "' in " << dllname;

        // Setup invocation argument and result types
        ffi_cif cif{};
        ffi_type* res_type = &ffi_type_info<data_type>::type_info;
        ffi_type* arg_types[] = { res_type, res_type };
        auto const status = ffi_prep_cif(&cif,
            fun_traits::abi_type, fun_traits::arity, res_type, arg_types);
        ASSERT_EQ(status, FFI_OK);

        // Generate random parameters for the invocation
        data_type parameters[fun_traits::arity];
        void* params[fun_traits::arity];
        std::random_device rd;  // seed generator
        distribution_type dist;
        for (auto i = 0; i < fun_traits::arity; ++i) {
            parameters[i] = static_cast<data_type>(dist(rd));
            params[i] = &parameters[i];
        }

        // Invoke target function and verify result
        return_type res{};
        ffi_call(&cif, fp, &res, params);
        data_type const result = *reinterpret_cast<data_type*>(&res);
        EXPECT_EQ(result, static_cast<data_type>(parameters[0] + parameters[1]));
    }
};

TYPED_TEST_SUITE(LibffiTests, LibffiTestTypes);

TYPED_TEST(LibffiTests, dll_add)
{
    this->test_invoke(TEST_DLL.c_str(), "add");
}

#pragma endregion
