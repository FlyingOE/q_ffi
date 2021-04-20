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

#include <algorithm>

using namespace std;

namespace
{
    struct Freer
    {
        void operator()(void* p)
        {
            std::free(p);
        }
    };
}

#pragma region Tests for different calling conventions (ABIs)
extern "C"
{

#   define DEFINE_TEST_ADD(Abi, AbiName, Type)  \
        API_EXPORT  \
        Type Abi add_##Type##_##AbiName(Type a, Type b)    \
        {   \
            auto const c = a + b;   \
            cout << dec << "In add_" #Type "_" #AbiName " with "    \
                << '(' << a << ") + (" << b << ") = (" << c << ')' << endl; \
            return c;   \
        }   \
        \
        API_EXPORT  \
        Type* Abi adds_##Type##_##AbiName(Type const* a, Type const* b, size_t n)   \
        {   \
            unique_ptr<void, Freer> cc{ std::malloc(sizeof(Type) * n) };    \
            Type* c = static_cast<Type*>(cc.get());     \
            cout << dec << "In adds_" #Type "_" #AbiName " with (" << n << ") " << hex  \
                << '[0x' << a << "] + [0x" << b << "] = [0x" << c << ']' << endl;   \
            for (auto i = 0; i < n; ++i)    \
                c[i] = a[i] + b[i];     \
            return static_cast<Type*>(cc.release());    \
        }

    DEFINE_TEST_ADD(CALL_CDECL, cdecl, char);
    DEFINE_TEST_ADD(CALL_CDECL, cdecl, int16_t);
    DEFINE_TEST_ADD(CALL_CDECL, cdecl, int32_t);
    DEFINE_TEST_ADD(CALL_CDECL, cdecl, int64_t);
    DEFINE_TEST_ADD(CALL_CDECL, cdecl, float);
    DEFINE_TEST_ADD(CALL_CDECL, cdecl, double);

    DEFINE_TEST_ADD(CALL_STDCALL, stdcall, char);
    DEFINE_TEST_ADD(CALL_STDCALL, stdcall, int16_t);
    DEFINE_TEST_ADD(CALL_STDCALL, stdcall, int32_t);
    DEFINE_TEST_ADD(CALL_STDCALL, stdcall, int64_t);
    DEFINE_TEST_ADD(CALL_STDCALL, stdcall, float);
    DEFINE_TEST_ADD(CALL_STDCALL, stdcall, double);

    DEFINE_TEST_ADD(CALL_FASTCALL, fastcall, char);
    DEFINE_TEST_ADD(CALL_FASTCALL, fastcall, int16_t);
    DEFINE_TEST_ADD(CALL_FASTCALL, fastcall, int32_t);
    DEFINE_TEST_ADD(CALL_FASTCALL, fastcall, int64_t);
    DEFINE_TEST_ADD(CALL_FASTCALL, fastcall, float);
    DEFINE_TEST_ADD(CALL_FASTCALL, fastcall, double);

}//extern "C"
#pragma endregion

#pragma region Tests for various argument/type combinations

namespace
{
    template<typename T0, typename T1>
    T0 any_hash(T0 p0, T1 p1)
    {
        auto px = reinterpret_cast<T0 const*>(&p1);
        auto pn = sizeof(p1) / sizeof(p0);
        for_each_n(px, pn, [&p0](auto const x) { p0 ^= x; });
        return p0;
    }
}

extern "C" API_EXPORT
uint32_t f0()
{
    return numeric_limits<uint32_t>::max();
}

extern "C" API_EXPORT
uint32_t CALL_STDCALL f0_stdcall()
{ return f0(); }

extern "C" API_EXPORT
uint32_t CALL_FASTCALL f0_fastcall()
{ return f0(); }

extern "C" API_EXPORT
char f1(char a)
{
    return static_cast<char>(a - 10);
}

extern "C" API_EXPORT
char CALL_STDCALL f1_stdcall(char a)
{ return f1(a); }

extern "C" API_EXPORT
char CALL_FASTCALL f1_fastcall(char a)
{ return f1(a); }

extern "C" API_EXPORT
unsigned char f2(char a, uint64_t b)
{
    return any_hash(a, b);
}

extern "C" API_EXPORT
unsigned char CALL_STDCALL f2_stdcall(char a, uint64_t b)
{ return f2(a, b); }

extern "C" API_EXPORT
unsigned char CALL_FASTCALL f2_fastcall(char a, uint64_t b)
{ return f2(a, b); }

extern "C" API_EXPORT
int16_t f3(char a, int16_t b, uint64_t c)
{
    int16_t aa = a;
    return any_hash(any_hash(aa, b), c);
}

extern "C" API_EXPORT
int16_t CALL_STDCALL f3_stdcall(char a, int16_t b, uint64_t c)
{ return f3(a, b, c); }

extern "C" API_EXPORT
int16_t CALL_FASTCALL f3_fastcall(char a, int16_t b, uint64_t c)
{ return f3(a, b, c); }

extern "C" API_EXPORT
int16_t f4(char a, int32_t b, int16_t c, int64_t d)
{
    int16_t aa = a;
    return any_hash(any_hash(any_hash(aa, b), c), d);
}

extern "C" API_EXPORT
int16_t CALL_STDCALL f4_stdcall(char a, int32_t b, int16_t c, int64_t d)
{ return f4(a, b, c, d); }

extern "C" API_EXPORT
int16_t CALL_FASTCALL f4_fastcall(char a, int32_t b, int16_t c, int64_t d)
{ return f4(a, b, c, d); }

extern "C" API_EXPORT
int16_t f5(char a, double b, int32_t c, int16_t d, int64_t e)
{
    int16_t aa = a;
    return b * any_hash(any_hash(any_hash(aa, c), d), e);
}

extern "C" API_EXPORT
int16_t CALL_STDCALL f5_stdcall(char a, double b, int32_t c, int16_t d, int64_t e)
{ return f5(a, b, c, e, d); }

extern "C" API_EXPORT
int16_t CALL_FASTCALL f5_fastcall(char a, double b, int32_t c, int16_t d, int64_t e)
{ return f5(a, b, c, e, d); }

extern "C" API_EXPORT
int16_t f6(float a, char b, int32_t c, int16_t d, int64_t e, double f)
{
    int16_t bb = b;
    return a * any_hash(any_hash(any_hash(bb, c), d), e) / f;
}

extern "C" API_EXPORT
int16_t CALL_STDCALL f6_stdcall(float a, char b, int32_t c, int16_t d, int64_t e, double f)
{ return f6(a, b, c, e, d, f); }

extern "C" API_EXPORT
int16_t CALL_FASTCALL f6_fastcall(float a, char b, int32_t c, int16_t d, int64_t e, double f)
{ return f6(a, b, c, d, e, f); }

extern "C" API_EXPORT
int32_t f7(float a, char b, int32_t c, int16_t d, int64_t e, double f, unsigned char g)
{
    int32_t bb = b;
    int32_t gg = g;
    return a * any_hash(any_hash(any_hash(any_hash(bb, c), d), e), gg) / f;
}

extern "C" API_EXPORT
int32_t CALL_STDCALL f7_stdcall(float a, char b, int32_t c, int16_t d, int64_t e, double f, unsigned char g)
{ return f7(a, b, c, d, e, f, g); }

extern "C" API_EXPORT
int32_t CALL_FASTCALL f7_fastcall(float a, char b, int32_t c, int16_t d, int64_t e, double f, unsigned char g)
{ return f7(a, b, c, d, e, f, g); }

#pragma endregion

#pragma region Tests for various variable types

extern "C" API_EXPORT bool v_bool;
bool v_bool = true;

extern "C" API_EXPORT char v_char;
char v_char = 'C';

extern "C" API_EXPORT int8_t v_byte;
int8_t v_byte = 0xCC;

extern "C" API_EXPORT int16_t v_short;
int16_t v_short = 0xDEAD;

extern "C" API_EXPORT int32_t v_int;
int32_t v_int = 0xDEAD'BEEF;

extern "C" API_EXPORT int64_t v_long;
int64_t v_long = 0xDEAD'BEEF'8BAD'F00Dll;

extern "C" API_EXPORT float v_real;
float v_real = 3.14159265f;

extern "C" API_EXPORT double v_float;
double v_float = 3.141592653589793;

#pragma endregion

#pragma region Tests for various pointer types

namespace
{
    template<typename T, typename Op>
    T* process_any(T* data, size_t count, Op const& op)
    {
        assert(nullptr != data);
        transform(data, data + count, data, op);
        return data;
    }
}

extern "C"
{
#   define DEFINE_TEST_NEGATE(Type) \
        API_EXPORT  \
        Type* negate_##Type(Type* data, size_t count)   \
        {   \
            return process_any(data, count, [](auto x) { return -x; }); \
        }

    DEFINE_TEST_NEGATE(int8_t);
    DEFINE_TEST_NEGATE(int16_t);
    DEFINE_TEST_NEGATE(int32_t);
    DEFINE_TEST_NEGATE(int64_t);
    DEFINE_TEST_NEGATE(float);
    DEFINE_TEST_NEGATE(double);

    API_EXPORT
    bool* negate_bool(bool* data, size_t count)
    {
        return process_any(data, count, [](auto x) { return !x; });
    }

    API_EXPORT
    char* negate_char(char* data, size_t count)
    {
        return process_any(data, count,
            [](auto x) { return static_cast<char>(toupper(x)); });
    }

    API_EXPORT
    size_t symbol_len(char const* sym)
    {
        assert(nullptr != sym);
        return strlen(sym);
    }

    API_EXPORT
    char* trans_symbol(char* output, char const* input, size_t count)
    {
        assert(nullptr != output);
        assert(nullptr != input);
        transform(input, input + count, output,
            [](auto x) { return static_cast<char>(toupper(x)); });
        return output;
    }

}//extern "C"
#pragma endregion
