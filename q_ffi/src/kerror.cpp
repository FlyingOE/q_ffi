#include "kerror.hpp"
#include "ktype_traits.hpp"

using namespace std;

string
q::details::q2error(::K const what)
{
    auto const msg = q::to_string(what);
    return kError == type(what) ? msg : TypeTraits<kError>::to_str(msg.c_str());
}

string
q::details::q2error(q::K_ptr const& what)
{
    return q2error(what.get());
}

q::K_ptr
q::K_error::report() const noexcept
{
    return TypeTraits<kError>::atom(what());
}