#include <gtest/gtest.h>
#include "types.hpp"
#include "pointer.hpp"

#pragma region TypeTraitsTests<> typed test suite

template<typename Params>
class TypeTraitsTests : public ::testing::Test
{
protected:

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

    EXPECT_TRUE((std::is_same_v<typename Traits::value_type, typename TypeParam::Value_>));
    EXPECT_EQ(Traits::type_id, TypeParam::id_);
    EXPECT_EQ(Traits::type_code, TypeParam::ch_);
}

TYPED_TEST(TypeTraitsTests, qTypeTraitsQuery)
{
    using Traits = q::TypeTraits<TypeParam::id_>;

    EXPECT_EQ(q::has_value_v<Traits::type_id>, TypeParam::hasValue_);
    EXPECT_EQ(q::has_null_v<Traits::type_id>, TypeParam::hasNull_);
    EXPECT_EQ(q::is_numeric_v<Traits::type_id>, TypeParam::isNumeric_);
}

TYPED_TEST(TypeTraitsTests, typeOf)
{
    test_typeOf(q::has_value<TypeParam::id_>());
}

#pragma region TypeTraitsOpsTests<> typed test suite

template<typename Tr>
class TypeTraitsOpsTests : public ::testing::Test
{
protected:
    using Traits = Tr;
    using value_type = typename Traits::value_type;

    static std::vector<std::pair<value_type, std::string>> const tests_;

    template<typename T>
    void expect_equal(T const& actual, T const& expected)
    {
        // Since there are values that are not comparable (e.g., NaN), use memory compare 
        auto const memory_equal = [](auto&& actual, auto&& expected) -> bool {
            return 0 == std::memcmp(&actual, &expected, sizeof(T));
        };
        EXPECT_PRED2(memory_equal, actual, expected);
    }

    void expect_equal(char const* actual, char const* expected)
    {
        EXPECT_STREQ(actual, expected);
    }
};

#define OPS_TEST_PARAMS(qType)   \
    template<>  \
    std::vector<std::pair<  \
        typename q::TypeTraits<qType>::value_type,  \
        std::string>> const  \
    TypeTraitsOpsTests<q::TypeTraits<qType>>::tests_

OPS_TEST_PARAMS(q::kBoolean) = {
    { true, "1b" },
    { false, "0b" }
};
OPS_TEST_PARAMS(q::kByte) = {
    { (uint8_t)0, "00" },
    { (uint8_t)0x20, "20" },
    { (uint8_t)0xA7, "a7" },
    { (uint8_t)0xFF, "ff" }
};
OPS_TEST_PARAMS(q::kShort) = {
    { (int16_t)0, "0h" },
    { (int16_t)129, "129h" },
    { (int16_t)-128, "-128h" },
    { q::TypeTraits<q::kShort>::null(), "0Nh" },
    { q::TypeTraits<q::kShort>::inf(), "0Wh" },
    { (int16_t)-q::TypeTraits<q::kShort>::inf(), "-0Wh" }
};
OPS_TEST_PARAMS(q::kInt) = {
    { 0, "0i" },
    { 65536, "65536i" },
    { -32768, "-32768i" },
    { q::TypeTraits<q::kInt>::null(), "0Ni" },
    { q::TypeTraits<q::kInt>::inf(), "0Wi" },
    { -q::TypeTraits<q::kInt>::inf(), "-0Wi" }
};
OPS_TEST_PARAMS(q::kLong) = {
    { 0, "0j" },
    { 4'294'967'296LL, "4294967296j" },
    { -2'147'483'648LL, "-2147483648j" },
    { q::TypeTraits<q::kLong>::null(), "0Nj" },
    { q::TypeTraits<q::kLong>::inf(), "0Wj" },
    { -q::TypeTraits<q::kLong>::inf(), "-0Wj" }
};
OPS_TEST_PARAMS(q::kReal) = {
    { 0.f, "0.000000e" },
    { 987.654f, "987.653992e" },
    { -123.456f, "-123.456001e" },
    { q::TypeTraits<q::kReal>::null(), "0Ne" },
    { q::TypeTraits<q::kReal>::inf(), "0We" },
    { -q::TypeTraits<q::kReal>::inf(), "-0We" }
};
OPS_TEST_PARAMS(q::kFloat) = {
    { 0., "0.000000f" },
    { 987.6543210123, "987.654321f" },
    { -123.4567890987, "-123.456789f" },
    { q::TypeTraits<q::kFloat>::null(), "0nf" },
    { q::TypeTraits<q::kFloat>::inf(), "0wf" },
    { -q::TypeTraits<q::kFloat>::inf(), "-0wf" }
};
OPS_TEST_PARAMS(q::kChar) = {
    { '\0', std::string("\0", 1) },
    { 'Z', "Z" },
    { '\xFF', "\xFF" },
    { q::TypeTraits<q::kChar>::null(), " " }
};
OPS_TEST_PARAMS(q::kSymbol) = {
    { "600000.SH", "600000.SH" },
    { "123 abc ABC", "123 abc ABC" },
    { "测试", "测试" },
    { q::TypeTraits<q::kSymbol>::null(), "" }
};

using OpsTypes = ::testing::Types<
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
TYPED_TEST_SUITE(TypeTraitsOpsTests, OpsTypes);

#pragma endregion

TYPED_TEST(TypeTraitsOpsTests, atom)
{
    for (auto const& test : tests_) {
        q::K_ptr k{ Traits::atom(test.first) };
        ASSERT_NE(k.get(), q::Nil);
        EXPECT_EQ(q::type_of(k.get()), -Traits::type_id);
        expect_equal(Traits::value(k.get()), test.first);
    }
}

TYPED_TEST(TypeTraitsOpsTests, list)
{
    std::vector<value_type> samples(tests_.size());
    std::transform(std::cbegin(tests_), std::cend(tests_),
        std::begin(samples), [](auto const& test) { return test.first; });

    q::K_ptr k{ Traits::list(samples.cbegin(), samples.cend()) };
    ASSERT_NE(k.get(), q::Nil);
    EXPECT_EQ(q::type_of(k.get()), Traits::type_id);

    ASSERT_EQ(k->n, tests_.size());
    auto s = std::cbegin(samples);
    auto const e = std::cend(samples);
    for (auto p = Traits::index(k.get()); s != e; ++p, ++s) {
        expect_equal(*p, *s);
    }
}

TEST(TypeTraitsOpsTests, kCharList)
{
    using Traits = q::TypeTraits<q::kChar>;
    char const sample[] = "ABC 123 测试";

    auto str_check = [&sample](q::K_ptr k, size_t length) {
        ASSERT_NE(k.get(), q::Nil);
        EXPECT_EQ(q::type_of(k.get()), Traits::type_id);
        EXPECT_EQ(k->n, length);
        for (size_t i = 0; i < length; ++i) {
            EXPECT_EQ(Traits::index(k.get())[i], sample[i]);
        }
    };

    size_t const length = std::extent_v<decltype(sample), 0> - 1;   //account for \0 terminator
    str_check(q::K_ptr{ Traits::list(sample) }, length);

    size_t const sublen = 7;
    assert(sublen < length);
    str_check(q::K_ptr{ Traits::list(sample, sublen) }, sublen);
}

TEST(TypeTraitsOpsTests, kSymbolList)
{
    using Traits = q::TypeTraits<q::kSymbol>;
    std::vector<std::string> sample{
        "600000.SH",
        "123 abc ABC",
        "测试",
        Traits::null()
    };
    size_t const length = sample.size();

    q::K_ptr k{ Traits::list(std::cbegin(sample), std::cend(sample)) };
    ASSERT_NE(k.get(), q::Nil);
    EXPECT_EQ(q::type_of(k.get()), Traits::type_id);
    EXPECT_EQ(k->n, length);
    for (size_t i = 0; i < length; ++i) {
        EXPECT_STREQ(Traits::index(k.get())[i], sample[i].c_str());
    }
}

TYPED_TEST(TypeTraitsOpsTests, toStr)
{
    for (auto const& test : tests_) {
        EXPECT_EQ(Traits::to_str(test.first), test.second);
    }
}
