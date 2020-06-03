# Code Conventions

These are the guidelines I try to follow.


# General

Code is a liability.  Less code is better code.  Use defaults where
you can.  Define defaults where they make sense.

`#include "zen-of-python.h"`

Always use spaces; never use a TAB character.  (Exception: Makefiles
require TAB characters.)

Short lines are more readable than long.  Do not exceed 80 columns.
In documentation and block comments, do not exceed 72 columns.


# File Hierarchy

(See top-level README.)


# File Names

File names are `lower-kebab-case`.  That's the easiest to type.
Exceptions:

 * multi-word Python modules use `lower_snake_case`.  Python doesn't
   allow kebab-case identifiers.

 * Makefiles are called `Makefile`.  That keeps them at the top of
   directory listings.

Both C and C++ include files have a `.h` extension.

C++ source files have a `.cpp` extension.

If a C++ source or header file defines a single class, call it
`thing.h` (singular).  If it defines the root or several parts of a
class hierarchy, call it `things.h` (plural).


# C and C++ Headers

The first three lines and the last two lines are guard lines.  They
look like this.

    #ifndef FILE_NAME_included
    #define FILE_NAME_included

    // (your code here)

    # endif /* !FILE_NAME_included */

If the file name is not unique across the project, prepend as many
directory names as needed to make it unique.  E.g., we have
`CORE_CONTROLS_included` and `MIDI_CONTROLS_included`.

If the file mostly contains namespaced definitions, then the guard
should be `NAMESPACE_NAME_FILE_NAME_included`.

Use a C-style comment on the last line -- C++ compilers have existed
that use a pre-C99 preprocessor that doesn't recognize `//` comments.
Ditto for comments on other preprocessor lines.

If a file can meaningfully be included as both C and C++, wrap the
C++-only parts in `#ifdef __cplusplus` and declare the C parts with
`extern "C"`.  Otherwise don't bother.

Each header should include the headers that define the names it
uses.  It should always be possible to `#include "foo.h"` without
first including foo.h's dependencies.  Header should not include
names they don't directly use.

Include directives come in this order.  Each group is separated by
a blank line.

 * foo.cpp and test-foo.cpp include foo.h on the very first line.
   That tests whether foo.h has any unresolved dependencies.

 * Language and POSIX headers next.  files in <sys/> follow
   files not in <sys/>

 * Additional groups of files in decreasing distance order.
   (e.g. `external-library/lib.h` comes before
   `../../other-subsystem/module/foo.h` comes before
   `../other-module/bar.h` comes before `./baz.h`.)

Most classes are completely in-line in the header file.


# C++ Source

Use C++11.  Try to avoid extensions.

No allocation except during initialization.  Everything allocated must
be deallocated.

I'm still working out whether class names are `CamelCase` or
`snake_case`.  I use both.  CamelCase classes are somehow higher-level
or less generic than snake_case classes.  But I don't know what the
rule is yet.  **Update 2020-06-03:**  I think I'm using CamelCase for
objects with domain meaning (*e.g.*, `Voice`) and snake_case for objects
with software meaning only (*e.g.*, ) `fixed_queue`.

If a name contains an acronym, use mixed_SNAKE_case: `TCP_socket`.

Prefer postfix `p++` to prefix `++p`; the PDP-11 has no pre-increment
addressing mode.  Similarly, prefer `--p` to `p--`.  (I know this is
stupid.  The PDP-11 has a special place in my heart.)

I am putting off using namespaces.  Eventually, there should probably
be roughly one namespace per directory.  Namespace names are snake_case.
At this time, the only namespace is `midi`.  Short namespace names
are very important.


# Unit Tests

I am gradually converging toward 100% unit test coverage.  Tests use
`CxxTest`.  Unit test code should use terse names -- that makes it
easy to distinguish the test from the testee.

Test suite class names are `that_thing_unit_test`.  The class under
test is either `that_thing` or `ThatThing`.


# Build System

Assume GNU Make and use its features.

Type `make help` in any directory to see what's available.

There is a `make cloc` target to count lines of code.  Run
it at top level to measure the whole project.

TBD: use clang code coverage to see how much the unit tests cover.

TBD: extend the build system to create and flash firmware images.
