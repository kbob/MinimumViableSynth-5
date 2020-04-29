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

File names are `lower-kebob-case`.  That's the easiest to type.
Exceptions:

 * multi-word Python modules use `lower_snake_case`.  Python doesn't
   allow kebob-case identifiers.

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

    #ifndef FILE_NAME_included define FILE_NAME_included

    // (your code here)
    
    # endif /* !FILE_NAME_included */

If the file name is not unique across the project, prepend as many
directory names as needed to make it unique.  E.g., we have
`CORE_CONTROLS_included` and `MIDI_CONTROLS_included`.

Use a C-style comment on the last line -- C++ compilers have existed
that use a pre-C99 preprocessor that doesn't recognize `//` comments.
Ditto for comments on other preprocessor lines.

If a file can meaningfully be included as both C and C++, wrap the
C++-only parts in `#ifdef __cplusplus` and declare the C parts with
`extern "C"`.  Otherwise don't bother.

Most classes are completely in-line in the header file.


# C++ Source

Use C++11.  Try to avoid extensions.

No allocation except during the initialization of the `Synth`
object(s).  Everything allocated must be deallocated.

I'm still working out whether class names are `CamelCase` or
`snake_case`.  I use both.  CamelCase classes are somehow higher-level
or less generic than snake_case classes.  But I don't know what the
rule is yet.

Prefer postfix `p++` to prefix `++p`; the PDP-11 has no pre-increment
addressing mode.  Similarly, prefer `--p` to `p--`.

I am putting off using namespaces.  Eventually, there should probably
be roughly one namespace per directory.

# Unit Tests

I am gradually converging toward 100% unit test coverage.  Tests use
`CxxTest`.  Unit test code should use terse names -- that makes it
easy to distinguish the test from the testee.

Test suite class names are `that_thing_unit_test`.  The class under
test is either `that_thing` or `ThatThing`.

# Build System

Assume GNU Make and use its features.

TBD: invoke clang code coverage and the cloc tool.  TBD: extend the
build system to create and flash firmware images.
