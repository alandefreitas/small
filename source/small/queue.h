//
// Created by Alan Freitas on 9/7/21.
//

#ifndef SMALL_QUEUE_H
#define SMALL_QUEUE_H

#include "vector.h"
#include <queue>

namespace small {
    template <class T, size_t N = default_inline_storage_v<T>, class Container = small::vector<T, N>>
    using queue = std::queue<T, Container>;

    template <class T, size_t N = default_inline_storage_v<T>, class Container = small::vector<T, N>,
              class Compare = std::less<>>
    using priority_queue = std::priority_queue<T, Container, Compare>;
} // namespace small

#endif // SMALL_QUEUE_H
