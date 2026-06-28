# PicoDualProbe — Dual SWD/JTAG CMSIS-DAP Debug Probe

A dual-mode SWD + JTAG debug probe built on the RP2040, forked from Raspberry Pi's debugprobe. Supports both ARM Cortex-M (SWD) and RISC-V (JTAG) targets from a single $4 board.

## Features

- **SWD mode**: PIO-based deterministic timing for ARM Cortex-M targets
- **JTAG mode**: SIO GPIO bit-banging for RISC-V and other JTAG targets
- **CMSIS-DAP v2**: WinUSB bulk transport, driverless on all platforms
- **UART bridge**: CDC ACM serial passthrough to target console
- **WS2812 LED status**: Surge-pulse signaling system (on RP2040-Zero)
- **Dual board support**: Raspberry Pi Pico and Waveshare RP2040-Zero

## Validated Targets

| Target | Interface | Status |
|--------|-----------|--------|
| RP2040 (Pico) | SWD | Both cores detected |
| NEORV32 RISC-V (Alchitry PtV2) | JTAG | Halt, register read, GDB server |
| SAMD21 | SWD | Via OpenOCD |

## Quick Start

### Build

```bash
# For RP2040-Zero (with WS2812 LED)
PICO_SDK_PATH=/path/to/pico-sdk bash build.sh zero

# For Raspberry Pi Pico
PICO_SDK_PATH=/path/to/pico-sdk bash build.sh pico
```

### Flash

Hold BOOTSEL, plug in, copy .uf2 to RPI-RP2 drive.

### Use with OpenOCD

SWD (ARM Cortex-M):
```bash
openocd -f scripts/openocd/picodualprobe-swd.cfg -f target/rp2040.cfg
```

JTAG (RISC-V / NEORV32):
```bash
openocd -f scripts/openocd/neorv32.cfg
```

## Pinout

### RP2040-Zero

| GPIO | SWD | JTAG |
|------|-----|------|
| GP0  | —   | TDI  |
| GP1  | —   | TMS  |
| GP2  | SWCLK | TCK |
| GP3  | SWDIO | TDO |
| GP4  | nRST | nRST |
| GP12 | UART TX | — |
| GP13 | UART RX | — |
| GP16 | WS2812 LED | WS2812 LED |

### Raspberry Pi Pico

| GPIO | SWD | JTAG |
|------|-----|------|
| GP2  | SWCLK | TCK |
| GP3  | SWDIO | TMS |
| GP4  | —   | TDI  |
| GP5  | —   | TDO  |
| GP6  | nRST | nRST |
| GP8  | UART TX | — |
| GP9  | UART RX | — |

## LED Signal States

| State | Pattern | Meaning |
|-------|---------|---------|
| 1 | Blue-blue | Ready |
| 2 | Red-red | Error |
| 3 | Green-green | SWD mode |
| 4 | Yellow-yellow | JTAG mode |

## License

MIT (upstream debugprobe license)
