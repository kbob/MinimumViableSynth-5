#!/usr/bin/env python3

"""Create a unit test `test-foo.h`."""


from collections import namedtuple
import os
import re
import sys


test_template = '''

#include "{kebab}.h"

#include <cxxtest/TestSuite.h>

class {snake}_unit_test : public CxxTest::TestSuite {{

public:

    void test_instantiate()
    {{
        (void){orig}();
    }}

}};
'''.lstrip()

Canon = namedtuple('Canon', 'orig kebab snake camel pascal '
                            'hot_kebab screaming_snake')

def canonicalize(name):
    orig = name
    unqual = name.split('::')[-1]
    separated = re.sub(r'(?<=[a-z])(?=[A-Z])', '-', unqual)
    kebab = re.sub(r'[_ ]', '-', separated).lower()
    snake = kebab.replace('-', '_')
    hot_kebab = kebab.upper()
    screaming_snake = snake.upper()
    camel = re.sub(r'-([a-z])', lambda m: m.group(1).upper(), kebab)
    pascal = camel[0].upper() + camel[1:]
    return Canon(orig=orig,
                 kebab=kebab,
                 snake=snake,
                 camel=camel,
                 pascal=pascal,
                 hot_kebab=hot_kebab,
                 screaming_snake=screaming_snake)

# print(canonicalize('Foo-Bar'))
# print(canonicalize('TCP_Header'))

test_names = [
    'foo bar',
    'Foo Bar',
    'FOO BAR',
    'foo-bar',
    'foo_bar',
    'fooBar',
    'foo_Bar',
    'foo-Bar',
    'FooBar',
    'Foo_Bar',
    'Foo-Bar',
    'FOO_BAR',
    'FOO-BAR',
    'ns::Foo-Bar',
]

for name in test_names:
    expected = Canon(orig=name,
                     kebab='foo-bar',
                     snake='foo_bar',
                     camel='fooBar',
                     pascal='FooBar',
                     hot_kebab='FOO-BAR',
                     screaming_snake='FOO_BAR')
    actual = canonicalize(name)
    assert actual == expected, f'expected {expected}, got {actual}'


def create_test(name):
    canon = canonicalize(name)
    filename = f'test-{canon.kebab}.h'
    contents = test_template.format(**canon._asdict())
    # print(f'{filename=}')
    # print(contents)
    # exit()
    try:
        with open(filename, 'xt') as f:
            f.write(contents)
    except FileExistsError as x:
        exit(x)


def main(argv):
    for arg in argv:
        create_test(arg)

if __name__ == '__main__':
    main(sys.argv[1:])
