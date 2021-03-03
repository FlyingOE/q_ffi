#include "version.hpp"
#include "ktypes.hpp"
#include "ffi.h"
//#include <dlfcn.h>

#if WIN32
BOOL APIENTRY DllMain(HMODULE /*hModule*/,
    DWORD  ul_reason_for_call,
    LPVOID /*lpReserved*/
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
#endif

#include <iostream>
::K K4_DECL load(::K dllSym, ::K fName, ::K resType, ::K parTypes)
{
	dllSym++;
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
	return ::kp(const_cast<::S>(q_ffi::version));
}
