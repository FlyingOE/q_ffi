#pragma once

#include "gtest/gtest.h"

namespace q_ffi
{

/// @remarks As test program makes use of <tt>c.dll</tt> for stand-alone tests outside kdb+,
///     we must manually initialize q's memory system before any K object creation!
struct K_setup : public ::testing::Environment
{
public:
    void SetUp() override;
};

}//namespace q_ffi
