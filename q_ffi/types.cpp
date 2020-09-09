#include "types.hpp"

q::K_ptr q::error(char const* msg, bool sys) noexcept
{
    return TypeTraits<kError>::atom(msg, sys);
}
