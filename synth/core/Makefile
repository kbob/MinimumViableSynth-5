
  SOURCES := main.cpp
 BINARIES := a.out

   CFILES := $(filter %.c, $(SOURCES))
 CXXFILES := $(filter %.cpp, $(SOURCES))
   OFILES := $(CFILES:.c=.o) $(CXXFILES:.cpp=.o)
   DFILES := $(OFILES:.o=.d)

 CPPFLAGS := -I../.. -MMD
 CXXFLAGS := -std=c++11 -Wall -Wextra

all: $(BINARIES)

clean:
	rm -f *.out *.o *.d

a.out: $(OFILES)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS)

-include $(DFILES)
