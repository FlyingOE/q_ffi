#include "ktype_traits.hpp"
#include "ffcall.hpp"
#include "version.hpp"
#include "ffi_api.h"

using namespace q;

::K K4_DECL load(::K dllSym, ::K fName, ::K abi, ::K ret, ::K args)
{
    try {
        return q_ffi::load(dllSym, fName, abi, ret, args).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL version(K)
{
    auto ver = TypeTraits<kChar>::list(q_ffi::version);
    return ver.release();
}
