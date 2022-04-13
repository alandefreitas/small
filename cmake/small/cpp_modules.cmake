# Setting hint paths and configuration hints is a delicate process
# if we want to ensure transitive dependencies work. Many hints
# are stored in environment variables so that child processes
# can find or build transitive dependencies with the proper
# configuration values we want.

# We need some functions to process semver version requirements, as
# CMake does not natively support semver requirements and they are
# important to ensure the correct dependency versions are found.
# To make sure this is not computationally expensive, these functions
# avoid relying on regex and do attempt to validate the input.
# The calling functions are also designed to avoid checking the most
# expensive requirements unless it really has to.
include(cmake/functions/semver.cmake)

#######################################################
### Configuration descriptor                        ###
#######################################################
# Create a configuration descriptor with the core config options for this library.
# This is the default core configuration, unless these options are overridden.
# It ensures all dependencies were compiled with compatible with the current toolchain.
function(generate_config_descriptor)
    # Build type
    string(TOLOWER ${CMAKE_BUILD_TYPE} LC_CMAKE_BUILD_TYPE)
    if (NOT BUILD_SHARED_LIBS)
        set(LC_BUILD_SHARED_LIBS static)
    else ()
        set(LC_BUILD_SHARED_LIBS dynamic)
    endif ()
    # Compiler and compiler version
    string(TOLOWER ${CMAKE_CXX_COMPILER_ID} LC_CMAKE_CXX_COMPILER_ID)
    string(TOLOWER ${CMAKE_CXX_COMPILER_VERSION} LC_CMAKE_CXX_COMPILER_VERSION)
    # Processor
    string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} LC_CMAKE_SYSTEM_PROCESSOR)
    # OS and OS version
    string(TOLOWER ${CMAKE_SYSTEM_NAME} LC_CMAKE_SYSTEM_NAME)
    string(TOLOWER ${CMAKE_SYSTEM_VERSION} LC_CMAKE_SYSTEM_VERSION)
    # Concatenate strings
    set(CONFIG_CORE
            "${LC_CMAKE_BUILD_TYPE}-${LC_BUILD_SHARED_LIBS}-${LC_CMAKE_CXX_COMPILER_ID}-${LC_CMAKE_CXX_COMPILER_VERSION}-${LC_CMAKE_SYSTEM_PROCESSOR}-${LC_CMAKE_SYSTEM_NAME}-${LC_CMAKE_SYSTEM_VERSION}"
            PARENT_SCOPE)
endfunction()
generate_config_descriptor()
message("- Find packages for ${CONFIG_CORE}")

# This function sets a path hint for the package in case it already exists locally in a cpp_modules
# directory. If no such directory exists, the process continues as usual with find_package so the
# process still is non-intrusive apart from this convenience system inspection.
# This logic is somewhat redundant with the "paths" toolchain, but it solves two problems:
# - The "paths" toolchain is somewhat intrusive because the user will ultimately need cppm
#   to generate this toolchain correctly.
# - This is very little intrusive as all of this will be ignored if the cpp_modules path does not exist.
# - This is also very little intrusive as it ensures package consumers don't need cppm to find dependencies.
# - Using a toolchain to set hints is optional and might be considered is bad workaround by some:
#   - Setting hint paths does not really constitute a new *toolchain* (compiler+link+...) at all.
#   - The "paths" toolchain creates difficulties for the user when a real toolchain is required for cross-compiling.
#      - This difficulty can be solved with "second-order" toolchains on a mock toolchain, which is quite a workaround.
# All the logic in these functions avoid compiling any semver string we don't really need. Whenever possible
# we check for an existing directory with the lock version. We need compare the parse and semver strings only if:
# - (i) no hint has been set by the user
# - (ii) the original lock directory does not exist
# - (iii) a number of other reasonable candidates exist for some rare reason
# Although option (iii) includes the cost of splitting semver strings, this is much cheaper than
# redundantly downloading the library again or throwing an error for a false positive.
function(set_local_module_hints PACKAGE_NAME LOCK_VERSION REQUIREMENTS)
    # Check if user's hasn't already set his own hint
    if (NOT ENV{${PACKAGE_NAME}_ROOT} AND NOT ${PACKAGE_NAME}_ROOT)
        # Look for local cpp_modules path with this package
        string(TOLOWER ${PACKAGE_NAME} LC_PACKAGE_NAME)
        set(${PACKAGE_NAME}_DIR_HINT ${CMAKE_SOURCE_DIR}/cpp_modules/${LC_PACKAGE_NAME})
        if (EXISTS ${${PACKAGE_NAME}_DIR_HINT} AND IS_DIRECTORY ${${PACKAGE_NAME}_DIR_HINT})
            # Base directory with package exists. Look for a compatible version now.
            # Try a directory with the lock version first, in order to avoid inspecting the system
            set(${PACKAGE_NAME}_PREFIX_HINT ${${PACKAGE_NAME}_DIR_HINT}/${LOCK_VERSION})
            # If the locked version is not available in cpp_modules
            if (NOT (EXISTS ${${PACKAGE_NAME}_PREFIX_HINT} AND IS_DIRECTORY ${${PACKAGE_NAME}_PREFIX_HINT}))
                # Go through other subdirectories looking for some compatible version
                file(GLOB VERSION_SUBDIRS ${${PACKAGE_NAME}_DIR_HINT}/*)
                foreach (VERSION_SUBDIR ${VERSION_SUBDIRS})
                    # Get semver version this directory represents
                    get_filename_component(VERSION_SUBDIR_VERSION ${VERSION_SUBDIR} NAME)
                    if (VERSION_SUBDIR_VERSION VERSION_GREATER_EQUAL LOCK_VERSION)
                        # Check if version is compatible with requirements
                        semver_requirements_compatible(${VERSION_SUBDIR_VERSION} ${REQUIREMENTS} match_requirements)
                        if (match_requirements)
                            # Store this compatible version and the prefix hint
                            if (NOT MOST_COMPATIBLE_VERSION)
                                set(MOST_COMPATIBLE_VERSION ${VERSION_SUBDIR_VERSION})
                                set(${PACKAGE_NAME}_PREFIX_HINT ${VERSION_SUBDIR})
                            else ()
                                # If this is not the first compatible version we find
                                # Check if this is a more recent candidate than what we had before
                                semver_greater(${VERSION_SUBDIR_VERSION} ${MOST_COMPATIBLE_VERSION} gt)
                                if (gt)
                                    set(MOST_COMPATIBLE_VERSION ${VERSION_SUBDIR_VERSION})
                                    set(${PACKAGE_NAME}_PREFIX_HINT ${VERSION_SUBDIR})
                                else ()
                                    # If this is not greater than the previous version we found,
                                    # it might still be the edge-case where the previous version we found
                                    # didn't match the requirements and we just had it here because
                                    # it was compatible with the lock version. In that case, we still favour
                                    # this version
                                    semver_requirements_compatible(${MOST_COMPATIBLE_VERSION} ${REQUIREMENTS} match_requirements)
                                    if (NOT match_requirements)
                                        set(MOST_COMPATIBLE_VERSION ${VERSION_SUBDIR_VERSION})
                                        set(${PACKAGE_NAME}_PREFIX_HINT ${VERSION_SUBDIR})
                                    endif ()
                                endif ()
                            endif ()
                        else ()
                            # We still store the hint if this does not match the requirements but
                            # it's semver compatible with the lock version
                            semver_compatible(${VERSION_SUBDIR_VERSION} ${LOCK_VERSION} compatible_with_lock)
                            if (compatible_with_lock)
                                set(MOST_COMPATIBLE_VERSION ${VERSION_SUBDIR_VERSION})
                                set(${PACKAGE_NAME}_PREFIX_HINT ${VERSION_SUBDIR})
                            endif ()
                        endif ()
                    endif ()
                endforeach ()
            endif ()
        endif ()

        # If we found no prefix with a reasonable hint
        if (NOT ${PACKAGE_NAME}_PREFIX_HINT)
            # Reset this to the lock version so that FetchContents puts the contents here
            # if we really need to use it. However, the user might have the package installed
            # globally before that.
            set(${PACKAGE_NAME}_PREFIX_HINT ${${PACKAGE_NAME}_DIR_HINT}/${LOCK_VERSION})
        endif ()

        # Return expected hint paths
        set(${PACKAGE_NAME}_PREFIX_HINT ${${PACKAGE_NAME}_PREFIX_HINT} PARENT_SCOPE)
        set(${PACKAGE_NAME}_SOURCE_HINT ${${PACKAGE_NAME}_PREFIX_HINT}/source PARENT_SCOPE)
        set(${PACKAGE_NAME}_BINARY_HINT ${${PACKAGE_NAME}_PREFIX_HINT}/build/${CONFIG_CORE} PARENT_SCOPE)
        set(${PACKAGE_NAME}_INSTALL_HINT ${${PACKAGE_NAME}_PREFIX_HINT}/install/${CONFIG_CORE} PARENT_SCOPE)

        # Set the root directory, where CMake might also look for the package
        set(${PACKAGE_NAME}_ROOT ${${PACKAGE_NAME}_PREFIX_HINT}/install/${CONFIG_CORE} PARENT_SCOPE)
        # Also set this as an ENV variable for the build script of transitive dependencies
        set(ENV{${PACKAGE_NAME}_ROOT} ${${PACKAGE_NAME}_PREFIX_HINT}/install/${CONFIG_CORE})

        # Do the same, with capitalized package name
        string(SUBSTRING ${PACKAGE_NAME} 0 1 PACKAGE_NAME_FRONT)
        string(TOUPPER ${PACKAGE_NAME_FRONT} PACKAGE_NAME_FRONT)
        string(SUBSTRING ${PACKAGE_NAME} 1 -1 PACKAGE_NAME_TAIL)
        set(CAPITALIZED_PACKAGE_NAME "${PACKAGE_NAME_FRONT}${PACKAGE_NAME_TAIL}")
        set(${CAPITALIZED_PACKAGE_NAME}_PREFIX_HINT ${${PACKAGE_NAME}_PREFIX_HINT} PARENT_SCOPE)
        set(${CAPITALIZED_PACKAGE_NAME}_SOURCE_HINT ${${PACKAGE_NAME}_PREFIX_HINT}/source PARENT_SCOPE)
        set(${CAPITALIZED_PACKAGE_NAME}_BINARY_HINT ${${PACKAGE_NAME}_PREFIX_HINT}/build/${CONFIG_CORE} PARENT_SCOPE)
        set(${CAPITALIZED_PACKAGE_NAME}_INSTALL_HINT ${${PACKAGE_NAME}_PREFIX_HINT}/install/${CONFIG_CORE} PARENT_SCOPE)
        set(${CAPITALIZED_PACKAGE_NAME}_ROOT ${${PACKAGE_NAME}_PREFIX_HINT}/install/${CONFIG_CORE} PARENT_SCOPE)
        set(ENV{${CAPITALIZED_PACKAGE_NAME}_ROOT} ${${PACKAGE_NAME}_PREFIX_HINT}/install/${CONFIG_CORE})
    endif ()
endfunction()