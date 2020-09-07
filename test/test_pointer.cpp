#include <gtest/gtest.h>
#include "pointer.hpp"
#include "types.hpp"

TEST(KptrTests, RefCount)
{
    q::K_ptr pk = q::TypeTraits<q::kSymbol>::atom(test_info_->name());
    ASSERT_NE(pk.get(), q::Nil) << "fail to create K object";

    K k = pk.get();
    auto refCount = k->r;
    EXPECT_EQ(refCount, 0) << "different initial ref count vs documentation?";

    q::K_ptr pk1{ k };
    EXPECT_EQ(k->r, refCount);

    for (auto i : { 1, 1 }) { r1(k); refCount += i; }
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

    using Traits = Tr;
    using value_type = typename Traits::value_type;

    static std::vector<value_type> tests_;

    KptrTests() : Test()
    {
        add_tests_for_null(q::has_null<Traits::id>());
        add_tests_for_numeric(q::is_numeric<Traits::id>());
    }

    void test_make_K(value_type const& v)
    {
        q::K_ptr pk{ Traits::atom(v) };
        ASSERT_NE(pk.get(), q::Nil) << "Fail to create <" << Traits::id << "> K object";
        EXPECT_EQ(q::typeOf(pk), -Traits::id);
        checkEqualAtoms(pk, v);
    }

private:

    void add_tests_for_null(std::false_type) {}
    void add_tests_for_null(std::true_type)
    {
        tests_.push_back(Traits::null());
    }

    void add_tests_for_numeric(std::false_type) {}
    void add_tests_for_numeric(std::true_type)
    {
        tests_.insert(std::end(tests_), {
            Traits::inf(),
            std::numeric_limits<value_type>::min(),
            std::numeric_limits<value_type>::max()
        });
    }

    template<typename = std::enable_if_t<!std::is_same_v<value_type, char const*>>>
    void checkEqualAtoms(q::K_ptr const& pk, value_type const& v)
    {
        auto const memory_equal = [](auto&& actual, auto&& expected) {
            return sizeof(actual) == sizeof(expected) &&
                0 == std::memcmp(&actual, &expected, sizeof(actual));
        };
        // Use memory comparison because some values (e.g. NaN) are not comparable
        EXPECT_PRED2(memory_equal, Traits::value(pk.get()), v);
    }

    void checkEqualAtoms(q::K_ptr const& pk, char const* str)
    {
        EXPECT_STREQ(Traits::value(pk.get()), str);
    }
};

#define KPTR_TEST_PARAMS(qType, cppType)    \
    template<>  \
    std::vector<cppType>    \
    KptrTests<q::TypeTraits<qType>>::tests_

KPTR_TEST_PARAMS(q::kBoolean, bool) = { true, false };
KPTR_TEST_PARAMS(q::kByte, uint8_t) = { 0, 0x20, 0xFF };
KPTR_TEST_PARAMS(q::kShort, int16_t) = { 0, 129, -128 };
KPTR_TEST_PARAMS(q::kInt, int32_t) = { 0, 65536, -32768 };
KPTR_TEST_PARAMS(q::kLong, int64_t) = { 0, 4'294'967'296LL, -2'147'483'648LL };
KPTR_TEST_PARAMS(q::kReal, float) = { 0.f, 987.654f, -123.456f };
KPTR_TEST_PARAMS(q::kFloat, double) = { 0., 987.6543210123, -123.4567890987 };
KPTR_TEST_PARAMS(q::kChar, char) = { '\0', 'Z', '\xFF' };
KPTR_TEST_PARAMS(q::kSymbol, char const*) = { "600000.SH", "123 abc ABC", "≤‚ ‘" };
//KPTR_TEST_PARAMS(q::kNil, void) = //<q::kNil> cannot be `created'!
//KPTR_TEST_PARAMS(q::kError, char const*) = //<q::kError> cannot be `created'!

using TestTypes = ::testing::Types <
    q::TypeTraits<q::kBoolean>, q::TypeTraits<q::kByte>,
    q::TypeTraits<q::kShort>, q::TypeTraits<q::kInt>, q::TypeTraits<q::kLong>,
    q::TypeTraits<q::kReal>, q::TypeTraits<q::kFloat>,
    q::TypeTraits<q::kChar>, q::TypeTraits<q::kSymbol>
    //q::TypeTraits<q::kNil>
    //q::TypeTraits<q::kError>
>;
TYPED_TEST_SUITE(KptrTests, TestTypes);

#pragma endregion

TYPED_TEST(KptrTests, makeK)
{
    for (auto const test : tests_) {
        test_make_K(test);
    }
}

TEST(KptrTests, dupK)
{
    q::K_ptr pk = q::TypeTraits<q::kSymbol>::atom(test_info_->name());
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

TEST(KptrTests, dupKNil)
{
    q::K_ptr nil{ q::TypeTraits<q::kNil>::atom() };
    ASSERT_EQ(nil.get(), q::Nil) << "unexpected non-nil value";

    q::K_ptr pk;
    EXPECT_NO_THROW(pk = q::dup_K(nil));
    EXPECT_EQ(pk.get(), q::Nil);
}
