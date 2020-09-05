#include <gtest/gtest.h>
#include "types.hpp"
#include "pointer.hpp"

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
    test_params<char, q::kByte, 'x', true, true, false>,
    test_params<short, q::kShort, 'h', true, true, true>,
    test_params<int32_t, q::kInt, 'i', true, true, true>,
    test_params<int64_t, q::kLong, 'j', true, true, true>,
    test_params<float, q::kReal, 'e', true, true, true>,
    test_params<double, q::kFloat, 'f', true, true, true>,
    test_params<char, q::kChar, 'c', true, true, false>,
    test_params<char const*, q::kSymbol, 's', true, true, false>,
    test_params<void, q::kNil, ' ', false, false, false>,
    test_params<char const*, q::kError, ' ', true, false, false>
>;

template<typename Params>
class TypeTraitsTests : public ::testing::Test {};
TYPED_TEST_SUITE(TypeTraitsTests, TestTypes);

TYPED_TEST(TypeTraitsTests, qTypeTraits)
{
    using Traits = q::TypeTraits<TypeParam::id_>;

    EXPECT_TRUE((std::is_same_v<typename Traits::value_type, typename TypeParam::Value_>))
        << "q::TypeTraits<" << TypeParam::id_ << "> reports incorrect value_type";
    EXPECT_EQ(Traits::id, TypeParam::id_)
        << "q::TypeTraits<" << TypeParam::id_ << "> reports incorrect type ID";
    EXPECT_EQ(Traits::ch, TypeParam::ch_)
        << "q::TypeTraits<" << TypeParam::id_ << "> reports incorrect type ID character";
}

TYPED_TEST(TypeTraitsTests, qTypeTraitsQuery)
{
    using Traits = q::TypeTraits<TypeParam::id_>;
    auto const non_prefix = [](bool f) { return f ? "" : "-non"; };

    EXPECT_EQ(q::has_value_t<Traits::id>, TypeParam::hasValue_)
        << "q::TypeTraits<" << TypeParam::id_ << "> should report as "
        << non_prefix(TypeParam::hasValue_) << "value type";
    EXPECT_EQ(q::has_value_t<Traits::id>, TypeParam::hasValue_)
        << "<" << TypeParam::id_ << "> should report as "
        << non_prefix(TypeParam::hasValue_) << "value type";

    EXPECT_EQ(q::has_null_t<Traits::id>, TypeParam::hasNull_)
        << "q::TypeTraits<" << TypeParam::id_ << "> should report as "
        << non_prefix(TypeParam::hasNull_) << "nullable type";
    EXPECT_EQ(q::has_null_t<Traits::id>, TypeParam::hasNull_)
        << "<" << TypeParam::id_ << "> should report as "
        << non_prefix(TypeParam::hasNull_) << "nullable type";

    EXPECT_EQ(q::is_numeric_t<Traits::id>, TypeParam::isNumeric_)
        << "q::TypeTraits<" << TypeParam::id_ << "> should report as "
        << non_prefix(TypeParam::isNumeric_) << "numeric type";
    EXPECT_EQ(q::is_numeric_t<Traits::id>, TypeParam::isNumeric_)
        << "<" << TypeParam::id_ << "> should report as "
        << non_prefix(TypeParam::isNumeric_) << "numeric type";
}
