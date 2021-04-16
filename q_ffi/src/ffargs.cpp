#include "ffargs.hpp"
#include "dlloader.hpp"

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
q_ffi::Pointer::getAddress<kSymbol>(::K k)
{
    using value_traits = TypeTraits<kSymbol>;

    if (nullptr == k)
        throw K_error("nil symbol");
    if (-value_traits::type_id != q::type(k))
        throw K_error("type: not a symbol");

    auto const ptr = value_traits::value(k);
    return qTraits::atom(*misc::ptr_alias<typename qTraits::const_pointer>(&ptr));
}

void*
q_ffi::Pointer::validate(::K addr)
{
    if (nullptr == addr)
        throw K_error("nil pointer");
    if (-qTraits::type_id != q::type(addr))
        throw q::K_error("type: not a pointer");

    auto const ptr = *misc::ptr_alias<void**>(&qTraits::value(addr));
    assert(nullptr != ptr);
    return ptr;
}

#pragma endregion

#pragma region Pointer manipulation exposed to q

K_ptr
q_ffi::address_of(::K k) noexcept(false)
{
#   define GET_ADDR_CASE(kType) \
        case (kType):   \
            return Pointer::getAddress<(kType)>(k)

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
        return Pointer::getAddress<kSymbol>(k);
    default:
        ostringstream buffer;
        buffer << "type: " << type(k) << "h cannot get address";
        throw K_error(buffer.str());
    }
}

K_ptr
q_ffi::get_from_address(::K addr, ::K typ) noexcept(false)
{
#   define GET_FROM_ADDR_CASE(tCode, kType) \
        case (tCode):   \
            return Pointer::getFromAddress<(kType)>(addr)

    switch (q2Char(typ)) {
        GET_FROM_ADDR_CASE('b', kBoolean);
        GET_FROM_ADDR_CASE('x', kByte);
        GET_FROM_ADDR_CASE('c', kChar);
        GET_FROM_ADDR_CASE('h', kShort);
        GET_FROM_ADDR_CASE('i', kInt);
        GET_FROM_ADDR_CASE('j', kLong);
        GET_FROM_ADDR_CASE('e', kReal);
        GET_FROM_ADDR_CASE('f', kFloat);
        GET_FROM_ADDR_CASE('s', kSymbol);
    default:
        ostringstream buffer;
        buffer << "type: \"" << q2Char(typ) << "\" cannot be get from address";
        throw K_error(buffer.str());
    }
}

void
q_ffi::set_to_address(::K addr, ::K val) noexcept(false)
{
#   define SET_TO_ADDR_CASE(kType)  \
        case -(kType):   \
            return Pointer::setToAddress<(kType)>(addr, val)

    switch (type(val)) {
        SET_TO_ADDR_CASE(kBoolean);
        SET_TO_ADDR_CASE(kByte);
        SET_TO_ADDR_CASE(kChar);
        SET_TO_ADDR_CASE(kShort);
        SET_TO_ADDR_CASE(kInt);
        SET_TO_ADDR_CASE(kLong);
        SET_TO_ADDR_CASE(kReal);
        SET_TO_ADDR_CASE(kFloat);
        SET_TO_ADDR_CASE(kSymbol);
    default:
        ostringstream buffer;
        buffer << "type: " << type(val) << "h cannot be set to address";
        throw K_error(buffer.str());
    }
}

namespace
{
    K_ptr
    addressOf(q_ffi::DLLoader& loader, ::K varName) noexcept(false)
    {
        try {
            auto const var = q2String(varName);
            auto const ptr = loader.locateProc(var.c_str());
            assert(nullptr != ptr);

            using pointer_traits = q_ffi::TypeCode<sizeof(ptr)>::traits;
            return pointer_traits::atom(
                    *misc::ptr_alias<pointer_traits::const_pointer>(&ptr));
        }
        catch (runtime_error const& ex) {
            throw K_error(ex);
        }
    }
}

K_ptr
q_ffi::get_variable(::K dllSym, ::K varName, ::K typ) noexcept(false)
{
    auto const dll = q2String(dllSym);
    q_ffi::DLLoader loader{ dll.c_str() };
    return get_from_address(addressOf(loader, varName).get(), typ);
}

void
q_ffi::set_variable(::K dllSym, ::K varName, ::K val) noexcept(false)
{
    auto const dll = q2String(dllSym);
    q_ffi::DLLoader loader{ dll.c_str() };
    set_to_address(addressOf(loader, varName).get(), val);
}

#pragma endregion
