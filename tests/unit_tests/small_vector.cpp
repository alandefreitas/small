#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <string_view>

#include <catch2/catch.hpp>

#include <small/map.h>
#include <small/string.h>
#include <small/vector.h>

// A relocatable custom type
struct custom_type {
    enum class custom_enum { even, odd };
    custom_type() {}
    custom_type(const std::string &v)
        : type_(v.size() & 1 ? custom_enum::even : custom_enum::odd), name_(v), url_("https://" + v),
          version_(v.size() < 4 ? std::optional<int>(std::nullopt) : std::optional<int>(v.size())), tag_(v.substr(2)),
          system_(v.substr(0, 2)), raw_(v) {}
    custom_type(const char *v) : custom_type(std::string(v)) {}
    custom_enum type_;
    std::string name_;
    std::string url_;
    std::optional<int> version_;
    std::optional<std::string> tag_;
    std::optional<std::string> system_;
    std::optional<std::string> raw_;
};

// Allow comparing with std::string to make tests easier
bool operator==(const custom_type &lhs, const custom_type &rhs) { return lhs.raw_ == rhs.raw_; }
bool operator!=(const custom_type &lhs, const custom_type &rhs) { return !(rhs == lhs); }

bool operator==(const custom_type &lhs, const std::string &rhs) { return lhs.raw_ == rhs; }
bool operator!=(const custom_type &lhs, const std::string &rhs) { return !(lhs == rhs); }
bool operator==(const std::string &lhs, const custom_type &rhs) { return lhs == rhs.raw_; }
bool operator!=(const std::string &lhs, const custom_type &rhs) { return !(lhs == rhs); }

bool operator==(const custom_type &lhs, const char *rhs) { return lhs.raw_ == std::string(rhs); }
bool operator!=(const custom_type &lhs, const char *rhs) { return !(lhs == rhs); }
bool operator==(const char *lhs, const custom_type &rhs) { return std::string(lhs) == rhs.raw_; }
bool operator!=(const char *lhs, const custom_type &rhs) { return !(lhs == rhs); }

bool operator<(const custom_type &lhs, const custom_type &rhs) { return lhs.raw_ < rhs.raw_; }
bool operator>(const custom_type &lhs, const custom_type &rhs) { return rhs < lhs; }
bool operator<=(const custom_type &lhs, const custom_type &rhs) { return !(rhs < lhs); }
bool operator>=(const custom_type &lhs, const custom_type &rhs) { return !(lhs < rhs); }

bool operator<(const custom_type &lhs, const std::string &rhs) { return lhs.raw_ < rhs; }
bool operator>(const custom_type &lhs, const std::string &rhs) { return rhs < lhs; }
bool operator<=(const custom_type &lhs, const std::string &rhs) { return !(rhs < lhs); }
bool operator>=(const custom_type &lhs, const std::string &rhs) { return !(lhs < rhs); }

namespace small {
    // The custom type has no internal pointers so we can relocate it faster
    // Most types might be relocatable, but we have to conservatively assume they are not
    template <> struct is_relocatable<custom_type> : std::true_type {};

    // Make strings relocatable since we are here anyway
    template <> struct is_relocatable<std::string> : std::true_type {};

    // Vectors of custom type should have 10 inlined values by default for some reason,
    // so we don't need to specify this default value for every small vector
    template <> struct default_inline_storage<custom_type> : std::integral_constant<size_t, 10> {};
} // namespace small

TEST_CASE("Vector") {
    using namespace small;

    SECTION("POD values") {
        STATIC_REQUIRE(is_relocatable_v<int>);
        auto equal_il = [](const auto &sm_vector, std::initializer_list<int> il) -> bool {
            return std::equal(sm_vector.begin(), sm_vector.end(), il.begin(), il.end());
        };

        SECTION("Constructor") {
            SECTION("Asserts") {
                REQUIRE(std::is_copy_constructible_v<vector<int, 5>>);
                REQUIRE(std::is_copy_assignable_v<vector<int, 5>>);
                REQUIRE(std::is_move_constructible_v<vector<int, 5>>);
                REQUIRE(std::is_move_assignable_v<vector<int, 5>>);

                REQUIRE(std::is_copy_constructible_v<vector<std::pair<int, int>, 5>>);
                REQUIRE(std::is_copy_assignable_v<vector<std::pair<int, int>, 5>>);
                REQUIRE(std::is_move_constructible_v<vector<std::pair<int, int>, 5>>);
                REQUIRE(std::is_move_assignable_v<vector<std::pair<int, int>, 5>>);
            }

            SECTION("Default") {
                vector<int, 5> a;
                REQUIRE(a.empty());
                REQUIRE(equal_il(a, {}));
            }

            SECTION("Allocator") {
                std::allocator<int> alloc;
                vector<int, 5, std::allocator<int>> a(alloc);
                REQUIRE(a.empty());
                REQUIRE(equal_il(a, {}));
                REQUIRE(a.get_allocator() == alloc);
            }

            SECTION("With size") {
                std::allocator<int> alloc;
                vector<int, 5> b(3, alloc);
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(b.get_allocator() == alloc);
            }

            SECTION("From value") {
                std::allocator<int> alloc;
                vector<int, 5> c(3, 1, alloc);
                REQUIRE(c.size() == 3);
                REQUIRE_FALSE(c.empty());
                REQUIRE(equal_il(c, {1, 1, 1}));
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From Iterators") {
                std::allocator<int> alloc;
                std::vector<int> dv = {6, 5, 4};
                vector<int, 5> d(dv.begin(), dv.end(), alloc);
                REQUIRE(d.size() == 3);
                REQUIRE_FALSE(d.empty());
                REQUIRE(equal_il(d, {6, 5, 4}));
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("From initializer list") {
                vector<int, 5> e = {1, 2};
                REQUIRE(e.size() == 2);
                REQUIRE_FALSE(e.empty());
                REQUIRE(equal_il(e, {1, 2}));
            }

            SECTION("From ranges") {
                std::vector<int> v = {1, 2, 3};
                vector<int, 5> e(v);
                REQUIRE(e.size() == 3);
                REQUIRE_FALSE(e.empty());
                REQUIRE(equal_il(e, {1, 2, 3}));
            }
        }

        SECTION("Assign") {
            SECTION("From initializer list") {
                vector<int, 5> a;
                REQUIRE(a.empty());
                a = {6, 5, 4};
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {6, 5, 4}));
            }

            SECTION("From another small vector") {
                vector<int, 5> a;
                REQUIRE(a.empty());
                a = {6, 5, 4};

                vector<int, 5> b;
                REQUIRE(b.empty());
                b = a;
                REQUIRE(b.size() == 3);
                REQUIRE_FALSE(b.empty());
                REQUIRE(a == b);
            }

            SECTION("From iterators") {
                vector<int, 5> a;
                REQUIRE(a.empty());
                std::vector<int> v = {6, 5, 4};
                a.assign(v.begin(), v.end());
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {6, 5, 4}));
            }

            SECTION("From size and value") {
                vector<int, 5> a;
                REQUIRE(a.empty());
                a.assign(3, 1);
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {1, 1, 1}));
            }

            SECTION("From initializer list") {
                vector<int, 5> a;
                REQUIRE(a.empty());
                a.assign({6, 5, 4});
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {6, 5, 4}));
            }

            SECTION("Fill") {
                vector<int, 5> a(3, 1);
                REQUIRE_FALSE(a.empty());
                a.fill(2);
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {2, 2, 2}));
            }

            SECTION("Swap") {
                vector<int, 5> a(4, 1);
                vector<int, 5> b(3, 2);

                std::initializer_list<int> ar = {1, 1, 1, 1};
                std::initializer_list<int> br = {2, 2, 2};

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 4);
                REQUIRE(equal_il(a, ar));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(equal_il(b, br));

                a.swap(b);

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 3);
                REQUIRE(equal_il(a, br));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 4);
                REQUIRE(equal_il(b, ar));

                std::swap(a, b);

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 4);
                REQUIRE(equal_il(a, ar));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(equal_il(b, br));
            }
        }

        SECTION("Iterators") {
            vector<int, 5> a = {1, 2, 3};

            REQUIRE(a.begin() == a.data());
            REQUIRE(a.end() == a.data() + a.size());
            REQUIRE_FALSE(a.end() == a.data() + a.capacity());
            REQUIRE(*a.begin() == 1);
            REQUIRE(*std::prev(a.end()) == 3);

            REQUIRE(a.cbegin() == a.data());
            REQUIRE(a.cend() == a.data() + a.size());
            REQUIRE_FALSE(a.cend() == a.data() + a.capacity());
            REQUIRE(*a.cbegin() == 1);
            REQUIRE(*std::prev(a.cend()) == 3);

            REQUIRE(*a.rbegin() == 3);
            REQUIRE(*std::prev(a.rend()) == 1);

            REQUIRE(*a.crbegin() == 3);
            REQUIRE(*std::prev(a.crend()) == 1);
        }

        SECTION("Capacity") {
            vector<int, 5> a = {1, 2, 3};
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.reserve(10);
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() >= 10);

            a.shrink_to_fit();
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.resize(4);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.shrink_to_fit();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);
        }

        SECTION("Element access") {
            vector<int, 5> a = {1, 2, 3};
            REQUIRE(a[0] == 1);
            REQUIRE(a[1] == 2);
            REQUIRE(a[2] == 3);
            REQUIRE(a.at(0) == 1);
            REQUIRE(a.at(1) == 2);
            REQUIRE(a.at(2) == 3);
            REQUIRE_THROWS(a.at(3) == 4);
            REQUIRE_THROWS(a.at(4) == 5);
            REQUIRE(a.front() == 1);
            REQUIRE(a.back() == 3);
            REQUIRE(*a.data() == 1);
            REQUIRE(*(a.data() + 1) == 2);
            REQUIRE(*(a.data() + 2) == 3);
            REQUIRE(*(a.data() + a.size() - 1) == 3);
            REQUIRE(*(a.data() + a.size() - 2) == 2);
            REQUIRE(*(a.data() + a.size() - 3) == 1);
        }

        SECTION("Modifiers") {
            vector<int, 5> a = {1, 2, 3};
            a.push_back(4);
            REQUIRE(a.back() == 4);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 3, 4}));

            // NOLINTNEXTLINE(performance-move-const-arg)
            a.push_back(std::move(5));
            REQUIRE(a.back() == 5);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 3, 4, 5}));

            a.pop_back();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 3, 4}));

            a.emplace_back(5);
            REQUIRE(a.back() == 5);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 3, 4, 5}));

            a.pop_back();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 3, 4}));

            auto it = a.emplace(a.begin() + 2, 10);
            REQUIRE(it == a.begin() + 2);
            REQUIRE(a.back() == 4);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 10, 3, 4}));

            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 10}));

            it = a.insert(a.begin() + 1, 20);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 20, 2, 10}));

            // NOLINTNEXTLINE(performance-move-const-arg)
            it = a.insert(a.begin() + 2, std::move(30));
            REQUIRE(it == a.begin() + 2);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 20, 30, 2, 10}));

            a.pop_back();
            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 20}));

            it = a.insert(a.begin() + 1, 2, 10);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 10, 10, 20}));

            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 10}));

            std::initializer_list<int> src = {2, 4, 8};
            it = a.insert(a.begin() + 1, src.begin(), src.end());
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 4, 8, 10}));

            a.pop_back();
            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2}));

            it = a.insert(a.begin() + 1, {2, 4, 8});
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 4, 8, 2}));

            it = a.erase(a.begin() + 1);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 4, 8, 2}));

            it = a.erase(a.begin() + 1, a.begin() + 3);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2}));

            a.clear();
            // NOLINTNEXTLINE(readability-container-size-empty)
            REQUIRE(a.size() == 0);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));

            a.resize(2);
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());

            a.resize(4, 5);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a[2] == 5);
            REQUIRE(a[3] == 5);
        }

        SECTION("Element access errors") {
            vector<int, 5> a = {1, 2, 3};
            try {
                a.at(4);
            } catch (std::exception &e) {
                REQUIRE(e.what() == std::string_view("at: cannot access element after vector::size()"));
            }
        }

        SECTION("Relational Operators") {
            vector<int, 5> a = {1, 2, 3};
            vector<int, 5> b = {2, 4, 5};

            REQUIRE_FALSE(a == b);
            REQUIRE(a != b);
            REQUIRE(a < b);
            REQUIRE(a <= b);
            REQUIRE_FALSE(a > b);
            REQUIRE_FALSE(a >= b);
        }

        SECTION("From raw vector") {
            auto a = to_vector({1, 2, 3});
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 3);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 2, 3}));

            auto b = to_vector<int, 3, 5>({1, 2, 3});
            REQUIRE(b.size() == 3);
            REQUIRE(b.max_size() > 5);
            REQUIRE(b.capacity() == 5);
            REQUIRE_FALSE(b.empty());
            REQUIRE(equal_il(b, {1, 2, 3}));

            int cr[] = {1, 2, 3};
            auto c = to_vector(cr);
            REQUIRE(c.size() == 3);
            REQUIRE(c.max_size() > 3);
            REQUIRE(c.is_inline());
            REQUIRE(c.capacity() == small::default_inline_storage_v<int>);
            REQUIRE(c.capacity() == 5);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(b, {1, 2, 3}));
        }
    }

    SECTION("String values") {
        STATIC_REQUIRE(is_relocatable_v<std::string>);
        auto equal_il = [](const auto &sm_vector, std::initializer_list<std::string> il) -> bool {
            return std::equal(sm_vector.begin(), sm_vector.end(), il.begin(), il.end());
        };

        SECTION("Constructor") {
            SECTION("Asserts") {
                REQUIRE(std::is_copy_constructible_v<vector<std::string, 5>>);
                REQUIRE(std::is_copy_assignable_v<vector<std::string, 5>>);
                REQUIRE(std::is_move_constructible_v<vector<std::string, 5>>);
                REQUIRE(std::is_move_assignable_v<vector<std::string, 5>>);

                REQUIRE(std::is_copy_constructible_v<vector<std::pair<std::string, std::string>, 5>>);
                REQUIRE(std::is_copy_assignable_v<vector<std::pair<std::string, std::string>, 5>>);
                REQUIRE(std::is_move_constructible_v<vector<std::pair<std::string, std::string>, 5>>);
                REQUIRE(std::is_move_assignable_v<vector<std::pair<std::string, std::string>, 5>>);
            }

            SECTION("Default") {
                vector<std::string, 5> a;
                REQUIRE(a.empty());
                REQUIRE(equal_il(a, {}));
            }

            SECTION("Allocator") {
                std::allocator<std::string> alloc;
                vector<std::string, 5, std::allocator<std::string>> a(alloc);
                REQUIRE(a.empty());
                REQUIRE(equal_il(a, {}));
                REQUIRE(a.get_allocator() == alloc);
            }

            SECTION("With size") {
                std::allocator<std::string> alloc;
                vector<std::string, 5> b(3, alloc);
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(b.get_allocator() == alloc);
            }

            SECTION("From value") {
                std::allocator<std::string> alloc;
                vector<std::string, 5> c(3, "one", alloc);
                REQUIRE(c.size() == 3);
                REQUIRE_FALSE(c.empty());
                REQUIRE(equal_il(c, {"one", "one", "one"}));
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From Iterators") {
                std::allocator<std::string> alloc;
                std::vector<std::string> dv = {"six", "five", "four"};
                vector<std::string, 5> d(dv.begin(), dv.end(), alloc);
                REQUIRE(d.size() == 3);
                REQUIRE_FALSE(d.empty());
                REQUIRE(equal_il(d, {"six", "five", "four"}));
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("From initializer list") {
                vector<std::string, 5> e = {"one", "two"};
                REQUIRE(e.size() == 2);
                REQUIRE_FALSE(e.empty());
                REQUIRE(equal_il(e, {"one", "two"}));
            }

            SECTION("From ranges") {
                std::vector<std::string> v = {"one", "two", "three"};
                vector<std::string, 5> e(v);
                REQUIRE(e.size() == 3);
                REQUIRE_FALSE(e.empty());
                REQUIRE(equal_il(e, {"one", "two", "three"}));
            }
        }

        SECTION("Assign") {
            SECTION("From initializer list") {
                vector<std::string, 5> a;
                REQUIRE(a.empty());
                a = {"six", "five", "four"};
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"six", "five", "four"}));
            }

            SECTION("From another small vector") {
                vector<std::string, 5> a;
                REQUIRE(a.empty());
                a = {"six", "five", "four"};

                vector<std::string, 5> b;
                REQUIRE(b.empty());
                b = a;
                REQUIRE(b.size() == 3);
                REQUIRE_FALSE(b.empty());
                REQUIRE(a == b);
            }

            SECTION("From iterators") {
                vector<std::string, 5> a;
                REQUIRE(a.empty());
                std::vector<std::string> v = {"six", "five", "four"};
                a.assign(v.begin(), v.end());
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"six", "five", "four"}));
            }

            SECTION("From size and value") {
                vector<std::string, 5> a;
                REQUIRE(a.empty());
                a.assign(3, "one");
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"one", "one", "one"}));
            }

            SECTION("From initializer list") {
                vector<std::string, 5> a;
                REQUIRE(a.empty());
                a.assign({"six", "five", "four"});
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"six", "five", "four"}));
            }

            SECTION("Fill") {
                vector<std::string, 5> a(3, "one");
                REQUIRE_FALSE(a.empty());
                a.fill("two");
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"two", "two", "two"}));
            }

            SECTION("Swap") {
                vector<std::string, 5> a(4, "one");
                vector<std::string, 5> b(3, "two");

                std::initializer_list<std::string> ar = {"one", "one", "one", "one"};
                std::initializer_list<std::string> br = {"two", "two", "two"};

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 4);
                REQUIRE(equal_il(a, ar));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(equal_il(b, br));

                a.swap(b);

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 3);
                REQUIRE(equal_il(a, br));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 4);
                REQUIRE(equal_il(b, ar));

                std::swap(a, b);

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 4);
                REQUIRE(equal_il(a, ar));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(equal_il(b, br));
            }
        }

        SECTION("Iterators") {
            vector<std::string, 5> a = {"one", "two", "three"};

            REQUIRE(a.begin() == a.data());
            REQUIRE(a.end() == a.data() + a.size());
            REQUIRE_FALSE(a.end() == a.data() + a.capacity());
            REQUIRE(*a.begin() == "one");
            REQUIRE(*std::prev(a.end()) == "three");

            REQUIRE(a.cbegin() == a.data());
            REQUIRE(a.cend() == a.data() + a.size());
            REQUIRE_FALSE(a.cend() == a.data() + a.capacity());
            REQUIRE(*a.cbegin() == "one");
            REQUIRE(*std::prev(a.cend()) == "three");

            REQUIRE(*a.rbegin() == "three");
            REQUIRE(*std::prev(a.rend()) == "one");

            REQUIRE(*a.crbegin() == "three");
            REQUIRE(*std::prev(a.crend()) == "one");
        }

        SECTION("Capacity") {
            vector<std::string, 5> a = {"one", "two", "three"};
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.reserve(10);
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() >= 10);

            a.shrink_to_fit();
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.resize(4);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.shrink_to_fit();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);
        }

        SECTION("Element access") {
            vector<std::string, 5> a = {"one", "two", "three"};
            REQUIRE(a[0] == "one");
            REQUIRE(a[1] == "two");
            REQUIRE(a[2] == "three");
            REQUIRE(a.at(0) == "one");
            REQUIRE(a.at(1) == "two");
            REQUIRE(a.at(2) == "three");
            REQUIRE_THROWS(a.at(3) == "four");
            REQUIRE_THROWS(a.at(4) == "five");
            REQUIRE(a.front() == "one");
            REQUIRE(a.back() == "three");
            REQUIRE(*a.data() == "one");
            REQUIRE(*(a.data() + 1) == "two");
            REQUIRE(*(a.data() + 2) == "three");
            REQUIRE(*(a.data() + a.size() - 1) == "three");
            REQUIRE(*(a.data() + a.size() - 2) == "two");
            REQUIRE(*(a.data() + a.size() - 3) == "one");
        }

        SECTION("Modifiers") {
            vector<std::string, 5> a = {"one", "two", "three"};
            a.push_back("four");
            REQUIRE(a.back() == "four");
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four"}));

            // NOLINTNEXTLINE(performance-move-const-arg)
            a.push_back(std::move("five"));
            REQUIRE(a.back() == "five");
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four", "five"}));

            a.pop_back();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four"}));

            a.emplace_back("five");
            REQUIRE(a.back() == "five");
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four", "five"}));

            a.pop_back();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four"}));

            auto it = a.emplace(a.begin() + 2, "ten");
            REQUIRE(it == a.begin() + 2);
            REQUIRE(a.back() == "four");
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "ten", "three", "four"}));

            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "ten"}));

            it = a.insert(a.begin() + 1, "twenty");
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "twenty", "two", "ten"}));

            // NOLINTNEXTLINE(performance-move-const-arg)
            it = a.insert(a.begin() + 2, std::move("thirty"));
            REQUIRE(it == a.begin() + 2);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "twenty", "thirty", "two", "ten"}));

            a.pop_back();
            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "twenty"}));

            it = a.insert(a.begin() + 1, 2, "ten");
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "ten", "ten", "twenty"}));

            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "ten"}));

            std::initializer_list<std::string> src = {"two", "four", "eight"};
            it = a.insert(a.begin() + 1, src.begin(), src.end());
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "four", "eight", "ten"}));

            a.pop_back();
            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two"}));

            it = a.insert(a.begin() + 1, {"two", "four", "eight"});
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "four", "eight", "two"}));

            it = a.erase(a.begin() + 1);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "four", "eight", "two"}));

            it = a.erase(a.begin() + 1, a.begin() + 3);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two"}));

            a.clear();
            // NOLINTNEXTLINE(readability-container-size-empty)
            REQUIRE(a.size() == 0);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));

            a.resize(2);
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());

            a.resize(4, "five");
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a[2] == "five");
            REQUIRE(a[3] == "five");
        }

        SECTION("Element access errors") {
            vector<std::string, 5> a = {"one", "two", "three"};
            try {
                a.at(4);
            } catch (std::exception &e) {
                REQUIRE(e.what() == std::string_view("at: cannot access element after vector::size()"));
            }
        }

        SECTION("Relational Operators") {
            vector<std::string, 5> a = {"one", "two", "three"};
            vector<std::string, 5> b = {"two", "four", "five"};

            REQUIRE_FALSE(a == b);
            REQUIRE(a != b);
            REQUIRE(a < b);
            REQUIRE(a <= b);
            REQUIRE_FALSE(a > b);
            REQUIRE_FALSE(a >= b);
        }

        SECTION("From raw vector") {
            auto a = to_vector({"one", "two", "three"});
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 3);
            REQUIRE(a.capacity() == default_inline_storage_v<std::string>);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three"}));

            auto b = to_vector<std::string, 3, 5>({"one", "two", "three"});
            REQUIRE(b.size() == 3);
            REQUIRE(b.max_size() > 5);
            REQUIRE(b.capacity() == 5);
            REQUIRE_FALSE(b.empty());
            REQUIRE(equal_il(b, {"one", "two", "three"}));

            std::string cr[] = {"one", "two", "three"};
            auto c = to_vector(cr);
            REQUIRE(c.size() == 3);
            REQUIRE(c.max_size() > 3);
            REQUIRE(c.capacity() == 5);
            REQUIRE(c.capacity() == default_inline_storage_v<std::string>);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(b, {"one", "two", "three"}));
        }
    }

    SECTION("Custom values") {
        STATIC_REQUIRE(is_relocatable_v<custom_type>);
        auto equal_il = [](const auto &sm_vector, std::initializer_list<custom_type> il) -> bool {
            return std::equal(sm_vector.begin(), sm_vector.end(), il.begin(), il.end());
        };

        SECTION("Constructor") {
            SECTION("Asserts") {
                REQUIRE(std::is_copy_constructible_v<vector<custom_type, 5>>);
                REQUIRE(std::is_copy_assignable_v<vector<custom_type, 5>>);
                REQUIRE(std::is_move_constructible_v<vector<custom_type, 5>>);
                REQUIRE(std::is_move_assignable_v<vector<custom_type, 5>>);

                REQUIRE(std::is_copy_constructible_v<vector<std::pair<custom_type, custom_type>, 5>>);
                REQUIRE(std::is_copy_assignable_v<vector<std::pair<custom_type, custom_type>, 5>>);
                REQUIRE(std::is_move_constructible_v<vector<std::pair<custom_type, custom_type>, 5>>);
                REQUIRE(std::is_move_assignable_v<vector<std::pair<custom_type, custom_type>, 5>>);
            }

            SECTION("Default") {
                vector<custom_type, 5> a;
                REQUIRE(a.empty());
                REQUIRE(equal_il(a, {}));
            }

            SECTION("Allocator") {
                std::allocator<custom_type> alloc;
                vector<custom_type, 5, std::allocator<custom_type>> a(alloc);
                REQUIRE(a.empty());
                REQUIRE(equal_il(a, {}));
                REQUIRE(a.get_allocator() == alloc);
            }

            SECTION("With size") {
                std::allocator<custom_type> alloc;
                vector<custom_type, 5> b(3, alloc);
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(b.get_allocator() == alloc);
            }

            SECTION("From value") {
                std::allocator<custom_type> alloc;
                vector<custom_type, 5> c(3, "one", alloc);
                REQUIRE(c.size() == 3);
                REQUIRE_FALSE(c.empty());
                REQUIRE(equal_il(c, {"one", "one", "one"}));
                REQUIRE(c.get_allocator() == alloc);
            }

            SECTION("From Iterators") {
                std::allocator<custom_type> alloc;
                std::vector<custom_type> dv = {"six", "five", "four"};
                vector<custom_type, 5> d(dv.begin(), dv.end(), alloc);
                REQUIRE(d.size() == 3);
                REQUIRE_FALSE(d.empty());
                REQUIRE(equal_il(d, {"six", "five", "four"}));
                REQUIRE(d.get_allocator() == alloc);
            }

            SECTION("From initializer list") {
                vector<custom_type, 5> e = {"one", "two"};
                REQUIRE(e.size() == 2);
                REQUIRE_FALSE(e.empty());
                REQUIRE(equal_il(e, {"one", "two"}));
            }

            SECTION("From ranges") {
                std::vector<custom_type> v = {"one", "two", "three"};
                vector<custom_type, 5> e(v);
                REQUIRE(e.size() == 3);
                REQUIRE_FALSE(e.empty());
                REQUIRE(equal_il(e, {"one", "two", "three"}));
            }
        }

        SECTION("Assign") {
            SECTION("From initializer list") {
                vector<custom_type, 5> a;
                REQUIRE(a.empty());
                a = {"six", "five", "four"};
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"six", "five", "four"}));
            }

            SECTION("From another small vector") {
                vector<custom_type, 5> a;
                REQUIRE(a.empty());
                a = {"six", "five", "four"};

                vector<custom_type, 5> b;
                REQUIRE(b.empty());
                b = a;
                REQUIRE(b.size() == 3);
                REQUIRE_FALSE(b.empty());
                REQUIRE(a == b);
            }

            SECTION("From iterators") {
                vector<custom_type, 5> a;
                REQUIRE(a.empty());
                std::vector<custom_type> v = {"six", "five", "four"};
                a.assign(v.begin(), v.end());
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"six", "five", "four"}));
            }

            SECTION("From size and value") {
                vector<custom_type, 5> a;
                REQUIRE(a.empty());
                a.assign(3, "one");
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"one", "one", "one"}));
            }

            SECTION("From initializer list") {
                vector<custom_type, 5> a;
                REQUIRE(a.empty());
                a.assign({"six", "five", "four"});
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"six", "five", "four"}));
            }

            SECTION("Fill") {
                vector<custom_type, 5> a(3, "one");
                REQUIRE_FALSE(a.empty());
                a.fill("two");
                REQUIRE(a.size() == 3);
                REQUIRE_FALSE(a.empty());
                REQUIRE(equal_il(a, {"two", "two", "two"}));
            }

            SECTION("Swap") {
                vector<custom_type, 5> a(4, "one");
                vector<custom_type, 5> b(3, "two");

                std::initializer_list<custom_type> ar = {"one", "one", "one", "one"};
                std::initializer_list<custom_type> br = {"two", "two", "two"};

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 4);
                REQUIRE(equal_il(a, ar));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(equal_il(b, br));

                a.swap(b);

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 3);
                REQUIRE(equal_il(a, br));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 4);
                REQUIRE(equal_il(b, ar));

                std::swap(a, b);

                REQUIRE_FALSE(a.empty());
                REQUIRE(a.size() == 4);
                REQUIRE(equal_il(a, ar));
                REQUIRE_FALSE(b.empty());
                REQUIRE(b.size() == 3);
                REQUIRE(equal_il(b, br));
            }
        }

        SECTION("Iterators") {
            vector<custom_type, 5> a = {"one", "two", "three"};

            REQUIRE(a.begin() == a.data());
            REQUIRE(a.end() == a.data() + a.size());
            REQUIRE_FALSE(a.end() == a.data() + a.capacity());
            REQUIRE(*a.begin() == "one");
            REQUIRE(*std::prev(a.end()) == "three");

            REQUIRE(a.cbegin() == a.data());
            REQUIRE(a.cend() == a.data() + a.size());
            REQUIRE_FALSE(a.cend() == a.data() + a.capacity());
            REQUIRE(*a.cbegin() == "one");
            REQUIRE(*std::prev(a.cend()) == "three");

            REQUIRE(*a.rbegin() == "three");
            REQUIRE(*std::prev(a.rend()) == "one");

            REQUIRE(*a.crbegin() == "three");
            REQUIRE(*std::prev(a.crend()) == "one");
        }

        SECTION("Capacity") {
            vector<custom_type, 5> a = {"one", "two", "three"};
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.is_inline());
            REQUIRE(sizeof(custom_type) == 160);
            REQUIRE(sizeof(custom_type *) == 8);
            REQUIRE(sizeof(custom_type) / sizeof(custom_type *) == 20);
            REQUIRE(sizeof(custom_type) > sizeof(custom_type *));
            REQUIRE(vector<custom_type, 5>::requested_inline_size == 5);
            REQUIRE(vector<custom_type, 5>::value_size == 160);
            REQUIRE(vector<custom_type, 5>::pointer_size == 8);
            REQUIRE(vector<custom_type, 5>::min_inline_elements == 2);
            REQUIRE(vector<custom_type, 5>::num_inline_elements == 5);
            REQUIRE(a.capacity() == 5);

            a.reserve(10);
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() >= 10);

            a.shrink_to_fit();
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.resize(4);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);

            a.shrink_to_fit();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a.capacity() == 5);
        }

        SECTION("Element access") {
            vector<custom_type, 5> a = {"one", "two", "three"};
            REQUIRE(a[0] == "one");
            REQUIRE(a[1] == "two");
            REQUIRE(a[2] == "three");
            REQUIRE(a.at(0) == "one");
            REQUIRE(a.at(1) == "two");
            REQUIRE(a.at(2) == "three");
            REQUIRE_THROWS(a.at(3) == "four");
            REQUIRE_THROWS(a.at(4) == "five");
            REQUIRE(a.front() == "one");
            REQUIRE(a.back() == "three");
            REQUIRE(*a.data() == "one");
            REQUIRE(*(a.data() + 1) == "two");
            REQUIRE(*(a.data() + 2) == "three");
            REQUIRE(*(a.data() + a.size() - 1) == "three");
            REQUIRE(*(a.data() + a.size() - 2) == "two");
            REQUIRE(*(a.data() + a.size() - 3) == "one");
        }

        SECTION("Modifiers") {
            vector<custom_type, 5> a = {"one", "two", "three"};
            a.push_back("four");
            REQUIRE(a.back() == "four");
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four"}));

            // NOLINTNEXTLINE(performance-move-const-arg)
            a.push_back(std::move("five"));
            REQUIRE(a.back() == "five");
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four", "five"}));

            a.pop_back();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four"}));

            a.emplace_back("five");
            REQUIRE(a.back() == "five");
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four", "five"}));

            a.pop_back();
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three", "four"}));

            auto it = a.emplace(a.begin() + 2, "ten");
            REQUIRE(it == a.begin() + 2);
            REQUIRE(a.back() == "four");
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "ten", "three", "four"}));

            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "ten"}));

            it = a.insert(a.begin() + 1, "twenty");
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "twenty", "two", "ten"}));

            // NOLINTNEXTLINE(performance-move-const-arg)
            it = a.insert(a.begin() + 2, std::move("thirty"));
            REQUIRE(it == a.begin() + 2);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "twenty", "thirty", "two", "ten"}));

            a.pop_back();
            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "twenty"}));

            it = a.insert(a.begin() + 1, 2, "ten");
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "ten", "ten", "twenty"}));

            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "ten"}));

            std::initializer_list<custom_type> src = {"two", "four", "eight"};
            it = a.insert(a.begin() + 1, src.begin(), src.end());
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "four", "eight", "ten"}));

            a.pop_back();
            a.pop_back();
            a.pop_back();
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two"}));

            it = a.insert(a.begin() + 1, {"two", "four", "eight"});
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 5);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "four", "eight", "two"}));

            it = a.erase(a.begin() + 1);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "four", "eight", "two"}));

            it = a.erase(a.begin() + 1, a.begin() + 3);
            REQUIRE(it == a.begin() + 1);
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two"}));

            a.clear();
            // NOLINTNEXTLINE(readability-container-size-empty)
            REQUIRE(a.size() == 0);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));

            a.resize(2);
            REQUIRE(a.size() == 2);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());

            a.resize(4, "five");
            REQUIRE(a.size() == 4);
            REQUIRE(a.max_size() > 5);
            REQUIRE(a.capacity() == 5);
            REQUIRE_FALSE(a.empty());
            REQUIRE(a[2] == "five");
            REQUIRE(a[3] == "five");
        }

        SECTION("Element access errors") {
            vector<custom_type, 5> a = {"one", "two", "three"};
            try {
                a.at(4);
            } catch (std::exception &e) {
                REQUIRE(e.what() == std::string_view("at: cannot access element after vector::size()"));
            }
        }

        SECTION("Relational Operators") {
            vector<custom_type, 5> a = {"one", "two", "three"};
            vector<custom_type, 5> b = {"two", "four", "five"};

            REQUIRE_FALSE(a == b);
            REQUIRE(a != b);
            REQUIRE(a < b);
            REQUIRE(a <= b);
            REQUIRE_FALSE(a > b);
            REQUIRE_FALSE(a >= b);
        }

        SECTION("From raw vector") {
            auto a = to_vector({"one", "two", "three"});
            REQUIRE(a.size() == 3);
            REQUIRE(a.max_size() > 3);
            REQUIRE(a.capacity() == 5);
            REQUIRE(a.capacity() == default_inline_storage_v<std::string>);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {"one", "two", "three"}));

            auto b = to_vector<custom_type, 3, 5>({"one", "two", "three"});
            REQUIRE(b.size() == 3);
            REQUIRE(b.max_size() > 5);
            REQUIRE(b.capacity() == 5);
            REQUIRE_FALSE(b.empty());
            REQUIRE(equal_il(b, {"one", "two", "three"}));

            custom_type cr[] = {"one", "two", "three"};
            auto c = to_vector(cr);
            REQUIRE(c.size() == 3);
            REQUIRE(c.max_size() > 3);
            REQUIRE(c.capacity() == 10);
            REQUIRE(c.capacity() == default_inline_storage_v<custom_type>);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(b, {"one", "two", "three"}));
        }
    }
}

TEST_CASE("Max size vector") {
    using namespace small;

    auto equal_il = [](const auto &sm_array, std::initializer_list<int> il) -> bool {
        return std::equal(sm_array.begin(), sm_array.end(), il.begin(), il.end());
    };

    auto full = [](const auto &sm_array) -> bool {
        return sm_array.is_inline() && sm_array.size() == sm_array.capacity();
    };

    SECTION("Constructor") {
        SECTION("Asserts") {
            REQUIRE(std::is_copy_constructible_v<max_size_vector<int, 5>>);
            REQUIRE(std::is_copy_assignable_v<max_size_vector<int, 5>>);
            REQUIRE(std::is_move_constructible_v<max_size_vector<int, 5>>);
            REQUIRE(std::is_move_assignable_v<max_size_vector<int, 5>>);

            REQUIRE(std::is_copy_constructible_v<max_size_vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_copy_assignable_v<max_size_vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_move_constructible_v<max_size_vector<std::pair<int, int>, 5>>);
            REQUIRE(std::is_move_assignable_v<max_size_vector<std::pair<int, int>, 5>>);
        }

        SECTION("Default") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("With size") {
            max_size_vector<int, 5> b(3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
        }

        SECTION("From value") {
            max_size_vector<int, 5> c(3, 1);
            REQUIRE(c.size() == 3);
            REQUIRE_FALSE(c.empty());
            REQUIRE(equal_il(c, {1, 1, 1}));
        }

        SECTION("From Iterators") {
            std::vector<int> dv = {6, 5, 4};
            max_size_vector<int, 5> d(dv.begin(), dv.end());
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(d, {6, 5, 4}));
        }

        SECTION("From initializer list") {
            max_size_vector<int, 5> e = {1, 2};
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(e, {1, 2}));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a = {6, 5, 4};
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {6, 5, 4}));
        }

        SECTION("From another small array") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a = {6, 5, 4};

            max_size_vector<int, 5> b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
        }

        SECTION("From iterators") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            std::vector<int> v = {6, 5, 4};
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {6, 5, 4}));
        }

        SECTION("From size and value") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a.assign(3, 1);
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {1, 1, 1}));
        }

        SECTION("From initializer list") {
            max_size_vector<int, 5> a;
            REQUIRE(a.empty());
            a.assign({6, 5, 4});
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {6, 5, 4}));
        }

        SECTION("Fill") {
            max_size_vector<int, 5> a(3, 1);
            REQUIRE_FALSE(a.empty());
            a.fill(2);
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(a, {2, 2, 2}));
        }

        SECTION("Swap") {
            max_size_vector<int, 5> a(4, 1);
            max_size_vector<int, 5> b(3, 2);

            std::initializer_list<int> ar = {1, 1, 1, 1};
            std::initializer_list<int> br = {2, 2, 2};

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));

            a.swap(b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 3);
            REQUIRE(equal_il(a, br));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 4);
            REQUIRE(equal_il(b, ar));

            std::swap(a, b);

            REQUIRE_FALSE(a.empty());
            REQUIRE(a.size() == 4);
            REQUIRE(equal_il(a, ar));
            REQUIRE_FALSE(b.empty());
            REQUIRE(b.size() == 3);
            REQUIRE(equal_il(b, br));
        }
    }

    SECTION("Iterators") {
        max_size_vector<int, 5> a = {1, 2, 3};

        REQUIRE(a.begin() == a.data());
        REQUIRE(a.end() == a.data() + a.size());
        REQUIRE_FALSE(a.end() == a.data() + a.capacity());
        REQUIRE(*a.begin() == 1);
        REQUIRE(*std::prev(a.end()) == 3);

        REQUIRE(a.cbegin() == a.data());
        REQUIRE(a.cend() == a.data() + a.size());
        REQUIRE_FALSE(a.cend() == a.data() + a.capacity());
        REQUIRE(*a.cbegin() == 1);
        REQUIRE(*std::prev(a.cend()) == 3);

        REQUIRE(*a.rbegin() == 3);
        REQUIRE(*std::prev(a.rend()) == 1);

        REQUIRE(*a.crbegin() == 3);
        REQUIRE(*std::prev(a.crend()) == 1);
    }

    SECTION("Capacity") {
        max_size_vector<int, 5> a = {1, 2, 3};
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(full(a));
    }

    SECTION("Element access") {
        max_size_vector<int, 5> a = {1, 2, 3};
        REQUIRE(a[0] == 1);
        REQUIRE(a[1] == 2);
        REQUIRE(a[2] == 3);
        REQUIRE(a.at(0) == 1);
        REQUIRE(a.at(1) == 2);
        REQUIRE(a.at(2) == 3);
        REQUIRE_THROWS(a.at(3) == 4);
        REQUIRE_THROWS(a.at(4) == 5);
        REQUIRE(a.front() == 1);
        REQUIRE(a.back() == 3);
        REQUIRE(*a.data() == 1);
        REQUIRE(*(a.data() + 1) == 2);
        REQUIRE(*(a.data() + 2) == 3);
        REQUIRE(*(a.data() + a.size() - 1) == 3);
        REQUIRE(*(a.data() + a.size() - 2) == 2);
        REQUIRE(*(a.data() + a.size() - 3) == 1);
    }

    SECTION("Modifiers") {
        max_size_vector<int, 5> a = {1, 2, 3};
        a.push_back(4);
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 2, 3, 4}));

        // NOLINTNEXTLINE(performance-move-const-arg)
        a.push_back(std::move(5));
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, {1, 2, 3, 4, 5}));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 2, 3, 4}));

        a.emplace_back(5);
        REQUIRE(a.back() == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, {1, 2, 3, 4, 5}));

        a.pop_back();
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 2, 3, 4}));

        auto it = a.emplace(a.begin() + 2, 10);
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.back() == 4);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, {1, 2, 10, 3, 4}));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 2, 10}));

        it = a.insert(a.begin() + 1, 20);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 20, 2, 10}));

        // NOLINTNEXTLINE(performance-move-const-arg)
        it = a.insert(a.begin() + 2, std::move(30));
        REQUIRE(it == a.begin() + 2);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, {1, 20, 30, 2, 10}));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 20}));

        it = a.insert(a.begin() + 1, 2, 10);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 10, 10, 20}));

        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 10}));

        std::initializer_list<int> src = {2, 4, 8};
        it = a.insert(a.begin() + 1, src.begin(), src.end());
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, {1, 2, 4, 8, 10}));

        a.pop_back();
        a.pop_back();
        a.pop_back();
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 2}));

        it = a.insert(a.begin() + 1, {2, 4, 8});
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(full(a));
        REQUIRE(equal_il(a, {1, 2, 4, 8, 2}));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 4, 8, 2}));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {1, 2}));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(equal_il(a, {}));

        a.resize(2);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));

        a.resize(4, 5);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE_FALSE(full(a));
        REQUIRE(a[2] == 5);
        REQUIRE(a[3] == 5);
    }

    SECTION("Element access errors") {
        max_size_vector<int, 5> a = {1, 2, 3};
        try {
            a.at(4);
        } catch (std::exception &e) {
            REQUIRE(e.what() == std::string_view("at: cannot access element after vector::size()"));
        }
    }

    SECTION("Relational Operators") {
        max_size_vector<int, 5> a = {1, 2, 3};
        max_size_vector<int, 5> b = {2, 4, 5};

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }

    SECTION("From raw array") {
        auto a = to_small_array({1, 2, 3});
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 3);
        REQUIRE(a.capacity() == 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.full());
        REQUIRE(equal_il(a, {1, 2, 3}));

        auto b = to_small_array<int, 3, 5>({1, 2, 3});
        REQUIRE(b.size() == 3);
        REQUIRE(b.max_size() == 5);
        REQUIRE(b.capacity() == 5);
        REQUIRE_FALSE(b.empty());
        REQUIRE_FALSE(b.full());
        REQUIRE(equal_il(b, {1, 2, 3}));

        int cr[] = {1, 2, 3};
        auto c = to_small_array(cr);
        REQUIRE(c.size() == 3);
        REQUIRE(c.max_size() == 3);
        REQUIRE(c.capacity() == 3);
        REQUIRE_FALSE(c.empty());
        REQUIRE(c.full());
        REQUIRE(equal_il(b, {1, 2, 3}));
    }
}

TEST_CASE("Pointer wrapper") {
    using namespace small;

    SECTION("Constructor") {
        SECTION("Empty") { [[maybe_unused]] pointer_wrapper<int *> p; }

        SECTION("From pointer") {
            int a = 2;
            pointer_wrapper<int *> p(&a);
            REQUIRE(p.base() == &a);
        }

        SECTION("From another pointer wrapper") {
            int a = 2;
            pointer_wrapper<int *> p1(&a);
            REQUIRE(p1.base() == &a);

            pointer_wrapper<int *> p2(p1);
            REQUIRE(p2.base() == &a);
        }
    }

    int a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    pointer_wrapper<int *> begin(&a[0]);
    pointer_wrapper<int *> end(&a[0] + 9);

    SECTION("Element access") {
        REQUIRE(begin != end);
        REQUIRE(*begin == 1);
        REQUIRE(*std::prev(end) == 9);
        REQUIRE(begin.base() == &a[0]);
        REQUIRE(begin[0] == 1);
        REQUIRE(begin[1] == 2);
        REQUIRE(begin[2] == 3);
    }

    SECTION("Modifiers") {
        ++begin;
        REQUIRE(*begin == 2);
        begin++;
        REQUIRE(*begin == 3);
        --begin;
        REQUIRE(*begin == 2);
        begin--;
        REQUIRE(*begin == 1);
        auto it = begin + 1;
        REQUIRE(*it == 2);
        it = begin + 2;
        REQUIRE(*it == 3);
        begin += 2;
        REQUIRE(*begin == 3);
        it = begin - 1;
        REQUIRE(*it == 2);
        it = begin - 2;
        REQUIRE(*it == 1);
        begin -= 2;
        REQUIRE(*begin == 1);
    }

    SECTION("Algorithms") {
        int b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
        pointer_wrapper<int *> b_begin(&b[0]);
        pointer_wrapper<int *> b_end(&b[0] + 9);

        std::copy(begin, end, b_begin);
        REQUIRE(std::equal(begin, end, b_begin, b_end));
    }
}