#pragma once

#include <sstream>
#include <iomanip>

#pragma region q::facets::ValueType<>

template<typename Tr, typename Value>
std::string
q::facets::ValueType<Tr, Value>::to_str(
    typename q::facets::ValueType<Tr, Value>::const_reference v)
{
    std::ostringstream buffer;
    Tr::print(buffer, v);
    return buffer.str();
}

template<typename Tr, typename Value>
template<typename Elem, typename ElemTr>
void
q::facets::ValueType<Tr, Value>::print(std::basic_ostream<Elem, ElemTr>& out,
    typename q::facets::ValueType<Tr, Value>::const_reference v)
{
    out << v;
}

#pragma endregion

#pragma region q::facets::NumericType<>

template<typename Tr, typename Value>
template<typename Elem, typename ElemTr>
void
q::facets::NumericType<Tr, Value>::print(std::basic_ostream<Elem, ElemTr>& out,
    typename q::facets::NumericType<Tr, Value>::const_reference v)
{
    if (!Tr::print_special(out, v))
        out << std::to_string(v) << TypeId2Code.at(Tr::type_id);
}

template<typename Tr, typename Value>
template<typename Elem, typename ElemTr>
bool
q::facets::NumericType<Tr, Value>::print_special(std::basic_ostream<Elem, ElemTr>& out,
    typename q::facets::NumericType<Tr, Value>::const_reference v)
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
    out << TypeId2Code.at(Tr::type_id);
    return true;
}

#pragma endregion

#pragma region q::TypeTraits<q::kSymbol>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kSymbol>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kSymbol>::const_reference v)
{
    out << '`' << v;
}

#pragma endregion

#pragma region q::TypeTraits<q::kTimestamp>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kTimestamp>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kTimestamp>::const_reference p)
{
    if (TypeTraits::print_special(out, p))
        return;

    auto const ts = decode(p);
    auto const d = std::chrono::floor<date::days>(ts);
    TypeTraits<kDate>::print(out, date::year_month_day{ d });
    out << 'D' << date::make_time(ts - d);
}

#pragma endregion

#pragma region q::TypeTraits<q::kMonth>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kMonth>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kMonth>::const_reference m)
{
    if (TypeTraits::print_special(out, m))
        return;

    date::year_month_day const ymd{ decode(m) };
    out << std::setfill('0')
        << std::setw(4) << int(ymd.year()) << '.'
        << std::setw(2) << unsigned(ymd.month())
        << TypeId2Code.at(TypeTraits::type_id);
}

#pragma endregion

#pragma region q::TypeTraits<q::kDate>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kDate>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kDate>::const_reference v)
{
    if (!TypeTraits::print_special(out, v))
        print(out, decode(v));
}

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kDate>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kDate>::temporal_type const& t)
{
    date::year_month_day const& ymd{ t };
    out << std::setfill('0')
        << std::setw(4) << int(ymd.year()) << '.'
        << std::setw(2) << unsigned(ymd.month()) << '.'
        << std::setw(2) << unsigned(ymd.day());
}

#pragma endregion

#pragma region q::TypeTraits<q::kDatetime>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kDatetime>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kDatetime>::const_reference z)
{
    if (TypeTraits::print_special(out, z))
        return;

    auto const t = decode(z);
    auto const d = std::chrono::floor<date::days>(t);
    TypeTraits<kDate>::print(out, date::year_month_day{ d });
    out << 'T' << date::make_time(t - d);
}

#pragma endregion

#pragma region q::TypeTraits<q::kTimespan>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kTimespan>::print(std::basic_ostream<Elem, ElemTr>& out, 
    q::TypeTraits<q::kTimespan>::const_reference n)
{
    if (TypeTraits::print_special(out, n))
        return;

    date::hh_mm_ss<std::chrono::nanoseconds> const t{ decode(n) };
    auto const h = t.hours().count();
    if (t.is_negative())
        out << '-';
    out << std::setfill('0') << std::internal
        << (h / time_scale_v<std::chrono::hours, date::days>) << 'D'
        << std::setw(2) << (h % time_scale_v<std::chrono::hours, date::days>) << ':'
        << std::setw(2) << t.minutes().count() << ':'
        << std::setw(2) << t.seconds().count() << '.'
        << std::setw(9) << t.subseconds().count();
}

#pragma endregion

#pragma region q::TypeTraits<q::kMinute>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kMinute>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kMinute>::const_reference u)
{
    if (TypeTraits::print_special(out, u))
        return;

    date::hh_mm_ss<std::chrono::seconds> const hms{ decode(u) };
    if (hms.is_negative())
        out << '-';
    out << std::setfill('0') << std::internal
        << std::setw(2) << hms.hours().count() << ':'
        << std::setw(2) << hms.minutes().count();
}

#pragma endregion

#pragma region q::TypeTraits<q::kSecond>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kSecond>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kSecond>::const_reference v)
{
    if (TypeTraits::print_special(out, v))
        return;

    auto const t = decode(v);
    out << date::hh_mm_ss<std::chrono::seconds>{ t };
}

#pragma endregion

#pragma region q::TypeTraits<q::kTime>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kTime>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kTime>::const_reference t)
{
    if (TypeTraits::print_special(out, t))
        return;

    auto const tt = decode(t);
    out << date::hh_mm_ss<std::chrono::milliseconds>{ tt };
}

#pragma endregion

#pragma region q::TypeTraits<q::kError>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kError>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kError>::value_type const& v)
{
    out << "<q>'" << v;
}

#pragma endregion

#pragma region q::TypeTraits<q::kDLL>

template<typename Elem, typename ElemTr>
void
q::TypeTraits<q::kDLL>::print(std::basic_ostream<Elem, ElemTr>& out,
    q::TypeTraits<q::kDLL>::value_type const& v)
{
    out << "{0x"
        << std::hex << std::setfill('0') << std::setw(sizeof(value_type) * 2) << v
        << '}';
}

#pragma endregion

#pragma region Make kdb+ temporal type representations I/O stream-able
namespace std
{
    namespace chrono
    {
#       define DEFINE_TEMPORAL_IO(T, KTime)    \
            template<typename Elem, typename ElemTr>    \
            std::basic_ostream<Elem, ElemTr>& operator<<(   \
                std::basic_ostream<Elem, ElemTr>& os, KTime const& t)   \
            {   \
                q::TypeTraits<(T)>::print(os, q::TypeTraits<(T)>::encode(t));   \
                return os;  \
            }

        DEFINE_TEMPORAL_IO(q::kMinute, q::Minutes)
        DEFINE_TEMPORAL_IO(q::kSecond, q::Seconds)
        DEFINE_TEMPORAL_IO(q::kTime, q::Milliseconds)
        DEFINE_TEMPORAL_IO(q::kTimespan, q::Nanoseconds)

        DEFINE_TEMPORAL_IO(q::kDate, q::Date)
        DEFINE_TEMPORAL_IO(q::kDatetime, q::DateTime)
        DEFINE_TEMPORAL_IO(q::kTimestamp, q::Timestamp)

#       undef DEFINE_TEMPORAL_IO
    }
}
#pragma endregion

