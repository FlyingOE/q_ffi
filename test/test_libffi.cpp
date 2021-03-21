#include <gtest/gtest.h>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <random>
#include <dlfcn.h>
#include <ffi.h>

namespace
{
    class DLLLoader
    {
        void *dll_{ nullptr };

    public:
        DLLLoader(char const* dll)
        {
            assert(nullptr == dll_);
            dll_ = dlopen(dll, RTLD_LAZY);

            if (nullptr == dll_) {
                std::ostringstream buffer;
                buffer << "Failed to load DLL `" << dll << "'"
                    << " (error = " << dlerror() << ')';
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

namespace
{
#ifdef _WIN32
    char const* TEST_DLL = "test_q_ffi_dll.dll";
#elif defined(__linux__) && !defined(__ANDROID__)
    char const* TEST_DLL = "libtest_q_ffi_dll.so";
#else
#   error FIXME: add unit test for this platform...
#endif
}

#ifdef _WIN32

TEST(libffiBaseTests, Win32API)
{
    auto const dll = MAKE_FILENAME("user32.dll");
    auto const func = "GetSystemMetrics";   // Win32 API

    DLLLoader dyn{ dll };
    auto const fp = dyn.getProc<void(*)()>(func);
    ASSERT_NE(fp, nullptr);

    ffi_cif cif{};
    ffi_type* args[1] = { &ffi_type_sint };
#ifdef _WIN64
    ffi_abi const abi = FFI_WIN64;
#else
    ffi_abi const abi = FFI_STDCALL;
#endif
    auto const status = ffi_prep_cif(&cif, abi, 1, &ffi_type_sint, args);
    ASSERT_EQ(status, FFI_OK);

    int widthQuery = SM_CXSCREEN;
    int heightQuery = SM_CYSCREEN;
    ffi_sarg width0, height0;
    void* params[1] = { &widthQuery };
    ffi_call(&cif, fp, &width0, params);
    params[0] = &heightQuery;
    ffi_call(&cif, fp, &height0, params);

    int const width = *reinterpret_cast<int*>(&width0);
    int const height = *reinterpret_cast<int*>(&height0);
    std::cout << "Current screen resolution = " << width << " x " << height << std::endl;
    EXPECT_GE(width, 640);
    EXPECT_GE(height, 480);
}

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
    struct function_traits<R(__cdecl *)(Args...)>
        : public function_traits<R(Args...)>
    {
        static constexpr ffi_abi abi_type =
#       ifdef _WIN64
            FFI_WIN64;
#       else
            FFI_MS_CDECL;
#       endif

        template<typename Type>
        static std::string mangle_name(char const* funcName, char const* typeName)
        {
            std::ostringstream buffer;
            buffer << funcName << '_' << typeName << "__cdecl";
            return buffer.str();
        }
    };

#ifndef _WIN64

    template<class R, class... Args>
    struct function_traits<R(__stdcall *)(Args...)>
        : public function_traits<R(Args...)>
    {
        static constexpr ffi_abi abi_type = FFI_STDCALL;

        template<typename Type>
        static std::string mangle_name(char const* funcName, char const* typeName)
        {
            std::ostringstream buffer;
            buffer << '_'
                << funcName << '_' << typeName << "__stdcall"
                << '@' << std::max<std::size_t>(8, sizeof(Type) * 2);
            return buffer.str();
        }
    };

    template<class R, class... Args>
    struct function_traits<R(__fastcall*)(Args...)>
        : public function_traits<R(Args...)>
    {
        static constexpr ffi_abi abi_type = FFI_FASTCALL;

        template<typename Type>
        static std::string mangle_name(char const* funcName, char const* typeName)
        {
            std::ostringstream buffer;
            buffer << '@'
                << funcName << '_' << typeName << "__fastcall"
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

#ifdef _WIN64
#   define ADD_FFI_TEST_CASES(Type)   \
        Type (*)(Type, Type)
#else
#   define ADD_FFI_TEST_CASES(Type)   \
        Type (__cdecl *)(Type, Type),   \
        Type (__stdcall *)(Type, Type), \
        Type (__fastcall *)(Type, Type)
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
    using function_type = Signature;
    using function_types = typename function_traits<function_type>;
    static_assert(function_types::arity == 2, "Expecting a diadic test function");

    using data_type = typename function_types::return_type;
    using return_type = std::conditional_t<
        sizeof(data_type) < sizeof(ffi_sarg),
        ffi_sarg,
        data_type>;

private:
    class uniform_char_distribution
    {
    private:
        std::uniform_int_distribution<short> dist{};

    public:
        template<typename Engine>
        char operator()(Engine& engine) const
        { return static_cast<char>(dist(engine)); }
    };

    using distribution_type = std::conditional_t<
        std::is_floating_point_v<data_type>,
        std::uniform_real_distribution<data_type>,
        std::conditional_t<
            std::is_same_v<data_type, char>,
            uniform_char_distribution,
            std::uniform_int_distribution<data_type>>>;

protected:
    void test_invoke(DLLLoader::filename_type const& dll, char const* func)
    {
        DLLLoader dyn{ dll };

        auto const mangled = function_types::mangle_name<data_type>(
            func, ffi_type_info<data_type>::type_name);
        SCOPED_TRACE("FFI invocation of " + mangled);

        auto const fp = dyn.getProc<void(*)()>(mangled.c_str());
        ASSERT_NE(fp, nullptr) << "Look up for `" << mangled << "' in " << dll;

        // Setup invocation argument and result types
        ffi_cif cif{};
        ffi_type* res_type = &ffi_type_info<data_type>::type_info;
        ffi_type* arg_types[] = { res_type, res_type };
        auto const status = ffi_prep_cif(&cif,
            function_types::abi_type, function_types::arity, res_type, arg_types);
        ASSERT_EQ(status, FFI_OK);

        // Generate random parameters for the invocation
        data_type parameters[function_types::arity];
        void* params[function_types::arity];
        std::random_device rd;  // seed generator
        distribution_type dist;
        for (auto i = 0; i < function_types::arity; ++i) {
            parameters[i] = dist(rd);
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

TYPED_TEST(LibffiTests, add)
{
    this->test_invoke("test_q_ffi_dll.dll", "add");
}

#pragma endregion

#else
//#error FIXME: Implement tests for this platform...
#endif
