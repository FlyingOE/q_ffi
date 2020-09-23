#include "ktype_traits.hpp"
#include <regex>

using namespace std::string_literals;

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

#pragma region *_to_str implementations
namespace
{
    template<q::TypeId tid>
    std::string atom_to_str(::K const k)
    {
        using Traits = q::TypeTraits<tid>;
        assert(nullptr != k && 0 > q::type(k));
        return Traits::to_str(Traits::value(k));
    }

    template<q::TypeId tid>
    std::string list_to_str(::K const k)
    {
        using Traits = q::TypeTraits<tid>;
        assert(nullptr != k && 0 < q::type(k));
        auto const len = q::count(k);
        if (0 == len) return "";

        char const* delimiter{ nullptr };
        switch (tid)
        {
        case q::kChar:
        case q::kSymbol:
            delimiter = "";
            break;
        default:
            delimiter = " ";
        }
        std::ostringstream buffer;
        auto const p = Traits::index(k);
        buffer << Traits::to_str(p[0]);
        for (auto i = 1u; i < len; ++i)
            buffer << delimiter << Traits::to_str(p[i]);
        return buffer.str();
    }

    std::string mixed_to_str(::K const k)
    {
        assert(nullptr != k && q::kMixed == q::type(k));
        return "<kMixed>";
    }

    std::string table_to_str(::K const k)
    {
        assert(nullptr != k && q::kTable == q::type(k));
        return "<kTable>";
    }

    std::string dict_to_str(::K const k)
    {
        assert(nullptr != k && q::kDict == q::type(k));
        return "<kDict>";
    }

    std::string any_to_str(::K const k)
    {
        assert(nullptr != k && 0 != q::type(k));
        auto const scalar = 0 > q::type(k);

        std::ostringstream buffer;
        buffer << '{';

        // Attribute & data type
        if (k->u) buffer << k->u << '#';
        buffer << '<' << q::type(k) << '>';

        // Contents & pointer/count
        if (scalar) {
            auto const bytes = std::max({ sizeof(k->j), sizeof(k->f), sizeof(k->s), sizeof(k->k) });
            auto const p = &q::TypeTraits<q::kByte>::value(k);
            for (auto i = 0u; i < bytes; ++i)
                buffer << q::TypeTraits<q::kByte>::to_str(p[i]);
        }
        else {
            auto const bytes = sizeof(kG(k));
            auto const p = (typename q::TypeTraits<q::kByte>::const_pointer)(&(kG(k)));
            if (!scalar) buffer << '*';
            for (auto i = 0u; i < bytes; ++i)
                buffer << q::TypeTraits<q::kByte>::to_str(p[i]);
            buffer << '[' << q::count(k) << ']';
        }

        // Reference count
        buffer << '(' << (1 + k->r) << ')';

        buffer << '}';
        return buffer.str();
    }

}//namespace <anonymous>

std::string q::to_string(::K const k)
{
#define TO_STR_BY_TYPETRAITS(tid)   \
    case -(tid):   \
        return atom_to_str<(tid)>(k);   \
    case (tid): \
        return list_to_str<(tid)>(k)

    switch (type(k))
    {
        TO_STR_BY_TYPETRAITS(kBoolean);
        TO_STR_BY_TYPETRAITS(kByte);
        TO_STR_BY_TYPETRAITS(kShort);
        TO_STR_BY_TYPETRAITS(kInt);
        TO_STR_BY_TYPETRAITS(kLong);
        TO_STR_BY_TYPETRAITS(kReal);
        TO_STR_BY_TYPETRAITS(kFloat);
        TO_STR_BY_TYPETRAITS(kChar);
        TO_STR_BY_TYPETRAITS(kSymbol);
        TO_STR_BY_TYPETRAITS(kTimestamp);
        TO_STR_BY_TYPETRAITS(kMonth);
        TO_STR_BY_TYPETRAITS(kDate);
        TO_STR_BY_TYPETRAITS(kDatetime);
        TO_STR_BY_TYPETRAITS(kTimespan);
        TO_STR_BY_TYPETRAITS(kMinute);
        TO_STR_BY_TYPETRAITS(kSecond);
        TO_STR_BY_TYPETRAITS(kTime);
    case kError:
        return atom_to_str<kError>(k);
    case kNil:
        return "\0"s;
    case kMixed:
        return mixed_to_str(k);
    case kTable:
        return table_to_str(k);
    case kDict:
        return dict_to_str(k);
    default:
        return any_to_str(k);
    }
}
#pragma endregion

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
