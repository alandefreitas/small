//
// Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt
//

#include <small/map.hpp>
#include <algorithm>
#include <array>
#include <set>
#include <string>
#include <catch2/catch.hpp>
#include <string_view>

TEST_CASE("Small Map") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_map,
           std::initializer_list<std::pair<const int, int>> il) -> bool {
        return std::equal(sm_map.begin(), sm_map.end(), il.begin(), il.end());
    };

    using small_map_type = map<int, int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            small_map_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            detail::associative_vector<
                true,
                false,
                true,
                vector<
                    std::pair<int, int>,
                    5,
                    std::allocator<std::pair<int, int>>>>
                a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::pair<int, int>> alloc;
            std::vector<std::pair<int, int>> dv = {
                {4, 5},
                {5, 6},
                {7, 8}
            };
            small_map_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(
                d,
                {
                    {4, 5},
                    {5, 6},
                    {7, 8}
            }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            small_map_type e = {
                {1, 2},
                {2, 3}
            };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(
                e,
                {
                    {1, 2},
                    {2, 3}
            }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                {6, 7},
                {5, 4},
                {4, 5}
            };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    {4, 5},
                    {5, 4},
                    {6, 7}
            }));
        }

        SECTION("From another flat map") {
            small_map_type a;
            REQUIRE(a.empty());
            a = {
                {6, 7},
                {5, 6},
                {4, 5}
            };

            small_map_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(
                a,
                {
                    {4, 5},
                    {5, 6},
                    {6, 7}
            }));
        }

        SECTION("From iterators") {
            small_map_type a;
            REQUIRE(a.empty());
            std::vector<std::pair<int, int>> v = {
                {6, 4},
                {5, 6},
                {4, 6}
            };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    {4, 6},
                    {5, 6},
                    {6, 4}
            }));
        }

        SECTION("From initializer list") {
            small_map_type a;
            REQUIRE(a.empty());
            a.assign({
                {6, 5},
                {5, 2},
                {4, 2}
            });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    {4, 2},
                    {5, 2},
                    {6, 5}
            }));
        }

        SECTION("Swap") {
            small_map_type a = {
                {1, 2},
                {3, 4},
                {5, 6},
                {7, 8}
            };
            small_map_type b = {
                { 9, 10},
                {11, 12},
                {13, 14}
            };

            std::initializer_list<std::pair<const int, int>> ar = {
                {1, 2},
                {3, 4},
                {5, 6},
                {7, 8}
            };
            std::initializer_list<std::pair<const int, int>> br = {
                { 9, 10},
                {11, 12},
                {13, 14}
            };

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
        small_map_type a = {
            {1, 2},
            {2, 3},
            {3, 3}
        };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin()->first == 1);
        REQUIRE(std::prev(a.end())->first == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin()->first == 1);
        REQUIRE(std::prev(a.cend())->first == 3);

        REQUIRE(a.rbegin()->first == 3);
        REQUIRE(std::prev(a.rend())->first == 1);

        REQUIRE(a.crbegin()->first == 3);
        REQUIRE(std::prev(a.crend())->first == 1);
    }

    SECTION("Capacity") {
        small_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
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

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        small_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        REQUIRE(a[1] == 1);
        REQUIRE(a[2] == 2);
        REQUIRE(a[3] == 3);
        REQUIRE(a.at(1) == 1);
        REQUIRE(a.at(2) == 2);
        REQUIRE(a.at(3) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front().first == 1);
        REQUIRE(a.back().first == 3);
        REQUIRE(a.data()->first == 1);
        REQUIRE((a.data() + 1)->first == 2);
        REQUIRE((a.data() + 2)->first == 3);
        REQUIRE((a.data() + a.size() - 1)->first == 3);
        REQUIRE((a.data() + a.size() - 2)->first == 2);
        REQUIRE((a.data() + a.size() - 3)->first == 1);
    }

    SECTION("Modifiers") {
        small_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        a.insert({ 4, 4 });
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4}
        }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        auto p = std::make_pair(5, 5);
        a.insert(std::move(p));
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4},
                {5, 5}
        }));

        a.erase(5);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4}
        }));

        a.emplace(5, 5);
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4},
                {5, 5}
        }));

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4}
        }));

        auto it = a.emplace_hint(a.lower_bound(10), 10, 10);
        REQUIRE(it->first == (a.begin() + 4)->first);
        REQUIRE(a.back().first == 10);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1,  1},
                { 2,  2},
                { 3,  3},
                { 4,  4},
                {10, 10}
        }));

        a.erase(10);
        a.erase(4);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3}
        }));

        std::initializer_list<std::pair<const int, int>> src = {
            {6, 6},
            {5, 5},
            {7, 7}
        };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() >= 6);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {5, 5},
                {6, 6},
                {7, 7}
        }));

        a.erase(3);
        a.erase(5);
        a.erase(6);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 3);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {7, 7}
        }));

        a.insert({
            {4, 4},
            {5, 5},
            {6, 6}
        });
        REQUIRE(a.size() == 6);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 6);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {4, 4},
                {5, 5},
                {6, 6},
                {7, 7}
        }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {4, 4},
                {5, 5},
                {6, 6},
                {7, 7}
        }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {6, 6},
                {7, 7}
        }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() > 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        small_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        try {
            a.at(4);
        }
        catch (std::exception &e) {
            REQUIRE(
                e.what()
                == std::string_view("at(): cannot find element in vector map"));
        }
    }

    SECTION("Relational Operators") {
        small_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        small_map_type b = {
            {2, 2},
            {4, 4},
            {5, 5}
        };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}

TEST_CASE("Max Size Map") {
    using namespace small;

    auto equal_il =
        [](const auto &sm_map,
           std::initializer_list<std::pair<const int, int>> il) -> bool {
        return std::equal(sm_map.begin(), sm_map.end(), il.begin(), il.end());
    };

    using max_size_map_type = max_size_map<int, int, 5>;

    SECTION("Constructor") {
        SECTION("Default") {
            max_size_map_type a;
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
        }

        SECTION("Allocator") {
            std::allocator<int> alloc;
            detail::associative_vector<
                true,
                false,
                true,
                vector<
                    std::pair<int, int>,
                    5,
                    std::allocator<std::pair<int, int>>>>
                a(alloc);
            REQUIRE(a.empty());
            REQUIRE(equal_il(a, {}));
            REQUIRE(a.get_allocator() == alloc);
        }

        SECTION("From Iterators") {
            std::allocator<std::pair<int, int>> alloc;
            std::vector<std::pair<int, int>> dv = {
                {4, 5},
                {5, 6},
                {7, 8}
            };
            max_size_map_type d(dv.begin(), dv.end(), alloc);
            REQUIRE(d.size() == 3);
            REQUIRE_FALSE(d.empty());
            REQUIRE(equal_il(
                d,
                {
                    {4, 5},
                    {5, 6},
                    {7, 8}
            }));
            REQUIRE(d.get_allocator() == alloc);
        }

        SECTION("From initializer list") {
            max_size_map_type e = {
                {1, 2},
                {2, 3}
            };
            REQUIRE(e.size() == 2);
            REQUIRE_FALSE(e.empty());
            REQUIRE(equal_il(
                e,
                {
                    {1, 2},
                    {2, 3}
            }));
        }
    }

    SECTION("Assign") {
        SECTION("From initializer list") {
            max_size_map_type a;
            REQUIRE(a.empty());
            a = {
                {6, 7},
                {5, 4},
                {4, 5}
            };
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    {4, 5},
                    {5, 4},
                    {6, 7}
            }));
        }

        SECTION("From another flat map") {
            max_size_map_type a;
            REQUIRE(a.empty());
            a = {
                {6, 7},
                {5, 6},
                {4, 5}
            };

            max_size_map_type b;
            REQUIRE(b.empty());
            b = a;
            REQUIRE(b.size() == 3);
            REQUIRE_FALSE(b.empty());
            REQUIRE(a == b);
            REQUIRE(equal_il(
                a,
                {
                    {4, 5},
                    {5, 6},
                    {6, 7}
            }));
        }

        SECTION("From iterators") {
            max_size_map_type a;
            REQUIRE(a.empty());
            std::vector<std::pair<int, int>> v = {
                {6, 4},
                {5, 6},
                {4, 6}
            };
            a.assign(v.begin(), v.end());
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    {4, 6},
                    {5, 6},
                    {6, 4}
            }));
        }

        SECTION("From initializer list") {
            max_size_map_type a;
            REQUIRE(a.empty());
            a.assign({
                {6, 5},
                {5, 2},
                {4, 2}
            });
            REQUIRE(a.size() == 3);
            REQUIRE_FALSE(a.empty());
            REQUIRE(equal_il(
                a,
                {
                    {4, 2},
                    {5, 2},
                    {6, 5}
            }));
        }

        SECTION("Swap") {
            max_size_map_type a = {
                {1, 2},
                {3, 4},
                {5, 6},
                {7, 8}
            };
            max_size_map_type b = {
                { 9, 10},
                {11, 12},
                {13, 14}
            };

            std::initializer_list<std::pair<const int, int>> ar = {
                {1, 2},
                {3, 4},
                {5, 6},
                {7, 8}
            };
            std::initializer_list<std::pair<const int, int>> br = {
                { 9, 10},
                {11, 12},
                {13, 14}
            };

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
        max_size_map_type a = {
            {1, 2},
            {2, 3},
            {3, 3}
        };

        REQUIRE(a.begin().operator->() == a.data());
        REQUIRE(a.end().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.end().operator->() == a.data() + a.capacity());
        REQUIRE(a.begin()->first == 1);
        REQUIRE(std::prev(a.end())->first == 3);

        REQUIRE(a.cbegin().operator->() == a.data());
        REQUIRE(a.cend().operator->() == a.data() + a.size());
        REQUIRE_FALSE(a.cend().operator->() == a.data() + a.capacity());
        REQUIRE(a.cbegin()->first == 1);
        REQUIRE(std::prev(a.cend())->first == 3);

        REQUIRE(a.rbegin()->first == 3);
        REQUIRE(std::prev(a.rend())->first == 1);

        REQUIRE(a.crbegin()->first == 3);
        REQUIRE(std::prev(a.crend())->first == 1);
    }

    SECTION("Capacity") {
        max_size_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        REQUIRE_THROWS(a.reserve(10));
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == 5);

        a.shrink_to_fit();
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(a.capacity() == (std::max)(size_t(5), a.size()));
    }

    SECTION("Element access") {
        max_size_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        REQUIRE(a[1] == 1);
        REQUIRE(a[2] == 2);
        REQUIRE(a[3] == 3);
        REQUIRE(a.at(1) == 1);
        REQUIRE(a.at(2) == 2);
        REQUIRE(a.at(3) == 3);
        REQUIRE_THROWS(a.at(4));
        REQUIRE_THROWS(a.at(5));
        REQUIRE(a.front().first == 1);
        REQUIRE(a.back().first == 3);
        REQUIRE(a.data()->first == 1);
        REQUIRE((a.data() + 1)->first == 2);
        REQUIRE((a.data() + 2)->first == 3);
        REQUIRE((a.data() + a.size() - 1)->first == 3);
        REQUIRE((a.data() + a.size() - 2)->first == 2);
        REQUIRE((a.data() + a.size() - 3)->first == 1);
    }

    SECTION("Modifiers") {
        max_size_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        a.insert({ 4, 4 });
        REQUIRE(a.back().first == 4);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4}
        }));

        // NOLINTNEXTLINE(performance-move-const-arg)
        auto p = std::make_pair(5, 5);
        a.insert(std::move(p));
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4},
                {5, 5}
        }));

        a.erase(5);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4}
        }));

        a.emplace(5, 5);
        REQUIRE(a.back().first == 5);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4},
                {5, 5}
        }));

        a.erase(std::prev(a.end()));
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {4, 4}
        }));

        auto it = a.emplace_hint(a.lower_bound(10), 10, 10);
        REQUIRE(it->first == (a.begin() + 4)->first);
        REQUIRE(a.back().first == 10);
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                { 1,  1},
                { 2,  2},
                { 3,  3},
                { 4,  4},
                {10, 10}
        }));

        a.erase(10);
        a.erase(4);
        REQUIRE(a.size() == 3);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3}
        }));

        std::initializer_list<std::pair<const int, int>> src = {
            {6, 6},
            {5, 5}
        };
        a.insert(src.begin(), src.end());
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {3, 3},
                {5, 5},
                {6, 6}
        }));

        a.erase(3);
        a.erase(5);
        a.erase(6);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2}
        }));

        a.insert({
            {4, 4},
            {5, 5},
            {6, 6}
        });
        REQUIRE(a.size() == 5);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() == 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {2, 2},
                {4, 4},
                {5, 5},
                {6, 6}
        }));

        it = a.erase(a.begin() + 1);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 4);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {4, 4},
                {5, 5},
                {6, 6}
        }));

        it = a.erase(a.begin() + 1, a.begin() + 3);
        REQUIRE(it == a.begin() + 1);
        REQUIRE(a.size() == 2);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE_FALSE(a.empty());
        REQUIRE(equal_il(
            a,
            {
                {1, 1},
                {6, 6}
        }));

        a.clear();
        // NOLINTNEXTLINE(readability-container-size-empty)
        REQUIRE(a.size() == 0);
        REQUIRE(a.max_size() == 5);
        REQUIRE(a.capacity() >= 5);
        REQUIRE(a.empty());
        REQUIRE(equal_il(a, {}));
    }

    SECTION("Element access errors") {
        max_size_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        try {
            a.at(4);
        }
        catch (std::exception &e) {
            REQUIRE(
                e.what()
                == std::string_view("at(): cannot find element in vector map"));
        }
    }

    SECTION("Relational Operators") {
        max_size_map_type a = {
            {1, 1},
            {2, 2},
            {3, 3}
        };
        max_size_map_type b = {
            {2, 2},
            {4, 4},
            {5, 5}
        };

        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE(a < b);
        REQUIRE(a <= b);
        REQUIRE_FALSE(a > b);
        REQUIRE_FALSE(a >= b);
    }
}
