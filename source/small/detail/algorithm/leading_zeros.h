//
// Created by Alan Freitas on 8/3/21.
//

#ifndef SMALL_LEADING_ZEROS_H
#define SMALL_LEADING_ZEROS_H

//! Implementation Detail
namespace small {

    /// \brief Count leading zeros utility
#if defined(__GNUC__)
#ifndef SMALL_HAS_LEADING_ZEROS_FUNCTION
#define SMALL_HAS_LEADING_ZEROS_FUNCTION true
#endif
    inline unsigned int leading_zeros(char value) noexcept { return (char)__builtin_clz(value); }
    inline unsigned int leading_zeros(unsigned int value) noexcept { return (unsigned int)__builtin_clz(value); }
    inline unsigned int leading_zeros(unsigned long int value) noexcept { return (unsigned int)__builtin_clzl(value); }
    inline unsigned int leading_zeros(char32_t value) noexcept {
        return sizeof(char32_t) == sizeof(unsigned long int) ? (unsigned int)__builtin_clzl(value)
                                                             : (unsigned int)__builtin_clz(value);
    }
#elif defined(_MSC_VER)
#ifndef SMALL_HAS_LEADING_ZEROS_FUNCTION
#define SMALL_HAS_LEADING_ZEROS_FUNCTION true
#endif
    template <typename T> inline unsigned int lzcnt(T value) noexcept {
        unsigned long value_log2;
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__)
        _BitScanReverse64(&value_log2, value);
#else
        _BitScanReverse(&value_log2, static_cast<unsigned long>(value));
#endif
        return sizeof(T) * 8 - value_log2 - 1;
    }
    inline unsigned int leading_zeros(std::uint16_t value) noexcept { return lzcnt(value); }
    inline unsigned int leading_zeros(std::uint32_t value) noexcept { return lzcnt(value); }
#ifndef WIN32
    inline unsigned int leading_zeros(std::uint64_t value) noexcept { return lzcnt(value); }
#endif // WIN32
    inline unsigned int leading_zeros(char32_t value) noexcept { return lzcnt(value); }
#endif

    /// \brief Wrapping this functionality in a trait

#if defined(SMALL_HAS_LEADING_ZEROS_FUNCTION) && SMALL_HAS_LEADING_ZEROS_FUNCTION == true
    struct system_has_leading_zeros : std::true_type {};
#else
    struct system_has_leading_zeros : std::false_type {};
#endif

    constexpr bool system_has_leading_zeros_v = system_has_leading_zeros::value;

} // namespace small

#endif // SMALL_LEADING_ZEROS_H
