# Sets and Maps

The small set/map classes use a more cache-friendly flat set/map and all other optimizations mentioned above for internal algorithms. As with other small containers, a custom template parameter can be used to define the number of inline elements in the container.

The `small::default_inline_storage` and `small::is_relocatable` trait can also be defined for custom types, and all the usual set/map, ordered/unordered, uni/multi variants are also provided:

```cpp
--8<-- "examples/associative.cpp"
```

Unlike a `small::vector` or `small::string`, the asymptotic time complexities of flat sets/maps are very different from their `std::` counterparts and should only be used when they are small. Because they are internally implemented as arrays, manipulating these containers costs `O(n)`.

For large containers, you can use `std` containers with custom allocators. Or for efficient large containers, you can use the abseil containers, implemented as B+-trees.

--8<-- "docs/references.md"