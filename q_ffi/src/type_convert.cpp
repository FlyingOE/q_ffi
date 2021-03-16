#include "type_convert.hpp"
#include <algorithm>
#include <date/date.h>
#include "ktype_traits.hpp"
#include "kerror.hpp"

#define GET_VALUE_TYPE(T, x)  \
    case -(T): \
        return TypeTraits<(T)>::value((x))

#define GET_VALUE_LIST(T, x, r)  \
    case (T):  \
        std::copy(TypeTraits<(T)>::index((x)), TypeTraits<(T)>::index((x)) + count((x)),    \
            (r).begin());   \
        break

#pragma region q <==> C++ decimal

long long q::q2Decimal(::K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil decimal");
    }
    switch (type(x))
    {
        GET_VALUE_TYPE(kBoolean, x);
        GET_VALUE_TYPE(kByte, x);
        GET_VALUE_TYPE(kShort, x);
        GET_VALUE_TYPE(kInt, x);
        GET_VALUE_TYPE(kLong, x);
    default:
        throw K_error("not a decimal");
    }
}

std::vector<long long> q::q2Decimals(::K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil decimal list");
    }
    else if (type(x) < 0) {
        throw K_error("not a list");
    }
    assert(count(x) >= 0);
    std::vector<long long> result(count(x), 0L);
    switch (type(x))
    {
        GET_VALUE_LIST(kBoolean, x, result);
        GET_VALUE_LIST(kByte, x, result);
        GET_VALUE_LIST(kShort, x, result);
        GET_VALUE_LIST(kInt, x, result);
        GET_VALUE_LIST(kLong, x, result);
    default:
        throw K_error("not a decimal list");
    }
    return std::move(result);
}

#pragma endregion

#pragma region q <==> C++ floating-point

double q::q2Real(::K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil floating-point");
    }
    switch (type(x))
    {
        GET_VALUE_TYPE(kReal, x);
        GET_VALUE_TYPE(kFloat, x);
    case -kBoolean:
    case -kByte:
    case -kShort:
    case -kInt:
    case -kLong:
        return static_cast<double>(q2Decimal(x));
    default:
        throw K_error("not a floating-point");
    }
}

std::vector<double> q::q2Reals(::K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil floating-point list");
    }
    else if (type(x) < 0) {
        throw K_error("not a list");
    }
    assert(count(x) >= 0);
    std::vector<double> result(count(x), 0.);
    switch (type(x))
    {
        GET_VALUE_LIST(kReal, x, result);
        GET_VALUE_LIST(kFloat, x, result);
    case kBoolean:
    case kByte:
    case kShort:
    case kInt:
    case kLong: {
        auto const decimals = q2Decimals(x);
        std::transform(decimals.cbegin(), decimals.cend(), result.begin(),
            [](auto x) { return static_cast<double>(x); });
        break;
    }
    default:
        throw K_error("not a floating-point list");
    }
    return std::move(result);
}

#pragma endregion

#pragma region q <==> C++ string

std::string q::q2String(::K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil symbol/char list");
    }
    switch (type(x)) {
    case kChar:
        assert(count(x) >= 0);
        return std::string(TypeTraits<kChar>::index(x), count(x));
    case -kSymbol:
        return std::string(TypeTraits<kSymbol>::value(x));
    default:
        if ((-kEnumMin >= type(x)) && (type(x) >= -kEnumMax))
            throw K_error("enumerated symbol");
        else
            throw K_error("not a symbol/char list");
    }
}

std::vector<std::string> q::q2Strings(K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil symbol list/char lists");
    }
    else if (type(x) < 0) {
        throw K_error("not a list");
    }
    assert(count(x) >= 0);
    std::vector<std::string> result;
    result.reserve(count(x));
    switch (type(x)) {
    case kMixed:
        try {
            std::for_each(TypeTraits<kMixed>::index(x), TypeTraits<kMixed>::index(x) + count(x),
                [&result](::K s) { result.push_back(q2String(s)); });
        }
        catch (K_error const& ) {
            throw K_error("mixed char lists");
        }
        break;
    case kSymbol:
        std::for_each(TypeTraits<kSymbol>::index(x), TypeTraits<kSymbol>::index(x) + count(x),
            [&result](auto const s) { result.push_back(s); });
        break;
    default:
        if ((kEnumMin <= type(x)) && (type(x) <= kEnumMax))
            throw K_error("enumerated symbol list");
        else
            throw K_error("not a symbol list/char lists");
    }
    assert(result.size() == count(x));
    return std::move(result);
}

#pragma endregion

#pragma region Unix <==> q temporal types

void q::details::signalError(char const* message) noexcept(false)
{
    throw K_error(message);
}

/*
#ifdef _MSC_VER
namespace ms_ext
{
    /// @ref https://support.microsoft.com/en-us/kb/167296 (Deprecated)
    /// @ref https://mskb.pkisolutions.com/kb/167296
    /// @ref https://stackoverflow.com/questions/44953084/how-to-pass-time-t-or-simply-its-date-information-over-com
    BOOL UnixTimeToSystemTime(std::time_t const t, LPSYSTEMTIME pst) noexcept
    {
        static auto const TICKS_PER_SEC = 10'000'000uLL;

        ULARGE_INTEGER time{};
        time.QuadPart = Int32x32To64(t, TICKS_PER_SEC) + 116'444'736'000'000'000uLL;
        if (time.QuadPart >= 0x8000'0000'0000'0000uLL)
            return FALSE;

        static_assert(sizeof(ULARGE_INTEGER) == sizeof(FILETIME), "FILETIME as 64-bit integer");
        assert(nullptr != pst);
        // FIXME: This cannot handle days after Y2k38!
        return ::FileTimeToSystemTime(reinterpret_cast<FILETIME const*>(&time), pst);
    }

    /// MSVC's <time.h> is buggy:
    ///  1. It does not provide ::gmtime_r;
    ///  2. Its ::gmtime_s implementation cannot handle anything before Unix epoch!
    /// FIXME: Y2k38 bug!
    std::tm* gmtime_r(std::time_t const* time, std::tm* buf) noexcept
    {
        try {
            if (nullptr == time || nullptr == buf)
                throw std::invalid_argument("null time/buf");

            SYSTEMTIME systime{};
            auto const status = UnixTimeToSystemTime(*time, &systime);
            if (!status)
                throw std::invalid_argument("invalid time");
            buf->tm_year = systime.wYear - 1900;
            buf->tm_mon = systime.wMonth - 1;
            buf->tm_mday = systime.wDay;
            buf->tm_hour = systime.wHour;
            buf->tm_min = systime.wMinute;
            buf->tm_sec = systime.wSecond;
            buf->tm_wday = systime.wDayOfWeek;

            auto const tt = reinterpret_cast<q::tm_ext*>(buf)->mktime();
            if (tt != *time)
                throw std::invalid_argument("time out of range?");
            return buf;
        }
        catch (std::invalid_argument const& ) {
            return nullptr;
        }
    }

}//namespace ms_ext
#endif

/// Utilities to convert between Unix and q temporal types.
/// @ref https://code.kx.com/q/interfaces/c-client-for-q/#strings-and-datetimes
namespace kx
{
    auto const NANOS_PER_SEC = 1'000'000'000uLL;
    auto const MILLIS_PER_SEC = 1'000uLL;
    auto const SECS_PER_MIN = 60u;
    auto const MINS_PER_HOUR = 60u;
    auto const HOURS_PER_DAY = 24u;
    auto const SECS_PER_DAY = HOURS_PER_DAY * MINS_PER_HOUR * SECS_PER_MIN;
    auto const KDB_EPOCH = 10957;   //kdb+ epoch since unix epoch (in days)

    struct DateTime
    {
        ::I days;   //days since kdb+ epoch (2000.01.01T00:00:00)
        ::J nanos;  //nanoseconds in the day
    };

    /// kdb+ datetime from unix
    ///<code>
    /// F zu(I u) { return u / 8.64e4 - 10957; }
    ///</code.
    DateTime unix2z(std::time_t secs, uint64_t nanos = 0) noexcept
    {
        return DateTime{
            static_cast<::I>(secs / SECS_PER_DAY - KDB_EPOCH),
            static_cast<::J>(secs % SECS_PER_DAY * NANOS_PER_SEC + nanos)
        };
    }

    /// unix from kdb+ datetime
    ///<code>
    /// I uz(F f) { return 86400 * (f + 10957); }
    ///</code>
    std::time_t z2unix(::F z) noexcept
    {
        return static_cast<std::time_t>((z + KDB_EPOCH) * SECS_PER_DAY);
    }
    std::time_t z2unix(DateTime const& z, uint64_t* nanos = nullptr) noexcept
    {
        if (nullptr != nanos)
            *nanos = z.nanos % NANOS_PER_SEC;
        return z2unix(z.days + z.nanos / static_cast<::F>(NANOS_PER_SEC * SECS_PER_DAY));
    }

    /// kdb+ timestamp from unix, use ktj(Kj,n) to create timestamp from n
    ///<code>
    /// J pu(J u) { return 1000000LL * (u - 10957LL * 86400000LL); }
    ///</code>
    ::J unix2p(std::time_t u, uint64_t nanos = 0) noexcept
    {
        return (u - KDB_EPOCH * SECS_PER_DAY) * NANOS_PER_SEC + nanos;
    }

    /// unix from kdb+ timestamp
    ///<code>
    /// I up(J f) { return (f / 8.64e13 + 10957) * 8.64e4; }
    ///</code>
    std::time_t p2unix(::J p, uint64_t* nanos = nullptr) noexcept
    {
        if (nullptr != nanos)
            *nanos = p % NANOS_PER_SEC;
        return p / NANOS_PER_SEC + KDB_EPOCH * SECS_PER_DAY;
    }

    ///<code>
    /// struct tm* lt(int kd) { time_t t = uz(kd); return localtime(&t); }
    /// struct tm* lt_r(int kd, struct tm* res) { time_t t = uz(kd); return localtime_r(&t, res); }
    ///</code>

    ///<code>
    /// struct tm* gt(int kd) { time_t t = uz(kd); return gmtime(&t); }
    /// struct tm* gt_r(int kd, struct tm* res) { time_t t = uz(kd); return gmtime_r(&t, res); }
    ///</code>
    q::tm_ext& z2gmtime_impl(std::time_t const& t, uint64_t n, q::tm_ext& tm) noexcept(false)
    {
#   ifdef _MSC_VER
        auto const p = ms_ext::gmtime_r(&t, &tm);
#   else
        auto const p = ::gmtime_r(&t, &tm);
#   endif
        if (nullptr == p) {
            throw std::runtime_error("time value invalid or out of range");
        }
        else {
            assert(p == &tm);
            tm.tm_nanos = static_cast<decltype(tm.tm_nanos)>(n);
        }
        return tm;
    }

    q::tm_ext& z2gmtime(::F z, q::tm_ext& tm) noexcept(false)
    {
        auto const t = z2unix(z);
        return z2gmtime_impl(t, 0, tm);
    }
    q::tm_ext& z2gmtime(DateTime const& z, q::tm_ext& tm) noexcept(false)
    {
        uint64_t nanos = 0;
        auto const t = z2unix(z, &nanos);
        z2gmtime_impl(t, nanos, tm);
        return z2gmtime_impl(t, nanos, tm);
    }

/*
    ///<code>
    /// char* fdt(struct tm* ptm, char* d) { strftime(d, 10, "%Y.%m.%d", ptm); return d; }
    ///</code>
    char* formatDate(q::tm_ext const& dt, char* s) noexcept
    {
        auto const FORMAT = "%Y.%m.%d";
        std::size_t const SIZE = 4 + 1 + 2 + 1 + 2;

        assert(nullptr != s);
        auto const count = std::strftime(s, SIZE + 1, FORMAT, &dt);
        assert(SIZE == count);
        assert('\0' == s[SIZE]);
        return s;
    }

    ///<code>
    /// void tsms(unsigned ts, char* h, char* m, char* s, short* mmm) { *h = ts / 3600000; ts -= 3600000 * (*h); *m = ts / 60000; ts -= 60000 * (*m); *s = ts / 1000; ts -= 1000 * (*s); *mmm = ts; }
    /// char* ftsms(unsigned ts, char* d) { char h, m, s; short mmm; tsms(ts, &h, &m, &s, &mmm); sprintf(d, "%02d:%02d:%02d.%03d", h, m, s, mmm); return d; }
    ///</code>

    char* formatTime(q::tm_ext const& dt, char* s, bool millis = true) noexcept
    {
        auto const FORMAT = "%H:%M:%S.";
        std::size_t const SIZE = 2 + 1 + 2 + 1 + 2 + 1;
        auto const FORMAT_MILLIS = "%3d";
        std::size_t const SIZE_MILLIS = 3;
        auto const FORMAT_NANOS = "%9d";
        std::size_t const SIZE_NANOS = 9;

        assert(nullptr != s);
        auto count = std::strftime(s, SIZE + 1, FORMAT, &dt);
        assert(SIZE == count);
        if (millis) {
            count = std::snprintf(s + SIZE, SIZE_MILLIS + 1, FORMAT_MILLIS, dt.tm_millis());
            assert(SIZE_MILLIS == count);
            assert('\0' == s[SIZE + SIZE_MILLIS]);
        }
        else {
            count = std::snprintf(s + SIZE, SIZE_NANOS + 1, FORMAT_NANOS, dt.tm_nanos);
            assert(SIZE_NANOS == count);
            assert('\0' == s[SIZE + SIZE_NANOS]);
        }
        return s;
    }

    char* formatDatetime(q::tm_ext const& dt, char* s, bool millis = true) noexcept
    {
        auto p = s;
        auto const d = formatDate(dt, p);
        assert(p == d);
        auto const dateLen = std::strlen(s);
        assert('\0' == s[dateLen]);
        s[dateLen] = millis ? 'T' : 'D';
        p += dateLen + 1;
        auto const t = formatTime(dt, p, millis);
        assert(p == t);
        return s;
    }
* /

}//namespace kx
#pragma endregion

#pragma region q <==> C++ DateTime

/// std::mktime in <ctime> uses local time, but we need UTC time instead!
std::time_t q::tm_ext::mktime() noexcept
{
#ifdef _MSC_VER
    auto t = ::_mkgmtime(this);
    if (-1 == t) {
        // MSVC's <time.h> cannot handle days before epoch or after Y2k38!
        // @ref https://cs.uwaterloo.ca/~alopez-o/math-faq/node73.html
        static auto const DayOfWeek = [](int y, int m, int d) {
            y -= m < 3;
            return (y + y / 4 - y / 100 + y / 400 + "-bed=pen+mad."[m] + d) % 7;
        };
        this->tm_wday = DayOfWeek(this->tm_year + 1900, this->tm_mon + 1, this->tm_mday);
    }
    return t;
#else
    return ::timegm(this);
#endif
}

bool q::tm_ext::operator==(q::tm_ext const& rhs) const noexcept
{
    return 0 == std::memcmp(this, &rhs, sizeof(tm_ext));
}

namespace
{
    template <q::TypeId tid>
    struct DateTimeConverter;

    template <>
    struct DateTimeConverter<q::kDate>
    {
        using arg_type = typename q::TypeTraits<q::kDate>::const_reference;

        kx::DateTime operator()(arg_type day) const noexcept
        {
            return kx::DateTime{ day, 0 };
        }
    };

    template <>
    struct DateTimeConverter<q::kTime>
    {
        using arg_type = typename q::TypeTraits<q::kTime>::const_reference;

        kx::DateTime operator()(arg_type millis) const noexcept
        {
            return kx::DateTime{
                static_cast<::I>(millis / (kx::MILLIS_PER_SEC * kx::SECS_PER_DAY)),
                static_cast<::J>(millis % (kx::MILLIS_PER_SEC * kx::SECS_PER_DAY)
                                        * (kx::NANOS_PER_SEC / kx::MILLIS_PER_SEC)),
            };
        }
    };

    template <>
    struct DateTimeConverter<q::kSecond>
    {
        using arg_type = typename q::TypeTraits<q::kSecond>::const_reference;

        kx::DateTime operator()(arg_type secs) const noexcept
        {
            return kx::DateTime{
                static_cast<::I>(secs / kx::SECS_PER_DAY),
                static_cast<::J>(secs % kx::SECS_PER_DAY * kx::NANOS_PER_SEC),
            };
        }
    };

    template <>
    struct DateTimeConverter<q::kMinute>
    {
        using arg_type = typename q::TypeTraits<q::kMinute>::const_reference;

        kx::DateTime operator()(arg_type mins) const noexcept
        {
            return kx::DateTime{
                static_cast<::I>(mins / (kx::MINS_PER_HOUR * kx::HOURS_PER_DAY)),
                static_cast<::J>(mins % (kx::MINS_PER_HOUR * kx::HOURS_PER_DAY)
                                        * (kx::NANOS_PER_SEC * kx::SECS_PER_MIN)),
            };
        }
    };

    template <>
    struct DateTimeConverter<q::kDatetime>
    {
        using arg_type = typename q::TypeTraits<q::kDatetime>::const_reference;

        kx::DateTime operator()(arg_type datetime) const noexcept
        {
            kx::DateTime dt{ static_cast<::I>(std::floor(datetime)), 0 };
            dt.nanos = static_cast<::J>(
                (datetime - dt.days) * (kx::NANOS_PER_SEC * kx::SECS_PER_DAY));
            return std::move(dt);
        }
    };

    template <>
    struct DateTimeConverter<q::kTimespan>
    {
        using arg_type = typename q::TypeTraits<q::kTimespan>::const_reference;

        kx::DateTime operator()(arg_type timespan) const noexcept
        {
            return kx::DateTime{
                static_cast<::I>(timespan / (kx::NANOS_PER_SEC * kx::SECS_PER_DAY)),
                static_cast<::J>(timespan % (kx::NANOS_PER_SEC * kx::SECS_PER_DAY)),
            };
        }
    };

    template <>
    struct DateTimeConverter<q::kTimestamp>
    {
        using arg_type = typename q::TypeTraits<q::kTimestamp>::const_reference;

        kx::DateTime operator()(arg_type timestamp) const noexcept
        {
            uint64_t nanos = 0;
            auto const time = kx::p2unix(timestamp, &nanos);
            return kx::unix2z(time, nanos);
        }
    };

    template <q::TypeId tid>
    q::tm_ext& q2DateTimeImpl(q::tm_ext& tm, K x) noexcept(false)
    {
        using DatetimeTraits = q::TypeTraits<tid>;
        auto const converter = DateTimeConverter<tid>{};

        auto const v = DatetimeTraits::value(x);
        if (DatetimeTraits::is_inf(v, true) || DatetimeTraits::is_inf(v, false)) {
            throw q::K_error("+/-inf date/time/datetime/timestamp");
        }
        try {
            kx::z2gmtime(converter(v), tm);
        }
        catch (std::runtime_error const& ex) {
            throw q::K_error(ex);
        }
        return tm;
    }

    template <q::TypeId tid>
    std::vector<q::tm_ext>& q2DateTimesImpl(std::vector<q::tm_ext>& tms, K x) noexcept(false)
    {
        using DatetimeTraits = q::TypeTraits<tid>;

        q::K_ptr t(DatetimeTraits::atom(0));
        assert(q::type(t.get()) == -q::type(x));

        assert(q::count(x) >= 0);
        tms.resize(q::count(x));

        auto o = tms.begin();
        for (std::size_t i = 0; i < q::count(x); ++i, ++o) {
            DatetimeTraits::value(t.get()) = DatetimeTraits::index(x)[i];
            q2DateTimeImpl<tid>(*o, t.get());
        }
        return tms;
    }

}//namespace /*anonymous* /

q::tm_ext q::q2DateTime(K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil date/time/datetime/timestamp");
    }
    tm_ext result{};
    switch (type(x)) {
    case -kDate:
        q2DateTimeImpl<kDate>(result, x);
        break;
    case -kTime:
        q2DateTimeImpl<kTime>(result, x);
        break;
    case -kSecond:
        q2DateTimeImpl<kSecond>(result, x);
        break;
    case -kMinute:
        q2DateTimeImpl<kMinute>(result, x);
        break;
    case -kDatetime:
        q2DateTimeImpl<kDatetime>(result, x);
        break;
    case -kTimespan:
        q2DateTimeImpl<kTimespan>(result, x);
        break;
    case -kTimestamp:
        q2DateTimeImpl<kTimestamp>(result, x);
        break;
    default:
        throw K_error("not a date/time/datetime/timestamp");
    }
    return std::move(result);
}

std::vector<q::tm_ext> q::q2DateTimes(K x) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil date/time/datetime/timestamp list");
    }
    else if (type(x) < 0) {
        throw K_error("not a list");
    }
    std::vector<q::tm_ext> result;
    switch (type(x)) {
    case kDate:
        q2DateTimesImpl<kDate>(result, x);
        break;
    case kTime:
        q2DateTimesImpl<kTime>(result, x);
        break;
    case kSecond:
        q2DateTimesImpl<kSecond>(result, x);
        break;
    case kMinute:
        q2DateTimesImpl<kMinute>(result, x);
        break;
    case kDatetime:
        q2DateTimesImpl<kDatetime>(result, x);
        break;
    case kTimespan:
        q2DateTimesImpl<kTimespan>(result, x);
        break;
    case kTimestamp:
        q2DateTimesImpl<kTimestamp>(result, x);
        break;
    default:
        throw K_error("not a date/time/datetime/timestamp list");
    }
    return std::move(result);
}
*/

#pragma endregion
