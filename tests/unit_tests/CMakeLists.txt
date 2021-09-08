#######################################################
### Unit Tests                                      ###
#######################################################
# Include catch helper CMake scripts
include(${catch2_DIR}/Catch.cmake)

# Create a common catch main for all tests
add_library(catch_main catch_main.cpp)
target_link_libraries(catch_main PUBLIC Catch2::Catch2)

# Macro to create test targets and register with ctest
# First parameter: filename without extension
# Other parameters: libraries to link in this test (if empty, it links libcppm)
macro(add_small_test TEST_NAME)
    # Identify libraries to link with
    set(EXTRA_MACRO_ARGS ${ARGN})
    list(LENGTH EXTRA_MACRO_ARGS NUM_EXTRA_ARGS)
    if (${NUM_EXTRA_ARGS} GREATER 0)
        set(LINK_LIBS ${ARGN})
    else ()
        set(LINK_LIBS small::small)
    endif ()

    # Check if these libraries really exist
    set(LINK_LIBS_EXIST TRUE)
    foreach (LINK_LIB ${LINK_LIBS})
        if (NOT TARGET ${LINK_LIB})
            set(LINK_LIBS_EXIST FALSE)
            break()
        endif ()
    endforeach ()

    if (LINK_LIBS_EXIST)
        # Create executable for test
        add_executable(ut_${TEST_NAME} ${TEST_NAME}.cpp)

        # Link with catch-main
        target_link_libraries(ut_${TEST_NAME} PUBLIC ${LINK_LIBS} catch_main)

        # Register with ctest
        catch_discover_tests(ut_${TEST_NAME})

    else ()
        # Library not found. Throw.
        message(FATAL_ERROR "${LINK_LIBS} does not exist")
    endif ()
endmacro()

# Main tests
add_small_test(small_vector small::small)
add_small_test(small_string small::small)
add_small_test(small_map small::small)
add_small_test(small_set small::small)
