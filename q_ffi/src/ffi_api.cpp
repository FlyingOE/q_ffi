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
        return q_ffi::load_function(dllSym, funName, abi, ret, args).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL get_var(::K dllSym, ::K varName, ::K typ)
{
    try {
        return q_ffi::get_variable(dllSym, varName, typ).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL set_var(::K dllSym, ::K varName, ::K val)
{
    try {
        q_ffi::set_variable(dllSym, varName, val);
        return Nil;
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL addr_of(::K k)
{
    try {
        return q_ffi::address_of(k).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL get_from_addr(::K addr, ::K typ)
{
    try {
        return q_ffi::get_from_address(addr, typ).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL set_to_addr(::K addr, ::K val)
{
    try {
        q_ffi::set_to_address(addr, val);
        return Nil;
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL free_addr(::K addr)
{
    try {
        q_ffi::free_address(addr);
        return Nil;
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL size_type(::K)
{
    return q_ffi::get_type<std::size_t>().release();
}

::K K4_DECL size_size(::K)
{
    return q_ffi::get_size<std::size_t>().release();
}

::K K4_DECL ptr_type(::K)
{
    return q_ffi::get_type<void*>().release();
}

::K K4_DECL ptr_size(::K)
{
    return q_ffi::get_size<void*>().release();
}
