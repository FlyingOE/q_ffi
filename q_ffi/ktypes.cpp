#include "ktype_traits.hpp"
#include <regex>

namespace q
{
    std::unordered_map<TypeId, char> const TypeCode{
        { kMixed, ' ' },
        { kBoolean, 'b' },
        { kGUID, 'g' },
        { kByte, 'x' },
        { kShort, 'h' },
        { kInt, 'i' },
        { kLong, 'j' },
        { kReal, 'e' },
        { kFloat, 'f' },
        { kChar, 'c' },
        { kSymbol, 's' },
        { kTimestamp, 'p' },
        { kMonth, 'm' },
        { kDate, 'd' },
        { kDatetime, 'z' },
        { kTimespan, 'n' },
        { kMinute, 'u' },
        { kSecond, 'v' },
        { kTime, 't' },
        { kEnumMin, 's' },
        { kEnumMax, 's' },
        { kTable, ' ' },
        { kDict, ' ' },
        { kNil, '\0' },
        { kError, '\0' }
    };

}//namespace q

::K q::error(char const* msg, bool sys) noexcept
{
    return TypeTraits<kError>::atom(msg, sys);
}

namespace q
{
    inline namespace literals
    {
        ::G operator"" _qb(unsigned long long b) noexcept
        { return static_cast<TypeTraits<kBoolean>::value_type>(b); }

        ::G operator"" _qx(unsigned long long i8) noexcept
        { return static_cast<TypeTraits<kByte>::value_type>(i8); }
        ::H operator"" _qh(unsigned long long i16) noexcept
        { return static_cast<TypeTraits<kShort>::value_type>(i16); }
        ::I operator"" _qi(unsigned long long i32) noexcept
        { return static_cast<TypeTraits<kInt>::value_type>(i32); }
        ::J operator"" _qj(unsigned long long i64) noexcept
        { return static_cast<TypeTraits<kLong>::value_type>(i64); }

        ::E operator"" _qe(long double f32) noexcept
        { return static_cast<TypeTraits<kReal>::value_type>(f32); }
        ::F operator"" _qf(long double f64) noexcept
        { return static_cast<TypeTraits<kFloat>::value_type>(f64); }

        ::J operator"" _qp(char const* ymdhmsf, std::size_t /*len*/) noexcept
        { return TypeTraits<kTimestamp>::parse(ymdhmsf); }
        ::J operator"" _qp(char const* yyyymmddhhmmssf9) noexcept
        { return TypeTraits<kTimestamp>::parse(yyyymmddhhmmssf9, true); }

        ::I operator"" _qm(char const* ym, std::size_t /*len*/) noexcept
        { return TypeTraits<kMonth>::parse(ym); }
        ::I operator"" _qm(unsigned long long int yyyymm) noexcept
        { return TypeTraits<kMonth>::parse(static_cast<int>(yyyymm)); }

        ::I operator"" _qd(char const* ymd, std::size_t /*len*/) noexcept
        { return TypeTraits<kDate>::parse(ymd); }
        ::I operator"" _qd(unsigned long long int yyyymmdd) noexcept
        { return TypeTraits<kDate>::parse(static_cast<int>(yyyymmdd)); }

        ::F operator"" _qz(char const* ymdhmsf, std::size_t /*len*/) noexcept
        { return TypeTraits<kDatetime>::parse(ymdhmsf); }
        ::F operator"" _qz(unsigned long long int yyyymmddhhmmssf3) noexcept
        { return TypeTraits<kDatetime>::parse(static_cast<long long>(yyyymmddhhmmssf3)); }

        ::J operator"" _qn(char const* dhmsf, std::size_t /*len*/) noexcept
        { return TypeTraits<kTimespan>::parse(dhmsf); }
        ::J operator"" _qn(unsigned long long int hhmmssf9) noexcept
        { return TypeTraits<kTimespan>::parse(static_cast<long long>(hhmmssf9)); }

        ::I operator"" _qu(char const* hm, std::size_t /*len*/) noexcept
        { return TypeTraits<kMinute>::parse(hm); }
        ::I operator"" _qu(unsigned long long int hhmm) noexcept
        { return TypeTraits<kMinute>::parse(static_cast<int>(hhmm)); }

        ::I operator"" _qv(char const* hms, std::size_t /*len*/) noexcept
        { return TypeTraits<kSecond>::parse(hms); }
        ::I operator"" _qv(unsigned long long int hhmmss) noexcept
        { return TypeTraits<kSecond>::parse(static_cast<int>(hhmmss)); }

        ::I operator"" _qt(char const* hmsf, std::size_t /*len*/) noexcept
        { return TypeTraits<kTime>::parse(hmsf); }
        ::I operator"" _qt(unsigned long long int hhmmssf3) noexcept
        { return TypeTraits<kTime>::parse(static_cast<int>(hhmmssf3)); }

    }//inline namespace q::literals
}//namespace q
