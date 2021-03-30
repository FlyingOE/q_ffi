#include <iostream>
#include <vector>
#include <cctype>
#include <mutex>
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
    namespace details
    {
        mutex MUTEX;
        unordered_map<string, unique_ptr<q_ffi::Invocator>> INVOCATORS;
    }

    q_ffi::Invocator& loadDLL(string const& dllName)
    {
        try {
            lock_guard<mutex> lock{ details::MUTEX };
            auto const p = details::INVOCATORS.emplace(dllName,
                make_unique<q_ffi::Invocator>(dllName.c_str()));
#       ifndef NDEBUG
            if (p.second) {
                cout << "# <q_ffi> loaded "
                    << details::INVOCATORS.size() << " foreign function(s)." << endl;
            }
#       endif
            return *p.first->second;
        }
        catch (runtime_error const& ex) {
            throw K_error(ex);
        }
    }

    q_ffi::Invocator& unwrapCaller(::K wrapped)
    {
        using wrapped_ptr = TypeTraits<kLong>;
        constexpr auto ptr_size = sizeof(q_ffi::Invocator*);
        static_assert(ptr_size <= sizeof(wrapped_ptr::value_type),
            "unwrap q_ffi::Invocator* from ::J");

        if (Nil == wrapped)
            throw K_error("nil FFI caller");
        else if (-wrapped_ptr::type_id != type(wrapped))
            throw K_error("invalid FFI caller");

        q_ffi::Invocator* caller{ nullptr };
        memcpy(&caller, &wrapped_ptr::value(wrapped), ptr_size);
        if (nullptr == caller)
            throw K_error("null FFI callre");
        else
            return *caller;
    }

    ::K doCall(::K caller, initializer_list<::K> params)
    {
        try {
            auto& call = unwrapCaller(caller);
            return call(params).release();
        }
        catch (K_error const& ex) {
            return ex.report().release();
        }
    }

    ::K doCall1(::K caller, ::K p1)
    { return doCall(caller, { p1 }); }

    ::K doCall2(::K caller, ::K p1, ::K p2)
    { return doCall(caller, { p1, p2 }); }

    ::K doCall3(::K caller, ::K p1, ::K p2, ::K p3)
    { return doCall(caller, { p1, p2, p3 }); }

    ::K doCall4(::K caller, ::K p1, ::K p2, ::K p3, ::K p4)
    { return doCall(caller, { p1, p2, p3, p4 }); }

    ::K doCall5(::K caller, ::K p1, ::K p2, ::K p3, ::K p4, ::K p5)
    { return doCall(caller, { p1, p2, p3, p4, p5 }); }

    ::K doCall6(::K caller, ::K p1, ::K p2, ::K p3, ::K p4, ::K p5, ::K p6)
    { return doCall(caller, { p1, p2, p3, p4, p5, p6 }); }

    ::K doCall7(::K caller, ::K p1, ::K p2, ::K p3, ::K p4, ::K p5, ::K p6, ::K p7)
    { return doCall(caller, { p1, p2, p3, p4, p5, p6, p7 }); }

    K_ptr createFunctor(size_t argc)
    {
        switch(argc) {
        case 0:
        case 1:
            return K_ptr{ ::dl(&doCall1, 1 + 1) };
        case 2:
            return K_ptr{ ::dl(&doCall2, 2 + 1) };
        case 3:
            return K_ptr{ ::dl(&doCall3, 3 + 1) };
        case 4:
            return K_ptr{ ::dl(&doCall4, 4 + 1) };
        case 5:
            return K_ptr{ ::dl(&doCall5, 5 + 1) };
        case 6:
            return K_ptr{ ::dl(&doCall6, 6 + 1) };
        case 7:
            return K_ptr{ ::dl(&doCall7, 7 + 1) };
        default:
            assert(!"q_ffi::MAX_ARGC exceeded!");
            return K_ptr{};
        }
    }

    K_ptr wrapCaller(q_ffi::Invocator& caller)
    {
        using wrapped_ptr = TypeTraits<kLong>;
        constexpr auto ptr_size = sizeof(q_ffi::Invocator*);
        static_assert(ptr_size <= sizeof(wrapped_ptr::value_type),
            "wrap q_ffi::Invocator* into ::J");

        auto wrapped = wrapped_ptr::atom(0);
        auto const pc = &caller;
        memcpy(&wrapped_ptr::value(wrapped), &pc, ptr_size);

        // .[doCall;enlist wrapped;::]
        K_ptr call = createFunctor(caller.rank());
        K_ptr pars{ ::knk(1, wrapped.release()) };
        K_ptr applied{ ::ee(::dot(call.get(), pars.get())) };
        if (kError == type(applied))
            throw K_error(applied);
        else
            return applied;
    }

}//namespace /*anonymous*/

::K K4_DECL load(::K dllSym, ::K fName, ::K abi, ::K ret, ::K args)
{
    try {
        auto const dll = q2String(dllSym);
        auto& caller = loadDLL(dll);

        auto const func = q2String(fName);

        auto const retType = q2Char(ret);

        auto argTypes = q2String(args);
        if (argTypes.empty() || argTypes == " ")
            argTypes = "";  // q function requires at least 1 argument

        auto abiType = q2String(abi);
        transform(abiType.cbegin(), abiType.cend(), abiType.begin(),
            [](char c) { return static_cast<char>(toupper(c)); });

        caller.load(func.c_str(), retType, argTypes.c_str(), abiType.c_str());

        // First argument is already occupied by q_ffi::Invocator*
        if (caller.rank() >= q_ffi::MAX_ARGC) {
            ostringstream buffer;
            buffer << "too many arguments (<=" << (q_ffi::MAX_ARGC - 1) << " expected)";
            throw K_error(buffer.str());
        }
        else {
            return wrapCaller(caller).release();
        }
    }
    catch (K_error const& ex) {
        return ex.report().release();
    }
}

::K K4_DECL version(K)
{
	return TypeTraits<kChar>::list(q_ffi::version).release();
}
