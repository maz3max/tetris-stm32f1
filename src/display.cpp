#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include "display.hpp"

static constexpr const uint32_t row_ports[DISPLAY_ROWS] = {
    GPIOA, // ROW1
    GPIOB, // ROW2
    GPIOA, // ROW3
    GPIOA, // ROW4
    GPIOB, // ROW5
    GPIOA, // ROW6
    GPIOB, // ROW7
    GPIOB, // ROW8
    GPIOA, // ROW9
    GPIOA, // ROW10
    GPIOB, // ROW11
    GPIOA, // ROW12
    GPIOA, // ROW13
    GPIOB, // ROW14
    GPIOB, // ROW15
    GPIOB  // ROW16
};
static constexpr const uint16_t row_pins[DISPLAY_ROWS] = {
    GPIO3,  // ROW1
    GPIO0,  // ROW2
    GPIO9,  // ROW3
    GPIO6,  // ROW4
    GPIO10, // ROW5
    GPIO8,  // ROW6
    GPIO11, // ROW7
    GPIO14, // ROW8
    GPIO10, // ROW9
    GPIO12, // ROW10
    GPIO6,  // ROW11
    GPIO11, // ROW12
    GPIO1,  // ROW13
    GPIO5,  // ROW14
    GPIO3,  // ROW15
    GPIO4   // ROW16
};
static constexpr const uint32_t col_ports[DISPLAY_COLS] = {
    GPIOA, // COL1
    GPIOB, // COL2
    GPIOB, // COL3
    GPIOA, // COL4
    GPIOB, // COL5
    GPIOA, // COL6
    GPIOB, // COL7
    GPIOB  // COL8
};
static constexpr const uint16_t col_pins[DISPLAY_COLS] = {
    GPIO7,  // COL1
    GPIO12, // COL2
    GPIO13, // COL3
    GPIO4,  // COL4
    GPIO15, // COL5
    GPIO5,  // COL6
    GPIO1,  // COL7
    GPIO2   // COL8
};

static constexpr uint16_t display_pins_rows(uint32_t port) {
  uint16_t result = 0;
  for (uint32_t i = 0; i < DISPLAY_ROWS; ++i) {
    if (row_ports[i] == port) {
      result |= row_pins[i];
    }
  }
  return result;
}

static constexpr uint16_t display_pins_cols(uint32_t port) {
  uint16_t result = 0;
  for (uint32_t i = 0; i < DISPLAY_COLS; ++i) {
    if (col_ports[i] == port) {
      result |= col_pins[i];
    }
  }
  return result;
}

static constexpr uint16_t display_pins(uint32_t port) {
  return display_pins_rows(port) | display_pins_cols(port);
}

__attribute__((always_inline)) inline static void clear_display() {
  gpio_clear(GPIOA, display_pins_rows(GPIOA));
  gpio_clear(GPIOB, display_pins_rows(GPIOB));
  gpio_set(GPIOA, display_pins_cols(GPIOA));
  gpio_set(GPIOB, display_pins_cols(GPIOB));
}

void draw_dot(uint8_t row, uint8_t col) {
  // turn off all leds
  clear_display();

  // turn on the wanted led
  gpio_set(row_ports[row], row_pins[row]);
  gpio_clear(col_ports[col], col_pins[col]);
}

void display_init() {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                display_pins(GPIOA));
  gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                display_pins(GPIOB));
  clear_display();
}
