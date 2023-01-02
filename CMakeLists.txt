# CMakeLists.txt
#
# Top-level CMake build file for the 'cooper' library.
#
# ---------------------------------------------------------------------------
# This file is part of the "cooper" C++ actor library.
#
# Copyright (c) 2019-2022 Frank Pagliughi
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

project(cooper VERSION "0.1.2")

# library name
set(COOPER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COOPER_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(COOPER_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# --- build options ---

option(BUILD_SHARED_LIBS "Build shared library" ON)
option(COOPER_BUILD_EXAMPLES "Build example applications" OFF)
option(COOPER_BUILD_TESTS "Build unit tests" OFF)
option(COOPER_BUILD_DOCUMENTATION "Create Doxygen reference documentation" OFF)

if(WIN32)
  include(GenerateExportHeader)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# --- Generate a version header ---

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in
  ${COOPER_GENERATED_DIR}/cooper/version.h
)

# --- Collect the targets ---


add_library(cooper
  ${COOPER_SRC_DIR}/actor.cpp
	${COOPER_SRC_DIR}/timer.cpp
	${COOPER_SRC_DIR}/work_thread.cpp
)

add_library(Cooper::cooper ALIAS cooper)

target_compile_features(cooper PUBLIC cxx_std_17)

target_compile_options(cooper
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

target_include_directories(cooper
  PUBLIC
  $<BUILD_INTERFACE:${COOPER_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${COOPER_GENERATED}
    ${COOPER_SRC_DIR}
)

if (BUILD_SHARED_LIBS)
  message(STATUS "Creating shared library")

  set_target_properties(cooper PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
else()
  message(STATUS "Creating static library")

  # Windows archive file renamed 'cooper-static'
  if(WIN32)
    set_target_properties(cooper PROPERTIES OUTPUT_NAME cooper-static)
  endif()
endif()

set_target_properties(cooper PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# --- Install Targets ---

include(GNUInstallDirs)

install(TARGETS cooper
  EXPORT cooper-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY include/ ${COOPER_GENERATED_DIR}/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT cooper-targets
  FILE cooperTargets.cmake
  NAMESPACE Cooper::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cooper
)

export(EXPORT cooper-targets
  NAMESPACE Cooper::
  FILE ${CMAKE_BINARY_DIR}/cmake/cooperTargets.cmake
)

# --- Generate CMake config/version ---

include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cooperConfig.cmake.in
  ${CMAKE_BINARY_DIR}/cmake/cooperConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cooper
)

write_basic_package_version_file(
  ${CMAKE_BINARY_DIR}/cmake/cooperConfigVersion.cmake
  VERSION ${COOPER_VERSION}
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
    ${CMAKE_BINARY_DIR}/cmake/cooperConfig.cmake
    ${CMAKE_BINARY_DIR}/cmake/cooperConfigVersion.cmake
  DESTINATION 
    ${CMAKE_INSTALL_LIBDIR}/cmake/cooper
)

# --- Documentation ---

if(COOPER_BUILD_DOCUMENTATION)
	add_subdirectory(doc)
endif()

# --- Default library for examples and unit tests ---

if(COOPER_BUILD_SHARED)
	set(COOPER_LIB ${COOPER})
else()
	set(COOPER_LIB ${COOPER_STATIC_LIB})
endif()

# --- Examples Applications ---

if(COOPER_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# --- Unit Tests ---

if(COOPER_BUILD_TESTS)
	add_subdirectory(tests/unit)
endif()

