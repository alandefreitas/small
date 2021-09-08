//
// Created by Alan Freitas on 8/3/21.
//

#ifndef SMALL_STRLEN_H
#define SMALL_STRLEN_H

#include <cstring>

namespace small {
    /// \brief strlen for different character types
    template <typename T> inline std::size_t strlen(const T *str) {
        std::size_t len = 0u;
        while (*str++) {
            ++len;
        }
        return len;
    }

    /// \brief Usual strlen function
    template <> inline std::size_t strlen<char>(const char *str) { return std::strlen(str); }

    /// \brief strlen for different character types with a size limit
    template <typename T> inline std::size_t strlen(const T *str, std::size_t limit) {
        std::size_t len = 0u;
        while (*str++ && len < limit) {
            ++len;
        }
        return len;
    }
} // namespace small

#endif // SMALL_STRLEN_H
