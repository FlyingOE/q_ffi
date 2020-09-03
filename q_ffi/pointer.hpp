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

    /// @brief Similar to <code>std::make_unique</code>, but specifically for @c K
    template<Type tid, typename T,
        typename = std::enable_if_t<
            std::is_same_v<typename TypeTraits<tid>::value_type, std::decay_t<T>>
        >>
    constexpr K_ptr make_K(T&& v) noexcept
    {
        using Traits = TypeTraits<tid>;
        return K_ptr{ Traits::atom(std::forward<T>(v)) };
    }

    /// @brief Duplicate a <code>q::K_ptr</code> (incrementing its internal reference count)
    inline K_ptr dup_K(K_ptr const& pk) noexcept
    {
        return K_ptr{ nullptr == pk.get() ? nullptr : r1(pk.get()) };
    }

}//namespace q