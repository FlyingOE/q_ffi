#pragma once

#include <functional>
#include <ffi.h>
#include "kerror.hpp"
#include "kpointer.hpp"
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
        ffi_type& type() const
        { return type_; }

        /// @brief Return a pointer to the value of the parameter
        ///     (may or may not point to the K object, depending on type matching)
        virtual void* get(::K k) const = 0;

        /// @brief Return a pointer to the value of the parameter from a q array
        ///     (may or may not point to the K value, depending on type matching)
        virtual void* get(::K k, std::size_t index) const = 0;

        /// @brief Put an FFI argument's value back into the parameter
        virtual void set(::K k, ffi_arg const& x) const = 0;

        /// @brief Size (in bytes) of the given argument type
        virtual std::size_t size() const = 0;

        /// @brief Create a new K oject of the given argument type
        virtual ::K create() const = 0;
    };

    class VoidArgument : public Argument
    {
    public:
        VoidArgument() : Argument(ffi_type_void)
        {}

        void* get(::K) const override
        { return nullptr; }

        void* get(::K, std::size_t) const override
        { return nullptr; }

        void set(::K, ffi_arg const&) const override
        {}

        std::size_t size() const override
        { return 0; }

        ::K create() const override
        { return q::Nil; }
    };

    template<typename Tr, typename S>
    class AtomArgument : public Argument
    {
    public:
        using converter_type = S(*)(::K, bool);
        using list_converter_type = std::vector<S>(*)(::K, bool);

    private:
        converter_type convert_atom_;
        list_converter_type convert_list_;
        mutable q::K_ptr mapped_;

    public:
        AtomArgument(ffi_type& type,
            converter_type convert_atom, list_converter_type convert_list)
            : Argument(type), convert_atom_(convert_atom), convert_list_(convert_list)
        {
            assert(nullptr != convert_atom_);
            assert(nullptr != convert_list_);
        }

        void* get(::K k) const override
        {
            if (q::Nil == k)
                return nullptr;

            if (-Tr::type_id == q::type(k))
                return getDirect(k);
            else
                return getMapped(k);
        }

        void* get(::K k, std::size_t index) const override
        {
            if (q::Nil == k)
                return nullptr;

            if (Tr::type_id == q::type(k))
                return getDirect(k, index);
            else
                return getMapped(k, index);
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

        ::K create() const override
        {
            static constexpr uint64_t DUMMY = 0xDEAD'BEEF'CCCC'CCCCuLL;
            static_assert(sizeof(DUMMY) >= sizeof(typename Tr::value_type),
                "ensure dummy bytes are filled");
            mapped_.reset(Tr::atom(*reinterpret_cast<typename Tr::const_pointer>(&DUMMY)));
            return mapped_.get();
        }

    private:
        void* getDirect(::K k) const
        {
            mapped_.reset();
            return &Tr::value(k);
        }

        void* getMapped(::K k) const
        {
            using value_type = Tr::value_type;
            ::K k2 = create();
            Tr::value(k2) = static_cast<value_type>(convert_atom_(k, false));
            return &Tr::value(k2);
        }

        void* getDirect(::K k, std::size_t index) const
        {
            mapped_.reset();
            assert(index < q::count(k));
            return Tr::index(k) + index;
        }

        /// FIXME: Rather inefficient, as parameter list is converted multiple times!
        void* getMapped(::K k, std::size_t index) const
        {
            using value_type = Tr::value_type;
            ::K k2 = create();
            Tr::value(k2) = static_cast<value_type>(convert_list_(k, false)[index]);
            return &Tr::value(k2);
        }
    };

}//namespace q_ffi
