//
// Created by Alan Freitas on 8/13/21.
//

#ifndef SMALL_EXTRACT_VALUE_TYPE_H
#define SMALL_EXTRACT_VALUE_TYPE_H

namespace small {
    /// \struct Get value type from T, if present
    template <class T, class = void> struct extract_value_type { using type = void; };

    template <class T> struct extract_value_type<T, std::void_t<typename T::value_type>> {
        using type = typename T::value_type;
    };

    template <class T> using extract_value_type_t = typename extract_value_type<T>::type;

} // namespace small

#endif // SMALL_EXTRACT_VALUE_TYPE_H
