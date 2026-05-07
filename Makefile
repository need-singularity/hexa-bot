# hexa-bot — top-level Makefile (super-targets).
#
# One project entry point for the v1.1.0 runnable surface. Each target
# delegates to the relevant subdirectory's own Makefile so this file stays
# thin and the per-subsystem Makefiles remain authoritative.
#
# Usage:
#   make                  # alias for `make all`
#   make all              # verify + sim + test + firmware + build (PDFs)
#   make verify           # python3 verify/cli.py
#   make sim              # build + selftest the host simulator
#   make test             # pytest auto + firmware unit tests
#   make firmware         # ARM cross-compile (skip if no toolchain)
#   make build            # pandoc PDFs (skip if no pandoc)
#   make clean            # rm everything that any sub-make produces
#   make help             # this summary

PYTHON ?= /usr/bin/env python3

.PHONY: all verify sim test test-pytest test-firmware-unit firmware build clean help

all: verify sim test firmware build
	@echo ""
	@echo "==== hexa-bot all targets done ===="
	@echo "    verify      ✓ verify/cli.py all"
	@echo "    sim         ✓ module/firmware/sim/ host build + selftest"
	@echo "    test        ✓ pytest -m auto + firmware unit tests"
	@echo "    firmware    ✓ module/firmware/ ARM cross-compile (or skip)"
	@echo "    build       ✓ build/ pandoc PDFs (or skip)"

verify:
	$(PYTHON) verify/cli.py

sim:
	$(MAKE) -C module/firmware/sim selftest

test: test-pytest test-firmware-unit

test-pytest:
	$(PYTHON) -m pytest tests/ -v -m auto

test-firmware-unit:
	$(MAKE) -C module/firmware/test run

firmware:
	@if command -v arm-none-eabi-gcc >/dev/null 2>&1; then \
	    $(MAKE) -C module/firmware all; \
	else \
	    echo "[hexa-bot] SKIP firmware: arm-none-eabi-gcc not installed"; \
	fi

build:
	@if command -v pandoc >/dev/null 2>&1 && command -v xelatex >/dev/null 2>&1; then \
	    $(MAKE) -C build all; \
	else \
	    echo "[hexa-bot] SKIP build: pandoc or xelatex not installed"; \
	fi

clean:
	-$(MAKE) -C module/firmware clean
	-$(MAKE) -C module/firmware/sim clean
	-$(MAKE) -C module/firmware/test clean
	-$(MAKE) -C build clean
	rm -rf .pytest_cache __pycache__

help:
	@echo "hexa-bot top-level Makefile targets:"
	@echo "  all              verify + sim + test + firmware + build"
	@echo "  verify           python3 verify/cli.py all (5 invariant checks)"
	@echo "  sim              module/firmware/sim host build + selftest"
	@echo "  test             pytest -m auto + firmware unit tests"
	@echo "  firmware         module/firmware ARM cross-compile (skip if no toolchain)"
	@echo "  build            pandoc PDFs (skip if no pandoc/xelatex)"
	@echo "  clean            remove everything any sub-make produces"
	@echo ""
	@echo "  python3 verify/cli.py --list   list registered verifiers"
	@echo "  hexa run cli/hexa-bot.hexa <subcmd>   .hexa dispatcher entry"
