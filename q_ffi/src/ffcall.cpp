#include <cctype>
#include "ffcall.hpp"

using namespace q;

#if defined(_WIN32)
#   if defined(_WIN64)
#       define PLATFORM_X86_64
#   else
#       define PLATFORM_X86
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

q_ffi::Invocator::Invocator(q_ffi::Invocator::function_type func,
    char resType, char const* argTypes, char const* abiType)
    : func_{ func }, cif_{}, args_{ nullptr }
    , return_{ mapReturnSpec(resType) }, params_{ mapArgumentSpecs(argTypes) }
{
    assert(nullptr != func);

    ffi_abi abi = FFI_DEFAULT_ABI;
    if (nullptr != abiType) {
#   ifdef PLATFORM_X86
        std::string abiId{ abiType };
        std::transform(abiId.cbegin(), abiId.cend(), abiId.begin(),
            [](char c) { return static_cast<char>(std::toupper(c)); });
        if (abiId == "CDECL") {
            abi = FFI_MS_CDECL;
        }
        else if (abiId == "STDCALL") {
            abi = FFI_STDCALL;
        }
        else if (abiId == "FASTCALL") {
            abi = FFI_FASTCALL;
        }
#   endif
    }
    prepareFuncSpec(abi);
}

void
q_ffi::Invocator::prepareFuncSpec(ffi_abi abi)
{
    auto const arity = params_.size();
    args_.reset(new ffi_type*[arity]);
    std::transform(params_.cbegin(), params_.cend(), args_.get(),
        [](auto const& arg) { return &arg->type(); });

    auto const status =
        ffi_prep_cif(&cif_, abi, params_.size(), &return_->type(), args_.get());
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
q_ffi::Invocator::mapReturnSpec(char type)
{
    argument_type ret{ createArgument(type) };
    assert(ret);
    return ret;
}

std::vector<q_ffi::Invocator::argument_type>
q_ffi::Invocator::mapArgumentSpecs(char const* types)
{
    auto const arity = std::strlen(types);
    std::vector<argument_type> args{ arity };
    for (auto i = 0u; i < arity; ++i) {
        args[i].reset(createArgument(types[i]));
        assert(args[i]);
    }
    return args;
}

q_ffi::Argument*
q_ffi::Invocator::createArgument(char typeCode)
{
    switch (typeCode) {
    case ' ':
        return new VoidArgument;
    case 'b':
        return new AtomArgument<TypeTraits<kBoolean>, long long>{
            ffi_type_sint, &q2Decimal
        };
    case 'x':
        return new AtomArgument<TypeTraits<kByte>, long long>{
            ffi_type_uint8, &q2Decimal
        };
    case 'h':
        return new AtomArgument<TypeTraits<kShort>, long long>{
            ffi_type_sint16, &q2Decimal
        };
    case 'i':
        return new AtomArgument<TypeTraits<kInt>, long long>{
            ffi_type_sint32, &q2Decimal
        };
    case 'j':
        return new AtomArgument<TypeTraits<kLong>, long long>{
            ffi_type_sint64, &q2Decimal
        };
    case 'e':
        return new AtomArgument<TypeTraits<kReal>, double>{
            ffi_type_float, &q2Real
        };
    case 'f':
        return new AtomArgument<TypeTraits<kFloat>, double>{
            ffi_type_double, &q2Real
        };
    default:
        std::ostringstream buffer;
        buffer << "unsupported type code: '" << typeCode << "'";
        throw K_error(buffer.str());
    }
}

::K
q_ffi::Invocator::operator()(std::vector<::K> const& params) noexcept(false)
{
    auto const arity = params_.size();
    if (params.size() != arity)
        throw K_error("rank");

    // Prepare parameters
    std::unique_ptr<void*[]> pars{ new void*[arity] };
    for (auto i = 0u; i < arity; ++i) {
        assert(params_[i]);
        pars[i] = params_[i]->get(params[i]);
        assert(nullptr != pars[i]);
    }

    // Prepare result
    assert(return_);
    K_ptr result{ return_->create() };
    ffi_arg placeholder{};
    bool usePlaceholder = false;
    void* res;
    if (result.get() == Nil) {
        res = nullptr;
    }
    else if (return_->size() < sizeof(ffi_arg)) {
        res = &placeholder;
        usePlaceholder = true;
    }
    else {
        res = return_->get(result.get());
    }

    // FFI invocation
    ffi_call(&cif_, func_, res, pars.get());

    // Fetch result, if necessary
    if (usePlaceholder)
        return_->set(result.get(), placeholder);

    return result.release();
}
