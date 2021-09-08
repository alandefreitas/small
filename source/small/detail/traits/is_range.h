//
// Created by Alan Freitas on 6/1/21.
//

#ifndef SMALL_CONTAINER_TRAITS_IS_RANGE_H
#define SMALL_CONTAINER_TRAITS_IS_RANGE_H

namespace small {
    /// Check if type is a range (has begin() and end() functions)
    template <typename T, typename = void> struct is_range : std::false_type {};

    template <typename T>
    struct is_range<
        T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end()), typename T::value_type>>
        : std::true_type {};

    /// True if type is a range (has begin() and end() functions)
    template <typename T> constexpr bool is_range_v = is_range<std::decay_t<T>>::value;

} // namespace small

#endif // SMALL_CONTAINER_TRAITS_IS_RANGE_H
