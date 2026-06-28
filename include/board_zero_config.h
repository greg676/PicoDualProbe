/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2026 PicoDualProbe
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef BOARD_ZERO_H_
#define BOARD_ZERO_H_

// ============================================================================
// PicoDualProbe — Waveshare RP2040-Zero (23×18mm, USB-C, WS2812 on GP16)
//
// Pin map (matches DirtyJTAG Zero soldering):
//   GP0  = TDI            (JTAG only)
//   GP1  = TMS / SWDIO    (shared by SWD and JTAG)
//   GP2  = TCK / SWCLK    (shared by SWD and JTAG)
//   GP3  = TDO            (JTAG only)
//   GP4  = nRESET         (open-drain, active-low)
//   GP16 = WS2812 RGB LED (status)
//   GP12 = UART TX → target RX
//   GP13 = UART RX ← target TX
//
// JTAG pinout matches DirtyJTAG BOARD_RP2040_ZERO exactly.
// ============================================================================

#define PROBE_IO_RAW
#define PROBE_CDC_UART

// PIO config (SWD mode) — GP2/GP1 are NOT consecutive, so use GP2 for SWCLK
// and GP1 for SWDIO. PIO needs consecutive pins, so we use offset 1:
// GP1=SWDIO, GP2=SWCLK — but PIO sideset needs SWCLK first.
// Actually PIO requires consecutive: use GP2=SWCLK, GP3=SWDIO for SWD PIO.
// But GP3 is TDO in JTAG... SWD and JTAG share TCK/SWCLK and TMS/SWDIO.
// DirtyJTAG: GP2=TCK, GP1=TMS. These aren't consecutive for PIO.
// Solution: SWD PIO uses GP2(SWCLK)+GP3(SWDIO), JTAG uses GP2(TCK)+GP1(TMS)+GP0(TDI)+GP3(TDO)
// SWD and JTAG share TCK/SWCLK=GP2. SWDIO=GP3 (PIO), TMS=GP1 (SIO in JTAG mode).
// This means SWD uses GP2/GP3 (consecutive, PIO works), JTAG uses GP0/1/2/3/4.

#define PROBE_SM 0
#define PROBE_PIN_OFFSET 2   // GP2=SWCLK, GP3=SWDIO (consecutive for PIO)
#define PROBE_PIN_SWCLK (PROBE_PIN_OFFSET + 0) // GP2 = TCK/SWCLK
#define PROBE_PIN_SWDIO (PROBE_PIN_OFFSET + 1) // GP3 = TDO/SWDIO

// JTAG pin definitions (used by DAP_config.h JTAG macros)
#define PROBE_PIN_TDI   0   // GP0 = TDI
#define PROBE_PIN_TDO   3   // GP3 = TDO (shared with SWDIO)
#define PROBE_PIN_TMS   1   // GP1 = TMS (separate from SWDIO in JTAG mode)

// Target reset config (open-drain, active-low)
#define PROBE_PIN_RESET 4   // GP4 = nRST

// UART config (CDC bridge to target console)
#define PROBE_UART_TX 12
#define PROBE_UART_RX 13
#define PROBE_UART_INTERFACE uart0
#define PROBE_UART_BAUDRATE 115200

// Status LED — WS2812 on GP16 (PIO SM1, see led_signal.c)
#define PICO_DEFAULT_WS2812_PIN 16

#define PROBE_PRODUCT_STRING "PicoDualProbe Zero (CMSIS-DAP)"

#endif
