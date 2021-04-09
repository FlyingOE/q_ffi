#include "ffargs.hpp"

using namespace q;
using namespace std;

#pragma region q_ffi::Argument implementation

ffi_type&
q_ffi::Argument::type() const
{
    return type_;
}

void*
q_ffi::Argument::get(K_ptr const& k) const
{
    return this->get(k.get());
}

void
q_ffi::Argument::set(K_ptr& k, ffi_arg const& x) const
{
    this->set(k.get(), x);
}

#pragma endregion

#pragma region q_ffi::VoidArgument implementation

void*
q_ffi::VoidArgument::get(::K) const
{
    return nullptr;
}

void
q_ffi::VoidArgument::set(::K, ffi_arg const&) const
{}

size_t
q_ffi::VoidArgument::size() const
{
    return 0;
}

K_ptr
q_ffi::VoidArgument::create() const
{
    return K_ptr{};
}

#pragma endregion

#pragma region q_ffi::PointerArgument implementation

void*
q_ffi::PointerArgument::get(::K k) const
{
    using pointer_traits = TypeCode<sizeof(void*)>::traits;
    validate<-pointer_traits::type_id>(k);
    return misc::ptr_alias<void*>(&pointer_traits::value(k));
}

void
q_ffi::PointerArgument::set(::K, ffi_arg const&) const
{
    throw K_error("type: invalid for a pointer");
}

size_t
q_ffi::PointerArgument::size() const
{
    return sizeof(void*);
}

K_ptr
q_ffi::PointerArgument::create() const
{
    assert(sizeof(pointer_traits::value_type) >= size());
    return pointer_traits::atom(0);
}

template<>
K_ptr
q_ffi::PointerArgument::getAddr<kSymbol>(::K k)
{
    validate<-kSymbol>(k);
    auto const ptr = TypeTraits<kSymbol>::value(k);
    return pointer_traits::atom(
        *misc::ptr_alias<pointer_traits::const_pointer>(&ptr));
}

#pragma endregion

#pragma region q_ffi::getAddr(...) implementation

K_ptr
q_ffi::getAddr(::K k) noexcept(false)
{
#   define GET_ADDR_CASE(kType) \
        case (kType):   \
            return PointerArgument::getAddr<(kType)>(k)

    switch (type(k)) {
        GET_ADDR_CASE(kBoolean);
        GET_ADDR_CASE(kByte);
        GET_ADDR_CASE(kChar);
        GET_ADDR_CASE(kShort);
        GET_ADDR_CASE(kInt);
        GET_ADDR_CASE(kLong);
        GET_ADDR_CASE(kReal);
        GET_ADDR_CASE(kFloat);
    case -kSymbol:
        return PointerArgument::getAddr<kSymbol>(k);
    default:
        ostringstream buffer;
        buffer << "invalid type for FFI pointer: " << type(k) << "h";
        throw K_error(buffer.str());
    }
}

#pragma endregion
