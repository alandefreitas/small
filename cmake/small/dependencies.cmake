#######################################################
### cpp_modules hints                               ###
#######################################################
# If user has a cpp_modules directory in their project but the user didn't
# set the hint paths properly (i.e.: no toolchain / no manager / no env variables),
# we need some helper functions to identify the default configuration and
# identify these extra hint paths.
include(cmake/functions/cpp_modules.cmake)

#######################################################
### Catch2                                          ###
#######################################################
set(catch2_VERSION_LOCK 2.13.6)
set(catch2_VERSION_REQUIREMENT ^2.0.0)
set_local_module_hints(catch2 ${catch2_VERSION_LOCK} ${catch2_VERSION_REQUIREMENT})

# Look for lock version
if (NOT Catch2_FOUND)
    semver_split(${catch2_VERSION_LOCK} catch2_VERSION_LOCK)
    find_package(Catch2 ${catch2_VERSION_LOCK_CORE} QUIET CONFIG)
endif ()

# Look for any version that matches our requirements
if (NOT Catch2_FOUND)
    find_package(Catch2 QUIET CONFIG)
    if (Catch2_FOUND AND catch2_VERSION AND NOT DEFINED ENV{catch2_ROOT})
        semver_requirements_compatible(${catch2_VERSION} ${catch2_VERSION_REQUIREMENT} ok)
        if (NOT ok)
            set(Catch2_FOUND FALSE)
        endif ()
    endif ()
endif ()

# Fetch catch2 if we couldn't find a valid version
if (NOT Catch2_FOUND)
    # Fallback to FetchContent and then find_package again
    message("Downloading catch2...")
    FetchContent_Declare(catch2
            URL https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.6.zip
            SOURCE_DIR ${catch2_SOURCE_HINT}
            BINARY_DIR ${catch2_BINARY_HINT}
            )

    # Check if already populated
    FetchContent_GetProperties(catch2)

    if (NOT catch2_POPULATED)
        # Download files
        FetchContent_Populate(catch2)

        # Run configure step
        execute_process(COMMAND "${CMAKE_COMMAND}"
                # CMake options
                -G "${CMAKE_GENERATOR}"
                -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
                # CMake install prefix
                -DCMAKE_INSTALL_PREFIX=${catch2_INSTALL_HINT}
                # Allow looking for <package>_ROOT
                -DCMAKE_POLICY_DEFAULT_CMP0074=NEW
                # Package options
                -DCATCH_USE_VALGRIND=OFF # "Perform SelfTests with Valgrind"
                -DCATCH_BUILD_TESTING=OFF # "Build SelfTest project"
                -DCATCH_BUILD_EXAMPLES=OFF # "Build documentation examples"
                -DCATCH_BUILD_EXTRA_TESTS=OFF # "Build extra tests"
                -DCATCH_BUILD_STATIC_LIBRARY=OFF # "Builds static library from the main implementation. EXPERIMENTAL"
                -DCATCH_ENABLE_COVERAGE=OFF # "Generate coverage for codecov.io"
                -DCATCH_ENABLE_WERROR=OFF # "Enable all warnings as errors"
                -DCATCH_INSTALL_DOCS=OFF # "Install documentation alongside library"
                -DCATCH_INSTALL_HELPERS=ON # "Install contrib alongside library"
                # Source dir
                ${catch2_SOURCE_DIR}
                # Build dir
                WORKING_DIRECTORY "${catch2_BINARY_DIR}"
                )

        # Run build step
        execute_process(COMMAND "${CMAKE_COMMAND}" --build .
                WORKING_DIRECTORY "${catch2_BINARY_DIR}"
                )

        # Run install step
        execute_process(COMMAND "${CMAKE_COMMAND}" --install .
                WORKING_DIRECTORY "${catch2_BINARY_DIR}"
                )
        # Find package again
        set(ENV{catch2_ROOT} ${catch2_INSTALL_HINT})
        find_package(Catch2 CONFIG  REQUIRED)
    endif ()
endif ()
version_requirement_message(catch2
        VERSION_FOUND ${catch2_VERSION}
        VERSION_LOCK ${catch2_VERSION_LOCK}
        VERSION_REQUIREMENTS ${catch2_VERSION_REQUIREMENT}
        PREFIX_HINT ${catch2_PREFIX_HINT})

