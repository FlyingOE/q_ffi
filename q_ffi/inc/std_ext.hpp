#pragma once

#include <limits>

namespace std
{
    // Features from C++17
    namespace ext
    {
        template <typename...>
        using void_t = void;

        template<typename T, unsigned N = 0>
        constexpr std::size_t extent_v = extent<T, N>::value;

        template<typename Base, typename Derived>
        constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

        template<typename From, typename To>
        constexpr bool is_convertible_v = is_convertible<From, To>::value;

        template<typename T1, typename T2>
        constexpr bool is_same_v = is_same<T1, T2>::value;

    }//inline namespace std::ext

}//namespace std

#ifdef _MSC_VER
#   include <type_traits>   // MSVC always includes some C++17 library features
#else
namespace std { using namespace ext; }
#endif

namespace std_ext
{
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
