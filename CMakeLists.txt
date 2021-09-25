#######################################################
### Small                                           ###
#######################################################
# Project information
cmake_minimum_required(VERSION 3.15)
project(small VERSION 0.1.2)
set(SMALL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SMALL_VERSION ${CMAKE_PROJECT_VERSION})
message(STATUS "${CMAKE_PROJECT_NAME} v${CMAKE_PROJECT_VERSION}: ${CMAKE_PROJECT_HOMEPAGE_URL}")

#######################################################
### CMake Functions                                 ###
#######################################################
# CMake dependencies for installer
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# Append ./cmake directory to our include paths for the find_package scripts
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Functions to find or download packages if we can't find_package
include(FetchContent)

# Our custom cmake functions
include(cmake/functions.cmake)

#######################################################
### Options                                         ###
#######################################################
# Set variables with project properties
set_master_project_booleans() # detect if master project / dev mode
set_debug_booleans() # detect if debug
set_optimization_flags() # detect and set default optimization flags
set_compiler_booleans() # detect compiler

# What to build
option(SMALL_BUILD_INSTALLER "Build installer target" ${MASTER_PROJECT})
option(SMALL_BUILD_PACKAGE "Build package" ${MASTER_PROJECT})

# How to build
option(SMALL_DEV_BUILD "Development build" ${MASTER_PROJECT})

#######################################################
### Dev Options                                     ###
#######################################################
# What to build
option(SMALL_BUILD_EXAMPLES "Build examples" ${SMALL_DEV_BUILD})
option(SMALL_BUILD_TESTS "Build tests" ${SMALL_DEV_BUILD})
if (APPLE OR CLANG)
    set(DEFAULT_SMALL_BUILD_TESTS_WITH_PCH OFF)
else()
    set(DEFAULT_SMALL_BUILD_TESTS_WITH_PCH ${SMALL_DEV_BUILD})
endif()
option(SMALL_BUILD_TESTS_WITH_PCH "Build tests with precompiled headers" ${DEFAULT_SMALL_BUILD_TESTS_WITH_PCH})

# How to build
option(SMALL_BUILD_WITH_PEDANTIC_WARNINGS "Use pedantic warnings. This is useful for developers because many of these warnings will be in continuous integration anyway." ${SMALL_DEV_BUILD})
option(SMALL_BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(SMALL_BUILD_WITH_SANITIZERS "Use pedantic warnings." ${DEBUG_MODE})

# MSVC hacks
option(SMALL_BUILD_WITH_MSVC_HACKS "Accept utf-8 in MSVC by default." ON)
option(SMALL_BUILD_WITH_UTF8 "Accept utf-8 in MSVC by default." ON)
option(SMALL_BUILD_WITH_EXCEPTIONS "Add compiler flags to use exceptions." ON)

#######################################################
### Apply global options                            ###
#######################################################
# In development, we can set some options for all targets
if (MASTER_PROJECT)
    message("Setting global options")
    # Maybe add sanitizers to all targets
    if (SMALL_BUILD_WITH_SANITIZERS AND NOT EMSCRIPTEN)
        add_sanitizers()
    endif ()

    # Allow exceptions in MSVC
    if (MSVC AND SMALL_BUILD_WITH_EXCEPTIONS)
        add_compile_options(/EHsc)
    endif ()

    # Allow utf-8 in MSVC
    if (SMALL_BUILD_WITH_UTF8 AND MSVC)
        set(CMAKE_CXX_FLAGS "/utf-8")
    endif ()

    # MSVC hack to disable windows min/max
    # http://www.suodenjoki.dk/us/archive/2010/min-max.htm
    if (MSVC AND SMALL_BUILD_WITH_MSVC_HACKS)
        # Check for min in Windows.h
        # include(CheckSymbolExists)
        # check_symbol_exists(min "WinDef.h" HAVE_WINDOWS_MINMAX)
        # if (NOT HAVE_WINDOWS_MINMAX)
        #     check_symbol_exists(min "Windows.h" HAVE_WINDOWS_MINMAX)
        # endif ()
        # if (HAVE_WINDOWS_MINMAX)
        add_compile_definitions(NOMINMAX)
        # endif ()
    endif ()
endif()

#######################################################
### Dependencies                                    ###
#######################################################
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/find_cpp_modules.cmake)
    include(find_cpp_modules.cmake)
else ()
    message(FATAL_ERROR "find_cpp_modules.cmake file not found.")
endif ()

#######################################################
### Libraries                                       ###
#######################################################
add_subdirectory(source)

#######################################################
### Tests                                           ###
#######################################################
if (SMALL_BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif ()

#######################################################
### Examples                                        ###
#######################################################
if (SMALL_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

#######################################################
### Installer                                       ###
#######################################################
if (SMALL_BUILD_INSTALLER)
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/installing.html
    # Set variable where the cmake config is
    set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/small)
    message(STATUS "${CMAKE_PROJECT_NAME} install directory: ${CMAKE_INSTALL_PREFIX}")
    message(STATUS "${CMAKE_PROJECT_NAME} library install directory: ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

    # Create small-config-version.cmake and install it
    if (CMAKE_PROJECT_VERSION VERSION_LESS 1.0.0)
        set(compatibility_mode SameMajorVersion)
    else()
        set(compatibility_mode SameMinorVersion)
    endif()
    write_basic_package_version_file(
            small-config-version.cmake
            VERSION ${PACKAGE_VERSION}
            COMPATIBILITY ${compatibility_mode}
    )

    # Install the file small-config-version.cmake
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/small-config-version.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small)

    # Create small-config.cmake from small-config.cmake.in
    configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/small-config.cmake.in # input file
            ${CMAKE_CURRENT_BINARY_DIR}/small-config.cmake    # output file
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small
    )

    # Install the file small-config.cmake
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/small-config.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small)
endif ()

#######################################################
### Packages                                        ###
#######################################################
if (SMALL_BUILD_INSTALLER AND SMALL_BUILD_PACKAGE)
    # Set the cpack variables
    # https://cliutils.gitlab.io/modern-cmake/chapters/install/packaging.html

    # The most common cpack variables
    set(CPACK_PACKAGE_VENDOR "small")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "small: C++ small containers")
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_RESOURCE_FILE_LICENSE "${SMALL_ROOT_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${SMALL_ROOT_DIR}/README.md")

    # Set CPACK_SOURCE_IGNORE_FILES with files source packages shouldn't install
    # We get these from .gitignore to avoid redundancy
    FILE(READ .gitignore GITIGNORE_CONTENTS)
    STRING(REGEX REPLACE ";" "\\\\;" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    STRING(REGEX REPLACE "\n" ";" GITIGNORE_CONTENTS "${GITIGNORE_CONTENTS}")
    set(CPACK_SOURCE_IGNORE_FILES ${GITIGNORE_CONTENTS})

    # Always include CPack at last
    include(CPack)
endif ()
