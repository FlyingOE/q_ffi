#include "ktype_traits.hpp"
#include <regex>

using namespace q;
using namespace std;
using namespace std::string_literals;

namespace q
{
    unordered_map<TypeId, char> const TypeId2Code{
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
        { kTable, ' ' },
        { kDict, ' ' },
        { kNil, '\0' },
        { kError, '\0' },
    };

}//namespace q

K_ptr
q::error(char const* msg, bool sys) noexcept
{
    return TypeTraits<kError>::atom(msg, sys);
}

#pragma region *_to_str implementations
namespace
{
    template<TypeId tid>
    string atom_to_str(::K const k)
    {
        using Traits = TypeTraits<tid>;
        assert(nullptr != k && 0 > type(k));
        return Traits::to_str(Traits::value(k));
    }

    template<TypeId tid>
    string list_to_str(::K const k)
    {
        using Traits = TypeTraits<tid>;
        assert(nullptr != k && 0 < type(k));
        auto const len = count(k);
        if (0 == len) return "";

        char const* delimiter{ nullptr };
        switch (tid)
        {
        case kChar:
        case kSymbol:
            delimiter = "";
            break;
        default:
            delimiter = " ";
        }
        ostringstream buffer;
        auto const p = Traits::index(k);
        buffer << Traits::to_str(p[0]);
        for (auto i = 1u; i < len; ++i)
            buffer << delimiter << Traits::to_str(p[i]);
        return buffer.str();
    }

#   pragma warning(disable: 4100)

    string mixed_to_str(::K const k)
    {
        assert(nullptr != k && kMixed == type(k));
        return "<kMixed>";
    }

    string table_to_str(::K const k)
    {
        assert(nullptr != k && kTable == type(k));
        return "<kTable>";
    }

    string dict_to_str(::K const k)
    {
        assert(nullptr != k && kDict == type(k));
        return "<kDict>";
    }

#   pragma warning(default: 4100)

    string any_to_str(::K const k)
    {
        assert(nullptr != k && 0 != type(k));
        auto const scalar = 0 > type(k);

        ostringstream buffer;
        buffer << '{';

        // Attribute & data type
        if (k->u) buffer << k->u << '#';
        buffer << '<' << type(k) << '>';

        // Contents & pointer/count
        if (scalar) {
            auto const bytes = max({ sizeof(k->j), sizeof(k->f), sizeof(k->s), sizeof(k->k) });
            auto const p = &TypeTraits<kByte>::value(k);
            for (auto i = 0u; i < bytes; ++i)
                buffer << TypeTraits<kByte>::to_str(p[i]);
        }
        else {
            auto const bytes = sizeof(kG(k));
            auto const p = (typename TypeTraits<kByte>::const_pointer)(&(kG(k)));
            if (!scalar) buffer << '*';
            for (auto i = 0u; i < bytes; ++i)
                buffer << TypeTraits<kByte>::to_str(p[i]);
            buffer << '[' << count(k) << ']';
        }

        // Reference count
        buffer << '(' << (1 + k->r) << ')';

        buffer << '}';
        return buffer.str();
    }

}//namespace <anonymous>

string
q::to_string(::K const k)
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
        return k ? atom_to_str<kError>(k) : "\0"s;
    case kNil:
        return "::";
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

string
q::to_string(K_ptr const& k)
{
    return to_string(k.get());
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

        ::J operator"" _qp(char const* ymdhmsf, size_t /*len*/) noexcept
        { return TypeTraits<kTimestamp>::parse(ymdhmsf); }
        ::J operator"" _qp(char const* yyyymmddhhmmssf9) noexcept
        { return TypeTraits<kTimestamp>::parse(yyyymmddhhmmssf9, true); }

        ::I operator"" _qm(char const* ym, size_t /*len*/) noexcept
        { return TypeTraits<kMonth>::parse(ym); }
        ::I operator"" _qm(unsigned long long int yyyymm) noexcept
        { return TypeTraits<kMonth>::parse(static_cast<int>(yyyymm)); }

        ::I operator"" _qd(char const* ymd, size_t /*len*/) noexcept
        { return TypeTraits<kDate>::parse(ymd); }
        ::I operator"" _qd(unsigned long long int yyyymmdd) noexcept
        { return TypeTraits<kDate>::parse(static_cast<int>(yyyymmdd)); }

        ::F operator"" _qz(char const* ymdhmsf, size_t /*len*/) noexcept
        { return TypeTraits<kDatetime>::parse(ymdhmsf); }
        ::F operator"" _qz(unsigned long long int yyyymmddhhmmssf3) noexcept
        { return TypeTraits<kDatetime>::parse(static_cast<long long>(yyyymmddhhmmssf3)); }

        ::J operator"" _qn(char const* dhmsf, size_t /*len*/) noexcept
        { return TypeTraits<kTimespan>::parse(dhmsf); }
        ::J operator"" _qn(unsigned long long int hhmmssf9) noexcept
        { return TypeTraits<kTimespan>::parse(static_cast<long long>(hhmmssf9)); }

        ::I operator"" _qu(char const* hm, size_t /*len*/) noexcept
        { return TypeTraits<kMinute>::parse(hm); }
        ::I operator"" _qu(unsigned long long int hhmm) noexcept
        { return TypeTraits<kMinute>::parse(static_cast<int>(hhmm)); }

        ::I operator"" _qv(char const* hms, size_t /*len*/) noexcept
        { return TypeTraits<kSecond>::parse(hms); }
        ::I operator"" _qv(unsigned long long int hhmmss) noexcept
        { return TypeTraits<kSecond>::parse(static_cast<int>(hhmmss)); }

        ::I operator"" _qt(char const* hmsf, size_t /*len*/) noexcept
        { return TypeTraits<kTime>::parse(hmsf); }
        ::I operator"" _qt(unsigned long long int hhmmssf3) noexcept
        { return TypeTraits<kTime>::parse(static_cast<int>(hhmmssf3)); }

    }//namespace q::literals
}//namespace q
