#pragma once

#include "q_ffi.h"
#include "std_ext.hpp"

namespace q_ffi
{
    class q_ffi_API DLLoader
    {
        void* dll_;

    public:
        DLLoader(char const* filename);
        ~DLLoader();

        void* locateProc(char const* funcName) const;

        template<typename ProcType>
        ProcType locateProc(char const* funcName) const
        {
            return misc::ptr_alias<ProcType>(locateProc(funcName));
        }

    private:
        void verifyState() const;
    };

}//namespace q_ffi