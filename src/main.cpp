#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <stdio.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#define PORT_LED GPIOA
#define PIN_LED GPIO8

void task_blink(void *args __attribute__((unused))) {
  while (1) {
    gpio_toggle(PORT_LED, PIN_LED);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

int main(void) {
  rcc_clock_setup_in_hsi_out_64mhz();
  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);

  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_set_mode(PORT_LED, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_PUSHPULL,
                PIN_LED);
  gpio_set(PORT_LED, PIN_LED);
  int *test_var = new (int);
  *test_var = 5;

  xTaskCreate(task_blink, "blink", 100, NULL, configMAX_PRIORITIES - 1, NULL);
  vTaskStartScheduler();

  while (1)
    ;
}
