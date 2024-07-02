# cooperConfig.cmake
#
# The following import target is always created:
#
#   Cooper::cooper
#
# This is for whichever target is build (shared or static). 
# If both are built it is for the shared target.
#
# One or both of the following import targets will be created depending
# on the configuration:
#
#   Cooper::cooper-shared
#   Cooper::cooper-static
#

include(CMakeFindDependencyMacro)

if(NOT TARGET Cooper::cooper-shared AND NOT TARGET Cooper::cooper-static)
    include(${CMAKE_CURRENT_LIST_DIR}/cooperTargets.cmake)

    if(TARGET Cooper::cooper-shared)
        add_library(Cooper::cooper ALIAS Cooper::cooper-shared)
    else()
        add_library(Cooper::cooper ALIAS Cooper::cooper-static)
    endif()
endif()
