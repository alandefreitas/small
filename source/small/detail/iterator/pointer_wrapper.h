//
// Created by Alan Freitas on 4/17/21.
//

#ifndef SMALL_DETAIL_ITERATOR_POINTER_WRAPPER_H
#define SMALL_DETAIL_ITERATOR_POINTER_WRAPPER_H

#include <algorithm>
#include <iterator>
#include <type_traits>

namespace small {

    /// \brief Wraps a pointer as an iterator
    template <class POINTER> class pointer_wrapper {
      public:
        static_assert(std::is_pointer_v<POINTER>, "pointer_wrapper can only wrap pointers");
        typedef POINTER iterator_type;
        typedef typename std::iterator_traits<iterator_type>::iterator_category iterator_category;
        typedef typename std::iterator_traits<iterator_type>::value_type value_type;
        typedef typename std::iterator_traits<iterator_type>::difference_type difference_type;
        typedef typename std::iterator_traits<iterator_type>::pointer pointer;
        typedef typename std::iterator_traits<iterator_type>::reference reference;

      public /* constructors */:
        /// \brief Construct empty pointer wrapper
        constexpr pointer_wrapper() noexcept : base_(nullptr) {}

        /// \brief Construct pointer wrapper from pointer x
        constexpr explicit pointer_wrapper(iterator_type x) noexcept : base_(x) {}

        /// \brief Construct pointer wrapper from pointer wrapper u, which might be another type
        template <class UP>
        constexpr pointer_wrapper( // NOLINT(google-explicit-constructor): expected for iterators
            const pointer_wrapper<UP> &u,
            typename std::enable_if_t<std::is_convertible_v<UP, iterator_type>> * = 0) noexcept
            : base_(u.base()) {}

      public /* element access */:
        /// \brief Dereference iterator
        constexpr reference operator*() const noexcept { return *base_; }

        /// \brief Dereference iterator and get member
        constexpr pointer operator->() const noexcept { return static_cast<pointer>(std::addressof(*base_)); }

        /// \brief Dereference iterator n positions ahead
        constexpr reference operator[](difference_type n) const noexcept { return base_[n]; }

        /// \brief Get base pointer
        constexpr iterator_type base() const noexcept { return base_; }

      public /* modifiers */:
        /// \brief Advance iterator
        constexpr pointer_wrapper &operator++() noexcept {
            ++base_;
            return *this;
        }

        /// \brief Advance iterator (postfix)
        constexpr pointer_wrapper operator++(int) noexcept { // NOLINT(cert-dcl21-cpp)
            pointer_wrapper tmp(*this);
            ++(*this);
            return tmp;
        }

        /// \brief Rewind iterator
        constexpr pointer_wrapper &operator--() noexcept {
            --base_;
            return *this;
        }

        /// \brief Rewind iterator (postfix)
        constexpr pointer_wrapper operator--(int) noexcept { // NOLINT(cert-dcl21-cpp)
            pointer_wrapper tmp(*this);
            --(*this);
            return tmp;
        }

        /// \brief Return copy of iterator advanced by n positions
        constexpr pointer_wrapper operator+(difference_type n) const noexcept {
            pointer_wrapper w(*this);
            w += n;
            return w;
        }

        /// \brief Advance iterator by n positions
        constexpr pointer_wrapper &operator+=(difference_type n) noexcept {
            base_ += n;
            return *this;
        }

        /// \brief Return copy of iterator n positions behind
        constexpr pointer_wrapper operator-(difference_type n) const noexcept { return *this + (-n); }

        /// \brief Rewind iterator by n positions
        constexpr pointer_wrapper &operator-=(difference_type n) noexcept {
            *this += -n;
            return *this;
        }

      public /* relational operators */:
        /// Make any other pointer wrapper a friend
        template <class UP> friend class pointer_wrapper;

      public /* friends */:
        /// \brief Get distance between iterators
        template <class Iter1, class Iter2>
        constexpr friend auto operator-(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept
            -> decltype(x.base() - y.base());

        /// \brief Sum iterators
        template <class Iter1>
        constexpr friend pointer_wrapper<Iter1> operator+(typename pointer_wrapper<Iter1>::difference_type,
                                                          pointer_wrapper<Iter1>) noexcept;

      private:
        /// Base pointer
        iterator_type base_;
    };

    template <class Iter1, class Iter2>
    inline constexpr bool operator==(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept {
        return x.base() == y.base();
    }

    template <class Iter1, class Iter2>
    inline constexpr bool operator!=(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept {
        return !(x == y);
    }

    template <class Iter1, class Iter2>
    inline constexpr bool operator<(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept {
        return x.base() < y.base();
    }

    template <class Iter1, class Iter2>
    inline constexpr bool operator>(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept {
        return y < x;
    }

    template <class Iter1, class Iter2>
    inline constexpr bool operator>=(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept {
        return !(x < y);
    }

    template <class Iter1, class Iter2>
    inline constexpr bool operator<=(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept {
        return !(y < x);
    }

    template <class Iter>
    inline constexpr bool operator==(const pointer_wrapper<Iter> &x, const pointer_wrapper<Iter> &y) noexcept {
        return x.base() == y.base();
    }

    template <class Iter>
    inline constexpr bool operator!=(const pointer_wrapper<Iter> &x, const pointer_wrapper<Iter> &y) noexcept {
        return !(x == y);
    }

    template <class Iter>
    inline constexpr bool operator>(const pointer_wrapper<Iter> &x, const pointer_wrapper<Iter> &y) noexcept {
        return y < x;
    }

    template <class Iter>
    inline constexpr bool operator>=(const pointer_wrapper<Iter> &x, const pointer_wrapper<Iter> &y) noexcept {
        return !(x < y);
    }

    template <class Iter>
    inline constexpr bool operator<=(const pointer_wrapper<Iter> &x, const pointer_wrapper<Iter> &y) noexcept {
        return !(y < x);
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator==(const pointer_wrapper<Iter> &x, const BaseIter &y) noexcept {
        return x.base() == y;
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator!=(const pointer_wrapper<Iter> &x, const BaseIter &y) noexcept {
        return !(x == y);
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator>(const pointer_wrapper<Iter> &x, const BaseIter &y) noexcept {
        return y < x;
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator>=(const pointer_wrapper<Iter> &x, const BaseIter &y) noexcept {
        return !(x < y);
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator<=(const pointer_wrapper<Iter> &x, const BaseIter &y) noexcept {
        return !(y < x);
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator==(const BaseIter &x, const pointer_wrapper<Iter> &y) noexcept {
        return x.base() == y.base();
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator!=(const BaseIter &x, const pointer_wrapper<Iter> &y) noexcept {
        return !(x == y);
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator>(const BaseIter &x, const pointer_wrapper<Iter> &y) noexcept {
        return y < x;
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator>=(const BaseIter &x, const pointer_wrapper<Iter> &y) noexcept {
        return !(x < y);
    }

    template <class Iter, class BaseIter>
    inline constexpr bool operator<=(const BaseIter &x, const pointer_wrapper<Iter> &y) noexcept {
        return !(y < x);
    }

    template <class Iter1, class Iter2>
    inline constexpr auto operator-(const pointer_wrapper<Iter1> &x, const pointer_wrapper<Iter2> &y) noexcept
        -> decltype(x.base() - y.base()) {
        return x.base() - y.base();
    }

    template <class Iter>
    inline constexpr pointer_wrapper<Iter> operator+(typename pointer_wrapper<Iter>::difference_type n,
                                                     pointer_wrapper<Iter> x) noexcept {
        x += n;
        return x;
    }

} // namespace small

#endif // SMALL_DETAIL_ITERATOR_POINTER_WRAPPER_H
