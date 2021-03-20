#include "ktype_traits.hpp"
#include "ffi_api.h"
#include "version.hpp"
//#include <dlfcn.h>
//#include <ffi.h>

#include <iostream>
#include "kerror.hpp"
::K K4_DECL load(::K dllSym, ::K fName, ::K resType, ::K parTypes)
{
//    auto dll = q::q2Str(dllSym);
    ++dllSym;
	fName++;
	resType++;
	parTypes++;
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
