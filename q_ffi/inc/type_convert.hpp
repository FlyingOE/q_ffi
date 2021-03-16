#pragma once

#include "q_ffi.h"
#include <ctime>
#include <stdexcept>
#include <vector>
#include <k_compat.h>

namespace q
{
    q_ffi_API long long q2Decimal(::K x) noexcept(false);
    q_ffi_API std::vector<long long> q2Decimals(::K x) noexcept(false);

    q_ffi_API double q2Real(::K x) noexcept(false);
    q_ffi_API std::vector<double> q2Reals(::K x) noexcept(false);

    q_ffi_API std::string q2String(::K x) noexcept(false);
    q_ffi_API std::vector<std::string> q2Strings(::K x) noexcept(false);

    struct q_ffi_API tm_ext : std::tm
    {
        int32_t tm_nanos;

        int tm_millis() const noexcept
        { return tm_nanos / 1000'000; }

        using std::tm::tm;

        std::time_t mktime() noexcept;

        bool operator==(tm_ext const& rhs) const noexcept;
    };
    static_assert(sizeof(std::tm) + sizeof(tm_ext::tm_nanos) == sizeof(tm_ext),
        "q::tm_ext should be backward compatible with std::tm");

    q_ffi_API tm_ext q2DateTime(::K x) noexcept(false);
//    q_ffi_API std::vector<tm_ext> q2DateTimes(::K x) noexcept(false);

}//namespace q
