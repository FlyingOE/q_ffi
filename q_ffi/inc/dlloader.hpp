#pragma once

namespace q_ffi
{
    class DLLoader
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