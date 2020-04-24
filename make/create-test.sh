#!/bin/sh

# Create a unit test `test-foo.h`.

testname="test-$1.h"

if [ -f "$testname" ]
then
    echo "Can't create $testname: file exists" >&2
    exit 1
fi

camel="$(python3 -c "print('$1'.title().replace('-', ''))")"

cat > "$testname" <<EOF
#include "$1.h"

#include <cxxtest/TestSuite.h>

class ${1}_unit_test : public CxxTest::TestSuite {

public:

    void test_x()
    {

    }

};
EOF
