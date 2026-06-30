# PicoDualProbe 🟡🟢🔵

*One probe. Two protocols. Five dollars.*

[![CMSIS-DAP](https://img.shields.io/badge/CMSIS--DAP-v2-blue)](https://arm-software.github.io/CMSIS_5/DAP/html/index.html) [![RISC-V](https://img.shields.io/badge/debugs-RISC--V-4B0082)](https://riscv.org) [![ARM](https://img.shields.io/badge/debugs-ARM_Cortex--M-0091BD)](https://developer.arm.com) [![License](https://img.shields.io/badge/license-BSD--3-green)](LICENSE)

---

Your Segger J-Link just bricked itself mid-session. A new one costs $60. You've got an RP2040-Zero in your parts bin that cost $4.37.

**This is the firmware that turns it into a better debug probe.**

PicoDualProbe does SWD for ARM Cortex-M *and* JTAG for RISC-V — from the same board, with a color-coded LED that tells you what mode it's in before you even open a terminal. It's a fork of the Raspberry Pi Debug Probe firmware, hardened for dual-protocol use with real RISC-V targets.

```
     ┌──────────────────────┐
     │   🔵🔵  Ready         │
     │   🟢🟢  SWD (ARM)     │
     │   🟡🟡  JTAG (RISC-V) │
     │   🔴🔴  Error         │
     └──────────────────────┘
     RP2040-Zero · $4.37 · USB-C
```

## What It Debugs

| Target | Protocol | Status |
|--------|----------|--------|
| **NeoRV32 RISC-V** (Alchitry Pt V2, XC7A100T) | JTAG | ✅ Halt, reg read, GDB, flash |
| **RP2040** (Pico) | SWD | ✅ DPIDR 0x0bc12477, both cores |
| **SAMD21** | SWD | ✅ Config provided |
| Any ARM Cortex-M | SWD | ✅ Standard CMSIS-DAP |
| Any RISC-V with JTAG DTM | JTAG | ✅ OpenOCD `riscv` target |

## 30-Second Start

```bash
# 1. Flash the firmware (drag .uf2 onto RPI-RP2 drive)
# 2. Wire it up:
#    GP2→TCK/SWCLK  GP1→TMS/SWDIO  GP0→TDI  GP3→TDO  GND→GND
# 3. The LED pulses blue. You're ready.

# SWD — ARM target:
openocd -f scripts/openocd/picodualprobe-swd.cfg -f target/rp2040.cfg

# JTAG — RISC-V target:
openocd -f scripts/openocd/picodualprobe-jtag.cfg -f scripts/openocd/neorv32.cfg
```

## Pinout

All debug signals on one edge. Clean wiring, no spaghetti.

```
GP0  = TDI          GP12 = UART TX →
GP1  = TMS / SWDIO  GP13 = UART RX ←
GP2  = TCK / SWCLK  GP16 = WS2812 LED
GP3  = TDO          GND  = Ground
GP4  = nRESET
```

## Why Not Just Buy a J-Link?

Because when a J-Link bricks (and it will), you're dead in the water waiting for shipping. A PicoDualProbe is:

- **$5** vs $60 for a J-Link EDU Mini
- **Dual protocol** — J-Link EDU is SWD only unless you pay $400+ for the Plus
- **Repairable** — brick it? Re-flash the .uf2 in 3 seconds
- **Visible state** — the LED tells you what's happening. J-Link gives you a blinking green dot
- **Open source** — you can read the firmware, fix bugs, add features

## Pre-built Firmware

| Board | File | 
|-------|------|
| RP2040-Zero | [`releases/debugprobe_on_zero.uf2`](releases/debugprobe_on_zero.uf2) |
| Standard Pico | [`releases/debugprobe_on_pico.uf2`](releases/debugprobe_on_pico.uf2) |

## Build From Source

```bash
git clone https://github.com/greg676/PicoDualProbe.git
cd PicoDualProbe
git submodule update --init --recursive

# For RP2040-Zero:
mkdir build-zero && cd build-zero
cmake -DPICO_BOARD=waveshare_rp2040_zero ..
make
```

## Known Issue

CMSIS-DAP USB endpoint doesn't cleanly reset between OpenOCD sessions. First connection works. Second fails. **Workaround:** unplug/replug, or use `scripts/jtag.sh` which does a USB reset. See [`BUG-CMSIS-DAP-RECONNECT.md`](BUG-CMSIS-DAP-RECONNECT.md).

## Companion Projects

| Project | What | 
|---------|------|
| **[NeoRV32 on Alchitry Pt V2](https://claw-studio.tail708254.ts.net:3100/claw/alchitry-ptv2-neorv32)** | The RISC-V platform this probe was built to debug |
| **[NeoRV32 MCP Server](https://claw-studio.tail708254.ts.net:3100/claw/neorv32-mcp)** | AI agent bridge — 15 tools for hardware control |

## Credits

Built on the [Raspberry Pi Debug Probe](https://github.com/raspberrypi/debugprobe) firmware. Added JTAG, LED signaling, Zero pinout, pre-built firmware, and OpenOCD configs for RISC-V.

BSD-3-Clause © 2026 Greg Jackson
