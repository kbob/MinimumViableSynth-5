# Derive project root as this file's grandparent.
r := $(lastword $(MAKEFILE_LIST))
r := $(r:%make/common.make=%)
r := $(r:%/=%)

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
     CPPFLAGS := -I../.. -MMD
     CXXFLAGS := -std=c++11 -Wall -Wextra -Werror $(OPT)

      HOSTCXX := c++
      CXXTEST := $r/submodules/cxxtest
      TESTGEN := $(CXXTEST)/bin/cxxtestgen
 TESTGENFLAGS := --have-eh --error-printer
     TEST_INC := $(CXXTEST)
       DFILES += $(TESTS:%=%.d)

# all:        test programs images
all:        $(SUBDIRS:%=%/all) $(TESTS) run-tests $(PROGRAM) $(IMAGES)
ifneq ($r,)
project:
	    make -C $r all
endif
programs:   $(SUBDIRS:%=%/programs) $(PROGRAM)
images:     $(SUBDIRS:%=%/images) $(IMAGES)
test:       $(SUBDIRS:%=%/test) $(TESTS) run-tests
tests:      $(SUBDIRS:%=%/tests) $(TESTS)
clean:      $(SUBDIRS:%=%/clean)
	    rm -f *.d *.o *.out test-*.cpp $(PROGRAM) $(TESTS) $(FILTH)
help:
	    @echo ''
	    @echo 'Common Targets'
	    @echo ''
	    @echo '    all            - build programs and images, run tests'
ifneq ($r,)
	    @echo '    project        - build whole project, run all tests'
endif
	    @echo '    programs       - build all programs'
	    @echo '    images         - build all firmware images'
	    @echo '    test           - build and run all tests'
	    @echo '    tests          - build all tests'
	    @echo '    clean          - remove all generated files'
	    @echo '    help           - print this text'
	    @echo ''

.PHONY: all project programs images test tests clean

# Recurse into subdirectories.

R_ACTIONS := all programs images test tests clean
define recur_template
 $$(SUBDIRS:=/$1):
	    make -C $$(@:/$(1)=) $1
 .PHONY:    $$(SUBDIRS:=/$1)
endef
$(foreach a, $(R_ACTIONS), $(eval $(call recur_template,$a)))

# Build program from $(SOURCES)

$(PROGRAM): $(OFILES)
	    $(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Build and run test runner from test-foo.h.

  RUN_TESTS := $(TESTS:%=run-%)

run-tests:  $(RUN_TESTS)
run-test-%: test-%
	    ./$<
test-%:     CXX := $(HOSTCXX)
test-%:     CXXFLAGS += -I$(TEST_INC)
test-%:     test-%.cpp
	    $(LINK.cpp) $< $(LOADLIBES) $(LDLIBS) -o $@
test-%.cpp: test-%.h
	    $(TESTGEN) $(TESTGENFLAGS) $< -o $@

.PHONY: run-tests $(RUN_TESTS)

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

-include $(DFILES)
