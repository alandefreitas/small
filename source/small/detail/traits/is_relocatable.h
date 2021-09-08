//
// Created by Alan Freitas on 6/1/21.
//

#ifndef SMALL_CONTAINER_TRAITS_IS_RELOCATABLE_H
#define SMALL_CONTAINER_TRAITS_IS_RELOCATABLE_H

namespace small {
    /// \brief True type if a type is relocatable
    /// We use this only for trivially copyable types, but one can
    /// use this as an extension point to make their vectors faster.
    /// Almost any data structure without internal pointers is
    /// relocatable.
    template <class T>
    struct is_relocatable : std::conjunction<std::is_trivially_copy_constructible<T>,
                                             std::is_trivially_copy_assignable<T>, std::is_trivially_destructible<T>> {
    };

    template <class T> constexpr bool is_relocatable_v = is_relocatable<T>::value;

} // namespace small

#endif // SMALL_CONTAINER_TRAITS_IS_RELOCATABLE_H
