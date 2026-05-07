# dog_robot firmware sim — host build (POSIX C, no ARM toolchain).
#
# Compiles the firmware src/*.c under -DSIM_BUILD against mock_hardware.c
# so the gait state machine + servo apply path can be exercised on
# Linux/macOS in seconds, no STM32 board required.
#
# Targets:
#   make           # build dog_sim
#   make run       # ./build/dog_sim --gait=trot
#   make selftest  # ./build/dog_sim --selftest  (returns 0 on coherence)
#   make all-gaits # run sim once per gait pattern with JSON summary
#   make clean

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra -std=c99 -g -DSIM_BUILD -I../include -I.
LDFLAGS ?=

SRC := \
  ../src/main.c \
  ../src/gait_sm.c \
  ../src/servo_pwm.c \
  ../src/leg_kinematics.c \
  ../src/system_init.c \
  mock_hardware.c \
  dog_sim.c

BUILDDIR := build
BIN      := $(BUILDDIR)/dog_sim

.PHONY: all run selftest all-gaits clean check

all: $(BIN)

$(BUILDDIR):
	mkdir -p $@

$(BIN): $(SRC) | $(BUILDDIR)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

run: $(BIN)
	@./$(BIN) --gait=trot --ms=1600

selftest: $(BIN)
	@./$(BIN) --selftest

all-gaits: $(BIN)
	@for g in stand trot bound gallop; do \
	    echo "==== gait=$$g ===="; \
	    ./$(BIN) --gait=$$g --ms=1600 --json; \
	done

check:
	@command -v $(CC) >/dev/null 2>&1 || { echo "MISSING: $(CC)"; exit 1; }
	@$(CC) --version | head -1

clean:
	rm -rf $(BUILDDIR)
