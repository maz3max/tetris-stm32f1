#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>

#include <atomic>
#include <cstdlib>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "buttons.hpp"
#include "display.hpp"
#include "tetris.hpp"

#define PG_HEIGHT 16 // playground height
#define PG_WIDTH 8   // playground width

Tetris<PG_WIDTH, PG_HEIGHT> tetris;       // game logic object
SemaphoreHandle_t game_data_mutex = NULL; // mutex for accessing above object
std::atomic<bool> btn_states[NUM_BTNS][2] = {0}; // true if buttons are pressed + also contains old button states

// this task has to run very regularly to cycle between all the display dots
// it will only run if the game object is not in use (mutex)
void task_display_refresh(void *args __attribute__((unused))) {
  while (1) {
    if (xSemaphoreTake(game_data_mutex, (TickType_t)10) == pdTRUE) {
      auto *playground = tetris.get_playground();
      for (size_t y = 0; y < PG_HEIGHT; ++y) {
        for (size_t x = 0; x < PG_WIDTH; ++x) {
          draw_dot(y, x, playground[x][y]);
          __asm__("nop"); // wait for the pin status to take effect
        }
      }
      // just clear display
      draw_dot(0, 0, 0);
      xSemaphoreGive(game_data_mutex);
    }
    taskYIELD(); // run other tasks with same priority
  }
}

// this task reads the button states into a global variable where state[0] is the latest
// and state[1] is the last (/old) state
void task_check_buttons(void *args __attribute__((unused))) {
  while (1) {
    for (size_t i = 0; i < NUM_BTNS; i++) {
      btn_states[i][1] = static_cast<bool>(btn_states[i][0]); // copying the last button states to the "old" states
    }
    for (size_t i = 0; i < NUM_BTNS; ++i) {
      btn_states[i][0] = btn_pressed(i); // actualize current button state
    }
    taskYIELD(); // run other tasks with same priority
  }
}

// this task updates the game logic
// it is delayed with a fixed delay and can therefore get a higher priority.
// due to the higher priority, it will take precedence over the display task
// and can claim the game object mutex
void task_game_logic(void *args __attribute__((unused))) {
  while (1) {
    if (xSemaphoreTake(game_data_mutex, (TickType_t)10) == pdTRUE) {
      auto &status = tetris.get_status();
      status.left = btn_states[BTN_LEFT][0] && !btn_states[BTN_LEFT][1];
      status.right = btn_states[BTN_RIGHT][0] && !btn_states[BTN_RIGHT][1];
      status.down = btn_states[BTN_DOWN][0] && !btn_states[BTN_DOWN][1];
      status.rotCW = btn_states[BTN_A][0] && !btn_states[BTN_A][1];
      status.rotCCW = btn_states[BTN_B][0] && !btn_states[BTN_B][1];
      if (status.ending) {
        status.reset = btn_states[BTN_LEFT][0] || btn_states[BTN_RIGHT][0] ||
                       btn_states[BTN_DOWN][0] || btn_states[BTN_A][0] ||
                       btn_states[BTN_B][0];
      }
      tetris.tick();
      xSemaphoreGive(game_data_mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
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

  // initialize IO
  display_init();
  btn_init();

  // add tasks and start scheduler
  xTaskCreate(task_display_refresh, "display", 100, NULL, 1, NULL);
  xTaskCreate(task_check_buttons, "buttons", 100, NULL, 1, NULL);
  xTaskCreate(task_game_logic, "game", 100, NULL, 2, NULL);
  vTaskStartScheduler();

  // this should never be reached
  while (1)
    ;
}
