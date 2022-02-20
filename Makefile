.DEFAULT_GOAL=all
.PRECIOUS: $(BUILD)/%.o

BUILDROOT=build
CFLAGS_BASE+=-Wall -Wextra -Werror -pedantic -std=c11 -Iinclude/ -MMD

ifeq ($(TARGET),release)
  CFLAGS_TARGET=-O2 -DNEBUG
else ifeq ($(TARGET),coverage)
  CFLAGS_TARGET=-g -fprofile-arcs -ftest-coverage
else
  TARGET=debug
  CFLAGS_TARGET=-g -fsanitize=address -fsanitize=undefined
endif

BUILD=$(BUILDROOT)/$(TARGET)

CFLAGS=$(CFLAGS_BASE) $(CFLAGS_TARGET)

SRCS=$(shell find src/ bins/ test/ -name '*.c')
OBJS=$(SRCS:%.c=$(BUILD)/%.o)
DEPS=$(SRCS:%.c=$(BUILD)/%.d)
-include $(DEPS)

LIBOBJS=$(filter $(BUILD)/src/%, $(OBJS))
TESTOBJS=$(filter $(BUILD)/test/%, $(OBJS))
BINOBJS=$(filter $(BUILD)/bins/%, $(OBJS))
BINS=$(BINOBJS:%.o=%)

$(BUILD)/bins/test: $(BUILD)/bins/test.o $(LIBOBJS) $(TESTOBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD)/bins/%: $(BUILD)/bins/%.o $(LIBOBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD)/%.o: %.c Makefile
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/libphbase.a: $(LIBOBJS)
	ar rcs $@ $(LIBOBJS)

.PHONY: all
all: $(BINS) $(BUILD)/libphbase.a
	@$(BUILD)/bins/test
	@echo "Build of $(TARGET) target successful at $(shell date)"

ifeq ($(TARGET),coverage)
.PHONY: coverage
coverage:
	lcov --capture --directory $(BUILD) --output-file $(BUILD)/coverage.info
	genhtml -o $(BUILD)/coverage $(BUILD)/coverage.info
	@echo "Coverage output to $(BUILD)/coverage/index.html"
endif

.PHONY: clean
clean:
	rm -rf $(BUILDROOT)
