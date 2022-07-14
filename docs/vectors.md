# Vectors

This small vector implementation includes:

- Inline allocation for small vectors
- Custom expected size
- Special treatment of relocatable types
    - Relocatable types can be moved with `memcpy`, bypassing destructors and constructors.
    - Relocatable types are defined by default for POD types and aggregate types of PODs
    - The `small::is_relocatable` traits can be used as an extension point for custom types
- Better growth factors
- Consider the cache line size in allocations
- Heap allocations can be disabled with `small::max_size_vector`

When there are fewer elements than a given threshold, the elements are kept in a stack buffer for small vectors. Otherwise, the vector works as usual. However, if you are 100% sure you will never need more than `N` elements, you can use a `max_size_vector`, where elements are always inline.

The default number of elements in a small vector is usually the number of elements we can already fit inline in a vector. For larger data types, the `default_inline_storage` trait can be used as an extension point where one can define how many elements a small vector of that type should contain by default.

```cpp
--8<-- "examples/default_inline_storage.cpp"
```

When there's a reasonable default for the number of inline elements, this strategy avoids multiple vector type instantiations for different inline storage sizes.

This small vector implementation used folly, abseil, and LLVM as a reference.

--8<-- "docs/references.md"