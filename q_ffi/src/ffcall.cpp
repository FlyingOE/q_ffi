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
    : dll_(dll), func_{ nullptr }, ret_{}, args_{}, ffi_{}
{}

size_t
q_ffi::Invocator::rank() const
{
    return args_.size();
}

void
q_ffi::Invocator::load(char const* func, char retType, char const* argTypes, char const* abiType)
{
    try {
        func_ = dll_.locateProc<function_type>(func);
        setReturnType(retType);
        setArgumentTypes(argTypes);
        auto const abi = mapABI(abiType, func);
#   ifdef PLATFORM_X86
        verifyArgumentTypes(func, abi);
#   endif
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
    if (params.size() != rank && !(params.size() == 1 && rank == 0))
        throw K_error("rank");

    auto pars = make_unique<void*[]>(rank);
    transform(args_.cbegin(), args_.cend(), params.begin(), pars.get(),
        [](auto const& arg, auto const& par) { return arg->get(par); });

    return invoke(pars.get());
}

K_ptr
q_ffi::Invocator::invoke(void* params[])
{
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
    ffi_call(&ffi_.cif, func_, res, params);

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

void
q_ffi::Invocator::verifyArgumentTypes(char const* funcName, ffi_abi abi)
{
#ifdef PLATFORM_X86
    switch (abi) {
    case FFI_STDCALL:
    case FFI_FASTCALL: {
            static const regex mangling{ R"([_@].+@(\d+))" };
            static constexpr auto PATTERN_CAPS = 1;
            cmatch matches;
            if (regex_match(funcName, matches, mangling)) {
                assert(matches.size() == 1 + PATTERN_CAPS);
                auto const paramSize = stoi(matches[1]);
                auto argSize = 0;
                for (auto const& arg : args_)
                    argSize += arg->size();
                if (paramSize != argSize)
                    throw K_error("incorrect argument spec?");
            }
        } break;
    case FFI_MS_CDECL:
    default:
        // Function names not mangled
        break;
    }
#endif
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

ffi_abi
q_ffi::Invocator::guessABI(char const* funcName)
{
#ifdef PLATFORM_X86
    assert(nullptr != funcName);

    static const regex stdcall{ R"(_.+@(\d+))" };
    if (regex_match(funcName, stdcall))
        return FFI_STDCALL;

    static const regex fastcall{ R"(@.+@(\d+))" };
    if (regex_match(funcName, fastcall))
        return FFI_FASTCALL;

    return FFI_MS_CDECL;
#else
    return FFI_DEFAULT_ABI;
#endif
}

q_ffi::Invocator::argument_type
q_ffi::Invocator::mapType(char typeCode)
{
    constexpr long long(*convert_decimal)(::K, bool) = &q2Decimal;
    constexpr double(*convert_real)(::K, bool) = &q2Real;
    constexpr vector<long long>(*convert_decimals)(::K, bool) = &q2Decimals;
    constexpr vector<double>(*convert_reals)(::K, bool) = &q2Reals;

    switch (typeCode) {
    case ' ':
        return make_unique<VoidArgument>();
    case 'b':
        return make_unique<AtomArgument<TypeTraits<kBoolean>, long long>>(
            ffi_type_sint, convert_decimal, convert_decimals);
    case 'x':
        return make_unique<AtomArgument<TypeTraits<kByte>, long long>>(
            ffi_type_uint8, convert_decimal, convert_decimals);
    case 'h':
        return make_unique<AtomArgument<TypeTraits<kShort>, long long>>(
            ffi_type_sint16, convert_decimal, convert_decimals);
    case 'i':
        return make_unique<AtomArgument<TypeTraits<kInt>, long long>>(
            ffi_type_sint32, convert_decimal, convert_decimals);
    case 'j':
        return make_unique<AtomArgument<TypeTraits<kLong>, long long>>(
            ffi_type_sint64, convert_decimal, convert_decimals);
    case 'e':
        return make_unique<AtomArgument<TypeTraits<kReal>, double>>(
            ffi_type_float, convert_real, convert_reals);
    case 'f':
        return make_unique<AtomArgument<TypeTraits<kFloat>, double>>(
            ffi_type_double, convert_real, convert_reals);
    default:
        ostringstream buffer;
        buffer << "unsupported type code: '" << typeCode << "'";
        throw K_error(buffer.str());
    }
}
