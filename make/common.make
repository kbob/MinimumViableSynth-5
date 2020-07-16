# Common Makefile definitions.  See the `help` target below for usage.
#
#
# Each directory's Makefile should define these variables if applicable.
#
#    TESTS    - unit tests to build and run.
#    PROGRAMS - standalone programs to build
#    IMAGES   - firmware images to build
#    SUBDIRS  - subdirectories to visit
#    FILTH    - generated files "make clean" should remove
#
#
# The Makefile in the project root may also define
#
#    SUBMODULES - list of git submodules - make checks that they
#                 are initialized.
#
#
# Test names must start with "test-".  Each test `test-foo` uses
# `test-foo.h` to build a CxxTest test runner.  We generate
# `test-foo.cpp` with cxxtestgen, then build it and run it.
#
#     test-foo.h   - the unit test source
#     test-foo.cpp - autogenerated unit test runner source
#     test-foo:    - test runner executable
#     run-test-foo - phony target that builds, then runs, the test.
#
#
# Optional variables:
#
#    test-foo-SOURCES - list of additional sources to be built into
#                       the `test-foo` executable
#
#
# For each program `foo`, the Makefile should define
#
#    foo-SOURCES - list of source files comprising foo.
#                  They will be compiled with CC or CXX, depending
#                  on suffix, then linked into an executable `foo`.
#
#
# Firmware images' variables are TBD.
#
#
# Advanced Trickery
#
#   Starting a new or experimental unit test?  Start running the test
#   without changing the makefile.
#
#     $ make TESTS=test-new-thing
#
#   SOURCES is magic.  If you define SOURCES on the command line,
#   we build a.out from SOURCES and run it.
#
#     $ make SOURCES='foo.c bar.cpp'
#
#   No makefile?  Use this file directly.
#
#     $ make -f ../make/common.make PROGRAMS=foo foo-SOURCES=foo.c
#     $ make -f ../make/common.make SOURCES=foo.c


# Derive project root as this file's grandparent.
r := $(lastword $(MAKEFILE_LIST))
r := $(r:make/common.make=)
r := $(r:/=)

# invoke make with BUILD=release for release build.
    debug_OPT := -O0 -fsanitize=address,bounds,undefined -g
  release_OPT := -O3 -DNDEBUG -flto
        BUILD := debug
          OPT := $($(BUILD)_OPT)
  TARGET_ARCH := -march=native
     CPPFLAGS += -I../.. -MMD $(EXTRA_CPPFLAGS)
       CFLAGS := -std=c99 -Wall -Wextra -Werror $(OPT)
     CXXFLAGS := -std=c++11 -Wall -Wextra -Werror $(OPT)

       HOSTCC := cc
      HOSTCXX := c++
      CXXTEST := $r/submodules/cxxtest
      TESTGEN := $(CXXTEST)/bin/cxxtestgen
 TESTGENFLAGS := --have-eh --error-printer
     TEST_INC := $(CXXTEST)
         JOBS := -j

ifneq ($(SOURCES),)
 # Hack for SOURCES -> a.out

 .DEFAULT_GOAL := run-a.out
      PROGRAMS := a.out
 a.out-SOURCES := $(SOURCES)

 run-a.out: a.out
	    ./a.out
endif

all:        $(SUBDIRS:%=%/all) $(TESTS) run-tests $(PROGRAMS) $(IMAGES)
ifeq ($r,)
world:      all
clean-world: clean
else
world:
	    make -C $r world
clean-world:
	    make -C $r clean
endif
programs:   $(SUBDIRS:%=%/programs) $(PROGRAMS)
images:     $(SUBDIRS:%=%/images) $(IMAGES)
test:       $(SUBDIRS:%=%/test) $(TESTS) run-tests
tests:      $(SUBDIRS:%=%/tests) $(TESTS)
clean:      $(SUBDIRS:%=%/clean)
	    rm -f *.d *.o a.out test-*.cpp $(PROGRAMS) $(TESTS) $(FILTH)
	    rm -rf *.dSYM/

pre-commit-check:
	    make clean-world
	    make $(JOBS) world BUILD=debug
	    make clean-world
	    make $(JOBS) world BUILD=release

# Try EXTRA_CLOCFLAGS=--ignore=/dev/stdout
    CLOCFLAGS := --vcs=git --force-lang=make,make                       \
                 --exclude-dir=experiments $(EXTRA_CLOCFLAGS)

cloc:
	    @cloc $(CLOCFLAGS)

help: general-help local-help

general-help:
	    @echo ''
	    @echo 'Common Targets'
	    @echo ''
	    @echo '    all              - build programs and images, run tests'
	    @echo '    world            - build whole project, run all tests'
	    @echo '    programs         - build all programs'
	    @echo '    images           - build all firmware images'
	    @echo '    test             - build and run all tests'
	    @echo '    run-tests        - build and run tests in this directory'
	    @echo '    tests            - build all test programs'
	    @echo '    clean            - remove all generated files'
	    @echo '    clean-world      - clean whole project'
	    @echo '    pre-commit-check - check world and tests'
	    @echo '    cloc             - count lines of code'
	    @echo '    help             - print this text'
	    @echo ''
	    @echo 'Common Variables'
	    @echo ''
	    @echo '    BUILD=debug      - use "BUILD=release" for release build'
	    @echo '    EXTRA_CPPFLAGS=  - add flags to cc and c++'
	    @echo '    TESTFLAGS=       - add flags to test runs'
	    @echo ''

local-help:
ifneq ($(PROGRAMS),)
	    @echo 'Programs in this directory'
	    @echo ''
	    @for p in $(PROGRAMS); do echo "    $$p"; done
	    @echo ''
endif
ifneq ($(IMAGES),)
	    @echo 'Firmware images in this directory'
	    @echo ''
	    @for i in $(IMAGES); do echo "    $$i"; done
	    @echo ''
endif
ifneq ($(TESTS),)
	    @echo 'Tests in this directory'
	    @echo ''
	    @echo '  Build:'
	    @echo $(TESTS) | tr \\40 \\12 | \
	        column -c 72 | expand  | sed 's,^,    ,'
	    @echo ''
	    @echo '  Run:'
	    @for t in $(TESTS); do echo "run-$$t"; done | \
	        column -c 72 | expand | sed 's,^,    ,'
	    @echo ''
endif

.PHONY:     all world programs images test tests clean clean-world cloc
.PHONY:     help general-help local-help

# Recurse into subdirectories.

R_ACTIONS := all programs images test tests clean
define recur_template
 $$(SUBDIRS:=/$1):
	    make -C $$(@:/$(1)=) $1
 .PHONY:    $$(SUBDIRS:=/$1)
endef
$(foreach a, $(R_ACTIONS), $(eval $(call recur_template,$a)))

# Build program from $(SOURCES)

define program_template
       $1-CFILES := $(filter %.c, $($1-SOURCES))
     $1-CXXFILES := $(filter %.cpp, $($1-SOURCES))
       $1-OFILES := $$($1-CFILES:.c=.o) $$($1-CXXFILES:.cpp=.o)
          DFILES += $$($1-OFILES:.o=.d)

$1:	    $$($1-OFILES)
	    $(LINK.cpp) $$^ $(LOADLIBS) $(LDLIBS) -o $$@
endef
$(foreach p, $(PROGRAMS), $(eval $(call program_template,$p)))

# Build and run test runner from test-foo.h.

  RUN_TESTS := $(TESTS:%=run-%)

run-tests:  $(RUN_TESTS)
run-test-%: test-%
	    ./$< ${TESTFLAGS}

define test_template =
       $1-CFILES := $(filter %.c, $$($1-SOURCES))
     $1-CXXFILES := $1.cpp $(filter %.cpp, $$($1-SOURCES))
     $1-CXXFILES := $1.cpp $$($1-SOURCES)
       $1-OFILES := $$($1-CFILES:.c=.o) $$($1-CXXFILES:.cpp=.o)
          DFILES += $$($1-OFILES:.o=.d)

$1.cpp:     $1.h
	    $(TESTGEN) $(TESTGENFLAGS) $$< -o $$@
$1:          CXX := $(HOSTCXX)
$1:     CPPFLAGS += -I$(TEST_INC)
$1:        $1.cpp $$($1-SOURCES)
	    $$(LINK.cpp) $1.cpp $$($1-SOURCES) $(LOADLIBES) $(LDLIBS) -o $$@
endef
$(foreach t, $(TESTS), $(eval $(call test_template,$t)))

.PHONY:     run-tests

# Check that git submodules have been initialized.

define check_submodule
 ifeq ($(wildcard $1/*),)
  missing_submodule += $1
 endif
endef
$(foreach sm, $(SUBMODULES), $(eval $(call check_submodule,$(sm))))

ifdef missing_submodule
    # Hack: newline variable
    # https://stackoverflow.com/questions/17055773
    define n


    endef
    $(error $(firstword $(missing_submodule)) submodule is not initialized.$n\
            please run:$n\
            $$ git submodule init$n\
            $$ git submodule update$n\
            before running make)
endif

# Include all those *.d files.  Sort them to remove duplicates.
-include $(sort $(DFILES))
