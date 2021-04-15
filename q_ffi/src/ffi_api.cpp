#include "ktype_traits.hpp"
#include "ffcall.hpp"
#include "version.hpp"
#include "ffi_api.h"

using namespace q;
using namespace std;

::K K4_DECL version(::K)
{
    auto ver = TypeTraits<kChar>::list(q_ffi::version);
    return ver.release();
}

::K K4_DECL load_fun(::K dllSym, ::K funName, ::K abi, ::K ret, ::K args)
{
    try {
        return q_ffi::loadFun(dllSym, funName, abi, ret, args).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL get_var(::K dllSym, ::K varName, ::K typ)
{
    try {
        return q_ffi::getVar(dllSym, varName, typ).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL set_var(::K dllSym, ::K varName, ::K typ, ::K val)
{
    try {
        return q_ffi::setVar(dllSym, varName, typ, val).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL to_addr(::K k)
{
    try {
        return q_ffi::to_addr(k).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL addr_get(::K addr, ::K typ)
{
    try {
        ++addr; ++typ;
//        return q_ffi::getAddrValue(addr, typ).release();
        throw K_error("get_at_addr");
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL addr_set(::K addr, ::K k)
{
    try {
        ++addr; ++k;
//        return q_ffi::setAddrValue(addr, k).release();
        throw K_error("set_at_addr");
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL size_type(::K)
{
    return q_ffi::get_type<std::size_t>().release();
}

::K K4_DECL ptr_type(::K)
{
    return q_ffi::get_type<void*>().release();
}
