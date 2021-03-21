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

extern "C"
{

#   define DEFINE_TEST_FUNCTION(Abi, Type)  \
        API_EXPORT  \
        Type Abi add_##Type##Abi(Type a, Type b)    \
        {   \
			auto const c = a + b;   \
			std::cout << "In add_" #Type #Abi " with "   \
                << '(' << a << ") + (" << b << ") = (" << c << ')' << std::endl;    \
            return c;   \
        }

    DEFINE_TEST_FUNCTION(CALL_CDECL, char);
    DEFINE_TEST_FUNCTION(CALL_CDECL, int16_t);
    DEFINE_TEST_FUNCTION(CALL_CDECL, int32_t);
    DEFINE_TEST_FUNCTION(CALL_CDECL, int64_t);
    DEFINE_TEST_FUNCTION(CALL_CDECL, float);
    DEFINE_TEST_FUNCTION(CALL_CDECL, double);

    DEFINE_TEST_FUNCTION(CALL_STDCALL, char);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, int16_t);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, int32_t);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, int64_t);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, float);
    DEFINE_TEST_FUNCTION(CALL_STDCALL, double);

    DEFINE_TEST_FUNCTION(CALL_FASTCALL, char);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, int16_t);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, int32_t);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, int64_t);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, float);
    DEFINE_TEST_FUNCTION(CALL_FASTCALL, double);

}//extern "C"