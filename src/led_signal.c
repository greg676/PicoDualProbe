#include "led_signal.h"
#include "ws2812.pio.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "hardware/timer.h"

#define WS2812_PIN 16
#define TIMER_MS    10
#define TIMER_US    (TIMER_MS * 1000)

// Each surge: 100 ticks @ 10ms = 1.0 second, 0% -> 50% -> 0%
#define SURGE_TICKS     100
// Gap between surges: 200 ms
#define GAP_TICKS       20
// Rest after last surge before repeating: 600 ms
#define REST_TICKS      60

// Color palette (GRB, dimmed)
typedef enum {
    C_OFF    = 0,
    C_BLUE   = 1,
    C_RED    = 2,
    C_GREEN  = 3,
    C_YELLOW = 4,
    C_WHITE  = 5,
} color_t;

static const uint32_t color_grb[] = {
    [C_OFF]    = 0x000000,
    [C_BLUE]   = 0x004000,  // G=0, R=0, B=64
    [C_RED]    = 0x004000,  // G=0, R=64, B=0
    [C_GREEN]  = 0x400000,  // G=64, R=0, B=0
    [C_YELLOW] = 0x404000,  // G=64, R=64, B=0
    [C_WHITE]  = 0x202020,  // G=32, R=32, B=32
};

// State table: 10 entries (0 unused, 1..9 valid)
// Each state defines the sequence of surges (1, 2, or 3 surges).
static const struct {
    uint8_t surge_count;
    color_t surges[3];
} states[LED_SIGNAL_STATE_COUNT] = {
    {0, {C_OFF,   C_OFF,   C_OFF}},  // 0  off
    {2, {C_BLUE,  C_BLUE,  C_OFF}},  // 1  blue-blue
    {2, {C_RED,   C_RED,   C_OFF}},  // 2  red-red
    {2, {C_GREEN, C_GREEN, C_OFF}},  // 3  green-green
    {2, {C_YELLOW,C_YELLOW,C_OFF}},  // 4  yellow-yellow
    {2, {C_WHITE, C_WHITE, C_OFF}},  // 5  white-white
    {2, {C_BLUE,  C_RED,   C_OFF}},  // 6  blue-red
    {2, {C_BLUE,  C_GREEN, C_OFF}},  // 7  blue-green
    {2, {C_BLUE,  C_YELLOW,C_OFF}},  // 8  blue-yellow
    {2, {C_BLUE,  C_WHITE, C_OFF}},  // 9  blue-white
};

static const char* state_names[LED_SIGNAL_STATE_COUNT] = {
    "off",
    "blue-blue",
    "red-red",
    "green-green",
    "yellow-yellow",
    "white-white",
    "blue-red",
    "blue-green",
    "blue-yellow",
    "blue-white",
};

static PIO led_pio;
static uint led_sm;
static volatile uint8_t current_state = 1;  // default state 1
static volatile uint8_t next_state = 1;
static volatile uint32_t tick = 0;

static inline uint32_t grb(uint8_t g, uint8_t r, uint8_t b) {
    return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(led_pio, led_sm, pixel_grb << 8u);
}

// Compute surge intensity 0..64 over ticks 0..SURGE_TICKS-1
static uint8_t surge_intensity(uint32_t t) {
    if (t >= SURGE_TICKS) return 0;
    uint32_t half = SURGE_TICKS / 2;
    uint32_t v;
    if (t < half) {
        v = (t * 64) / half;
    } else {
        v = ((SURGE_TICKS - t) * 64) / half;
    }
    return (uint8_t)v;
}

// Total ticks for one complete cycle of a state
static uint32_t cycle_ticks(uint8_t state_idx) {
    uint8_t n = states[state_idx].surge_count;
    return (n * SURGE_TICKS) + ((n - 1) * GAP_TICKS) + REST_TICKS;
}

static void update_led(void) {
    uint8_t state_idx = current_state;

    // State 0 = LED off
    if (state_idx == 0) {
        put_pixel(0);
        return;
    }

    uint32_t cyc = cycle_ticks(state_idx);
    uint32_t t = tick % cyc;

    uint8_t n = states[state_idx].surge_count;
    for (uint8_t i = 0; i < n; i++) {
        uint32_t seg_start = i * (SURGE_TICKS + GAP_TICKS);
        uint32_t seg_end = seg_start + SURGE_TICKS;
        if (t >= seg_start && t < seg_end) {
            uint32_t st = t - seg_start;
            uint8_t intensity = surge_intensity(st);
            color_t c = states[state_idx].surges[i];
            if (c == C_BLUE) {
                put_pixel(grb(0, 0, intensity));
            } else if (c == C_RED) {
                put_pixel(grb(0, intensity, 0));
            } else if (c == C_GREEN) {
                put_pixel(grb(intensity, 0, 0));
            } else if (c == C_YELLOW) {
                put_pixel(grb(intensity, intensity, 0));
            } else if (c == C_WHITE) {
                put_pixel(grb(intensity / 2, intensity / 2, intensity / 2));
            }
            return;
        }
    }

    // In a gap or rest period: LED off
    put_pixel(0);
}

static void timer_callback(void) {
    hw_clear_bits(&timer_hw->intr, 1u << 0);
    timer_hw->alarm[0] = timer_hw->timerawl + TIMER_US;

    if (current_state != next_state) {
        current_state = next_state;
        tick = 0;
    }

    update_led();
    tick++;
}

void led_signal_init(void) {
    led_pio = pio0;
    led_sm = 1;  // SM1 — SM0 is used by the probe PIO
    uint offset = pio_add_program(led_pio, &ws2812_program);
    ws2812_program_init(led_pio, led_sm, offset, WS2812_PIN, 800000, false);

    irq_set_exclusive_handler(TIMER_IRQ_0, timer_callback);
    irq_set_enabled(TIMER_IRQ_0, true);
    timer_hw->alarm[0] = timer_hw->timerawl + TIMER_US;
    hw_set_bits(&timer_hw->inte, 1u << 0);
}

void led_signal_set(uint8_t state) {
    if (state <= LED_SIGNAL_STATE_MAX) {
        next_state = state;
    }
}

uint8_t led_signal_get(void) {
    return current_state;
}

const char* led_signal_name(uint8_t state) {
    if (state < LED_SIGNAL_STATE_COUNT) {
        return state_names[state];
    }
    return "invalid";
}