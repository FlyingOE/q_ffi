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
