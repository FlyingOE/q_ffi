#include "ktype_traits.hpp"
#include "ffi.h"
#include <dlfcn.h>
#include "version.hpp"

#include <iostream>
::K K4_DECL load(::K dllSym, ::K fName, ::K resType, ::K parTypes)
{
//    auto dll = q::q2Str(dllSym);
    ++dllSym;
	fName++;
	resType++;
	parTypes++;
    return ::krr("error!");
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
