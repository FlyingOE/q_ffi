#pragma once

#include <cassert>
#include <limits>
#include "std_ext.hpp"
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

    namespace impl {

        /// @brief Helper to detect if <code>Tr::value(K)</code> is available
        template<typename T>
        using value_signature = decltype(T::value(std::declval<K>()));

        template<typename T>
        using can_value = std_ext::can_apply<value_signature, T>;

        /// @brief Helper to detect if <code>Tr::null()</code> is available
        template<typename T>
        using null_sig_t = decltype(std::declval<T>().null(std::declval<void>()));
        template<typename T>
        using can_null = std_ext::can_apply<null_sig_t, T>;

        /// @brief Helper to detect if <code>Tr::inf()</code> is available
        template<typename T>
        using inf_signature = decltype(T::inf());

        template<typename T>
        using can_inf = std_ext::can_apply<inf_signature, T>;

        /// @remark Use CRTP to provide common traits
        template<typename Value, q::Type id, char ch, typename Tr>
        struct TypeBase
        {
            using value_type = Value; 
            constexpr static Type const id = id;
            constexpr static char const ch = ch;
        };

        template<typename Traits>
        using has_value_t = decltype(Traits::value(std::declval<K>()));

        template<typename Traits>
        using has_null_t = decltype(Traits::null());

        template<typename Traits>
        using has_inf_t = decltype(Traits::inf());

    }//namespace q::impl

    /// @brief check if q type @c tid has <code>value(K)</code>
    template<Type tid>
    using has_value = std_ext::can_apply<impl::has_value_t, TypeTraits<tid>>;

    template<Type tid>
    constexpr bool has_value_t = has_value<tid>::value;

    /// @brief check if q type @c tid has <code>null()</code>
    template<Type tid>
    using has_null = std_ext::can_apply<impl::has_null_t, TypeTraits<tid>>;

    template<Type tid>
    constexpr bool has_null_t = has_null<tid>::value;

    /// @brief check if a @c Traits is of a numeric type (thus, has <code>inf()</code>)
    template<Type tid>
    using is_numeric = std_ext::can_apply<impl::has_inf_t, TypeTraits<tid>>;

    template<Type tid>
    constexpr bool is_numeric_t = is_numeric<tid>::value;

    template<>
    struct TypeTraits<kBoolean>
        : public impl::TypeBase<bool, kBoolean, 'b', TypeTraits<kBoolean>>
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
        : public impl::TypeBase<char, kByte, 'x', TypeTraits<kByte>>
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
        : public impl::TypeBase<short, kShort, 'h', TypeTraits<kShort>>
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
        : public impl::TypeBase<int32_t, kInt, 'i', TypeTraits<kInt>>
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
        : public impl::TypeBase<int64_t, kLong, 'j', TypeTraits<kLong>>
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
        : public impl::TypeBase<float, kReal, 'e', TypeTraits<kReal>>
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
        : public impl::TypeBase<double, kFloat, 'f', TypeTraits<kFloat>>
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
        : public impl::TypeBase<char, kChar, 'c', TypeTraits<kChar>>
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
        : public impl::TypeBase<char const*, kSymbol, 's', TypeTraits<kSymbol>>
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
        : public impl::TypeBase<void, kNil, ' ', TypeTraits<kNil>>
    {
        constexpr static K atom() noexcept
        { return nullptr; }
    };

    template<>
    struct TypeTraits<kError>
        : public impl::TypeBase<char const*, kError, ' ', TypeTraits<kError>>
    {
        static_assert(sizeof(S) == sizeof(value_type), "sizeof(S) == sizeof(<q::kError>)");

        constexpr static value_type value(K k) noexcept
        { return k->s; }

        /// @return Always <code>q::Nil</code> while errors will be signaled to kdb+ host
        static K atom(value_type msg, bool sys = false) noexcept
        { return (sys ? orr : krr)(const_cast<S>(msg)); }
    };

    constexpr K const Nil = TypeTraits<kNil>::atom();

    inline K error(char const* msg, bool sys = false) noexcept
    {
        return TypeTraits<kError>::atom(msg, sys);
    }

}//namespace q