#
# Copyright (c) 2022 alandefreitas (alandefreitas@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#

#######################################################
### Dependencies                                    ###
#######################################################
find_program(DOXYGEN_EXECUTABLE doxygen)
if (DOXYGEN_EXECUTABLE)
    message(STATUS "doxygen found: ${DOXYGEN_EXECUTABLE}")
else ()
    message(FATAL_ERROR "doxygen executable not found (https://www.doxygen.nl/index.html)")
endif()

find_program(DOXYBOOK2_EXECUTABLE doxybook2)
if (DOXYBOOK2_EXECUTABLE)
    message(STATUS "doxybook2 found: ${DOXYBOOK2_EXECUTABLE}")
else ()
    message(STATUS "doxybook2 executable not found (https://github.com/matusnovak/doxybook2)")
endif()

if (NOT DOXYBOOK2_EXECUTABLE)
    include(FetchContent)
    if (APPLE)
        set(DOXYBOOK_SYSTEM_FILE osx-amd64)
    elseif (UNIX)
        set(DOXYBOOK_SYSTEM_FILE linux-amd64)
    elseif (WIN32)
        set(DOXYBOOK_SYSTEM_FILE windows-win64)
    endif ()
    FetchContent_Declare(
            doxybook2
            URL "https://github.com/matusnovak/doxybook2/releases/download/v1.4.0/doxybook2-${DOXYBOOK_SYSTEM_FILE}-v1.4.0.zip"
            SOURCE_DIR "${PROJECT_BINARY_DIR}/doxybook2"
            UPDATE_DISCONNECTED YES
    )
    FetchContent_GetProperties(doxybook2)
    if (NOT doxybook2_POPULATED)
        FetchContent_Populate(doxybook2)
        find_program(DOXYBOOK2_EXECUTABLE doxybook2 PATHS "${doxybook2_SOURCE_DIR}/bin")
        if (DOXYBOOK2_EXECUTABLE)
            message(STATUS "doxybook2 downloaded: ${DOXYBOOK2_EXECUTABLE}")
        else ()
            message(WARNING "doxybook2 executable not found (https://github.com/matusnovak/doxybook2)")
        endif()
    endif()
endif ()

find_program(MKDOCS_EXECUTABLE mkdocs)
if (MKDOCS_EXECUTABLE)
    message(STATUS "mkdocs found: ${MKDOCS_EXECUTABLE}")
else ()
    message(FATAL_ERROR "mkdocs executable not found (https://www.mkdocs.org/)")
endif()

#######################################################
### Documentation targets                           ###
#######################################################
# Documentation is built in-source as required by mkdocs and github
set(DOCS_WORKING_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(DOXYGEN_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile" CACHE PATH "Path for the generated Doxygen documentation")
set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/xml" CACHE PATH "Path for the generated Doxygen documentation")
set(DOXYBOOK2_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/.doxybook/config.json" CACHE PATH "Path for the doxybook config file")
set(DOXYBOOK2_TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.doxybook/templates" CACHE PATH "Path for the doxybook templates")
set(DOXYBOOK2_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/reference" CACHE PATH "Doxybook reference directory")
set(MKDOCS_CONFIG_FILE "${PROJECT_SOURCE_DIR}/mkdocs.yml" CACHE PATH "Documentation output directory")
set(MKDOCS_OUTPUT_DIR "${PROJECT_BINARY_DIR}/docs" CACHE PATH "Documentation output directory")

add_custom_target(
        doxygen

        # Clean previous doxygen directory
        COMMAND "${CMAKE_COMMAND}" -E remove_directory "${DOXYGEN_OUTPUT_DIR}"

        # Run doxygen to generate xml files
        COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYGEN_CONFIG_FILE}"

        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generate Doxygen files"
        VERBATIM
)

add_custom_target(
        doxybook2

        # Run doxybook2 to convert from xml to markdown
        COMMAND "${DOXYBOOK2_EXECUTABLE}"
        --input "${DOXYGEN_OUTPUT_DIR}"
        --output "${DOXYBOOK2_OUTPUT_DIR}"
        --config "${DOXYBOOK2_CONFIG_FILE}"
        --templates "${DOXYBOOK2_TEMPLATES_DIR}"

        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building documentation using Doxygen, Doxybook, and Mkdocs"
        VERBATIM
)
add_dependencies(doxybook2 doxygen)

add_custom_target(
        docs

        # Run mkdocs
        COMMAND "${MKDOCS_EXECUTABLE}" build
        -f "${MKDOCS_CONFIG_FILE}"
        -d "${MKDOCS_OUTPUT_DIR}"

        WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
        COMMENT "Building documentation using Doxygen, Doxybook, and Mkdocs"
        VERBATIM
)
add_dependencies(docs doxybook2)