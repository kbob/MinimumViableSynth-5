#!/bin/sh

# Create a unit test `test-foo.h`.

kebab="$(python3 -c "print('$1'.lower().replace('_', '-'))")"
snake="$(python3 -c "print('$kebab'.lower().replace('-', '_'))")"
camel="$(python3 -c "print('$kebab'.title().replace('-', ''))")"

echo "kebab = $kebab"
echo "snake = $snake"
echo "camel = $camel"

testname="test-${kebab}.h"

if [ -f "$testname" ]
then
    echo "Can't create $testname: file exists" >&2
    exit 1
fi

cat > "$testname" <<EOF
#include "$kebab.h"

#include <cxxtest/TestSuite.h>

class ${snake}_unit_test : public CxxTest::TestSuite {

public:

    void test_instantiate()
    {
        (void)${1}();
    }

};
EOF
