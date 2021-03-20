#include "ktype_traits.hpp"
#include <regex>
#include "kerror.hpp"

using namespace std::chrono;

#pragma region <kTimestamp> conversions

namespace
{
    ::J compose_timestamp(long long date, long long time) noexcept
    {
        return date * q::time_scale_v<nanoseconds, date::days> + time;
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

        auto const date = q::TypeTraits<q::kDate>::parse(matches.str(1).c_str());
        auto const time = q::TypeTraits<q::kTimespan>::parse(std::stoll(matches.str(2)));
        if (q::TypeTraits<q::kDate>::null() == date || q::TypeTraits<q::kTimespan>::null() == time)
            return Traits::null();
        return compose_timestamp(date, time);
    }

}//namespace /*anonymous*/

q::TypeTraits<q::kTimestamp>::value_type
q::TypeTraits<q::kTimestamp>::encode(long long year, long long month, long long day,
    long long hour, long long minute, long long second, long long nanos)
noexcept
{
    auto const date = TypeTraits<kDate>::encode(
        static_cast<::I>(year), static_cast<::I>(month), static_cast<::I>(day));
    auto const time = TypeTraits<kTimespan>::encode(0, hour, minute, second, nanos);
    return compose_timestamp(date, time);
}

q::TypeTraits<q::kTimestamp>::value_type
q::TypeTraits<q::kTimestamp>::encode(q::TypeTraits<q::kTimestamp>::temporal_type const& p)
noexcept
{
    auto const d = floor<date::days>(p);
    date::year_month_day const ymd{ d };
    auto const hmsf = date::make_time(p - d);
    return encode(int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()),
        hmsf.hours().count(), hmsf.minutes().count(), hmsf.seconds().count(),
        hmsf.subseconds().count());
}

q::TypeTraits<q::kTimestamp>::temporal_type
q::TypeTraits<q::kTimestamp>::decode(q::TypeTraits<q::kTimestamp>::const_reference p)
noexcept
{
    return Timestamp{ q::Date{ Epoch } } + Nanoseconds{ p };
}

q::TypeTraits<q::kTimestamp>::value_type
q::TypeTraits<q::kTimestamp>::parse(char const* ymdhmsf, bool raw)
noexcept
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

    auto const date = TypeTraits<kDate>::parse(matches.str(1).c_str());
    auto const time = 0 < matches.length(2) ? TypeTraits<kTimespan>::parse(matches.str(2).c_str()) : 0;
    if (TypeTraits<kDate>::null() == date || TypeTraits<kTimespan>::null() == time)
        return Traits::null();
    return compose_timestamp(date, time);
}

#pragma endregion

#pragma region <kMonth> conversions

q::TypeTraits<q::kMonth>::value_type
q::TypeTraits<q::kMonth>::encode(int year, int month)
noexcept
{
    return (year - int(Epoch.year())) * time_scale_v<date::months, date::years>
        + (month - 1);
}

q::TypeTraits<q::kMonth>::value_type
q::TypeTraits<q::kMonth>::encode(q::TypeTraits<q::kMonth>::temporal_type const& m)
noexcept
{
    date::year_month_day const ym{ m };
    return encode(int(ym.year()), unsigned(ym.month()));
}

q::TypeTraits<q::kMonth>::temporal_type
q::TypeTraits<q::kMonth>::decode(q::TypeTraits<q::kMonth>::const_reference m)
noexcept
{
    auto yyyy = (m + 1) / time_scale_v<date::months, date::years> + int(Epoch.year());
    auto mm = (m + 1) % time_scale_v<date::months, date::years>;
    if (mm <= 0) {
        mm += time_scale_v<date::months, date::years>;
        yyyy--;
    }
    return Date{ date::year{ yyyy } / mm / 1 };
}

q::TypeTraits<q::kMonth>::value_type
q::TypeTraits<q::kMonth>::parse(int yyyymm)
noexcept
{
    return encode(yyyymm / 100, yyyymm % 100);
}

q::TypeTraits<q::kMonth>::value_type
q::TypeTraits<q::kMonth>::parse(char const* ym)
noexcept
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
    return encode(year, month);
}

#pragma endregion

#pragma region <kDate> conversions

q::TypeTraits<q::kDate>::value_type
q::TypeTraits<q::kDate>::encode(int year, int month, int day)
noexcept
{
    return ::ymd(year, month, day);
}

q::TypeTraits<q::kDate>::value_type
q::TypeTraits<q::kDate>::encode(q::TypeTraits<q::kDate>::temporal_type const& d)
noexcept
{
    ::date::year_month_day const ymd{ d };
    return encode(int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()));
}

q::TypeTraits<q::kDate>::temporal_type
q::TypeTraits<q::kDate>::decode(q::TypeTraits<q::kDate>::const_reference d)
noexcept
{
    auto yyyymmdd = ::dj(d);
    auto const y = yyyymmdd / 100'00;
    yyyymmdd %= 100'00;
    return Date{ date::year{ y } / (yyyymmdd / 100) / (yyyymmdd % 100) };
}

q::TypeTraits<q::kDate>::value_type
q::TypeTraits<q::kDate>::parse(int yyyymmdd)
noexcept
{
    return encode(yyyymmdd / 100'00, yyyymmdd / 100 % 100, yyyymmdd % 100);
}

q::TypeTraits<q::kDate>::value_type
q::TypeTraits<q::kDate>::parse(char const* ymd) noexcept
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
    return encode(year, month, day);
}

#pragma endregion

#pragma region <kDatetime> conversions

namespace
{
    ::F compose_datetime(int date, int time) noexcept
    {
        return date + time / ::F(q::time_scale_v<milliseconds, date::days>);
    }

}//namespace /*anonymous* /

q::TypeTraits<q::kDatetime>::value_type
q::TypeTraits<q::kDatetime>::encode(int year, int month, int day,
    int hour, int minute, int second, int millis)
noexcept
{
    auto const date = TypeTraits<kDate>::encode(year, month, day);
    auto const time = TypeTraits<kTime>::encode(hour, minute, second, millis);
    return compose_datetime(date, time);
}

q::TypeTraits<q::kDatetime>::value_type
q::TypeTraits<q::kDatetime>::encode(q::TypeTraits<q::kDatetime>::temporal_type const& z)
noexcept
{
    auto const d = floor<date::days>(z);
    date::year_month_day const ymd{ d };
    auto const hmsf = date::make_time(z - d);
    return encode(int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()),
        hmsf.hours().count(), hmsf.minutes().count(), int(hmsf.seconds().count()),
        int(hmsf.subseconds().count()));
}

q::TypeTraits<q::kDatetime>::temporal_type
q::TypeTraits<q::kDatetime>::decode(q::TypeTraits<q::kDatetime>::const_reference z)
noexcept
{
    return DateTime{ Date{ Epoch } } + milliseconds{
        (long long)(std::round(z * time_scale_v<milliseconds, date::days>)) };
}

q::TypeTraits<q::kDatetime>::value_type
q::TypeTraits<q::kDatetime>::parse(long long yyyymmddhhmmssf3)
noexcept
{
    auto const date = TypeTraits<kDate>::parse(static_cast<::I>(yyyymmddhhmmssf3 / 100'00'00'000LL));
    auto const time = TypeTraits<kTime>::parse(static_cast<::I>(yyyymmddhhmmssf3 % 100'00'00'000LL));
    return compose_datetime(date, time);
}

q::TypeTraits<q::kDatetime>::value_type
q::TypeTraits<q::kDatetime>::parse(char const* ymdhmsf)
noexcept
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

    auto const date = TypeTraits<kDate>::parse(matches.str(1).c_str());
    auto const time = 0 < matches.length(2) ? TypeTraits<kTime>::parse(matches.str(2).c_str()) : 0;
    if (TypeTraits<kDate>::null() == date || TypeTraits<kTime>::null() == time)
        return Traits::null();
    return compose_datetime(date, time);
}

#pragma endregion

#pragma region <kTimespan> conversions

q::TypeTraits<q::kTimespan>::value_type
q::TypeTraits<q::kTimespan>::encode(long long day,
    long long hour, long long minute, long long second, long long nanos)
noexcept
{
    return (((day * time_scale_v<hours, ::date::days> + hour
                ) * time_scale_v<minutes, hours> + minute
            ) * time_scale_v<seconds, minutes> + second
        ) * time_scale_v<nanoseconds, seconds> + nanos;
}

q::TypeTraits<q::kTimespan>::value_type
q::TypeTraits<q::kTimespan>::encode(q::TypeTraits<q::kTimespan>::temporal_type const& n)
noexcept
{
    date::hh_mm_ss<nanoseconds> const hmsf{ n };
    return (hmsf.is_negative() ? -1 : 1)
        * encode(0, hmsf.hours().count(), hmsf.minutes().count(),
            hmsf.seconds().count(), hmsf.subseconds().count());
}

q::TypeTraits<q::kTimespan>::temporal_type
q::TypeTraits<q::kTimespan>::decode(q::TypeTraits<q::kTimespan>::const_reference n)
noexcept
{
    return Nanoseconds{ n };
}

q::TypeTraits<q::kTimespan>::value_type
q::TypeTraits<q::kTimespan>::parse(long long hhmmssf9)
noexcept
{
    auto const nanos = std::abs(hhmmssf9) % 1000'000'000uLL;
    auto const hhmmss = std::abs(hhmmssf9) / 1000'000'000uLL;
    return std_ext::signum(hhmmssf9)
        * encode(0, hhmmss / 100'00, hhmmss / 100 % 100, hhmmss % 100, nanos);
}

q::TypeTraits<q::kTimespan>::value_type
q::TypeTraits<q::kTimespan>::parse(char const* dhmsf)
noexcept
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
    return sign * encode(day, hour, minute, second, nanos);
}

#pragma endregion

#pragma region <kMinute> conversions

q::TypeTraits<q::kMinute>::value_type
q::TypeTraits<q::kMinute>::encode(int hour, int minute)
noexcept
{
    return hour * time_scale_v<minutes, hours> + minute;
}

q::TypeTraits<q::kMinute>::value_type
q::TypeTraits<q::kMinute>::encode(q::TypeTraits<q::kMinute>::temporal_type const& u)
noexcept
{
    date::hh_mm_ss<Seconds> hms{ u };
    return (hms.is_negative() ? -1 : 1)
        * encode(hms.hours().count(), hms.minutes().count());
}

q::TypeTraits<q::kMinute>::temporal_type
q::TypeTraits<q::kMinute>::decode(q::TypeTraits<q::kMinute>::const_reference u)
noexcept
{
    return std::chrono::duration_cast<Seconds>(std::chrono::minutes{ u });
}

q::TypeTraits<q::kMinute>::value_type
q::TypeTraits<q::kMinute>::parse(int hhmm)
noexcept
{
    return encode(hhmm / 100, hhmm % 100);
}

q::TypeTraits<q::kMinute>::value_type
q::TypeTraits<q::kMinute>::parse(char const* hm)
noexcept
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
    return sign * encode(hour, minute);
}

#pragma endregion

#pragma region <kSecond> conversions

q::TypeTraits<q::kSecond>::value_type
q::TypeTraits<q::kSecond>::encode(int hour, int minute, int second)
noexcept
{
    return (hour * time_scale_v<minutes, hours> + minute
        ) * time_scale_v<seconds, minutes> + second;
}

q::TypeTraits<q::kSecond>::value_type
q::TypeTraits<q::kSecond>::encode(q::TypeTraits<q::kSecond>::temporal_type const& v)
noexcept
{
    date::hh_mm_ss<Seconds> const hms{ v };
    return (hms.is_negative() ? -1 : 1)
        * encode(hms.hours().count(), hms.minutes().count(), int(hms.seconds().count()));
}

q::TypeTraits<q::kSecond>::temporal_type
q::TypeTraits<q::kSecond>::decode(q::TypeTraits<q::kSecond>::const_reference v)
noexcept
{
    return Seconds{ v };
}

q::TypeTraits<q::kSecond>::value_type
q::TypeTraits<q::kSecond>::parse(int hhmmss)
noexcept
{
    return encode(hhmmss / 100'00, hhmmss / 100 % 100, hhmmss % 100);
}

q::TypeTraits<q::kSecond>::value_type
q::TypeTraits<q::kSecond>::parse(char const* hms)
noexcept
{
    using Traits = TypeTraits<kSecond>;

    static std::regex const pattern{ R"(^(-?)(\d+):(\d\d?)(?::(\d\d?))?v?$)" };
    static constexpr auto PATTERN_CAPS = 1 + 1 + 1 + 1;

    if (nullptr == hms)
        return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(hms, matches, pattern))
        return Traits::null();
    assert(1 + PATTERN_CAPS == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const hour = std::stoi(matches.str(2));
    auto const minute = std::stoi(matches.str(3));
    auto const second = 0 < matches.length(4) ? std::stoi(matches.str(4)) : 0;
    return sign * encode(hour, minute, second);
}

#pragma endregion

#pragma region <kTime> conversions

q::TypeTraits<q::kTime>::value_type
q::TypeTraits<q::kTime>::encode(int hour, int minute, int second, int millis)
noexcept
{
    auto const t = ((hour * time_scale_v<minutes, hours> + minute
            ) * time_scale_v<seconds, minutes> + second
        ) * time_scale_v<milliseconds, seconds> + millis;
    assert(std::numeric_limits<::I>::min() <= t && t <= std::numeric_limits<::I>::max());
    return static_cast<::I>(t);
}

q::TypeTraits<q::kTime>::value_type
q::TypeTraits<q::kTime>::encode(q::TypeTraits<q::kTime>::temporal_type const& t)
noexcept
{
    date::hh_mm_ss<Milliseconds> hmsf{ t };
    return (hmsf.is_negative() ? -1 : 1)
        * encode(hmsf.hours().count(), hmsf.minutes().count(),
            int(hmsf.seconds().count()), int(hmsf.subseconds().count()));
}

q::TypeTraits<q::kTime>::temporal_type
q::TypeTraits<q::kTime>::decode(q::TypeTraits<q::kTime>::const_reference t)
noexcept
{
    return Milliseconds{ t };
}

q::TypeTraits<q::kTime>::value_type
q::TypeTraits<q::kTime>::parse(int hhmmssf3)
noexcept
{
    auto const millis = hhmmssf3 % 1000;
    auto const hhmmss = hhmmssf3 / 1000;
    return encode(hhmmss / 100'00, hhmmss / 100 % 100, hhmmss % 100, millis);
}

q::TypeTraits<q::kTime>::value_type
q::TypeTraits<q::kTime>::parse(char const* hmsf)
noexcept
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
    return sign * encode(hour, minute, second, millis);
}

#pragma endregion
