//
// Copyright (c) 2021 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//


#ifndef SMALL_DETAIL_TRAITS_HAS_ALLOCATOR_H
#define SMALL_DETAIL_TRAITS_HAS_ALLOCATOR_H

namespace small {
    /// Check if type has an associated allocator type
    template <typename T, typename = void> struct has_allocator : std::false_type {};

    template <typename T>
    struct has_allocator<T, std::void_t<decltype(std::declval<T>().get_allocator()), typename T::allocator_type>>
        : std::true_type {};

    template <typename T> static constexpr bool has_allocator_v = has_allocator<T>::value;

} // namespace small

#endif // SMALL_DETAIL_TRAITS_HAS_ALLOCATOR_H
