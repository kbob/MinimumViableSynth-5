CFILES := main.c midi-file.c
OFILES := $(CFILES:.c=.o)

CFLAGS += -Wall -Werror

offline: $(OFILES)
	$(LINK.c) $* $^ $(LOADLIBES) $(LdLIBS) -o $@
