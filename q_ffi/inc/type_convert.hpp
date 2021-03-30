#pragma once

#include "q_ffi.h"
#include <vector>
#include <date/date.h>
#include <k_compat.h>
#include <ktype_traits.hpp>

namespace q
{
    q_ffi_API long long q2Decimal(::K x, bool dryRun = false) noexcept(false);

    inline long long q2Decimal(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2Decimal(x.get(), dryRun);
    }

    q_ffi_API std::vector<long long> q2Decimals(::K x, bool dryRun = false) noexcept(false);

    inline std::vector<long long> q2Decimals(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2Decimals(x.get(), dryRun);
    }

    q_ffi_API double q2Real(::K x, bool dryRun = false) noexcept(false);

    inline double q2Real(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2Real(x.get(), dryRun);
    }

    q_ffi_API std::vector<double> q2Reals(::K x, bool dryRun = false) noexcept(false);

    inline std::vector<double> q2Reals(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2Reals(x.get(), dryRun);
    }

    q_ffi_API char q2Char(::K x, bool dryRun = false) noexcept(false);

    inline char q2Char(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2Char(x.get(), dryRun);
    }

    q_ffi_API std::string q2String(::K x, bool dryRun = false) noexcept(false);

    inline std::string q2String(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2String(x.get(), dryRun);
    }

    q_ffi_API std::vector<std::string> q2Strings(::K x, bool dryRun = false) noexcept(false);

    inline std::vector<std::string> q2Strings(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2Strings(x.get(), dryRun);
    }

    template<typename TimePt>
    TimePt q2TimePoint(::K x, bool dryRun = false) noexcept(false);

    template<typename TimePt>
    TimePt q2TimePoint(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2TimePoint<TimePt>(x.get(), dryRun);
    }

    template<typename TimePt>
    std::vector<TimePt> q2TimePoints(::K x, bool dryRun = false) noexcept(false);

    template<typename TimePt>
    std::vector<TimePt> q2TimePoints(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2TimePoints<TimePt>(x.get(), dryRun);
    }

    template<typename Duration>
    Duration q2TimeSpan(::K x, bool dryRun = false) noexcept(false);

    template<typename Duration>
    Duration q2TimeSpan(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2TimeSpan<Duration>(x.get(), dryRun);
    }

    template<typename Duration>
    std::vector<Duration> q2TimeSpans(::K x, bool dryRun = false) noexcept(false);

    template<typename Duration>
    std::vector<Duration> q2TimeSpans(K_ptr const& x, bool dryRun = false) noexcept(false)
    {
        return q2TimeSpans<Duration>(x.get(), dryRun);
    }

}//namespace q

#pragma region Temporal type conversion implementations

namespace q
{
    namespace details
    {
        q_ffi_API void signalError(char const* message) noexcept(false);

        template<typename ToDuration, TypeId tid>
        date::sys_time<ToDuration> get_time_point(
            typename TypeTraits<tid>::const_reference x) noexcept
        {
            auto const t = TypeTraits<tid>::decode(x);
            return date::sys_time<ToDuration>{
                std::chrono::duration_cast<ToDuration>(t.time_since_epoch())
            };
        }

        template<typename ToDuration, TypeId tid>
        date::sys_time<ToDuration> get_time_point(::K k) noexcept
        {
            assert(nullptr != k);
            return get_time_point<ToDuration, tid>(TypeTraits<tid>::value(k));
        }

        template<typename ToDuration, TypeId tid>
        ToDuration get_time_span(typename TypeTraits<tid>::const_reference x) noexcept
        {
            auto const t = TypeTraits<tid>::decode(x);
            return std::chrono::duration_cast<ToDuration>(t);
        }

        template<typename ToDuration, TypeId tid>
        ToDuration get_time_span(::K k) noexcept
        {
            assert(nullptr != k);
            return get_time_span<ToDuration, tid>(TypeTraits<tid>::value(k));
        }

    }//namespace q::details
}//namespace q

template<typename TimePt>
TimePt q::q2TimePoint(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        details::signalError("nil timepoint");
    }

#   define GET_TIME_POINT(T, Dur, x, dryRun)    \
        case -(T):  \
            return (dryRun) ? TimePt{} : details::get_time_point<Dur, (T)>((x))
    switch (type(x)) {
        GET_TIME_POINT(kTimestamp, typename TimePt::duration, x, dryRun);
        GET_TIME_POINT(kMonth, typename TimePt::duration, x, dryRun);
        GET_TIME_POINT(kDate, typename TimePt::duration, x, dryRun);
        GET_TIME_POINT(kDatetime, typename TimePt::duration, x, dryRun);
    default:
        details::signalError("not a timepoint");
        assert(!"Shall never arrive here!");
        return TimePt{};
    }
#   undef GET_TIME_POINT
}

template<typename TimePt>
std::vector<TimePt> q::q2TimePoints(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        details::signalError("nil time-point list");
    }
    else if (type(x) < 0) {
        details::signalError("not a list");
    }

    assert(count(x) >= 0);
    std::vector<TimePt> result(dryRun ? 0 : count(x));

#   define GET_TIME_POINTS(T, Dur, x, r, dryRun)    \
        case (T):  \
            if (!(dryRun))  \
                std::transform(TypeTraits<(T)>::index((x)), \
                    TypeTraits<(T)>::index((x)) + count((x)),   \
                    (r).begin(),    \
                    [](auto const t) { return details::get_time_point<Dur, (T)>(t); }); \
            break
    switch (type(x)) {
        GET_TIME_POINTS(kTimestamp, typename TimePt::duration, x, result, dryRun);
        GET_TIME_POINTS(kMonth, typename TimePt::duration, x, result, dryRun);
        GET_TIME_POINTS(kDate, typename TimePt::duration, x, result, dryRun);
        GET_TIME_POINTS(kDatetime, typename TimePt::duration, x, result, dryRun);
    default:
        details::signalError("not a time-point list");
    }
#   undef GET_TIME_POINTS

    return result;
}

template<typename Duration>
Duration q::q2TimeSpan(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        details::signalError("nil time-span");
    }

#   define GET_TIME_SPAN(T, Dur, x, dryRun) \
        case -(T):  \
            return (dryRun) ? Duration{} : details::get_time_span<Dur, (T)>((x))
    switch (type(x)) {
        GET_TIME_SPAN(kTimespan, Duration, x, dryRun);
        GET_TIME_SPAN(kMinute, Duration, x, dryRun);
        GET_TIME_SPAN(kSecond, Duration, x, dryRun);
        GET_TIME_SPAN(kTime, Duration, x, dryRun);
    default:
        details::signalError("not a time-span");
        assert(!"Shall never arrive here!");
        return Duration{};
    }
#   undef GET_TIME_SPAN
}

template<typename Duration>
std::vector<Duration> q::q2TimeSpans(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        details::signalError("nil time-span list");
    }
    else if (type(x) < 0) {
        details::signalError("not a list");
    }

    assert(count(x) >= 0);
    std::vector<Duration> result(dryRun ? 0 : count(x));

#   define GET_TIME_SPANS(T, Dur, x, r, dryRun) \
        case (T):  \
            if (!(dryRun))  \
                std::transform(TypeTraits<(T)>::index((x)), \
                    TypeTraits<(T)>::index((x)) + count((x)),   \
                    (r).begin(),    \
                    [](auto const t) { return details::get_time_span<Dur, (T)>(t); });  \
            break
    switch (type(x)) {
        GET_TIME_SPANS(kTimespan, Duration, x, result, dryRun);
        GET_TIME_SPANS(kMinute, Duration, x, result, dryRun);
        GET_TIME_SPANS(kSecond, Duration, x, result, dryRun);
        GET_TIME_SPANS(kTime, Duration, x, result, dryRun);
    default:
        details::signalError("not a time-span list");
    }
#   undef GET_TIME_SPANS

    return result;
}

#pragma endregion
