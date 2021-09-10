#######################################################
### Small containers library                        ###
#######################################################
add_library(small
        INTERFACE
        small/detail/algorithm/intcmp.h
        small/detail/algorithm/leading_zeros.h
        small/detail/algorithm/shift.h
        small/detail/algorithm/strlen.h
        small/detail/algorithm/to_unsigned.h
        small/detail/algorithm/utf.h
        small/detail/container/aligned_storage_vector.h
        small/detail/container/associative_vector.h
        small/detail/container/lookup_table_view.h
        small/detail/container/span-lite.h
        small/detail/container/variant_vector.h
        small/detail/exception/scope_guard.h
        small/detail/exception/throw.h
        small/detail/iterator/codepoint_iterator.h
        small/detail/iterator/const_key_iterator.h
        small/detail/iterator/iterator_type_traits.h
        small/detail/iterator/ordered_concat_iterator.h
        small/detail/iterator/pointer_wrapper.h
        small/detail/traits/add_key_const.h
        small/detail/traits/default_inline_storage.h
        small/detail/traits/enable_allocator_from_this.h
        small/detail/traits/extract_value_type.h
        small/detail/traits/has_allocator.h
        small/detail/traits/is_range.h
        small/detail/traits/is_relocatable.h
        small/detail/traits/little_endian.h
        small/detail/traits/strong_type.h
        small/map.h
        small/set.h
        small/string.h
        small/vector.h
        small/detail/algorithm/console_unicode_guard.h)

# Target aliases
# This emulates find_package when add_subdirectory has been used
add_library(small::small ALIAS small)

# This library requires C++17
target_compile_features(small INTERFACE cxx_std_17)

# Include directories (development and installation)
target_include_directories(small
        INTERFACE $<BUILD_INTERFACE:${SMALL_ROOT_DIR}/source>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )

# Compiler options
# MSVC compatibility
target_bigobj_options(small)
target_utf8_options(small)
target_nominmax_definition(small)

if (BUILD_WITH_EXCEPTIONS)
    target_exception_options(small)
endif ()

# Always enable warnings in development mode
maybe_target_pedantic_warnings(small)

#######################################################
### Installer                                       ###
#######################################################
if (SMALL_BUILD_INSTALLER)
    # Install targets
    install(TARGETS small
            EXPORT small-targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )

    # Install headers
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h"
            )

    # Install cmake script
    install(EXPORT small-targets
            FILE small-targets.cmake
            NAMESPACE small::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small
            )
endif()
