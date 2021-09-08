//
// Created by Alan Freitas on 6/1/21.
//

#ifndef SMALL_CONTAINER_TRAITS_HAS_ALLOCATOR_H
#define SMALL_CONTAINER_TRAITS_HAS_ALLOCATOR_H

namespace small {
    /// Check if type has an associated allocator type
    template <typename T, typename = void> struct has_allocator : std::false_type {};

    template <typename T>
    struct has_allocator<T, std::void_t<decltype(std::declval<T>().get_allocator()), typename T::allocator_type>>
        : std::true_type {};

    template <typename T> static constexpr bool has_allocator_v = has_allocator<T>::value;

} // namespace small

#endif // SMALL_CONTAINER_TRAITS_HAS_ALLOCATOR_H
