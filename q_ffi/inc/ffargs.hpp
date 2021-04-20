#pragma once

#include <ffi.h>
#include "kerror.hpp"
#include "ktype_traits.hpp"
#include "type_convert.hpp"
#include "ffargs.hpp"

namespace q_ffi
{
#pragma region Memory address access

    q::K_ptr address_of(::K k) noexcept(false);

    q::K_ptr get_from_address(::K addr, ::K typ) noexcept(false);

    void set_to_address(::K addr, ::K val) noexcept(false);

    void free_address(::K addr) noexcept(false);

#pragma endregion

#pragma region FFI variable access

    q::K_ptr get_variable(::K dllSym, ::K varName, ::K typ) noexcept(false);

    void set_variable(::K dllSym, ::K varName, ::K val) noexcept(false);

#pragma endregion

#pragma region Data size (in bytes) to q type code mapping
    template<std::size_t bytes>
    struct TypeCode;

    template<typename T>
    constexpr q::K_ptr get_type() noexcept
    {
        constexpr auto typeId = TypeCode<sizeof(T)>::traits::type_id;
        return q::TypeTraits<kChar>::atom(q::TypeId2Code.at(typeId));
    }

    template<typename T>
    constexpr q::K_ptr get_size() noexcept
    {
        using traits = TypeCode<sizeof(T)>::traits;
        return traits::atom(sizeof(typename traits::value_type));
    }

    template<>
    struct TypeCode<sizeof(q::TypeTraits<q::kInt>::value_type)>
    {
        using traits = q::TypeTraits<q::kInt>;
    };

    template<>
    struct TypeCode<sizeof(q::TypeTraits<q::kLong>::value_type)>
    {
        using traits = q::TypeTraits<q::kLong>;
    };

#pragma endregion

#pragma region Argument descriptor & parameter mapping

    /// @brief FFI callback parameter mapper
    class Parameter
    {
    public:
        virtual ~Parameter() {}

        virtual void* get() = 0;

        virtual void set(::K value) = 0;
        virtual void set(q::K_ptr const& value);

        virtual q::K_ptr release() = 0;
    };

    /// @brief FFI callback argument descriptor
    class Argument
    {
    private:
        std::reference_wrapper<ffi_type> type_;

    public:
        Argument(ffi_type& type) : type_{ type } {}

        virtual ~Argument() {}

        /// @brief Return the FFI type of the argument
        ffi_type& type() const;

        /// @brief Map a K object as a parameter for this argument
        virtual std::unique_ptr<Parameter> map(::K k, bool asReturn = false) const = 0;
        virtual std::unique_ptr<Parameter> map(q::K_ptr const& k, bool asReturn = false) const;

        /// @brief Create a new K object as a parameter for this argument
        virtual std::unique_ptr<Parameter> create(bool asReturn = true) const = 0;
    };

    /// @brief FFI callback argument: void
    class Void : public Argument
    {
    protected:
        class Param : public Parameter
        {
        public:
            virtual ~Param() {}

            void* get() override;
            void set(::K) override;

            q::K_ptr release() override;
        };

    public:
        Void() : Argument(ffi_type_void)
        {}

        std::unique_ptr<Parameter> map(::K, bool asReturn = false) const override;
        std::unique_ptr<Parameter> create(bool asReturn = true) const override;
    };

    /// @brief FFI callback argument: atomic value
    template<q::TypeId tid>
    class Atom : public Argument
    {
    protected:
        using qTraits = q::TypeTraits<tid>;

        /// @brief Parameter placeholder in case it is smaller than ffi_arg
        class ParamPlaceholder
        {
        private:
            ffi_arg tmp_;

        public:
            void* read(q::K_ptr&)
            {
                return &tmp_;
            }

            void write(q::K_ptr&, ::K val)
            {
                *misc::ptr_alias<typename qTraits::pointer>(&tmp_) =
                    qTraits::value(val);
            }

            void writeBack(q::K_ptr& k)
            {
                if (k)
                    qTraits::value(k) =
                        *misc::ptr_alias<typename qTraits::const_pointer>(&tmp_);
            }
        };

        /// @brief Parameter by directly mapping from K object
        class ParamMapper
        {
        public:
            void* read(q::K_ptr& k)
            {
                assert(k);
                return &qTraits::value(k);
            }

            void write(q::K_ptr& k, ::K val)
            {
                assert(k);
                qTraits::value(k) = qTraits::value(val);
            }

            void writeBack(q::K_ptr&)
            {}
        };

        template<bool asReturn>
        class Param : public Parameter
        {
        private:
            using param_mapper = std::conditional_t<
                asReturn && sizeof(typename qTraits::value_type) < sizeof(ffi_arg),
                ParamPlaceholder, ParamMapper>;
            param_mapper mapper_;

            q::K_ptr param_;

        public:
            Param(::K k) : mapper_{}, param_{ q::dup_K(k) }
            {
                validate(param_.get());
                mapper_.write(param_, param_.get());
            }

            virtual ~Param()
            {
                mapper_.writeBack(param_);
            }
                
            void* get() override
            {
                if (param_)
                    return mapper_.read(param_);
                else
                    throw K_error("state: invalid atomic parameter");
            }

            void set(::K k) override
            {
                validate(k);
                if (param_)
                    return mapper_.write(param_, k);
                else
                    throw K_error("state: invalid atomic parameter");
            }

            q::K_ptr release() override
            {
                mapper_.writeBack(param_);
                return std::move(param_);
            }

        private:
            static void validate(::K k)
            {
                if (nullptr == k) {
                    throw q::K_error("nil atomic parameter value");
                }

                if (-qTraits::type_id != q::type(k)) {
                    std::ostringstream buffer;
                    buffer << "type: atomic parameter value"
                        " (" << -qTraits::type_id << "h expected)";
                    throw q::K_error(buffer.str());
                }
            }
        };

    public:
        Atom(ffi_type& type) : Argument(type)
        {}

        std::unique_ptr<Parameter> map(::K k, bool asReturn = false) const override
        {
            if (asReturn)
                return std::make_unique<Param<true>>(k);
            else
                return std::make_unique<Param<false>>(k);
        }

        std::unique_ptr<Parameter> create(bool asReturn = true) const override
        {
            auto k = qTraits::atom(0);
#       ifndef NDEBUG
            constexpr auto DUMMY_BYTES = 0x8BAD'F00D'DEAD'BEEFuLL;
            static_assert(sizeof(DUMMY_BYTES) >= sizeof(typename qTraits::value_type),
                "ensure enough dummy bytes");
            qTraits::value(k) =
                *misc::ptr_alias<typename qTraits::const_pointer>(&DUMMY_BYTES);
#       endif
            return this->map(k.get(), asReturn);
        }
    };

    /// @brief FFI callback argument: symbol
    template<>
    class Atom<q::kSymbol> : public Argument
    {
    protected:
        using qTraits = q::TypeTraits<q::kSymbol>;

        class Param : public Parameter
        {
        private:
            q::K_ptr param_;
            char const* str_;

        public:
            Param(::K k);

            void* get() override;
            void set(::K) override;
            q::K_ptr release() override;

        private:
            static void validate(::K k);
        };

    public:
        Atom() : Argument(ffi_type_pointer)
        {}

        std::unique_ptr<Parameter> map(::K k, bool asReturn = false) const override;
        std::unique_ptr<Parameter> create(bool asReturn = true) const override;

        q::K_ptr getAddress(::K k) const;
    };

    /// @brief FFI callback argument: pointer
    class Pointer : public Atom<TypeCode<sizeof(void*)>::traits::type_id>
    {
    protected:
        using Atom::qTraits;

    public:
        Pointer() : Atom(ffi_type_pointer)
        {}

        template<q::TypeId tid>
        static q::K_ptr getFromAddress(::K addr)
        {
            using traits = q::TypeTraits<tid>;
            auto const ptr = static_cast<typename traits::const_pointer>(validate(addr));
            return traits::atom(*ptr);
        }

        template<>
        static q::K_ptr getFromAddress<q::kSymbol>(::K addr);

        template<q::TypeId tid>
        static q::K_ptr listFromAddress(::K addr);

        template<>
        static q::K_ptr listFromAddress<q::kChar>(::K addr);

        template<q::TypeId tid>
        static void setToAddress(::K addr, ::K val)
        {
            using traits = q::TypeTraits<tid>;
            auto const ptr = static_cast<typename traits::pointer>(validate(addr));
            *ptr = traits::value(val);
        }

        template<>
        static void setToAddress<q::kSymbol>(::K addr, ::K val);

        void freeAddress(::K addr) const
        {
            auto const param = this->map(addr);
            auto const ptr = *misc::ptr_alias<char**>(param->get());
            std::free(ptr);
        }

    private:
        static void* validate(::K addr);
    };

    /// @brief FFI callback argument: list value
    template<q::TypeId tid>
    class List : public Argument
    {
    protected:
        using qTraits = q::TypeTraits<tid>;

        class Param : public Parameter
        {
        private:
            q::K_ptr param_;
            typename qTraits::pointer ptr_;

        public:
            Param(::K k) : param_{ q::dup_K(k) }, ptr_{ nullptr }
            {
                validate(param_.get());
                ptr_ = qTraits::index(param_);
            }

            void* get() override
            {
                return &ptr_;
            }

            void set(::K k) override
            {
                validate(k);
                if (param_)
                    param_ = qTraits::list(qTraits::index(k), count(k));
                else
                    throw q::K_error("state: invalid list parameter");
            }

            q::K_ptr release() override
            {
                return std::move(param_);
            }

        private:
            static void validate(::K k)
            {
                if (nullptr == k)
                    throw q::K_error("nil list parameter");

                if (qTraits::type_id != q::type(k)) {
                    std::ostringstream buffer;
                    buffer << "type: list parameter"
                        " (" << qTraits::type_id << "h expected)";
                    throw q::K_error(buffer.str());
                }
            }
        };

    public:
        List() : Argument(ffi_type_pointer)
        {}

        std::unique_ptr<Parameter> map(::K k, bool asReturn = false) const override
        {
            if (asReturn)
                throw q::K_error("nyi: use pointer return instead");
            else
                return std::make_unique<Param>(k);
        }

        std::unique_ptr<Parameter> create(bool = true) const override
        {
            throw q::K_error("nyi: use pointer parameter/return instead");
        }

        q::K_ptr getAddress(::K k) const
        {
            auto const param = this->map(k);
            using pointer_traits = TypeCode<sizeof(typename qTraits::const_pointer)>::traits;
            return pointer_traits::atom(
                *misc::ptr_alias<pointer_traits::const_pointer>(param->get()));
        }
    };

#pragma endregion
}//namespace q_ffi
