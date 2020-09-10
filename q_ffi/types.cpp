#include "types.hpp"
#include <regex>

::K q::error(char const* msg, bool sys) noexcept
{
    return TypeTraits<kError>::atom(msg, sys);
}

::I q::parse_month(int year, int month)
{
    return (year - 2000) * 12 + (month - 1);
}

::I q::parse_month(char const* ym)
{
    using traits = typename TypeTraits<kMonth>;
    static std::regex pattern{R"(^(\d{4})[.\-/]?(\d{2})m?$)"};

    if (nullptr == ym) return traits::null();
    std::cmatch matches;
    if (!std::regex_match(ym, matches, pattern)) return traits::null();
    assert(1+2 == matches.size());

    int const year = std::strtol(matches[1].str().c_str(), nullptr, 10);
    int const month = std::strtol(matches[2].str().c_str(), nullptr, 10);
    return parse_month(year, month);
}

::I q::decode_month(::I m)
{
    int month = (m + 1) % 12;
    int year = (m + 1) / 12 + 2000;
    if (month <= 0) {
        month += 12;
        year--;
    }
    return year * 100 + month;
}

::I q::parse_date(char const* ymd)
{
    using traits = typename TypeTraits<kDate>;
    static std::regex pattern{R"(^(\d{4})([.\-/]?)(\d{2})\2(\d{2})$)"};

    if (nullptr == ymd) return traits::null();
    std::cmatch matches;
    if (!std::regex_match(ymd, matches, pattern)) return traits::null();
    assert(1+4 == matches.size());

    int const year = std::strtol(matches[1].str().c_str(), nullptr, 10);
    int const month = std::strtol(matches[3].str().c_str(), nullptr, 10);
    int const day = std::strtol(matches[4].str().c_str(), nullptr, 10);
    return traits::value(year, month, day);
}
