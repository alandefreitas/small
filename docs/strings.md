# Strings

The small string includes all the common optimizations for small vectors, and a custom template parameter to set how large we expect the string to be (in bytes).

However, when strings are representing text, if there's one thing that makes them not small is not supporting UTF8. In addition to the common interface for strings, `small::string` includes extra functions to identify and work with UTF8 code points with random access.

```cpp
--8<-- "examples/unicode_strings.cpp"
```

The problem of supporting UTF8 is easier to explain than it is to solve. Programming languages tend to solve this problem by (1) forbidding byte or substring access, and/or (2) allowing only access to code points with `O(n)` cost, where `n` is the number of code points. Because anything that forbids byte access would be incompatible with a C++ string, we allow direct byte access, and strings are allowed to be malformed unicode, which we can check with `small::is_malformed`.

All capacity and access functions contain extra overloads that accept codepoint indexes, defined as a strong type, rather than byte indexes. By using these functions, one can ensure the string is never malformed. It's up to the user to decide whether these access functions are useful and worth it in a particular application.

Access to codepoints is provided with an inline lookup-table trick that allows us to access codepoints in `O(log m)` time, where `m` is the number of multibyte code points in the strings. When there are no multibyte codepoints in the string, the string works as usual and no extra memory is required for the table.

--8<-- "docs/references.md"