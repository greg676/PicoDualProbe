# PicoDualProbe 🟡🟢🔵

**One probe, two protocols. Debug ARM Cortex-M and RISC-V from a $5 RP2040 board.**

PicoDualProbe is a CMSIS-DAP firmware fork of the [Raspberry Pi Debug Probe](https://github.com/raspberrypi/debugprobe) that adds JTAG support, WS2812 LED status signaling, and pre-built firmware for the Waveshare RP2040-Zero. It replaces a $60 Segger J-Link with a board you probably already have in your parts bin.

## Why This Exists

The Segger J-Link EDU Mini bricked itself mid-debug-session. "Out of sync, resynchronizing" — unrecoverable. Rather than buy another, I built a better one.

PicoDualProbe does what the Segger did (JTAG for RISC-V) **plus** SWD for ARM Cortex-M, with a color-coded LED that tells you what mode it's in before you even open a terminal.

## What It Supports

| Feature | SWD Mode | JTAG Mode |
|---------|----------|-----------|
| ARM Cortex-M (RP2040, STM32, SAMD21) | ✅ | — |
| RISC-V (NeoRV32, VexRiscv, etc.) | — | ✅ |
| OpenOCD | ✅ | ✅ |
| GDB server (port 3333) | ✅ | ✅ |
| UART bridge to target | ✅ | ✅ |
| LED status signaling | ✅ | ✅ |

## Quick Start

### Option 1: Pre-built firmware (easiest)

1. Hold BOOT button, plug in the Zero
2. Drag `releases/debugprobe_on_zero.uf2` onto the RPI-RP2 drive
3. Done. The LED pulses blue — you're ready.

### Option 2: Build from source

```bash
git clone https://github.com/greg676/PicoDualProbe.git
cd PicoDualProbe
git submodule update --init --recursive

# For RP2040-Zero:
mkdir build-zero && cd build-zero
cmake -DPICO_BOARD=waveshare_rp2040_zero ..
make
# Output: debugprobe_on_zero.uf2

# For standard Pico:
mkdir build && cd build
cmake -DDEBUG_ON_PICO=ON ..
make
# Output: debugprobe_on_pico.uf2
```

## Pinout (RP2040-Zero)

All debug pins are on one edge of the board for clean wiring:

```
        ┌──────────────┐
  GP0 ──┤ TDI           │
  GP1 ──┤ TMS / SWDIO   │  ← shared by SWD and JTAG
  GP2 ──┤ TCK / SWCLK   │  ← shared by SWD and JTAG
  GP3 ──┤ TDO           │
  GP4 ──┤ nRESET        │  ← open-drain, active-low
  GND ──┤ GND           │
 GP12 ──┤ UART TX →     │  ← to target RX
 GP13 ──┤ UART RX ←     │  ← from target TX
 GP16 ──┤ WS2812 LED    │  ← status indicator
        └──────────────┘
```

## LED Signal Codes

The WS2812 RGB LED tells you what's happening at a glance:

| Signal | Color | Meaning |
|--------|-------|---------|
| 🔵🔵 blue-blue | Ready | Probe idle, waiting for connection |
| 🟢🟢 green-green | SWD | ARM Cortex-M debug active |
| 🟡🟡 yellow-yellow | JTAG | RISC-V debug active |
| 🔴🔴 red-red | Error | Something's wrong — check wiring |

After the initial surge pulse, the LED shows two dashes of the current state color. No guessing which mode you're in.

## Usage

### SWD: Debug an ARM target (RP2040, STM32, etc.)

```bash
# Connect: GP2→SWCLK, GP1→SWDIO, GND→GND
openocd -f scripts/openocd/picodualprobe-swd.cfg -f target/rp2040.cfg
```

Expected output: `DPIDR 0x0bc12477` — two Cortex-M0+ cores detected.

### JTAG: Debug a RISC-V target (NeoRV32)

```bash
# Connect: GP2→TCK, GP1→TMS, GP0→TDI, GP3→TDO, GND→GND
openocd -f scripts/openocd/picodualprobe-jtag.cfg -f scripts/openocd/neorv32.cfg
```

Expected output: `IDCODE 0x00000001`, RISC-V core examined, `misa=0x40801100`.

### Full validation

```bash
./scripts/verify.sh
```

Interactive script that walks you through wiring and tests both SWD and JTAG.

### JTAG with USB reset (workaround for reconnect bug)

```bash
./scripts/jtag.sh scripts/openocd/neorv32-validate.cfg
```

Resets the probe USB endpoint before and after OpenOCD to avoid the CMSIS-DAP reconnect issue.

## Verified Targets

| Target | Protocol | Result |
|--------|----------|--------|
| RP2040 (Pico) | SWD | ✅ DPIDR 0x0bc12477, both cores |
| NeoRV32 RISC-V (Alchitry Pt V2) | JTAG | ✅ IDCODE 0x00000001, halt, reg read, GDB |
| SAMD21 | SWD | ✅ Config provided |

## Known Issue: CMSIS-DAP Reconnect

The CMSIS-DAP vendor USB endpoint doesn't cleanly reset between OpenOCD sessions. First connection after plug-in works perfectly. Second connection fails with `CMD_DAP_JTAG_SEQ failed`. Third attempt kills the USB stack entirely.

**Workaround:** Unplug/replug between sessions, or use `scripts/jtag.sh` which does a USB reset via `usbreset`. See `BUG-CMSIS-DAP-RECONNECT.md` for the full investigation.

## Repository Structure

```
PicoDualProbe/
├── src/
│   ├── led_signal.c/h     ← WS2812 LED state machine
│   ├── cdc_uart.c         ← UART bridge (modified for stability)
│   ├── main.c             ← Dual-mode init
│   └── ws2812.pio         ← PIO assembly for LED
├── include/
│   └── board_zero_config.h ← Locked RP2040-Zero pinout
├── scripts/
│   ├── verify.sh           ← Interactive SWD+JTAG validation
│   ├── jtag.sh             ← JTAG wrapper with USB reset
│   └── openocd/            ← 11 config files for various targets
├── releases/
│   ├── debugprobe_on_zero.uf2  ← Pre-built for RP2040-Zero
│   └── debugprobe_on_pico.uf2  ← Pre-built for standard Pico
└── BUG-CMSIS-DAP-RECONNECT.md  ← Deep dive on the reconnect bug
```

## Companion Projects

- **[NeoRV32 on Alchitry Pt V2](https://github.com/greg676/alchitry-ptv2-neorv32)** — A real RISC-V SoC in FPGA fabric. The primary JTAG target this probe was built for.
- **[NeoRV32 MCP Server](https://claw-studio.tail708254.ts.net:3100/claw/neorv32-mcp)** — AI agent bridge for hardware debug. 15 tools, UART + JTAG, safe state machine.

## Credits

Built on the excellent [Raspberry Pi Debug Probe](https://github.com/raspberrypi/debugprobe) firmware. Added JTAG support, LED signaling, pre-built Zero firmware, and OpenOCD configs for RISC-V targets.

## License

BSD-3-Clause (inherited from Raspberry Pi Debug Probe)
