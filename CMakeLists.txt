# CMakeLists.txt
#
# Top-level CMake build file for the 'cooper' library.
#
# ---------------------------------------------------------------------------
# This file is part of the "cooper" C++ socket library.
#
# Copyright (c) 2019 Frank Pagliughi
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

## CMake required version
cmake_minimum_required(VERSION 3.8)

## project
project(cooper VERSION "0.1.0")

## library name
set(COOPER cooper)
set(COOPER_STATIC ${COOPER}-static)

set(CMAKE_BUILD_TYPE Release)

# --- build options ---

if(WIN32)
	option(COOPER_BUILD_STATIC "Build static library" ON)
	option(COOPER_BUILD_SHARED "Build shared library (DLL)" OFF)
else()
	option(COOPER_BUILD_STATIC "Build static library" OFF)
	option(COOPER_BUILD_SHARED "Build shared library" ON)
endif()

option(COOPER_BUILD_EXAMPLES "Build example applications" OFF)
option(COOPER_BUILD_TESTS "Build unit tests" OFF)
option(COOPER_BUILD_DOCUMENTATION "Create Doxygen reference documentation" OFF)

# --- C++14 build flags ---

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Common Library Sources, etc ---

add_subdirectory(src)

# --- Header Locations ---

target_include_directories(cooper-objs 
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
	PRIVATE 
		src
)

# --- System libraries ---

include(GNUInstallDirs)

if(WIN32)
	set(LIBS_SYSTEM ws2_32)
elseif(UNIX)
	set(LIBS_SYSTEM c stdc++)
endif()


## --- create the shared library ---

if(COOPER_BUILD_SHARED)
	message(STATUS "Creating shared library: ${COOPER}")

	add_library(${COOPER} SHARED $<TARGET_OBJECTS:cooper-objs>)

    ## add dependencies to the shared library
    target_link_libraries(${COOPER} ${LIBS_SYSTEM})

	target_include_directories(${COOPER} PUBLIC 
		$<TARGET_PROPERTY:cooper-objs,INCLUDE_DIRECTORIES>
	)

	set_target_properties(${COOPER} PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR})

	install(TARGETS ${COOPER}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
		RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()

# --- Create the static library ---

if(COOPER_BUILD_STATIC)
	message(STATUS "Creating static library: ${COOPER_STATIC}")

	add_library(${COOPER_STATIC} STATIC $<TARGET_OBJECTS:cooper-objs>)

    ## add dependencies to the static library
    target_link_libraries(${COOPER_STATIC} ${LIBS_SYSTEM})

	target_include_directories(${COOPER_STATIC} PUBLIC 
		$<TARGET_PROPERTY:cooper-objs,INCLUDE_DIRECTORIES>
	)

	install(TARGETS ${COOPER}-static
		ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

	# Let the archive use the same name as the shared library
	if(UNIX)
		set_target_properties(${COOPER_STATIC} PROPERTIES OUTPUT_NAME ${COOPER})
	endif()
endif()

# --- Install the library ---

install(DIRECTORY include/${COOPER}/
	DESTINATION include/${COOPER}
	FILES_MATCHING PATTERN "*.h*")

# --- Documentation ---

if(COOPER_BUILD_DOCUMENTATION)
	add_subdirectory(doc)
endif()

# --- Default library for examples and unit tests ---

if(COOPER_BUILD_SHARED)
	set(COOPER_LIB ${COOPER})
else()
	set(COOPER_LIB ${COOPER_STATIC})
endif()

# --- Examples Applications ---

if(COOPER_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

# --- Unit Tests ---

#if(COOPER_BUILD_TESTS)
#	add_subdirectory(tests/unit)
#endif()

