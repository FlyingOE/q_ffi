#include "ktype_traits.hpp"
#include "ffcall.hpp"
#include "version.hpp"
#include "ffi_api.h"

using namespace q;

::K K4_DECL loadFun(::K dllSym, ::K funName, ::K abi, ::K ret, ::K args)
{
    try {
        return q_ffi::loadFun(dllSym, funName, abi, ret, args).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL getVar(::K dllSym, ::K varName, ::K typ)
{
    try {
        return q_ffi::getVar(dllSym, varName, typ).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL setVar(::K dllSym, ::K varName, ::K typ, ::K val)
{
    try {
        return q_ffi::setVar(dllSym, varName, typ, val).release();
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
