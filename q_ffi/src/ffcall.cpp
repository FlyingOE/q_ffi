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
    : dll_(dll), sym_{ nullptr }, ret_{}, args_{}, ffi_{}
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
        sym_.func = dll_.locateProc<function_type>(func);
        setReturnType(retType);
        setArgumentTypes(argTypes);
        auto const abi = mapABI(abiType, func);
        verifyArgumentTypes(func, abi);
        prepareFFI(abi);
    }
    catch (runtime_error const& ex) {
        throw K_error(ex);
    }
}

void
q_ffi::Invocator::load(char const* var, char varType)
{
    try {
        sym_.var = dll_.locateProc<void*>(var);
        setReturnType(varType);
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

    auto pars = make_unique<void*[]>(rank);
    transform(args_.cbegin(), args_.cend(), params.begin(), pars.get(),
        [](auto const& arg, auto const& par) { return arg->get(par); });

    return invoke(pars.get());
}

K_ptr
q_ffi::Invocator::operator()()
{
    if (nullptr != ffi_.ret_type)
        throw K_error("not a foreign variable");
    else
        assert(!ffi_.arg_types);

    K_ptr val = ret_->create();
    memcpy(ret_->get(val), sym_.var, ret_->size());
    return val;
}

void
q_ffi::Invocator::operator()(::K val)
{
    if (nullptr != ffi_.ret_type)
        throw K_error("not a foreign variable");
    else
        assert(!ffi_.arg_types);

    memcpy(sym_.var, ret_->get(val), ret_->size());
}

K_ptr
q_ffi::Invocator::invoke(void* params[])
{
    if (nullptr == ffi_.ret_type)
        throw K_error("not a foreign function");
    else
        assert(ffi_.arg_types);

    // Prepare result placeholder
    assert(ret_);
    K_ptr result{ ret_->create() };
    ffi_arg placeholder{};
    bool mapped = false;
    void* res = nullptr;
    if (Nil == result.get()) {
        res = nullptr;
    }
    else if (ret_->size() < sizeof(ffi_arg)) {
        mapped = true;
        res = &placeholder;
    }
    else {
        res = ret_->get(result.get());
    }

    // FFI invocation
    ffi_call(&ffi_.cif, sym_.func, res, params);

    // Fetch result
    if (mapped)
        ret_->set(result, placeholder);

    return result;
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
    if (arity > MAX_ARGC)
        throw K_error("too many arguments");

    args_.resize(arity);
    transform(typeCodes, typeCodes + arity, args_.begin(),
        [](char t) { return mapType(t); });
}

#ifdef PLATFORM_X86
void
q_ffi::Invocator::verifyArgumentTypes(char const* funcName, ffi_abi abi)
{
    static const regex mangling{ R"([_@].+@(\d+))" };
    static constexpr auto PATTERN_CAPS = 1;
    cmatch matches;
    switch (abi) {
    case FFI_STDCALL:
    case FFI_FASTCALL:
        if (regex_match(funcName, matches, mangling)) {
            assert(matches.size() == 1 + PATTERN_CAPS);
            auto const paramSize = stoi(matches[1]);
            auto argSize = 0;
            for (auto const& arg : args_)
                argSize += max(arg->size(), sizeof(ffi_arg));
            if (paramSize != argSize) {
                ostringstream buffer;
                buffer << "incorrect argument spec? ("
                    << "actual=" << paramSize << ' '
                    << "expected=" << argSize << ')';
                throw K_error(buffer.str());
            }
        }
        else {
            //FIXME: Not MSVC default mangling
        }
        break;
    case FFI_MS_CDECL:
    default:
        // Function names not mangled
        break;
    }
}
#else
void
q_ffi::Invocator::verifyArgumentTypes(char const* /*funcName*/, ffi_abi /*abi*/)
{}
#endif

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
    switch (typeCode) {
    case ' ':
        return make_unique<VoidArgument>();
    case 'b':
        return make_unique<SimpleArgument<kBoolean>>(ffi_type_sint);
    case 'x':
        return make_unique<SimpleArgument<kByte>>(ffi_type_uint8);
    case 'c':
        return make_unique<SimpleArgument<kChar>>(ffi_type_uchar);
    case 'h':
        return make_unique<SimpleArgument<kShort>>(ffi_type_sint16);
    case 'i':
        return make_unique<SimpleArgument<kInt>>(ffi_type_sint32);
    case 'j':
        return make_unique<SimpleArgument<kLong>>(ffi_type_sint64);
    case 'e':
        return make_unique<SimpleArgument<kReal>>(ffi_type_float);
    case 'f':
        return make_unique<SimpleArgument<kFloat>>(ffi_type_double);
    default:
        ostringstream buffer;
        buffer << "unsupported type code: '" << typeCode << "'";
        throw K_error(buffer.str());
    }
}

#pragma endregion

#pragma region q_ffi::loadFun(...) implementations

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
        using wrapped_ptr = TypeTraits<kLong>;
        constexpr auto ptr_size = sizeof(&caller);
        static_assert(ptr_size <= sizeof(wrapped_ptr::value_type),
            "wrap q_ffi::Invocator* into ::J");

        auto wrapped = wrapped_ptr::list({ 0 });
        auto const pc = &caller;
        memcpy(wrapped_ptr::index(wrapped), &pc, ptr_size);

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
q_ffi::loadFun(::K dllSym, ::K funName, ::K abi, ::K ret, ::K args)
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
    if (caller.rank() >= q_ffi::MAX_ARGC) {
        ostringstream buffer;
        buffer << "too many arguments (<=" << (q_ffi::MAX_ARGC - 1) << " expected)";
        throw K_error(buffer.str());
    }
    else {
        return wrapCaller(caller);
    }
}

#pragma endregion

#pragma region q_ffi::getVar(...) and q_ffi::setVar(...) implementations

namespace
{
    q_ffi::Invocator& createAccessor(::K dllSym, ::K varName, ::K typeCode)
    {
        auto const dll = q2String(dllSym);
        auto const var = q2String(varName);
        auto& accessor = createCaller(dll, var);

        auto const varType = q2Char(typeCode);
        accessor.load(var.c_str(), varType);

        return accessor;
    }

}//namespace /*anonymous*/

K_ptr
q_ffi::getVar(::K dllSym, ::K varName, ::K typeCode)
noexcept(false)
{
    auto& accessor = createAccessor(dllSym, varName, typeCode);
    return accessor();
}

K_ptr
q_ffi::setVar(::K dllSym, ::K varName, ::K typeCode, ::K val)
noexcept(false)
{
    auto& accessor = createAccessor(dllSym, varName, typeCode);
    accessor(val);
    return K_ptr{};
}

#pragma endregion
