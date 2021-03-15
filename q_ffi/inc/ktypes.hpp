#pragma once

#include "q_ffi.h"
#include <unordered_map>
#include <date/date.h>
#include <k_compat.h>

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

    /// @brief Map @c TypeId to single-character type code in q.
    q_ffi_API extern std::unordered_map<TypeId, char> const TypeCode;

    /// @brief Generic nil value in q.
    /// @ref q::kNil
    constexpr ::K Nil = static_cast<::K>(nullptr);

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

    /// @brief Report error into q host.
    /// @param sys If the error should be prepended with system error message.
    q_ffi_API ::K error(char const* msg, bool sys = false) noexcept;

    /// @brief Stringize any @c K object as much as possible.
    ///     If the q type is recognized, @c k is converted using the @c to_str method in the respective type traits.
    q_ffi_API std::string to_string(::K const k);

    /// @brief kdb+ date/month.
    using Days = date::sys_days;

    /// @brief kdb+ minute/second.
    using Seconds = date::sys_seconds;

    /// @brief kdb+ time/datetime.
    using Milliseconds = date::sys_time<std::chrono::milliseconds>;

    /// @brief kdb+ timespan/timestamp.
    using Nanoseconds = date::sys_time<std::chrono::nanoseconds>;

    /// @brief kdb+ epoch.
    static constexpr auto Epoch = date::January / 1 / 2000;

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

    /// (Time units) number of Num per unit of Denom
    template<typename Num, typename Denom>
    struct time_scale
    {
        static constexpr auto value =
            std::chrono::duration_cast<Num>(Denom{ 1 }).count();
    };

    template<typename Num, typename Denom>
    inline constexpr auto time_scale_v = time_scale<Num, Denom>::value;

}//namespace q

#pragma endregion
