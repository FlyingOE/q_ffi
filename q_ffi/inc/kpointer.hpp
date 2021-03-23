#pragma once

#include <memory>
#include <k_compat.h>

namespace q {

    /// @remark Must be default constructible to avoid the hassle during @c K_ptr construction.
    struct K_deleter
    {
        void operator()(::K k) const noexcept
        {
            if (nullptr != k)
                ::r0(k);
        }
    };

    /// @brief Smart pointer for @c K
    /// @remark As @c K is internally reference counted, we need a custom deleter to manage it.
    using K_ptr = std::unique_ptr<std::remove_pointer_t<::K>, K_deleter>;

    /// @brief Duplicate a @c K object (incrementing its internal reference count)
    inline K_ptr dup_K(::K k) noexcept
    {
        return K_ptr{ nullptr == k ? nullptr : ::r1(k) };
    }

    /// @brief Duplicate a @c K_ptr (incrementing its internal reference count)
    inline K_ptr dup_K(K_ptr const& pk) noexcept
    {
        return dup_K(pk.get());
    }

}//namespace q
