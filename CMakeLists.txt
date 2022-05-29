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

cmake_minimum_required(VERSION 3.10)

project(cooper VERSION "0.1.2")

# library name
set(COOPER cooper)
set(COOPER_STATIC_LIB cooper-static)
set(COOPER_OBJ_LIB cooper-objs)
set(COOPER_INCLUDES ${CMAKE_SOURCE_DIR}/include)
set(COOPER_SRC ${CMAKE_SOURCE_DIR}/src)
set(COOPER_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)

# --- build options ---

option(COOPER_BUILD_SHARED "Build shared library" ON)
option(COOPER_BUILD_STATIC "Build static library" OFF)
option(COOPER_BUILD_EXAMPLES "Build example applications" OFF)
option(COOPER_BUILD_TESTS "Build unit tests" OFF)
option(COOPER_BUILD_DOCUMENTATION "Create Doxygen reference documentation" OFF)

# --- C++17 build flags ---

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(WIN32)
  include(GenerateExportHeader)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# --- Generate a version header ---

configure_file(
  ${CMAKE_SOURCE_DIR}/cmake/version.h.in
  ${COOPER_GENERATED_DIR}/cooper/version.h
)

# --- Common library sources, etc ---

add_subdirectory(src)

# --- System libraries ---

if(WIN32)
	set(LIBS_SYSTEM ws2_32)
elseif(UNIX)
	set(LIBS_SYSTEM c stdc++)
endif()


# --- Collect the targets ---

if(COOPER_BUILD_SHARED)
	message(STATUS "Creating shared library: ${COOPER}")
  list(APPEND COOPER_TARGETS ${COOPER})
endif()

if(COOPER_BUILD_STATIC)
  message(STATUS "Creating static library: ${COOPER_STATIC_LIB}")
  list(APPEND COOPER_TARGETS ${COOPER_STATIC_LIB})
endif()


include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/cooper)

if(NOT COOPER_TARGETS)
  message(FATAL_ERROR "No build targets specified.")
endif()

target_include_directories(${COOPER_OBJ_LIB}
  PUBLIC
    $<BUILD_INTERFACE:${COOPER_INCLUDES}>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${COOPER_GENERATED}
    ${COOPER_SRC}
)

if (COOPER_BUILD_SHARED)
  add_library(${COOPER} SHARED $<TARGET_OBJECTS:${COOPER_OBJ_LIB}>)

  ## add dependencies to the shared library
  target_link_libraries(${COOPER} ${LIBS_SYSTEM})

  target_include_directories(${COOPER}
    PUBLIC 
      $<BUILD_INTERFACE:${COOPER_INCLUDES}>
      $<INSTALL_INTERFACE:include>
    PRIVATE
      ${COOPER_GENERATED}
)

  set_target_properties(${COOPER} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )

  # Add an alias so that library can be used inside the build tree
  add_library(Cooper::${COOPER} ALIAS ${COOPER})

  list(APPEND TARGET_FILES ${COOPER})
endif()

if(COOPER_BUILD_STATIC)
  add_library(${COOPER_STATIC_LIB} STATIC $<TARGET_OBJECTS:${COOPER_OBJ_LIB}>)

  # add dependencies to the static library
  target_link_libraries(${COOPER_STATIC_LIB} ${LIBS_SYSTEM})

  target_include_directories(${COOPER_STATIC_LIB}
    PUBLIC 
      $<BUILD_INTERFACE:${COOPER_INCLUDES}>
      $<INSTALL_INTERFACE:include>
    PRIVATE
      ${COOPER_GENERATED}
  )

  # Let the archive use the same name as the shared library
  if(UNIX)
    set_target_properties(${COOPER_STATIC_LIB} PROPERTIES OUTPUT_NAME ${COOPER})
  endif()

  # Add an alias so that library can be used inside the build tree
  add_library(Cooper::${COOPER_STATIC_LIB} ALIAS ${COOPER_STATIC_LIB})

  list(APPEND TARGET_FILES ${COOPER_STATIC_LIB})
endif()

install(TARGETS ${TARGET_FILES}
  EXPORT cooper-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

export(TARGETS ${COOPER_TARGETS} ${COOPER_OBJ_LIB}
  NAMESPACE Cooper::
  FILE "${CMAKE_BINARY_DIR}/CooperTargets.cmake"
)

install(EXPORT cooper-targets
  NAMESPACE Cooper::
  DESTINATION ${INSTALL_CONFIGDIR}
)

install(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
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

