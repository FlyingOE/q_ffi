#include "ktype_traits.hpp"
#include <regex>
#include "kerror.hpp"

using namespace std::chrono;

#pragma region <kTimestamp> conversions

namespace
{
    ::J compose_timestamp(long long date, long long time) noexcept
    {
        return date * q::ratio<nanoseconds, date::days>() + time;
    }

    ::J parse_raw_timestamp(char const* yyyymmddhhmmssf9) noexcept
    {
        using Traits = q::TypeTraits<q::kTimestamp>;

        static const std::regex separator{ "'" };
        static const std::regex pattern{ R"(^(\d{8})(\d{6}\d{9})$)" };
        static constexpr auto PATTERN_LEN = 8 + 6 + 9;
        static constexpr auto PATTERN_CAPS = 1 + 1;

        if (nullptr == yyyymmddhhmmssf9)
            return Traits::null();
        auto const ymdhmsf = std::regex_replace(yyyymmddhhmmssf9, separator, "");
        if (PATTERN_LEN != ymdhmsf.length())
            return Traits::null();

        std::smatch matches;
        if (!std::regex_match(ymdhmsf, matches, pattern))
            return Traits::null();
        assert(1 + PATTERN_CAPS == matches.size());

        auto const date = q::parse_date(matches.str(1).c_str());
        auto const time = q::parse_timespan(std::stoll(matches.str(2)));
        if (q::TypeTraits<q::kDate>::null() == date || q::TypeTraits<q::kTimespan>::null() == time)
            return Traits::null();
        return compose_timestamp(date, time);
    }

}//namespace /*anonymous*/

::J q::temporal::encode_timestamp(long long year, long long month, long long day,
    long long hour, long long minute, long long second, long long nanos) noexcept
{
    auto const date = encode_date(
        static_cast<::I>(year), static_cast<::I>(month), static_cast<::I>(day));
    auto const time = encode_timespan(0, hour, minute, second, nanos);
    return compose_timestamp(date, time);
}

::J q::temporal::encode_timestamp(q::TimePoint const& t) noexcept
{
    auto const d = floor<date::days>(t);
    date::year_month_day const ymd{ d };
    auto const hmsn = date::make_time(t - d);

    auto const date = encode_date(
        int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()));
    auto const time = encode_timespan(0,
        hmsn.hours().count(), hmsn.minutes().count(), hmsn.seconds().count(),
        hmsn.subseconds().count());
    return compose_timestamp(date, time);
}

::J q::temporal::parse_timestamp(char const* ymdhmsf, bool raw) noexcept
{
    using Traits = TypeTraits<kTimestamp>;

    if (raw)
        return parse_raw_timestamp(ymdhmsf);

    static const std::regex pattern{ R"(^([^D]+)(?:D([^p]+))?p?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1;

    if (nullptr == ymdhmsf)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ymdhmsf, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const date = parse_date(matches.str(1).c_str());
    auto const time = 0 < matches.length(2) ? parse_timespan(matches.str(2).c_str()) : 0;
    if (TypeTraits<kDate>::null() == date || TypeTraits<kTimespan>::null() == time)
        return Traits::null();
    return compose_timestamp(date, time);
}

#pragma endregion

#pragma region <kMonth> conversions

::I q::temporal::encode_month(int year, int month) noexcept
{
    return (year - int(Epoch.year())) * ratio<date::months, date::years>()
        + month - 1;
}

::I q::temporal::parse_month(int yyyymm) noexcept
{
    return encode_month(yyyymm / 100, yyyymm % 100);
}

::I q::temporal::parse_month(char const* ym) noexcept
{
    using Traits = TypeTraits<kMonth>;

    static std::regex const pattern{ R"(^(\d{4})[.\-/](\d\d?)m?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1;

    if (nullptr == ym)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ym, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const year = std::stoi(matches.str(1));
    auto const month = std::stoi(matches.str(2));
    return encode_month(year, month);
}

date::year_month q::temporal::decode_month(::I m) noexcept
{
    auto yyyy = (m + 1) / ratio<date::months, date::years>() + int(Epoch.year());
    auto mm = (m + 1) % ratio<date::months, date::years>();
    if (mm <= 0) {
        mm += ratio<date::months, date::years>();
        yyyy--;
    }
    return date::year{ yyyy } / mm;
}

#pragma endregion

#pragma region <kDate> conversions

::I q::temporal::encode_date(int year, int month, int day) noexcept
{
    return ::ymd(year, month, day);
}

::I q::temporal::parse_date(int yyyymmdd) noexcept
{
    return encode_date(yyyymmdd / 100'00, yyyymmdd / 100 % 100, yyyymmdd % 100);
}

::I q::temporal::parse_date(char const* ymd) noexcept
{
    using Traits = TypeTraits<kDate>;

    static std::regex const pattern{ R"(^(\d{4})([.\-/]?)(\d\d?)\2(\d\d?)d?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1 + 1 + 1;

    if (nullptr == ymd)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ymd, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const year = std::stoi(matches.str(1));
    auto const month = std::stoi(matches.str(3));
    auto const day = std::stoi(matches.str(4));
    return encode_date(year, month, day);
}

date::year_month_day q::temporal::decode_date(::I d) noexcept
{
    auto yyyymmdd = ::dj(d);
    auto const y = yyyymmdd / 100'00;
    yyyymmdd %= 100'00;
    return date::year{ y } / (yyyymmdd / 100) / (yyyymmdd % 100);
}

#pragma endregion

#pragma region <kDatetime> conversions

namespace
{
    ::F compose_datetime(int date, int time) noexcept
    {
        return date + time
            / ::F(q::ratio<std::chrono::milliseconds, date::days>());
    }

}//namespace /*anonymous*/

::F q::temporal::encode_datetime(int year, int month, int day,
    int hour, int minute, int second, int millis) noexcept
{
    ::I const date = encode_date(year, month, day);
    ::I const time = encode_time(hour, minute, second, millis);
    return compose_datetime(date, time);
}

::F q::temporal::encode_datetime(q::TimePoint const& t) noexcept
{
    auto const d = floor<date::days>(t);
    date::year_month_day const ymd{ d };
    auto const hmsf = date::make_time(t - d);

    auto const date = encode_date(
        int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()));
    auto const time = encode_time(
        hmsf.hours().count(), hmsf.minutes().count(), int(hmsf.seconds().count()),
        int(hmsf.subseconds().count() / ratio<nanoseconds, milliseconds>()));
    return compose_datetime(date, time);
}

::F q::temporal::parse_datetime(long long yyyymmddhhmmssf3) noexcept
{
    ::I const date = parse_date(static_cast<::I>(yyyymmddhhmmssf3 / 100'00'00'000LL));
    ::I const time = parse_time(static_cast<::I>(yyyymmddhhmmssf3 % 100'00'00'000LL));
    return compose_datetime(date, time);
}

::F q::temporal::parse_datetime(char const* ymdhmsf) noexcept
{
    using Traits = TypeTraits<kDatetime>;
    static const std::regex pattern{ R"(^([^T]+)(?:T([^z]+))?z?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1;

    if (nullptr == ymdhmsf)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ymdhmsf, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const date = parse_date(matches.str(1).c_str());
    auto const time = 0 < matches.length(2) ? parse_time(matches.str(2).c_str()) : 0;
    if (TypeTraits<kDate>::null() == date || TypeTraits<kTime>::null() == time)
        return Traits::null();
    return compose_datetime(date, time);
}

#pragma endregion

#pragma region <kTimespan> conversions

::J q::temporal::encode_timespan(long long day,
    long long hour, long long minute, long long second, long long nanos) noexcept
{
    return (((day * ratio<hours, date::days>() + hour
                ) * ratio<minutes, hours>() + minute
            ) * ratio<seconds, minutes>() + second
        ) * ratio<nanoseconds, seconds>() + nanos;
}

::J q::temporal::parse_timespan(long long hhmmssf9) noexcept
{
    auto const nanos = hhmmssf9 % 1000'000'000uLL;
    auto const hhmmss = hhmmssf9 / 1000'000'000uLL;
    return encode_timespan(0, hhmmss / 100'00, hhmmss / 100 % 100, hhmmss % 100, nanos);
}

::J q::temporal::parse_timespan(char const* dhmsf) noexcept
{
    using Traits = TypeTraits<kTimespan>;

    static const std::regex
        pattern{ R"(^(-?)(?:(\d+)D)?(\d+)(?::(\d\d?)(?::(\d\d?)(?:\.(\d{1,9}))?)?)?n?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1 + 1 + 1 + 1 + 1;

    if (nullptr == dhmsf)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(dhmsf, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const day = 0 < matches.length(2) ? std::stoll(matches.str(2)) : 0;
    auto const hour = std::stoll(matches.str(3));
    auto const minute = 0 < matches.length(4) ? std::stoll(matches.str(4)) : 0;
    auto const second = 0 < matches.length(5) ? std::stoll(matches.str(5)) : 0;
    auto const nanos = 0 < matches.length(6) ?
        static_cast<long long>(std::stold("0." + matches.str(6)) * 1e9) : 0;
    return sign * encode_timespan(day, hour, minute, second, nanos);
}

#pragma endregion

#pragma region <kMinute> conversions

::I q::temporal::encode_minute(int hour, int minute) noexcept
{
    return hour * ratio<minutes, hours>() + minute;
}

::I q::temporal::parse_minute(int hhmm) noexcept
{
    return encode_minute(hhmm / 100, hhmm % 100);
}

::I q::temporal::parse_minute(char const* hm) noexcept
{
    using Traits = TypeTraits<kMinute>;
    static std::regex const pattern{ R"(^(-?)(\d+):(\d\d?)u?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1 + 1;

    if (nullptr == hm)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(hm, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const hour = std::stoi(matches.str(2));
    auto const minute = std::stoi(matches.str(3));
    return sign * encode_minute(hour, minute);
}

::I q::temporal::decode_minute(::I m) noexcept
{
    auto const sign = std_ext::signum(m);
    m *= sign;
    auto const hour = m / ratio<minutes, hours>();
    auto const minute = m % ratio<minutes, hours>();
    return sign * (hour * 100 + minute);
}

#pragma endregion

#pragma region <kSecond> conversions

::I q::encode_second(int hour, int minute, int second) noexcept
{
    return hour * 3600 + minute * 60 + second;
}

::I q::parse_second(int hhmmss) noexcept
{
    return encode_second(hhmmss / 100'00, hhmmss / 100 % 100, hhmmss % 100);
}

::I q::parse_second(char const* hms) noexcept
{
    using Traits = TypeTraits<kSecond>;
    static std::regex const pattern{ R"(^(-?)(\d+):(\d\d?)(?::(\d\d?))?v?$)" };

    if (nullptr == hms)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(hms, matches, pattern))
        return Traits::null();
    assert(1 + 4 == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const hour = std::stoi(matches.str(2));
    auto const minute = std::stoi(matches.str(3));
    auto const second = 0 < matches.length(4) ? std::stoi(matches.str(4)) : 0;
    return sign * encode_second(hour, minute, second);
}

::I q::decode_second(::I s) noexcept
{
    auto const sign = std_ext::signum(s);
    s *= sign;
    auto const hour = s / 3600;
    auto const minute = s / 60 % 60;
    auto const second = s % 60;
    return sign * (hour * 100'00 + minute * 100 + second);
}

#pragma endregion

#pragma region <kTime> conversions

::I q::temporal::encode_time(int hour, int minute, int second, int millis) noexcept
{
    auto const t = ((hour * ratio<minutes, hours>() + minute
            ) * ratio<seconds, minutes>() + second
        ) * ratio<milliseconds, seconds>() + millis;
    assert(std::numeric_limits<::I>::min() <= t && t <= std::numeric_limits<::I>::max());
    return static_cast<::I>(t);
}

::I q::temporal::parse_time(int hhmmssf3) noexcept
{
    auto const millis = hhmmssf3 % 1000;
    auto const hhmmss = hhmmssf3 / 1000;
    return encode_time(hhmmss / 100'00, hhmmss / 100 % 100, hhmmss % 100, millis);
}

::I q::temporal::parse_time(char const* hmsf) noexcept
{
    using Traits = TypeTraits<kTime>;

    static std::regex const
        pattern{ R"(^(-?)(\d+):(\d\d?)(?::(\d\d?)(?:\.(\d{1,3}))?)?t?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1 + 1 + 1 + 1;

    if (nullptr == hmsf)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(hmsf, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const hour = std::stoi(matches.str(2));
    auto const minute = std::stoi(matches.str(3));
    auto const second = 0 < matches.length(4) ? std::stoi(matches.str(4)) : 0;
    auto const millis = 0 < matches.length(5) ?
        static_cast<int>(std::stof("0." + matches.str(5)) * 1000) : 0;
    return sign * encode_time(hour, minute, second, millis);
}

#pragma endregion
