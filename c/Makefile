PROGRAM = cbot
SOURCES = $(wildcard *.c)
CFLAGS += -Wall -Wextra -pedantic -ansi
LDLIBS += -lm

all: $(PROGRAM)

$(PROGRAM): $(SOURCES)

clean:
	rm -f $(PROGRAM)

re: clean all


.PHONY: clean, re

