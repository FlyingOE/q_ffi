#pragma once

#include <cassert>
#include <algorithm>
#include <limits>
#include <type_traits>
#include <iterator>
#include <sstream>
#include <iomanip>
#include "k_compat.h"
#include "q_ffi.h"
#include "std_ext.hpp"

namespace q {

    /// @brief All q data type IDs.
    enum Type : ::H
    {
        kMixed = 0,
        kBoolean = (KB),
        kGUID = (UU),
        kByte = (KG),
        kShort = (KH),
        kInt = (KI),
        kLong = (KJ),
        kReal = (KE),
        kFloat = (KF),
        kChar = (KC),
        kSymbol = (KS),
        kEnumMin = 20,
        kEnumMax = 76,
        kTimestamp = (KP),
        kMonth = (KM),
        kDate = (KD),
        kDatetime = (KZ),
        kTimespan = (KN),
        kMinute = (KU),
        kSecond = (KV),
        kTime = (KT),
        kTable = (XT),
        kDict = (XD),
        kNil = 101,
        kError = -128
    };

    inline Type type_of(::K const k) noexcept
    {
        return nullptr == k ? kNil : static_cast<Type>(k->t);
    }

    /// @brief q data type traits and mapping logic to C++ types.
    ///     All traits must contain the following:
    ///     <dl>
    ///     <dt>@c value_type
    ///         <dd>C++ type corresponding to @c tid
    ///     <dt>@c type_id
    ///         <dd>Same as @c tid
    ///     <dt>@c type_code
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
    ///     <dt>above functions with slightly different signatures,
    ///             catering to the specific of the respective q types
    ///     </dl>
    /// @tparam tid Result of q @c type function
    template<Type tid>
    struct TypeTraits;

}//namespace q

#include "pointer.hpp"

namespace q {

    namespace impl {

        /// @brief Common type traits logic for <code>q::TypeTraits<id></code>
        template<typename Value, Type tid, char code>
        struct TypeBase
        {
            using value_type = Value; 
            constexpr static Type const type_id = tid;
            constexpr static char const type_code = code;
            using base_traits = TypeBase<value_type, type_id, type_code>;

            template<typename It,
                typename = std::enable_if_t<
                    std::is_same_v<typename std::iterator_traits<It>::value_type, value_type>
                >>
            static K_ptr list(It begin, It end) noexcept
            {
                ptrdiff_t n = std::distance(begin, end);
                assert(0 <= n && n <= std::numeric_limits<::J>::max());
                K_ptr k{ ::ktn(type_id, n) };
//                std::copy(begin, end, index(k.get()));
                return k;
            }

#       pragma region q::TypeTraits<>::to_str(...)

            template<typename T,
                typename = std::enable_if_t<!std::is_void_v<value_type>>>
            static std::string to_str(T&& value)
            {
                return to_str(std::forward<T>(value), is_numeric<type_id>());
            }

        private:

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
                if (value == TypeTraits<type_id>::null()) {
                    buffer << "0N";
                }
                else if (value == TypeTraits<type_id>::inf()) {
                    buffer << "0W";
                }
                else if (value == -TypeTraits<type_id>::inf()) {
                    buffer << "-0W";
                }
                else {
                    buffer << std::to_string(value);
                }
                buffer << type_code;
                return buffer.str();
            }

#       pragma endregion
        };

#       pragma region Type traits signatures to be detected

        template<typename Traits>
        using value_sig = decltype(Traits::value(std::declval<::K>()));

        template<typename Traits>
        using null_sig = decltype(Traits::null());

        template<typename Traits>
        using inf_sig = decltype(Traits::inf());

#       pragma endregion

    }//namespace q::impl

    q_ffi_API
    K_ptr error(char const* msg, bool sys = false) noexcept;

    constexpr K const Nil = static_cast<K>(nullptr);

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
        static_assert(sizeof(::G) == sizeof(value_type),
            "sizeof(G) == sizeof(<q::kBoolean>)");

        inline static value_type value(::K k) noexcept { return static_cast<value_type>(k->g); }
        inline static value_type* index(::K k) noexcept { return (value_type*)(kG(k)); }
        inline static K_ptr atom(value_type b) noexcept { return K_ptr{ ::kb(b) }; } 

        template<typename T,
            typename = std::enable_if_t<std::is_same_v<std::decay_t<T>, value_type>>>
        static std::string to_str(T&& v)
        {
            std::ostringstream buffer;
            buffer << v << type_code;
            return buffer.str();
        }
    };

    template<>
    struct TypeTraits<kByte>
        : public impl::TypeBase<uint8_t, kByte, 'x'>
    {
        static_assert(sizeof(::G) == sizeof(value_type),
            "sizeof(G) == sizeof(<q::kByte>)");

        constexpr static value_type null() noexcept { return 0x00; }
        inline static value_type value(::K k) noexcept { return k->g; }
        inline static value_type* index(::K k) noexcept { return static_cast<value_type*>(kG(k)); }
        inline static K_ptr atom(value_type b) noexcept { return K_ptr{ ::kg(b) }; }

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
        static_assert(sizeof(::H) == sizeof(value_type),
            "sizeof(H) == sizeof(<q::kShort>)");

        constexpr static value_type null() noexcept { return (nh); }
        constexpr static value_type inf() noexcept { return (wh); }
        inline static value_type value(::K k) noexcept { return k->h; }
        inline static value_type* index(::K k) noexcept { return (kH(k)); }
        inline static K_ptr atom(value_type h) noexcept { return K_ptr{ ::kh(h) }; }

    };

    template<>
    struct TypeTraits<kInt>
        : public impl::TypeBase<int32_t, kInt, 'i'>
    {
        static_assert(sizeof(::I) == sizeof(value_type),
            "sizeof(I) == sizeof(<q::kInt>)");

        constexpr static value_type null() noexcept { return (ni); }
        constexpr static value_type inf() noexcept { return (wi); }
        inline static value_type value(::K k) noexcept { return k->i; }
        inline static value_type* index(::K k) noexcept { return (kI(k)); }
        inline static K_ptr atom(value_type i) noexcept { return K_ptr{ ::ki(i) }; }
    };

    template<>
    struct TypeTraits<kLong>
        : public impl::TypeBase<int64_t, kLong, 'j'>
    {
        static_assert(sizeof(::J) == sizeof(value_type),
            "sizeof(J) == sizeof(<q::kLong>)");

        constexpr static value_type null() noexcept { return (nj); }
        constexpr static value_type inf() noexcept { return (wj); }
        inline static value_type value(::K k) noexcept { return k->j; }
        inline static value_type* index(K k) noexcept { return (kJ(k)); }
        inline static K_ptr atom(value_type j) noexcept { return K_ptr{ ::kj(j) }; }
    };

    template<>
    struct TypeTraits<kReal>
        : public impl::TypeBase<float, kReal, 'e'>
    {
        static_assert(std::numeric_limits<float>::is_iec559,
            "<q::kReal> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(::E) == sizeof(value_type),
            "sizeof(E) == sizeof(<q::kReal>)");

        inline static value_type null() noexcept { return static_cast<value_type>(nf); }
        inline static value_type inf() noexcept { return static_cast<value_type>(wf); }
        inline static value_type value(::K k) noexcept { return k->e; }
        inline static value_type* index(::K k) noexcept { return (kE(k)); }

        static K_ptr atom(value_type e) noexcept
        { return K_ptr{ ke(e) }; }
    };

    template<>
    template<typename T>
    static std::string
    impl::TypeBase<float, kReal, 'e'>::to_str(T&& value, std::true_type)
    {
        std::ostringstream buffer;
        value_type const null = TypeTraits<type_id>::null();
        if (0 == std::memcmp(&value, &null, sizeof(value_type))) {
            buffer << "0N";
        }
        else if (value == TypeTraits<type_id>::inf()) {
            buffer << "0W";
        }
        else if (value == -TypeTraits<type_id>::inf()) {
            buffer << "-0W";
        }
        else {
            buffer << std::to_string(value);
        }
        buffer << type_code;
        return buffer.str();
    }

    template<>
    struct TypeTraits<kFloat>
        : public impl::TypeBase<double, kFloat, 'f'>
    {
        static_assert(std::numeric_limits<double>::is_iec559,
            "<q::kFloat> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(::F) == sizeof(value_type),
            "sizeof(F) == sizeof(<q::kFloat>)");

        inline static value_type null() noexcept { return (nf); }
        inline static value_type inf() noexcept { return (wf); }
        inline static value_type value(::K k) noexcept { return k->f; }
        inline static value_type* index(::K k) noexcept { return (kF(k)); }
        inline static K_ptr atom(value_type f) noexcept { return K_ptr{ ::kf(f) }; }
    };

    template<>
    template<typename T>
    static std::string
    impl::TypeBase<double, kFloat, 'f'>::to_str(T&& value, std::true_type)
    {
        std::ostringstream buffer;
        value_type const null = TypeTraits<type_id>::null();
        if (0 == std::memcmp(&value, &null, sizeof(value_type))) {
            buffer << "0n";
        }
        else if (value == TypeTraits<type_id>::inf()) {
            buffer << "0w";
        }
        else if (value == -TypeTraits<type_id>::inf()) {
            buffer << "-0w";
        }
        else {
            buffer << std::to_string(value);
        }
        buffer << type_code;
        return buffer.str();
    }

    template<>
    struct TypeTraits<kChar>
        : public impl::TypeBase<char, kChar, 'c'>
    {
        static_assert(sizeof(::C) == sizeof(value_type),
            "sizeof(C) == sizeof(<q::kChar>)");

        constexpr static value_type null() noexcept { return ' '; }
        inline static value_type value(K k) noexcept { return k->g; }
        inline static value_type* index(K k) noexcept { return (value_type*)(kG(k)); }
        inline static K_ptr atom(value_type c) noexcept { return K_ptr{ ::kc(c) }; }

        using base_traits::list;

        inline static K_ptr list(char const* str) noexcept
        {
            return K_ptr{ ::kp(const_cast<::S>(str)) };
        }

        inline static K_ptr list(char const* str, size_t len) noexcept
        {
            return K_ptr{ ::kpn(const_cast<::S>(str), len) };
        }
    };

    template<>
    struct TypeTraits<kSymbol>
        : public impl::TypeBase<char const*, kSymbol, 's'>
    {
        static_assert(sizeof(::S) == sizeof(value_type),
            "sizeof(S) == sizeof(<q::kSymbol>)");

        using Base = impl::TypeBase<char, kChar, 'c'>;

        constexpr static value_type null() noexcept { return ""; }
        inline static value_type value(::K k) noexcept { return k->s; }
        inline static value_type* index(::K k) noexcept { return (value_type*)(kS(k)); }
        inline static K_ptr atom(value_type s) noexcept { return K_ptr{ ::ks(const_cast<::S>(s)) }; }

        using base_traits::list;

        template<typename It>
        static K_ptr list(It begin, It end) noexcept
        {
            ptrdiff_t n = std::distance(begin, end);
            assert(0 <= n && n <= std::numeric_limits<::J>::max());
            K_ptr k{ ::ktn(type_id, n) };
            std::transform(begin, end, index(k.get()),
                [](auto const& sym) { return intern_str(sym); });
            return k;
        }
    
    private:
        inline static auto intern_str(value_type const& sym) noexcept { return ss(const_cast<::S>(sym)); }
        inline static auto intern_str(std::string const& sym) noexcept { return ss(const_cast<::S>(sym.c_str())); }
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
        inline static K_ptr atom() noexcept { return K_ptr{ Nil }; }
    };

    template<>
    struct TypeTraits<kError>
        : public impl::TypeBase<char const*, kError, ' '>
    {
        static_assert(sizeof(::S) == sizeof(value_type),
            "sizeof(S) == sizeof(<q::kError>)");

        inline static value_type value(::K k) noexcept { return k->s; }

        /// @return Always <code>q::Nil</code> while errors will be signaled to kdb+ host
        inline static K_ptr atom(value_type msg, bool sys = false) noexcept
        {
            return K_ptr{ (sys ? ::orr : ::krr)(const_cast<::S>(msg)) };
        }
    };

}//namespace q