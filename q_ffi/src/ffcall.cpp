#include <sstream>
#include <mutex>
#ifndef NDEBUG
#   include <iostream>
#endif
#include "kerror.hpp"
#include "ffcall.hpp"

#if defined(_WIN32)
#   if defined(_WIN64)
#       define PLATFORM_X86_64
#   else
#       define PLATFORM_X86
#       include <regex>
#   endif
#elif defined(__linux__)
#   if defined(__ANDROID__)
#       error FIXME: Android not supported yet!
#   elif defined(__x86_64__)
#       define PLATFORM_X86_64
#   else
#       define PLATFORM_X86
#   endif
#else
#   error FIXME: platform not supported yet!
#endif

using namespace q;
using namespace std;

#pragma region q_ffi::Invocator implementations

q_ffi::Invocator::Invocator(char const* dll)
    : dll_(dll), func_{ nullptr }, ret_{}, args_{}, ffi_{}
{}

unsigned int
q_ffi::Invocator::rank() const
{
    assert(args_.size() <= numeric_limits<unsigned int>::max());
    return static_cast<unsigned int>(args_.size());
}

void
q_ffi::Invocator::load(char const* func, char retType, char const* argTypes,
    char const* abiType)
{
    try {
        func_ = dll_.locateProc<function_type>(func);
        setReturnType(retType);
        setArgumentTypes(argTypes);
        auto const abi = mapABI(abiType, func);
        prepareFFI(abi);
    }
    catch (runtime_error const& ex) {
        throw K_error(ex);
    }
}

K_ptr
q_ffi::Invocator::operator()(initializer_list<::K> params)
{
    auto const rank = this->rank();
    if (params.size() != rank && !(params.size() == 1 && rank == 0)) {
        ostringstream buffer;
        buffer << "rank (" << rank << " expected)";
        throw K_error(buffer.str());
    }

    // Prepare result
    assert(ret_);
    auto ret = ret_->create();

    // Prepare parameters
    vector<unique_ptr<Parameter>> parms{ rank };
    transform(args_.cbegin(), args_.cend(), params.begin(), parms.begin(),
        [](auto const& arg, auto const param) { return arg->map(param); });

    auto pars = make_unique<void*[]>(rank);
    transform(parms.cbegin(), parms.cend(), pars.get(),
        [](auto const& par) { return par->get(); });

    // FFI invocation
    ffi_call(&ffi_.cif, func_, ret->get(), pars.get());
    return ret->release();
}

void
q_ffi::Invocator::prepareFFI(ffi_abi abi)
{
    assert(ret_);
    ffi_.ret_type = &ret_->type();

    ffi_.arg_types = make_unique<ffi_type*[]>(rank());
    transform(args_.cbegin(), args_.cend(), ffi_.arg_types.get(),
        [](auto const& arg) { return &arg->type(); });

    auto const status = ffi_prep_cif(
        &ffi_.cif, abi, rank(), ffi_.ret_type, ffi_.arg_types.get());
    switch (status) {
    case FFI_OK:
        break;
    case FFI_BAD_TYPEDEF:
        throw K_error("bad FFI argument/return type spec");
    case FFI_BAD_ABI:
        throw K_error("bad FFI ABI spec");
    default:
        throw K_error("unexpected libFFI status");
    }
}

void
q_ffi::Invocator::setReturnType(char typeCode)
{
    ret_ = mapType(typeCode);
    assert(ret_);
}

void
q_ffi::Invocator::setArgumentTypes(char const* typeCodes)
{
    assert(nullptr != typeCodes);
    auto const arity = strlen(typeCodes);
    if (arity > MAX_ARGC - 1)
        throw K_error("too many arguments");

    args_.resize(arity);
    transform(typeCodes, typeCodes + arity, args_.begin(),
        [](char t) { return mapType(t); });
}

ffi_abi
q_ffi::Invocator::mapABI(char const* abiType, char const* funcName)
{
    ffi_abi abi = static_cast<ffi_abi>(FFI_FIRST_ABI - 1);
    if (nullptr != abiType) {
#   ifdef PLATFORM_X86
        static const unordered_map<string, ffi_abi> ABI_TYPES{
            { "CDECL", FFI_MS_CDECL },
            { "STDCALL", FFI_STDCALL },
            { "FASTCALL", FFI_FASTCALL },
        };

        auto const t = ABI_TYPES.find(abiType);
        if (0 == strcmp(abiType, "")) {
            abi = guessABI(funcName);
        }
        else if (t == ABI_TYPES.cend()) {
            ostringstream buffer;
            buffer << "unknown ABI type: `" << abiType << "'";
            throw K_error(buffer.str());
        }
        else {
            return t->second;
        }
#   else
        abi = guessABI(funcName);
#   endif
    }
    else {
        abi = guessABI(funcName);
    }
    assert(FFI_FIRST_ABI <= abi && abi <= FFI_LAST_ABI);
    return abi;
}

#ifdef PLATFORM_X86
ffi_abi
q_ffi::Invocator::guessABI(char const* funcName)
{
    assert(nullptr != funcName);

    static const regex stdcall{ R"(_.+@(\d+))" };
    if (regex_match(funcName, stdcall))
        return FFI_STDCALL;

    static const regex fastcall{ R"(@.+@(\d+))" };
    if (regex_match(funcName, fastcall))
        return FFI_FASTCALL;

    return FFI_MS_CDECL;
}
#else
ffi_abi
q_ffi::Invocator::guessABI(char const* /*funcName*/)
{
    return FFI_DEFAULT_ABI;
}
#endif

q_ffi::Invocator::argument_type
q_ffi::Invocator::mapType(char typeCode)
{
#   define FFI_MAP_TYPE_CASE(kTypeCode, kType, ffiType)    \
        case (kTypeCode):   \
            return make_unique<Atom<(kType)>>((ffiType))

    switch (typeCode) {
        FFI_MAP_TYPE_CASE('b', kBoolean, ffi_type_sint8);
        FFI_MAP_TYPE_CASE('x', kByte, ffi_type_uint8);
        FFI_MAP_TYPE_CASE('c', kChar, ffi_type_uchar);
        FFI_MAP_TYPE_CASE('h', kShort, ffi_type_sint16);
        FFI_MAP_TYPE_CASE('i', kInt, ffi_type_sint32);
        FFI_MAP_TYPE_CASE('j', kLong, ffi_type_sint64);
        FFI_MAP_TYPE_CASE('e', kReal, ffi_type_float);
        FFI_MAP_TYPE_CASE('f', kFloat, ffi_type_double);
    case ' ':
        return make_unique<Void>();
    case 's':
        return make_unique<Atom<kSymbol>>();
    case '&':
        return make_unique<Pointer>();
    default:
        ostringstream buffer;
        buffer << "invalid type for FFI argument: '" << typeCode << "'";
        throw K_error(buffer.str());
    }
}

#pragma endregion

#pragma region q_ffi::load_function(...) implementations

namespace
{
    namespace details
    {
        mutex MUTEX;
        unordered_map<string, unique_ptr<q_ffi::Invocator>> INVOCATORS;
    }

    q_ffi::Invocator& createCaller(string const& dllName, string const& id)
    {
        try {
            lock_guard<mutex> lock{ details::MUTEX };
            auto const p = details::INVOCATORS.emplace(id,
                make_unique<q_ffi::Invocator>(dllName.c_str()));
            if (p.second) {
#           ifndef NDEBUG
                cout << "# <q_ffi> has loaded "
                    << details::INVOCATORS.size() << " foreign symbol(s)." << endl;
#           endif
            }
            return *p.first->second;
        }
        catch (runtime_error const& ex) {
            throw K_error(ex);
        }
    }

    q_ffi::Invocator& unwrapCaller(::K wrapped)
    {
        using wrapped_ptr = q_ffi::TypeCode<sizeof(q_ffi::Invocator*)>::traits;

        if (Nil == wrapped)
            throw K_error("nil FFI caller");
        else if (-wrapped_ptr::type_id != type(wrapped))
            throw K_error("invalid FFI caller");

        auto caller = *misc::ptr_alias<q_ffi::Invocator**>(&wrapped_ptr::value(wrapped));
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
    {
        return doCall(caller, { p1 });
    }

    ::K doCall2(::K caller, ::K p1, ::K p2)
    {
        return doCall(caller, { p1, p2 });
    }

    ::K doCall3(::K caller, ::K p1, ::K p2, ::K p3)
    {
        return doCall(caller, { p1, p2, p3 });
    }

    ::K doCall4(::K caller, ::K p1, ::K p2, ::K p3, ::K p4)
    {
        return doCall(caller, { p1, p2, p3, p4 });
    }

    ::K doCall5(::K caller, ::K p1, ::K p2, ::K p3, ::K p4, ::K p5)
    {
        return doCall(caller, { p1, p2, p3, p4, p5 });
    }

    ::K doCall6(::K caller, ::K p1, ::K p2, ::K p3, ::K p4, ::K p5, ::K p6)
    {
        return doCall(caller, { p1, p2, p3, p4, p5, p6 });
    }

    ::K doCall7(::K caller, ::K p1, ::K p2, ::K p3, ::K p4, ::K p5, ::K p6, ::K p7)
    {
        return doCall(caller, { p1, p2, p3, p4, p5, p6, p7 });
    }

    K_ptr createFunctor(size_t argc)
    {
        switch (argc) {
        case 0:
        case 1:
            return TypeTraits<kDLL>::atom(&doCall1);
        case 2:
            return TypeTraits<kDLL>::atom(&doCall2);
        case 3:
            return TypeTraits<kDLL>::atom(&doCall3);
        case 4:
            return TypeTraits<kDLL>::atom(&doCall4);
        case 5:
            return TypeTraits<kDLL>::atom(&doCall5);
        case 6:
            return TypeTraits<kDLL>::atom(&doCall6);
        case 7:
            return TypeTraits<kDLL>::atom(&doCall7);
        default:
            assert(!"q_ffi::MAX_ARGC exceeded!");
            return K_ptr{};
        }
    }

    K_ptr wrapCaller(q_ffi::Invocator& caller)
    {
        using wrapped_ptr = q_ffi::TypeCode<sizeof(&caller)>::traits;

        auto wrapped = wrapped_ptr::list({ 0 });
        auto const pc = &caller;
        *wrapped_ptr::index(wrapped) = *misc::ptr_alias<wrapped_ptr::const_pointer>(&pc);

        // .[doCall;enlist wrapped;::]
        auto call = createFunctor(caller.rank());
        K_ptr applied{ ::ee(::dot(call.get(), wrapped.get())) };
        if (kError == type(applied))
            throw K_error(applied);
        else
            return applied;
    }

}//namespace /*anonymous*/

K_ptr
q_ffi::load_function(::K dllSym, ::K funName, ::K abi, ::K ret, ::K args)
noexcept(false)
{
    auto const dll = q2String(dllSym);
    auto const func = q2String(funName);
    auto& caller = createCaller(dll, func);

    auto const retType = q2Char(ret);

    string argTypes;
    try {
        argTypes = q2String(args);
    }
    catch (K_error const&) {
        if (-kChar != type(args))
            throw;
        else
            argTypes += q2Char(args);
    }
    if (argTypes.empty() || argTypes == " ")
        argTypes = "";  // q function requires at least 1 argument

    string abiType;
    try {
        abiType = q2String(abi);
    }
    catch (K_error const&) {
        if (kNil != type(abi))
            throw;
        else
            abiType = "";
    }
    transform(abiType.cbegin(), abiType.cend(), abiType.begin(),
        [](char c) { return static_cast<char>(toupper(c)); });

    caller.load(func.c_str(), retType, argTypes.c_str(), abiType.c_str());

    // First argument is already occupied by q_ffi::Invocator*
    if (caller.rank() > q_ffi::MAX_ARGC - 1) {
        ostringstream buffer;
        buffer << "too many arguments (<=" << (q_ffi::MAX_ARGC - 1) << " expected)";
        throw K_error(buffer.str());
    }
    else {
        return wrapCaller(caller);
    }
}

#pragma endregion