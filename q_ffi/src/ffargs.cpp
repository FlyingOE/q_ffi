#include "ffargs.hpp"

using namespace q;
using namespace std;

#pragma region q_ffi::Argument & q_ffi::Parameter base implementations

void
q_ffi::Parameter::set(K_ptr const& value)
{
    this->set(value.get());
}

ffi_type&
q_ffi::Argument::type() const
{
    return type_;
}

unique_ptr<q_ffi::Parameter>
q_ffi::Argument::map(K_ptr const& k, bool asReturn) const
{
    return this->map(k.get(), asReturn);
}

#pragma endregion

#pragma region q_ffi::Void implementation

void*
q_ffi::Void::Param::get()
{
    return nullptr;
}

void
q_ffi::Void::Param::set(::K)
{
}

K_ptr
q_ffi::Void::Param::release()
{
    return K_ptr{};
}

unique_ptr<q_ffi::Parameter>
q_ffi::Void::map(::K, bool) const
{
    return make_unique<Param>();
}

unique_ptr<q_ffi::Parameter>
q_ffi::Void::create(bool) const
{
    return make_unique<Param>();
}

#pragma endregion

#pragma region q_ffi::Atom<q::kSymbol> implementation

q_ffi::Atom<kSymbol>::Param::Param(::K k)
    : param_{ dup_K(k) }, str_{ nullptr }
{
    validate(param_.get());
    str_ = qTraits::value(param_);
}

q_ffi::Atom<kSymbol>::Param::~Param()
{
    // No write-back, as symbol atoms are read-only!
}

void*
q_ffi::Atom<kSymbol>::Param::get()
{
    return &str_;
}

void
q_ffi::Atom<kSymbol>::Param::set(::K)
{
    throw K_error("nyi: symbol atoms are read-only!");
}

K_ptr
q_ffi::Atom<kSymbol>::Param::release()
{
    throw K_error("nyi: symbol atoms are read-only!");
}

void
q_ffi::Atom<kSymbol>::Param::validate(::K k)
{
    if (nullptr == k) {
        throw K_error("nil atomic symbol");
    }

    if (-kSymbol != q::type(k)) {
        ostringstream buffer;
        buffer << "type: atomic symbol (" << -kSymbol << "h expected)";
        throw K_error(buffer.str());
    }
}

unique_ptr<q_ffi::Parameter>
q_ffi::Atom<kSymbol>::map(::K k, bool asReturn) const
{
    if (asReturn)
        throw K_error("nyi: symbol atoms are read-only!");
    else
        return make_unique<Param>(k);
}

unique_ptr<q_ffi::Parameter>
q_ffi::Atom<kSymbol>::create(bool) const
{
    throw K_error("nyi: symbol atoms are read-only!");
}

#pragma endregion

#pragma region q_ffi::Pointer implementation

template<>
K_ptr
q_ffi::Pointer::toAddress<kSymbol>(::K k)
{
    if (nullptr == k)
        throw K_error("nil symbol");
    if (-kSymbol != q::type(k))
        throw K_error("type: not a symbol");

    auto const ptr = TypeTraits<kSymbol>::value(k);
    return qTraits::atom(*misc::ptr_alias<typename qTraits::const_pointer>(&ptr));
}

#pragma endregion
/*
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

K_ptr
q_ffi::PointerArgument::create(size_t& bytes) const
{
    bytes = sizeof(pointer_traits::value_type);
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
*/

#pragma region FFI address handling

K_ptr
q_ffi::to_addr(::K k) noexcept(false)
{
#   define GET_ADDR_CASE(kType) \
        case (kType):   \
            return Pointer::toAddress<(kType)>(k)

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
        return Pointer::toAddress<kSymbol>(k);
    default:
        ostringstream buffer;
        buffer << "type: " << type(k) << "h cannot get address";
        throw K_error(buffer.str());
    }
}

/*
K_ptr
q_ffi::setAddr(::K addr, ::K k) noexcept(false)
{
#   define SET_ADDR_CASE(kType) \
        case -(kType):  \
            PointerArgument::setAddr<(kType)>(addr, k); \
            break

    switch (type(k)) {
        SET_ADDR_CASE(kBoolean);
        SET_ADDR_CASE(kByte);
        SET_ADDR_CASE(kChar);
        SET_ADDR_CASE(kShort);
        SET_ADDR_CASE(kInt);
        SET_ADDR_CASE(kLong);
        SET_ADDR_CASE(kReal);
        SET_ADDR_CASE(kFloat);
    default:
        ostringstream buffer;
        buffer << "cannot set value from address for " << type(k) << "h";
        throw K_error(buffer.str());
    }
    return K_ptr{};
}

*/
