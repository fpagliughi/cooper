# CMakeLists.txt
#
# src-level CMake build file for the 'cooper' library.
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

include(GenerateExportHeader)

set(SRCS
    actor.cpp
    timer.cpp
    work_thread.cpp
)

# --- Create the targets ---

if(${COOPER_BUILD_SHARED})
    message(STATUS "Creating shared library")

    add_library(cooper-shared SHARED ${SRCS})

    add_library(Cooper::cooper-shared ALIAS cooper-shared)
    add_library(Cooper::cooper ALIAS cooper-shared)

    set_target_properties(cooper-shared PROPERTIES
        OUTPUT_NAME cooper
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )

    generate_export_header(cooper-shared
        BASE_NAME cooper
        EXPORT_FILE_NAME ${COOPER_GENERATED_DIR}/include/cooper/export.h
    )
endif()

if(${COOPER_BUILD_STATIC})
    message(STATUS "Creating static library")

    add_library(cooper-static STATIC ${SRCS})
    add_library(Cooper::cooper-static ALIAS cooper-static)

    if(${COOPER_BUILD_SHARED})
        # This lib should configure for static exports
        target_compile_definitions(cooper-static PRIVATE COOPER_STATIC_DEFINE)
    else()
        add_library(Cooper::cooper ALIAS cooper-static)

        generate_export_header(cooper-static
            BASE_NAME cooper
            EXPORT_FILE_NAME ${COOPER_GENERATED_DIR}/include/cooper/export.h
        )
    endif()

    # On *nix systems, the static library can have the same base filename
    # as the shared library, thus 'libcooper.a' for the static lib.
    # On Windows they need different names to tell the static lib from the
    # DLL import library.
    if(UNIX)
        set_target_properties(cooper-static PROPERTIES OUTPUT_NAME cooper)
    endif()
endif()


foreach(TARGET ${COOPER_TARGETS})
    target_compile_features(${TARGET} PRIVATE cxx_std_17)

    set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    target_compile_options(${TARGET}
        PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/W3>
            $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wdocumentation>
            $<$<NOT:$<OR:$<CXX_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:Clang>>>:-Wall -Wextra -Wpedantic>
        )

    target_include_directories(${TARGET}
        PUBLIC
            $<BUILD_INTERFACE:${COOPER_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${COOPER_GENERATED_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            ${COOPER_GENERATED}
            ${COOPER_SRC_DIR}
    )
endforeach()

