#pragma once

#include "q_ffi.h"
#include <limits>
#include <vector>
#include <ffi.h>
#include "kerror.hpp"
#include "ktype_traits.hpp"
#include "type_convert.hpp"
#include "ffargs.hpp"

namespace q_ffi
{
    /// @brief Foreign Function invocator
    class Invocator
    {
    public:
        using function_type = void (*)();
        using argument_type = std::unique_ptr<Argument>;

    private:
        function_type func_;

        ffi_cif cif_;
        std::unique_ptr<ffi_type*[]> args_;

        argument_type return_;
        std::vector<argument_type> params_;

    public:
        Invocator(function_type func, char resType, char const* argTypes,
            char const* abiType = nullptr);

        Invocator(function_type func, char resType, std::string const& argTypes,
            char const* abi = nullptr)
            : Invocator(func, resType, argTypes.c_str(), abi)
        {}

        ::K operator()(std::vector<::K> const& params) noexcept(false);

    private:
        void prepareFuncSpec(ffi_abi abi);

        static argument_type mapReturnSpec(char type);
        static std::vector<argument_type> mapArgumentSpecs(char const* types);

        static Argument* createArgument(char typeCode);
    };

}//namespace q_ffi