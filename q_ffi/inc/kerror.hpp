#pragma once

#include <stdexcept>
#include "ktypes.hpp"

namespace q
{
    namespace details
    {
        q_ffi_API std::string q2error(::K const what);

    }//namespace q::details

    class K_error : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;

        K_error(std::runtime_error const& ex) noexcept
            : runtime_error(ex)
        {}

        explicit K_error(::K const what)
            : K_error{ details::q2error(what) }
        {}

        /// @return Always @c Nil (result of what() will be signaled to kdb+ host)
        ::K report() const noexcept;
    };

}//namespace q
