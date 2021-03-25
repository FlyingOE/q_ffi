#include <gtest/gtest.h>
#include "ktype_traits.hpp"
#include "kpointer.hpp"
#include <map>

using namespace std;

namespace q
{

#pragma region TypeTraitsTests<> typed test suite

    template<typename TraitsInfo>
    class TypeTraitsTests : public ::testing::Test
    {
    protected:
        template<typename Tr>
        void test_type(true_type /*has_value*/, bool notKError);
        template<typename Tr>
        void test_type(false_type /*has_value*/, bool)
        { ASSERT_FALSE(TraitsInfo::has_value); }

        template<typename Tr>
        void test_list_type(true_type /*can_index*/);
        template<typename Tr>
        void test_list_type(false_type /*can_index*/)
        { ASSERT_FALSE(TraitsInfo::can_index); }

        template<typename Tr>
        K_ptr init_atom(false_type /*convertible from (char const*)*/)
        { return K_ptr{ Tr::atom(typename Tr::value_type()) }; }
        template<typename Tr>
        K_ptr init_atom(true_type /*convertible from (char const*)*/)
        { return K_ptr{ Tr::atom("") }; }

        template<typename Tr>
        K_ptr init_list(false_type /*convertible from (char const*)*/)
        { return K_ptr{ Tr::list({ typename Tr::value_type() }) }; }
        template<typename Tr>
        K_ptr init_list(true_type /*convertible from (char const*)*/)
        { return K_ptr{ Tr::list({ "" }) }; }
    };

    template<typename Value,
        TypeId tid, char code, bool hasValue, bool hasNull, bool canIndex, bool isNumeric>
    struct TraitsInfo
    {
        using value_type = Value;
        static constexpr TypeId type_id = tid;
        static constexpr char type_code = code;
        static constexpr bool has_value = hasValue;
        static constexpr bool has_null = hasNull;
        static constexpr bool can_index = canIndex;
        static constexpr bool is_numeric = isNumeric;
    };
    using TraitsTestTypes = ::testing::Types<
        TraitsInfo<unsigned char, kBoolean, 'b', true, false, true, false>,
        TraitsInfo<uint8_t, kByte, 'x', true, true, true, false>,
        TraitsInfo<int16_t, kShort, 'h', true, true, true, true>,
        TraitsInfo<int32_t, kInt, 'i', true, true, true, true>,
        TraitsInfo<int64_t, kLong, 'j', true, true, true, true>,
        TraitsInfo<float, kReal, 'e', true, true, true, true>,
        TraitsInfo<double, kFloat, 'f', true, true, true, true>,
        TraitsInfo<char, kChar, 'c', true, true, true, false>,
        TraitsInfo<char const*, kSymbol, 's', true, true, true, false>,
        TraitsInfo<int64_t, kTimestamp, 'p', true, true, true, true>,
        TraitsInfo<int32_t, kMonth, 'm', true, true, true, true>,
        TraitsInfo<int32_t, kDate, 'd', true, true, true, true>,
        TraitsInfo<double, kDatetime, 'z', true, true, true, true>,
        TraitsInfo<int64_t, kTimespan, 'n', true, true, true, true>,
        TraitsInfo<int32_t, kMinute, 'u', true, true, true, true>,
        TraitsInfo<int32_t, kSecond, 'v', true, true, true, true>,
        TraitsInfo<int32_t, kTime, 't', true, true, true, true>,
        TraitsInfo<void, kNil, '\0', false, false, false, false>,
        TraitsInfo<char const*, kError, '\0', true, false, false, false>//,
        //TraitsInfo<void*, kDLL, '\0', true, false, false, false>
    >;

    TYPED_TEST_SUITE(TypeTraitsTests, TraitsTestTypes);

    TYPED_TEST(TypeTraitsTests, qTypeTraits)
    {
        using Traits = TypeTraits<TypeParam::type_id>;
        ASSERT_TRUE((is_same_v<typename Traits::value_type, typename TypeParam::value_type>));
        EXPECT_EQ(Traits::type_id, TypeId{ TypeParam::type_id });
        EXPECT_EQ(TypeId2Code.at(Traits::type_id), char{ TypeParam::type_code });
    }

    TYPED_TEST(TypeTraitsTests, qTypeTraitsQueries)
    {
        EXPECT_EQ(has_value_v<TypeParam::type_id>, bool{ TypeParam::has_value });
        EXPECT_EQ(has_null_v<TypeParam::type_id>, bool{ TypeParam::has_null });
        EXPECT_EQ(can_index_v<TypeParam::type_id>, bool{ TypeParam::can_index });
        EXPECT_EQ(is_numeric_v<TypeParam::type_id>, bool{ TypeParam::is_numeric });
    }

    template<typename TraitsInfo>
    template<typename Tr>
    void TypeTraitsTests<TraitsInfo>::test_type(true_type /*has_value*/, bool notKError)
    {
        K_ptr k = init_atom<Tr>(is_convertible<char const*, typename Tr::value_type>());
        if (notKError) {
            ASSERT_NE(k.get(), nullptr)
                << "TypeTraits<" << Tr::type_id << ">::atom() failed";
            EXPECT_EQ(type(k.get()), -Tr::type_id);
        }
        else {
            ASSERT_EQ(k.get(), nullptr)
                << "TypeTraits<" << Tr::type_id << ">::atom() should not return non-null error";
        }
    }

    template<typename TraitsInfo>
    template<typename Tr>
    void TypeTraitsTests<TraitsInfo>::test_list_type(true_type /*can_index*/)
    {
        K_ptr k = init_list<Tr>(integral_constant<bool,
            is_convertible_v<char const*, typename Tr::value_type>>());
        ASSERT_NE(k.get(), nullptr)
            << "TypeTraits<" << Tr::type_id << ">::list() failed";
        EXPECT_EQ(type(k.get()), Tr::type_id);
    }

    TYPED_TEST(TypeTraitsTests, qType)
    {
        using Traits = TypeTraits<TypeParam::type_id>;
        {
            SCOPED_TRACE("q::type() for atoms");
            this->template test_type<Traits>(
                has_value<Traits::type_id>(), kError != TypeParam::type_id);
        }
        {
            SCOPED_TRACE("q::type() for lists");
            this->template test_list_type<Traits>(
                can_index<Traits::type_id>());
        }
    }

#pragma endregion

#pragma region TypeTraitsOpsTests<> typed test suite

    template<typename TraitsInfo>
    class TypeTraitsOpsTests : public ::testing::Test
    {
    protected:
        static map<typename TraitsInfo::value_type, string> const samples_;

        template<typename T>
        void expect_equal(T const& actual, T const& expected)
        {
            // Some values are not comparable (e.g. NaN), use bit comparison
            auto const bit_equal = [](auto&& actual, auto&& expected) -> bool {
                return 0 == memcmp(&actual, &expected, sizeof(T));
            };
            EXPECT_PRED2(bit_equal, actual, expected);
        }

        void expect_equal(char const* actual, char const* expected)
        {
            EXPECT_STREQ(actual, expected);
        }
    };

#   define OPS_TEST_SET(tid)   \
        template<>  \
        map<typename TypeTraits<(tid)>::value_type, string> const  \
        TypeTraitsOpsTests<TypeTraits<(tid)>>::samples_

    using namespace std::literals;
    using namespace literals;

    OPS_TEST_SET(kBoolean) = {
        { 1_qb, "1b" },
        { 0_qb, "0b" }
    };
    OPS_TEST_SET(kByte) = {
        { 0_qx, "00" },
        { 0x20_qx, "20" },
        { 0xA7_qx, "a7" },
        { 0xFF_qx, "ff" }
    };
    OPS_TEST_SET(kShort) = {
        { 0_qh, "0h" },
        { 129_qh, "129h" },
        { TypeTraits<kShort>::value_type{ -128 }, "-128h" },
        { TypeTraits<kShort>::null(), "0Nh" },
        { TypeTraits<kShort>::inf(), "0Wh" },
        { TypeTraits<kShort>::inf(false), "-0Wh" }
    };
    OPS_TEST_SET(kInt) = {
        { 0_qi, "0i" },
        { 65536_qi, "65536i" },
        { -32768_qi, "-32768i" },
        { TypeTraits<kInt>::null(), "0Ni" },
        { TypeTraits<kInt>::inf(), "0Wi" },
        { TypeTraits<kInt>::inf(false), "-0Wi" }
    };
    OPS_TEST_SET(kLong) = {
        { 0_qj, "0j" },
        { 4'294'967'296_qj, "4294967296j" },
        { -2'147'483'648_qj, "-2147483648j" },
        { TypeTraits<kLong>::null(), "0Nj" },
        { TypeTraits<kLong>::inf(), "0Wj" },
        { TypeTraits<kLong>::inf(false), "-0Wj" }
    };
    OPS_TEST_SET(kReal) = {
        { 0._qe, "0.000000e" },
        { 987.654_qe, "987.653992e" },
        { -123.456_qe, "-123.456001e" },
        { TypeTraits<kReal>::null(), "0Ne" },
        { TypeTraits<kReal>::inf(), "0We" },
        { TypeTraits<kReal>::inf(false), "-0We" }
    };
    OPS_TEST_SET(kFloat) = {
        { 0._qf, "0.000000f" },
        { 987.6543210123_qf, "987.654321f" },
        { -123.4567890987_qf, "-123.456789f" },
        { TypeTraits<kFloat>::null(), "0Nf" },
        { TypeTraits<kFloat>::inf(), "0Wf" },
        { TypeTraits<kFloat>::inf(false), "-0Wf" }
    };
    OPS_TEST_SET(kChar) = {
        { '\0', "\0"s },
        { 'Z', "Z"s },
        { '\xFF', "\xFF"s },
        { TypeTraits<kChar>::null(), " "s }
    };
    OPS_TEST_SET(kSymbol) = {
        { "600000.SH", "`600000.SH"s },
        { "123 abc ABC", "`123 abc ABC"s },
        { u8"测试", u8"`测试"s },
        { TypeTraits<kSymbol>::null(), "`"s }
    };
    OPS_TEST_SET(kTimestamp) = {
        { "2000.01.01D00:00:00.000000000"_qp, "2000.01.01D00:00:00.000000000"s },
        { "2020/9/10"_qp, "2020.09.10D00:00:00.000000000"s },
        { "1997-11-28D12:34"_qp, "1997.11.28D12:34:00.000000000"s },
        { 19700101'012345'678901234_qp, "1970.01.01D01:23:45.678901234"s },
        { "1900.1.1D12:34:56.2"_qp, "1900.01.01D12:34:56.200000000"s },
        { TypeTraits<kTimestamp>::null(), "0Np"s },
        { TypeTraits<kTimestamp>::inf(), "0Wp"s },
        { TypeTraits<kTimestamp>::inf(false), "-0Wp"s }
    };
    OPS_TEST_SET(kMonth) = {
        { "2000.01m"_qm, "2000.01m"s },
        { "2020/9"_qm, "2020.09m"s },
        { "1997-11"_qm, "1997.11m"s },
        { 197001_qm, "1970.01m"s },
        { "1900.1"_qm, "1900.01m"s },
        { "2038.2"_qm, "2038.02m"s },
        { TypeTraits<kMonth>::null(), "0Nm"s },
        { TypeTraits<kMonth>::inf(), "0Wm"s },
        { TypeTraits<kMonth>::inf(false), "-0Wm"s }
    };
    OPS_TEST_SET(kDate) = {
        { "2000.01.01"_qd, "2000.01.01"s },
        { "2020/9/10"_qd, "2020.09.10"s },
        { "1997-11-28"_qd, "1997.11.28"s },
        { 19700101_qd, "1970.01.01"s },
        { "19000101"_qd, "1900.01.01"s },
        { TypeTraits<kDate>::null(), "0Nd"s },
        { TypeTraits<kDate>::inf(), "0Wd"s },
        { TypeTraits<kDate>::inf(false), "-0Wd"s }
    };
    OPS_TEST_SET(kDatetime) = {
        { "2000.01.01T00:00:00.000"_qz, "2000.01.01T00:00:00.000"s },
        { "2020/9/10"_qz, "2020.09.10T00:00:00.000"s },
        { "1997-11-28T12:34"_qz, "1997.11.28T12:34:00.000"s },
        { 19700101012345678_qz, "1970.01.01T01:23:45.678"s },
        { "1900.1.1T12:34:56.2"_qz, "1900.01.01T12:34:56.200"s },
        { TypeTraits<kDatetime>::null(), "0Nz"s },
        { TypeTraits<kDatetime>::inf(), "0Wz"s },
        { TypeTraits<kDatetime>::inf(false), "-0Wz"s }
    };
    OPS_TEST_SET(kTimespan) = {
        { "00:00:00"_qn, "0D00:00:00.000000000"s },
        { "100D15:7:1.1"_qn, "100D15:07:01.100000000"s },
        { "-9:59:59.999999999n"_qn, "-0D09:59:59.999999999"s },
        { 93000'001000000_qn, "0D09:30:00.001000000"s },
        { -135959'123456789_qn, "-0D13:59:59.123456789"s },
        { "-42:01:60.012"_qn, "-1D18:02:00.012000000"s },
        { TypeTraits<kTimespan>::null(), "0Nn"s },
        { TypeTraits<kTimespan>::inf(), "0Wn"s },
        { TypeTraits<kTimespan>::inf(false), "-0Wn"s }
    };
    OPS_TEST_SET(kMinute) = {
        { "00:00"_qu, "00:00"s },
        { "15:7"_qu, "15:07"s },
        { "-9:59u"_qu, "-09:59"s },
        { 930_qu, "09:30"s },
        { -1359_qu, "-13:59"s },
        { "-42:01"_qu, "-42:01"s },
        { TypeTraits<kMinute>::null(), "0Nu"s },
        { TypeTraits<kMinute>::inf(), "0Wu"s },
        { TypeTraits<kMinute>::inf(false), "-0Wu"s }
    };
    OPS_TEST_SET(kSecond) = {
        { "00:00:00"_qv, "00:00:00"s },
        { "15:7:1"_qv, "15:07:01"s },
        { "-9:59:59v"_qv, "-09:59:59"s },
        { 93000_qv, "09:30:00"s },
        { -135959_qv, "-13:59:59"s },
        { "-42:01:60"_qv, "-42:02:00"s },
        { TypeTraits<kSecond>::null(), "0Nv"s },
        { TypeTraits<kSecond>::inf(), "0Wv"s },
        { TypeTraits<kSecond>::inf(false), "-0Wv"s }
    };
    OPS_TEST_SET(kTime) = {
        { "00:00:00"_qt, "00:00:00.000"s },
        { "15:7:1.1"_qt, "15:07:01.100"s },
        { "-9:59:59.999t"_qt, "-09:59:59.999"s },
        { 93000001_qt, "09:30:00.001"s },
        { -135959123_qt, "-13:59:59.123"s },
        { "-42:01:60.012"_qt, "-42:02:00.012"s },
        { TypeTraits<kTime>::null(), "0Nt"s },
        { TypeTraits<kTime>::inf(), "0Wt"s },
        { TypeTraits<kTime>::inf(false), "-0Wt"s }
    };

    using TraitsOpsTestTypes = ::testing::Types<
        TypeTraits<kBoolean>,
        TypeTraits<kByte>,
        TypeTraits<kShort>,
        TypeTraits<kInt>,
        TypeTraits<kLong>,
        TypeTraits<kReal>,
        TypeTraits<kFloat>,
        TypeTraits<kChar>,
        TypeTraits<kSymbol>,
        TypeTraits<kTimestamp>,
        TypeTraits<kMonth>,
        TypeTraits<kDate>,
        TypeTraits<kDatetime>,
        TypeTraits<kTimespan>,
        TypeTraits<kMinute>,
        TypeTraits<kSecond>,
        TypeTraits<kTime>
        //TypeTraits<kNil>
        //TypeTraits<kError>
    >;

    TYPED_TEST_SUITE(TypeTraitsOpsTests, TraitsOpsTestTypes);

    TYPED_TEST(TypeTraitsOpsTests, atomAndValue)
    {
        using Traits = TypeTraits<TypeParam::type_id>;

        for (auto const& sample : this->samples_) {
            K_ptr k{ Traits::atom(sample.first) };
            ASSERT_NE(k.get(), Nil);
            EXPECT_EQ(type(k.get()), -TypeParam::type_id);

            SCOPED_TRACE("literal for atom");
            this->expect_equal(Traits::value(k.get()), sample.first);
        }
    }

    TYPED_TEST(TypeTraitsOpsTests, listAndIndex)
    {
        using Traits = TypeTraits<TypeParam::type_id>;

        vector<typename TypeParam::value_type> values(this->samples_.size());
        transform(this->samples_.cbegin(), this->samples_.cend(), values.begin(),
            [](auto const& sample) { return sample.first; });

        K_ptr k{ Traits::list(values.cbegin(), values.cend()) };
        ASSERT_NE(k.get(), Nil);
        EXPECT_EQ(type(k.get()), TypeParam::type_id);
        ASSERT_EQ(count(k.get()), values.size());

        auto s = values.cbegin();
        auto const e = values.cend();
        auto p = Traits::index(k.get());
        ASSERT_NE(p, nullptr);
        for (; s != e; ++p, ++s) {
            SCOPED_TRACE("literal list");
            this->expect_equal(*p, *s);
        }
    }

    TEST(TypeTraitsOpsTests, kCharList)
    {
        using Traits = TypeTraits<kChar>;
        char const sample[] = u8"ABC 123 测试\0+-/";

        auto str_check = [&sample](K_ptr k, size_t length) {
            ASSERT_NE(k.get(), Nil);
            EXPECT_EQ(type(k.get()), Traits::type_id);
            ASSERT_EQ(count(k.get()), length);
            for (size_t i = 0; i < length; ++i) {
                EXPECT_EQ(Traits::index(k.get())[i], sample[i]);
            }
        };

        size_t const strlen = strlen(sample);
        str_check(K_ptr{ Traits::list(sample) }, strlen);

        size_t const length = extent_v<decltype(sample), 0> - 1;   // less trailing '\0'
        str_check(K_ptr{ Traits::list(sample, length) }, length);
    }

    TEST(TypeTraitsOpsTests, kSymbolList)
    {
        using Traits = TypeTraits<kSymbol>;
        vector<string> sample{
            "600000.SH"s,
            "123 abc ABC"s,
            "  abc\0ABC"s,
            u8"测试"s,
            "\0"s,
            Traits::null()
        };
        size_t const length = sample.size();

        K_ptr k{ Traits::list(sample.cbegin(), sample.cend()) };
        ASSERT_NE(k.get(), Nil);
        EXPECT_EQ(type(k.get()), Traits::type_id);
        ASSERT_EQ(count(k.get()), length);
        for (size_t i = 0; i < length; ++i) {
            EXPECT_STREQ(Traits::index(k.get())[i], sample[i].c_str());
        }
    }

    TYPED_TEST(TypeTraitsOpsTests, toStr)
    {
        using Traits = TypeTraits<TypeParam::type_id>;

        for (auto const& sample : this->samples_) {
            EXPECT_EQ(Traits::to_str(sample.first), sample.second);
        }
    }

    TYPED_TEST(TypeTraitsOpsTests, toString)
    {
        using Traits = TypeTraits<TypeParam::type_id>;

        for (auto const& sample : this->samples_) {
            K_ptr k{ Traits::atom(sample.first) };
            EXPECT_EQ(to_string(k.get()), sample.second);
        }
    }

#pragma endregion

}//namespace q
