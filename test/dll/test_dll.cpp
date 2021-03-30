#if defined(_WIN32)
#   define PLATFORM_WIN
#   ifdef _WIN64
#       define PLATFORM_WIN_x86
#   else
#       define PLATFORM_WIN_x64
#   endif

#   include "test_dll_win.hpp"

#elif defined(__linux__) && !defined(__ANDROID__)
#   define PLATFORM_LINUX
#   if defined(__i386__)
#       define PLATFORM_LINUX_i686
#   elif defined(__x86_64__)
#       define PLATFORM_LINUX_x86_64
#   else
#       error FIXME: add unit test for this platform...
#   endif

#   include "test_dll_linux.hpp"

#else
#   error FIXME: add unit test for this platform...
#endif

using namespace std;

extern "C"
{

#   define DEFINE_TEST_FUNCTION(Abi, AbiName, Type)  \
        API_EXPORT  \
        Type Abi add_##Type##_##AbiName(Type a, Type b)    \
        {   \
			auto const c = a + b;   \
			cout << dec << "In add_" #Type "_" #AbiName " with "    \
                << '(' << a << ") + (" << b << ") = (" << c << ')' << endl; \
            return c;   \
        }

    DEFINE_TEST_FUNCTION(CALL_CDECL, cdecl, char);
    DEFINE_TEST_FUNCTION(CALL_CDECL, cdecl, int16_t);
    DEFINE_TEST_FUNCTION(CALL_CDECL, cdecl, int32_t);
    DEFINE_TEST_FUNCTION(CALL_CDECL, cdecl, int64_t);
    DEFINE_TEST_FUNCTION(CALL_CDECL, cdecl, float);
    DEFINE_TEST_FUNCTION(CALL_CDECL, cdecl, double);

    DEFINE_TEST_FUNCTION(CALL_STDCALL, stdcall, char);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, stdcall, int16_t);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, stdcall, int32_t);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, stdcall, int64_t);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, stdcall, float);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, stdcall, double);

    DEFINE_TEST_FUNCTION(CALL_FASTCALL, fastcall, char);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, fastcall, int16_t);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, fastcall, int32_t);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, fastcall, int64_t);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, fastcall, float);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, fastcall, double);

}//extern "C"
