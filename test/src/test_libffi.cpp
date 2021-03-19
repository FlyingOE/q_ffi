#include <gtest/gtest.h>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <ffi.h>

#ifdef _WIN32

#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>

namespace
{
    class DLLLoader
    {
        HMODULE dll_{ nullptr };

    public:
        DLLLoader(LPCSTR dll)
        {
            assert(nullptr == dll_);
            dll_ = LoadLibrary(dll);
            if (nullptr == dll_) {
                std::ostringstream buffer;
                buffer << "Failed to load DLL (error = "
                    << std::hex << GetLastError() << std::endl;
                throw std::runtime_error(buffer.str());
            }
        }

        template<typename ProcType>
        ProcType getProc(char const* funcName) const
        {
            checkState();
            return (ProcType)GetProcAddress(dll_, funcName);
        }

        ~DLLLoader()
        {
            if (nullptr != dll_) {
                FreeLibrary(dll_);
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

TEST(libffiTests, stdcall)
{
    auto const dll = TEXT("user32.dll");
    auto const func = "GetSystemMetrics";   // Win32 API

    DLLLoader dyn{ dll };
    auto const fp = dyn.getProc<void(*)()>(func);
    ASSERT_NE(fp, nullptr);

    ffi_cif cif{};
    ffi_type* args[1] = { &ffi_type_sint };
    auto const status = ffi_prep_cif(&cif, FFI_STDCALL, 1, &ffi_type_sint, args);
    ASSERT_EQ(status, FFI_OK);

    ASSERT_GE(sizeof(int), sizeof(ffi_arg)) << "Use <int> to receive result";
    int widthQuery = SM_CXSCREEN;
    int heightQuery = SM_CYSCREEN;
    int width{ 0 }, height{ 0 };
    void* params[1] = { &widthQuery };
    ffi_call(&cif, fp, &width, params);
    params[0] = &heightQuery;
    ffi_call(&cif, fp, &height, params);

    std::cout << "Current screen resolution = " << width << " x " << height << std::endl;
    EXPECT_GE(width, 640);
    EXPECT_GE(height, 480);
}

#endif
