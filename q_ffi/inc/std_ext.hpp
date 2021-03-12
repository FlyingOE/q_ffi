#pragma once

#include <type_traits>

/// @ref https://stackoverflow.com/questions/1903954/is-there-a-standard-sign-function-signum-sgn-in-c-c
namespace std_ext
{
    template<typename Num>
    inline constexpr int signum(Num x, std::false_type) noexcept
    {
        return Num(x) < x;
    }

    template<typename Num>
    inline constexpr int signum(Num x, std::true_type) noexcept
    {
        return (Num(0) < x) - (x < Num(0));
    }

    template<typename Num>
    inline constexpr int signum(Num const& x) noexcept
    {
        return signum(x, std::is_signed<Num>());
    }

}//namespace std_ext
