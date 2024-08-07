# CMakeLists.txt
#
# Top-level CMake build file for the 'cooper' library.
#
# ---------------------------------------------------------------------------
# This file is part of the "cooper" C++ actor library.
#
# Copyright (c) 2019-2024 Frank Pagliughi
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from this
# software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ---------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)

project(cooper VERSION "0.2.0")

# --- Build options ---

option(COOPER_BUILD_SHARED "Build shared library" ON)
option(COOPER_BUILD_STATIC "Build static library" ON)
option(COOPER_BUILD_EXAMPLES "Build example applications" OFF)
option(COOPER_BUILD_TESTS "Build unit tests" OFF)
option(COOPER_BUILD_DOCUMENTATION "Create Doxygen reference documentation" OFF)

# --- Collect the targets names ---

if(${COOPER_BUILD_SHARED})
    list(APPEND COOPER_TARGETS cooper-shared)
endif()

if(${COOPER_BUILD_STATIC})
    list(APPEND COOPER_TARGETS cooper-static)
endif()

if(NOT COOPER_TARGETS)
    message(FATAL_ERROR "No targets are specified. Select shared or static.")
endif()

# --- directories ---

set(COOPER_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(COOPER_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(COOPER_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# --- Generate a version header ---

if(WIN32)
    include(GenerateExportHeader)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/cmake/version.h.in
    ${COOPER_GENERATED_DIR}/cooper/version.h
)

# --- Source dir creates library target(s)

add_subdirectory(src)

# --- Install Targets ---

include(GNUInstallDirs)

install(TARGETS ${COOPER_TARGETS}
    EXPORT Cooper
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT Cooper
    FILE cooperTargets.cmake
    NAMESPACE Cooper::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cooper
)

install(
    DIRECTORY 
        ${COOPER_INCLUDE_DIR}/
        ${COOPER_GENERATED_DIR}/include/
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

# --- Generate CMake config/version ---

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${COOPER_GENERATED_DIR}/cmake/cooperConfigVersion.cmake
    VERSION ${COOPER_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(
    FILES
        ${PROJECT_SOURCE_DIR}/cmake/cooperConfig.cmake
        ${COOPER_GENERATED_DIR}/cmake/cooperConfigVersion.cmake
    DESTINATION 
        ${CMAKE_INSTALL_LIBDIR}/cmake/cooper
)

# --- Documentation ---

if(COOPER_BUILD_DOCUMENTATION)
    add_subdirectory(doc)
endif()

# --- Examples Applications ---

if(COOPER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Unit Tests ---

if(COOPER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests/unit)
endif()

