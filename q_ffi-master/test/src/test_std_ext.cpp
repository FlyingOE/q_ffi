#include <gtest/gtest.h>
#include "std_ext.hpp"

#pragma region std_ext::can_apply<..> test suite

class canApplyTests : public ::testing::Test
{
protected:

    /// @brief Check if <code>std::tostring(t)</code> is valid, where @code t is of type @code T
    /// @ref https://stackoverflow.com/questions/30189926/metaprograming-failure-of-function-definition-defines-a-separate-function/30195655#30195655
    template<typename T>
    using to_string_t = decltype(std::to_string(std::declval<T>()));
    template<typename T>
    using can_to_string = std_ext::can_apply<to_string_t, T>;

    /// @brief Check if <code>t.c_str()</code> is valid, where @code t is of type @code T
    template<typename T>
    using member_c_str_t = decltype(std::declval<T>().c_str());
    template<typename T>
    using has_c_str = std_ext::can_apply<member_c_str_t, T>;

    /// @brief Check if <code>t.value</code> is valid, where @code t is of type @code T
    template<typename T>
    using member_value_t = decltype(std::declval<T>().value);
    template<typename T>
    using has_value = std_ext::can_apply<member_value_t, T>;

    /// @brief Check if <code>T.fun()</code> is valid
    template<typename T>
    using static_member_t = decltype(T::fun());
    template<typename T>
    using has_static_member = std_ext::can_apply<static_member_t, T>;

    /// @brief Check if <code>T.fun1(str)</code> is valid, where @c str is of type <code>std::string</code>
    template<typename T>
    using static_member1_t = decltype(T::fun1(std::declval<std::string>()));
    template<typename T>
    using has_static_member1 = std_ext::can_apply<static_member1_t, T>;

    /// @brief Check if <code>T.fun1a(str)</code> is valid, where @c str is of type <code>std::string</code>
    template<typename T>
    using static_member1a_t = decltype(T::fun1a(std::declval<std::string>()));
    template<typename T>
    using has_static_member1a = std_ext::can_apply<static_member1a_t, T>;

    /// @brief Check if <code>T::fun(r)</code> is valid, where @c r is of type @c R
    template<typename T, typename R>
    using member_overload_t = decltype(T::fun(std::declval<R>()));
    template<typename T, typename R>
    using has_member_overload = std_ext::can_apply<member_overload_t, T, R>;
};

TEST_F(canApplyTests, canApplyFunction)
{
    class no_to_string {};

    EXPECT_TRUE(can_to_string<int>::value);
    EXPECT_TRUE(can_to_string<double>::value);
    EXPECT_FALSE(can_to_string<std::string>::value);
    EXPECT_FALSE(can_to_string<char*>::value);
    EXPECT_FALSE(can_to_string<no_to_string>::value);
}

TEST_F(canApplyTests, canApplyMemberFunction)
{
    struct const_c_str { char const* c_str() const; };
    struct nonconst_c_str { char const* c_str(); };

    EXPECT_TRUE(has_c_str<std::string>::value);
    EXPECT_TRUE(has_c_str<std::string const>::value);
    EXPECT_TRUE(has_c_str<const_c_str>::value);
    EXPECT_TRUE(has_c_str<const_c_str const>::value);
    EXPECT_TRUE(has_c_str<nonconst_c_str>::value);
    EXPECT_FALSE(has_c_str<nonconst_c_str const>::value);
}

TEST_F(canApplyTests, canApplyMemberVariable)
{
    class public_value { public: bool value; };
    class private_value { private: bool value; };

    EXPECT_TRUE(has_value<public_value>::value);
    EXPECT_FALSE(has_value<private_value>::value);
}

TEST_F(canApplyTests, canApplyStaticMember)
{
    struct static_member
    {
        static int fun();
        static char const* fun1(std::string const&);
        static char const* fun1a(int);
    };
    struct nonstatic_member
    {
        int fun();
        char const* fun1(std::string const&);
        char const* fun1a(int);
    };

    EXPECT_TRUE(has_static_member<static_member>::value);
    EXPECT_TRUE(has_static_member1<static_member>::value);
    EXPECT_FALSE(has_static_member1a<static_member>::value);

    EXPECT_FALSE(has_static_member<nonstatic_member>::value);
    EXPECT_FALSE(has_static_member1<nonstatic_member>::value);
    EXPECT_FALSE(has_static_member1a<nonstatic_member>::value);
}

TEST_F(canApplyTests, canApplyMemberOverload)
{
    struct member_overloads
    {
        static int fun(int);
        static int fun(float);
    };

    EXPECT_TRUE((has_member_overload<member_overloads, int>::value));
    EXPECT_TRUE((has_member_overload<member_overloads, float>::value));
    EXPECT_FALSE((has_member_overload<member_overloads, char const*>::value));
    EXPECT_FALSE((has_member_overload<member_overloads, double>::value));
}

#pragma endregion

TEST(stdextTests, sgn)
{
    EXPECT_EQ(std_ext::sgn(+1),  1);
    EXPECT_EQ(std_ext::sgn(-1), -1);
    EXPECT_EQ(std_ext::sgn(-0),  0);
    EXPECT_EQ(std_ext::sgn(std::numeric_limits<long long>::max()),  1);
    EXPECT_EQ(std_ext::sgn(std::numeric_limits<long long>::min()), -1);

    EXPECT_EQ(std_ext::sgn(+1.),  1);
    EXPECT_EQ(std_ext::sgn(-1.), -1);
    EXPECT_EQ(std_ext::sgn(-0.),  0);
    EXPECT_EQ(std_ext::sgn(std::numeric_limits<double>::max()),  1);
    EXPECT_EQ(std_ext::sgn(std::numeric_limits<double>::min()),  0);
    EXPECT_EQ(std_ext::sgn(-std::numeric_limits<double>::max()), -1);
    EXPECT_EQ(std_ext::sgn(-std::numeric_limits<double>::min()),  0);
    EXPECT_EQ(std_ext::sgn( std::numeric_limits<double>::infinity()),  1);
    EXPECT_EQ(std_ext::sgn(-std::numeric_limits<double>::infinity()), -1);
    EXPECT_EQ(std_ext::sgn( std::numeric_limits<double>::epsilon()),  0);
    EXPECT_EQ(std_ext::sgn(-std::numeric_limits<double>::epsilon()),  0);
    EXPECT_EQ(std_ext::sgn( std::numeric_limits<double>::epsilon() * 2),  1);
    EXPECT_EQ(std_ext::sgn(-std::numeric_limits<double>::epsilon() * 2), -1);
}
