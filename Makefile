BIN	= ciz

CC ?= gcc
CFLAGS = -std=c99 -g -O2 -Wall -Wextra -Werror -I src/lib

SRCS := $(shell find src -type f -name "*.c")
OBJS := $(patsubst src/%.c, src/%.o, $(SRCS))

all: $(BIN)
$(BIN): $(OBJS)
	@echo " [LD] $@"
	@${CC} -g -O2 $^ -o $@

%.o: %.c
	@echo " [CC] $<"
	@${CC} $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(BIN)
