#pragma once

#include <cstdint>
#include <limits>
#include "k_compat.h"

namespace q
{
    /// @brief All q data type IDs.
    enum class Type : ::H
    {
        Boolean = -KB,
        GUID = -UU,
        Byte = -KG,
        Short = -KH,
        Int = -KI,
        Long = -KJ,
        Real = -KE,
        Float = -KF,
        Char = -KC,
        Symbol = -KS,
        EnumMin = -20,
        EnumMax = -76,
        Timestamp = -KP,
        Month = -KM,
        Date = -KD,
        Datetime = -KZ,
        Timespan = -KN,
        Minute = -KU,
        Second = -KV,
        Time = -KT,

        Mixed = 0,
        Table = XT,
        Dict = XD,
        Nil = 101,

        Booleans = -Boolean,
        GUIDs = -GUID,
        Bytes = -Byte,
        Shorts = -Short,
        Ints = -Int,
        Longs = -Long,
        Reals = -Real,
        Floats = -Float,
        Chars = -Char,
        Symbols = -Symbol,
        Timestamps = -Timestamp,
        Months = -Month,
        Dates = -Date,
        Datetimes = -Datetime,
        Timespans = -Timespan,
        Minutes = -Minute,
        Seconds = -Second,
        Times = -Time,

        Error = -128
    };

    /// @brief q data type traits and mapping logic to C++ types.
    ///     All traits must contain the following:
    ///     <dl>
    ///     <dt>@c value_type
    ///     <dd>C++ type corresponding to @c tid
    ///     <dt>@c id
    ///     <dd>Same as @c tid
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

    namespace impl {

        template<q::Type id, typename Value>
        struct TypeBase
        {
            using value_type = Value; 
            static Type const id = id;
        };

    }//namespace q::impl

    template<>
    struct TypeTraits<Type::Boolean>
        : public impl::TypeBase<Type::Boolean, bool>
    {
        static_assert(sizeof(::G) == sizeof(value_type), "sizeof(G) == sizeof(<Type::Boolean>)");
        constexpr static value_type value(K k) { return k->g; }
        constexpr static value_type* index(K k) { return (value_type*)kG(k); }
        static K atom(value_type b) { return kb(b); }
    };

    template<>
    struct TypeTraits<Type::Byte>
        : public impl::TypeBase<Type::Byte, char>
    {
        static_assert(sizeof(::G) == sizeof(value_type), "sizeof(G) == sizeof(<Type::Byte>)");
        constexpr static value_type null() { return '\0'; }
        constexpr static value_type value(K k) { return k->g; }
        constexpr static value_type* index(K k) { return (value_type*)kG(k); }
        static K atom(value_type b) { return kg(b); }
    };

    template<>
    struct TypeTraits<Type::Short>
        : public impl::TypeBase<Type::Short, short>
    {
        static_assert(sizeof(::H) == sizeof(value_type), "sizeof(H) == sizeof(<Type::Short>)");
        constexpr static value_type null() { return nh; }
        constexpr static value_type inf() { return wh; }
        constexpr static value_type value(K k) { return k->h; }
        constexpr static value_type* index(K k) { return kH(k); }
        static K atom(value_type h) { return kh(h); }
    };

    template<>
    struct TypeTraits<Type::Int>
        : public impl::TypeBase<Type::Int, int32_t>
    {
        static_assert(sizeof(::I) == sizeof(value_type), "sizeof(I) == sizeof(<Type::Int>)");
        constexpr static value_type null() { return ni; }
        constexpr static value_type inf() { return wi; }
        constexpr static value_type value(K k) { return k->i; }
        constexpr static value_type* index(K k) { return kI(k); }
        static K atom(value_type i) { return ki(i); }
    };

    template<>
    struct TypeTraits<Type::Long>
        : public impl::TypeBase<Type::Long, int64_t>
    {
        static_assert(sizeof(::J) == sizeof(value_type), "sizeof(J) == sizeof(<Type::Long>)");
        constexpr static value_type null() { return nj; }
        constexpr static value_type inf() { return wj; }
        constexpr static value_type value(K k) { return k->j; }
        constexpr static value_type* index(K k) { return kJ(k); }
        static K atom(value_type j) { return kj(j); }
    };

    template<>
    struct TypeTraits<Type::Real>
        : public impl::TypeBase<Type::Real, float>
    {
        static_assert(std::numeric_limits<float>::is_iec559, "<Type::Real> is IEC 559/IEEE 754");
        static_assert(sizeof(::E) == sizeof(value_type), "sizeof(E) == sizeof(<Type::Real>)");
        static value_type null() { static value_type e = static_cast<value_type>(nf); return e; }
        static value_type inf() { static value_type e = static_cast<value_type>(wf); return e; }
        constexpr static value_type value(K k) { return k->e; }
        constexpr static value_type* index(K k) { return kE(k); }
        static K atom(value_type e) { return ke(e); }
    };

    template<>
    struct TypeTraits<Type::Float>
        : public impl::TypeBase<Type::Float, double>
    {
        static_assert(std::numeric_limits<double>::is_iec559, "<Type::Float> is IEC 559/IEEE 754");
        static_assert(sizeof(::F) == sizeof(value_type), "sizeof(F) == sizeof(<Type::Float>)");
        static value_type null() { static value_type f = nf; return f; }
        static value_type inf() { static value_type f = wf; return f; }
        constexpr static value_type value(K k) { return k->f; }
        constexpr static value_type* index(K k) { return kF(k); }
        static K atom(value_type f) { return kf(f); }
    };

    template<>
    struct TypeTraits<Type::Char>
        : public impl::TypeBase<Type::Char, char>
    {
        static_assert(sizeof(::C) == sizeof(value_type), "sizeof(C) == sizeof(<Type::Char>)");
        constexpr static value_type null() { return ' '; }
        constexpr static value_type value(K k) { return k->g; }
        constexpr static value_type* index(K k) { return (value_type*)kG(k); }
        static K atom(value_type c) { return kc(c); }
    };

    template<>
    struct TypeTraits<Type::Symbol>
        : public impl::TypeBase<Type::Symbol, char const*>
    {
        static_assert(sizeof(::S) == sizeof(value_type), "sizeof(S) == sizeof(<Type::Symbol>)");
        constexpr static value_type null() { return ""; }
        constexpr static value_type value(K k) { return k->s; }
        constexpr static value_type* index(K k) { return (value_type*)kS(k); }
        static K atom(value_type s) { return ks(const_cast<S>(s)); }
    };

    template<>
    struct TypeTraits<Type::Nil>
        : public impl::TypeBase<Type::Nil, void>
    {
        static K atom() { return static_cast<K>(nullptr); }
    };

    template<>
    struct TypeTraits<Type::Booleans>
        : public impl::TypeBase<Type::Booleans, bool*>
    {
    };

    template<>
    struct TypeTraits<Type::Error>
        : public impl::TypeBase<Type::Error, char const*>
    {
        static_assert(sizeof(::S) == sizeof(value_type), "sizeof(S) == sizeof(<Type::Symbol>)");
        constexpr static value_type value(K k) { return k->s; }
        static K atom(value_type msg, bool sys = false) { return (sys ? orr : krr)(const_cast<S>(msg)); }
    };

}//namespace q