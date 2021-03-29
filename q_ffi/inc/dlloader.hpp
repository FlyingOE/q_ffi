#pragma once

#include "q_ffi.h"

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
            return reinterpret_cast<ProcType>(locateProc(funcName));
        }

    private:
        void verifyState() const;
    };

}//namespace q_ffi