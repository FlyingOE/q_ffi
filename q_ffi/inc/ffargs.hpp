#pragma once

#include <functional>
#include <ffi.h>
#include "kerror.hpp"
#include "ktype_traits.hpp"
#include "type_convert.hpp"
#include "ffargs.hpp"

namespace q_ffi
{
    /// @brief FFI callback argument descriptor
    class Argument
    {
    private:
        std::reference_wrapper<ffi_type> type_;

    public:
        Argument(ffi_type& type) : type_{ type }
        {}

        virtual ~Argument()
        {}

        /// @brief Return the FFI type of the argument
        ffi_type& type() const;

        /// @brief Return a pointer to the value of the parameter
        ///     (may or may not point to the K object, depending on type matching)
        virtual void* get(::K k) const = 0;
        virtual void* get(q::K_ptr const& k) const;

        /// @brief Put an FFI argument's value back into the parameter
        virtual void set(::K k, ffi_arg const& x) const = 0;
        virtual void set(q::K_ptr& k, ffi_arg const& x) const;

        /// @brief Size (in bytes) of the given argument type
        virtual std::size_t size() const = 0;

        /// @brief Create a new K oject of the given argument type
        virtual q::K_ptr create() const = 0;
    };

    class VoidArgument : public Argument
    {
    public:
        VoidArgument() : Argument(ffi_type_void)
        {}

        void* get(::K) const override;

        void set(::K, ffi_arg const&) const override;

        std::size_t size() const override;

        q::K_ptr create() const override;
    };

    template<typename Tr, typename S>
    class SimpleArgument : public Argument
    {
    public:
        using converter_type = S(*)(::K, bool);

    private:
        mutable q::K_ptr mapped_;

        converter_type convert_atom_;

    public:
        SimpleArgument(ffi_type& type, converter_type convert_atom)
            : Argument(type), convert_atom_{ convert_atom }
        {
            assert(nullptr != convert_atom_);
        }

        void* get(::K k) const override
        {
            if (q::Nil == k)
                return nullptr;

            if (-Tr::type_id == q::type(k))
                return get_direct(k);
            else
                return get_mapped(k);
        }

        void set(::K k, ffi_arg const& x) const override
        {
            assert(size() < sizeof(ffi_arg));
            Tr::value(k) = *reinterpret_cast<typename Tr::const_pointer>(&x);
        }

        std::size_t size() const override
        {
            return sizeof(typename Tr::value_type);
        }

        q::K_ptr create() const override
        {
            q::K_ptr val{ Tr::atom(0) };
#       ifndef NDEBUG
            constexpr auto DUMMY_VALUE = 0x8BAD'F00D'DEAD'BEEFuLL;
            static_assert(sizeof(DUMMY_VALUE) >= sizeof(typename Tr::value_type),
                "ensure dummy bytes are filled");
            Tr::value(val) =
                *reinterpret_cast<typename Tr::const_pointer>(&DUMMY_VALUE);
#       endif
            return val;
        }

    private:
        void* get_direct(::K k) const
        {
            assert(-Tr::type_id == q::type(k));
            return &Tr::value(k);
        }

        void* get_mapped(::K k) const
        {
            mapped_ = create();
            assert(mapped_);
            Tr::value(mapped_) =
                static_cast<typename Tr::value_type>(convert_atom_(k, false));
            return get_direct(mapped_.get());
        }
    };

}//namespace q_ffi
