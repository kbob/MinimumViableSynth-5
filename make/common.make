# Derive project root as this file's grandparent.
r := $(lastword $(MAKEFILE_LIST))
r := $(r:make/common.make=)
r := $(r:/=)

# unpack sources
       CFILES := $(filter %.c, $(SOURCES))
     CXXFILES := $(filter %.cpp, $(SOURCES))
       OFILES := $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
       DFILES := $(OFILES:.o=.d)

# invoke make with BUILD=release for release build.
    debug_OPT := -O0 -fsanitize=bounds
  release_OPT := -O3 -DNDEBUG
        BUILD := debug
          OPT := $($(BUILD)_OPT)
     CPPFLAGS := -I../.. -MMD $(EXTRA_CPPFLAGS)
     CXXFLAGS := -std=c++11 -Wall -Wextra -Werror $(OPT)

      HOSTCXX := c++
      CXXTEST := $r/submodules/cxxtest
      TESTGEN := $(CXXTEST)/bin/cxxtestgen
 TESTGENFLAGS := --have-eh --error-printer
     TEST_INC := $(CXXTEST)

all:        $(SUBDIRS:%=%/all) $(TESTS) run-tests $(PROGRAMS) $(IMAGES)
ifeq ($r,)
world:      all
else
world:
	    make -C $r world
endif
programs:   $(SUBDIRS:%=%/programs) $(PROGRAMS)
images:     $(SUBDIRS:%=%/images) $(IMAGES)
test:       $(SUBDIRS:%=%/test) $(TESTS) run-tests
tests:      $(SUBDIRS:%=%/tests) $(TESTS)
clean:      $(SUBDIRS:%=%/clean)
	    rm -f *.d *.o *.out test-*.cpp $(PROGRAMS) $(TESTS) $(FILTH)
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
	    @echo '    run-tests        - build and run all tests'
	    @echo '    tests            - build all test programss'
	    @echo '    clean            - remove all generated files'
	    @echo '    help             - print this text'
	    @echo ''
	    @echo 'Common Variables'
	    @echo ''
	    @echo '    BUILD=debug      - use "BUILD=release" for release build'
	    @echo '    EXTRA_CPPFLAGS=  - add flags to cc and c++'
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
	    @for t in $(TESTS); do echo "    $$t"; done
	    @echo ''
	    @echo '  Run:'
	    @for t in $(TESTS); do echo "    run-$$t"; done
	    @echo ''
endif

PHONY:     all world programs images test tests clean
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

# $(info PROGRAMS = $(PROGRAMS))
# $(info a.out-SOURCES = $(a.out-SOURCES))
# $(info a.out-CXXFILES = $(a.out-CXXFILES))
# $(info a.out-OFILES = $(a.out-OFILES))
# $(info DFILES = $(DFILES))

# $(PROGRAM): $(OFILES)
# 	    $(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Build and run test runner from test-foo.h.

  RUN_TESTS := $(TESTS:%=run-%)

run-tests:  $(RUN_TESTS)
run-test-%: test-%
	    ./$<

define test_template =
       $1-CFILES := $(filter %.c, $$($1-SOURCES))
     $1-CXXFILES := $1.cpp $(filter %.cpp, $$($1-SOURCES))
     $1-CXXFILES := $1.cpp $$($1-SOURCES)
       $1-OFILES := $$($1-CFILES:.c=.o) $$($1-CXXFILES:.cpp=.o)
          DFILES += $$($1-OFILES:.o=.d)

$1.cpp:     $1.h
	    $(TESTGEN) $(TESTGENFLAGS) $$< -o $$@
$1:          CXX := $(HOSTCXX)
$1:     CXXFLAGS += -I$(TEST_INC)
$1:         $$($1-OFILES)
	    $$(LINK.cpp) $$^ $(LOADLIBES) $(LDLIBS) -o $$@
endef
$(foreach t, $(TESTS), $(eval $(call test_template,$t)))

# $(info TESTS = $(TESTS))
# $(info test-mod-network-SOURCES = $(test-mod-network-SOURCES))
# $(info test-mod-network-CXXFILES = $(test-mod-network-CXXFILES))
# $(info test-mod-network-OFILES = $(test-mod-network-OFILES))
# $(info DFILES = $(DFILES))

# test-%:     CXX := $(HOSTCXX)
# test-%:     CXXFLAGS += -I$(TEST_INC)
# test-%:     test-%.cpp $($@-SOURCES)
# #	    $(LINK.cpp) $* $(LOADLIBES) $(LDLIBS) -o $@
# 	    $(LINK.cpp) $< $(test-%-SOURCES) $(LOADLIBES) $(LDLIBS) -o $@
# test-%.cpp: test-%.h
# 	    $(TESTGEN) $(TESTGENFLAGS) $< -o $@

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

# These come last.

# sort to remove duplicates.
-include $(sort $(DFILES))
