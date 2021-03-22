#define CALL_CDECL    __attribute__((cdecl))
#define CALL_STDCALL  __attribute__((stdcall))
#define CALL_FASTCALL __attribute__((fastcall))

namespace
{
    char const* TEST_DLL = "libtest_q_ffi_dll.so";
}

TEST(libffiBaseTests, libdl)
{
    auto const dll = "libdl.so";
    auto const func = "dladdr";

    DLLLoader dyn{ dll };
    auto const fp = dyn.getProc<void(*)()>(func);
    ASSERT_NE(fp, nullptr);

    ffi_cif cif{};
    ffi_type* args[2] = { &ffi_type_pointer, &ffi_type_pointer };
#if defined(_WIN64) || defined(__x86_64__)
    ffi_abi const abi = FFI_DEFAULT_ABI;
#else
    ffi_abi const abi = FFI_STDCALL;
#endif
    auto const status = ffi_prep_cif(&cif, abi, 2, &ffi_type_sint, args);
    ASSERT_EQ(status, FFI_OK);

    struct Dl_info
    {
        char const* dli_fname;  // pathname of so that contains addr
        void* dli_fbase;        // base addr at which so is loaded
        char const* dli_sname;  // name of symbol that overlaps addr
        void* dli_saddr;        // base addr of symbol
    };
    Dl_info info_v{};
    void* addr = reinterpret_cast<void*>(fp);
    Dl_info* info = &info_v;
    void* params[2] = { &addr, &info };

    static_assert(sizeof(int) <= sizeof(ffi_sarg), "int <= ffi_sarg");
    ffi_sarg result;
    ffi_call(&cif, fp, &result, params);
    int const res = *reinterpret_cast<int*>(&result);
    EXPECT_NE(res, 0);
    EXPECT_NE(info_v.dli_fname, nullptr);
    EXPECT_NE(info_v.dli_fbase, nullptr);
    EXPECT_STREQ(info_v.dli_sname, func);
    EXPECT_EQ(info_v.dli_saddr, reinterpret_cast<void*>(fp));
}
