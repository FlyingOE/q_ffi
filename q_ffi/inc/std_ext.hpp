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

namespace misc
{
    /// @brief Alias a pointer.
    ///
    /// NOTE: Beware of the strict aliasing rule in modern C/C++ implementations!
    ///
    /// Users of this template is expected to be aware of and to take care of the
    /// potential memory layout difference between From and To types.
    template<typename To, typename From,
        typename = std::enable_if_t<
            std::is_pointer_v<To> && std::is_pointer_v<From> && sizeof(To) <= sizeof(From)
        >>
    inline constexpr To ptr_alias(From p) noexcept
    {
        union Aliaser
        {
            From fptr;
            To tptr;
        };
        Aliaser alias{};
        alias.fptr = p;
        return alias.tptr;
    }

}//namespace misc
