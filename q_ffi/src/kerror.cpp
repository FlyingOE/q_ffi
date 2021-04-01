#include "kerror.hpp"
#include "ktype_traits.hpp"

using namespace q;
using namespace std;

string
details::q2error(::K const what)
{
    auto const msg = to_string(what);
    return kError == type(what) ? msg : TypeTraits<kError>::to_str(msg.c_str());
}

string
details::q2error(K_ptr const& what)
{
    return q2error(what.get());
}

K_ptr
K_error::report() const noexcept
{
    return TypeTraits<kError>::atom(what());
}
