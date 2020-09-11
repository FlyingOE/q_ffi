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

    /// @brief All q data type IDs, same as those returnd by @c type function in q
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
    { return nullptr == k ? q::kNil : static_cast<Type>(k->t); }

    inline size_t count_of(::K const k) noexcept
    { return nullptr == k ? 0 : 0 > type_of(k) ? 1 : static_cast<size_t>(k->n); }

    /// @ref q::kNil
    constexpr ::K Nil = static_cast<::K>(nullptr);

    /// @tparam tid <code>q::Type</code> corresponding to the q type.
    /// @tparam Tr (optional) subclass that is the actual type traits (using CRTP).
    /// @ref q::impl::TypeBase
    template<Type tid, typename Tr = void>
    struct TypeTraits;

    q_ffi_API ::K error(char const* msg, bool sys = false) noexcept;

    q_ffi_API ::J encode_timestamp(long long year, long long month, long long day,
        long long hour, long long minute, long long second, long long nanos) noexcept;
    q_ffi_API ::J parse_timestamp(long long yyyymmddhhmmssf9) noexcept;
    q_ffi_API ::J parse_timestamp(char const* ymdhmsf) noexcept;

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

}//namespace q

#include "pointer.hpp"

namespace q
{
    namespace impl {

        /// @tparam Tr subclass that is the actual type traits.
        /// @remark Type traits use CRTP to provide common logic in base class and
        ///     manage type-specific details in implementation classes.
        template<typename Tr>
        struct TypeBase
        {
            template<typename T,
                typename = std::enable_if_t<
                    can_index_v<Tr::type_id> &&
                    std::is_convertible_v<std::decay_t<T>, typename Tr::value_type>
                >>
            static ::K list(std::initializer_list<T> const& vs) noexcept
            { return Tr::list(std::cbegin(vs), std::cend(vs)); }

            template<typename It,
                typename = std::enable_if_t<
                    can_index_v<Tr::type_id> &&
                    std::is_convertible_v<
                        typename std::iterator_traits<It>::value_type,
                        typename Tr::value_type>
                >>
            static ::K list(It begin, It end) noexcept
            {
                ptrdiff_t n = std::distance(begin, end);
                assert(0 <= n && n <= std::numeric_limits<::J>::max());
                K_ptr k{ ::ktn(Tr::type_id, n) };
                std::copy(begin, end, Tr::index(k.get()));
                return k.release();
            }

            template<typename T,
                typename = std::enable_if_t<
                    has_value_v<Tr::type_id>&&
                    std::is_convertible_v<std::decay_t<T>, typename Tr::value_type>
                >>
            static std::string to_str(T&& v)
            {
                std::ostringstream buffer;
                Tr::print(buffer, static_cast<Tr::value_type>(v), is_numeric<Tr::type_id>());
                return buffer.str();
            }

        protected:

            template<typename Tr2, typename V>
            inline static ::K generic_atom(V&& v) noexcept
            {
                K_ptr k{ ::ka(-Tr2::type_id) };
                Tr2::value(k.get()) = v;
                return k.release();
            }

            template<typename Elem, typename ElemTr, typename T>
            static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::false_type /*is_numeric*/)
            {
                out << std::forward<T>(v);
            }

            template<typename Elem, typename ElemTr, typename T>
            static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
            {
                if (print_special(out, v)) return;

                out << std::to_string(std::forward<T>(v)) << Tr::type_code;
            }

            template<typename Elem, typename ElemTr, typename T>
            static bool print_special(std::basic_ostream<Elem, ElemTr>& out, T&& v)
            {
                static_assert(sizeof(typename Tr::value_type) == sizeof(v),
                    "<BUG> unexpected specialization w/ differing types");

                Tr::value_type const null_v = Tr::null();
                Tr::value_type const inf_v = Tr::inf();
                // Some null values are not comparable (e.g. NaN), use bit comparison
                if (0 == std::memcmp(&null_v, &v, sizeof(typename Tr::value_type))) {
                    out << "0N";
                }
                else if (v == inf_v) {
                    out << "0W";
                }
                else if (v == -inf_v) {
                    out << "-0W";
                }
                else {
                    return false;   // not a special value, after all
                }
                out << Tr::type_code;
                return true;
            }
        };

    }//namespace q::impl

#pragma region Type traits queries

    namespace impl
    {
        template<typename Tr>
        using value_sig = decltype(Tr::value(std::declval<::K>()));

        template<typename Tr>
        using null_sig = decltype(Tr::null());

        template<typename Tr>
        using index_sig = decltype(Tr::index(std::declval<::K>()));

        template<typename Tr>
        using inf_sig = decltype(Tr::inf());

    }//namespace q::impl

    template<Type tid>
    using has_value = std_ext::can_apply<impl::value_sig, TypeTraits<tid>>;
    template<Type tid>
    constexpr static bool has_value_v = has_value<tid>::value;

    template<Type tid>
    using has_null = std_ext::can_apply<impl::null_sig, TypeTraits<tid>>;
    template<Type tid>
    constexpr static bool has_null_v = has_null<tid>::value;

    template<Type tid>
    using can_index = std_ext::can_apply<impl::index_sig, TypeTraits<tid>>;
    template<Type tid>
    constexpr static bool can_index_v = can_index<tid>::value;

    template<Type tid>
    using is_numeric = std_ext::can_apply<impl::inf_sig, TypeTraits<tid>>;
    template<Type tid>
    constexpr static bool is_numeric_v = is_numeric<tid>::value;

#pragma endregion

#pragma region Type traits implementations

    template<>
    struct TypeTraits<kBoolean> final
        : public impl::TypeBase<TypeTraits<kBoolean>>
    {
        using value_type = unsigned char;
        constexpr static Type type_id = kBoolean;
        constexpr static char type_code = 'b';

        static_assert(sizeof(::G) == sizeof(value_type),
            "sizeof(G) == sizeof(<kBoolean>)");

        inline static value_type& value(::K k) noexcept { return k->g; }
        inline static value_type* index(::K k) noexcept { return static_cast<value_type*>(kG(k)); }
        inline static ::K atom(bool b) noexcept { return ::kb(!!b); } 

        template<typename T,
            typename = std::enable_if_t<std::is_convertible_v<std::decay_t<T>, value_type>>>
        static std::string to_str(T&& v)
        {
            std::ostringstream buffer;
            buffer << static_cast<bool>(v) << type_code;
            return buffer.str();
        }
    };

    template<>
    struct TypeTraits<kByte> final
        : public impl::TypeBase<TypeTraits<kByte>>
    {
        using value_type = uint8_t;
        constexpr static Type type_id = kByte;
        constexpr static char type_code = 'x';

        static_assert(sizeof(::G) == sizeof(value_type),
            "sizeof(G) == sizeof(<kByte>)");

        constexpr static value_type null() noexcept { return 0x00; }
        inline static value_type& value(::K k) noexcept { return k->g; }
        inline static value_type* index(::K k) noexcept { return (kG(k)); }
        inline static ::K atom(value_type b) noexcept { return ::kg(b); }

        template<typename T,
            typename = std::enable_if_t<std::is_convertible_v<std::decay_t<T>, value_type>>>
        static std::string to_str(T&& v)
        {
            std::ostringstream buffer;
            buffer << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(v);
            return buffer.str();
        }
    };

    template<>
    struct TypeTraits<kShort> final
        : public impl::TypeBase<TypeTraits<kShort>>
    {
        using value_type = int16_t;
        constexpr static Type type_id = kShort;
        constexpr static char type_code = 'h';

        static_assert(sizeof(::H) == sizeof(value_type),
            "sizeof(H) == sizeof(<kShort>)");

        constexpr static value_type null() noexcept { return (nh); }
        constexpr static value_type inf() noexcept { return (wh); }
        inline static value_type& value(::K k) noexcept { return k->h; }
        inline static value_type* index(::K k) noexcept { return (kH(k)); }
        inline static ::K atom(value_type h) noexcept { return ::kh(h); }
    };

    template<typename Tr>
    struct TypeTraits<kInt, Tr>
        : public impl::TypeBase<std::conditional_t<std::is_void_v<Tr>, TypeTraits<kInt>, Tr>>
    {
    private:
        template<typename T>
        struct TypeDefault  // dummy type
        {
            constexpr static Type type_id{};
            constexpr static char type_code{};
        };

        template<>
        struct TypeDefault<void>
        {
            constexpr static Type type_id = kInt;
            constexpr static char type_code = 'i';
        };

    public:
        using value_type = int32_t;
        constexpr static Type type_id = TypeDefault<Tr>::type_id;
        constexpr static char type_code = TypeDefault<Tr>::type_code;

        static_assert(sizeof(::I) == sizeof(value_type),
            "sizeof(I) == sizeof(<kInt>)");

        constexpr static value_type null() noexcept { return (ni); }
        constexpr static value_type inf() noexcept { return (wi); }
        inline static value_type& value(::K k) noexcept { return k->i; }
        inline static value_type* index(::K k) noexcept { return (kI(k)); }
        inline static ::K atom(value_type i) noexcept { return ::ki(i); }
    };

    template<typename Tr>
    struct TypeTraits<kLong, Tr>
        : public impl::TypeBase<std::conditional_t<std::is_void_v<Tr>, TypeTraits<kLong>, Tr>>
    {
    private:
        template<typename T>
        struct TypeDefault  // dummy type
        {
            constexpr static Type type_id{};
            constexpr static char type_code{};
        };

        template<>
        struct TypeDefault<void>
        {
            constexpr static Type type_id = kLong;
            constexpr static char type_code = 'j';
        };

    public:
        using value_type = int64_t;
        constexpr static Type type_id = TypeDefault<Tr>::type_id;
        constexpr static char type_code = TypeDefault<Tr>::type_code;

        static_assert(sizeof(::J) == sizeof(value_type),
            "sizeof(J) == sizeof(<kLong>)");

        constexpr static value_type null() noexcept { return (nj); }
        constexpr static value_type inf() noexcept { return (wj); }
        inline static value_type& value(::K k) noexcept { return k->j; }
        inline static value_type* index(K k) noexcept { return (kJ(k)); }
        inline static ::K atom(value_type j) noexcept { return ::kj(j); }
    };

    template<>
    struct TypeTraits<kReal> final
        : public impl::TypeBase<TypeTraits<kReal>>
    {
        using value_type = float;
        constexpr static Type type_id = kReal;
        constexpr static char type_code = 'e';

        static_assert(std::numeric_limits<float>::is_iec559,
            "<q::kReal> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(::E) == sizeof(value_type),
            "sizeof(E) == sizeof(<kReal>)");

        inline static value_type null() noexcept { return static_cast<value_type>(nf); }
        inline static value_type inf() noexcept { return static_cast<value_type>(wf); }
        inline static value_type& value(::K k) noexcept { return k->e; }
        inline static value_type* index(::K k) noexcept { return (kE(k)); }
        inline static ::K atom(value_type e) noexcept { return ::ke(e); }
    };

    template<typename Tr>
    struct TypeTraits<kFloat, Tr>
        : public impl::TypeBase<std::conditional_t<std::is_void_v<Tr>, TypeTraits<kFloat>, Tr>>
    {
    private:
        template<typename T>
        struct TypeDefault  // dummy type
        {
            constexpr static Type type_id{};
            constexpr static char type_code{};
        };

        template<>
        struct TypeDefault<void>
        {
            constexpr static Type type_id = kFloat;
            constexpr static char type_code = 'f';
        };

    public:
        using value_type = double;
        constexpr static Type type_id = TypeDefault<Tr>::type_id;
        constexpr static char type_code = TypeDefault<Tr>::type_code;

        static_assert(std::numeric_limits<double>::is_iec559,
            "<q::kFloat> should be IEC 559/IEEE 754-compliant");
        static_assert(sizeof(::F) == sizeof(value_type),
            "sizeof(F) == sizeof(<kFloat>)");

        inline static value_type null() noexcept { return (nf); }
        inline static value_type inf() noexcept { return (wf); }
        inline static value_type& value(::K k) noexcept { return k->f; }
        inline static value_type* index(::K k) noexcept { return (kF(k)); }
        inline static ::K atom(value_type f) noexcept { return ::kf(f); }
    };

    template<>
    struct TypeTraits<kChar> final
        : public impl::TypeBase<TypeTraits<kChar>>
    {
        using value_type = char;
        constexpr static Type type_id = kChar;
        constexpr static char type_code = 'c';

        static_assert(sizeof(::C) == sizeof(value_type),
            "sizeof(C) == sizeof(<kChar>)");

        constexpr static value_type null() noexcept { return ' '; }
        inline static value_type& value(K k) noexcept { return (value_type&)(k->g); }
        inline static value_type* index(K k) noexcept { return (value_type*)(kG(k)); }
        inline static ::K atom(value_type c) noexcept { return ::kc(c); }

        using impl::TypeBase<TypeTraits<kChar>>::list;

        inline static ::K list(char const* str) noexcept
        { return ::kp(const_cast<::S>(str)); }

        inline static ::K list(char const* str, size_t len) noexcept
        { return ::kpn(const_cast<::S>(str), len); }

        inline static ::K list(std::string const& str) noexcept
        { return list(str.c_str(), str.length()); }
    };

    template<>
    struct TypeTraits<kSymbol> final
        : public impl::TypeBase<TypeTraits<kSymbol>>
    {
        using value_type = char const*;
        constexpr static Type type_id = kSymbol;
        constexpr static char type_code = 's';

        static_assert(sizeof(::S) == sizeof(value_type),
            "sizeof(S) == sizeof(<kSymbol>)");
        static_assert(sizeof(std::remove_const_t<std::remove_pointer_t<::S>>)
                == sizeof(std::remove_const_t<std::remove_pointer_t<value_type>>),
            "sizeof(S#char) == sizeof(<kSymbol>#char)");

        constexpr static value_type null() noexcept { return ""; }
        inline static value_type& value(::K k) noexcept { return (value_type&)(k->s); }
        inline static value_type* index(::K k) noexcept { return (value_type*)(kS(k)); }
        inline static ::K atom(value_type s) noexcept { return ::ks(const_cast<::S>(s)); }

        using impl::TypeBase<TypeTraits<kSymbol>>::list;

        template<typename It>
        inline static ::K list(It begin, It end) noexcept
        {
            return make_list(begin, end,
                str_getter<typename std::iterator_traits<It>::value_type>());
        }

    private:
        template<typename StrT>
        struct str_getter;

        template<>
        struct str_getter<value_type>
        { inline value_type operator()(value_type s) const { return s; } };

        template<>
        struct str_getter<std::string>
        { inline value_type operator()(std::string const& s) const { return s.c_str(); } };

        template<typename It, typename GetStr>
        static ::K make_list(It begin, It end, GetStr const& get_str) noexcept
        {
            ptrdiff_t n = std::distance(begin, end);
            assert(0 <= n && n <= std::numeric_limits<::J>::max());
            K_ptr k{ ::ktn(type_id, n) };
            std::transform(begin, end, index(k.get()),
                [&get_str](auto&& sym) {
                    return ::ss(const_cast<::S>(get_str(std::forward<decltype(sym)>(sym))));
                });
            return k.release();
        }
    };

    template<>
    struct TypeTraits<kTimestamp> final
        : public TypeTraits<kLong, TypeTraits<kTimestamp>>
    {
        constexpr static Type type_id = kTimestamp;
        constexpr static char type_code = 'p';
 
        inline static ::K atom(value_type p) noexcept { return ::ktj(-type_id, p); }

        inline static value_type encode(long long year, long long month, long long day,
            long long hour, long long minute, long long second, long long nanos) noexcept
        { return encode_timestamp(year, month, day, hour, minute, second, nanos); }

        inline static value_type parse(long long yyyymmddhhmmssf9) noexcept
        { return parse_timestamp(yyyymmddhhmmssf9); }

        inline static value_type parse(char const* ymdhmsf) noexcept
        { return parse_timestamp(ymdhmsf); }

    private:
        friend impl::TypeBase<TypeTraits<kTimestamp>>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
        {
            if (print_special(out, std::forward<T>(v))) return;

            ::J date = v / 86400'000'000'000LL;
            ::J time = v % 86400'000'000'000LL;
            if (v < 0) {
                date--;
                time += 86400'000'000'000LL;
            }
            assert(std::numeric_limits<::I>::min() <= date
                && date <= std::numeric_limits<::I>::max());
            TypeTraits<kDate>::print(out, static_cast<::I>(date), std::true_type());
            out << 'D';
            TypeTraits<kTimespan>::print(out, time, std::true_type(), true);
        }
    };

    template<>
    struct TypeTraits<kMonth> final
        : public TypeTraits<kInt, TypeTraits<kMonth>>
    {
        constexpr static Type type_id = kMonth;
        constexpr static char type_code = 'm';

        inline static ::K atom(value_type m) noexcept { return generic_atom<TypeTraits<type_id>>(m); }

        inline static value_type encode(int year, int month) noexcept
        { return encode_month(year, month); }

        inline static value_type parse(int yyyymm) noexcept
        { return parse_month(yyyymm); }

        inline static value_type parse(char const* ym) noexcept
        { return parse_month(ym); }

    private:
        friend impl::TypeBase<TypeTraits<kMonth>>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
        {
            static_assert(sizeof(value_type) == sizeof(v),
                "<BUG> unexpected specialization w/ differing types");

            if (print_special(out, std::forward<T>(v))) return;

            ::I const yyyymm = decode_month(std::forward<T>(v));
            out << std::setfill('0')
                << std::setw(4) << (yyyymm / 100) << '.' << std::setw(2) << (yyyymm % 100)
                << type_code;
        }
    };

    template<>
    struct TypeTraits<kDate> final
        : public TypeTraits<kInt, TypeTraits<kDate>>
    {
        constexpr static Type type_id = kDate;
        constexpr static char type_code = 'd';

        inline static ::K atom(value_type d) noexcept { return ::kd(d); }

        inline static value_type encode(int year, int month, int day) noexcept
        { return encode_date(year, month, day); }

        inline static value_type parse(int yyyymmdd) noexcept
        { return parse_date(yyyymmdd); }

        inline static value_type parse(char const* ymd) noexcept
        { return parse_date(ymd); }

    private:
        friend impl::TypeBase<TypeTraits<kDate>>;
        friend TypeTraits<kTimestamp>;
        friend TypeTraits<kDatetime>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
        {
            if (print_special(out, std::forward<T>(v))) return;

            ::I const yyyymmdd = decode_date(std::forward<T>(v));
            out << std::setfill('0')
                << std::setw(4) << (yyyymmdd / 100'00) << '.'
                << std::setw(2) << (yyyymmdd / 100 % 100) << '.'
                << std::setw(2) << (yyyymmdd % 100);
        }
    };

    template<>
    struct TypeTraits<kDatetime> final
        : public TypeTraits<kFloat, TypeTraits<kDatetime>>
    {
        constexpr static Type type_id = kDatetime;
        constexpr static char type_code = 'z';

        inline static ::K atom(value_type z) noexcept { return ::kz(z); }

        inline static value_type encode(int year, int month, int day,
            int hour, int minute, int second, int millis) noexcept
        { return encode_datetime(year, month, day, hour, minute, second, millis); }

        inline static value_type parse(long long yyyymmddhhmmssf3) noexcept
        { return parse_datetime(yyyymmddhhmmssf3); }

        inline static value_type parse(char const* ymdhmsf) noexcept
        { return parse_datetime(ymdhmsf); }

    private:
        friend impl::TypeBase<TypeTraits<kDatetime>>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
        {
            if (print_special(out, std::forward<T>(v))) return;

            ::I date = static_cast<::I>(v);
            ::I time = static_cast<::I>(std::round((v - date) * 86400'000.));
            if (v < 0) {
                date--;
                time += 86400'000;
            }
            TypeTraits<kDate>::print(out, date, std::true_type());
            out << 'T';
            TypeTraits<kTime>::print(out, time, std::true_type());
        }
    };

    template<>
    struct TypeTraits<kTimespan> final
        : public TypeTraits<kLong, TypeTraits<kTimespan>>
    {
        constexpr static Type type_id = kTimespan;
        constexpr static char type_code = 'n';

        inline static ::K atom(value_type n) noexcept { return ::ktj(-type_id, n); }

        inline static value_type encode(long long day,
            long long hour, long long minute, long long second, long long nanos) noexcept
        { return encode_timespan(day, hour, minute, second, nanos); }

        inline static value_type parse(long long hhmmssf9) noexcept
        { return parse_timespan(hhmmssf9); }

        inline static value_type parse(char const* dhmsf) noexcept
        { return parse_timespan(dhmsf); }

    private:
        friend impl::TypeBase<TypeTraits<kTimespan>>;
        friend TypeTraits<kTimestamp>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/,
            bool no_day = false)
        {
            if (print_special(out, std::forward<T>(v))) return;

            ::J dhhmmssf9 = decode_timespan(std::forward<T>(v));
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
    struct TypeTraits<kMinute> final
        : public TypeTraits<kInt, TypeTraits<kMinute>>
    {
        constexpr static Type type_id = kMinute;
        constexpr static char type_code = 'u';

        inline static ::K atom(value_type m) noexcept { return generic_atom<TypeTraits<type_id>>(m); }

        inline static value_type encode(int hour, int minute) noexcept
        { return encode_minute(hour, minute); }

        inline static value_type parse(int hhmm) noexcept
        { return parse_minute(hhmm); }

        inline static value_type parse(char const* hm) noexcept
        { return parse_minute(hm); }

    private:
        friend impl::TypeBase<TypeTraits<kMinute>>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
        {
            if (print_special(out, std::forward<T>(v))) return;

            ::I hhmm = decode_minute(std::forward<T>(v));
            auto const sign = std_ext::sgn(hhmm);
            hhmm *= sign;
            out << std::setfill('0') << std::internal
                << (0 <= sign ? "" : "-")
                << std::setw(2) << (hhmm / 100) << ':'
                << std::setw(2) << (hhmm % 100);
        }
    };

    template<>
    struct TypeTraits<kSecond> final
        : public TypeTraits<kInt, TypeTraits<kSecond>>
    {
        constexpr static Type type_id = kSecond;
        constexpr static char type_code = 'v';

        inline static ::K atom(value_type m) noexcept { return generic_atom<TypeTraits<type_id>>(m); }

        inline static value_type encode(int hour, int minute, int second) noexcept
        { return encode_second(hour, minute, second); }

        inline static value_type parse(int hhmmss) noexcept
        { return parse_second(hhmmss); }

        inline static value_type parse(char const* hms) noexcept
        { return parse_second(hms); }

    private:
        friend impl::TypeBase<TypeTraits<kSecond>>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
        {
            if (print_special(out, std::forward<T>(v))) return;

            ::I hhmmss = decode_second(std::forward<T>(v));
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
    struct TypeTraits<kTime> final
        : public TypeTraits<kInt, TypeTraits<kTime>>
    {
        constexpr static Type type_id = kTime;
        constexpr static char type_code = 't';

        inline static ::K atom(value_type t) noexcept { return ::kt(t); }

        inline static value_type encode(int hour, int minute, int second, int millis) noexcept
        { return encode_time(hour, minute, second, millis); }

        inline static value_type parse(int hhmmssf3) noexcept
        { return parse_time(hhmmssf3); }

        inline static value_type parse(char const* hmsf) noexcept
        { return parse_time(hmsf); }

    private:
        friend impl::TypeBase<TypeTraits<kTime>>;
        friend TypeTraits<kDatetime>;

        template<typename Elem, typename ElemTr, typename T>
        static void print(std::basic_ostream<Elem, ElemTr>& out, T&& v, std::true_type /*is_numeric*/)
        {
            if (print_special(out, std::forward<T>(v))) return;

            ::I hhmmssf3 = decode_time(std::forward<T>(v));
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
    struct TypeTraits<kNil> final
        : public impl::TypeBase<TypeTraits<kNil>>
    {
        using value_type = void;
        constexpr static Type type_id = kNil;
        constexpr static char type_code = ' ';

        inline static ::K atom(...) noexcept { return Nil; }
    };

    template<>
    struct TypeTraits<kError> final
        : public impl::TypeBase<TypeTraits<kError>>
    {
        using value_type = char const*;
        constexpr static Type type_id = kError;
        constexpr static char type_code = ' ';

        static_assert(sizeof(::S) == sizeof(value_type),
            "sizeof(S) == sizeof(<kError>)");

        inline static value_type value(::K k) noexcept { return k->s; }

        /// @return Always @c Nil (errors will be signaled to kdb+ host)
        inline static ::K atom(value_type msg, bool sys = false) noexcept
        { return (sys ? ::orr : ::krr)(const_cast<::S>(msg)); }
    };

#pragma endregion

    /// @brief UDLs that are adapted from q literal suffices
    inline namespace literals
    {
#       define Q_FFI_LITERAL(suffix, tid, cppType)   \
            constexpr decltype(auto) operator"" suffix(cppType v) noexcept  \
            { return static_cast<TypeTraits<(tid)>::value_type>(v); }

        Q_FFI_LITERAL(_qb, kBoolean, unsigned long long int)
        Q_FFI_LITERAL(_qx, kByte, unsigned long long int)
        Q_FFI_LITERAL(_qh, kShort, unsigned long long int)
        Q_FFI_LITERAL(_qi, kInt, unsigned long long int)
        Q_FFI_LITERAL(_qj, kLong, unsigned long long int)
        Q_FFI_LITERAL(_qe, kReal, long double)
        Q_FFI_LITERAL(_qf, kFloat, long double)

#       undef Q_FFI_LITERAL

        inline decltype(auto) operator""_qp(char const* ymdhmsf, size_t /*len*/) noexcept
        { return TypeTraits<kTimestamp>::parse(ymdhmsf); }
        inline decltype(auto) operator""_qp(unsigned long long int yyyymmddhhmmssf9) noexcept
        { return TypeTraits<kTimestamp>::parse(static_cast<long long>(yyyymmddhhmmssf9)); }

        inline decltype(auto) operator""_qm(char const* ym, size_t /*len*/) noexcept
        { return TypeTraits<kMonth>::parse(ym); }
        inline decltype(auto) operator""_qm(unsigned long long int yyyymm) noexcept
        { return TypeTraits<kMonth>::parse(static_cast<int>(yyyymm)); }

        inline decltype(auto) operator""_qd(char const* ymd, size_t /*len*/) noexcept
        { return TypeTraits<kDate>::parse(ymd); }
        inline decltype(auto) operator""_qd(unsigned long long int yyyymmdd) noexcept
        { return TypeTraits<kDate>::parse(static_cast<int>(yyyymmdd)); }

        inline decltype(auto) operator""_qz(char const* ymdhmsf, size_t /*len*/) noexcept
        { return TypeTraits<kDatetime>::parse(ymdhmsf); }
        inline decltype(auto) operator""_qz(unsigned long long int yyyymmddhhmmssf3) noexcept
        { return TypeTraits<kDatetime>::parse(static_cast<long long>(yyyymmddhhmmssf3)); }

        inline decltype(auto) operator""_qn(char const* dhmsf, size_t /*len*/) noexcept
        { return TypeTraits<kTimespan>::parse(dhmsf); }
        inline decltype(auto) operator""_qn(unsigned long long int hhmmssf9) noexcept
        { return TypeTraits<kTimespan>::parse(static_cast<long long>(hhmmssf9)); }

        inline decltype(auto) operator""_qu(char const* hm, size_t /*len*/) noexcept
        { return TypeTraits<kMinute>::parse(hm); }
        inline decltype(auto) operator""_qu(unsigned long long int hhmm) noexcept
        { return TypeTraits<kMinute>::parse(static_cast<int>(hhmm)); }

        inline decltype(auto) operator""_qv(char const* hms, size_t /*len*/) noexcept
        { return TypeTraits<kSecond>::parse(hms); }
        inline decltype(auto) operator""_qv(unsigned long long int hhmmss) noexcept
        { return TypeTraits<kSecond>::parse(static_cast<int>(hhmmss)); }

        inline decltype(auto) operator""_qt(char const* hmsf, size_t /*len*/) noexcept
        { return TypeTraits<kTime>::parse(hmsf); }
        inline decltype(auto) operator""_qt(unsigned long long int hhmmssf3) noexcept
        { return TypeTraits<kTime>::parse(static_cast<int>(hhmmssf3)); }

    }//inline namespace q::literals

}//namespace q