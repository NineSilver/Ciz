BIN	= ciz

CC ?= gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -I src/lib

SRCS := $(shell find src -type f -name "*.c")
OBJS := $(patsubst src/%.c, src/%.o, $(SRCS))

.DEFAULT: all
.PHONY: all
all: release

.PHONY: debug
debug: CFLAGS += -g -O0
debug: LDFLAGS += -g -O0
debug: $(BIN)

.PHONY: release
release: CFLAGS += -Ofast
release: LDFLAGS += -Ofast
release: $(BIN)

$(BIN): $(OBJS)
	@echo " [LD] $@"
	@${CC} $(LDFLAGS) -o $@ $^

%.o: %.c
	@echo " [CC] $<"
	@${CC} $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJS) $(BIN)
