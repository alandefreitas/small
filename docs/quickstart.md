# Quickstart

Integration:

=== "CMake"

    === "Add subdirectory"

        ```bash
        git clone https://github.com/alandefreitas/small/
        ```

        ```cmake
        add_subdirectory(small)
        # ...
        add_executable(your_target main.cpp)
        target_link_libraries(your_target PUBLIC small::small)
        ```

    === "Fetch content"

        ```cmake
        include(FetchContent)
        
        FetchContent_Declare(small
            GIT_REPOSITORY https://github.com/alandefreitas/small
            GIT_TAG origin/master # or whatever tag you want
        )

        FetchContent_GetProperties(small)
        if(NOT small_POPULATED)
            FetchContent_Populate(small)
            add_subdirectory(${small_SOURCE_DIR} ${small_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()

        # ...
        add_executable(your_target main.cpp)
        target_link_libraries(your_target PUBLIC small::small)
        ```

    === "External package"

        ```cmake
        # Follow installation instructions and then... 
        find_package(small REQUIRED)
        if(NOT small_FOUND)
            # Throw or put your FetchContent script here
        endif()

        # ...
        add_executable(your_target main.cpp)
        target_link_libraries(your_target PUBLIC small::small)
        ```

=== "Install"

    !!! note

        Get the binary package from the [release section](https://github.com/alandefreitas/small/releases). 

        These binaries refer to the latest release version of small.

    !!! hint
        
        If you need a more recent version of `small`, you can download the binary packages from the CI artifacts or build the library from the source files.

=== "Build from source"

    !!! note

        Ensure your C++ compiler and CMake are up-to-date and then:

    === "Ubuntu + GCC"

        ```bash
        # Create a new directory
        mkdir build
        cd build
        # Configure
        cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
        # Build
        sudo cmake --build . --parallel 2 --config Release
        # Install 
        sudo cmake --install .
        # Create packages
        sudo cpack .
        ```

    === "Mac Os + Clang"
    
        ```bash
        # Create a new directory
        mkdir build
        cd build
        # Configure
        cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2"
        # Build
        cmake --build . --parallel 2 --config Release
        # Install 
        cmake --install .
        # Create packages
        cpack .
        ```
    
    === "Windows + MSVC"
    
        ```bash
        # Create a new directory
        mkdir build
        cd build
        # Configure
        cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="/O2"
        # Build
        cmake --build . --parallel 2 --config Release
        # Install 
        cmake --install .
        # Create packages
        cpack .
        ```
    
    !!! hint "Parallel Build"
        
        Replace `--parallel 2` with `--parallel <number of cores in your machine>`

    !!! note "Setting C++ Compiler"

        If your C++ compiler that supports C++17 is not your default compiler, make sure you provide CMake with the compiler location with the DCMAKE_C_COMPILER and DCMAKE_CXX_COMPILER options. For instance:
    
        ```bash
        cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O2" -DCMAKE_C_COMPILER=/usr/bin/gcc-8 -DCMAKE_CXX_COMPILER=/usr/bin/g++-8
        ```

=== "File amalgamation"

    !!! note

        Because containers are header-only, you can directly copy the contents from the `source` directory into your project.

    !!! hint

        In that case, you are responsible for setting the appropriate target include directories and any compile definitions you might require.


Once the library is properly integrated, you can create containers from the namespace `small` like any other STL container:

```cpp
--8<-- "examples/main.cpp"
```

All containers are optimized for the case when they're small but also efficient when they are large. The containers mix the common techniques found in other small container libraries:

- Inline allocation for small containers
- Custom expected sizes
- Identification of relocatable types
- Custom growth factors with better defaults
- Communication with system memory allocators
- Explicit consideration of CPU cache sizes and branch prediction

Most applications have many small lists and sets of elements. These containers avoid spending a lot of time with large containers
that contain just a few elements. Small containers usually try to use the stack before dynamically allocating memory and try
to represent associative containers with stack arrays, unless these sets are very large.

The following containers are available:

- `small::vector`
- `small::max_size_vector`
- `small::string`
- `small::set`
- `small::max_size_set`
- `small::multiset`
- `small::max_size_multiset`
- `small::unordered_set`
- `small::max_size_unordered_set`
- `small::unordered_multiset`
- `small::max_size_unordered_multiset`
- `small::map`
- `small::max_size_map`
- `small::multimap`
- `small::max_size_multimap`
- `small::unordered_map`
- `small::max_size_unordered_map`
- `small::unordered_multimap`
- `small::max_size_unordered_multimap`
- `small::stack`
- `small::queue`
- `small::priority_queue`

Although many compilers support small string optimization (SSO) already, this library will ensure all strings support SOO, custom inline sizes, relocation, and unicode.    

--8<-- "docs/references.md"