//
// Created by Alan Freitas on 5/31/21.
//

#ifndef SMALL_SMALL_SET_H
#define SMALL_SMALL_SET_H

#include "detail/container/associative_vector.h"
#include "detail/container/max_size_map.h"
#include "vector.h"

/// \file A set container for small sets

namespace small {
    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>,
              class Allocator = std::allocator<T>>
    using set = associative_vector<false, false, true, vector<T, N, Allocator>, Compare>;

    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>>
    using max_size_set = small::associative_vector<false, false, true, max_size_vector<T, N>, Compare>;

    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>,
              class Allocator = std::allocator<T>>
    using multiset = associative_vector<false, true, true, vector<T, N, Allocator>, Compare>;

    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>>
    using max_size_multiset = small::associative_vector<false, true, true, max_size_vector<T, N>, Compare>;

    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>,
              class Allocator = std::allocator<T>>
    using unordered_set = associative_vector<false, false, false, vector<T, N, Allocator>, Compare>;

    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>>
    using max_size_unordered_set = small::associative_vector<false, false, false, max_size_vector<T, N>, Compare>;

    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>,
              class Allocator = std::allocator<T>>
    using unordered_multiset = associative_vector<false, true, false, vector<T, N, Allocator>, Compare>;

    template <class T, size_t N = default_inline_storage_v<T>, class Compare = std::less<>>
    using max_size_unordered_multiset = small::associative_vector<false, true, false, max_size_vector<T, N>, Compare>;

} // namespace small

#endif // SMALL_SMALL_SET_H
