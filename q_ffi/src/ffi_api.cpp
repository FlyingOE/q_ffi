#include <iostream>
#include <dlfcn.h>
#include <ffi.h>

#include "ktype_traits.hpp"
#include "ffi_api.h"
#include "version.hpp"
#include "kerror.hpp"
#include "type_convert.hpp"

using namespace q;

::K add(::K x, ::K y)
{
    using qType = TypeTraits<kFloat>;
    return qType::atom(-(q2Real(x) + q2Real(y)));
}

::K K4_DECL load(::K dllSym, ::K fName, ::K abi, ::K resType, ::K parTypes)
{
    std::string dllName, funcName, abiType, result, params;
    try {
        dllName = q::q2String(dllSym);
        funcName = q::q2String(fName);
        abiType = q::q2String(abi);
        result = q::q2String(resType);
        params = q::q2String(parTypes);
    }
    catch (q::K_error const& ) {
        dllSym->r += 1;
        K_ptr y{ ::knk(1, dllSym) };
        K_ptr f{ ::dl(&add, 2) };
        return ::dot(f.get(), y.get());
//    catch (q::K_error const& ex) {
//        return ex.report();
    }

    return q::K_error("error!").report();
    /*
    auto handle = dlopen("QQQQQ:kernel32.dll", RTLD_NOW);
    std::cout << handle << std::endl;
    if (!handle) {
        return q::error("failed to load kernel32.dll", true);
    }
    else {
        dlclose(handle);
    }
	return version(nullptr);
    */
}

::K K4_DECL version(K)
{
	return q::TypeTraits<q::kChar>::list(q_ffi::version);
}
