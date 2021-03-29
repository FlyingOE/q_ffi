#include <sstream>
#include "kerror.hpp"
#include "ffcall.hpp"

using namespace q;
using namespace std;

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

q_ffi::Invocator::Invocator(char const* dll)
    : dll_(dll), func_{ nullptr }, return_{}, args_{}, ffi_{}
{}

size_t
q_ffi::Invocator::arity() const
{
    return args_.size();
}

void
q_ffi::Invocator::load(char const* func, char retType, char const* argTypes, char const* abiType)
{
    try {
        func_ = dll_.locateProc<function_type>(func);
        return_ = mapReturnSpec(retType);
        args_ = mapArgumentSpecs(argTypes);
        auto const abi = mapABI(abiType, func);
        prepareFuncSpec(abi);
    }
    catch (runtime_error const& ex) {
        throw K_error(ex);
    }
}

::K
q_ffi::Invocator::operator()(vector<::K> const& params)
{
    auto const arity = this->arity();
    if (params.size() != arity && !(params.size() == 1 && arity == 0))
        throw K_error("rank");

    auto pars = make_unique<void*[]>(arity);
    auto p = params.cbegin();
    for (auto i = 0u; i < arity; ++i, ++p) {
        assert(args_[i]);
        pars[i] = args_[i]->get(*p);
        assert(nullptr != pars[i]);
    }
    return invoke(pars.get());
}

::K
q_ffi::Invocator::operator()(::K params)
{
    auto const arity = this->arity();
    if (count(params) != arity && !(count(params) == 1 && arity == 0))
        throw K_error("rank");

    auto pars = make_unique<void*[]>(arity);
    for (auto i = 0u; i < arity; ++i) {
        assert(args_[i]);
        pars[i] = args_[i]->get(params, i);
        assert(nullptr != pars[i]);
    }
    return invoke(pars.get());
}

::K
q_ffi::Invocator::invoke(void* params[])
{
    assert(params);

    // Prepare result
    assert(return_);
    K_ptr result{ return_->create() };
    unique_ptr<ffi_arg> placeholder;
    void* res;
    if (result.get() == Nil) {
        res = nullptr;
    }
    else if (return_->size() < sizeof(ffi_arg)) {
        placeholder = make_unique<ffi_arg>();
        res = placeholder.get();
    }
    else {
        res = return_->get(result.get());
    }

    // FFI invocation
    ffi_call(&ffi_.cif, func_, res, params);

    // Fetch result, if necessary
    if (placeholder)
        return_->set(result.get(), *placeholder);

    return result.release();
}

void
q_ffi::Invocator::prepareFuncSpec(ffi_abi abi)
{
    assert(return_);
    ffi_.ret_type = &return_->type();

    auto const arity = this->arity();
    ffi_.arg_types = make_unique<ffi_type*[]>(arity);
    transform(args_.cbegin(), args_.cend(), ffi_.arg_types.get(),
        [](auto const& arg) { return &arg->type(); });

    auto const status = ffi_prep_cif(
        &ffi_.cif, abi, args_.size(), ffi_.ret_type, ffi_.arg_types.get());
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

q_ffi::Invocator::argument_type
q_ffi::Invocator::mapReturnSpec(char typeCode)
{
    argument_type ret{ createArgument(typeCode) };
    assert(ret);
    return ret;
}

vector<q_ffi::Invocator::argument_type>
q_ffi::Invocator::mapArgumentSpecs(char const* typeCodes)
{
    assert(nullptr != typeCodes);
    auto const arity = strlen(typeCodes);
    vector<argument_type> args{ arity };
    for (auto i = 0u; i < arity; ++i) {
        args[i].reset(createArgument(typeCodes[i]));
        assert(args[i]);
    }
    return args;
}

ffi_abi
q_ffi::Invocator::mapABI(char const* abiType, char const* funcName)
{
    ffi_abi abi = static_cast<ffi_abi>(FFI_FIRST_ABI - 1);
    if (nullptr != abiType) {
#ifdef PLATFORM_X86
        if (0 == strcmp(abiType, "")) {
            abi = guessABI(funcName);
        }
        if (0 == strcmp(abiType, "CDECL")) {
            abi = FFI_MS_CDECL;
        }
        else if (0 == strcmp(abiType, "STDCALL")) {
            abi = FFI_STDCALL;
        }
        else if (0 == strcmp(abiType, "FASTCALL")) {
            abi = FFI_FASTCALL;
        }
        else {
            ostringstream buffer;
            buffer << "unknown ABI: `" << abiType << "'";
            throw K_error(buffer.str());
        }
#endif
    }
    else {
        abi = guessABI(funcName);
    }
    assert(FFI_FIRST_ABI <= abi && abi <= FFI_LAST_ABI);
    return abi;
}

ffi_abi
q_ffi::Invocator::guessABI(char const* funcName)
{
#ifdef PLATFORM_X86
    assert(nullptr != funcName);

    regex stdcall{ R"(_.+@\d+)" };
    if (regex_match(funcName, stdcall))
        return FFI_STDCALL;

    regex fastcall{ R"(@.+@\d+)" };
    if (regex_match(funcName, fastcall))
        return FFI_FASTCALL;

    return FFI_MS_CDECL;
#else
    return FFI_DEFAULT_ABI;
#endif
}

q_ffi::Argument*
q_ffi::Invocator::createArgument(char typeCode)
{
    switch (typeCode) {
    case ' ':
        return new VoidArgument;
    case 'b':
        return new AtomArgument<TypeTraits<kBoolean>, long long>{
            ffi_type_sint, &q2Decimal, &q2Decimals
        };
    case 'x':
        return new AtomArgument<TypeTraits<kByte>, long long>{
            ffi_type_uint8, &q2Decimal, &q2Decimals
        };
    case 'h':
        return new AtomArgument<TypeTraits<kShort>, long long>{
            ffi_type_sint16, &q2Decimal, &q2Decimals
        };
    case 'i':
        return new AtomArgument<TypeTraits<kInt>, long long>{
            ffi_type_sint32, &q2Decimal, &q2Decimals
        };
    case 'j':
        return new AtomArgument<TypeTraits<kLong>, long long>{
            ffi_type_sint64, &q2Decimal, &q2Decimals
        };
    case 'e':
        return new AtomArgument<TypeTraits<kReal>, double>{
            ffi_type_float, &q2Real, &q2Reals
        };
    case 'f':
        return new AtomArgument<TypeTraits<kFloat>, double>{
            ffi_type_double, &q2Real, &q2Reals
        };
    default:
        ostringstream buffer;
        buffer << "unsupported type code: '" << typeCode << "'";
        throw K_error(buffer.str());
    }
}
