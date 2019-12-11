#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <cstdlib>
#include <atomic>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "display.hpp"
#include "buttons.hpp"
#include "tetris.hpp"

Tetris<8, 16> tetris;
SemaphoreHandle_t game_data_mutex = NULL;
std::atomic<bool> btn_states[NUM_BTNS] = {0};

void task_display_refresh(void *args __attribute__((unused))) {
  while (1) {
    if (xSemaphoreTake(game_data_mutex, (TickType_t)10) == pdTRUE) {
      for (int y = 0; y < 16; ++y) {
        for (int x = 0; x < 8; ++x) {
          draw_dot(y, x, (x + y * 9) % 2 == 0);
          __asm__("nop");
        }
      }
      // just clear display
      draw_dot(0, 0, 0);
      xSemaphoreGive(game_data_mutex);
    }
    taskYIELD();
  }
}

void task_check_buttons(void *args __attribute__((unused))) {
  while (1) {
    for (size_t i = 0; i < NUM_BTNS; ++i){
      btn_states[i] = btn_pressed(i);
    }
    taskYIELD();
  }
}

int main(void) {
  // use the HSI clock and PLL to reach 64 MHz
  rcc_clock_setup_in_hsi_out_64mhz();

  // specify grouping of interrupt priorities (what part of the 8 bits is
  // allocated to index and subindex),  we use no subindex
  // https://www.freertos.org/RTOS-Cortex-M3-M4.html
  scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP16_NOSUB);

  // repurpose JTAG PINS for GPIO
  rcc_periph_clock_enable(RCC_AFIO);
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);

  // create mutex for game data
  game_data_mutex = xSemaphoreCreateMutex();
  configASSERT(game_data_mutex);

  display_init();
  btn_init();

  xTaskCreate(task_display_refresh, "display", 100, NULL, 1, NULL);
  xTaskCreate(task_check_buttons, "buttons", 100, NULL, 1, NULL);
  vTaskStartScheduler();

  while (1)
    ;
}
