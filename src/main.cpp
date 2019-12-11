#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <stdio.h>
#include <stdlib.h>

#include <FreeRTOS.h>
#include <task.h>

#include "display.hpp"

#define PORT_LED GPIOA
#define PIN_LED GPIO8

void task_blink(void *args __attribute__((unused))) {
  while (1) {
    for (int y = 0; y < 16; ++y){
      for(int x = 0; x < 8; ++x){
        draw_dot(y,x);
        __asm__("nop");
      }
    }
  }
}

int main(void) {
  // use the HSI clock and PLL to reach 64 MHz
  rcc_clock_setup_in_hsi_out_64mhz();

  // specify grouping of interrupt priorities (what part of the 8 bits is
  // allocated to index and subindex),  we use no subindex
  // https://www.freertos.org/RTOS-Cortex-M3-M4.html
  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);

  rcc_periph_clock_enable(RCC_AFIO);
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);

  display_init();

  xTaskCreate(task_blink, "blink", 100, NULL, configMAX_PRIORITIES - 1, NULL);
  vTaskStartScheduler();

  while (1)
    ;
}
