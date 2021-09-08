//
// Created by Alan Freitas on 9/7/21.
//

#ifndef SMALL_IS_PAIR_H
#define SMALL_IS_PAIR_H

#include <type_traits>

namespace small {
    /// Check if type is a pair
    template <typename> struct is_pair : std::false_type {};

    template <typename T, typename U> struct is_pair<std::pair<T, U>> : std::true_type {};

    template <class T> constexpr bool is_pair_v = is_pair<T>::value;
} // namespace small

#endif // SMALL_IS_PAIR_H
