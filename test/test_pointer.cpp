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

namespace {

    template<q::Type tid, typename T>
    void checkEqualAtoms(q::K_ptr const& pk, T const& v)
    {
        using Traits = q::TypeTraits<tid>;

        auto memory_compare = [](auto const actual, auto const expected)
        {
            return sizeof(actual) == sizeof(expected) &&
                0 == std::memcmp(&actual, &expected, sizeof(actual));
        };
        // Use memory comparison because some values (e.g. NaN) are not comparable
        EXPECT_PRED2(memory_compare, Traits::value(pk.get()), v)
            << "unexpected different K object values (" << tid << ')';
    }

    template<q::Type tid>
    void checkEqualAtoms(q::K_ptr const& pk, char const* str)
    {
        using Traits = q::TypeTraits<tid>;

        EXPECT_STREQ(Traits::value(pk.get()), str)
            << "unexpected different K object strings (" << tid << ')';
    }

    template<q::Type = q::kNil>
    void checkEqualAtoms(q::K_ptr const& pk, void* /*ignore*/) {}

    template<q::Type tid, typename T>
    void test_make_K(T&& v)
    {
        using Traits = q::TypeTraits<tid>;
        auto pk = q::make_K<tid>(std::forward<T>(v));
        ASSERT_NE(pk.get(), q::Nil) << "q::make_K<" << tid << ">() fails to create K object";
        EXPECT_EQ(q::typeOf(pk), tid) << "q::make_K<" << tid << ">() creates K object of the wrong type";
        checkEqualAtoms<tid>(pk, v);
    }

}//namespace <anonymous>

TEST(KptrTests, makeK)
{
    {
        using Traits = q::TypeTraits<q::kBoolean>;
        test_make_K<Traits::id>(true);
        test_make_K<Traits::id>(false);
    }
    {
        using Traits = q::TypeTraits<q::kByte>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>(static_cast<char>(0x20));
        test_make_K<Traits::id>(static_cast<char>(0xFF));
    }
    {
        using Traits = q::TypeTraits<q::kShort>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>(Traits::inf());
        test_make_K<Traits::id>((short)0);
        test_make_K<Traits::id>((short)-128);
        test_make_K<Traits::id>(std::numeric_limits<short>::min());
        test_make_K<Traits::id>(std::numeric_limits<short>::max());
    }
    {
        using Traits = q::TypeTraits<q::kInt>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>(Traits::inf());
        test_make_K<Traits::id>(0);
        test_make_K<Traits::id>(-32768);
        test_make_K<Traits::id>(std::numeric_limits<int32_t>::min());
        test_make_K<Traits::id>(std::numeric_limits<int32_t>::max());
    }
    {
        using Traits = q::TypeTraits<q::kLong>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>(Traits::inf());
        test_make_K<Traits::id>(0LL);
        test_make_K<Traits::id>(-2147483648LL);
        test_make_K<Traits::id>(std::numeric_limits<int64_t>::min());
        test_make_K<Traits::id>(std::numeric_limits<int64_t>::max());
    }
    {
        using Traits = q::TypeTraits<q::kReal>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>(Traits::inf());
        test_make_K<Traits::id>(0.f);
        test_make_K<Traits::id>(-123.456f);
        test_make_K<Traits::id>(std::numeric_limits<float>::min());
        test_make_K<Traits::id>(std::numeric_limits<float>::max());
    }
    {
        using Traits = q::TypeTraits<q::kFloat>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>(Traits::inf());
        test_make_K<Traits::id>(0.);
        test_make_K<Traits::id>(-123.456789012);
        test_make_K<Traits::id>(std::numeric_limits<double>::min());
        test_make_K<Traits::id>(std::numeric_limits<double>::max());
    }
    {
        using Traits = q::TypeTraits<q::kChar>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>('\0');
        test_make_K<Traits::id>('Z');
    }
    {
        using Traits = q::TypeTraits<q::kSymbol>;
        test_make_K<Traits::id>(Traits::null());
        test_make_K<Traits::id>("600000.SH");
        test_make_K<Traits::id>("≤‚ ‘");
    }
    {
        using Traits = q::TypeTraits<q::kNil>;
        // <q::kNil> cannot be created with q::make_K<>()
    }
    {
        using Traits = q::TypeTraits<q::kSymbol>;
        test_make_K<Traits::id>("(default) non-system error");
    }
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
