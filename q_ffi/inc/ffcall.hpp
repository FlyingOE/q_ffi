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

    /// @brief Implementation of <code>.ffi.loadFun</code>
    q_ffi_API q::K_ptr loadFun(::K dllSym, ::K funName, ::K abi, ::K ret, ::K args)
        noexcept(false);

    /// @brief Implementation of <code>.ffi.getVar</code>
    q_ffi_API q::K_ptr getVar(::K dllSym, ::K varName, ::K typeCode)
        noexcept(false);

    /// @brief Implementation of <code>.ffi.setVar</code>
    q_ffi_API q::K_ptr setVar(::K dllSym, ::K varName, ::K typeCode, ::K val)
        noexcept(false);

    /// @brief Foreign Function invocator
    class Invocator
    {
    public:
        using function_type = void(*)();
        using argument_type = std::unique_ptr<Argument>;

    private:
        DLLoader dll_;
        union {
            function_type func;
            void* var;
        } sym_;
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

#   pragma region FFI function
        void load(char const* func, char retType, char const* argTypes,
            char const* abiType = nullptr);

        unsigned int rank() const;

        q::K_ptr operator()(std::initializer_list<::K> params);
#   pragma endregion

#   pragma region FFI variable
//        void load(char const* var, char varType);
//
//        q::K_ptr operator()();
//        void operator()(::K val);
#   pragma endregion

    private:
        /// @brief Regenerates FFI info cache
        void prepareFFI(ffi_abi abi);

        void setReturnType(char typeCode);
        void setArgumentTypes(char const* typeCodes);

        static ffi_abi mapABI(char const* abiType, char const* funcName);
        static ffi_abi guessABI(char const* funcName);

        static argument_type mapType(char typeCode);
    };

}//namespace q_ffi
