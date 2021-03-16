#pragma once

#include "q_ffi.h"
#include <unordered_map>
#include <date/date.h>
#include <k_compat.h>
#include <iosfwd>

namespace q
{
    /// @brief All q data type IDs, same as those returnd by @c type function in q.
    enum TypeId : ::H
    {
        kMixed = 0,
        kBoolean = (KB),
        kGUID = (UU),
        kByte = (KG),
        kShort = (KH),
        kInt = (KI),
        kLong = (KJ),
        kReal = (KE),
        kFloat = (KF),
        kChar = (KC),
        kSymbol = (KS),
        kTimestamp = (KP),
        kMonth = (KM),
        kDate = (KD),
        kDatetime = (KZ),
        kTimespan = (KN),
        kMinute = (KU),
        kSecond = (KV),
        kTime = (KT),
        kEnumMin = 20,
        kEnumMax = 76,
        kTable = (XT),
        kDict = (XD),
        kNil = 101,
        kError = -128
    };

    template<typename Elem, typename ElemTraits>
    std::basic_ostream<Elem, ElemTraits>& operator<<(
        std::basic_ostream<Elem, ElemTraits>& os, TypeId tid)
    {
#   define Q_TYPEID_OUTPUT(T)  \
        case -(k##T): return os << "{" #T "}";  \
        case (k##T): return os << "{" #T "s}"
        switch (tid)
        {
        case kMixed: return os << "{(...)}";
            Q_TYPEID_OUTPUT(Boolean);
            Q_TYPEID_OUTPUT(GUID);
            Q_TYPEID_OUTPUT(Byte);
            Q_TYPEID_OUTPUT(Short);
            Q_TYPEID_OUTPUT(Int);
            Q_TYPEID_OUTPUT(Long);
            Q_TYPEID_OUTPUT(Real);
            Q_TYPEID_OUTPUT(Float);
            Q_TYPEID_OUTPUT(Char);
            Q_TYPEID_OUTPUT(Symbol);
            Q_TYPEID_OUTPUT(Timestamp);
            Q_TYPEID_OUTPUT(Month);
            Q_TYPEID_OUTPUT(Date);
            Q_TYPEID_OUTPUT(Datetime);
            Q_TYPEID_OUTPUT(Timespan);
            Q_TYPEID_OUTPUT(Minute);
            Q_TYPEID_OUTPUT(Second);
            Q_TYPEID_OUTPUT(Time);
        case kTable: return os << "{Table}";
        case kDict: return os << "{Dict}";
        case kNil: return os << "{(::)}";
        case kError: return os << "{'Error}";
        default:
            if (-kEnumMin >= tid && tid >= -kEnumMax)
                return os << "{Enum}";
            else if (kEnumMin <= tid && tid <= kEnumMax)
                return os << "{Enums}";
            else
                return os << '{' << tid << '}';
        }
#   undef Q_TYPEID_OUTPUT
    }

    /// @brief Map @c TypeId to single-character type code in q.
    q_ffi_API extern std::unordered_map<TypeId, char> const TypeCode;

    /// @brief Generic nil value in q.
    /// @ref q::kNil
    constexpr ::K Nil = static_cast<::K>(nullptr);

    /// @brief kdb+ epoch.
    static constexpr auto Epoch = date::January / 1 / 2000;

#   pragma region K object queries

    /// @brief Inspect type ID of a (potentially null) @c K object.
    inline std::underlying_type_t<TypeId> type(::K const k) noexcept
    {
        return nullptr == k ? q::kNil : static_cast<TypeId>(k->t);
    }

    /// @brief Inspect the element count of a (potentially null) @c K object. Atoms' size is 1.
    inline std::size_t count(::K const k) noexcept
    {
        return nullptr == k ? 0 : 0 > type(k) ? 1 : static_cast<std::size_t>(k->n);
    }

#   pragma endregion

    /// @brief Report error into q host.
    /// @param sys If the error should be prepended with system error message.
    q_ffi_API ::K error(char const* msg, bool sys = false) noexcept;

    /// @brief Stringize any @c K object as much as possible.
    ///     If the q type is recognized, @c k is converted using the @c to_str method in the respective type traits.
    q_ffi_API std::string to_string(::K const k);

#   pragma region kdb+ temporal type representations

    /// @brief kdb+ date/month (time point).
    using Date = date::sys_days;

    /// @brief kdb+ minute (time duration).
    using Minutes = std::chrono::minutes;

    /// @brief kdb+ second (time duration).
    using Seconds = std::chrono::seconds;

    /// @brief kdb+ time (time duration).
    using Milliseconds = std::chrono::milliseconds;

    /// @brief kdb+ datetime (time point).
    using DateTime = date::sys_time<std::chrono::milliseconds>;

    /// @brief kdb+ timespan (time duration).
    using Nanoseconds = std::chrono::nanoseconds;

    /// @brief kdb+ timestamp (time point).
    using Timestamp = date::sys_time<std::chrono::nanoseconds>;

#   pragma endregion

    /// @brief UDLs that are adapted from q literal suffices.
    inline namespace literals
    {
        q_ffi_API ::G operator"" _qb(unsigned long long b) noexcept;

        q_ffi_API ::G operator"" _qx(unsigned long long i8) noexcept;
        q_ffi_API ::H operator"" _qh(unsigned long long i16) noexcept;
        q_ffi_API ::I operator"" _qi(unsigned long long i32) noexcept;
        q_ffi_API ::J operator"" _qj(unsigned long long i64) noexcept;

        q_ffi_API ::E operator"" _qe(long double f32) noexcept;
        q_ffi_API ::F operator"" _qf(long double f64) noexcept;

        q_ffi_API ::J operator"" _qp(char const* ymdhmsf, std::size_t len) noexcept;
        q_ffi_API ::J operator"" _qp(char const* yyyymmddhhmmssf9) noexcept;

        q_ffi_API ::I operator"" _qm(char const* ym, std::size_t len) noexcept;
        q_ffi_API ::I operator"" _qm(unsigned long long int yyyymm) noexcept;

        q_ffi_API ::I operator"" _qd(char const* ymd, std::size_t len) noexcept;
        q_ffi_API ::I operator"" _qd(unsigned long long int yyyymmdd) noexcept;

        q_ffi_API ::F operator"" _qz(char const* ymdhmsf, std::size_t len) noexcept;
        q_ffi_API ::F operator"" _qz(unsigned long long int yyyymmddhhmmssf3) noexcept;

        q_ffi_API ::J operator"" _qn(char const* dhmsf, std::size_t len) noexcept;
        q_ffi_API ::J operator"" _qn(unsigned long long int hhmmssf9) noexcept;

        q_ffi_API ::I operator"" _qu(char const* hm, std::size_t len) noexcept;
        q_ffi_API ::I operator"" _qu(unsigned long long int hhmm) noexcept;

        q_ffi_API ::I operator"" _qv(char const* hms, std::size_t len) noexcept;
        q_ffi_API ::I operator"" _qv(unsigned long long int hhmmss) noexcept;

        q_ffi_API ::I operator"" _qt(char const* hmsf, std::size_t len) noexcept;
        q_ffi_API ::I operator"" _qt(unsigned long long int hhmmssf3) noexcept;

    }//inline namespace q::literals

}//namespace q

#pragma endregion
