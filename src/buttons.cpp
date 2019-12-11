#include "buttons.hpp"

#include <FreeRTOS.h>
#include <task.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
static constexpr const uint32_t btns_ports[NUM_BTNS] = {GPIOA, GPIOA, GPIOB,
                                                        GPIOB, GPIOB};

static constexpr const uint32_t btns_pins[NUM_BTNS] = {GPIO0, GPIO15, GPIO7,
                                                       GPIO8, GPIO9};

static constexpr uint16_t btn_all_pins(uint32_t port) {
  uint16_t result = 0;
  for (uint32_t i = 0; i < NUM_BTNS; ++i) {
    if (btns_ports[i] == port) {
      result |= btns_pins[i];
    }
  }
  return result;
}

bool btn_pressed (size_t num) {
  configASSERT(num < 5);
  return !gpio_get(btns_ports[num], btns_pins[num]);
}

void btn_init() {
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOB);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
                btn_all_pins(GPIOA));
  gpio_set_mode(GPIOB, GPIO_MODE_INPUT, GPIO_CNF_INPUT_PULL_UPDOWN,
                btn_all_pins(GPIOB));
  gpio_set(GPIOA, btn_all_pins(GPIOA));
  gpio_set(GPIOB, btn_all_pins(GPIOB));
}