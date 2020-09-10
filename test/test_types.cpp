#include <gtest/gtest.h>
#include "types.hpp"
#include "pointer.hpp"
#include <map>

#pragma region TypeTraitsTests<> typed test suite

template<typename TraitsInfo>
class TypeTraitsTests : public ::testing::Test
{
protected:
    template<typename Tr>
    void test_type_of(std::true_type /*has_value*/, bool notKError);
    template<typename Tr>
    void test_type_of(std::false_type /*has_value*/, bool)
    { ASSERT_FALSE(TraitsInfo::has_value); }

    template<typename Tr>
    void test_type_of_list(std::true_type /*can_index*/);
    template<typename Tr>
    void test_type_of_list(std::false_type /*can_index*/)
    { ASSERT_FALSE(TraitsInfo::can_index); }

    template<typename Tr>
    q::K_ptr init_atom(std::false_type /*convertible from (char const*)*/)
    { return q::K_ptr{ Tr::atom(typename Tr::value_type()) }; }
    template<typename Tr>
    q::K_ptr init_atom(std::true_type /*convertible from (char const*)*/)
    { return q::K_ptr{ Tr::atom("") }; }

    template<typename Tr>
    q::K_ptr init_list(std::false_type /*convertible from (char const*)*/)
    { return q::K_ptr{ Tr::list({ typename Tr::value_type() }) }; }
    template<typename Tr>
    q::K_ptr init_list(std::true_type /*convertible from (char const*)*/)
    { return q::K_ptr{ Tr::list({ "" }) }; }
};

template<typename Value,
    q::Type tid, char code, bool hasValue, bool hasNull, bool canIndex, bool isNumeric>
struct TraitsInfo
{
    using value_type = Value;
    constexpr static q::Type type_id = tid;
    constexpr static char type_code = code;
    constexpr static bool has_value = hasValue;
    constexpr static bool has_null = hasNull;
    constexpr static bool can_index = canIndex;
    constexpr static bool is_numeric = isNumeric;
};
using TraitsTestTypes = ::testing::Types<
    TraitsInfo<unsigned char, q::kBoolean, 'b', true, false, true, false>,
    TraitsInfo<uint8_t, q::kByte, 'x', true, true, true, false>,
    TraitsInfo<int16_t, q::kShort, 'h', true, true, true, true>,
    TraitsInfo<int32_t, q::kInt, 'i', true, true, true, true>,
    TraitsInfo<int64_t, q::kLong, 'j', true, true, true, true>,
    TraitsInfo<float, q::kReal, 'e', true, true, true, true>,
    TraitsInfo<double, q::kFloat, 'f', true, true, true, true>,
    TraitsInfo<char, q::kChar, 'c', true, true, true, false>,
    TraitsInfo<char const*, q::kSymbol, 's', true, true, true, false>,
    TraitsInfo<int64_t, q::kTimestamp, 'p', true, true, true, true>,
    TraitsInfo<int32_t, q::kMonth, 'm', true, true, true, true>,
    TraitsInfo<int32_t, q::kDate, 'd', true, true, true, true>,
    TraitsInfo<double, q::kDatetime, 'z', true, true, true, true>,
    TraitsInfo<int64_t, q::kTimespan, 'n', true, true, true, true>,
    TraitsInfo<int32_t, q::kMinute, 'u', true, true, true, true>,
    TraitsInfo<int32_t, q::kSecond, 'v', true, true, true, true>,
    TraitsInfo<int32_t, q::kTime, 't', true, true, true, true>,
    TraitsInfo<void, q::kNil, ' ', false, false, false, false>,
    TraitsInfo<char const*, q::kError, ' ', true, false, false, false>
>;

TYPED_TEST_SUITE(TypeTraitsTests, TraitsTestTypes);

TYPED_TEST(TypeTraitsTests, qTypeTraits)
{
    using traits = q::TypeTraits<TypeParam::type_id>;
    ASSERT_TRUE((std::is_same_v<
        typename traits::value_type,
        typename TypeParam::value_type
    >));
    EXPECT_EQ(traits::type_id, TypeParam::type_id);
    EXPECT_EQ(traits::type_code, TypeParam::type_code);
}

TYPED_TEST(TypeTraitsTests, qTypeTraitsQueries)
{
    EXPECT_EQ(q::has_value_v<TypeParam::type_id>, TypeParam::has_value);
    EXPECT_EQ(q::has_null_v<TypeParam::type_id>, TypeParam::has_null);
    EXPECT_EQ(q::can_index_v<TypeParam::type_id>, TypeParam::can_index);
    EXPECT_EQ(q::is_numeric_v<TypeParam::type_id>, TypeParam::is_numeric);
}

template<typename TraitsInfo>
template<typename Tr>
void TypeTraitsTests<TraitsInfo>::test_type_of(std::true_type /*has_value*/, bool notKError)
{
    q::K_ptr k = init_atom<Tr>(std::integral_constant<bool,
        std::is_convertible_v<char const*, typename Tr::value_type>>());
    if (notKError) {
        ASSERT_NE(k.get(), nullptr)
            << "TypeTraits<" << Tr::type_id << ">::atom() failed";
        EXPECT_EQ(q::type_of(k.get()), -Tr::type_id);
    }
    else {
        ASSERT_EQ(k.get(), nullptr)
            << "TypeTraits<" << Tr::type_id << ">::atom() should not return non-null error";
    }
}

template<typename TraitsInfo>
template<typename Tr>
void TypeTraitsTests<TraitsInfo>::test_type_of_list(std::true_type /*can_index*/)
{
    q::K_ptr k = init_list<Tr>(std::integral_constant<bool,
        std::is_convertible_v<char const*, typename Tr::value_type>>());
    ASSERT_NE(k.get(), nullptr)
        << "TypeTraits<" << Tr::type_id << ">::list() failed";
    EXPECT_EQ(q::type_of(k.get()), Tr::type_id);
}

TYPED_TEST(TypeTraitsTests, qTypeOf)
{
    using traits = q::TypeTraits<TypeParam::type_id>;
    test_type_of<traits>(q::has_value<traits::type_id>(), q::kError != traits::type_id);
    test_type_of_list<traits>(q::can_index<traits::type_id>());
}

#pragma endregion

#pragma region TypeTraitsOpsTests<> typed test suite

template<typename TraitsInfo>
class TypeTraitsOpsTests : public ::testing::Test
{
protected:
    static std::map<typename TraitsInfo::value_type, std::string> const samples_;

    template<typename T>
    void expect_equal(T const& actual, T const& expected)
    {
        // Some values are not comparable (e.g. NaN), use bit comparison
        auto const bit_equal = [](auto&& actual, auto&& expected) -> bool {
            return 0 == std::memcmp(&actual, &expected, sizeof(T));
        };
        EXPECT_PRED2(bit_equal, actual, expected);
    }

    void expect_equal(char const* actual, char const* expected)
    {
        EXPECT_STREQ(actual, expected);
    }
};

#define OPS_TEST_SET(tid)   \
    template<>  \
    std::map<typename q::TypeTraits<(tid)>::value_type, std::string> const  \
    TypeTraitsOpsTests<q::TypeTraits<(tid)>>::samples_

using namespace std::literals;
using namespace q::literals;

OPS_TEST_SET(q::kBoolean) = {
    { 1_qb, "1b" },
    { 0_qb, "0b" }
};
OPS_TEST_SET(q::kByte) = {
    { 0_qx, "00" },
    { 0x20_qx, "20" },
    { 0xA7_qx, "a7" },
    { 0xFF_qx, "ff" }
};
OPS_TEST_SET(q::kShort) = {
    { 0_qh, "0h" },
    { 129_qh, "129h" },
    { q::TypeTraits<q::kShort>::value_type{ -128 }, "-128h" },
    { q::TypeTraits<q::kShort>::null(), "0Nh" },
    { q::TypeTraits<q::kShort>::inf(), "0Wh" },
    { q::TypeTraits<q::kShort>::value_type{ -q::TypeTraits<q::kShort>::inf() }, "-0Wh" }
};
OPS_TEST_SET(q::kInt) = {
    { 0_qi, "0i" },
    { 65536_qi, "65536i" },
    { -32768_qi, "-32768i" },
    { q::TypeTraits<q::kInt>::null(), "0Ni" },
    { q::TypeTraits<q::kInt>::inf(), "0Wi" },
    { -q::TypeTraits<q::kInt>::inf(), "-0Wi" }
};
OPS_TEST_SET(q::kLong) = {
    { 0_qj, "0j" },
    { 4'294'967'296_qj, "4294967296j" },
    { -2'147'483'648_qj, "-2147483648j" },
    { q::TypeTraits<q::kLong>::null(), "0Nj" },
    { q::TypeTraits<q::kLong>::inf(), "0Wj" },
    { -q::TypeTraits<q::kLong>::inf(), "-0Wj" }
};
OPS_TEST_SET(q::kReal) = {
    { 0._qe, "0.000000e" },
    { 987.654_qe, "987.653992e" },
    { -123.456_qe, "-123.456001e" },
    { q::TypeTraits<q::kReal>::null(), "0Ne" },
    { q::TypeTraits<q::kReal>::inf(), "0We" },
    { -q::TypeTraits<q::kReal>::inf(), "-0We" }
};
OPS_TEST_SET(q::kFloat) = {
    { 0._qf, "0.000000f" },
    { 987.6543210123_qf, "987.654321f" },
    { -123.4567890987_qf, "-123.456789f" },
    { q::TypeTraits<q::kFloat>::null(), "0Nf" },
    { q::TypeTraits<q::kFloat>::inf(), "0Wf" },
    { -q::TypeTraits<q::kFloat>::inf(), "-0Wf" }
};
OPS_TEST_SET(q::kChar) = {
    { '\0', "\0"s },
    { 'Z', "Z"s },
    { '\xFF', "\xFF"s },
    { q::TypeTraits<q::kChar>::null(), " "s }
};
OPS_TEST_SET(q::kSymbol) = {
    { "600000.SH", "600000.SH"s },
    { "123 abc ABC", "123 abc ABC"s },
    { "测试", "测试"s },
    { q::TypeTraits<q::kSymbol>::null(), ""s }
};

OPS_TEST_SET(q::kMonth) = {
    { "2000.01m"_qm, "2000.01m"s },
    { "2020/9"_qm, "2020.09m"s },
    { "1997-11"_qm, "1997.11m"s },
    { 197001_qm, "1970.01m"s },
    { "1900.1"_qm, "1900.01m"s },
    { q::TypeTraits<q::kMonth>::null(), "0Nm"s },
    { q::TypeTraits<q::kMonth>::inf(), "0Wm"s },
    { -q::TypeTraits<q::kMonth>::inf(), "-0Wm"s }
};
OPS_TEST_SET(q::kDate) = {
    { "2000.01.01"_qd, "2000.01.01"s },
    { "2020/9/10"_qd, "2020.09.10"s },
    { "1997-11-28"_qd, "1997.11.28"s },
    { 19700101_qd, "1970.01.01"s },
    { "1900.1.1"_qd, "1900.01.01"s },
    { q::TypeTraits<q::kDate>::null(), "0Nd"s },
    { q::TypeTraits<q::kDate>::inf(), "0Wd"s },
    { -q::TypeTraits<q::kDate>::inf(), "-0Wd"s }
};

using TraitsOpsTestTypes = ::testing::Types<
    q::TypeTraits<q::kBoolean>,
    q::TypeTraits<q::kByte>,
    q::TypeTraits<q::kShort>,
    q::TypeTraits<q::kInt>,
    q::TypeTraits<q::kLong>,
    q::TypeTraits<q::kReal>,
    q::TypeTraits<q::kFloat>,
    q::TypeTraits<q::kChar>,
    q::TypeTraits<q::kSymbol>/*,
    q::TypeTraits<q::kTimestamp>*/,
    q::TypeTraits<q::kMonth>,
    q::TypeTraits<q::kDate>/*,
    q::TypeTraits<q::kDatetime>,
    q::TypeTraits<q::kTimespan>,
    q::TypeTraits<q::kMinute>,
    q::TypeTraits<q::kSecond>,
    q::TypeTraits<q::kTime>*/
    //q::TypeTraits<q::kNil>
    //q::TypeTraits<q::kError>
>;

TYPED_TEST_SUITE(TypeTraitsOpsTests, TraitsOpsTestTypes);

TYPED_TEST(TypeTraitsOpsTests, atomAndValue)
{
    using traits = q::TypeTraits<TypeParam::type_id>;

    for (auto const& sample : samples_) {
        q::K_ptr k{ traits::atom(sample.first) };
        ASSERT_NE(k.get(), q::Nil);
        EXPECT_EQ(q::type_of(k.get()), -TypeParam::type_id);
        expect_equal(traits::value(k.get()), sample.first);
    }
}

TYPED_TEST(TypeTraitsOpsTests, listAndIndex)
{
    using traits = q::TypeTraits<TypeParam::type_id>;

    std::vector<typename TypeParam::value_type> values(samples_.size());
    std::transform(std::cbegin(samples_), std::cend(samples_), std::begin(values),
        [](auto const& sample) { return sample.first; });

    q::K_ptr k{ traits::list(std::cbegin(values), std::cend(values)) };
    ASSERT_NE(k.get(), q::Nil);
    EXPECT_EQ(q::type_of(k.get()), TypeParam::type_id);
    ASSERT_EQ(q::count_of(k.get()), values.size());

    auto s = std::cbegin(values);
    auto const e = std::cend(values);
    auto p = traits::index(k.get());
    ASSERT_NE(p, nullptr);
    for (; s != e; ++p, ++s) expect_equal(*p, *s);
}

TEST(TypeTraitsOpsTests, kCharList)
{
    using traits = q::TypeTraits<q::kChar>;
    char const sample[] = "ABC 123 测试\0+-/";

    auto str_check = [&sample](q::K_ptr k, size_t length) {
        ASSERT_NE(k.get(), q::Nil);
        EXPECT_EQ(q::type_of(k.get()), traits::type_id);
        ASSERT_EQ(q::count_of(k.get()), length);
        for (size_t i = 0; i < length; ++i) {
            EXPECT_EQ(traits::index(k.get())[i], sample[i]);
        }
    };

    size_t const strlen = std::strlen(sample);
    str_check(q::K_ptr{ traits::list(sample) }, strlen);

    size_t const length = std::extent_v<decltype(sample), 0> - 1;   // less trailing '\0'
    str_check(q::K_ptr{ traits::list(sample, length) }, length);
}

TEST(TypeTraitsOpsTests, kSymbolList)
{
    using traits = q::TypeTraits<q::kSymbol>;
    std::vector<std::string> sample{
        "600000.SH"s,
        "123 abc ABC"s,
        "  abc\0ABC"s,
        "测试"s,
        "\0"s,
        traits::null()
    };
    size_t const length = sample.size();

    q::K_ptr k{ traits::list(std::cbegin(sample), std::cend(sample)) };
    ASSERT_NE(k.get(), q::Nil);
    EXPECT_EQ(q::type_of(k.get()), traits::type_id);
    ASSERT_EQ(q::count_of(k.get()), length);
    for (size_t i = 0; i < length; ++i) {
        EXPECT_STREQ(traits::index(k.get())[i], sample[i].c_str());
    }
}

TYPED_TEST(TypeTraitsOpsTests, toStr)
{
    using traits = q::TypeTraits<TypeParam::type_id>;

    for (auto const& sample : samples_) {
        EXPECT_EQ(traits::to_str(sample.first), sample.second);
    }
}

#pragma endregion
