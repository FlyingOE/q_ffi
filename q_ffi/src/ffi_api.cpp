#include <iostream>
#include <vector>
#include <cctype>
#include <dlfcn.h>
#include <ffi.h>
#include "ktype_traits.hpp"
#include "ffi_api.h"
#include "version.hpp"
#include "kerror.hpp"
#include "type_convert.hpp"
#include "ffcall.hpp"

using namespace q;
using namespace std;

namespace
{
#pragma region Implementation details for loading FFI

    vector<unique_ptr<q_ffi::Invocator>> CALLERS;

    q_ffi::Invocator* createCaller(string const& dllName)
    {
        try {
            auto caller = make_unique<q_ffi::Invocator>(dllName.c_str());
            CALLERS.push_back(move(caller));
            return CALLERS.rbegin()->get();
        }
        catch (runtime_error const& ex) {
            throw K_error(ex);
        }
    }

    char verifyReturnType(string const& retType)
    {
        switch (retType.size()) {
        case 0:
            return ' '; // default to `void' return
        case 1:
            return retType[0];
        default:
            throw K_error("invalid return type");
        }
    }

    string verifyArgumentTypes(string const& argTypes)
    {
        // Special case: q functions reqiure at least 1 argument
        if (argTypes.empty() || argTypes == " ")
            return "";
        else
            return argTypes;
    }

    string verifyABIType(string const& abiType)
    {
        string abi{ abiType };
        transform(abi.rbegin(), abi.rend(), abi.begin(),
            [](char c) { return static_cast<char>(toupper(c)); });
        return abi;
    }

    q_ffi::Invocator* loadFFI(
        string const& dllName, string const& funcName, string const& abiType,
        string const& retType, string const& argTypes)
    {
        auto caller = createCaller(dllName);
        assert(nullptr != caller);

        caller->load(funcName.c_str(),
            verifyReturnType(retType), verifyArgumentTypes(argTypes).c_str(),
            verifyABIType(abiType).c_str());
        return caller;
    }

#pragma endregion

#pragma region Implementation details for invoking FFI

    q_ffi::Invocator* unwrapCaller(::K caller)
    {
        using wrapped_pointer = TypeTraits<kLong>;

        if (caller == Nil)
            throw K_error("nil FFI invocator");
        else if (type(caller) != kLong)
            throw K_error("invalid FFI invocator");

        q_ffi::Invocator* invocator{ nullptr };
        static_assert(sizeof(invocator) <= sizeof(wrapped_pointer::value_type),
            "ensure storage for q_ffi::Invocator*");
        memcpy(&invocator, &wrapped_pointer::value(caller), sizeof(invocator));
        assert(nullptr != invocator);
        return invocator;
    }

    ::K invoke(::K caller, ::K params)
    {
        using wrapped_pointer = TypeTraits<kLong>;
        try {
            q_ffi::Invocator& invocator = *unwrapCaller(caller);

            if (params == Nil)
                throw K_error("nil parameter list");
            if (type(params) == kMixed) {
                using mixed_list = TypeTraits<kMixed>;
                vector<::K> pars{ count(params) };
                copy(mixed_list::index(params), mixed_list::index(params) + count(params),
                    pars.begin());
                return invocator(pars);
            }
            else if (type(params) > kMixed) {
                return invocator(params);
            }
            else {
                throw K_error("parameter list as an atom");
            }
        }
        catch (K_error const& ex) {
            return ex.report();
        }
    }

    ::K wrapCaller(q_ffi::Invocator* caller)
    {
        using wrapped_pointer = TypeTraits<kLong>;
        assert(nullptr != caller);

        K_ptr params{ wrapped_pointer::atom(0) };
        static_assert(sizeof(caller) <= sizeof(wrapped_pointer::value_type),
            "ensure storage for q_ffi::Invocator*");
        memcpy(&wrapped_pointer::value(params.get()), &caller, sizeof(caller));
        params.reset(::knk(1, params.release()));

        K_ptr func{ ::dl(&invoke, 2) };
        return ::dot(func.get(), params.get());
    }

#pragma endregion

}//namespace /*anonymous*/

::K K4_DECL load(::K dllSym, ::K fName, ::K abi, ::K ret, ::K args)
{
    try {
        q_ffi::Invocator* caller = loadFFI(q2String(dllSym),
            q2String(fName), q2String(abi), q2String(ret), q2String(args));
        return wrapCaller(caller);
    }
    catch (K_error const& ex) {
        return ex.report();
    }
}

::K K4_DECL version(K)
{
	return TypeTraits<kChar>::list(q_ffi::version);
}
