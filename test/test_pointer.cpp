#include <gtest/gtest.h>
#include "pointer.hpp"
#include "types.hpp"

TEST(KptrTests, refCount)
{
    q::K_ptr pk{ q::TypeTraits<q::kSymbol>::atom(test_info_->name()) };
    ASSERT_NE(pk.get(), q::Nil) << "fail to create K object";

    K k = pk.get();
    auto refCount = k->r;
    EXPECT_EQ(refCount, 0) << "different initial ref count vs documentation?";

    q::K_ptr pk1{ k };
    EXPECT_EQ(k->r, refCount);

    for (auto i : { 1, 1 }) {
        r1(k);
        refCount += i;
    }
    EXPECT_EQ(k->r, refCount);
    {
        q::K_ptr pk2;
        EXPECT_EQ(pk2.get(), q::Nil);
        pk2.reset(k);
        EXPECT_EQ(k->r, refCount);
        pk2.release();
        EXPECT_EQ(k->r, refCount);
        pk2.reset(k);
        EXPECT_EQ(k->r, refCount);
        pk2.reset();
        --refCount;
        EXPECT_EQ(k->r, refCount);

        pk2.reset(k);
        q::K_ptr pk3 = std::move(pk2);
        EXPECT_EQ(k->r, refCount);
    }
    --refCount;
    EXPECT_EQ(k->r, refCount);
}

#pragma region KptrTests<> typed test suite

template<typename Tr>
class KptrTests : public ::testing::Test
{
protected:
    using traits = Tr;
    using value_type = typename traits::value_type;

    static std::vector<value_type> samples_;

    void SetUp() override
    {
        extra_samples_for_null(q::has_null<traits::type_id>());
        extra_samples_for_numeric(q::is_numeric<traits::type_id>());
    }

    void extra_samples_for_null(std::false_type /*has_null*/) {}
    void extra_samples_for_null(std::true_type /*has_null*/)
    {
        samples_.push_back(traits::null());
    }

    void extra_samples_for_numeric(std::false_type /*is_numeric*/) {}
    void extra_samples_for_numeric(std::true_type /*is_numeric*/)
    {
        samples_.insert(std::end(samples_), {
            traits::inf(),
            std::numeric_limits<value_type>::min(),
            std::numeric_limits<value_type>::max()
        });
    }

    template<typename = std::enable_if_t<!std::is_same_v<value_type, char const*>>>
    void expect_equal(::K const k, value_type const& v)
    {
        auto const bit_equal = [](auto&& actual, auto&& expected) {
            return sizeof(actual) == sizeof(expected) &&
                0 == std::memcmp(&actual, &expected, sizeof(actual));
        };
        // Use bit comparison because some values (e.g. NaN) are not comparable
        EXPECT_PRED2(bit_equal, traits::value(k), v);
    }

    void expect_equal(::K const k, char const* str)
    {
        EXPECT_STREQ(traits::value(k), str);
    }
};

#define KPTR_TEST_SET(tid)  \
    template<>  \
    std::vector<typename q::TypeTraits<(tid)>::value_type>  \
    KptrTests<q::TypeTraits<(tid)>>::samples_

using namespace q::literals;

KPTR_TEST_SET(q::kBoolean) = { 1_qb, 0_qb };
KPTR_TEST_SET(q::kByte) = { 0_qx, 0x20_qx, 0xA7_qx, 0xFF_qx };
KPTR_TEST_SET(q::kShort) = { 0_qh, 129_qh, q::TypeTraits<q::kShort>::value_type{ -128 } };
KPTR_TEST_SET(q::kInt) = { 0_qi, 65536_qi, -32768_qi };
KPTR_TEST_SET(q::kLong) = { 0_qj, 4'294'967'296_qj, -2'147'483'648_qj };
KPTR_TEST_SET(q::kReal) = { 0._qe, 987.654_qe, -123.456_qe };
KPTR_TEST_SET(q::kFloat) = { 0._qf, 987.6543210123_qf, -123.4567890987_qf };
KPTR_TEST_SET(q::kChar) = { '\0', 'Z', '\xFF' };
KPTR_TEST_SET(q::kSymbol) = { "600000.SH", "123 abc ABC", "测试" };

KPTR_TEST_SET(q::kMonth) = { "2000.01m"_qm, "2020/9"_qm,
    "1997-11"_qm, 197001_qm, "1900.1"_qm };
KPTR_TEST_SET(q::kDate) = { "2000.01.01"_qd, "2020/9/10"_qd,
    "1997-11-28"_qd, 19700101_qd, "1900.1.1"_qd };
//KPTR_TEST_PARAMS(q::kNil) =   // cannot be `created'
//KPTR_TEST_PARAMS(q::kError) = // cannot be `created'

using KptrTestTypes = ::testing::Types <
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

TYPED_TEST_SUITE(KptrTests, KptrTestTypes);

TYPED_TEST(KptrTests, makeK)
{
    for (auto const sample : samples_) {
        q::K_ptr pk{ traits::atom(sample) };
        ASSERT_NE(pk.get(), q::Nil) << "Fail to create <" << traits::type_id << "> K object";
        EXPECT_EQ(q::type_of(pk.get()), -traits::type_id);
        expect_equal(pk.get(), sample);
    }
}

TEST(KptrTests, dupK)
{
    q::K_ptr pk{ q::TypeTraits<q::kSymbol>::atom(test_info_->name()) };
    ASSERT_NE(pk.get(), q::Nil) << "fail to create K object";

    K k = pk.get();
    auto refCount = k->r;
    EXPECT_EQ(refCount, 0) << "different initial ref count vs documentation?";

    q::K_ptr pk1{ k };
    EXPECT_EQ(k->r, refCount);
    EXPECT_EQ(pk1.get(), k);

    q::K_ptr pk2 = q::dup_K(pk1);
    ++refCount;
    EXPECT_EQ(k->r, refCount);
    EXPECT_EQ(pk2.get(), pk1.get());

    pk1.reset();
    --refCount;
    EXPECT_EQ(k->r, refCount);
    EXPECT_EQ(pk2.get(), k);

    q::K_ptr pk3;
    ASSERT_EQ(pk3.get(), q::Nil) << "fail to create empty K_ptr<>";
    pk2 = q::dup_K(pk3);
    EXPECT_EQ(pk2.get(), q::Nil);
}

#pragma endregion

TEST(KptrTests, dupKNil)
{
    q::K_ptr nil{ q::TypeTraits<q::kNil>::atom() };
    ASSERT_EQ(nil.get(), q::Nil) << "unexpected non-nil value";

    q::K_ptr pk;
    EXPECT_NO_THROW(pk = q::dup_K(nil));
    EXPECT_EQ(pk.get(), q::Nil);
}
