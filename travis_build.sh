#!/bin/bash
#
# travis_build.sh
#
# Travis CI build/test script for the "cooper" C++ actor library.
#

set -e

echo "travis build dir $TRAVIS_BUILD_DIR pwd $PWD"
cmake -Bbuild -H. -DCOOPER_BUILD_EXAMPLES=ON -DCOOPER_BUILD_TESTS=ON
VERBOSE=1 cmake --build build/

# Run the unit tests
./build/tests/unit/unit_tests --success

#ctest -VV --timeout 600
#cpack --verbose

