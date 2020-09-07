#pragma once

#include <cassert>
#include <limits>
#include <type_traits>
#include <iterator>
#include <sstream>
#include <iomanip>
#include "std_ext.hpp"
#include "k_compat.h"

namespace q {

    /// @brief All q data type IDs.
    enum Type : H
    {
        kMixed = 0,
        kBoolean = KB,
        kGUID = UU,
        kByte = KG,
        kShort = KH,
        kInt = KI,
        kLong = KJ,
        kReal = KE,
        kFloat = KF,
        kChar = KC,
        kSymbol = KS,
        kEnumMin = 20,
        kEnumMax = 76,
        kTimestamp = KP,
        kMonth = KM,
        kDate = KD,
        kDatetime = KZ,
        kTimespan = KN,
        kMinute = KU,
        kSecond = KV,
        kTime = KT,
        kTable = XT,
        kDict = XD,
        kNil = 101,
        kError = -128
    };

    /// @brief q data type traits and mapping logic to C++ types.
    ///     All traits must contain the following:
    ///     <dl>
    ///     <dt>@c value_type
    ///         <dd>C++ type corresponding to @c tid
    ///     <dt>@c id
    ///         <dd>Same as @c tid
    ///     <dt>@c ch
    ///         <dd>q type character corresponding to @c tid
    ///     </dl>
    ///     Many traits may contain the following:
    ///     <dl>
    ///     <dt><code>value_type null()</code>
    ///         <dd>The special null value for the q type, if available
    ///     <dt><code>value_type inf()</code>
    ///         <dd>The special infinity value for the q type, if available
    ///     <dt><code>value_type value(K)</code>
    ///         <dd>Get C++ value from a @c K pointer
    ///     <dt><code>value_type* index(K)</code>
    ///         <dd>Get C++ array pointer from a @c K pointer
    ///     <dt><code>q::K_ptr atom(value_type)</code>
    ///         <dd>Allocate a @c K atom for a C++ atom
    ///     <dt><code>q::K_ptr list(begin, end)</code>
    ///         <dd>Allocate a @c K list for a C++ range
    ///     <dt><code>std::string to_str(value_type)</code>
    ///         <dd>Convert to string</dd>
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

        /// @brief Common type traits logic for <code>q::TypeTraits<id></code>
        template<typename Value, q::Type id, char ch>
        struct TypeBase
        {
            using value_type = Value; 

            constexpr static Type const id = id;
            constexpr static char const ch = ch;

#       pragma region q::TypeTraits<>::list(...)

            template<typename It,
                typename = std::enable_if_t<
                    std::is_same_v<typename std::iterator_traits<It>::value_type, value_type>
                >>
            static K_ptr list(It begin, It end)
            {
                ptrdiff_t n = std::distance(begin, end);
                assert(0 <= n && n <= std::numeric_limits<J>::max());
                K_ptr k{ ktn(id, n) };
                std::copy(begin, end, TypeTraits<id>::index(k.get()));
                return k;
            }

#       pragma endregion

#       pragma region q::TypeTraits<>::to_str(...)

            template<typename T,
                typename = std::enable_if_t<!std::is_void_v<value_type>>>
            static std::string to_str(T&& value)
            {
                return to_str(std::forward<T>(value), is_numeric<id>());
            }

            template<typename T>
            static std::string to_str(T&& value, std::false_type)
            {
                std::ostringstream buffer;
                buffer << value;
                return buffer.str();
            }

            template<typename T>
            static std::string to_str(T&& value, std::true_type)
            {
                std::ostringstream buffer;
                if (value == TypeTraits<id>::null()) {
                    buffer << "0N";
                }
                else if (value == TypeTraits<id>::inf()) {
                    buffer << "0W";
                }
                else if (value == -TypeTraits<id>::inf()) {
                    buffer << "-0W";
                }
                else {
                    buffer << std::to_string(value);
                }
                buffer << ch;
                return buffer.str();
            }

        private:

#       pragma endregion

        };

#       pragma region Type traits signatures to be detected

        template<typename Traits>
        using value_sig = decltype(Traits::value(std::declval<K>()));

        template<typename Traits>
        using null_sig = decltype(Traits::null());

        template<typename Traits>
        using inf_sig = decltype(Traits::inf());

#       pragma endregion

    }//namespace q::impl

#   pragma region Type traits detection

    /// @brief check if q type @c tid has <code>value(K)</code>
    template<Type tid>
    using has_value = std_ext::can_apply<impl::value_sig, TypeTraits<tid>>;

    template<Type tid>
    constexpr bool has_value_v = has_value<tid>::value;

    /// @brief check if q type @c tid has <code>null()</code>
    template<Type tid>
    using has_null = std_ext::can_apply<impl::null_sig, TypeTraits<tid>>;

    template<Type tid>
    constexpr bool has_null_v = has_null<tid>::value;

    /// @brief check if a @c Traits is of a numeric type (thus, has <code>inf()</code>)
    template<Type tid>
    using is_numeric = std_ext::can_apply<impl::inf_sig, TypeTraits<tid>>;

    template<Type tid>
    constexpr bool is_numeric_v = is_numeric<tid>::value;

#   pragma endregion

    template<>
    struct TypeTraits<kBoolean>
        : public impl::TypeBase<bool, kBoolean, 'b'>
    {
        static_assert(sizeof(G) == sizeof(value_type),
            "sizeof(G) == sizeof(<q::kBoolean>)");

        constexpr static value_type value(K k) noexcept
        { return (value_type)k->g; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kG(k)); }

        static K_ptr atom(value_type b) noexcept
        { return K_ptr{ kb(b) }; } 

        template<typename T,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, value_type>>>
        static std::string to_str(T&& v)
        {
            std::ostringstream buffer;
            buffer << v << ch;
            return buffer.str();
        }
    };

    template<>
    struct TypeTraits<kByte>
        : public impl::TypeBase<uint8_t, kByte, 'x'>
    {
        static_assert(sizeof(G) == sizeof(value_type),
            "sizeof(G) == sizeof(<q::kByte>)");

        constexpr static value_type null() noexcept
        { return 0x00; }

        constexpr static value_type value(K k) noexcept
        { return k->g; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kG(k)); }

        static K_ptr atom(value_type b) noexcept
        { return K_ptr{ kg(b) }; }

        template<typename T,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, value_type>>>
        static std::string to_str(T&& v)
        {
            std::ostringstream buffer;
            buffer << std::setw(2) << std::setfill('0') << std::hex << (int)v;
            return buffer.str();
        }
    };

    template<>
    struct TypeTraits<kShort>
        : public impl::TypeBase<int16_t, kShort, 'h'>
    {
        static_assert(sizeof(H) == sizeof(value_type),
            "sizeof(H) == sizeof(<q::kShort>)");

        constexpr static value_type null() noexcept
        { return nh; }

        constexpr static value_type inf() noexcept
        { return wh; }

        constexpr static value_type value(K k) noexcept
        { return k->h; }

        constexpr static value_type* index(K k) noexcept
        { return kH(k); }

        static K_ptr atom(value_type h) noexcept
        { return K_ptr{ kh(h) }; }
    };

    template<>
    struct TypeTraits<kInt>
        : public impl::TypeBase<int32_t, kInt, 'i'>
    {
        static_assert(sizeof(I) == sizeof(value_type),
            "sizeof(I) == sizeof(<q::kInt>)");

        constexpr static value_type null() noexcept
        { return ni; }

        constexpr static value_type inf() noexcept
        { return wi; }

        constexpr static value_type value(K k) noexcept
        { return k->i; }

        constexpr static value_type* index(K k) noexcept
        { return kI(k); }

        static K_ptr atom(value_type i) noexcept
        { return K_ptr{ ki(i) }; }
    };

    template<>
    struct TypeTraits<kLong>
        : public impl::TypeBase<int64_t, kLong, 'j'>
    {
        static_assert(sizeof(J) == sizeof(value_type),
            "sizeof(J) == sizeof(<q::kLong>)");

        constexpr static value_type null() noexcept
        { return nj; }

        constexpr static value_type inf() noexcept
        { return wj; }

        constexpr static value_type value(K k) noexcept
        { return k->j; }

        constexpr static value_type* index(K k) noexcept
        { return kJ(k); }

        static K_ptr atom(value_type j) noexcept
        { return K_ptr{ kj(j) }; }
    };

    template<>
    struct TypeTraits<kReal>
        : public impl::TypeBase<float, kReal, 'e'>
    {
        static_assert(std::numeric_limits<float>::is_iec559,
            "<q::kReal> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(E) == sizeof(value_type),
            "sizeof(E) == sizeof(<q::kReal>)");

        static value_type null() noexcept
        { static value_type const e = (value_type)nf; return e; }

        static value_type inf() noexcept
        { static value_type const e = (value_type)wf; return e; }

        constexpr static value_type value(K k) noexcept
        { return k->e; }

        constexpr static value_type* index(K k) noexcept
        { return kE(k); }

        static K_ptr atom(value_type e) noexcept
        { return K_ptr{ ke(e) }; }
    };

    template<>
    template<typename T>
    static std::string
    impl::TypeBase<float, kReal, 'e'>::to_str(T&& value, std::true_type)
    {
        std::ostringstream buffer;
        auto const bit_match = [](auto a, auto b) -> bool
        {
            return 0 == std::memcmp(&a, &b, sizeof(value_type));
        };

        if (bit_match(value, TypeTraits<id>::null())) {
            buffer << "0N";
        }
        else if (value == TypeTraits<id>::inf()) {
            buffer << "0W";
        }
        else if (value == -TypeTraits<id>::inf()) {
            buffer << "-0W";
        }
        else {
            buffer << std::to_string(value);
        }
        buffer << ch;
        return buffer.str();
    }

    template<>
    struct TypeTraits<kFloat>
        : public impl::TypeBase<double, kFloat, 'f'>
    {
        static_assert(std::numeric_limits<double>::is_iec559,
            "<q::kFloat> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(F) == sizeof(value_type),
            "sizeof(F) == sizeof(<q::kFloat>)");

        static value_type null() noexcept
        { static value_type const f = nf; return f; }

        static value_type inf() noexcept
        { static value_type const f = wf; return f; }

        constexpr static value_type value(K k) noexcept
        { return k->f; }

        constexpr static value_type* index(K k) noexcept
        { return kF(k); }

        static K_ptr atom(value_type f) noexcept
        { return K_ptr{ kf(f) }; }
    };

    template<>
    template<typename T>
    static std::string
    impl::TypeBase<double, kFloat, 'f'>::to_str(T&& value, std::true_type)
    {
        std::ostringstream buffer;
        auto const bit_match = [](auto a, auto b) -> bool
        {
            return 0 == std::memcmp(&a, &b, sizeof(value_type));
        };

        if (bit_match(value, TypeTraits<id>::null())) {
            buffer << "0n";
        }
        else if (value == TypeTraits<id>::inf()) {
            buffer << "0w";
        }
        else if (value == -TypeTraits<id>::inf()) {
            buffer << "-0w";
        }
        else {
            buffer << std::to_string(value);
        }
        buffer << ch;
        return buffer.str();
    }

    template<>
    struct TypeTraits<kChar>
        : public impl::TypeBase<char, kChar, 'c'>
    {
        static_assert(sizeof(C) == sizeof(value_type),
            "sizeof(C) == sizeof(<q::kChar>)");

        constexpr static value_type null() noexcept
        { return ' '; }

        constexpr static value_type value(K k) noexcept
        { return k->g; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kG(k)); }

        static K_ptr atom(value_type c) noexcept
        { return K_ptr{ kc(c) }; }
    };

    template<>
    struct TypeTraits<kSymbol>
        : public impl::TypeBase<char const*, kSymbol, 's'>
    {
        static_assert(sizeof(S) == sizeof(value_type),
            "sizeof(S) == sizeof(<q::kSymbol>)");

        constexpr static value_type null() noexcept
        { return ""; }

        constexpr static value_type value(K k) noexcept
        { return k->s; }

        constexpr static value_type* index(K k) noexcept
        { return (value_type*)(kS(k)); }

        static K_ptr atom(value_type s) noexcept
        { return K_ptr{ ks(const_cast<S>(s)) }; }
    };

    template<>
    struct TypeTraits<kTimestamp>
        : public TypeTraits<kLong>
    {
        constexpr static Type id = kTimestamp;
        constexpr static char ch = 'p';
    };

    template<>
    struct TypeTraits<kMonth>
        : public TypeTraits<kInt>
    {
        constexpr static Type id = kMonth;
        constexpr static char ch = 'm';
    };

    template<>
    struct TypeTraits<kDate>
        : public TypeTraits<kInt>
    {
        constexpr static Type id = kDate;
        constexpr static char ch = 'd';
    };

    template<>
    struct TypeTraits<kDatetime>
        : public TypeTraits<kFloat>
    {
        constexpr static Type id = kDatetime;
        constexpr static char ch = 'z';
    };

    template<>
    struct TypeTraits<kTimespan>
        : public TypeTraits<kLong>
    {
        constexpr static Type id = kTimespan;
        constexpr static char ch = 'n';
    };

    template<>
    struct TypeTraits<kMinute>
        : public TypeTraits<kInt>
    {
        constexpr static Type id = kMinute;
        constexpr static char ch = 'u';
    };

    template<>
    struct TypeTraits<kSecond>
        : public TypeTraits<kInt>
    {
        constexpr static Type id = kSecond;
        constexpr static char ch = 'v';
    };

    template<>
    struct TypeTraits<kTime>
        : public TypeTraits<kInt>
    {
        constexpr static Type id = kTime;
        constexpr static char ch = 't';
    };

    template<>
    struct TypeTraits<kNil>
        : public impl::TypeBase<void, kNil, ' '>
    {
        static K_ptr atom() noexcept
        { return K_ptr{ nullptr }; }
    };

    template<>
    struct TypeTraits<kError>
        : public impl::TypeBase<char const*, kError, ' '>
    {
        static_assert(sizeof(S) == sizeof(value_type),
            "sizeof(S) == sizeof(<q::kError>)");

        constexpr static value_type value(K k) noexcept
        { return k->s; }

        /// @return Always <code>q::Nil</code> while errors will be signaled to kdb+ host
        static K_ptr atom(value_type msg, bool sys = false) noexcept
        { return K_ptr{ (sys ? orr : krr)(const_cast<S>(msg)) }; }
    };

    inline K_ptr error(char const* msg, bool sys = false) noexcept
    {
        return TypeTraits<kError>::atom(msg, sys);
    }

    constexpr K const Nil = static_cast<K>(nullptr);

}//namespace q