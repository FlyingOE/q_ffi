#include <gtest/gtest.h>
#include "kpointer.hpp"
#include "ktype_traits.hpp"

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
    static std::vector<typename Tr::value_type> samples_;

    void SetUp() override
    {
        extra_samples_for_null(q::has_null<Tr::type_id>());
        extra_samples_for_numeric(q::is_numeric<Tr::type_id>());
    }

    void extra_samples_for_null(std::false_type /*has_null*/)
    {}
    void extra_samples_for_null(std::true_type /*has_null*/)
    {
        this->samples_.push_back(Tr::null());
    }

    void extra_samples_for_numeric(std::false_type /*is_numeric*/)
    {}
    void extra_samples_for_numeric(std::true_type /*is_numeric*/)
    {
        this->samples_.insert(this->samples_.end(), {
            Tr::inf(),
            Tr::inf(false),
            std::numeric_limits<typename Tr::value_type>::min(),
            std::numeric_limits<typename Tr::value_type>::max()
        });
    }

    template<typename T>
    void expect_equal(::K const k, T&& v)
    {
        ASSERT_TRUE((std::is_same_v<std::decay_t<T>, typename Tr::value_type>));

        auto const bit_equal = [](auto&& actual, auto&& expected) {
            return sizeof(actual) == sizeof(expected) &&
                0 == std::memcmp(&actual, &expected, sizeof(actual));
        };
        // Use bit comparison because some values (e.g. NaN) are not comparable
        EXPECT_PRED2(bit_equal, Tr::value(k), std::forward<T>(v));
    }

    void expect_equal(::K const k, char const* str)
    {
        EXPECT_STREQ(Tr::value(k), str);
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

KPTR_TEST_SET(q::kTimestamp) = { "2020.01.01D00:00:00.000000000"_qp,
    "2020/9/10"_qp, "1997-11-23D12:34"_qp, 19700101'012345'678901234_qp,
    "1900-01-01D12:34:56.789"_qp
};
KPTR_TEST_SET(q::kMonth) = { "2000.01m"_qm, "2020/9"_qm,
    "1997-11"_qm, 197001_qm, "1900.1"_qm
};
KPTR_TEST_SET(q::kDate) = { "2000.01.01"_qd, "2020/9/10"_qd,
    "1997-11-28"_qd, 19700101_qd, "1900.1.1"_qd
};
KPTR_TEST_SET(q::kDatetime) = { "2000.01.01T00:00:00.000"_qz,
    "2020/9/10"_qz, "1997-11-23T12:34"_qz, 19700101012345678_qz,
    "1900.1.1T12:34:56.2"_qz
};
KPTR_TEST_SET(q::kTimespan) = { "00:00:00"_qn, "100D15:7:1.1"_qn,
    "-9:59:59.999999999n"_qn, 93000'001000000_qn, -135959'123456789_qn,
    "-42:01:60.012"_qn
};
KPTR_TEST_SET(q::kMinute) = { "00:00"_qu, "15:7"_qu,
    "-9:59u"_qu, 930_qu, -1359_qu, "-42:01"_qu
};
KPTR_TEST_SET(q::kSecond) = { "00:00:00"_qv, "15:7:1"_qv,
    "-9:59:59v"_qv, 93000_qv, -135959_qv, "-42:01:60"_qv
};
KPTR_TEST_SET(q::kTime) = { "00:00:00"_qt, "15:7:1.1"_qt,
    "-9:59:59.999t"_qt, 93000001_qt, -135959123_qt, "-42:01:60.012"_qt
};
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
    q::TypeTraits<q::kSymbol>,
    q::TypeTraits<q::kTimestamp>,
    q::TypeTraits<q::kMonth>,
    q::TypeTraits<q::kDate>,
    q::TypeTraits<q::kDatetime>,
    q::TypeTraits<q::kTimespan>,
    q::TypeTraits<q::kMinute>,
    q::TypeTraits<q::kSecond>,
    q::TypeTraits<q::kTime>
    //q::TypeTraits<q::kNil>
    //q::TypeTraits<q::kError>
>;

TYPED_TEST_SUITE(KptrTests, KptrTestTypes);

TYPED_TEST(KptrTests, makeK)
{
    for (auto const sample : this->samples_) {
        q::K_ptr pk{ TypeParam::atom(sample) };
        ASSERT_NE(pk.get(), q::Nil) << "Fail to create <" << TypeParam::type_id << "> K object";
        EXPECT_EQ(q::type(pk.get()), -TypeParam::type_id);
        SCOPED_TRACE("q::K_ptr c'tor result check");
        this->expect_equal(pk.get(), sample);
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

    pk1.reset(); --refCount; EXPECT_EQ(k->r, refCount);
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
