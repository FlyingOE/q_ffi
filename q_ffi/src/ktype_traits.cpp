#include "ktype_traits.hpp"
#include <limits>
#include <regex>

#pragma region <kTimestamp> conversions

inline ::J compose_timestamp(long long date, long long time) noexcept
{
    return date * 86400'000'000'000LL + time;
}

::J q::encode_timestamp(long long year, long long month, long long day,
    long long hour, long long minute, long long second, long long nanos) noexcept
{
    auto const date = encode_date(
        static_cast<::I>(year), static_cast<::I>(month), static_cast<::I>(day));
    auto const time = encode_timespan(0, hour, minute, second, nanos);
    return compose_timestamp(date, time);
}

::J parse_raw_timestamp(char const* yyyymmddhhmmssf9) noexcept
{
    using Traits = q::TypeTraits<q::kTimestamp>;
    static std::regex const separator{ "'" };
    static std::regex const pattern{ R"(^(\d{8})(\d{15})$)" };

    if (nullptr == yyyymmddhhmmssf9) return Traits::null();
    std::string const ymdhmsf = std::regex_replace(yyyymmddhhmmssf9, separator, "");
    if (8 + 6 + 9 != ymdhmsf.length()) return Traits::null();

    std::smatch matches;
    if (!std::regex_match(ymdhmsf, matches, pattern)) return Traits::null();
    assert(1 + 2 == matches.size());

    ::I const date = q::parse_date(matches.str(1).c_str());
    ::J const time = q::parse_timespan(std::stoll(matches.str(2)));
    if (q::TypeTraits<q::kDate>::null() == date || q::TypeTraits<q::kTimespan>::null() == time)
        return Traits::null();
    return compose_timestamp(date, time);
}

::J q::parse_timestamp(char const* ymdhmsf, bool raw) noexcept
{
    if (raw) return parse_raw_timestamp(ymdhmsf);

    using Traits = TypeTraits<kTimestamp>;
    static std::regex const pattern{ R"(^([^D]+)(?:D([^p]+))?p?$)" };

    if (nullptr == ymdhmsf) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ymdhmsf, matches, pattern)) return Traits::null();
    assert(1 + 2 == matches.size());

    ::I const date = parse_date(matches.str(1).c_str());
    ::J const time = 0 < matches.length(2) ? parse_timespan(matches.str(2).c_str()) : 0;
    if (TypeTraits<kDate>::null() == date || TypeTraits<kTimespan>::null() == time)
        return Traits::null();
    return compose_timestamp(date, time);
}

#pragma endregion

#pragma region <kMonth> conversions

::I q::encode_month(int year, int month) noexcept
{
    return (year - 2000) * 12 + (month - 1);
}

::I q::parse_month(int yyyymm) noexcept
{
    return encode_month(yyyymm / 100, yyyymm % 100);
}

::I q::parse_month(char const* ym) noexcept
{
    using Traits = TypeTraits<kMonth>;
    static std::regex const pattern{ R"(^(\d{4})[.\-/](\d\d?)m?$)" };

    if (nullptr == ym) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ym, matches, pattern)) return Traits::null();
    assert(1 + 2 == matches.size());

    auto const year = std::stoi(matches.str(1));
    auto const month = std::stoi(matches.str(2));
    return encode_month(year, month);
}

::I q::decode_month(::I m) noexcept
{
    auto year = (m + 1) / 12 + 2000;
    auto month = (m + 1) % 12;
    if (month <= 0) {
        month += 12;
        year--;
    }
    return year * 100 + month;
}

#pragma endregion

#pragma region <kDate> conversions

::I q::encode_date(int year, int month, int day) noexcept
{
    return ::ymd(year, month, day);
}

::I q::parse_date(int yyyymmdd) noexcept
{
    return encode_date(yyyymmdd / 100'00, yyyymmdd / 100 % 100, yyyymmdd % 100);
}

::I q::parse_date(char const* ymd) noexcept
{
    using Traits = TypeTraits<kDate>;
    static std::regex const pattern{ R"(^(\d{4})([.\-/]?)(\d\d?)\2(\d\d?)d?$)" };

    if (nullptr == ymd) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ymd, matches, pattern)) return Traits::null();
    assert(1 + 3 + 1 == matches.size());

    auto const year = std::stoi(matches.str(1));
    auto const month = std::stoi(matches.str(3));
    auto const day = std::stoi(matches.str(4));
    return encode_date(year, month, day);
}

::I q::decode_date(::I d) noexcept
{
    return ::dj(d);
}

#pragma endregion

#pragma region <kDatetime> conversions

inline ::F compose_datetime(int date, int time) noexcept
{
    return date + time / 86400'000.;
}

::F q::encode_datetime(int year, int month, int day,
    int hour, int minute, int second, int millis) noexcept
{
    ::I const date = encode_date(year, month, day);
    ::I const time = encode_time(hour, minute, second, millis);
    return compose_datetime(date, time);
}

::F q::parse_datetime(long long yyyymmddhhmmssf3) noexcept
{
    ::I const date = parse_date(static_cast<::I>(yyyymmddhhmmssf3 / 100'00'00'000LL));
    ::I const time = parse_time(static_cast<::I>(yyyymmddhhmmssf3 % 100'00'00'000LL));
    return compose_datetime(date, time);
}

::F q::parse_datetime(char const* ymdhmsf) noexcept
{
    using Traits = TypeTraits<kDatetime>;
    static std::regex const pattern{ R"(^([^T]+)(?:T([^z]+))?z?$)" };

    if (nullptr == ymdhmsf) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(ymdhmsf, matches, pattern)) return Traits::null();
    assert(1 + 2 == matches.size());

    ::I const date = parse_date(matches.str(1).c_str());
    ::I const time = 0 < matches.length(2) ? parse_time(matches.str(2).c_str()) : 0;
    if (TypeTraits<kDate>::null() == date || TypeTraits<kTime>::null() == time)
        return Traits::null();
    return compose_datetime(date, time);
}

#pragma endregion

#pragma region <kTimespan> conversions

::J q::encode_timespan(long long day,
    long long hour, long long minute, long long second, long long nanos) noexcept
{
    return day * 86400'000'000'000LL
        + hour * 3600'000'000'000LL + minute * 60'000'000'000LL +second * 1000'000'000LL + nanos;
}

::J q::parse_timespan(long long hhmmssf9) noexcept
{
    return encode_timespan(0,
        hhmmssf9 / 100'00'000'000'000LL, hhmmssf9 / 100'000'000'000LL % 100,
        hhmmssf9 / 1000'000'000LL % 100, hhmmssf9 % 1000'000'000LL);
}

::J q::parse_timespan(char const* dhmsf) noexcept
{
    using Traits = TypeTraits<kTimespan>;
    static std::regex const pattern{
        R"(^(-?)(?:(\d+)D)?(\d+)(?::(\d\d?)(?::(\d\d?)(?:\.(\d{1,9}))?)?)?n?$)"
    };

    if (nullptr == dhmsf) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(dhmsf, matches, pattern)) return Traits::null();
    assert(1 + 6 == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const day = 0 < matches.length(2) ? std::stoll(matches.str(2)) : 0;
    auto const hour = std::stoll(matches.str(3));
    auto const minute = 0 < matches.length(4) ? std::stoll(matches.str(4)) : 0;
    auto const second = 0 < matches.length(5) ? std::stoll(matches.str(5)) : 0;
    auto const nanos = 0 < matches.length(6) ?
        static_cast<long long>(std::stold("0." + matches.str(6)) * 1000'000'000LL) : 0;
    return sign * encode_timespan(day, hour, minute, second, nanos);
}

::J q::decode_timespan(::J n) noexcept
{
    auto const sign = std_ext::sgn(n);
    n *= sign;
    auto const hour = n / 3600'000'000'000LL;
    auto const minute = n / 60'000'000'000LL % 60;
    auto const second = n / 1000'000'000LL % 60;
    auto const nanos = n % 1000'000'000LL;
    return sign * (hour * 100'00'000'000'000LL
        + minute * 100'000'000'000LL + second * 1000'000'000LL + nanos);
}

#pragma endregion

#pragma region <kMinute> conversions

::I q::encode_minute(int hour, int minute) noexcept
{
    return hour * 60 + minute;
}

::I q::parse_minute(int hhmm) noexcept
{
    return encode_minute(hhmm / 100, hhmm % 100);
}

::I q::parse_minute(char const* hm) noexcept
{
    using Traits = TypeTraits<kMinute>;
    static std::regex const pattern{ R"(^(-?)(\d+):(\d\d?)u?$)" };

    if (nullptr == hm) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(hm, matches, pattern)) return Traits::null();
    assert(1 + 3 == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const hour = std::stoi(matches.str(2));
    auto const minute = std::stoi(matches.str(3));
    return sign * encode_minute(hour, minute);
}

::I q::decode_minute(::I m) noexcept
{
    auto const sign = std_ext::sgn(m);
    m *= sign;
    auto const hour = m / 60;
    auto const minute = m % 60;
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

    if (nullptr == hms) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(hms, matches, pattern)) return Traits::null();
    assert(1 + 4 == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const hour = std::stoi(matches.str(2));
    auto const minute = std::stoi(matches.str(3));
    auto const second = 0 < matches.length(4) ? std::stoi(matches.str(4)) : 0;
    return sign * encode_second(hour, minute, second);
}

::I q::decode_second(::I s) noexcept
{
    auto const sign = std_ext::sgn(s);
    s *= sign;
    auto const hour = s / 3600;
    auto const minute = s / 60 % 60;
    auto const second = s % 60;
    return sign * (hour * 100'00 + minute * 100 + second);
}

#pragma endregion

#pragma region <kTime> conversions

::I q::encode_time(int hour, int minute, int second, int millis) noexcept
{
    return hour * 3600'000 + minute * 60'000 + second * 1000 + millis;
}

::I q::parse_time(int hhmmssf3) noexcept
{
    return encode_time(
        hhmmssf3 / 100'00'000, hhmmssf3 / 100'000 % 100,
        hhmmssf3 / 1000 % 100, hhmmssf3 % 1000);
}

::I q::parse_time(char const* hmsf) noexcept
{
    using Traits = TypeTraits<kTime>;
    static std::regex const pattern{
        R"(^(-?)(\d+):(\d\d?)(?::(\d\d?)(?:\.(\d{1,3}))?)?t?$)"
    };

    if (nullptr == hmsf) return Traits::null();
    std::cmatch matches;
    if (!std::regex_match(hmsf, matches, pattern)) return Traits::null();
    assert(1 + 5 == matches.size());

    auto const sign = 0 < matches.length(1) ? -1 : 1;
    auto const hour = std::stoi(matches.str(2));
    auto const minute = std::stoi(matches.str(3));
    auto const second = 0 < matches.length(4) ? std::stoi(matches.str(4)) : 0;
    auto const millis = 0 < matches.length(5) ?
        static_cast<int>(std::stof("0." + matches.str(5)) * 1000) : 0;
    return sign * encode_time(hour, minute, second, millis);
}

::I q::decode_time(::I t) noexcept
{
    auto const sign = std_ext::sgn(t);
    t *= sign;
    auto const hour = t / 3600'000;
    auto const minute = t / 60'000 % 60;
    auto const second = t / 1000 % 60;
    auto const millis = t % 1000;
    return sign * (hour * 100'00'000 + minute * 100'000 + second * 1000 + millis);
}

#pragma endregion

#pragma region Type traits constants

constexpr q::TypeId q::TypeTraits<q::kBoolean>::type_id;

constexpr q::TypeId q::TypeTraits<q::kByte>::type_id;
constexpr q::TypeId q::TypeTraits<q::kShort>::type_id;
constexpr q::TypeId q::TypeTraits<q::kInt>::type_id;
constexpr q::TypeId q::TypeTraits<q::kLong>::type_id;

constexpr q::TypeId q::TypeTraits<q::kReal>::type_id;
constexpr q::TypeId q::TypeTraits<q::kFloat>::type_id;

constexpr q::TypeId q::TypeTraits<q::kChar>::type_id;
constexpr q::TypeId q::TypeTraits<q::kSymbol>::type_id;

constexpr q::TypeId q::TypeTraits<q::kTimestamp>::type_id;
constexpr q::TypeId q::TypeTraits<q::kMonth>::type_id;
constexpr q::TypeId q::TypeTraits<q::kDate>::type_id;
constexpr q::TypeId q::TypeTraits<q::kDatetime>::type_id;
constexpr q::TypeId q::TypeTraits<q::kTimespan>::type_id;
constexpr q::TypeId q::TypeTraits<q::kMinute>::type_id;
constexpr q::TypeId q::TypeTraits<q::kSecond>::type_id;
constexpr q::TypeId q::TypeTraits<q::kTime>::type_id;

constexpr q::TypeId q::TypeTraits<q::kNil>::type_id;
constexpr q::TypeId q::TypeTraits<q::kError>::type_id;

#pragma endregion
