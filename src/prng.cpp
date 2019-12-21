#include "prng.hpp"

#include <random>

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

static std::mt19937 generator;

static void adc_setup(void) {
  rcc_periph_clock_enable(RCC_ADC1);

  /* Make sure the ADC doesn't run during config. */
  adc_power_off(ADC1);

  /* We configure everything for one single conversion. */
  adc_disable_scan_mode(ADC1);
  adc_set_single_conversion_mode(ADC1);
  adc_disable_external_trigger_regular(ADC1);
  adc_set_right_aligned(ADC1);
  /* We want to read the temperature sensor, so we have to enable it. */
  adc_enable_temperature_sensor();
  adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC);

  adc_power_on(ADC1);

  /* Wait for ADC starting up. */
  for (int i = 0; i < 800000; ++i) /* Wait a bit. */
    __asm__("nop");

  adc_reset_calibration(ADC1);
  adc_calibrate(ADC1);
}

static void gpio_setup(void) {
  /* Enable GPIO clocks. */
  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_set_mode(GPIOA, GPIO_MODE_INPUT, GPIO_CNF_INPUT_ANALOG, GPIO1);
}

void prng_init() {
  gpio_setup();
  adc_setup();

  /* Select the channel we want to convert. 1=PA1. */
  uint8_t channel_array[16];
  channel_array[0] = 1;
  adc_set_regular_sequence(ADC1, 1, channel_array);
  uint32_t seed = 0;
  for (size_t i = 0; i < 4; ++i) {
    adc_start_conversion_direct(ADC1);
    while (!(adc_eoc(ADC1)))
      ;
    // use only 8 bits of the ADC to receive much entropy
    uint8_t val = adc_read_regular(ADC1) & 0xFF;
    seed |= val;
    seed = seed << 8;
  }
  adc_power_off(ADC1);
  rcc_periph_clock_disable(RCC_ADC1);
  generator.seed(seed);
}

uint32_t random(uint32_t top) {
  std::uniform_int_distribution<> dis(0, top - 1);
  return dis(generator);
}