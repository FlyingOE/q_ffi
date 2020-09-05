#include <gtest/gtest.h>
#include "pointer.hpp"
#include "types.hpp"

TEST(KptrTests, RefCount)
{
    K k = q::TypeTraits<q::kSymbol>::atom(test_info_->name());
    ASSERT_NE(k, q::Nil) << "fail to create K object";
    auto refCount = k->r;
    EXPECT_EQ(refCount, 0) << "different initial ref count vs documentation?";

    q::K_ptr pk1{ k };
    EXPECT_EQ(k->r, refCount) << "creating K_ptr<> should not change ref count";

    for (auto i : { 1, 1 }) { r1(k); refCount += i; }
    ASSERT_EQ(k->r, refCount) << "r1() fail to increment ref count";
    {
        q::K_ptr pk2;
        EXPECT_EQ(pk2.get(), q::Nil) << "empty K_ptr<> cannot be properly created";
        pk2.reset(k);
        EXPECT_EQ(k->r, refCount) << "setting K_ptr<> should not change ref count";
        pk2.release();
        EXPECT_EQ(k->r, refCount) << "releasing K_ptr<> should not change ref count";
        pk2.reset(k);
        EXPECT_EQ(k->r, refCount) << "setting K_ptr<> again should not change ref count";
        pk2.reset();
        --refCount;
        EXPECT_EQ(k->r, refCount) << "resetting K_ptr<> should decrement ref count";

        pk2.reset(k);
        q::K_ptr pk3 = std::move(pk2);
        EXPECT_EQ(k->r, refCount) << "moving K_ptr<> should not change ref count";
    }
    --refCount;
    EXPECT_EQ(k->r, refCount) << "destructing K_ptr<> should decrement ref count";
}

template<typename Tr>
class KptrTests : public ::testing::Test
{
protected:
    using Traits = Tr;
    using value_type = typename Traits::value_type;

private:
    static std::vector<value_type> const tests_;

    static bool memory_compare(value_type const& actual, value_type const& expected)
    {
        return sizeof(actual) == sizeof(expected) &&
            0 == std::memcmp(&actual, &expected, sizeof(actual));
    };

    template<typename = std::enable_if_t<!std::is_same_v<value_type, char const*>>>
    void checkEqualAtoms(q::K_ptr const& pk, value_type const& v)
    {
        // Use memory comparison because some values (e.g. NaN) are not comparable
        EXPECT_PRED2(memory_compare, Traits::value(pk.get()), v)
            << "unexpected different K object values (" << Traits::id << ')';
    }

    void checkEqualAtoms(q::K_ptr const& pk, char const* str)
    {
        EXPECT_STREQ(Traits::value(pk.get()), str)
            << "unexpected different K object strings (" << Traits::id << ')';
    }

    template<typename T>
    void test_make_K(T&& v)
    {
        ASSERT_TRUE((std::is_same_v<value_type, std::decay_t<T>>)) << "<BUG> invalid test data type";

        auto pk = q::make_K<Traits::id>(std::forward<T>(v));
        ASSERT_NE(pk.get(), q::Nil) << "q::make_K<" << Traits::id << ">() fails to create K object";
        EXPECT_EQ(q::typeOf(pk), Traits::id) << "q::make_K<" << Traits::id << ">() creates K object of the wrong type";
        checkEqualAtoms(pk, v);
    }

protected:

    void test_samples()
    {
        for (auto const test : tests_)
            test_make_K(test);
    }

    void test_null(std::false_type) {}
    void test_null(std::true_type)
    {
        test_make_K(Traits::null());
    }

    void test_numeric(std::false_type) {}
    void test_numeric(std::true_type)
    {
        test_make_K(Traits::inf());
        test_make_K(std::numeric_limits<value_type>::min());
        test_make_K(std::numeric_limits<value_type>::max());
    }
};

#define KPTR_TEST_PARAMS(qType, cppType, ...)   \
    template<>  \
    std::vector<cppType> const  \
    KptrTests<q::TypeTraits<qType>>::tests_ =   \
        { __VA_ARGS__ }

KPTR_TEST_PARAMS(q::kBoolean, bool,
    true, false);
KPTR_TEST_PARAMS(q::kByte, char,
    0, 0x20, -127);
KPTR_TEST_PARAMS(q::kShort, short,
    0, 129, -128);
KPTR_TEST_PARAMS(q::kInt, int32_t,
    0, 65536, -32768);
KPTR_TEST_PARAMS(q::kLong, int64_t,
    0, 4294967296LL, -2147483648LL);
KPTR_TEST_PARAMS(q::kReal, float,
    0.f, 987.654f, -123.456f);
KPTR_TEST_PARAMS(q::kFloat, double,
    0., 987.6543210123, -123.4567890987);
KPTR_TEST_PARAMS(q::kChar, char,
    '\0', 'Z', '\xFF');
KPTR_TEST_PARAMS(q::kSymbol, char const*,
    "600000.SH", "123 abc ABC", "≤‚ ‘");
//KPTR_TEST_PARAMS(q::kNil, void,
//    );    //<q::kNil> cannot be `created'!
//KPTR_TEST_PARAMS(q::kError, char const*,
//    );    //<q::kError> cannot be `created'!   

using TestTypes = ::testing::Types <
    q::TypeTraits<q::kBoolean>, q::TypeTraits<q::kByte>,
    q::TypeTraits<q::kShort>, q::TypeTraits<q::kInt>, q::TypeTraits<q::kLong>,
    q::TypeTraits<q::kReal>, q::TypeTraits<q::kFloat>,
    q::TypeTraits<q::kChar>, q::TypeTraits<q::kSymbol>
    //q::TypeTraits<q::kNil>
    //q::TypeTraits<q::kError>
>;
TYPED_TEST_SUITE(KptrTests, TestTypes);

TYPED_TEST(KptrTests, makeK)
{
    test_samples();
    test_null(q::has_null<Traits::id>());
    test_numeric(q::is_numeric<Traits::id>());
}

TEST(KptrTests, dupK)
{
    K k = q::TypeTraits<q::kSymbol>::atom(test_info_->name());
    ASSERT_NE(k, q::Nil) << "fail to create K object";
    auto refCount = k->r;
    EXPECT_EQ(refCount, 0) << "different initial ref count vs documentation?";

    q::K_ptr pk1{ k };
    EXPECT_EQ(k->r, refCount) << "creating K_ptr<> should not change ref count";
    EXPECT_EQ(pk1.get(), k) << "creating K_ptr<> should point to the original K object";

    q::K_ptr pk2 = q::dup_K(pk1);
    ++refCount;
    EXPECT_EQ(k->r, refCount) << "duplicating K_ptr<> should increment ref count";
    EXPECT_EQ(pk2.get(), pk1.get()) << "duplicated K_ptr<> should point to the same K object";

    pk1.reset();
    --refCount;
    EXPECT_EQ(k->r, refCount) << "releasing a duplicated K_ptr<> should decrement ref count";
    EXPECT_EQ(pk2.get(), k) << "releasing a duplicated K_ptr<> should not affect other copies";

    q::K_ptr pk3;
    ASSERT_EQ(pk3.get(), q::Nil) << "fail to create empty K_ptr<>";
    pk2 = q::dup_K(pk3);
    EXPECT_EQ(pk2.get(), q::Nil) << "duplicate of an empty K_ptr<> should be empty, too";
}
