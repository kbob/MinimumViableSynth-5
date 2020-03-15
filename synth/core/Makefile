
  SOURCES := main.cpp
 BINARIES := a.out
    TESTS := test-heap-map
    VPATH := ../util

   CFILES := $(filter %.c, $(SOURCES))
 CXXFILES := $(filter %.cpp, $(SOURCES))
   OFILES := $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
   DFILES := $(OFILES:.o=.d)

      OPT := -O0
 CPPFLAGS := -I../.. -MMD
 CXXFLAGS := -std=c++11 -Wall -Wextra -Werror $(OPT)

  CXXTEST := ../../submodules/cxxtest
  TESTGEN := $(CXXTEST)/bin/cxxtestgen
  TESTGENFLAGS := --have-eh --error-printer
 TEST_INC := $(CXXTEST)
   DFILES += $(TESTS:%=%.d)

all: test bin

bin: $(BINARIES)

a.out: $(OFILES)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS)


clean:
	rm -f *.d *.o *.out test-*.cpp $(TESTS)


test: $(TESTS) run-tests

run-tests: $(TESTS:%=run-%)

run-test-%: test-%
	./$<

test-%: CXXFLAGS += -I$(TEST_INC)
test-%: test-%.cpp
	$(LINK.cpp) $< $(LOADLIBES) $(LDLIBS) -o $@

test-%.cpp: test-%.h
	$(TESTGEN) $(TESTGENFLAGS) $< -o $@

.PHONY: all bin clean tests run-tests

-include $(DFILES)
