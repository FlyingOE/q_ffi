#include <gtest/gtest.h>
#include "std_ext.hpp"

using namespace std;

namespace std_ext
{
    TEST(stdextTests, signum)
    {
        EXPECT_EQ(signum(+1),  1);
        EXPECT_EQ(signum(-1), -1);
        EXPECT_EQ(signum(-0),  0);
        EXPECT_EQ(signum(numeric_limits<long long>::max()),  1);
        EXPECT_EQ(signum(numeric_limits<long long>::min()), -1);

        EXPECT_EQ(signum(+1.),  1);
        EXPECT_EQ(signum(-1.), -1);
        EXPECT_EQ(signum(-0.),  0);
        EXPECT_EQ(signum(numeric_limits<double>::max()),  1);
        EXPECT_EQ(signum(numeric_limits<double>::min()),  1);
        EXPECT_EQ(signum(-numeric_limits<double>::max()), -1);
        EXPECT_EQ(signum(-numeric_limits<double>::min()), -1);
        EXPECT_EQ(signum( numeric_limits<double>::infinity()),  1);
        EXPECT_EQ(signum(-numeric_limits<double>::infinity()), -1);
        EXPECT_EQ(signum( numeric_limits<double>::epsilon()),  1);
        EXPECT_EQ(signum(-numeric_limits<double>::epsilon()), -1);
        EXPECT_EQ(signum( numeric_limits<double>::epsilon() * 2),  1);
        EXPECT_EQ(signum(-numeric_limits<double>::epsilon() * 2), -1);
    }

}//namespace std_ext
