#pragma once

#include <type_traits>  // Need std::void_t from C++17

namespace std {

    template <typename...>
    using void_t = void;

    template<typename From, typename To>
    constexpr bool is_convertible_v = is_convertible<From, To>::value;

}//namespace std

namespace std_ext {

#pragma region std_ext::can_apply<...>

    /// @brief Type traits to detect, with SFINAE, if a type can apply call
    /// @ref https://stackoverflow.com/questions/30189926/metaprograming-failure-of-function-definition-defines-a-separate-function/30195655#30195655
    namespace details {

        template<template<typename...> class, typename, typename...>
        struct can_apply : std::false_type {};

        template<template<typename...> class Z_, typename... Ts>
        struct can_apply<Z_, std::void_t<Z_<Ts...>>, Ts...> : std::true_type {};

    }//namespace std_ext::details

    template<template<typename...> class Z_, typename... Ts>
    using can_apply = details::can_apply<Z_, std::void_t<>, Ts...>;

#pragma endregion

    template<typename Num>
    inline auto sgn(Num v) noexcept
    {
        constexpr auto e = std::numeric_limits<Num>::epsilon();
        return -e > v ? -1 : v > e;
    }

}//namespace std_ext
