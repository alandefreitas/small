//
// Created by Alan Freitas on 5/31/21.
//

#ifndef SMALL_MAP_H
#define SMALL_MAP_H

#include "detail/container/associative_vector.h"
#include "vector.h"

/// \file A map container for small maps

namespace small {
    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>,
              class Allocator = std::allocator<std::pair<K, T>>>
    using map = associative_vector<true, false, true, vector<std::pair<K, T>, N, Allocator>, Compare>;

    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>>
    using max_size_map = small::associative_vector<true, false, true, max_size_vector<std::pair<K, T>, N>, Compare>;

    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>,
              class Allocator = std::allocator<std::pair<K, T>>>
    using multimap = associative_vector<true, true, true, vector<std::pair<K, T>, N, Allocator>, Compare>;

    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>>
    using max_size_multimap = small::associative_vector<true, true, true, max_size_vector<std::pair<K, T>, N>, Compare>;

    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>,
              class Allocator = std::allocator<std::pair<K, T>>>
    using unordered_map = associative_vector<true, false, false, vector<std::pair<K, T>, N, Allocator>, Compare>;

    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>>
    using max_size_unordered_map =
        small::associative_vector<true, false, false, max_size_vector<std::pair<K, T>, N>, Compare>;

    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>,
              class Allocator = std::allocator<std::pair<K, T>>>
    using unordered_multimap = associative_vector<true, true, false, vector<std::pair<K, T>, N, Allocator>, Compare>;

    template <class K, class T, size_t N = default_inline_storage_v<std::pair<K, T>>, class Compare = std::less<>>
    using max_size_unordered_multimap =
        small::associative_vector<true, true, false, max_size_vector<std::pair<K, T>, N>, Compare>;

} // namespace small

#endif // SMALL_MAP_H
