#include <gtest/gtest.h>
#include "types.hpp"
#include <map>

#pragma region TemporalTraitsTests<> typed test suite

template<typename TraitsInfo>
class TemporalTraitsTests : public ::testing::Test
{
protected:
    using value_type = typename TraitsInfo::value_type;
    static std::map<value_type, value_type> const temporals_;
};

#define TEMPORAL_TEST_SET(tid)   \
    template<>  \
    std::map<typename q::TypeTraits<(tid)>::value_type, typename q::TypeTraits<(tid)>::value_type> const  \
    TemporalTraitsTests<q::TypeTraits<(tid)>>::temporals_

using namespace q::literals;

TEMPORAL_TEST_SET(q::kTimestamp) = {
    { "2000.01.01D00:00:00.000000000"_qp, 0 },
    { "2020/9/10D15:7:1.012345678"_qp, 653065621012345678LL },
    { "1997-11-28D9:59:59.1p"_qp, -65973600900000000LL },
    { 19700101012345678900000_qp, -946679774321100000LL },
    { 21980808'093000'000000001_qp, 6267317400000000001LL },
    { "1970.01.01D01:23:45.6789"_qp, -946679774321100000LL },
    { "2198/08/08D09:30:00.000000001"_qp, 6267317400000000001LL },
    { "2020.09.11D24:01"_qp, 653184060000000000LL },
    { "1900-1-1D23:45:7.999999999"_qp, -3155588092000000001LL },
    { "abc"_qp, q::TypeTraits<q::kLong>::null() },
    { "1998-08-08D12:01:"_qp, q::TypeTraits<q::kLong>::null() },
    { "1999/12/31D12:01:23."_qp, q::TypeTraits<q::kLong>::null() },
    { "2000.01.01T12:01:23"_qp, q::TypeTraits<q::kLong>::null() },
    { q::TypeTraits<q::kTimestamp>::null(), q::TypeTraits<q::kLong>::null() },
    { q::TypeTraits<q::kTimestamp>::inf(), q::TypeTraits<q::kLong>::inf() },
    { -q::TypeTraits<q::kTimestamp>::inf(), -q::TypeTraits<q::kLong>::inf() }
};
TEMPORAL_TEST_SET(q::kMonth) = {
    { "2000.01m"_qm, 0 },
    { "2020/9"_qm, 248 },
    { "1997-11"_qm, -26 },
    { 197001_qm, -360 },
    { "1900.1"_qm, -1200 },
    { 199912_qm, -1 },
    { "abc"_qm, q::TypeTraits<q::kInt>::null() },
    { "2000."_qm, q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kMonth>::null(), q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kMonth>::inf(), q::TypeTraits<q::kInt>::inf() },
    { -q::TypeTraits<q::kMonth>::inf(), -q::TypeTraits<q::kInt>::inf() }
};
TEMPORAL_TEST_SET(q::kDate) = {
    { "2000.01.01"_qd, 0 },
    { "2020/9/10"_qd, 7558 },
    { "1997-11-28"_qd, -764 },
    { 19700101_qd, -10957 },
    { "1900.1.1"_qd, -36524 },
    { "19991231"_qd, -1 },
    { "abc"_qd, q::TypeTraits<q::kInt>::null() },
    { "2000.01"_qm, q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kDate>::null(), q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kDate>::inf(), q::TypeTraits<q::kInt>::inf() },
    { -q::TypeTraits<q::kDate>::inf(), -q::TypeTraits<q::kInt>::inf() }
};
TEMPORAL_TEST_SET(q::kDatetime) = {
    { "2000.01.01T00:00:00.000"_qz, 0. },
    { "2020/9/10T15:7:1.012"_qz, 7558.6298728240745 },
    { "1997-11-28T9:59:59.1z"_qz, -763.58334375000004 },
    { 19700101'012345'678_qz, -10956.941832430555 },
    { 21980808'093000'000_qz, 72538.395833333328 },
    { "2020.09.11T24:01:60.012"_qz, 7560.0013890277778 },
    { "1900-1-1T23:45:7.999"_qz, -36523.010324085648 },
    { "abc"_qz, q::TypeTraits<q::kFloat>::null() },
    { "1998-08-08T12:01:"_qz, q::TypeTraits<q::kFloat>::null() },
    { "1999/12/31T12:01:23."_qz, q::TypeTraits<q::kFloat>::null() },
    { "2000.01.01P12:01:23"_qz, q::TypeTraits<q::kFloat>::null() },
    { q::TypeTraits<q::kDatetime>::null(), q::TypeTraits<q::kFloat>::null() },
    { q::TypeTraits<q::kDatetime>::inf(), q::TypeTraits<q::kFloat>::inf() },
    { -q::TypeTraits<q::kDatetime>::inf(), -q::TypeTraits<q::kFloat>::inf() }
};
TEMPORAL_TEST_SET(q::kTimespan) = {
    { "0D00:00:00.000000000"_qn, 0LL },
    { "100D15:7:1.01234"_qn, 8694421'012'340'000LL },
    { "-1D9:59:59.1n"_qn, -122399'100'000'000LL },
    { 93000'000'000'000_qn, 34200'000'000'000LL },
    { -375959'000'000'001_qn, -136799'000'000'001LL },
    { "-42:01:60.012"_qn, -151320'012'000'000LL },
    { "123:45:67.987654321"_qn, 445567'987654321LL },
    { "abc"_qn, q::TypeTraits<q::kLong>::null() },
    { "12:01:"_qn, q::TypeTraits<q::kLong>::null() },
    { "12:01:23."_qn, q::TypeTraits<q::kLong>::null() },
    { "1P12:01:23"_qn, q::TypeTraits<q::kLong>::null() },
    { q::TypeTraits<q::kTimespan>::null(), q::TypeTraits<q::kLong>::null() },
    { q::TypeTraits<q::kTimespan>::inf(), q::TypeTraits<q::kLong>::inf() },
    { -q::TypeTraits<q::kTimespan>::inf(), -q::TypeTraits<q::kLong>::inf() }
};
TEMPORAL_TEST_SET(q::kMinute) = {
    { "00:00"_qu, 0 },
    { "15:7"_qu, 907 },
    { "-9:59u"_qu, -599 },
    { 930_qu, 570 },
    { -1359_qu, -839 },
    { -0001_qu, -1 },
    { "-42:01"_qu, -2521 },
    { "123:45"_qu, 7425 },
    { "abc"_qu, q::TypeTraits<q::kInt>::null() },
    { "12:"_qu, q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kMinute>::null(), q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kMinute>::inf(), q::TypeTraits<q::kInt>::inf() },
    { -q::TypeTraits<q::kMinute>::inf(), -q::TypeTraits<q::kInt>::inf() }
};
TEMPORAL_TEST_SET(q::kSecond) = {
    { "00:00:00"_qv, 0 },
    { "15:7:1"_qv, 54421 },
    { "-9:59:59v"_qv, -35999 },
    { 93000_qv, 34200 },
    { -135959_qv, -50399 },
    { "123:45:67"_qv, 445567 },
    { "-42:01"_qv, -151260 },
    { "12:01:"_qv, q::TypeTraits<q::kInt>::null() },
    { "abc"_qv, q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kSecond>::null(), q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kSecond>::inf(), q::TypeTraits<q::kInt>::inf() },
    { -q::TypeTraits<q::kSecond>::inf(), -q::TypeTraits<q::kInt>::inf() }
};
TEMPORAL_TEST_SET(q::kTime) = {
    { "00:00:00.000"_qt, 0 },
    { "15:7:1.0"_qt, 54421'000 },
    { "-9:59:59.1t"_qt, -35999'100 },
    { 93000000_qt, 34200'000 },
    { -135959001_qt, -50399'001 },
    { "-42:01"_qt, -151260'000 },
    { "123:45:67"_qt, 445567'000 },
    { "abc"_qt, q::TypeTraits<q::kInt>::null() },
    { "12:01:"_qt, q::TypeTraits<q::kInt>::null() },
    { "12:01:23."_qt, q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kTime>::null(), q::TypeTraits<q::kInt>::null() },
    { q::TypeTraits<q::kTime>::inf(), q::TypeTraits<q::kInt>::inf() },
    { -q::TypeTraits<q::kTime>::inf(), -q::TypeTraits<q::kInt>::inf() }
};

using TemporalTraitsTestTypes = ::testing::Types<
    q::TypeTraits<q::kTimestamp>,
    q::TypeTraits<q::kMonth>,
    q::TypeTraits<q::kDate>,
    q::TypeTraits<q::kDatetime>,
    q::TypeTraits<q::kTimespan>,
    q::TypeTraits<q::kMinute>,
    q::TypeTraits<q::kSecond>,
    q::TypeTraits<q::kTime>
>;

TYPED_TEST_SUITE(TemporalTraitsTests, TemporalTraitsTestTypes);

TYPED_TEST(TemporalTraitsTests, parsing)
{
    using traits = q::TypeTraits<TypeParam::type_id>;

    // Test samples are parsed as user-defined literals
    for (auto const& sample : temporals_) {
        EXPECT_EQ(sample.first, sample.second);
    }
}

#pragma endregion
