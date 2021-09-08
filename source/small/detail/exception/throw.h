//
// Created by Alan Freitas on 8/2/21.
//

#ifndef SMALL_THROW_H
#define SMALL_THROW_H

#include <exception>
#include <type_traits>
#include <utility>

namespace small {

    /// \brief Throw an exception but terminate if we can't throw
    template <typename Ex> [[noreturn]] void throw_exception(Ex &&ex) {
#ifndef SMALL_DISABLE_EXCEPTIONS
        throw static_cast<Ex &&>(ex);
#else
        (void)ex;
        std::terminate();
#endif
    }

    /// \brief Construct and throw an exception but terminate otherwise
    template <typename Ex, typename... Args> [[noreturn]] void throw_exception(Args &&...args) {
        if constexpr (std::is_constructible_v<Ex, Args...>) {
            throw_exception(Ex(args...));
        } else {
            throw_exception(Ex());
        }
    }

    /// \brief Throw an exception but terminate if we can't throw
    template <typename ThrowFn, typename CatchFn> void catch_exception(ThrowFn &&thrower, CatchFn &&catcher) {
#ifndef SMALL_DISABLE_EXCEPTIONS
        try {
            return static_cast<ThrowFn &&>(thrower)();
        } catch (std::exception &e) {
            return static_cast<CatchFn &&>(catcher)();
        }
#else
        return static_cast<ThrowFn &&>(thrower)();
#endif
    }

} // namespace small

#endif // SMALL_THROW_H
