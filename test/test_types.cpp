#include <gtest/gtest.h>
#include "types.hpp"
#include "pointer.hpp"

#pragma region TypeTraitsTests<> typed test suite

template<typename Params>
class TypeTraitsTests : public ::testing::Test
{
protected:
    void test_typeOf()
    {
        test_typeOf(q::has_value<Params::id_>());
    }

private:
    template<typename TF>
    void test_typeOf(TF) { }

    template<>
    void test_typeOf(std::true_type)
    {
        using Traits = q::TypeTraits<Params::id_>;

        q::K_ptr pk{ Traits::atom(0) };
        ASSERT_NE(pk.get(), nullptr)
            << "q::TypeTraits<" << Params::id_ << ">::atom(...) fail";
        EXPECT_EQ(q::typeOf(pk), Params::id_);
    }
};

template<typename Value,
    q::Type id, char ch, bool hasValue, bool hasNull, bool isNumeric>
struct test_params
{
    using Value_ = Value;
    constexpr static q::Type const id_ = id;
    constexpr static char const ch_ = ch;
    constexpr static bool const hasValue_ = hasValue;
    constexpr static bool const hasNull_ = hasNull;
    constexpr static bool const isNumeric_ = isNumeric;
};
using TestTypes = ::testing::Types<
    test_params<bool, q::kBoolean, 'b', true, false, false>,
    test_params<uint8_t, q::kByte, 'x', true, true, false>,
    test_params<int16_t, q::kShort, 'h', true, true, true>,
    test_params<int32_t, q::kInt, 'i', true, true, true>,
    test_params<int64_t, q::kLong, 'j', true, true, true>,
    test_params<float, q::kReal, 'e', true, true, true>,
    test_params<double, q::kFloat, 'f', true, true, true>,
    test_params<char, q::kChar, 'c', true, true, false>,
    test_params<char const*, q::kSymbol, 's', true, true, false>,
    test_params<int64_t, q::kTimestamp, 'p', true, true, true>,
    test_params<int32_t, q::kMonth, 'm', true, true, true>,
    test_params<int32_t, q::kDate, 'd', true, true, true>,
    test_params<double, q::kDatetime, 'z', true, true, true>,
    test_params<int64_t, q::kTimespan, 'n', true, true, true>,
    test_params<int32_t, q::kMinute, 'u', true, true, true>,
    test_params<int32_t, q::kSecond, 'v', true, true, true>,
    test_params<int32_t, q::kTime, 't', true, true, true>,
    test_params<void, q::kNil, ' ', false, false, false>,
    test_params<char const*, q::kError, ' ', true, false, false>
>;

TYPED_TEST_SUITE(TypeTraitsTests, TestTypes);

#pragma endregion

TYPED_TEST(TypeTraitsTests, qTypeTraits)
{
    using Traits = q::TypeTraits<TypeParam::id_>;

    EXPECT_TRUE((std::is_same_v<
        typename Traits::value_type,
        typename TypeParam::Value_
    >));
    EXPECT_EQ(Traits::id, TypeParam::id_);
    EXPECT_EQ(Traits::ch, TypeParam::ch_);
}

TYPED_TEST(TypeTraitsTests, qTypeTraitsQuery)
{
    using Traits = q::TypeTraits<TypeParam::id_>;

    EXPECT_EQ(q::has_value_v<Traits::id>, TypeParam::hasValue_);
    EXPECT_EQ(q::has_null_v<Traits::id>, TypeParam::hasNull_);
    EXPECT_EQ(q::is_numeric_v<Traits::id>, TypeParam::isNumeric_);
}

TYPED_TEST(TypeTraitsTests, typeOf)
{
    test_typeOf();
}

#pragma region TypeTraitsToStrTests<> typed test suite

template<typename Tr>
class TypeTraitsToStrTests : public ::testing::Test
{
protected:
    using Traits = Tr;
    using value_type = typename Traits::value_type;

private:
    static std::vector<std::pair<value_type, std::string>> const tests_;

protected:
    void test_samples()
    {
        for (auto const& test : tests_)
        {
            EXPECT_EQ(Traits::to_str(test.first), test.second);
        }
    }
};

#define TO_STR_TEST_PARAMS(qType)   \
    template<>  \
    std::vector<std::pair<  \
        typename q::TypeTraits<qType>::value_type,  \
        std::string>> const  \
    TypeTraitsToStrTests<q::TypeTraits<qType>>::tests_

TO_STR_TEST_PARAMS(q::kBoolean) = {
    { true, "1b" },
    { false, "0b" }
};
TO_STR_TEST_PARAMS(q::kByte) = {
    { (uint8_t)0, "00" },
    { (uint8_t)0x20, "20" },
    { (uint8_t)0xA7, "a7" },
    { (uint8_t)0xFF, "ff" }
};
TO_STR_TEST_PARAMS(q::kShort) = {
    { (int16_t)0, "0h" },
    { (int16_t)129, "129h" },
    { (int16_t)-128, "-128h" },
    { q::TypeTraits<q::kShort>::null(), "0Nh" },
    { q::TypeTraits<q::kShort>::inf(), "0Wh" },
    { (int16_t)-q::TypeTraits<q::kShort>::inf(), "-0Wh" }
};
TO_STR_TEST_PARAMS(q::kInt) = {
    { 0, "0i" },
    { 65536, "65536i" },
    { -32768, "-32768i" },
    { q::TypeTraits<q::kInt>::null(), "0Ni" },
    { q::TypeTraits<q::kInt>::inf(), "0Wi" },
    { -q::TypeTraits<q::kInt>::inf(), "-0Wi" }
};
TO_STR_TEST_PARAMS(q::kLong) = {
    { 0, "0j" },
    { 4'294'967'296LL, "4294967296j" },
    { -2'147'483'648LL, "-2147483648j" },
    { q::TypeTraits<q::kLong>::null(), "0Nj" },
    { q::TypeTraits<q::kLong>::inf(), "0Wj" },
    { -q::TypeTraits<q::kLong>::inf(), "-0Wj" }
};
TO_STR_TEST_PARAMS(q::kReal) = {
    { 0.f, "0.000000e" },
    { 987.654f, "987.653992e" },
    { -123.456f, "-123.456001e" },
    { q::TypeTraits<q::kReal>::null(), "0Ne" },
    { q::TypeTraits<q::kReal>::inf(), "0We" },
    { -q::TypeTraits<q::kReal>::inf(), "-0We" }
};
TO_STR_TEST_PARAMS(q::kFloat) = {
    { 0., "0.000000f" },
    { 987.6543210123, "987.654321f" },
    { -123.4567890987, "-123.456789f" },
    { q::TypeTraits<q::kFloat>::null(), "0nf" },
    { q::TypeTraits<q::kFloat>::inf(), "0wf" },
    { -q::TypeTraits<q::kFloat>::inf(), "-0wf" }
};
TO_STR_TEST_PARAMS(q::kChar) = {
    { '\0', std::string("\0", 1) },
    { 'Z', "Z" },
    { '\xFF', "\xFF" },
    { q::TypeTraits<q::kChar>::null(), " " }
};
TO_STR_TEST_PARAMS(q::kSymbol) = {
    { "600000.SH", "600000.SH" },
    { "123 abc ABC", "123 abc ABC" },
    { "≤‚ ‘", "≤‚ ‘" },
    { q::TypeTraits<q::kSymbol>::null(), "" }
};

using ToStrTypes = ::testing::Types<
    q::TypeTraits<q::kBoolean>,
    q::TypeTraits<q::kByte>,
    q::TypeTraits<q::kShort>,
    q::TypeTraits<q::kInt>,
    q::TypeTraits<q::kLong>,
    q::TypeTraits<q::kReal>,
    q::TypeTraits<q::kFloat>,
    q::TypeTraits<q::kChar>,
    q::TypeTraits<q::kSymbol>/*,
    q::TypeTraits<q::kTimestamp>,
    q::TypeTraits<q::kMonth>,
    q::TypeTraits<q::kDate>,
    q::TypeTraits<q::kDatetime>,
    q::TypeTraits<q::kTimespan>,
    q::TypeTraits<q::kMinute>,
    q::TypeTraits<q::kSecond>,
    q::TypeTraits<q::kTime>
    */
    //q::TypeTraits<q::kNil>
    //q::TypeTraits<q::kError>
>;
TYPED_TEST_SUITE(TypeTraitsToStrTests, ToStrTypes);

#pragma endregion

TYPED_TEST(TypeTraitsToStrTests, toStr)
{
    test_samples();
}
