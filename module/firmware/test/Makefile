# dog_robot firmware unit tests — host build (POSIX C, no ARM toolchain).
#
# Each test_*.c links the firmware sources under -DSIM_BUILD against
# mock_hardware.c. xunit-style framework — no external dependency.
#
# Targets:
#   make           # build all test_*.c into build/
#   make run       # build + run each test, fail on first non-zero
#   make clean

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c99 -g -DSIM_BUILD -I../include -I.
LDFLAGS ?=

FW_SRC := \
  ../src/main.c \
  ../src/gait_sm.c \
  ../src/servo_pwm.c \
  ../src/leg_kinematics.c \
  ../src/system_init.c \
  ../sim/mock_hardware.c

TEST_SRC := $(wildcard test_*.c)
TEST_BIN := $(TEST_SRC:%.c=build/%)

BUILDDIR := build

.PHONY: all run clean check

all: $(TEST_BIN)

$(BUILDDIR):
	mkdir -p $@

build/test_%: test_%.c $(FW_SRC) | $(BUILDDIR)
	$(CC) $(CFLAGS) $< $(FW_SRC) -o $@ $(LDFLAGS)

run: $(TEST_BIN)
	@fails=0; for t in $(TEST_BIN); do \
	    echo "==== $$t ===="; \
	    if ! ./$$t; then fails=$$((fails+1)); fi; \
	done; \
	if [ $$fails -ne 0 ]; then \
	    echo "==== $$fails test binary/binaries FAILED ===="; \
	    exit 1; \
	fi; \
	echo "==== all tests PASS ===="

clean:
	rm -rf $(BUILDDIR)

check:
	@command -v $(CC) >/dev/null 2>&1 || { echo "MISSING: $(CC)"; exit 1; }
	@$(CC) --version | head -1
