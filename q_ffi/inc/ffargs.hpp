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

    template<q::TypeId tid>
    class SimpleArgument : public Argument
    {
    private:
        using TypeTraits = q::TypeTraits<tid>;
        using value_type = typename TypeTraits::value_type;
        using const_reference = typename TypeTraits::const_reference;
        using const_pointer = typename TypeTraits::const_pointer;

    public:
        SimpleArgument(ffi_type& type) : Argument(type)
        {}

        void* get(::K k) const override
        {
            validate(k);
            return &TypeTraits::value(k);
        }

        void set(::K k, ffi_arg const& x) const override
        {
            validate(k);

            auto const p = reinterpret_cast<char const*>(&x);
            assert(nullptr != p);
            assert(sizeof(ffi_arg) > size());
            TypeTraits::value(k) = *reinterpret_cast<const_pointer>(p);
        }

        std::size_t size() const override
        {
            return sizeof(value_type);
        }

        q::K_ptr create() const override
        {
            q::K_ptr k{ TypeTraits::atom(0) };
#       ifndef NDEBUG
            constexpr auto DUMMY_BYTES = 0x8BAD'F00D'DEAD'BEEFuLL;
            auto const p = reinterpret_cast<char const*>(&DUMMY_BYTES);
            assert(nullptr != p);
            assert(sizeof(DUMMY_BYTES) >= size());
            TypeTraits::value(k) = *reinterpret_cast<const_pointer>(p);
#       endif
            return k;
        }

    private:
        static void validate(::K k)
        {
            if (q::Nil == k)
                throw q::K_error("nil: simple argument");

            if (-TypeTraits::type_id != q::type(k)) {
                std::ostringstream buffer;
                buffer << "type: simple argument"
                    " (" << -TypeTraits::type_id << "h expected)";
                throw q::K_error(buffer.str());
            }
        }
    };

}//namespace q_ffi
