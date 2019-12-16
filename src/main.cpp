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

#include <libopencm3/stm32/timer.h>

#define PG_HEIGHT 16 // playground height
#define PG_WIDTH 8   // playground width

// define tone frequencys
#define A2 1760 // 440
#define G 1568  // 392
#define F 1396  // 349
#define E 1318  // 329
#define D 1174  // 293
#define C 1046  // 261
#define H 987   // 246
#define A 880   // 220
#define N 40000 

const int32_t TetrisMusic[64] = {
    E, E, H, C, D, D, C, H, A, A, A, C, E, E, D,  C,  H, H, H, C, D, D,
    E, E, C, C, A, A, A, A, N, N, D, D, D, F, A2, A2, G, F, E, E, E, C,
    E, E, D, C, H, H, H, C, D, D, E, E, C, C, A,  A,  A, A, N, N};

Tetris<PG_WIDTH, PG_HEIGHT> tetris;       // game logic object
SemaphoreHandle_t game_data_mutex = NULL; // mutex for accessing above object
std::atomic<bool> btn_states[NUM_BTNS][2] = {
    0}; // true if buttons are pressed + also contains old button states
std::atomic<bool> btn_flank_state[NUM_BTNS] = {
    0}; // contains rising edge button states
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

// this task reads the button states into a global variable where state[0] is
// the latest and state[1] is the last (/old) state
void task_check_buttons(void *args __attribute__((unused))) {
  while (1) {
    for (size_t i = 0; i < NUM_BTNS; i++) {
      btn_states[i][1] = static_cast<bool>(
          btn_states[i]
                    [0]); // copying the last button states to the "old" states
    }
    for (size_t i = 0; i < NUM_BTNS; ++i) {
      btn_states[i][0] = btn_pressed(i); // update current button state
    }
    for (size_t i = 0; i < NUM_BTNS; ++i) {
      if (btn_states[i][0] && !btn_states[i][1]) {
        btn_flank_state[i] = true;
      }
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

      status.left = btn_states[BTN_LEFT][0];
      status.right = btn_states[BTN_RIGHT][0];
      status.down = btn_states[BTN_DOWN][0];
      status.rotCW = btn_flank_state[BTN_A];
      status.rotCCW = btn_flank_state[BTN_B];

      if (status.ending) {
        status.reset = btn_flank_state[BTN_LEFT] ||
                       btn_flank_state[BTN_RIGHT] ||
                       btn_flank_state[BTN_DOWN] || btn_flank_state[BTN_A] ||
                       btn_flank_state[BTN_B];
      }

      for (size_t i = 0; i < NUM_BTNS; ++i) {
        btn_flank_state[i] = false;
      }

      tetris.tick();
      xSemaphoreGive(game_data_mutex);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void init_tetris_music() {
  uint32_t my_sick_tone = (8000000 / 21000) / 2;

  rcc_clock_setup_in_hsi_out_64mhz();
  /* Enable TIM1 clock. */
  rcc_periph_clock_enable(RCC_TIM2);

  /* Enable GPIOC, Alternate Function clocks. */
  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_AFIO);

  gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                GPIO_TIM2_CH3);

  // gpio_set_output_options(GPIOA, GPIO_OTYPE_PP,
  //                      GPIO_OSPEED_50MHZ, GPIO8 | GPIO9);
  timer_set_mode(TIM2, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_CENTER_1,
                 TIM_CR1_DIR_UP);
  timer_set_oc_mode(TIM2, TIM_OC3, TIM_OCM_PWM2);
  timer_enable_oc_output(TIM2, TIM_OC3);
  timer_enable_break_main_output(TIM2);
  timer_set_oc_value(TIM2, TIM_OC3, my_sick_tone);
  timer_set_prescaler(TIM2, 4);
  timer_set_period(TIM2, my_sick_tone * 2);
  timer_enable_counter(TIM2);
}

void task_music_update(void *args __attribute__((unused))){
  while (1) {
    for (int i = 0; i < 64; i++) {
      int my_sick_tone = (8000000 / TetrisMusic[i]) / 2;
      // timer_enable_break_main_output(TIM2);
      timer_set_period(TIM2, my_sick_tone * 2);
      timer_set_oc_value(TIM2, TIM_OC3, my_sick_tone);
      vTaskDelay(pdMS_TO_TICKS(200));
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

  // repurpose JTAG PINS for GPIO
  rcc_periph_clock_enable(RCC_AFIO);
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);

  // create mutex for game data
  game_data_mutex = xSemaphoreCreateMutex();
  configASSERT(game_data_mutex);

  // initialize IO
  display_init();
  btn_init();
  init_tetris_music();

  // add tasks and start scheduler
  xTaskCreate(task_display_refresh, "display", 100, NULL, 1, NULL);
  xTaskCreate(task_check_buttons, "buttons", 100, NULL, 1, NULL);
  xTaskCreate(task_game_logic, "game", 100, NULL, 2, NULL);
  xTaskCreate(task_music_update, "music", 100, NULL, 3, NULL);
  vTaskStartScheduler();

  // this should never be reached
  while (1)
    ;
}