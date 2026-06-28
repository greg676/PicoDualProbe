#ifndef LED_SIGNAL_H_
#define LED_SIGNAL_H_

#include <stdint.h>

// 9-state surge-pulse signaling system.
// Base: blue surge 0% -> 50% -> 0% in ~1 sec.
// After surge: either two same-color dashes OR blue-color-blue.
//
// State encoding (1-based, 0 treated as state 1):
//   1 = blue-blue
//   2 = red-red
//   3 = green-green
//   4 = yellow-yellow
//   5 = white-white
//   6 = blue-red
//   7 = blue-green
//   8 = blue-yellow
//   9 = blue-white
//
// PicoDualProbe signal assignments:
//   1 = ready (blue-blue)
//   2 = error (red-red)
//   3 = SWD mode (green-green)
//   4 = JTAG mode (yellow-yellow)

#define LED_SIGNAL_STATE_COUNT 10  // states 0..9, 0 = off
#define LED_SIGNAL_STATE_MIN    0
#define LED_SIGNAL_STATE_MAX    9

void led_signal_init(void);
void led_signal_set(uint8_t state);
uint8_t led_signal_get(void);
const char* led_signal_name(uint8_t state);

#endif