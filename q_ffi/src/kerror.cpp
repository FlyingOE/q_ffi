#include "kerror.hpp"
#include "ktype_traits.hpp"

using namespace std;

string q::details::q2error(::K const what)
{
    auto const msg = q::to_string(what);
    return kError == type(what) ? msg : TypeTraits<kError>::to_str(msg.c_str());
}

::K q::K_error::report() const noexcept
{
    return TypeTraits<kError>::atom(what());
}