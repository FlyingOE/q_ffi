#pragma once

#include <cstdint>
#include <limits>
#include <iostream>
#include "k_compat.h"

namespace q {

    /// @brief All q data type IDs.
    enum Type : H
    {
        kMixed = 0,
        kBoolean = -KB,
        kGUID = -UU,
        kByte = -KG,
        kShort = -KH,
        kInt = -KI,
        kLong = -KJ,
        kReal = -KE,
        kFloat = -KF,
        kChar = -KC,
        kSymbol = -KS,
        kEnumMin = -20,
        kEnumMax = -76,
        kTimestamp = -KP,
        kMonth = -KM,
        kDate = -KD,
        kDatetime = -KZ,
        kTimespan = -KN,
        kMinute = -KU,
        kSecond = -KV,
        kTime = -KT,
        kTable = XT,
        kDict = XD,
        kNil = 101,
        kError = -128
    };

    /// @brief q data type traits and mapping logic to C++ types.
    ///     All traits must contain the following:
    ///     <dl>
    ///     <dt>@c value_type
    ///     <dd>C++ type corresponding to @c tid
    ///     <dt>@c id
    ///     <dd>Same as @c tid
    ///     <dt>@c ch
    ///     <dd>q type character corresponding to @c tid
    ///     <dt>@c has_null
    ///     <dd>True if the q type has a null value
    ///     <dt>@c is_numeric
    ///     <dd>True if the q type is a numeric, thus has infinity values
    ///     </dl>
    ///     Many traits may contain the following:
    ///     <dl>
    ///     <dt><code>value_type null()</code>
    ///     <dd>The special null value for the q type, if available
    ///     <dt><code>value_type inf()</code>
    ///     <dd>The special infinity value for the q type, if available
    ///     <dt><code>value_type* value(K)</code>
    ///     <dd>Get C++ value from a @c K pointer
    ///     <dt><code>value_type* index(K)</code>
    ///     <dd>Get C++ array pointer from a @c K pointer
    ///     <dt><code>K atom(value_type)</code>
    ///     <dd>Create allocate a @c K atom for a C++ atom
    ///     </dl>
    /// @tparam tid Result of q @c type function
    template<Type tid>
    struct TypeTraits;

}//namespace q

#include "pointer.hpp"

namespace q {

    constexpr Type typeOf(K k) noexcept
    { return nullptr == k ? kNil : (Type)k->t; }

    constexpr Type typeOf(K_ptr const& pk) noexcept
    { return typeOf(pk.get()); }

    namespace {

        /// @remark Use CRTP to provide common traits
        template<typename Value, q::Type id, char ch, typename Tr>
        struct TypeBase
        {
            using value_type = Value; 
            constexpr static Type const id = id;
            constexpr static char const ch = ch;

            /// @brief Helper to detect if <code>Tr::null()</code> is valid
            template<typename T, typename = void>
            struct null_detector : std::false_type {};

            template<typename T>
            struct null_detector<T, std::void_t<
                decltype(T::null())
            >> : std::true_type {};

            /// @brief Helper to detect if <code>Tr::inf()</code> is valid
            template<typename T, typename = void>
            struct numeric_detector : std::false_type {};

            template<typename T>
            struct numeric_detector<T, std::void_t<
                decltype(T::inf())
            >> : std::true_type {};

            constexpr static bool const has_null = null_detector<Tr>::value;
            constexpr static bool const is_numeric = numeric_detector<Tr>::value;
        };

    }//namespace q::<anonymous>

    template<>
    struct TypeTraits<kBoolean>
        : public TypeBase<bool, kBoolean, 'b', TypeTraits<kBoolean>>
    {
        static_assert(sizeof(G) == sizeof(value_type), "sizeof(G) == sizeof(<q::kBoolean>)");

        constexpr static value_type value(K k) noexcept
        { return (value_type)k->g; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kG(k)); }

        static K atom(value_type b) noexcept
        { return kb(b); }
    };

    template<>
    struct TypeTraits<kByte>
        : public TypeBase<char, kByte, 'x', TypeTraits<kByte>>
    {
        static_assert(sizeof(G) == sizeof(value_type), "sizeof(G) == sizeof(<q::kByte>)");

        constexpr static value_type null() noexcept
        { return 0x00; }

        constexpr static value_type value(K k) noexcept
        { return k->g; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kG(k)); }

        static K atom(value_type b) noexcept
        { return kg(b); }
    };

    template<>
    struct TypeTraits<kShort>
        : public TypeBase<short, kShort, 'h', TypeTraits<kShort>>
    {
        static_assert(sizeof(H) == sizeof(value_type), "sizeof(H) == sizeof(<q::kShort>)");

        constexpr static value_type null() noexcept
        { return nh; }

        constexpr static value_type inf() noexcept
        { return wh; }

        constexpr static value_type value(K k) noexcept
        { return k->h; }

        constexpr static value_type* index(K k) noexcept
        { return kH(k); }

        static K atom(value_type h) noexcept
        { return kh(h); }
    };

    template<>
    struct TypeTraits<kInt>
        : public TypeBase<int32_t, kInt, 'i', TypeTraits<kInt>>
    {
        static_assert(sizeof(I) == sizeof(value_type), "sizeof(I) == sizeof(<q::kInt>)");

        constexpr static value_type null() noexcept
        { return ni; }

        constexpr static value_type inf() noexcept
        { return wi; }

        constexpr static value_type value(K k) noexcept
        { return k->i; }

        constexpr static value_type* index(K k) noexcept
        { return kI(k); }

        static K atom(value_type i) noexcept
        { return ki(i); }
    };

    template<>
    struct TypeTraits<kLong>
        : public TypeBase<int64_t, kLong, 'j', TypeTraits<kLong>>
    {
        static_assert(sizeof(J) == sizeof(value_type), "sizeof(J) == sizeof(<q::kLong>)");

        constexpr static value_type null() noexcept
        { return nj; }

        constexpr static value_type inf() noexcept
        { return wj; }

        constexpr static value_type value(K k) noexcept
        { return k->j; }

        constexpr static value_type* index(K k) noexcept
        { return kJ(k); }

        static K atom(value_type j) noexcept
        { return kj(j); }
    };

    template<>
    struct TypeTraits<kReal>
        : public TypeBase<float, kReal, 'e', TypeTraits<kReal>>
    {
        static_assert(std::numeric_limits<float>::is_iec559, "<q::kReal> is IEC 559/IEEE 754-compliant");
        static_assert(sizeof(E) == sizeof(value_type), "sizeof(E) == sizeof(<q::kReal>)");

        static value_type null() noexcept
        { static value_type const e = (value_type)nf; return e; }

        static value_type inf() noexcept
        { static value_type const e = (value_type)wf; return e; }

        constexpr static value_type value(K k) noexcept
        { return k->e; }

        constexpr static value_type* index(K k) noexcept
        { return kE(k); }

        static K atom(value_type e) noexcept
        { return ke(e); }
    };

    template<>
    struct TypeTraits<kFloat>
        : public TypeBase<double, kFloat, 'f', TypeTraits<kFloat>>
    {
        static_assert(std::numeric_limits<double>::is_iec559, "<q::kFloat> is IEC 559/IEEE 754-compliant");
        static_assert(sizeof(F) == sizeof(value_type), "sizeof(F) == sizeof(<q::kFloat>)");

        static value_type null() noexcept
        { static value_type const f = nf; return f; }

        static value_type inf() noexcept
        { static value_type const f = wf; return f; }

        constexpr static value_type value(K k) noexcept
        { return k->f; }

        constexpr static value_type* index(K k) noexcept
        { return kF(k); }

        static K atom(value_type f) noexcept
        { return kf(f); }
    };

    template<>
    struct TypeTraits<kChar>
        : public TypeBase<char, kChar, 'c', TypeTraits<kChar>>
    {
        static_assert(sizeof(C) == sizeof(value_type), "sizeof(C) == sizeof(<q::kChar>)");

        constexpr static value_type null() noexcept
        { return ' '; }

        constexpr static value_type value(K k) noexcept
        { return k->g; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kG(k)); }

        static K atom(value_type c) noexcept
        { return kc(c); }
    };

    template<>
    struct TypeTraits<kSymbol>
        : public TypeBase<char const*, kSymbol, 's', TypeTraits<kSymbol>>
    {
        static_assert(sizeof(S) == sizeof(value_type), "sizeof(S) == sizeof(<q::kSymbol>)");

        constexpr static value_type null() noexcept
        { return ""; }

        constexpr static value_type value(K k) noexcept
        { return k->s; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kS(k)); }

        static K atom(value_type s) noexcept
        { return ks(const_cast<S>(s)); }
    };

    template<>
    struct TypeTraits<kNil>
        : public TypeBase<void, kNil, ' ', TypeTraits<kNil>>
    {
        constexpr static K atom() noexcept
        { return nullptr; }
    };

    template<>
    struct TypeTraits<kError>
        : public TypeBase<char const*, kError, ' ', TypeTraits<kError>>
    {
        static_assert(sizeof(S) == sizeof(value_type), "sizeof(S) == sizeof(<q::kError>)");

        constexpr static value_type value(K k) noexcept
        { return k->s; }

        static K atom(value_type msg, bool sys = false) noexcept
        { return (sys ? orr : krr)(const_cast<S>(msg)); }
    };

    constexpr K const Nil = TypeTraits<kNil>::atom();

}//namespace q