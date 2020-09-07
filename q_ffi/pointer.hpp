#pragma once

#include <memory>
#include "k_compat.h"

namespace q {

    struct K_deleter
    {
        void operator()(K& k) const noexcept
        {
            if (nullptr != k) r0(k);
            k = nullptr;
        }
    };

    /// @brief Smart pointer for @c K
    /// @remark As @c K is reference counted internally, we need a custom deleter to manage it.
    using K_ptr = std::unique_ptr<std::remove_pointer_t<K>, K_deleter>;

}//namespace q

#include "types.hpp"

namespace q {

    /// @brief Duplicate a <code>q::K_ptr</code> (incrementing its internal reference count)
    inline K_ptr dup_K(K_ptr const& pk) noexcept
    {
        return K_ptr{ nullptr == pk.get() ? nullptr : r1(pk.get()) };
    }

}//namespace q