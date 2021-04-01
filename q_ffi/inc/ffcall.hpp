#pragma once

#include <limits>
#include <vector>
#include <ffi.h>
#include "dlloader.hpp"
#include "ffargs.hpp"

namespace q_ffi
{
    /// @brief Maximum argument count of a q function.
    constexpr std::size_t MAX_ARGC = 8;

    /// @brief Implementation of <code>.ffi.load</code>
    q_ffi_API q::K_ptr load(::K dllSym, ::K fName, ::K abi, ::K ret, ::K args)
        noexcept(false);

    /// @brief Foreign Function invocator
    class Invocator
    {
    public:
        using function_type = void(*)();
        using argument_type = std::unique_ptr<Argument>;

    private:
        DLLoader dll_;
        function_type func_;
        argument_type ret_;
        std::vector<argument_type> args_;

        struct FFIinfo
        {
            ffi_cif cif;
            ffi_type* ret_type;
            std::unique_ptr<ffi_type*[]> arg_types;
        };
        FFIinfo ffi_;

    public:
        Invocator(char const* dll);

        unsigned int rank() const;

        void load(char const* func,
            char retType, char const* argTypes, char const* abiType = nullptr);

        q::K_ptr operator()(std::initializer_list<::K> params);

    private:
        /// @brief Regenerates FFI info cache
        void prepareFFI(ffi_abi abi);

        q::K_ptr invoke(void* params[]);

        void setReturnType(char typeCode);
        void setArgumentTypes(char const* typeCodes);
        void verifyArgumentTypes(char const* funcName, ffi_abi abi);

        static ffi_abi mapABI(char const* abiType, char const* funcName);
        static ffi_abi guessABI(char const* funcName);

        static argument_type mapType(char typeCode);
    };

}//namespace q_ffi