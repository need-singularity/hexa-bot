"""
hexa-bot — pytest configuration.

Pattern source: hexa-sscb/tests/conftest.py.

Marker semantics:
  auto      — runs without external bench equipment or hardware boards
  bench     — requires physical bench equipment (oscilloscope, IMU rig)
  hardware  — requires the actual STM32F429 Nucleo + servo set
  cert      — requires regulatory certification labs (UL/KC/etc.)

Default invocation skips bench/hardware/cert with reason; CI uses `-m auto`.

Run all auto-checks:
    pytest tests/ -v -m auto

Run only the firmware-sim test (requires the sim binary to be built):
    pytest tests/ -v -k firmware_sim
"""
from __future__ import annotations

import pytest


def pytest_configure(config):
    config.addinivalue_line(
        "markers",
        "auto: acceptance check that runs without bench equipment",
    )
    config.addinivalue_line(
        "markers",
        "bench: acceptance check requiring physical bench equipment",
    )
    config.addinivalue_line(
        "markers",
        "hardware: acceptance check requiring the actual STM32 + servo board",
    )
    config.addinivalue_line(
        "markers",
        "cert: acceptance check requiring regulatory certification lab",
    )
