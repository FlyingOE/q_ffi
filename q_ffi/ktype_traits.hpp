#pragma once

#include <cassert>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <iomanip>
#include "k_compat.h"
#include "q_ffi.h"
#include "std_ext.hpp"
#include "ktypes.hpp"
#include "kpointer.hpp"

namespace q {

    inline namespace temporal
    {
        q_ffi_API ::J encode_timestamp(long long year, long long month, long long day,
            long long hour, long long minute, long long second, long long nanos) noexcept;
        /// @param raw If @c ymdhmsf is a "raw literal" string
        q_ffi_API ::J parse_timestamp(char const* ymdhmsf, bool raw = false) noexcept;

        q_ffi_API ::I encode_month(int year, int month) noexcept;
        q_ffi_API ::I parse_month(int yyyymm) noexcept;
        q_ffi_API ::I parse_month(char const* ym) noexcept;
        q_ffi_API ::I decode_month(::I m) noexcept;

        q_ffi_API ::I encode_date(int year, int month, int day) noexcept;
        q_ffi_API ::I parse_date(int yyyymmdd) noexcept;
        q_ffi_API ::I parse_date(char const* ymd) noexcept;
        q_ffi_API ::I decode_date(::I d) noexcept;

        q_ffi_API ::F encode_datetime(int year, int month, int day,
            int hour, int minute, int second, int millis) noexcept;
        q_ffi_API ::F parse_datetime(long long yyyymmddhhmmssf3) noexcept;
        q_ffi_API ::F parse_datetime(char const* ymdhmsf) noexcept;

        q_ffi_API ::J encode_timespan(long long day,
            long long hour, long long minute, long long second, long long nanos) noexcept;
        q_ffi_API ::J parse_timespan(long long hhmmssf9) noexcept;
        q_ffi_API ::J parse_timespan(char const* dhmsf) noexcept;
        q_ffi_API ::J decode_timespan(::J n) noexcept;

        q_ffi_API ::I encode_minute(int hour, int minute) noexcept;
        q_ffi_API ::I parse_minute(int hhmm) noexcept;
        q_ffi_API ::I parse_minute(char const* hm) noexcept;
        q_ffi_API ::I decode_minute(::I m) noexcept;

        q_ffi_API ::I encode_second(int hour, int minute, int second) noexcept;
        q_ffi_API ::I parse_second(int hhmmss) noexcept;
        q_ffi_API ::I parse_second(char const* hms) noexcept;
        q_ffi_API ::I decode_second(::I s) noexcept;

        q_ffi_API ::I encode_time(int hour, int minute, int second, int millis) noexcept;
        q_ffi_API ::I parse_time(int hhmmssf3) noexcept;
        q_ffi_API ::I parse_time(char const* hmsf) noexcept;
        q_ffi_API ::I decode_time(::I t) noexcept;

    }//inline namespace q::temporal

#pragma region Type trait facets
    inline namespace facets
    {
        template<typename Tr, typename Value>
        struct ValueType
        {
            using value_type = Value;
            using reference = value_type&;
            using const_reference = value_type const&;
            using pointer = value_type*;
            using const_pointer = value_type const*;

            static ::K atom(TypeId tid, const_reference v) noexcept
            {
                K_ptr k{ ::ka(tid > 0 ? -tid : tid) };
                Tr::value(k.get()) = v;
                return k.release();
            }

            static reference value(::K k) noexcept /*to be defined*/;

            static std::string to_str(const_reference v)
            {
                std::ostringstream buffer;
                Tr::print(buffer, v);
                return buffer.str();
            }

            template<typename Elem, typename ElemTr>
            static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
            { out << v; }
        };

        template<typename Tr, typename Value>
        struct NullableType
        {
            using value_type = typename ValueType<Tr, Value>::value_type;
            using reference = typename ValueType<Tr, Value>::reference;
            using const_reference = typename ValueType<Tr, Value>::const_reference;

            static value_type null() noexcept /*to be defined*/;

            static bool is_null(const_reference v) noexcept
            {
                auto const n = Tr::null();
                return 0 == std::memcmp(&v, &n, sizeof(value_type));
            }
        };

        template<typename Tr, typename Value>
        struct IndexableType
        {
            using value_type = typename ValueType<Tr, Value>::value_type;
            using pointer = typename ValueType<Tr, Value>::pointer;
            using const_pointer = typename ValueType<Tr, Value>::const_pointer;

            static pointer index(::K k) noexcept /*to be defined*/;

            static ::K list(std::initializer_list<value_type> const& vs) noexcept
            { return Tr::list(vs.begin(), vs.end()); }

            template<typename It>
            static ::K list(It begin, It end) noexcept
            {
                auto const n = std::distance(begin, end);
                assert(0 <= n && n <= std::numeric_limits<::J>::max());
                K_ptr k{ ::ktn(Tr::type_id, n) };
                std::copy(begin, end, Tr::index(k.get()));
                return k.release();
            }
        };

        template<typename Tr, typename Value>
        struct NumericType
        {
            using value_type = typename NullableType<Tr, Value>::value_type;
            using reference = typename NullableType<Tr, Value>::reference;
            using const_reference = typename NullableType<Tr, Value>::const_reference;

            static value_type inf(bool sign = true) noexcept /*to be defined*/;

            static bool is_inf(const_reference& v, bool sign = true) noexcept
            {
                auto const i = Tr::inf(sign);
                return 0 == std::memcmp(&v, &i, sizeof(value_type));
            }

            template<typename Elem, typename ElemTr>
            static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
            {
                if (!Tr::print_special(out, v))
                    out << std::to_string(v) << TypeCode.at(Tr::type_id);
            }

            template<typename Elem, typename ElemTr>
            static bool print_special(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
            {
                if (Tr::is_null(v)) {
                    out << "0N";
                }
                else if (Tr::is_inf(v)) {
                    out << "0W";
                }
                else if (Tr::is_inf(v, false)) {
                    out << "-0W";
                }
                else {
                    return false;   // not a special value
                }
                out << TypeCode.at(Tr::type_id);
                return true;
            }
        };

    }//inline namespace facets
#pragma endregion

    template<TypeId tid>
    struct TypeTraits;

#pragma region Type traits implementations

    template<>
    struct TypeTraits<kBoolean> : public
        ValueType<TypeTraits<kBoolean>, unsigned char>,
        IndexableType<TypeTraits<kBoolean>, unsigned char>
    {
        static constexpr TypeId type_id = kBoolean;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(sizeof(::G) == sizeof(value_type), "sizeof(G) == sizeof(<kBoolean>)");

        static ::K atom(bool b) noexcept
        { return ::kb(b); }

        static reference value(::K k) noexcept
        { return k->g; }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return static_cast<pointer>(kG(k)); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            out << static_cast<bool>(v) << TypeCode.at(TypeTraits::type_id);
        }
    };

    template<>
    struct TypeTraits<kByte> : public
        ValueType<TypeTraits<kByte>, uint8_t>,
        IndexableType<TypeTraits<kByte>, uint8_t>,
        NullableType<TypeTraits<kByte>, uint8_t>
    {
        static constexpr TypeId type_id = kByte;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(sizeof(::G) == sizeof(value_type), "sizeof(G) == sizeof(<kByte>)");

        static ::K atom(value_type b) noexcept
        { return ::kg(b); }

        static reference value(::K k) noexcept
        { return k->g; }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return (kG(k)); }

        static constexpr value_type null() noexcept
        { return 0x00; }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            out << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(v);
        }
    };

    template<>
    struct TypeTraits<kShort> : public
        ValueType<TypeTraits<kShort>, int16_t>,
        IndexableType<TypeTraits<kShort>, int16_t>,
        NullableType<TypeTraits<kShort>, int16_t>,
        NumericType<TypeTraits<kShort>, int16_t>
    {
        static constexpr TypeId type_id = kShort;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(sizeof(::H) == sizeof(value_type), "sizeof(H) == sizeof(<kShort>)");

        static ::K atom(value_type h) noexcept
        { return ::kh(h); }

        static reference value(::K k) noexcept
        { return k->h; }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return (kH(k)); }

        static constexpr value_type null() noexcept
        { return (nh); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return sign ? (wh) : -(wh); }

        using NumericType::print;
    };

    template<>
    struct TypeTraits<kInt> : public
        ValueType<TypeTraits<kInt>, int32_t>,
        IndexableType<TypeTraits<kInt>, int32_t>,
        NullableType<TypeTraits<kInt>, int32_t>,
        NumericType<TypeTraits<kInt>, int32_t>
    {
        static constexpr TypeId type_id = kInt;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type i) noexcept
        { return ::ki(i); }

        static reference value(::K k) noexcept
        { return k->i; }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return (kI(k)); }

        static constexpr value_type null() noexcept
        { return (ni); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return sign ? (wi) : -(wh); }

        using NumericType::print;
    };

    template<>
    struct TypeTraits<kLong> : public
        ValueType<TypeTraits<kLong>, int64_t>,
        IndexableType<TypeTraits<kLong>, int64_t>,
        NullableType<TypeTraits<kLong>, int64_t>,
        NumericType<TypeTraits<kLong>, int64_t>
    {
        static constexpr TypeId type_id = kLong;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(sizeof(::J) == sizeof(value_type), "sizeof(J) == sizeof(<kLong>)");

        static ::K atom(value_type j) noexcept
        { return ::kj(j); }

        static reference value(::K k) noexcept
        { return (reference)(k->j); }   // int64_t & long long are different in GCC!

        using IndexableType::list;

        static pointer index(K k) noexcept
        { return (pointer)(kJ(k)); }    // int64_t & long long are different in GCC!

        static constexpr value_type null() noexcept
        { return (nj); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return sign ? (wj) : -(wj); }

        using NumericType::print;
    };

    template<>
    struct TypeTraits<kReal> : public
        ValueType<TypeTraits<kReal>, float>,
        IndexableType<TypeTraits<kReal>, float>,
        NullableType<TypeTraits<kReal>, float>,
        NumericType<TypeTraits<kReal>, float>
    {
        static constexpr TypeId type_id = kReal;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(std::numeric_limits<value_type>::is_iec559,
                    "<kReal> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(::E) == sizeof(value_type), "sizeof(E) == sizeof(<kReal>)");

        static ::K atom(value_type e) noexcept
        { return ::ke(e); }

        static reference value(::K k) noexcept
        { return k->e; }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return (kE(k)); }

        static value_type null() noexcept
        { return static_cast<value_type>((nf)); }

        static value_type inf(bool sign = true) noexcept
        { return static_cast<value_type>(sign ? (wf) : -(wf)); }

        using NumericType::print;
    };

    template<>
    struct TypeTraits<kFloat> : public
        ValueType<TypeTraits<kFloat>, double>,
        IndexableType<TypeTraits<kFloat>, double>,
        NullableType<TypeTraits<kFloat>, double>,
        NumericType<TypeTraits<kFloat>, double>
    {
        static constexpr TypeId type_id = kFloat;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(std::numeric_limits<value_type>::is_iec559,
                    "<kFloat> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(::F) == sizeof(value_type), "sizeof(F) == sizeof(<kFloat>)");

        static ::K atom(value_type f) noexcept
        { return ::kf(f); }

        static reference value(::K k) noexcept
        { return k->f; }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return (kF(k)); }

        static value_type null() noexcept
        { return (nf); }

        static value_type inf(bool sign = true) noexcept
        { return sign ? (wf) : -(wf); }

        using NumericType::print;
    };

    template<>
    struct TypeTraits<kChar> : public
        ValueType<TypeTraits<kChar>, char>,
        IndexableType<TypeTraits<kChar>, char>,
        NullableType<TypeTraits<kChar>, char>
    {
        static constexpr TypeId type_id = kChar;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(sizeof(::C) == sizeof(value_type), "sizeof(C) == sizeof(<kChar>)");

        static ::K atom(value_type c) noexcept
        { return ::kc(c); }

        static reference value(::K k) noexcept
        { return (reference)(k->g); }

        using IndexableType::list;

        static ::K list(char const* str) noexcept
        { return ::kp(const_cast<::S>(str)); }

        static ::K list(char const* str, std::size_t len) noexcept
        { return ::kpn(const_cast<::S>(str), len); }

        static ::K list(std::string const& str) noexcept
        { return list(str.c_str(), str.length()); }

        static pointer index(::K k) noexcept
        { return (pointer)(kG(k)); }

        static constexpr value_type null() noexcept
        { return ' '; }

        using ValueType::print;
    };

    template<>
    struct TypeTraits<kSymbol> : public
        ValueType<TypeTraits<kSymbol>, char const*>,
        IndexableType<TypeTraits<kSymbol>, char const*>,
        NullableType<TypeTraits<kSymbol>, char const*>
    {
        static constexpr TypeId type_id = kSymbol;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static_assert(sizeof(::S) == sizeof(value_type), "sizeof(S) == sizeof(<kSymbol>)");
        static_assert(
            sizeof(std::remove_pointer_t<::S>) == sizeof(std::remove_pointer_t<value_type>),
            "sizeof(S::char) == sizeof(<kSymbol>::char)");

        static ::K atom(value_type s) noexcept
        { return ::ks(const_cast<::S>(s)); }

        static reference value(::K k) noexcept
        { return (value_type&)(k->s); }

        using IndexableType::list;

        template<typename It>
        static ::K list(It begin, It end) noexcept
        {
            auto const n = std::distance(begin, end);
            assert(0 <= n && n <= std::numeric_limits<::J>::max());
            K_ptr k{ ::ktn(type_id, n) };
            std::transform(begin, end, index(k.get()),
                [](auto&& sym) {
                    return ::ss(const_cast<::S>(str_getter()(std::forward<decltype(sym)>(sym))));
                });
            return k.release();
        }

        static pointer index(::K k) noexcept
        { return (value_type*)(kS(k)); }

        static constexpr value_type null() noexcept
        { return ""; }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        { out << '`' << v; }

    private:
        struct str_getter
        {
            value_type operator()(char const* s) const
            { return s; }

            value_type operator()(std::string const& s) const
            { return s.c_str(); }
        };
    };

    template<>
    struct TypeTraits<kMonth> : public
        ValueType<TypeTraits<kMonth>, int32_t>,
        IndexableType<TypeTraits<kMonth>, int32_t>,
        NullableType<TypeTraits<kMonth>, int32_t>,
        NumericType<TypeTraits<kMonth>, int32_t>
    {
        static constexpr TypeId type_id = kMonth;
        using BaseTypeTraits = TypeTraits<kInt>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type m) noexcept
        { return ValueType::atom(type_id, m); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static constexpr value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(int year, int month) noexcept
        { return encode_month(year, month); }

        static value_type parse(int yyyymm) noexcept
        { return parse_month(yyyymm); }

        static value_type parse(char const* ym) noexcept
        { return parse_month(ym); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto const yyyymm = decode_month(v);
            out << std::setfill('0')
                << std::setw(4) << (yyyymm / 100) << '.' << std::setw(2) << (yyyymm % 100)
                << TypeCode.at(TypeTraits::type_id);
        }
    };

    template<>
    struct TypeTraits<kDate> : public
        ValueType<TypeTraits<kDate>, int32_t>,
        IndexableType<TypeTraits<kDate>, int32_t>,
        NullableType<TypeTraits<kDate>, int32_t>,
        NumericType<TypeTraits<kDate>, int32_t>
    {
        static constexpr TypeId type_id = kDate;
        using BaseTypeTraits = TypeTraits<kInt>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type d) noexcept
        { return ::kd(d); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static constexpr value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(int year, int month, int day) noexcept
        { return encode_date(year, month, day); }

        static value_type parse(int yyyymmdd) noexcept
        { return parse_date(yyyymmdd); }

        static value_type parse(char const* ymd) noexcept
        { return parse_date(ymd); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto const yyyymmdd = decode_date(v);
            out << std::setfill('0')
                << std::setw(4) << (yyyymmdd / 100'00) << '.'
                << std::setw(2) << (yyyymmdd / 100 % 100) << '.'
                << std::setw(2) << (yyyymmdd % 100);
        }
    };

    template<>
    struct TypeTraits<kTimespan> : public
        ValueType<TypeTraits<kTimespan>, int64_t>,
        IndexableType<TypeTraits<kTimespan>, int64_t>,
        NullableType<TypeTraits<kTimespan>, int64_t>,
        NumericType<TypeTraits<kTimespan>, int64_t>
    {
        static constexpr TypeId type_id = kTimespan;
        using BaseTypeTraits = TypeTraits<kLong>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type n) noexcept
        { return ::ktj(-type_id, n); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static constexpr value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(long long day,
            long long hour, long long minute, long long second, long long nanos) noexcept
        { return encode_timespan(day, hour, minute, second, nanos); }

        static value_type parse(long long hhmmssf9) noexcept
        { return parse_timespan(hhmmssf9); }

        static value_type parse(char const* dhmsf) noexcept
        { return parse_timespan(dhmsf); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v, bool no_day = false)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto dhhmmssf9 = decode_timespan(v);
            auto const sign = std_ext::sgn(dhhmmssf9);
            dhhmmssf9 *= sign;
            out << std::setfill('0') << std::internal
                << (0 <= sign ? "" : "-");
            if (no_day)
                out << std::setw(2) << (dhhmmssf9 / 100'00'000'000'000LL);
            else
                out << (dhhmmssf9 / 24'00'00'000'000'000LL) << 'D'
                    << std::setw(2) << (dhhmmssf9 / 100'00'000'000'000LL % 24);
            out << ':'
                << std::setw(2) << (dhhmmssf9 / 100'000'000'000LL % 100) << ':'
                << std::setw(2) << (dhhmmssf9 / 1000'000'000LL % 100) << '.'
                << std::setw(9) << (dhhmmssf9 % 1000'000'000LL);
        }
    };

    template<>
    struct TypeTraits<kTimestamp> : public
        ValueType<TypeTraits<kTimestamp>, int64_t>,
        IndexableType<TypeTraits<kTimestamp>, int64_t>,
        NullableType<TypeTraits<kTimestamp>, int64_t>,
        NumericType<TypeTraits<kTimestamp>, int64_t>
    {
        static constexpr TypeId type_id = kTimestamp;
        using BaseTypeTraits = TypeTraits<kLong>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type p) noexcept
        { return ::ktj(-type_id, p); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static constexpr value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(long long year, long long month, long long day,
            long long hour, long long minute, long long second, long long nanos) noexcept
        { return encode_timestamp(year, month, day, hour, minute, second, nanos); }

        static value_type parse(char const* ymdhmsf, bool raw = false) noexcept
        { return parse_timestamp(ymdhmsf, raw); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto date = v / 86400'000'000'000LL;
            auto time = v % 86400'000'000'000LL;
            if (v < 0) {
                date--;
                time += 86400'000'000'000LL;
            }
            assert(std::numeric_limits<::I>::min() <= date
                && date <= std::numeric_limits<::I>::max());
            TypeTraits<kDate>::print(out, static_cast<::I>(date));
            out << 'D';
            TypeTraits<kTimespan>::print(out, time, true);
        }
    };

    template<>
    struct TypeTraits<kTime> : public
        ValueType<TypeTraits<kTime>, int32_t>,
        IndexableType<TypeTraits<kTime>, int32_t>,
        NullableType<TypeTraits<kTime>, int32_t>,
        NumericType<TypeTraits<kTime>, int32_t>
    {
        static constexpr TypeId type_id = kTime;
        using BaseTypeTraits = TypeTraits<kInt>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type t) noexcept
        { return ::kt(t); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static constexpr value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(int hour, int minute, int second, int millis) noexcept
        { return encode_time(hour, minute, second, millis); }

        static value_type parse(int hhmmssf3) noexcept
        { return parse_time(hhmmssf3); }

        static value_type parse(char const* hmsf) noexcept
        { return parse_time(hmsf); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto hhmmssf3 = decode_time(v);
            auto const sign = std_ext::sgn(hhmmssf3);
            hhmmssf3 *= sign;
            out << std::setfill('0') << std::internal
                << (0 <= sign ? "" : "-")
                << std::setw(2) << (hhmmssf3 / 100'00'000) << ':'
                << std::setw(2) << (hhmmssf3 / 100'000 % 100) << ':'
                << std::setw(2) << (hhmmssf3 / 1000 % 100) << '.'
                << std::setw(3) << (hhmmssf3 % 1000);
        }
    };

    template<>
    struct TypeTraits<kDatetime> : public
        ValueType<TypeTraits<kDatetime>, double>,
        IndexableType<TypeTraits<kDatetime>, double>,
        NullableType<TypeTraits<kDatetime>, double>,
        NumericType<TypeTraits<kDatetime>, double>
    {
        static constexpr TypeId type_id = kDatetime;
        using BaseTypeTraits = TypeTraits<kFloat>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type z) noexcept
        { return ::kz(z); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(int year, int month, int day,
            int hour, int minute, int second, int millis) noexcept
        { return encode_datetime(year, month, day, hour, minute, second, millis); }

        static value_type parse(long long yyyymmddhhmmssf3) noexcept
        { return parse_datetime(yyyymmddhhmmssf3); }

        static value_type parse(char const* ymdhmsf) noexcept
        { return parse_datetime(ymdhmsf); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto date = static_cast<::I>(v);
            auto time = static_cast<::I>(std::round((v - date) * 86400'000.));
            if (v < 0) {
                date--;
                time += 86400'000;
            }
            TypeTraits<kDate>::print(out, date);
            out << 'T';
            TypeTraits<kTime>::print(out, time);
        }
    };

    template<>
    struct TypeTraits<kMinute> : public
        ValueType<TypeTraits<kMinute>, int32_t>,
        IndexableType<TypeTraits<kMinute>, int32_t>,
        NullableType<TypeTraits<kMinute>, int32_t>,
        NumericType<TypeTraits<kMinute>, int32_t>
    {
        static constexpr TypeId type_id = kMinute;
        using BaseTypeTraits = TypeTraits<kInt>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type m) noexcept
        { return ValueType::atom(type_id, m); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static constexpr value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(int hour, int minute) noexcept
        { return encode_minute(hour, minute); }

        static value_type parse(int hhmm) noexcept
        { return parse_minute(hhmm); }

        static value_type parse(char const* hm) noexcept
        { return parse_minute(hm); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto hhmm = decode_minute(v);
            auto const sign = std_ext::sgn(hhmm);
            hhmm *= sign;
            out << std::setfill('0') << std::internal
                << (0 <= sign ? "" : "-")
                << std::setw(2) << (hhmm / 100) << ':'
                << std::setw(2) << (hhmm % 100);
        }
    };

    template<>
    struct TypeTraits<kSecond> : public
        ValueType<TypeTraits<kSecond>, int32_t>,
        IndexableType<TypeTraits<kSecond>, int32_t>,
        NullableType<TypeTraits<kSecond>, int32_t>,
        NumericType<TypeTraits<kSecond>, int32_t>
    {
        static constexpr TypeId type_id = kSecond;
        using BaseTypeTraits = TypeTraits<kInt>;
        using typename ValueType::value_type;
        using typename ValueType::reference;
        using typename ValueType::const_reference;
        using typename ValueType::pointer;
        using typename ValueType::const_pointer;

        static ::K atom(value_type s) noexcept
        { return ValueType::atom(type_id, s); }

        static reference value(::K k) noexcept
        { return BaseTypeTraits::value(k); }

        using IndexableType::list;

        static pointer index(::K k) noexcept
        { return BaseTypeTraits::index(k); }

        static constexpr value_type null() noexcept
        { return BaseTypeTraits::null(); }

        static constexpr value_type inf(bool sign = true) noexcept
        { return BaseTypeTraits::inf(sign); }

        static value_type encode(int hour, int minute, int second) noexcept
        { return encode_second(hour, minute, second); }

        static value_type parse(int hhmmss) noexcept
        { return parse_second(hhmmss); }

        static value_type parse(char const* hms) noexcept
        { return parse_second(hms); }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, const_reference v)
        {
            if (TypeTraits::print_special(out, v)) return;

            auto hhmmss = decode_second(v);
            auto const sign = std_ext::sgn(hhmmss);
            hhmmss *= sign;
            out << std::setfill('0') << std::internal
                << (0 <= sign ? "" : "-")
                << std::setw(2) << (hhmmss / 100'00) << ':'
                << std::setw(2) << (hhmmss / 100 % 100) << ':'
                << std::setw(2) << (hhmmss % 100);
        }
    };

    template<>
    struct TypeTraits<kNil>
    {
        static constexpr TypeId type_id = kNil;
        using value_type = void;

        static ::K atom(...) noexcept
        { return Nil; }
    };

    template<>
    struct TypeTraits<kError> : public
        ValueType<TypeTraits<kError>, char const*>
    {
        static constexpr TypeId type_id = kError;
        using ValueType::value_type;
        using ValueType::reference;
        using ValueType::const_reference;
        using ValueType::pointer;
        using ValueType::const_pointer;

        static_assert(sizeof(::S) == sizeof(value_type),
            "sizeof(S) == sizeof(<kError>)");

        /// @return Always @c Nil (error will be signaled to kdb+ host)
        static ::K atom(value_type msg, bool sys = false) noexcept
        { return (sys ? ::orr : ::krr)(const_cast<::S>(msg)); }

        static value_type value(::K k) noexcept
        { return k->s; }

        template<typename Elem, typename ElemTr>
        static void print(std::basic_ostream<Elem, ElemTr>& out, value_type const& v)
        {
            out << "<q>'" << v;
        }
    };

#pragma endregion

#pragma region Type traits aspects

    template<TypeId tid>
    struct has_value
        : public std::is_base_of<
            ValueType<TypeTraits<tid>, typename TypeTraits<tid>::value_type>,
            TypeTraits<tid>>
    {};
    template<TypeId tid>
    static constexpr bool has_value_v = has_value<tid>::value;

    template<TypeId tid>
    struct has_null
        : public std::is_base_of<
            NullableType<TypeTraits<tid>, typename TypeTraits<tid>::value_type>,
            TypeTraits<tid>>
    {};
    template<TypeId tid>
    static constexpr bool has_null_v = has_null<tid>::value;

    template<TypeId tid>
    struct can_index
        : public std::is_base_of<
            IndexableType<TypeTraits<tid>, typename TypeTraits<tid>::value_type>,
            TypeTraits<tid>>
    {};
    template<TypeId tid>
    static constexpr bool can_index_v = can_index<tid>::value;

    template<TypeId tid>
    struct is_numeric
        : public std::is_base_of<
            NumericType<TypeTraits<tid>, typename TypeTraits<tid>::value_type>,
            TypeTraits<tid>>
    {};
    template<TypeId tid>
    static constexpr bool is_numeric_v = is_numeric<tid>::value;

#pragma endregion

}//namespace q
