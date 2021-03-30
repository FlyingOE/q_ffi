#include "type_convert.hpp"
#include <algorithm>
#include <date/date.h>
#include "ktype_traits.hpp"
#include "kerror.hpp"

using namespace std;

void q::details::signalError(char const* message) noexcept(false)
{
    throw K_error(message);
}

#define GET_VALUE_TYPE(T, x)    \
    case -(T): \
        return TypeTraits<(T)>::value((x))

#define GET_VALUE_LIST(T, x, r, dryRun) \
    case (T):  \
        if (!(dryRun))  \
            copy(TypeTraits<(T)>::index((x)), TypeTraits<(T)>::index((x)) + count((x)), \
                (r).begin());   \
        break

#pragma region q <==> C++ decimal

long long q::q2Decimal(::K x, bool /*dryRun*/) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil decimal");
    }
    switch (type(x))
    {
        GET_VALUE_TYPE(kBoolean, x);
        GET_VALUE_TYPE(kByte, x);
        GET_VALUE_TYPE(kShort, x);
        GET_VALUE_TYPE(kInt, x);
        GET_VALUE_TYPE(kLong, x);
    default:
        throw K_error("not a decimal");
    }
}

vector<long long> q::q2Decimals(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil decimal list");
    }
    else if (type(x) < 0) {
        throw K_error("not a list");
    }
    vector<long long> result(dryRun ? 0 : count(x), 0L);
    switch (type(x))
    {
        GET_VALUE_LIST(kBoolean, x, result, dryRun);
        GET_VALUE_LIST(kByte, x, result, dryRun);
        GET_VALUE_LIST(kShort, x, result, dryRun);
        GET_VALUE_LIST(kInt, x, result, dryRun);
        GET_VALUE_LIST(kLong, x, result, dryRun);
    default:
        throw K_error("not a decimal list");
    }
    return result;
}

#pragma endregion

#pragma region q <==> C++ floating-point

double q::q2Real(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil floating-point");
    }
    switch (type(x))
    {
        GET_VALUE_TYPE(kReal, x);
        GET_VALUE_TYPE(kFloat, x);
    case -kBoolean:
    case -kByte:
    case -kShort:
    case -kInt:
    case -kLong:
        return static_cast<double>(q2Decimal(x, dryRun));
    default:
        throw K_error("not a floating-point");
    }
}

vector<double> q::q2Reals(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil floating-point list");
    }
    else if (type(x) < 0) {
        throw K_error("not a list");
    }
    vector<double> result(dryRun ? 0 : count(x), 0.);
    switch (type(x))
    {
        GET_VALUE_LIST(kReal, x, result, dryRun);
        GET_VALUE_LIST(kFloat, x, result, dryRun);
    case kBoolean:
    case kByte:
    case kShort:
    case kInt:
    case kLong: {
        auto const decimals = q2Decimals(x, dryRun);
        if (!dryRun)
            transform(decimals.cbegin(), decimals.cend(), result.begin(),
                [](auto x) { return static_cast<double>(x); });
        break;
    }
    default:
        throw K_error("not a floating-point list");
    }
    return result;
}

#pragma endregion

#pragma region q <==> C++ char

char q::q2Char(::K x, bool /*dryRun*/) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil char");
    }
    switch (type(x)) {
    case -kChar:
        return TypeTraits<kChar>::value(x);
    default:
        throw K_error("not a char");
    }
}

#pragma endregion

#pragma region q <==> C++ string

string q::q2String(::K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil symbol/char list");
    }
    switch (type(x)) {
    case kChar:
        return dryRun ? "" : string(TypeTraits<kChar>::index(x), count(x));
    case -kSymbol:
        return dryRun ? "" : string(TypeTraits<kSymbol>::value(x));
    default:
        if ((-kEnumMin >= type(x)) && (type(x) >= -kEnumMax))
            throw K_error("enumerated symbol");
        else
            throw K_error("not a symbol/char list");
    }
}

vector<string> q::q2Strings(K x, bool dryRun) noexcept(false)
{
    if (Nil == x) {
        throw K_error("nil symbol list/char lists");
    }
    else if (type(x) < 0) {
        throw K_error("not a list");
    }
    vector<string> result;
    if (!dryRun)
        result.reserve(count(x));
    switch (type(x)) {
    case kMixed:
        try {
            if (dryRun)
                for_each(TypeTraits<kMixed>::index(x), TypeTraits<kMixed>::index(x) + count(x),
                    [](::K s) { q2String(s); });
            else
                for_each(TypeTraits<kMixed>::index(x), TypeTraits<kMixed>::index(x) + count(x),
                    [&result](::K s) { result.push_back(q2String(s)); });
        }
        catch (K_error const& ) {
            throw K_error("mixed char lists");
        }
        break;
    case kSymbol:
        if (!dryRun)
            for_each(TypeTraits<kSymbol>::index(x), TypeTraits<kSymbol>::index(x) + count(x),
                [&result](auto const s) { result.push_back(s); });
        break;
    default:
        if ((kEnumMin <= type(x)) && (type(x) <= kEnumMax))
            throw K_error("enumerated symbol list");
        else
            throw K_error("not a symbol list/char lists");
    }
    assert(result.size() == count(x));
    return result;
}

#pragma endregion
