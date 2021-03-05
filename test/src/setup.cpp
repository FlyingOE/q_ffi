#include "setup.hpp"
#include <k_compat.h>

/// @ref https://code.kx.com/q/interfaces/c-client-for-q/#managing-memory-and-reference-counting
void q_ffi::K_setup::SetUp()
{
    ::khp(const_cast<::S>(""), -1);
}

namespace q_ffi
{

    ::testing::Environment* const init_c_call =
        ::testing::AddGlobalTestEnvironment(new K_setup);

}//namespace q_ffi
