#!/usr/bin/env python3

# Analyze a set of C++ headers, sources, and test suites.
# Calculate dependencies, compare to the #include dependencies.
# Print a list of differences.
#
# This tool is approximate.  It does not actually parse C++ source;
# it just looks for specific regular expression patterns to indicate
# uses and definitions.
#
# The function, `apply_heuristics`, has special rules for test suites
# and sources.

from itertools import groupby
from pathlib import PurePath
import re
import sys
from types import SimpleNamespace

findall_define = re.compile(r'^\s*#\s*define\s+(\w+)', re.MULTILINE).findall
findall_typedef = re.compile(r'^typedef [^;]* (\w+)\s*;', re.MULTILINE).findall
findall_using = re.compile(r'^using\s+(\w+)\s*=', re.MULTILINE).findall
findall_class = re.compile(r'^class\s+(\w+)\s*[:{]', re.MULTILINE).findall
findall_forward_class = re.compile(r'^class\s+(\w+)\s*;', re.MULTILINE).findall

findall_std = re.compile(r'\bstd::\w+').findall
findall_assert = re.compile(r'\bassert\b').findall

findall_words = re.compile(r'\b\w+\b').findall

inc = r'.*#\s*include.*'
search_headers = re.compile(fr'(.*{inc}(?:(?:.|\n)*{inc})?\n)').search

# map names in std:: to the files that declare them.
std_names_to_files = {
    'aligned_storage': 'type_traits',
    'array': 'array',
    'assert': 'cassert',
    'back_insert_iterator': 'iterator',
    'back_inserter': 'iterator',
    'bitset': 'bitset',
    'copy': 'algorithm',
    'cout': 'iostream',
    'deque': 'deque',
    'distance': 'iterator',
    'enable_if': 'type_traits',
    'endl': 'iostream',
    'find': 'algorithm',
    'forward_iterator_tag': 'iterator',
    'function': 'functional',
    'hex': 'iostream',
    'initializer_list': 'initializer_list',
    'input_iterator_tag': 'iterator',
    'int8_t': 'cstddef',
    'is_base_of': 'type_traits',
    'is_constructible': 'type_traits',
    'iterator': 'iterator',
    'iterator_traits': 'iterator',
    'length_error': 'stdexcept',
    'list': 'list',
    'logic_error': 'stdexcept',
    'move': 'algorithm',
    'nullptr_t': 'cstddef',
    'numeric_limits': 'limits',
    'ostream': 'iostream',
    'ostream_iterator': 'iterator',
    'ostringstream': 'sstream',
    'out_of_range': 'stdexcept',
    'pair': 'utility',
    'reverse_iterator': 'iterator',
    'runtime_error': 'stdexcept',
    'string': 'string',
    'stringstream': 'sstream',
    'swap': 'utility',
    'to_string': 'string',
    'type_index': 'typeindex',
    'uint8_t': 'cstdint',
    'vector': 'vector',
}

std_files_to_names = {k: [p[0] for p in v]
                      for (k, v) in groupby(sorted(std_names_to_files.items(),
                                                   key=lambda p: p[::-1]),
                                            key=lambda p: p[1])}


class IncludeList(SimpleNamespace):

    def field_key(self, field):
        return {
            'self': 1,
            'system': 2,
            'framework': 3,
            'local': 4,
        }[field]

    def __repr__(self):
        keys = sorted(self.__dict__, key=self.field_key)
        items = ("{}={!r}".format(k, self.__dict__[k]) for k in keys)
        return "{}({})".format(type(self).__name__, ", ".join(items))

    def statements(self):
        stmts = ''
        separator = ''
        for group in sorted(self.__dict__, key=self.field_key):
            files = sorted(self.__dict__[group])
            if files:
                stmts += separator
                separator = '\n'
                for file in files:
                    if group in ('framework', 'system'):
                        stmts += f'#include <{file}>\n'
                    else:
                        stmts += f'#include "{file}"\n'
        return stmts


def indent(text, amount=4):
    spaces = ' ' * amount
    return re.sub(r'^', spaces, text, flags=re.MULTILINE)

class FileInfo(SimpleNamespace):

    def is_test_suite(self):
        name = self.path.name
        return name.startswith('test-')and name.endswith('.h')

    def is_cpp_file(self):
        return self.path.suffix in {'.c++', '.cxx', '.cc', '.cpp'}

    def make_uut(self):
        assert self.is_test_suite()
        assert self.path.name.startswith('test-')
        uut_name = self.path.name[len('test-'):]
        return self.path.with_name(uut_name)

    def set_uses(self, uses):
        self.uses = uses

    def set_expected_includes(self, ei):
        self.expected_includes = ei


def check_contents(path, contents):
    for line in contents.split('\n'):
        if line == '':
            continue
        if line[0] in ' #{}':
            continue
        if line.startswith('//'):
            continue
        if line.startswith('class '):
            continue
        if line.startswith('namespace '):
            continue
        if line.startswith('template '):
            continue
        if line.startswith('typedef '):
            continue
        if line.startswith('using '):
            continue
        if any(line.startswith(a)
               for a in ('public:', 'protected:', 'private:')):
            continue
        print(f'{path}: {line}')


def find_declarations(contents):
    defines = findall_define(contents)
    usings = findall_using(contents)
    typedefs = findall_typedef(contents)
    classes = findall_class(contents)
    return set(defines + usings + typedefs + classes)


def find_std_uses(contents):
    return sorted(set(findall_std(contents) + findall_assert(contents)))


def find_headers(contents):
    m = search_headers(contents)
    if m:
        return m.group(0)
    return ''

def find_forward_declarations(contents):
    classes = findall_forward_class(contents);
    return set(classes);

def process_file(path):
    with open(path) as f:
        contents = f.read()
    # check_contents(path, contents)
    fi = FileInfo(path=PurePath(path))
    fi.headers = find_headers(contents)
    if fi.is_test_suite():
        fi.declarations = set()
    else:
        fi.declarations = find_declarations(contents)
    fi.forward_declarations = find_forward_declarations(contents)
    fi.std_uses = find_std_uses(contents)
    return fi


def collect_global_decls(info):
    return {decl: fi.path for fi in info for decl in fi.declarations}


def process_uses(fi, global_decls):
    nonlocal_decls = {dcl: path
                      for (dcl, path) in global_decls.items()
                      if path != fi.path}
    with open(fi.path) as f:
        contents = f.read()
    words = set(findall_words(contents))
    uses = set()
    for (decl, path) in nonlocal_decls.items():
        if decl.endswith('_unit_test'):
            continue
        if decl in words:
            uses.add(decl)
    fi.uses = uses


def apply_heuristics(fi, info_by_path):
    if fi.is_test_suite():
        # test suite:
        #   - self is unit-under-test.h (aka uut.h)
        #   - add "cxxtest/TestSuite.h" as separate category
        #   - remove headers included by uut.h
        #   - remove path/to/uut.h from locals
        uut = fi.make_uut()
        uut_info = info_by_path[uut]
        uut_system = uut_info.expected_includes.system
        uut_local = uut_info.expected_includes.local
        incs = fi.expected_includes
        incs.self = {uut.name}
        incs.framework = {'cxxtest/TestSuite.h'}
        incs.system = {h for h in incs.system if h not in uut_system}
        incs.local = {h for h in incs.local if h not in uut_local}
        incs.local.discard(uut)
    if fi.is_cpp_file():
        # C++ source:
        #   - self is 'self.h'
        #   - exclude headers included by self.h
        #   = exclude self.h
        self_h = fi.path.with_suffix('.h')
        h_info = info_by_path[self_h]
        h_system = h_info.expected_includes.system
        h_local = h_info.expected_includes.local
        incs = fi.expected_includes
        incs.self = {self_h}
        incs.system = {h for h in incs.system if h not in h_system}
        incs.local = {h for h in incs.local if h not in h_local}
        incs.local.discard(self_h)


def process_expected_includes(fi, global_decls):
    name_to_file = lambda u: std_names_to_files[u.replace('std::', '')]
    sys_inc = {name_to_file(u) for u in fi.std_uses}
    local_inc = {global_decls[u] for u in fi.uses - fi.forward_declarations}
    fi.set_expected_includes(IncludeList(system=sys_inc, local=local_inc))


def print_differences(info_by_path):
    for (path, fi) in info_by_path.items():
        actual = fi.headers
        expected = fi.expected_includes.statements()
        if actual == expected:
            print(f'{path}: OK')
        else:
            print('================')
            print()
            print(f'{path}: different')
            print()
            print(actual)
            print('---')
            print(expected)
            print('================')


def main(argv):
    info = [process_file(path) for path in argv]
    info_by_path = {fi.path: fi for fi in info}
    global_decls = collect_global_decls(info)
    for fi in info:
        process_uses(fi, global_decls)
        process_expected_includes(fi, global_decls)
    for fi in info:
        apply_heuristics(fi, info_by_path)
    print_differences(info_by_path)

if __name__ == '__main__':
    exit(main(sys.argv[1:]))
