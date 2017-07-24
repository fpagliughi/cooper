#!/bin/bash
#
# devenv.sh
#
# Set up a Linux development environment for cooper

# Set the top-level project directory the current dir,
# unless one specified on the command line
if [ -n "$1" ]; then
    export COOPER_DIR=$1
else
    export COOPER_DIR=$PWD
fi

# The library path during development
COOPER_LIB_DIR=${COOPER_DIR}/lib

# Add the cooper shared libraies to the loader search path
export LD_LIBRARY_PATH=${COOPER_LIB_DIR}:${LD_LIBRARY_PATH}

