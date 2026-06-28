/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Raspberry Pi (Trading) Ltd.
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

#ifndef BOARD_PICO_H_
#define BOARD_PICO_H_

// ============================================================================
// PicoDualProbe — Dual SWD/JTAG CMSIS-DAP probe on Raspberry Pi Pico (RP2040)
//
// Pin map (LOCKED — do not change without updating OpenOCD configs):
//   GP2  = SWCLK / TCK   (shared by SWD and JTAG)
//   GP3  = SWDIO / TMS   (shared by SWD and JTAG)
//   GP4  = TDI            (JTAG only)
//   GP5  = TDO            (JTAG only)
//   GP6  = nRESET         (open-drain, active-low)
//   GP8  = UART TX → target RX
//   GP9  = UART RX ← target TX
//   GP25 = LED (on-board)
// ============================================================================

#define PROBE_IO_RAW
#define PROBE_CDC_UART

// PIO config (SWD mode)
#define PROBE_SM 0
#define PROBE_PIN_OFFSET 2
#define PROBE_PIN_SWCLK (PROBE_PIN_OFFSET + 0) // GP2
#define PROBE_PIN_SWDIO (PROBE_PIN_OFFSET + 1) // GP3

// JTAG pin definitions (used by DAP_config.h JTAG macros)
#define PROBE_PIN_TDI   4
#define PROBE_PIN_TDO   5

// Target reset config (open-drain, active-low)
#define PROBE_PIN_RESET 6

// UART config (CDC bridge to target console)
#define PROBE_UART_TX 8
#define PROBE_UART_RX 9
#define PROBE_UART_INTERFACE uart1
#define PROBE_UART_BAUDRATE 115200

// Status LED
#define PROBE_USB_CONNECTED_LED 25

#define PROBE_PRODUCT_STRING "PicoDualProbe (CMSIS-DAP)"

#endif
