# hexa-bot — pandoc PDF rebuild for 4 verb specs.
#
# Generates four PDFs (one per verb) from the .md sources under
# robotics/, transport/, automation/, dog_robot/. Pattern from
# hexa-sscb/build/Makefile.
#
# Usage:
#   make -C build check     # verify pandoc + xelatex available
#   make -C build all       # build 4 PDFs into build/out/
#   make -C build clean     # remove build/out/
#   make -C build verify    # delegate to ../verify/cli.py
#   make -C build firmware  # delegate to ../module/firmware/ (if present)
#   make -C build sim       # delegate to ../module/firmware/sim/
#   make -C build all-targets   # check + verify + sim + all (PDFs)
#
# Generated PDFs land in build/out/ and are .gitignore'd via the project
# root .gitignore. If pandoc/xelatex are missing the toolchain check emits
# a SKIP line rather than failing — so CI without LaTeX still passes.

ROOT     := ..
OUT      := out
PANDOC   := pandoc
HEADER   := header.tex

PANDOC_FLAGS := \
  --pdf-engine=xelatex \
  --include-in-header=$(HEADER) \
  --variable=geometry:a4paper,margin=25mm \
  --variable=fontsize:11pt \
  --variable=colorlinks:true \
  --variable=linkcolor:black \
  --variable=urlcolor:blue \
  --syntax-highlighting=tango \
  --toc \
  --toc-depth=2

PDFS := \
  $(OUT)/hexa-bot_robotics.pdf \
  $(OUT)/hexa-bot_transport.pdf \
  $(OUT)/hexa-bot_automation.pdf \
  $(OUT)/hexa-bot_dog_robot.pdf

.PHONY: all clean check verify firmware sim all-targets help

all: check-soft $(PDFS)

$(OUT):
	mkdir -p $(OUT)

$(OUT)/hexa-bot_robotics.pdf: $(ROOT)/robotics/robotics.md $(HEADER) | $(OUT)
	$(PANDOC) $(PANDOC_FLAGS) \
	  --metadata=title:"hexa-bot — robotics (manipulation / arm / actuator)" \
	  --metadata=date:"2026-05-06" \
	  -o $@ $<

$(OUT)/hexa-bot_transport.pdf: $(ROOT)/transport/robotics-transport.md $(HEADER) | $(OUT)
	$(PANDOC) $(PANDOC_FLAGS) \
	  --metadata=title:"hexa-bot — transport (ground+aerial + logistics)" \
	  --metadata=date:"2026-05-06" \
	  -o $@ $<

$(OUT)/hexa-bot_automation.pdf: $(ROOT)/automation/control-automation.md $(HEADER) | $(OUT)
	$(PANDOC) $(PANDOC_FLAGS) \
	  --metadata=title:"hexa-bot — automation (control + factory-line)" \
	  --metadata=date:"2026-05-06" \
	  -o $@ $<

$(OUT)/hexa-bot_dog_robot.pdf: $(ROOT)/dog_robot/dog-robot-test.md $(HEADER) | $(OUT)
	$(PANDOC) $(PANDOC_FLAGS) \
	  --metadata=title:"hexa-bot — dog_robot (quadruped τ(6)=4 gait)" \
	  --metadata=date:"2026-05-06" \
	  -o $@ $<

clean:
	rm -rf $(OUT)

# Hard check — exit 1 if pandoc/xelatex missing.
check:
	@command -v $(PANDOC)  >/dev/null 2>&1 || { echo "MISSING: pandoc";  exit 1; }
	@command -v xelatex    >/dev/null 2>&1 || { echo "MISSING: xelatex"; exit 1; }
	@$(PANDOC) --version | head -1
	@xelatex --version | head -1
	@echo "OK: build toolchain ready"

# Soft check — emit SKIP and short-circuit `all` if pandoc/xelatex missing.
check-soft:
	@if ! command -v $(PANDOC) >/dev/null 2>&1; then \
	    echo "[build] SKIP: pandoc not installed (brew install pandoc)"; \
	    exit 0; \
	fi
	@if ! command -v xelatex >/dev/null 2>&1; then \
	    echo "[build] SKIP: xelatex not installed (brew install --cask mactex-no-gui)"; \
	    exit 0; \
	fi

# Super-targets — delegate to sibling subdirs so `make -C build` is a
# single project entry point for everything except git operations.
verify:
	@cd $(ROOT) && /usr/bin/env python3 verify/cli.py

firmware:
	@if [ -f $(ROOT)/module/firmware/Makefile ]; then \
	    $(MAKE) -C $(ROOT)/module/firmware check && \
	    $(MAKE) -C $(ROOT)/module/firmware all; \
	else \
	    echo "[build] SKIP: module/firmware/Makefile not present"; \
	fi

sim:
	@if [ -f $(ROOT)/module/firmware/sim/Makefile ]; then \
	    $(MAKE) -C $(ROOT)/module/firmware/sim run; \
	else \
	    echo "[build] SKIP: module/firmware/sim/Makefile not present"; \
	fi

# One-shot kitchen sink for CI / pre-release.
all-targets: check-soft verify sim $(PDFS)
	@echo "==== build all-targets done ===="

help:
	@echo "hexa-bot build/ targets:"
	@echo "  all          — 4 PDFs (skip if pandoc/xelatex missing)"
	@echo "  check        — hard toolchain check (exit 1 on miss)"
	@echo "  check-soft   — soft toolchain check (skip on miss)"
	@echo "  clean        — rm build/out/"
	@echo "  verify       — delegate to ../verify/cli.py"
	@echo "  firmware     — delegate to ../module/firmware/"
	@echo "  sim          — delegate to ../module/firmware/sim/"
	@echo "  all-targets  — check-soft + verify + sim + all PDFs"
