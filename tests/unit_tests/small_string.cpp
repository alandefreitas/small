#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <numeric>

#include <catch2/catch.hpp>

#include <small/map.h>
#include <small/string.h>
#include <small/vector.h>

// UTF8 string literals are really not safe in MSVC.
// u8"" doesn't work properly even with escape sequences.
// More recent versions might improve on that a little, but we will still
// need a more robust solution to support older versions in any case.
// Until then, it seems like the most robust solution is to initialize
// small::strings with U"...", even if that requires converting the
// codepoints.
constexpr bool is_windows() {
#if defined(_WIN32)
    return true;
#else
    return false;
#endif
}


TEST_CASE("String") {
    using namespace small;

    auto equal_il = [](const auto &sm_string, std::initializer_list<char> il) -> bool {
        return std::equal(sm_string.begin(), sm_string.end(), il.begin(), il.end());
    };

    SECTION("Asserts") {
        STATIC_REQUIRE(std::is_copy_constructible_v<string>);
        STATIC_REQUIRE(std::is_copy_assignable_v<string>);
        STATIC_REQUIRE(std::is_move_constructible_v<string>);
        STATIC_REQUIRE(std::is_move_assignable_v<string>);
        STATIC_REQUIRE(is_utf8_v<char>);
        STATIC_REQUIRE(is_utf16_v<char16_t>);
        STATIC_REQUIRE(is_utf32_v<char32_t>);
    }

    SECTION("Unicode") {
        SECTION("UTF8") {
            utf8_char_type a = 'g';
            // utf8_char_type b = 'á'; // <- can't fit in 8 bits
            // utf8_char_type c = '😀';
            std::basic_string<utf8_char_type> d = "g";
            std::basic_string<utf8_char_type> e = "á";
            std::basic_string<utf8_char_type> f = "😀";
            SECTION("Check") {
                // Check container sizes
                REQUIRE(d.size() == 1);
                REQUIRE(e.size() == 2);
                REQUIRE(f.size() == 4);

                // Identify continuation bytes
                REQUIRE_FALSE(is_utf8_continuation(d[0]));
                REQUIRE_FALSE(is_utf8_continuation(e[0]));
                REQUIRE(is_utf8_continuation(e[1]));
                REQUIRE_FALSE(is_utf8_continuation(f[0]));
                REQUIRE(is_utf8_continuation(f[1]));
                REQUIRE(is_utf8_continuation(f[2]));
                REQUIRE(is_utf8_continuation(f[3]));

                // Identify utf size from first char
                REQUIRE(utf8_size(a) == 1);
                REQUIRE(utf8_size(d[0]) == 1);
                REQUIRE(utf8_size(e[0]) == 2);
                REQUIRE(utf8_size(e[1]) == 1);
                REQUIRE(utf8_size(f[0]) == 4);
                REQUIRE(utf8_size(f[1]) == 1);
                REQUIRE(utf8_size(f[2]) == 1);
                REQUIRE(utf8_size(f[3]) == 1);

                // Identify continuation bytes (inferring input type)
                REQUIRE_FALSE(is_utf_continuation(d[0]));
                REQUIRE_FALSE(is_utf_continuation(e[0]));
                REQUIRE(is_utf_continuation(e[1]));
                REQUIRE_FALSE(is_utf_continuation(f[0]));
                REQUIRE(is_utf_continuation(f[1]));
                REQUIRE(is_utf_continuation(f[2]));
                REQUIRE(is_utf_continuation(f[3]));

                // Identify utf size from first char (inferring input type)
                REQUIRE(utf_size(a, 1) == 1);
                REQUIRE(utf_size(d[0], 1) == 1);
                REQUIRE(utf_size(e[0], 2) == 2);
                REQUIRE(utf_size(e[1], 1) == 1);
                REQUIRE(utf_size(f[0], 4) == 4);
                REQUIRE(utf_size(f[1], 4) == 1);
                REQUIRE(utf_size(f[2], 4) == 1);
                REQUIRE(utf_size(f[3], 4) == 1);
            }
            SECTION("To UTF16") {
                utf16_char_type buf[2];

                REQUIRE(from_utf8_to_utf16(&a, 1, buf, 2) == 1);
                utf32_char_type r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf8_to_utf16(d.begin(), d.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf8_to_utf16(e.begin(), e.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(from_utf8_to_utf16(f.begin(), f.size(), buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input
                REQUIRE(to_utf16(&a, 1, buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf16(d.begin(), d.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf16(e.begin(), e.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf16(f.begin(), f.size(), buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input and output
                REQUIRE(to_utf(&a, 1, buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(d.begin(), d.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(e.begin(), e.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf(f.begin(), f.size(), buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');
            }

            SECTION("To UTF32") {
                utf32_char_type r;

                r = from_utf8_to_utf32(&a, 1);
                REQUIRE(r == U'g');

                r = from_utf8_to_utf32(d.begin(), d.size());
                REQUIRE(r == U'g');

                r = from_utf8_to_utf32(e.begin(), e.size());
                REQUIRE(r == U'á');

                r = from_utf8_to_utf32(f.begin(), f.size());
                REQUIRE(r == U'😀');

                // Inferring type from input
                REQUIRE(to_utf32(&a, 1, &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf32(d.begin(), d.size(), &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf32(e.begin(), e.size(), &r, 1) == 1);
                REQUIRE(r == U'á');

                REQUIRE(to_utf32(f.begin(), f.size(), &r, 1) == 1);
                REQUIRE(r == U'😀');

                // Inferring type from input and output
                REQUIRE(to_utf(&a, 1, &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf(d.begin(), d.size(), &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf(e.begin(), e.size(), &r, 1) == 1);
                REQUIRE(r == U'á');

                REQUIRE(to_utf(f.begin(), f.size(), &r, 1) == 1);
                REQUIRE(r == U'😀');
            }
        }
        SECTION("UTF16") {
            utf16_char_type a = u'g';
            utf16_char_type b = u'á';
            // utf16_char_type c = u'😀'; // <- can't fit in a char
            std::basic_string<utf16_char_type> d = u"g";
            std::basic_string<utf16_char_type> e = u"á";
            std::basic_string<utf16_char_type> f = u"😀";

            SECTION("Check") {
                // Check container sizes
                REQUIRE(d.size() == 1);
                REQUIRE(e.size() == 1);
                REQUIRE(f.size() == 2);

                // Identify surrogate code units
                REQUIRE_FALSE(is_utf16_surrogate(a));
                REQUIRE_FALSE(is_utf16_surrogate(b));
                REQUIRE_FALSE(is_utf16_surrogate(d[0]));
                REQUIRE_FALSE(is_utf16_surrogate(e[0]));
                REQUIRE(is_utf16_surrogate(f[0]));
                REQUIRE(is_utf16_surrogate(f[1]));

                // Identify high and low surrogate code units
                REQUIRE_FALSE(is_utf16_high_surrogate(a));
                REQUIRE_FALSE(is_utf16_low_surrogate(a));
                REQUIRE_FALSE(is_utf16_high_surrogate(b));
                REQUIRE_FALSE(is_utf16_low_surrogate(b));
                REQUIRE_FALSE(is_utf16_high_surrogate(d[0]));
                REQUIRE_FALSE(is_utf16_low_surrogate(d[0]));
                REQUIRE_FALSE(is_utf16_high_surrogate(e[0]));
                REQUIRE_FALSE(is_utf16_low_surrogate(e[0]));
                REQUIRE(is_utf16_high_surrogate(f[0]));
                REQUIRE_FALSE(is_utf16_low_surrogate(f[0]));
                REQUIRE_FALSE(is_utf16_high_surrogate(f[1]));
                REQUIRE(is_utf16_low_surrogate(f[1]));

                // Identify continuation code units (alias for low surrogates)
                REQUIRE_FALSE(is_utf16_continuation(a));
                REQUIRE_FALSE(is_utf16_continuation(b));
                REQUIRE_FALSE(is_utf16_continuation(d[0]));
                REQUIRE_FALSE(is_utf16_continuation(e[0]));
                REQUIRE_FALSE(is_utf16_continuation(f[0]));
                REQUIRE(is_utf16_continuation(f[1]));

                // Identify utf size from first char
                REQUIRE(utf16_size(a) == 1);
                REQUIRE(utf16_size(b) == 1);
                REQUIRE(utf16_size(d[0]) == 1);
                REQUIRE(utf16_size(e[0]) == 1);
                REQUIRE(utf16_size(f[0]) == 2);
                REQUIRE(utf16_size(f[1]) == 1);

                // Identify continuation code units identifying input type
                REQUIRE_FALSE(is_utf_continuation(a));
                REQUIRE_FALSE(is_utf_continuation(b));
                REQUIRE_FALSE(is_utf_continuation(d[0]));
                REQUIRE_FALSE(is_utf_continuation(e[0]));
                REQUIRE_FALSE(is_utf_continuation(f[0]));
                REQUIRE(is_utf_continuation(f[1]));

                // Identify utf size from first char identifying input type
                REQUIRE(utf_size(a, 1) == 1);
                REQUIRE(utf_size(b, 1) == 1);
                REQUIRE(utf_size(d[0], 1) == 1);
                REQUIRE(utf_size(e[0], 1) == 1);
                REQUIRE(utf_size(f[0], 2) == 2);
                REQUIRE(utf_size(f[1], 2) == 1);
            }
            SECTION("To UTF8") {
                utf8_char_type buf[8];

                REQUIRE(from_utf16_to_utf8(&a, 1, buf, 8) == 1);
                utf32_char_type r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf16_to_utf8(&b, 1, buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(from_utf16_to_utf8(d.begin(), d.size(), buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf16_to_utf8(e.begin(), e.size(), buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(from_utf16_to_utf8(f.begin(), f.size(), buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input
                REQUIRE(to_utf8(&a, 1, buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf8(&b, 1, buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf8(d.begin(), d.size(), buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf8(e.begin(), e.size(), buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf8(f.begin(), f.size(), buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input and output
                REQUIRE(to_utf(&a, 1, buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(d.begin(), d.size(), buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(e.begin(), e.size(), buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf(f.begin(), f.size(), buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');
            }
            SECTION("To UTF32") {
                REQUIRE(utf16_surrogates_to_utf32(f[0], f[1]) == U'😀');

                utf32_char_type r;

                r = from_utf16_to_utf32(&a, 1);
                REQUIRE(r == U'g');

                r = from_utf16_to_utf32(&b, 1);
                REQUIRE(r == U'á');

                r = from_utf16_to_utf32(d.begin(), d.size());
                REQUIRE(r == U'g');

                r = from_utf16_to_utf32(e.begin(), e.size());
                REQUIRE(r == U'á');

                r = from_utf16_to_utf32(f.begin(), f.size());
                REQUIRE(r == U'😀');

                // Inferring type from input
                REQUIRE(to_utf32(&a, 1, &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf32(&b, 1, &r, 1) == 1);
                REQUIRE(r == U'á');

                REQUIRE(to_utf32(d.begin(), d.size(), &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf32(e.begin(), e.size(), &r, 1) == 1);
                REQUIRE(r == U'á');

                REQUIRE(to_utf32(f.begin(), f.size(), &r, 1) == 1);
                REQUIRE(r == U'😀');

                // Inferring type from input and output
                REQUIRE(to_utf(&a, 1, &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf(d.begin(), d.size(), &r, 1) == 1);
                REQUIRE(r == U'g');

                REQUIRE(to_utf(e.begin(), e.size(), &r, 1) == 1);
                REQUIRE(r == U'á');

                REQUIRE(to_utf(f.begin(), f.size(), &r, 1) == 1);
                REQUIRE(r == U'😀');
            }
        }
        SECTION("UTF32") {
            utf32_char_type a = U'g';
            utf32_char_type b = U'á';
            utf32_char_type c = U'😀';
            std::basic_string<utf32_char_type> d = U"g";
            std::basic_string<utf32_char_type> e = U"á";
            std::basic_string<utf32_char_type> f = U"😀";
            SECTION("Check") {
                // Check container sizes
                REQUIRE(d.size() == 1);
                REQUIRE(e.size() == 1);
                REQUIRE(f.size() == 1);

                // Identify continuation code units (always false for utf32)
                REQUIRE_FALSE(is_utf32_continuation(a));
                REQUIRE_FALSE(is_utf32_continuation(b));
                REQUIRE_FALSE(is_utf32_continuation(d[0]));
                REQUIRE_FALSE(is_utf32_continuation(e[0]));
                REQUIRE_FALSE(is_utf32_continuation(f[0]));

                // Identify utf size from first char (always 1 for utf32)
                REQUIRE(utf32_size(a) == 1);
                REQUIRE(utf32_size(b) == 1);
                REQUIRE(utf32_size(c) == 1);
                REQUIRE(utf32_size(d[0]) == 1);
                REQUIRE(utf32_size(e[0]) == 1);
                REQUIRE(utf32_size(f[0]) == 1);

                // Identify continuation code units identifying input type
                REQUIRE_FALSE(is_utf_continuation(a));
                REQUIRE_FALSE(is_utf_continuation(b));
                REQUIRE_FALSE(is_utf_continuation(d[0]));
                REQUIRE_FALSE(is_utf_continuation(e[0]));
                REQUIRE_FALSE(is_utf_continuation(f[0]));

                // Identify utf size from first char identifying input type
                REQUIRE(utf_size(a, 1) == 1);
                REQUIRE(utf_size(b, 1) == 1);
                REQUIRE(utf_size(c, 1) == 1);
                REQUIRE(utf_size(d[0], 1) == 1);
                REQUIRE(utf_size(e[0], 1) == 1);
                REQUIRE(utf_size(f[0], 1) == 1);
            }
            SECTION("To UTF8") {
                utf8_char_type buf[8];

                REQUIRE(from_utf32_to_utf8(a, buf, 8) == 1);
                utf32_char_type r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf32_to_utf8(b, buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(from_utf32_to_utf8(c, buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');

                REQUIRE(from_utf32_to_utf8(d.front(), buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf32_to_utf8(e.front(), buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(from_utf32_to_utf8(f.front(), buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input
                REQUIRE(to_utf8(&a, 1, buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf8(&b, 1, buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf8(&c, 1, buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');

                REQUIRE(to_utf8(d.begin(), d.size(), buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf8(e.begin(), e.size(), buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf8(f.begin(), f.size(), buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input and output
                REQUIRE(to_utf(&a, 1, buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(&b, 1, buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf(&c, 1, buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');

                REQUIRE(to_utf(d.begin(), d.size(), buf, 8) == 1);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(e.begin(), e.size(), buf, 8) == 2);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf(f.begin(), f.size(), buf, 8) == 4);
                r = from_utf8_to_utf32(buf, utf8_size(buf[0]));
                REQUIRE(r == U'😀');
            }

            SECTION("To UTF16") {
                utf16_char_type buf[2];

                REQUIRE(from_utf32_to_utf16(a, buf, 2) == 1);
                utf32_char_type r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf32_to_utf16(b, buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(from_utf32_to_utf16(c, buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');

                REQUIRE(from_utf32_to_utf16(d.front(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(from_utf32_to_utf16(e.front(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(from_utf32_to_utf16(f.front(), buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input
                REQUIRE(to_utf16(&a, 1, buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf16(&b, 1, buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf16(&c, 1, buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');

                REQUIRE(to_utf16(d.begin(), d.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf16(e.begin(), e.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf16(f.begin(), f.size(), buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');

                // Inferring type from input and output
                REQUIRE(to_utf(&a, 1, buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(&b, 1, buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf(&c, 1, buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');

                REQUIRE(to_utf(d.begin(), d.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'g');

                REQUIRE(to_utf(e.begin(), e.size(), buf, 2) == 1);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'á');

                REQUIRE(to_utf(f.begin(), f.size(), buf, 2) == 2);
                r = from_utf16_to_utf32(buf, utf16_size(buf[0]));
                REQUIRE(r == U'😀');
            }
        }
    }

    SECTION("Constructor") {
        SECTION("Default") {
            string a;
            REQUIRE(a.empty());
            REQUIRE(a.size() == 0); // NOLINT(readability-container-size-empty)
            REQUIRE(a.size_codepoints() == 0);
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            string a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From char value") {
            SECTION("Zero values") {
                std::allocator<int> alloc;
                string c(0, 'x', alloc); // NOLINT(bugprone-string-constructor)
                REQUIRE(c.empty());
                REQUIRE(c.size() == 0); // NOLINT(readability-container-size-empty)
                REQUIRE(c.size_codepoints() == 0);
                REQUIRE(c == "");  // NOLINT(readability-container-size-empty)
                REQUIRE(c == U""); // NOLINT(readability-container-size-empty)
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("Constant values") {
                std::allocator<int> alloc;
                string c(3, 'x', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size() == 3);
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c == "xxx");
                REQUIRE(c == U"xxx");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From UTF16 value") {
                std::allocator<int> alloc;
                string c(3, u'x', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size() == 3);
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c == "xxx");
                REQUIRE(c == u"xxx");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From multibyte UTF16 value") {
                // We don't provide full support/conversions for multi-code-unit UTF16 for now,
                // but other uni-code-unit UTF16 strings should be fine.
                // In any case, UTF8 and UTF32 should be able to represent anything.
                // You can use UTF32 as an intermediary if you need a case that is not supported.
                std::allocator<int> alloc;
                string c(3, u'á', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c.size() == 6);
                REQUIRE(c == "ááá");
                REQUIRE(c == u"ááá");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From utf32 value") {
                std::allocator<int> alloc;
                string c(3, U'x', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size() == 3);
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c == "xxx");
                REQUIRE(c == U"xxx");
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From multibyte utf32 value") {
                std::allocator<int> alloc;
                string c(3, U'😀', alloc);
                REQUIRE_FALSE(c.empty());
                REQUIRE(c.size_codepoints() == 3);
                REQUIRE(c.size() == 12);
                REQUIRE(c == "😀😀😀");
                REQUIRE(c == U"😀😀😀");
                REQUIRE(c.get_allocator() == alloc);
            }
        }

        SECTION("From char iterators") {
            SECTION("Empty range") {
                std::allocator<int> alloc;
                std::string dv;
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE(d.empty());
                REQUIRE(d.size_codepoints() == 0);
                REQUIRE(d.size() == 0); // NOLINT(readability-container-size-empty)
                REQUIRE(d == "");       // NOLINT(readability-container-size-empty)
                REQUIRE(d == U"");      // NOLINT(readability-container-size-empty)
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("No unicode") {
                std::allocator<int> alloc;
                std::string dv = "654";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "654");
                REQUIRE(d == U"654");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Half unicode") {
                std::allocator<int> alloc;
                std::string dv = "😀6😀5😀4";
                REQUIRE(dv.size() == 15);
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "😀6😀5😀4");
                REQUIRE(d == U"😀6😀5😀4");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Full unicode") {
                std::allocator<int> alloc;
                std::string dv = "😀😀😀😀😀😀";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 24);
                REQUIRE(d == "😀😀😀😀😀😀");
                REQUIRE(d == U"😀😀😀😀😀😀");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From wide char iterators") {
            SECTION("No unicode") {
                std::allocator<int> alloc;
                std::u32string dv = U"654";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "654");
                REQUIRE(d == U"654");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Half unicode") {
                std::allocator<int> alloc;
                std::u32string dv = U"😀6😀5😀4";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "😀6😀5😀4");
                REQUIRE(d == U"😀6😀5😀4");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Full unicode") {
                std::allocator<int> alloc;
                std::u32string dv = U"😀😀😀😀😀😀";
                string d(dv.begin(), dv.end(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 24);
                REQUIRE(d == "😀😀😀😀😀😀");
                REQUIRE(d == U"😀😀😀😀😀😀");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From codepoint iterators") {
            SECTION("No unicode") {
                std::allocator<int> alloc;
                string dv = U"654";
                string d(dv.begin_codepoint(), dv.end_codepoint(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "654");
                REQUIRE(d == U"654");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Half unicode") {
                std::allocator<int> alloc;
                string dv = U"😀6😀5😀4";
                string d(dv.begin_codepoint(), dv.end_codepoint(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "😀6😀5😀4");
                REQUIRE(d == U"😀6😀5😀4");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Full unicode") {
                std::allocator<int> alloc;
                string dv = U"😀😀😀😀😀😀";
                string d(dv.begin_codepoint(), dv.end_codepoint(), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 24);
                REQUIRE(d == "😀😀😀😀😀😀");
                REQUIRE(d == U"😀😀😀😀😀😀");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From substring") {
            SECTION("From begin") {
                std::allocator<int> alloc;
                string dv = "123456";
                string d(dv, 3, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 3);
                REQUIRE(d.size() == 3);
                REQUIRE(d == "456");
                REQUIRE(d == U"456");
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("From range") {
                std::allocator<int> alloc;
                string dv = "123456";
                string d(dv, 2, 2, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 2);
                REQUIRE(d.size() == 2);
                REQUIRE(d == "34");
                REQUIRE(d == U"34");
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("From npos range") {
                std::allocator<int> alloc;
                string dv = "123456";
                string d(dv, 2, string::npos, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 4);
                REQUIRE(d.size() == 4);
                REQUIRE(d == "3456");
                REQUIRE(d == U"3456");
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("Literal string count") {
                SECTION("Char") {
                    std::allocator<int> alloc;
                    string d("123456", 2, alloc);
                    REQUIRE_FALSE(d.empty());
                    REQUIRE(d.size_codepoints() == 2);
                    REQUIRE(d.size() == 2);
                    REQUIRE(d == "12");
                    REQUIRE(d == U"12");
                    REQUIRE(d.get_allocator() == alloc);
                }

                SECTION("Wide char") {
                    std::allocator<int> alloc;
                    string d(U"123456", 2, alloc);
                    REQUIRE_FALSE(d.empty());
                    REQUIRE(d.size_codepoints() == 2);
                    REQUIRE(d.size() == 2);
                    REQUIRE(d == "12");
                    REQUIRE(d == U"12");
                    REQUIRE(d.get_allocator() == alloc);
                }
            }
        }

        SECTION("From literal") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d("123456", alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d(U"123456", alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From initializer list") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d({'1', '2', '3', '4', '5', '6'}, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d({U'1', U'2', U'3', U'4', U'5', U'6'}, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From string view") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d(std::string_view("123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d(std::u32string_view(U"123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("From std string") {
            SECTION("Char") {
                std::allocator<int> alloc;
                string d(std::string("123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
            SECTION("Wide char") {
                std::allocator<int> alloc;
                string d(std::u32string(U"123456"), alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 6);
                REQUIRE(d == "123456");
                REQUIRE(d == U"123456");
                REQUIRE(d.get_allocator() == alloc);
            }
        }

        SECTION("Rule of five") {
            SECTION("Copy") {
                string dv = "1😀2😀3😀";
                string d(dv);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(dv.size_codepoints() == 6);
                REQUIRE(dv.size() == 15);
                REQUIRE(dv == "1😀2😀3😀");
                REQUIRE(dv == U"1😀2😀3😀");
                REQUIRE(d == dv);
            }

            SECTION("Copy and set alloc") {
                std::allocator<int> alloc;
                string dv = "1😀2😀3😀";
                string d(dv, alloc);
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(dv.size_codepoints() == 6);
                REQUIRE(dv.size() == 15);
                REQUIRE(dv == "1😀2😀3😀");
                REQUIRE(dv == U"1😀2😀3😀");
                REQUIRE(d == dv);
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("Move") {
                string dv = "1😀2😀3😀";
                string d(std::move(dv));
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(dv.size_codepoints() == 0); // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(dv.size() == 0);            // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(dv == "");                  // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(dv == U"");                 // NOLINT(readability-container-size-empty,bugprone-use-after-move)
                REQUIRE(dv.empty());
                // is null terminated
                REQUIRE(dv[0] == '\0');
            }

            if constexpr (not is_windows()) {
                SECTION("Move and set alloc") {
                    std::allocator<int> alloc;
                    // There's no safe way to do that on MSVC :O
                    string dv = u8"1😀2😀3😀";
                    string d(std::move(dv), alloc);
                    REQUIRE_FALSE(d.empty());
                    REQUIRE(d.size_codepoints() == 6);
                    REQUIRE(d.size() == 15);
                    REQUIRE(d == "1😀2😀3😀");
                    REQUIRE(d == U"1😀2😀3😀");
                    REQUIRE(dv.size_codepoints() == 0); // NOLINT(bugprone-use-after-move)
                    REQUIRE(dv.size() == 0);            // NOLINT(readability-container-size-empty)
                    REQUIRE(dv == "");                  // NOLINT(readability-container-size-empty)
                    REQUIRE(dv == U"");                 // NOLINT(readability-container-size-empty)
                    REQUIRE(dv.empty());
                    // is null terminated
                    REQUIRE(dv[0] == '\0');
                    REQUIRE(d.get_allocator() == alloc);
                }
            }
        }
    }

    SECTION("Assignment Operator") {
        if constexpr (not is_windows()) {
            SECTION("String") {
                string dv = u8"1😀2😀3😀";
                string d;
                d = dv;
                REQUIRE_FALSE(d.empty());
                REQUIRE(d.size_codepoints() == 6);
                REQUIRE(d.size() == 15);
                REQUIRE(d == "1😀2😀3😀");
                REQUIRE(d == U"1😀2😀3😀");
                REQUIRE(dv.size_codepoints() == 6);
                REQUIRE(dv.size() == 15);
                REQUIRE(dv == "1😀2😀3😀");
                REQUIRE(dv == U"1😀2😀3😀");
                REQUIRE(d == dv);
            }
        }

        SECTION("Move String") {
            string dv = "1😀2😀3😀";
            string d;
            d = std::move(dv);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 15);
            REQUIRE(d == "1😀2😀3😀");
            REQUIRE(d == U"1😀2😀3😀");
            REQUIRE(dv.size_codepoints() == 0); // NOLINT(bugprone-use-after-move)
            REQUIRE(dv.size() == 0);            // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv == "");                  // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv == U"");                 // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE_FALSE(d == dv);
            REQUIRE(dv.size() == 0); // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv.front() == '\0');
        }

        SECTION("Literal") {
            string d;
            d = "1😀2😀3😀";
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 15);
            REQUIRE(d == "1😀2😀3😀");
            REQUIRE(d == U"1😀2😀3😀");
        }

        SECTION("Wide Literal") {
            string d;
            d = U"1😀2😀3😀";
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 15);
            REQUIRE(d == "1😀2😀3😀");
            REQUIRE(d == U"1😀2😀3😀");
        }

        SECTION("Char") {
            string d;
            d = '1';
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 1);
            REQUIRE(d.size() == 1);
            REQUIRE(d == "1");
            REQUIRE(d == U"1");
        }

        SECTION("Wide Char") {
            string d;
            d = U'1';
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 1);
            REQUIRE(d.size() == 1);
            REQUIRE(d == "1");
            REQUIRE(d == U"1");
        }

        SECTION("Unicode Wide Char") {
            string d;
            d = U'😀';
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 1);
            REQUIRE(d.size() == 4);
            REQUIRE(d == "😀");
            REQUIRE(d == U"😀");
        }
    }

    SECTION("Assign") {
        SECTION("Char") {
            string d;
            d.assign(3, '1');
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "111");
            REQUIRE(d == U"111");
        }

        SECTION("Wide Char") {
            string d;
            d.assign(3, U'1');
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "111");
            REQUIRE(d == U"111");
        }

        SECTION("Unicode Wide Char") {
            string d;
            d.assign(3, U'😀');
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 12);
            REQUIRE(d == "😀😀😀");
            REQUIRE(d == U"😀😀😀");
        }

        SECTION("Substring") {
            string dv = "123456";
            string d;
            d.assign(dv, 2, 2);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 2);
            REQUIRE(d.size() == 2);
            REQUIRE(d == "34");
            REQUIRE(d == U"34");
            REQUIRE(dv.size_codepoints() == 6);
            REQUIRE(dv.size() == 6);
            REQUIRE(dv == "123456");
            REQUIRE(dv == U"123456");
            REQUIRE_FALSE(d == dv);
        }

        SECTION("String") {
            string dv = "123456";
            string d;
            d.assign(dv);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
            REQUIRE(dv.size_codepoints() == 6);
            REQUIRE(dv.size() == 6);
            REQUIRE(dv == "123456");
            REQUIRE(dv == U"123456");
            REQUIRE(d == dv);
        }

        SECTION("String Move") {
            string dv = "123456";
            string d;
            d.assign(std::move(dv));
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
            REQUIRE(dv.size_codepoints() == 0); // NOLINT(bugprone-use-after-move)
            REQUIRE(dv.size() == 0);            // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv == "");                  // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv == U"");                 // NOLINT(bugprone-use-after-move,readability-container-size-empty)
            REQUIRE(dv.front() == '\0');
            REQUIRE_FALSE(d == dv);
        }

        SECTION("Literal") {
            string d;
            d.assign("123456", 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "123");
            REQUIRE(d == U"123");
        }

        SECTION("Wide Literal") {
            string d;
            d.assign(U"123456", 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 3);
            REQUIRE(d.size() == 3);
            REQUIRE(d == "123");
            REQUIRE(d == U"123");
        }

        SECTION("Initializer list") {
            string d;
            d.assign({'1', '2', '3', '4', '5', '6'});
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
        }

        SECTION("String view") {
            string d;
            d.assign(std::string_view("123456"));
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
        }

        SECTION("Wide string view") {
            string d;
            d.assign(std::u32string_view(U"123456"));
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 6);
            REQUIRE(d.size() == 6);
            REQUIRE(d == "123456");
            REQUIRE(d == U"123456");
        }

        SECTION("Substring view") {
            string d;
            d.assign(std::string_view("123456"), 2, 2);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 2);
            REQUIRE(d.size() == 2);
            REQUIRE(d == "34");
            REQUIRE(d == U"34");
        }

        SECTION("Wide string view") {
            string d;
            d.assign(std::u32string_view(U"123456"), 2, 2);
            REQUIRE_FALSE(d.empty());
            REQUIRE(d.size_codepoints() == 2);
            REQUIRE(d.size() == 2);
            REQUIRE(d == "34");
            REQUIRE(d == U"34");
        }
    }

    SECTION("Element access") {
        SECTION("At") {
            string s = "123456";
            REQUIRE(s.at(0) == '1');
            REQUIRE(s.at(1) == '2');
            REQUIRE(s.at(2) == '3');
            REQUIRE(s.at(3) == '4');
            REQUIRE(s.at(4) == '5');
            REQUIRE(s.at(5) == '6');
        }

        using cp_index = string::codepoint_index;

        SECTION("At codepoint (through references)") {
            SECTION("No unicode") {
                string s = "123456";
                REQUIRE(s.at(cp_index(0)) == '1');
                REQUIRE(s.at(cp_index(1)) == '2');
                REQUIRE(s.at(cp_index(2)) == '3');
                REQUIRE(s.at(cp_index(3)) == '4');
                REQUIRE(s.at(cp_index(4)) == '5');
                REQUIRE(s.at(cp_index(5)) == '6');
            }
            SECTION("Half unicode") {
                string s = "1😀2😀3😀";
                REQUIRE(s.at(cp_index(0)) == '1');
                REQUIRE(s.at(cp_index(1)) == U'😀');
                REQUIRE(s.at(cp_index(2)) == '2');
                REQUIRE(s.at(cp_index(3)) == U'😀');
                REQUIRE(s.at(cp_index(4)) == '3');
                REQUIRE(s.at(cp_index(5)) == U'😀');
                REQUIRE(s.at(cp_index(0)) == "1");
                REQUIRE(s.at(cp_index(1)) == "😀");
                REQUIRE(s.at(cp_index(2)) == "2");
                REQUIRE(s.at(cp_index(3)) == "😀");
                REQUIRE(s.at(cp_index(4)) == "3");
                REQUIRE(s.at(cp_index(5)) == "😀");
            }
            SECTION("Full unicode") {
                string s = "🙂😀🙂😀🙂😀";
                REQUIRE(s.at(cp_index(0)) == U'🙂');
                REQUIRE(s.at(cp_index(1)) == U'😀');
                REQUIRE(s.at(cp_index(2)) == U'🙂');
                REQUIRE(s.at(cp_index(3)) == U'😀');
                REQUIRE(s.at(cp_index(4)) == U'🙂');
                REQUIRE(s.at(cp_index(5)) == U'😀');
                REQUIRE(s.at(cp_index(0)) == "🙂");
                REQUIRE(s.at(cp_index(1)) == "😀");
                REQUIRE(s.at(cp_index(2)) == "🙂");
                REQUIRE(s.at(cp_index(3)) == "😀");
                REQUIRE(s.at(cp_index(4)) == "🙂");
                REQUIRE(s.at(cp_index(5)) == "😀");
            }
        }

        SECTION("Subscript") {
            string s = "123456";
            REQUIRE(s[0] == '1');
            REQUIRE(s[1] == '2');
            REQUIRE(s[2] == '3');
            REQUIRE(s[3] == '4');
            REQUIRE(s[4] == '5');
            REQUIRE(s[5] == '6');
        }

        SECTION("Subscript codepoint") {
            string s = "123456";
            REQUIRE(s[cp_index(0)] == '1');
            REQUIRE(s[cp_index(1)] == '2');
            REQUIRE(s[cp_index(2)] == '3');
            REQUIRE(s[cp_index(3)] == '4');
            REQUIRE(s[cp_index(4)] == '5');
            REQUIRE(s[cp_index(5)] == '6');
        }

        SECTION("Subscript codepoint (direct values)") {
            string s = "1😀2😀3😀";
            REQUIRE(s(cp_index(0)) == '1');
            REQUIRE(s(cp_index(1)) == U'😀');
            REQUIRE(s(cp_index(2)) == '2');
            REQUIRE(s(cp_index(3)) == U'😀');
            REQUIRE(s(cp_index(4)) == '3');
            REQUIRE(s(cp_index(5)) == U'😀');
        }

        SECTION("Subscript codepoint (through references)") {
            string s = "1😀2😀3😀";
            REQUIRE(s[cp_index(0)] == '1');
            REQUIRE(s[cp_index(1)] == U'😀');
            REQUIRE(s[cp_index(2)] == '2');
            REQUIRE(s[cp_index(3)] == U'😀');
            REQUIRE(s[cp_index(4)] == '3');
            REQUIRE(s[cp_index(5)] == U'😀');
        }

        SECTION("Front/Back") {
            string s = "1😀2😀3😀5";
            REQUIRE(s.front() == '1');
            REQUIRE(s.back() == '5');
        }

        SECTION("Front/Back Codepoints") {
            string s = "😀1😀2😀3😀5😀";
            REQUIRE(s.front_codepoint() == U'😀');
            REQUIRE(s.back_codepoint() == U'😀');
            REQUIRE(s.front_codepoint() == "😀");
            REQUIRE(s.back_codepoint() == "😀");
        }

        SECTION("Data") {
            string s = "1😀2😀3😀5";
            std::string_view sv(s.data(), s.size());
            REQUIRE(s == sv);
            REQUIRE(s.data() == s.c_str());
            REQUIRE(s.operator std::string_view() == sv);
        }
    }

    SECTION("Iterators") {
        SECTION("Byte Iterators") {
            string a = "123";
            REQUIRE(a.begin() == a.data());
            REQUIRE(a.end() == a.data() + a.size());

            REQUIRE(*a.begin() == '1');
            REQUIRE(*std::next(a.begin()) == '2');
            REQUIRE(*std::prev(a.end()) == '3');

            REQUIRE(a.cbegin() == a.data());
            REQUIRE(a.cend() == a.data() + a.size());

            REQUIRE(*a.cbegin() == '1');
            REQUIRE(*std::next(a.cbegin()) == '2');
            REQUIRE(*std::prev(a.cend()) == '3');

            REQUIRE(*a.rbegin() == '3');
            REQUIRE(*std::next(a.rbegin()) == '2');
            REQUIRE(*std::prev(a.rend()) == '1');

            REQUIRE(*a.crbegin() == '3');
            REQUIRE(*std::next(a.crbegin()) == '2');
            REQUIRE(*std::prev(a.crend()) == '1');
        }

        SECTION("Codepoint Iterators") {
            string a = "😐🙂😀";
            REQUIRE(static_cast<size_t>(a.end_codepoint() - a.begin_codepoint()) == a.size_codepoints());

            REQUIRE(*a.begin_codepoint() == U'😐');
            REQUIRE(*std::next(a.begin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.end_codepoint()) == U'😀');

            REQUIRE(*a.cbegin_codepoint() == a.front_codepoint());
            REQUIRE(*std::prev(a.cend_codepoint()) == a.back_codepoint());

            REQUIRE(*a.cbegin_codepoint() == U'😐');
            REQUIRE(*std::next(a.cbegin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.cend_codepoint()) == U'😀');

            REQUIRE(*a.rbegin_codepoint() == U'😀');
            REQUIRE(*std::next(a.rbegin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.rend_codepoint()) == U'😐');

            REQUIRE(*a.crbegin_codepoint() == U'😀');
            REQUIRE(*std::next(a.crbegin_codepoint()) == U'🙂');
            REQUIRE(*std::prev(a.crend_codepoint()) == U'😐');
        }
    }

    SECTION("Capacity") {
        string a = U"1😀3";

        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= 13);
        REQUIRE(a.capacity() <= 15);
        size_t old_cap = a.capacity();

        a.reserve(10);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        size_t new_cap = a.capacity();
        REQUIRE(new_cap >= old_cap);

        a.reserve(20);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        new_cap = a.capacity();
        REQUIRE(new_cap > old_cap);

        a.shrink_to_fit();
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.size() == 6);
        REQUIRE(a.size_codepoints() == 3);
        REQUIRE(a.max_size() > 100000);
        REQUIRE_FALSE(a.empty());
        new_cap = a.capacity();
        REQUIRE(new_cap >= 6); // larger than initial size but might not be inline anymore

        a = U"1😀3";
        a.shrink_to_fit();
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() >= a.size());
        REQUIRE_FALSE(is_malformed(a));
    }

    SECTION("Resize") {
        SECTION("Code units") {
            string a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(4);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() >= 13);
            REQUIRE(a.capacity() <= 15);
            REQUIRE(a.size_codepoints() == 1);
            REQUIRE(is_malformed(a));

            a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(20);
            REQUIRE(a.size() == 20);
            REQUIRE(a.max_size() > 20);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 20);
            REQUIRE(a.size_codepoints() == 17);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(14, 'x');
            REQUIRE(a.size() == 6 + 8);
            REQUIRE(a.size_codepoints() == 3 + 8);
            REQUIRE(a.max_size() > 14);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(14, U'x');
            REQUIRE(a.size() == 6 + 8);
            REQUIRE(a.size_codepoints() == 3 + 8);
            REQUIRE(a.max_size() > 14);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";          // <- size 6
            a.resize(14, U'😀'); // <- size 6 + 8 = 14 (two extra codepoints)
            REQUIRE(a.size_codepoints() == 5);
            REQUIRE(a.size() == 14);
            REQUIRE(a.max_size() > 40);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.shrink_to_fit();
            REQUIRE(a.size() == 6);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > a.size());
            REQUIRE_FALSE(is_malformed(a));
        }

        SECTION("Code points") {
            string a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(string::codepoint_index(4));
            REQUIRE(a.size() == 7);
            REQUIRE(a.max_size() > 7);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() >= 13);
            REQUIRE(a.capacity() <= 17);
            REQUIRE(a.size_codepoints() == 4);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            REQUIRE_FALSE(is_malformed(a));
            a.resize(string::codepoint_index(20));
            REQUIRE(a.size() == 23);
            REQUIRE(a.max_size() > 23);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 23);
            REQUIRE(a.size_codepoints() == 20);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(string::codepoint_index(14), 'x');
            REQUIRE(a.size() == 6 + 14 - 3);
            REQUIRE(a.size_codepoints() == 3 + 11);
            REQUIRE(a.max_size() > 14);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(string::codepoint_index(14), U'x');
            REQUIRE(a.size() == 6 + 11);
            REQUIRE(a.size_codepoints() == 3 + 11);
            REQUIRE(a.max_size() > 17);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 17);
            REQUIRE_FALSE(is_malformed(a));

            a = "1😀3";
            a.resize(string::codepoint_index(14), U'😀');
            REQUIRE(a.size_codepoints() == 14);
            REQUIRE(a.size() == 12 * 4 + 2);
            REQUIRE(a.max_size() > 40);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() > 14);
            REQUIRE_FALSE(is_malformed(a));
        }
    }

    SECTION("Clear") {
        string a = "1😀3";
        a.clear();
        REQUIRE(a.empty());
        REQUIRE(a.size() == 0); // NOLINT(readability-container-size-empty)
        REQUIRE(a.size_codepoints() == 0);
        REQUIRE(a.max_size() > 10);
        REQUIRE(a.capacity() > a.size());
        REQUIRE_FALSE(is_malformed(a));
    }

    SECTION("Insert") {
        SECTION("Char") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(2, 1, '3');
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(3, 3, '.');
                    REQUIRE(a == "abc...z");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(3, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), 1, '3');
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), 3, '.');
                    REQUIRE(a == "😐🙂...😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), 3, U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }
            }

            SECTION("At iterator") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(a.begin() + 2, 1, '3');
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, 3, '.');
                    REQUIRE(a == "abc...z");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀z");
                }
            }

            SECTION("At codepoint iterator") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), 1, '3');
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), 3, '.');
                    REQUIRE(a == "😐🙂...😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), 3, U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }
            }
        }

        SECTION("Literal string") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(2, "3");
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(3, "defgh");
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(3, U"🙂😀");
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "3");
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "defgh");
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), U"🙂😀");
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Partial literal string") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(2, "3456", 1);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(3, "defghijklmn", 5);
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(3, U"🙂😀🙂😀🙂😀", 2);
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "3456", 1);
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), "defghijkl", 5);
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    a.insert(string::codepoint_index(2), U"🙂😀🙂😀🙂😀", 2);
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Other string") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3");
                    a.insert(2, other);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defgh");
                    a.insert(3, other);
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀");
                    a.insert(3, other);
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defgh");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Other string suffix") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(2, other, 1);
                    REQUIRE(a == "124564");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(3, other, 5);
                    REQUIRE(a == "abcijklmnz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, 8);
                    REQUIRE(a == "abc🙂😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(string::codepoint_index(2), other, 1);
                    REQUIRE(a == "😐🙂456😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(string::codepoint_index(2), other, 5);
                    REQUIRE(a == "😐🙂ijkl😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, 8);
                    REQUIRE(a == "😐🙂🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Other string codepoint suffix") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(2, other, string::codepoint_index(1));
                    REQUIRE(a == "124564");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(3, other, string::codepoint_index(5));
                    REQUIRE(a == "abcijklmnz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, string::codepoint_index(2));
                    REQUIRE(a == "abc🙂😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(string::codepoint_index(2), other, string::codepoint_index(1));
                    REQUIRE(a == "😐🙂456😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(string::codepoint_index(2), other, string::codepoint_index(5));
                    REQUIRE(a == "😐🙂ijkl😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, string::codepoint_index(2));
                    REQUIRE(a == "😐🙂🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Other string substr") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(2, other, 0, 1);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(3, other, 1, 3);
                    REQUIRE(a == "abcefgz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, 4, 12);
                    REQUIRE(a == "abc😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(string::codepoint_index(2), other, 1, 2);
                    REQUIRE(a == "😐🙂45😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(string::codepoint_index(2), other, 5, 2);
                    REQUIRE(a == "😐🙂ij😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, 12, 8);
                    REQUIRE(a == "😐🙂😀🙂😐");
                }
            }
        }

        SECTION("Other string codepoint substr") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    string other("3456");
                    a.insert(2, other, string::codepoint_index(1), string::codepoint_index(2));
                    REQUIRE(a == "12454");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    string other("defghijklmn");
                    a.insert(3, other, string::codepoint_index(1), string::codepoint_index(3));
                    REQUIRE(a == "abcefgz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, string::codepoint_index(2), string::codepoint_index(3));
                    REQUIRE(a == "abc🙂😀🙂z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    string other("3456");
                    a.insert(string::codepoint_index(2), other, string::codepoint_index(1), string::codepoint_index(2));
                    REQUIRE(a == "😐🙂45😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    string other("defghijkl");
                    a.insert(string::codepoint_index(2), other, string::codepoint_index(5), string::codepoint_index(2));
                    REQUIRE(a == "😐🙂ij😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    string other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, string::codepoint_index(3), string::codepoint_index(2));
                    REQUIRE(a == "😐🙂😀🙂😐");
                }
            }
        }

        SECTION("Single char") {
            SECTION("At index") {
                SECTION("Unibyte") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, 'd');
                    REQUIRE(a == "abcdz");
                }
                SECTION("Multibyte") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, U'🙂');
                    REQUIRE(a == "abc🙂z");
                }
            }

            SECTION("At codepoint") {
                SECTION("Unibyte") {
                    string a = "😐🙂😐";
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), '3');
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multibyte") {
                    string a = "😐🙂😐";
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), U'😀');
                    REQUIRE(a == "😐🙂😀😐");
                }
            }
        }

        SECTION("Other container iterator") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    std::string other("3");
                    a.insert(a.begin() + 2, other.begin(), other.end());
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    std::string other("defgh");
                    a.insert(a.begin() + 3, other.begin(), other.end());
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    std::u32string other(U"🙂😀");
                    a.insert(a.begin() + 3, other.begin(), other.end());
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    std::string other("3");
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), other.begin(), other.end());
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    std::string other("defgh");
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), other.begin(), other.end());
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string other(U"🙂😀");
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), other.begin(), other.end());
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("Initializer list") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    a.insert(a.begin() + 2, {'3'});
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, {'d', 'e', 'f', 'g', 'h'});
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    a.insert(a.begin() + 3, {U'🙂', U'😀'});
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), {'3'});
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), {'d', 'e', 'f', 'g', 'h'});
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string other(U"🙂😀");
                    a.insert(a.begin_codepoint() + string::codepoint_index(2), {U'🙂', U'😀'});
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("String view") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    std::string_view other("3");
                    a.insert(2, other);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    std::string_view other("defgh");
                    a.insert(3, other);
                    REQUIRE(a == "abcdefghz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    std::u32string_view other(U"🙂😀");
                    a.insert(3, other);
                    REQUIRE(a == "abc🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    std::string_view other("3");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂3😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    std::string_view other("defgh");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂defgh😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string_view other(U"🙂😀");
                    a.insert(string::codepoint_index(2), other);
                    REQUIRE(a == "😐🙂🙂😀😐");
                }
            }
        }

        SECTION("String view substr") {
            SECTION("At index") {
                SECTION("One element") {
                    string a = "124";
                    std::string_view other("3456");
                    a.insert(2, other, 0, 1);
                    REQUIRE(a == "1234");
                }
                SECTION("Multiple elements") {
                    string a = "abcz";
                    std::string_view other("defghijklmn");
                    a.insert(3, other, 1, 3);
                    REQUIRE(a == "abcefgz");
                }
                SECTION("Unicode") {
                    string a = "abcz";
                    std::u32string_view other(U"🙂😀🙂😀🙂😀");
                    a.insert(3, other, 1, 3);
                    REQUIRE(a == "abc😀🙂😀z");
                }
            }

            SECTION("At codepoint") {
                SECTION("One element") {
                    string a = "😐🙂😐";
                    std::string_view other("3456");
                    a.insert(string::codepoint_index(2), other, 1, 2);
                    REQUIRE(a == "😐🙂45😐");
                }
                SECTION("Multiple elements") {
                    string a = "😐🙂😐";
                    std::string_view other("defghijkl");
                    a.insert(string::codepoint_index(2), other, 5, 2);
                    REQUIRE(a == "😐🙂ij😐");
                }
                SECTION("Unicode") {
                    string a = "😐🙂😐";
                    std::u32string_view other(U"🙂😀🙂😀🙂😀");
                    a.insert(string::codepoint_index(2), other, 1, 3);
                    REQUIRE(a == "😐🙂😀🙂😀😐");
                }
            }
        }
    }

    SECTION("Erase") {
        SECTION("Index suffix") {
            string a = "abcdefghij";
            a.erase(3);
            REQUIRE(a == "abc");
        }
        SECTION("Index substr") {
            string a = "abcdefghij";
            a.erase(3, 2);
            REQUIRE(a == "abcfghij");
        }
        SECTION("Codepoint suffix") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(string::codepoint_index(3));
            REQUIRE(a == "😐🙂😀");
        }
        SECTION("Codepoint substr") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(string::codepoint_index(3), string::codepoint_index(2));
            REQUIRE(a == "😐🙂😀😐");
        }
        SECTION("Single position") {
            string a = "abcdefghij";
            a.erase(a.begin() + 3);
            REQUIRE(a == "abcefghij");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(a.begin_codepoint() + string::codepoint_index(3));
            REQUIRE(a == "😐🙂😀😀😐");
        }
        SECTION("Index range") {
            string a = "abcdefghij";
            a.erase(a.begin() + 3, a.begin() + 5);
            REQUIRE(a == "abcfghij");
        }
        SECTION("Codepoint range") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(a.begin_codepoint() + string::codepoint_index(3), a.begin_codepoint() + string::codepoint_index(5));
            REQUIRE(a == "😐🙂😀😐");
        }
    }

    SECTION("Push back") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.push_back('k');
            REQUIRE(a == "abcdefghijk");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.push_back(U'😀');
            REQUIRE(a == "😐🙂😀🙂😀😐😀");
        }
    }

    SECTION("Pop back") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.pop_back();
            REQUIRE(a == "abcdefghi");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.pop_back_codepoint();
            REQUIRE(a == "😐🙂😀🙂😀");
        }
    }

    SECTION("Push \"front\"") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.insert(a.begin(), 'k');
            REQUIRE(a == "kabcdefghij");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.insert(a.begin(), U'😀');
            REQUIRE(a == "😀😐🙂😀🙂😀😐");
        }
    }

    SECTION("Pop \"front\"") {
        SECTION("Single position") {
            string a = "abcdefghij";
            a.erase(a.begin());
            REQUIRE(a == "bcdefghij");
        }
        SECTION("Single codepoint") {
            string a = "😐🙂😀🙂😀😐";
            a.erase(a.begin_codepoint());
            REQUIRE(a == "🙂😀🙂😀😐");
        }
    }

    SECTION("Append") {
        SECTION("Chars") {
            SECTION("Single position") {
                string a = "abcdefghij";
                a.append(3, 'k');
                REQUIRE(a == "abcdefghijkkk");
            }
            SECTION("Single codepoint") {
                string a = "😐🙂😀🙂😀😐";
                a.append(3, U'😀');
                REQUIRE(a == "😐🙂😀🙂😀😐😀😀😀");
            }
        }
        SECTION("Other string") {
            SECTION("Complete") {
                string a = "abcdefghij";
                string b = "klmnop";
                a.append(b);
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Suffix") {
                string a = "abcdefghij";
                string b = "klmnop";
                a.append(b, 2);
                REQUIRE(a == "abcdefghijmnop");
            }
            SECTION("Substr") {
                string a = "abcdefghij";
                string b = "klmnop";
                a.append(b, 2, 3);
                REQUIRE(a == "abcdefghijmno");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a.append(b);
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Suffix") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a.append(b, string::codepoint_index(2));
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀😐");
            }
            SECTION("Codepoint Substr") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a.append(b, string::codepoint_index(2), string::codepoint_index(3));
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀");
            }
        }
        SECTION("String Literal") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a.append("klmnop");
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Prefix") {
                string a = "abcdefghij";
                a.append("klmnop", 2);
                REQUIRE(a == "abcdefghijkl");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a.append("😐🙂😀🙂😀😐");
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Prefix") {
                string a = "😐🙂😀🙂😀😐";
                a.append(U"😐🙂😀🙂😀😐", 2);
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂");
            }
        }

        SECTION("Iterator ranges") {
            SECTION("Complete") {
                string a = "abcdefghij";
                std::string b = "klmnop";
                a.append(b.begin(), b.end());
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Prefix") {
                string a = "abcdefghij";
                std::string b = "klmnop";
                a.append(b.begin(), b.begin() + 2);
                REQUIRE(a == "abcdefghijkl");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string b = U"😐🙂😀🙂😀😐";
                a.append(b.begin(), b.end());
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Prefix") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string b = U"😐🙂😀🙂😀😐";
                a.append(b.begin(), b.begin() + 2);
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂");
            }
        }

        SECTION("Initializer list") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a.append({'k', 'l', 'm', 'n', 'o', 'p'});
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a.append({U'😐', U'🙂', U'😀', U'🙂', U'😀', U'😐'});
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }

        SECTION("Other string view") {
            SECTION("Complete") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a.append(b);
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Suffix") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a.append(b, 2);
                REQUIRE(a == "abcdefghijmnop");
            }
            SECTION("Substr") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a.append(b, 2, 3);
                REQUIRE(a == "abcdefghijmno");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                std::string_view b = "😐🙂😀🙂😀😐";
                a.append(b);
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
            SECTION("Codepoint Suffix") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string_view b = U"😐🙂😀🙂😀😐";
                a.append(b, 2);
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀😐");
            }
            SECTION("Codepoint Substr") {
                string a = "😐🙂😀🙂😀😐";
                std::u32string_view b = U"😐🙂😀🙂😀😐";
                a.append(b, 2, 3);
                REQUIRE(a == "😐🙂😀🙂😀😐😀🙂😀");
            }
        }
    }

    SECTION("Operator +=") {
        SECTION("Other string") {
            SECTION("Complete") {
                string a = "abcdefghij";
                string b = "klmnop";
                a += b;
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                string b = "😐🙂😀🙂😀😐";
                a += b;
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }
        SECTION("Chars") {
            SECTION("Single position") {
                string a = "abcdefghij";
                a += 'k';
                REQUIRE(a == "abcdefghijk");
            }
            SECTION("Single codepoint") {
                string a = "😐🙂😀🙂😀😐";
                a += U'😀';
                REQUIRE(a == "😐🙂😀🙂😀😐😀");
            }
        }
        SECTION("String Literal") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a += "klmnop";
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a += "😐🙂😀🙂😀😐";
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }

        SECTION("Initializer list") {
            SECTION("Complete") {
                string a = "abcdefghij";
                a += {'k', 'l', 'm', 'n', 'o', 'p'};
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                a += {U'😐', U'🙂', U'😀', U'🙂', U'😀', U'😐'};
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }

        SECTION("Other string view") {
            SECTION("Complete") {
                string a = "abcdefghij";
                std::string_view b = "klmnop";
                a += b;
                REQUIRE(a == "abcdefghijklmnop");
            }
            SECTION("Codepoint Complete") {
                string a = "😐🙂😀🙂😀😐";
                std::string_view b = "😐🙂😀🙂😀😐";
                a += b;
                REQUIRE(a == "😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
            }
        }
    }

    SECTION("Starts with") {
        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Starts not-empty") {
                    std::string_view b = "abcde";
                    REQUIRE(a.starts_with(b));
                }

                SECTION("Might find but does not start with") {
                    std::string_view b = "bcdef";
                    REQUIRE_FALSE(a.starts_with(b));
                }

                SECTION("Always start with empty") {
                    std::string_view b = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.starts_with(b));
                }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    std::string_view b = "bcdef";
                    REQUIRE_FALSE(a.starts_with(b));
                }

                SECTION("Always start if both empty") {
                    a.clear();
                    std::string_view b = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.starts_with(b));
                }
            }

            if constexpr (not is_windows()) {
                SECTION("UTF32 rhs") {
                    string a = "😐🙂😀🙂😀😐";
                    SECTION("Starts not-empty") {
                        std::u32string_view b = U"😐🙂😀";
                        REQUIRE(a.starts_with(b));
                    }

                    SECTION("Might find but does not start with") {
                        std::u32string_view b = U"🙂😀🙂";
                        REQUIRE_FALSE(a.starts_with(b));
                    }

                    SECTION("Always start with empty") {
                        std::u32string_view b = U""; // NOLINT(readability-redundant-string-init)
                        REQUIRE(a.starts_with(b));
                    }

                    SECTION("Cannot start with if empty") {
                        a.clear();
                        std::u32string_view b = U"😐🙂😀";
                        REQUIRE_FALSE(a.starts_with(b));
                    }

                    SECTION("Always start if both empty") {
                        a.clear();
                        std::u32string_view b = U""; // NOLINT(readability-redundant-string-init)
                        REQUIRE(a.starts_with(b));
                    }
                }
            }
        }

        SECTION("Char") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Starts not-empty") { REQUIRE(a.starts_with('a')); }

                SECTION("Might find but does not start with") { REQUIRE_FALSE(a.starts_with('b')); }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with('a'));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Starts not-empty") { REQUIRE(a.starts_with(U'😐')); }

                SECTION("Might find but does not start with") { REQUIRE_FALSE(a.starts_with(U'🙂')); }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with(U'😐'));
                }
            }
        }

        SECTION("String literal") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Starts not-empty") { REQUIRE(a.starts_with("abcde")); }

                SECTION("Might find but does not start with") { REQUIRE_FALSE(a.starts_with("bcdef")); }

                SECTION("Always start with empty") { REQUIRE(a.starts_with("")); }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with("bcdef"));
                }

                SECTION("Always start if both empty") {
                    a.clear();
                    REQUIRE(a.starts_with(""));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Starts not-empty") { REQUIRE(a.starts_with(U"😐🙂😀")); }

                SECTION("Might find but does not start with") { REQUIRE_FALSE(a.starts_with(U"🙂😀🙂")); }

                SECTION("Always start with empty") { REQUIRE(a.starts_with(U"")); }

                SECTION("Cannot start with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.starts_with(U"😐🙂😀"));
                }

                SECTION("Always start if both empty") {
                    a.clear();
                    REQUIRE(a.starts_with(U""));
                }
            }
        }
    }

    SECTION("Ends with") {
        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Ends not-empty") {
                    std::string_view b = "ghij";
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Might find but does not end with") {
                    std::string_view b = "bcdef";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end with empty") {
                    std::string_view b = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    std::string_view b = "ghij";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    std::string_view b = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Ends not-empty") {
                    std::u32string_view b = U"🙂😀😐";
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Might find but does not end with") {
                    std::u32string_view b = U"🙂😀🙂";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end with empty") {
                    std::u32string_view b = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    std::u32string_view b = U"🙂😀😐";
                    REQUIRE_FALSE(a.ends_with(b));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    std::u32string_view b = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.ends_with(b));
                }
            }
        }

        SECTION("Char") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Ends not-empty") { REQUIRE(a.ends_with('j')); }

                SECTION("Might find but does not end with") { REQUIRE_FALSE(a.ends_with('b')); }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with('j'));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Ends not-empty") { REQUIRE(a.ends_with(U'😐')); }

                SECTION("Might find but does not end with") { REQUIRE_FALSE(a.ends_with(U'🙂')); }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with(U'😐'));
                }
            }
        }

        SECTION("String literal") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Ends not-empty") { REQUIRE(a.ends_with("ghij")); }

                SECTION("Might find but does not end with") { REQUIRE_FALSE(a.ends_with("bcdef")); }

                SECTION("Always end with empty") { REQUIRE(a.ends_with("")); }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with("bcdef"));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    REQUIRE(a.ends_with(""));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Ends not-empty") { REQUIRE(a.ends_with(U"🙂😀😐")); }

                SECTION("Might find but does not end with") { REQUIRE_FALSE(a.ends_with(U"🙂😀🙂")); }

                SECTION("Always end with empty") { REQUIRE(a.ends_with(U"")); }

                SECTION("Cannot end with if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.ends_with(U"🙂😀😐"));
                }

                SECTION("Always end if both empty") {
                    a.clear();
                    REQUIRE(a.ends_with(U""));
                }
            }
        }
    }

    SECTION("Contains") {
        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Contains start") {
                    std::string_view b = "abc";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains middle") {
                    std::string_view b = "def";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains end") {
                    std::string_view b = "hij";
                    REQUIRE(a.contains(b));
                }

                SECTION("Does not contain") {
                    std::string_view b = "ijk";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains empty") {
                    std::string_view b = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    std::string_view b = "ghij";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    std::string_view b = ""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Contains start") {
                    std::u32string_view b = U"😐🙂";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains middle") {
                    std::u32string_view b = U"😀🙂";
                    REQUIRE(a.contains(b));
                }

                SECTION("Contains end") {
                    std::u32string_view b = U"😀😐";
                    REQUIRE(a.contains(b));
                }

                SECTION("Does not contain") {
                    std::u32string_view b = U"😐😀";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains empty") {
                    std::u32string_view b = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    std::u32string_view b = U"😐🙂";
                    REQUIRE_FALSE(a.contains(b));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    std::u32string_view b = U""; // NOLINT(readability-redundant-string-init)
                    REQUIRE(a.contains(b));
                }
            }
        }

        SECTION("Char") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Start") { REQUIRE(a.contains('a')); }

                SECTION("Middle") { REQUIRE(a.contains('f')); }

                SECTION("End") { REQUIRE(a.contains('j')); }

                SECTION("Cannot contains if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains('j'));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Start") { REQUIRE(a.contains(U'😐')); }

                SECTION("Middle") { REQUIRE(a.contains(U'🙂')); }

                SECTION("End") { REQUIRE(a.contains(U'😐')); }

                SECTION("Cannot contains if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains(U'😐'));
                }
            }
        }

        SECTION("String view") {
            SECTION("UTF8 rhs") {
                string a = "abcdefghij";
                SECTION("Contains start") { REQUIRE(a.contains("abc")); }

                SECTION("Contains middle") { REQUIRE(a.contains("def")); }

                SECTION("Contains end") { REQUIRE(a.contains("hij")); }

                SECTION("Does not contain") { REQUIRE_FALSE(a.contains("ijk")); }

                SECTION("Always contains empty") { REQUIRE(a.contains("")); }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains("ghij"));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    REQUIRE(a.contains(""));
                }
            }
            SECTION("UTF32 rhs") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Contains start") { REQUIRE(a.contains(U"😐🙂")); }

                SECTION("Contains middle") { REQUIRE(a.contains(U"😀🙂")); }

                SECTION("Contains end") { REQUIRE(a.contains(U"😀😐")); }

                SECTION("Does not contain") { REQUIRE_FALSE(a.contains(U"😐😀")); }

                SECTION("Always contains empty") { REQUIRE(a.contains(U"")); }

                SECTION("Cannot contain if empty") {
                    a.clear();
                    REQUIRE_FALSE(a.contains(U"😐🙂"));
                }

                SECTION("Always contains if both empty") {
                    a.clear();
                    REQUIRE(a.contains(U""));
                }
            }
        }
    }

    SECTION("Replace") {
        SECTION("Other string") {
            SECTION("Replace code units") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    string b = "xxx";
                    a.replace(a.begin(), a.begin() + 3, b);
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    string b = "xxx";
                    a.replace(a.begin() + 3, a.begin() + 6, b);
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    string b = "xxx";
                    a.replace(a.begin() + 7, a.begin() + 10, b);
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code points") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    string b = "xxx";
                    a.replace(a.begin_codepoint(), a.begin_codepoint() + 3, b);
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    string b = "xxx";
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, b);
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    string b = "xxx";
                    a.replace(a.begin_codepoint() + 4, a.begin_codepoint() + 6, b);
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code units with substr") {
                string a = "abcdefghij";

                SECTION("Replace with suffix") {
                    string b = "123";
                    a.replace(3, 3, b, 1);
                    REQUIRE(a == "abc23ghij");
                }

                SECTION("Replace with substr") {
                    string b = "123";
                    a.replace(3, 3, b, 1, 1);
                    REQUIRE(a == "abc2ghij");
                }

                SECTION("Replace with code point suffix") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(3, 3, b, string::codepoint_index(2));
                    REQUIRE(a == "abc😀🙂😀😐ghij");
                }

                SECTION("Replace with code point substr") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(3, 3, b, string::codepoint_index(2), string::codepoint_index(2));
                    REQUIRE(a == "abc😀🙂ghij");
                }
            }

            SECTION("Replace code points with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace with suffix") {
                    string b = "123";
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), b, 1);
                    REQUIRE(a == "😐🙂23😀😐");
                }

                SECTION("Replace with substr") {
                    string b = "123";
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), b, 1, 1);
                    REQUIRE(a == "😐🙂2😀😐");
                }

                SECTION("Replace with code point suffix") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), b, string::codepoint_index(2));
                    REQUIRE(a == "😐🙂😀🙂😀😐😀😐");
                }

                SECTION("Replace with code point substr") {
                    string b = "😐🙂😀🙂😀😐";
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), b, string::codepoint_index(2),
                              string::codepoint_index(2));
                    REQUIRE(a == "😐🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Iterators") {
            SECTION("Code units iterator") {
                string a = "abcdefghij";
                SECTION("Rhs is smaller") {
                    string b = "123";
                    a.replace(a.begin() + 3, a.begin() + 5, b.begin() + 1, b.begin() + 2);
                    REQUIRE(a == "abc2fghij");
                }

                SECTION("Rhs is same size") {
                    string b = "123";
                    a.replace(a.begin() + 3, a.begin() + 5, b.begin() + 1, b.begin() + 3);
                    REQUIRE(a == "abc23fghij");
                }

                SECTION("Rhs is larger") {
                    string b = "123";
                    a.replace(a.begin() + 3, a.begin() + 5, b.begin() + 0, b.begin() + 3);
                    REQUIRE(a == "abc123fghij");
                }
            }

            SECTION("Code point iterator") {
                string a = "😐🙂😀🙂😀😐";
                SECTION("Rhs is smaller") {
                    string b = "🙃🙃🙃🙃";
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, b.begin_codepoint() + 1,
                              b.begin_codepoint() + 2);
                    REQUIRE(a == "😐🙂🙃😀😐");
                }

                SECTION("Rhs is same size") {
                    string b = "🙃🙃🙃🙃";
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, b.begin_codepoint() + 1,
                              b.begin_codepoint() + 3);
                    REQUIRE(a == "😐🙂🙃🙃😀😐");
                }

                SECTION("Rhs is larger") {
                    string b = "🙃🙃🙃🙃";
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, b.begin_codepoint() + 1,
                              b.begin_codepoint() + 4);
                    REQUIRE(a == "😐🙂🙃🙃🙃😀😐");
                }
            }
        }

        SECTION("String literal") {
            SECTION("Replace code unit indexes") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(0, 0 + 3, "xxx");
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(3, 3, "xxx");
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    a.replace(7, 3, "xxx");
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code unit indexes with substr") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(0, 0 + 3, "xxx", 2);
                    REQUIRE(a == "xxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(3, 3, "xxx", 2);
                    REQUIRE(a == "abcxxghij");
                }

                SECTION("Replace end") {
                    a.replace(7, 3, "xxx", 2);
                    REQUIRE(a == "abcdefgxx");
                }
            }

            SECTION("Replace code point indexes") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(string::codepoint_index(0), string::codepoint_index(3), U"🙃🙃🙃🙃");
                    REQUIRE(a == "🙃🙃🙃🙃🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), U"🙃🙃🙃🙃");
                    REQUIRE(a == "😐🙂🙃🙃🙃🙃😀😐");
                }

                SECTION("Replace end") {
                    a.replace(string::codepoint_index(4), string::codepoint_index(2), U"🙃🙃🙃🙃");
                    REQUIRE(a == "😐🙂😀🙂🙃🙃🙃🙃");
                }
            }

            SECTION("Replace code point indexes with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(string::codepoint_index(0), string::codepoint_index(2), U"🙃🙃🙃🙃", 2);
                    REQUIRE(a == "🙃🙃😀🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), U"🙃🙃🙃🙃", 2);
                    REQUIRE(a == "😐🙂🙃🙃😀😐");
                }

                SECTION("Replace end") {
                    a.replace(string::codepoint_index(4), string::codepoint_index(2), U"🙃🙃🙃🙃", 2);
                    REQUIRE(a == "😐🙂😀🙂🙃🙃");
                }
            }

            SECTION("Replace code units iterators") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(a.begin(), a.begin() + 3, "xxx");
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin() + 3, a.begin() + 6, "xxx");
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    a.replace(a.begin() + 7, a.begin() + 10, "xxx");
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code units iterators with substr") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(a.begin(), a.begin() + 3, "xxx", 2);
                    REQUIRE(a == "xxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin() + 3, a.begin() + 6, "xxx", 2);
                    REQUIRE(a == "abcxxghij");
                }

                SECTION("Replace end") {
                    a.replace(a.begin() + 7, a.begin() + 10, "xxx", 2);
                    REQUIRE(a == "abcdefgxx");
                }
            }

            SECTION("Replace code points") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(a.begin_codepoint(), a.begin_codepoint() + 3, "xxx");
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, "xxx");
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    a.replace(a.begin_codepoint() + 4, a.begin_codepoint() + 6, "xxx");
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code points with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(a.begin_codepoint(), a.begin_codepoint() + 3, "xxx", 2);
                    REQUIRE(a == "xx🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, "xxx", 2);
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Replace end") {
                    a.replace(a.begin_codepoint() + 4, a.begin_codepoint() + 6, "xxx", 2);
                    REQUIRE(a == "😐🙂😀🙂xx");
                }
            }

            SECTION("Replace code units with substr") {
                string a = "abcdefghij";

                SECTION("Replace with suffix") {
                    a.replace(3, 3, "123", 1);
                    REQUIRE(a == "abc1ghij");
                }

                SECTION("Replace with substr") {
                    a.replace(3, 3, "123", 1, 1);
                    REQUIRE(a == "abc2ghij");
                }

                SECTION("Replace with code point suffix") {
                    a.replace(3, 3, "😐🙂😀🙂😀😐", 8);
                    REQUIRE(a == "abc😐🙂ghij");
                }

                SECTION("Replace with code point substr") {
                    a.replace(3, 3, "😐🙂😀🙂😀😐", 8, 8);
                    REQUIRE(a == "abc😀🙂ghij");
                }
            }

            SECTION("Replace code points with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace with suffix") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), "123", 1);
                    REQUIRE(a == "😐🙂1😀😐");
                }

                SECTION("Replace with substr") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), "123", 1, 1);
                    REQUIRE(a == "😐🙂2😀😐");
                }

                SECTION("Replace with code point suffix") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), "😐🙂😀🙂😀😐", 8);
                    REQUIRE(a == "😐🙂😐🙂😀😐");
                }

                SECTION("Replace with code point substr") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), "😐🙂😀🙂😀😐", 8, 8);
                    REQUIRE(a == "😐🙂😀🙂😀😐");
                }
            }
        }

        SECTION("Char") {
            SECTION("Replace code unit indexes") {
                string a = "abcdefghij";

                SECTION("Smaller") {
                    a.replace(3, 2, 1, 'x');
                    REQUIRE(a == "abcxfghij");
                }

                SECTION("Same size") {
                    a.replace(3, 2, 2, 'x');
                    REQUIRE(a == "abcxxfghij");
                }

                SECTION("Larger") {
                    a.replace(3, 2, 3, 'x');
                    REQUIRE(a == "abcxxxfghij");
                }

                SECTION("Wide char") {
                    a.replace(3, 2, 1, U'😀');
                    REQUIRE(a == "abc😀fghij");
                }

                SECTION("Wide char twice") {
                    a.replace(3, 2, 2, U'😀');
                    REQUIRE(a == "abc😀😀fghij");
                }

                SECTION("Wide char 3x") {
                    a.replace(3, 2, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀fghij");
                }
            }

            SECTION("Replace code point indexes") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Smaller") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), 1, 'x');
                    REQUIRE(a == "😐🙂x😀😐");
                }

                SECTION("Same size") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), 2, 'x');
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Larger") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), 3, 'x');
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Wide char") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), 1, U'😀');
                    REQUIRE(a == "😐🙂😀😀😐");
                }

                SECTION("Wide char twice") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), 2, U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }

                SECTION("Wide char 3x") {
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), 3, U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😀😐");
                }
            }

            SECTION("Replace code unit iterators") {
                string a = "abcdefghij";

                SECTION("Smaller") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 1, 'x');
                    REQUIRE(a == "abcxfghij");
                }

                SECTION("Same size") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 2, 'x');
                    REQUIRE(a == "abcxxfghij");
                }

                SECTION("Larger") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 3, 'x');
                    REQUIRE(a == "abcxxxfghij");
                }

                SECTION("Wide char") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 1, U'😀');
                    REQUIRE(a == "abc😀fghij");
                }

                SECTION("Wide char twice") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 2, U'😀');
                    REQUIRE(a == "abc😀😀fghij");
                }

                SECTION("Wide char 3x") {
                    a.replace(a.begin() + 3, a.begin() + 3 + 2, 3, U'😀');
                    REQUIRE(a == "abc😀😀😀fghij");
                }
            }

            SECTION("Replace code point iterators") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Smaller") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 2 + 2, 1, 'x');
                    REQUIRE(a == "😐🙂x😀😐");
                }

                SECTION("Same size") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 2 + 2, 2, 'x');
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Larger") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 2 + 2, 3, 'x');
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Wide char") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 2 + 2, 1, U'😀');
                    REQUIRE(a == "😐🙂😀😀😐");
                }

                SECTION("Wide char twice") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 2 + 2, 2, U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😐");
                }

                SECTION("Wide char 3x") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 2 + 2, 3, U'😀');
                    REQUIRE(a == "😐🙂😀😀😀😀😐");
                }
            }
        }

        SECTION("Initializer list") {
            SECTION("Replace code unit iterators") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    a.replace(a.begin(), a.begin() + 3, {'x', 'x', 'x'});
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin() + 3, a.begin() + 6, {'x', 'x', 'x'});
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    a.replace(a.begin() + 7, a.begin() + 10, {'x', 'x', 'x'});
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code point iterators") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    a.replace(a.begin_codepoint(), a.begin_codepoint() + 3, {'x', 'x', 'x'});
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, {'x', 'x', 'x'});
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    a.replace(a.begin_codepoint() + 4, a.begin_codepoint() + 6, {'x', 'x', 'x'});
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }
        }

        SECTION("String view") {
            SECTION("Replace code unit indexes") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(0, 3, b);
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(3, 3, b);
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(7, 3, b);
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code point indexes") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(string::codepoint_index(0), string::codepoint_index(3), b);
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), b);
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(string::codepoint_index(4), string::codepoint_index(2), b);
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code unit iterators") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(a.begin(), a.begin() + 3, b);
                    REQUIRE(a == "xxxdefghij");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(a.begin() + 3, a.begin() + 6, b);
                    REQUIRE(a == "abcxxxghij");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(a.begin() + 7, a.begin() + 10, b);
                    REQUIRE(a == "abcdefgxxx");
                }
            }

            SECTION("Replace code point iterators") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(a.begin_codepoint(), a.begin_codepoint() + 3, b);
                    REQUIRE(a == "xxx🙂😀😐");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(a.begin_codepoint() + 2, a.begin_codepoint() + 4, b);
                    REQUIRE(a == "😐🙂xxx😀😐");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(a.begin_codepoint() + 4, a.begin_codepoint() + 6, b);
                    REQUIRE(a == "😐🙂😀🙂xxx");
                }
            }

            SECTION("Replace code unit indexes with substr") {
                string a = "abcdefghij";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(0, 3, b, 1);
                    REQUIRE(a == "xxdefghij");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(3, 3, b, 1);
                    REQUIRE(a == "abcxxghij");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(7, 3, b, 1);
                    REQUIRE(a == "abcdefgxx");
                }
            }

            SECTION("Replace code point indexes with substr") {
                string a = "😐🙂😀🙂😀😐";

                SECTION("Replace start") {
                    std::string_view b = "xxx";
                    a.replace(string::codepoint_index(0), string::codepoint_index(3), b, 1);
                    REQUIRE(a == "xx🙂😀😐");
                }

                SECTION("Replace middle") {
                    std::string_view b = "xxx";
                    a.replace(string::codepoint_index(2), string::codepoint_index(2), b, 1);
                    REQUIRE(a == "😐🙂xx😀😐");
                }

                SECTION("Replace end") {
                    std::string_view b = "xxx";
                    a.replace(string::codepoint_index(4), string::codepoint_index(2), b, 1);
                    REQUIRE(a == "😐🙂😀🙂xx");
                }
            }
        }
    }

    SECTION("Substr") {
        SECTION("Code unit") {
            string a = "abcdefghij";

            SECTION("Start") {
                string b = a.substr(0, 3);
                REQUIRE(b == "abc");
            }

            SECTION("Middle") {
                string b = a.substr(3, 3);
                REQUIRE(b == "def");
            }

            SECTION("End") {
                string b = a.substr(6, 4);
                REQUIRE(b == "ghij");
            }
        }

        SECTION("Code point") {
            string a = "😐🙂😀🙂😀😐";
            SECTION("Start") {
                string b = a.substr(string::codepoint_index(0), string::codepoint_index(2));
                REQUIRE(b == "😐🙂");
            }

            SECTION("Middle") {
                string b = a.substr(string::codepoint_index(2), string::codepoint_index(2));
                REQUIRE(b == "😀🙂");
            }

            SECTION("End") {
                string b = a.substr(string::codepoint_index(4), string::codepoint_index(2));
                REQUIRE(b == "😀😐");
            }
        }
    }

    SECTION("Copy") {
        SECTION("To UTF8") {
            SECTION("Copy count") {
                string a("abcdefghij");
                char b[7]{};
                a.copy(b, sizeof b);
                REQUIRE(std::string_view(b, 7) == "abcdefg");
            }

            SECTION("From pos") {
                string a("abcdefghij");
                char b[7]{};
                a.copy(b, sizeof b, 3);
                REQUIRE(std::string_view(b, 7) == "defghij");
            }
        }

        SECTION("To UTF32") {
            SECTION("Copy count") {
                string a("😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
                utf32_char_type b[7]{};
                a.copy(b, string::codepoint_index(7));
                REQUIRE(std::u32string_view(b, 7) == U"😐🙂😀🙂😀😐😐");
            }

            SECTION("From pos") {
                string a("😐🙂😀🙂😀😐😐🙂😀🙂😀😐");
                utf32_char_type b[7]{};
                a.copy(b, string::codepoint_index(7), string::codepoint_index(3));
                REQUIRE(std::u32string_view(b, 7) == U"🙂😀😐😐🙂😀🙂");
            }
        }
    }

    SECTION("Swap") {
        string a = "abc";
        string b = "def";
        SECTION("Member") {
            a.swap(b);
            REQUIRE(b == "abc");
            REQUIRE(a == "def");
        }

        SECTION("Non-member") {
            swap(a, b);
            REQUIRE(b == "abc");
            REQUIRE(a == "def");
        }

        SECTION("Std swap") {
            std::swap(a, b);
            REQUIRE(b == "abc");
            REQUIRE(a == "def");
        }
    }

    SECTION("Search") {
        SECTION("Codeunit / Codepoint convert") {
            string a = "😐a😀b😀c😐d😀e😀f";
            auto cu_it = a.find_codeunit(string::codepoint_index(5));
            REQUIRE(*cu_it == 'c');
            auto cp_it = a.find_codepoint(10);
            REQUIRE(*cp_it == U'😀');
        }

        SECTION("Find first substring") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "def";
                    REQUIRE(a.find(b) == 3);
                    REQUIRE(a.find(b, 3) == 3);
                    REQUIRE(a.find(b, 4) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find("def") == 3);
                    REQUIRE(a.find("def", 3) == 3);
                    REQUIRE(a.find("def", 4) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find("defzxc", 0, 3) == 3);
                    REQUIRE(a.find("defzxc", 3, 3) == 3);
                    REQUIRE(a.find("defzxc", 4, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find('d', 0) == 3);
                    REQUIRE(a.find('d', 3) == 3);
                    REQUIRE(a.find('d', 4) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find(std::string_view("def"), 0) == 3);
                    REQUIRE(a.find(std::string_view("def"), 3) == 3);
                    REQUIRE(a.find(std::string_view("def"), 4) == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    string b = "😀c😐";
                    REQUIRE(a.find(b) == 10);
                    REQUIRE(a.find(b, 10) == 10);
                    REQUIRE(a.find(b, 14) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find(U"😀c😐") == 10);
                    REQUIRE(a.find(U"😀c😐", 10) == 10);
                    REQUIRE(a.find(U"😀c😐", 14) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find(U"😀c😐zxc", 0, 3) == 10);
                    REQUIRE(a.find(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.find(U"😀c😐zxc", 14, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find(U'😀', 0) == 5);
                    REQUIRE(a.find(U'😀', 10) == 10);
                    REQUIRE(a.find(U'😐', 19) == string::npos); // <- idx 19 / value d / cp_idx 7
                }

                SECTION("String view") {
                    REQUIRE(a.find(std::u32string_view(U"😀c😐"), 0) == 10);
                    REQUIRE(a.find(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(a.find(std::u32string_view(U"😀c😐"), 14) == string::npos);
                }
            }
        }

        SECTION("Find last substring") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "def";
                    REQUIRE(a.rfind(b) == 3);
                    REQUIRE(a.rfind(b, 3) == 3);
                    REQUIRE(a.rfind(b, 4) == 3);
                    REQUIRE(a.rfind(b, 2) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.rfind("def") == 3);
                    REQUIRE(a.rfind("def", 3) == 3);
                    REQUIRE(a.rfind("def", 4) == 3);
                    REQUIRE(a.rfind("def", 2) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.rfind("defzxc", 0, 3) == string::npos);
                    REQUIRE(a.rfind("defzxc", 3, 3) == 3);
                    REQUIRE(a.rfind("defzxc", 4, 3) == 3);
                    REQUIRE(a.rfind("defzxc", 2, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.rfind('d', 0) == string::npos);
                    REQUIRE(a.rfind('d', 3) == 3);
                    REQUIRE(a.rfind('d', 4) == 3);
                    REQUIRE(a.rfind('d', 2) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.rfind(std::string_view("def"), 0) == string::npos);
                    REQUIRE(a.rfind(std::string_view("def"), 3) == 3);
                    REQUIRE(a.rfind(std::string_view("def"), 4) == 3);
                    REQUIRE(a.rfind(std::string_view("def"), 2) == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    string b = "😀c😐";
                    REQUIRE(a.rfind(b) == 10);
                    REQUIRE(a.rfind(b, 10) == 10);
                    REQUIRE(a.rfind(b, 9) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.rfind(U"😀c😐") == 10);
                    REQUIRE(a.rfind(U"😀c😐", 10) == 10);
                    REQUIRE(a.rfind(U"😀c😐", 9) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.rfind(U"😀c😐zxc", 14, 3) == 10);
                    REQUIRE(a.rfind(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.rfind(U"😀c😐zxc", 9, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.rfind(U'😀', 19) == 10);
                    REQUIRE(a.rfind(U'😀', 10) == 10);
                    REQUIRE(a.rfind(U'😀', 9) == 5);
                    REQUIRE(a.rfind(U'😀', 0) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.rfind(std::u32string_view(U"😀c😐"), 14) == 10);
                    REQUIRE(a.rfind(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(a.rfind(std::u32string_view(U"😀c😐"), 0) == string::npos);
                }
            }
        }

        SECTION("Find first of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_first_of(b) == 3);
                    REQUIRE(a.find_first_of(b, 3) == 3);
                    REQUIRE(a.find_first_of(b, 6) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_of("fed") == 3);
                    REQUIRE(a.find_first_of("fed", 3) == 3);
                    REQUIRE(a.find_first_of("fed", 6) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_of("fedzxc", 0, 3) == 3);
                    REQUIRE(a.find_first_of("fedzxc", 3, 3) == 3);
                    REQUIRE(a.find_first_of("fedzxc", 6, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_of('e', 0) == 4);
                    REQUIRE(a.find_first_of('e', 3) == 4);
                    REQUIRE(a.find_first_of('e', 6) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find_first_of(std::string_view("fed"), 0) == 3);
                    REQUIRE(a.find_first_of(std::string_view("fed"), 3) == 3);
                    REQUIRE(a.find_first_of(std::string_view("fed"), 6) == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_first_of(b) == 0);
                    REQUIRE(a.find_first_of(b, 10) == 10);
                    REQUIRE(a.find_first_of(b, 26) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_of(U"😀c😐") == 0);
                    REQUIRE(a.find_first_of(U"😀c😐", 10) == 10);
                    REQUIRE(a.find_first_of(U"😀c😐", 26) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_of(U"😀c😐zxc", 0, 3) == 0);
                    REQUIRE(a.find_first_of(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.find_first_of(U"😀c😐zxc", 26, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_of(U'😀', 0) == 5);
                    REQUIRE(a.find_first_of(U'😀', 10) == 10);
                    REQUIRE(a.find_first_of(U'😐', 26) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find_first_of(std::u32string_view(U"😀c😐"), 0) == 0);
                    REQUIRE(a.find_first_of(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(a.find_first_of(std::u32string_view(U"😀c😐"), 26) == string::npos);
                }
            }
        }

        SECTION("Find first not of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_first_not_of(b) == 0);
                    REQUIRE(a.find_first_not_of(b, 3) == 6);
                    REQUIRE(a.find_first_not_of(b, 11) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_not_of("fed") == 0);
                    REQUIRE(a.find_first_not_of("fed", 3) == 6);
                    REQUIRE(a.find_first_not_of("fed", 11) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_not_of("fedzxc", 0, 3) == 0);
                    REQUIRE(a.find_first_not_of("fedzxc", 3, 3) == 6);
                    REQUIRE(a.find_first_not_of("fedzxc", 11, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_not_of('e', 0) == 0);
                    REQUIRE(a.find_first_not_of('e', 3) == 3);
                    REQUIRE(a.find_first_not_of('e', 4) == 5);
                    REQUIRE(a.find_first_not_of('e', 11) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find_first_not_of(std::string_view("fed"), 0) == 0);
                    REQUIRE(a.find_first_not_of(std::string_view("fed"), 3) == 6);
                    REQUIRE(a.find_first_not_of(std::string_view("fed"), 11) == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_first_not_of(b) == 4);
                    REQUIRE(a.find_first_not_of(b, 10) == 19);
                    REQUIRE(a.find_first_not_of(b, 31) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_first_not_of(U"😀c😐") == 4);
                    REQUIRE(a.find_first_not_of(U"😀c😐", 10) == 19);
                    REQUIRE(a.find_first_not_of(U"😀c😐", 31) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_first_not_of(U"😀c😐zxc", 0, 3) == 4);
                    REQUIRE(a.find_first_not_of(U"😀c😐zxc", 10, 3) == 19);
                    REQUIRE(a.find_first_not_of(U"😀c😐zxc", 31, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_first_not_of(U'😀', 0) == 0);
                    REQUIRE(a.find_first_not_of(U'😀', 10) == 14);
                    REQUIRE(a.find_first_not_of(U'😐', 31) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find_first_not_of(std::u32string_view(U"😀c😐"), 0) == 4);
                    REQUIRE(a.find_first_not_of(std::u32string_view(U"😀c😐"), 10) == 19);
                    REQUIRE(a.find_first_not_of(std::u32string_view(U"😀c😐"), 31) == string::npos);
                }
            }
        }

        SECTION("Find last of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_last_of(b) == 5);
                    REQUIRE(a.find_last_of(b, 3) == 3);
                    REQUIRE(a.find_last_of(b, 2) == string::npos);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_of("fed") == 5);
                    REQUIRE(a.find_last_of("fed", 3) == 3);
                    REQUIRE(a.find_last_of("fed", 2) == string::npos);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_last_of("fedzxc", 6, 3) == 5);
                    REQUIRE(a.find_last_of("fedzxc", 3, 3) == 3);
                    REQUIRE(a.find_last_of("fedzxc", 2, 3) == string::npos);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_of('e', 6) == 4);
                    REQUIRE(a.find_last_of('e', 4) == 4);
                    REQUIRE(a.find_last_of('e', 3) == string::npos);
                }

                SECTION("String view") {
                    REQUIRE(a.find_last_of(std::string_view("fed"), 6) == 5);
                    REQUIRE(a.find_last_of(std::string_view("fed"), 3) == 3);
                    REQUIRE(a.find_last_of(std::string_view("fed"), 2) == string::npos);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_last_of(b) == 25);
                    REQUIRE(a.find_last_of(b, 10) == 10);
                    REQUIRE(a.find_last_of(b, 0) == 0);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_of(U"😀c😐") == 25);
                    REQUIRE(a.find_last_of(U"😀c😐", 10) == 10);
                    REQUIRE(a.find_last_of(U"😀c😐", 0) == 0);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_last_of(U"😀c😐zxc", 0, 3) == 0);
                    REQUIRE(a.find_last_of(U"😀c😐zxc", 10, 3) == 10);
                    REQUIRE(a.find_last_of(U"😀c😐zxc", 26, 3) == 25);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_of(U'😀', 0) == string::npos);
                    REQUIRE(a.find_last_of(U'😀', 10) == 10);
                    REQUIRE(a.find_last_of(U'😐', 26) == 15);
                }

                SECTION("String view") {
                    REQUIRE(a.find_last_of(std::u32string_view(U"😀c😐"), 26) == 25);
                    REQUIRE(a.find_last_of(std::u32string_view(U"😀c😐"), 10) == 10);
                    REQUIRE(a.find_last_of(std::u32string_view(U"😀c😐"), 0) == 0);
                }
            }
        }

        SECTION("Find last not of chars") {
            SECTION("Same encoding") {
                string a = "abcdefghij";

                SECTION("Other string") {
                    string b = "fed";
                    REQUIRE(a.find_last_not_of(b) == 9);
                    REQUIRE(a.find_last_not_of(b, 3) == 2);
                    REQUIRE(a.find_last_not_of(b, 11) == 9);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_not_of("fed") == 9);
                    REQUIRE(a.find_last_not_of("fed", 3) == 2);
                    REQUIRE(a.find_last_not_of("fed", 11) == 9);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_last_not_of("fedzxc", 0, 3) == 0);
                    REQUIRE(a.find_last_not_of("fedzxc", 3, 3) == 2);
                    REQUIRE(a.find_last_not_of("fedzxc", 11, 3) == 9);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_not_of('e', 0) == 0);
                    REQUIRE(a.find_last_not_of('e', 3) == 3);
                    REQUIRE(a.find_last_not_of('e', 4) == 3);
                    REQUIRE(a.find_last_not_of('e', 11) == 9);
                }

                SECTION("String view") {
                    REQUIRE(a.find_last_not_of(std::string_view("fed"), 0) == 0);
                    REQUIRE(a.find_last_not_of(std::string_view("fed"), 3) == 2);
                    REQUIRE(a.find_last_not_of(std::string_view("fed"), 11) == 9);
                }
            }

            SECTION("Different encoding") {
                string a = "😐a😀b😀c😐d😀e😀f";

                SECTION("Other string") {
                    std::u32string b = U"😀c😐";
                    REQUIRE(a.find_last_not_of(b) == 29);
                    REQUIRE(a.find_last_not_of(b, 10) == 9);
                    REQUIRE(a.find_last_not_of(b, 31) == 29);
                }

                SECTION("Literal string") {
                    REQUIRE(a.find_last_not_of(U"😀c😐") == 29);
                    REQUIRE(a.find_last_not_of(U"😀c😐", 10) == 9);
                    REQUIRE(a.find_last_not_of(U"😀c😐", 31) == 29);
                }

                SECTION("Literal substring") {
                    REQUIRE(a.find_last_not_of(U"😀c😐zxc", 0, 3) == string::npos);
                    REQUIRE(a.find_last_not_of(U"😀c😐zxc", 10, 3) == 9);
                    REQUIRE(a.find_last_not_of(U"😀c😐zxc", 31, 3) == 29);
                }

                SECTION("Char") {
                    REQUIRE(a.find_last_not_of(U'😀', 0) == 0);
                    REQUIRE(a.find_last_not_of(U'😀', 10) == 9);
                    REQUIRE(a.find_last_not_of(U'😐', 31) == 29);
                }

                SECTION("String view") {
                    REQUIRE(a.find_last_not_of(std::u32string_view(U"😀c😐"), 0) == string::npos);
                    REQUIRE(a.find_last_not_of(std::u32string_view(U"😀c😐"), 10) == 9);
                    REQUIRE(a.find_last_not_of(std::u32string_view(U"😀c😐"), 31) == 29);
                }
            }
        }
    }

    SECTION("Non-member") {
        SECTION("Concatenate strings") {
            SECTION("const string & + const string &") {
                string lhs = "abc";
                string rhs = "def";
                string c = lhs + rhs;
                REQUIRE(c == "abcdef");
            }
            SECTION("const string & + const char *") {
                string lhs = "abc";
                string c = lhs + "def";
                REQUIRE(c == "abcdef");
            }
            SECTION("const string & + char ") {
                string lhs = "abc";
                char rhs = 'd';
                string c = lhs + rhs;
                REQUIRE(c == "abcd");
            }
            SECTION("const char * + const string &") {
                string rhs = "def";
                string c = "abc" + rhs;
                REQUIRE(c == "abcdef");
            }
            SECTION("char  + const string &") {
                char lhs = 'a';
                string rhs = "def";
                string c = lhs + rhs;
                REQUIRE(c == "adef");
            }
            SECTION("string && + string &&") {
                string lhs = "abc";
                string rhs = "def";
                string c = std::move(lhs) + std::move(rhs);
                REQUIRE(c == "abcdef");
            }
            SECTION("string && + const string &") {
                string lhs = "abc";
                string rhs = "def";
                string c = std::move(lhs) + rhs;
                REQUIRE(c == "abcdef");
            }
            SECTION("string && + const char *") {
                string lhs = "abc";
                string c = std::move(lhs) + "def";
                REQUIRE(c == "abcdef");
            }
            SECTION("string && + char ") {
                string lhs = "abc";
                char rhs = 'd';
                string c = std::move(lhs) + rhs;
                REQUIRE(c == "abcd");
            }
            SECTION("const string & + string &&") {
                string lhs = "abc";
                string rhs = "def";
                string c = lhs + std::move(rhs);
                REQUIRE(c == "abcdef");
            }
            SECTION("const char * + string &&") {
                string rhs = "def";
                string c = "abc" + std::move(rhs);
                REQUIRE(c == "abcdef");
            }
            SECTION("char  + string &&") {
                char lhs = 'a';
                string rhs = "def";
                string c = lhs + std::move(rhs);
                REQUIRE(c == "adef");
            }
        }

        SECTION("Erase") {
            string cnt(10, ' ');
            std::iota(cnt.begin(), cnt.end(), '0');

            SECTION("Values") {
                erase(cnt, '3');
                REQUIRE(cnt == "012456789");
            }

            SECTION("Condition") {
                size_t n_erased = erase_if(cnt, [](char x) { return (x - '0') % 2 == 0; });
                REQUIRE(cnt == "13579");
                REQUIRE(n_erased == 5);
            }
        }

        SECTION("Streams") {
            small::string a = "123456";

            SECTION("Output") {
                std::stringstream ss;
                ss << a;
                REQUIRE(ss.str() == "123456");
            }

            SECTION("Input") {
                std::stringstream ss;
                ss << "123";
                ss >> a;
                REQUIRE(a == "123");
            }

            SECTION("Getline") {
                std::stringstream ss;
                ss << "123 456\n789\n";
                getline(ss, a);
                REQUIRE(a == "123 456");
            }
        }

        SECTION("String to number") {
            SECTION("Integer") {
                small::string i = "123";
                std::unique_ptr<size_t> size = std::make_unique<size_t>(0);
                SECTION("stoi") {
                    int n = stoi(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stol") {
                    long n = stol(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stoll") {
                    long long n = stoll(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stoul") {
                    unsigned long n = stoul(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
                SECTION("stoull") {
                    unsigned long long n = stoull(i, size.get(), 10);
                    REQUIRE(n == 123);
                    REQUIRE(*size == 3);
                }
            }

            SECTION("Floating") {
                small::string d = "123.456";
                std::unique_ptr<size_t> size = std::make_unique<size_t>(0);
                SECTION("stof") {
                    float n = stof(d, size.get());
                    REQUIRE(n >= 123.455);
                    REQUIRE(n <= 123.457);
                    REQUIRE(*size == 7);
                }
                SECTION("stod") {
                    double n = stod(d, size.get());
                    REQUIRE(n >= 123.455);
                    REQUIRE(n <= 123.457);
                    REQUIRE(*size == 7);
                }
                SECTION("stold") {
                    long double n = stold(d, size.get());
                    REQUIRE(n >= 123.455);
                    REQUIRE(n <= 123.457);
                    REQUIRE(*size == 7);
                }
            }
        }

        SECTION("Number to string") {
            REQUIRE(small::to_string(static_cast<int>(123)) == "123");
            REQUIRE(small::to_string(static_cast<long>(123)) == "123");
            REQUIRE(small::to_string(static_cast<long long>(123)) == "123");
            REQUIRE(small::to_string(static_cast<unsigned>(123)) == "123");
            REQUIRE(small::to_string(static_cast<unsigned long>(123)) == "123");
            REQUIRE(small::to_string(static_cast<unsigned long long>(123)) == "123");
            REQUIRE(small::to_string(static_cast<float>(123)) == "123");
            REQUIRE(small::to_string(static_cast<double>(123)) == "123");
            REQUIRE(small::to_string(static_cast<long double>(123)) == "123");
        }

        SECTION("Hash") {
            SECTION("Isolated") {
                std::hash<small::string> hasher;
                string a = "abc";
                REQUIRE_FALSE(hasher(a) == 0);
            }

            SECTION("Hash table") {
                std::unordered_set<small::string> s;
                s.insert("abc");
                s.insert("def");
                s.insert("ghi");
                REQUIRE(s.size() == 3);
            }
        }

        SECTION("Relocatable in inline vector") {
            small::vector<small::string> v(5);
            v.emplace_back("new str");
            v.emplace(v.begin() + 3, "middle str");
            REQUIRE(v.size() == 7);
        }
    }
}