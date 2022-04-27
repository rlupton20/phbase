BUILDROOT?=build

.DEFAULT_GOAL=all
.EXTRA_PREREQS:=Makefile clib.mk

include clib.mk

CFLAGS_BASE+=-Wall -Wextra -Werror -pedantic -MMD -I$(INTERFACE) $(EXTRA_CFLAGS)

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

DEPS=$(SOURCES:%.c=$(BUILD)/%.d) $(TEST_SOURCES:%c=$(BUILD)/%.d)
-include $(DEPS)

$(BUILD)/bins/test: $(SOURCES:%.c=$(BUILD)/%.o) $(TEST_SOURCES:%.c=$(BUILD)/%.o)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BUILD)/libphbase.a: $(SOURCES:%.c=$(BUILD)/%.o)
	@mkdir -p $(@D)
	ar rcs $@ $^

.PHONY: all
all: $(BUILD)/libphbase.a $(BUILD)/bins/test
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
