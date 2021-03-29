#pragma once

#include <limits>
#include <vector>
#include <ffi.h>
#include "dlloader.hpp"
#include "ffargs.hpp"

namespace q_ffi
{
    /// @brief Foreign Function invocator
    class Invocator
    {
    public:
        using function_type = void(*)();
        using argument_type = std::unique_ptr<Argument>;

    private:
        DLLoader dll_;
        function_type func_;
        argument_type return_;
        std::vector<argument_type> args_;

        struct FFICache
        {
            ffi_cif cif;
            ffi_type* ret_type;
            std::unique_ptr<ffi_type*[]> arg_types;
        };
        FFICache ffi_;

    public:
        Invocator(char const* dll);

        std::size_t arity() const;

        void load(char const* func, char retType, char const* argTypes,
            char const* abiType = nullptr);

        ::K operator()(std::vector<::K> const& params);
        ::K operator()(::K params);

    private:
        /// @brief Regenerates FFI cache
        void prepareFuncSpec(ffi_abi abi);

        static argument_type mapReturnSpec(char typeCode);
        static std::vector<argument_type> mapArgumentSpecs(char const* typeCodes);

        static ffi_abi mapABI(char const* abiType, char const* funcName);
        static ffi_abi guessABI(char const* funcName);

        static Argument* createArgument(char typeCode);

        ::K invoke(void* params[]);
    };

}//namespace q_ffi