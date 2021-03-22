#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#define CALL_CDECL    __cdecl
#define CALL_STDCALL  __stdcall
#define CALL_FASTCALL __fastcall

namespace
{
    std::string TEST_DLL{ "test_q_ffi_dll.dll" };
}

TEST(LibffiBaseTests, Win32API)
{
    auto const dll = "user32.dll";
    auto const func = "GetSystemMetrics";   // Win32 API

    DLLLoader dyn{ dll };
    auto const fp = dyn.getProc<void(*)()>(func);
    ASSERT_NE(fp, nullptr);

    ffi_cif cif{};
    ffi_type* args[1] = { &ffi_type_sint };
#ifdef _WIN64
    ffi_abi const abi = FFI_DEFAULT_ABI;
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
