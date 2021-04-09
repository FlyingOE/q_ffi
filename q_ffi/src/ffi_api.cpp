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

::K K4_DECL size_type(::K)
{
    using size_type = size_t;
    static constexpr auto typeId = q_ffi::TypeCode<sizeof(size_type)>::traits::type_id;
    return TypeTraits<kChar>::atom(q::TypeId2Code.at(typeId)).release();
}

::K K4_DECL ptr_type(::K)
{
    using ptr_type = void*;
    static constexpr auto typeId = q_ffi::TypeCode<sizeof(ptr_type)>::traits::type_id;
    return TypeTraits<kChar>::atom(q::TypeId2Code.at(typeId)).release();
}

::K K4_DECL get_addr(::K k)
{
    try {
        return q_ffi::getAddr(k).release();
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}
