#include "ffi.h"
#include "version.hpp"

K K4_DECL load(K dllSym, K fName, K resType, K parTypes)
{
	dllSym;
	fName;
	resType;
	parTypes;
	return version(nullptr);
}

K K4_DECL version(K)
{
	return kp(const_cast<S>(q_ffi::version));
}