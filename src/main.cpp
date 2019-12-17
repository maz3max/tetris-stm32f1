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
#include "matrixFont.h"

#define PG_HEIGHT 16 // playground height
#define PG_WIDTH 8   // playground width


Tetris<PG_WIDTH, PG_HEIGHT> tetris;       // game logic object
SemaphoreHandle_t game_data_mutex = NULL; // mutex for accessing above object
std::atomic<bool> btn_states[NUM_BTNS][2] = {0}; // true if buttons are pressed + also contains old button states
std::atomic<bool> btn_flank_state[NUM_BTNS] = {0}; // contains rising edge button states
// this task has to run very regularly to cycle between all the display dots
// it will only run if the game object is not in use (mutex)
void task_display_refresh(void *args __attribute__((unused))) {
  while (1) {
    if (xSemaphoreTake(game_data_mutex, (TickType_t)10) == pdTRUE) {
      auto *playground = tetris.get_playground();
      for (size_t y = 0; y < PG_HEIGHT / 2; ++y) {
        for (size_t x = 0; x < PG_WIDTH; ++x) {
          double_draw_dot(y, x, playground[x][y],
                          playground[x][y + (PG_HEIGHT / 2)]);
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
      btn_states[i][1] = static_cast<bool>(btn_states[i][0]); // copying the last button states to the "old" states
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
    int score = tetris.get_score();
    vTaskDelay(pdMS_TO_TICKS(100-(score * 5)));
  }
}

// this funktion takes two numbers between 0 and 99 and displays them one on the top matrix and an other on the bottom matrix
void draw_number_test(int nmb_top, int nmb_bot){
  if(nmb_top>99 || nmb_bot>99){return;}
  int a = nmb_top / 10;
  int b = nmb_top - (a*10) ;
  int a_bot = nmb_bot / 10 ;
  int b_bot = nmb_bot - (a_bot*10) ;
  uint8_t* numberarray[10] = {ze, on, tw, th, fo, fi, si, se, ei, ni};
  
  while(1) {
    for(uint8_t y = 0; y< PG_HEIGHT ; y++){
      for(uint8_t x = 0; x< PG_WIDTH; x++){
        if(x<3 && y<8){
          uint8_t n = numberarray[a][x] ;
          uint8_t bit = 1<<(7-y);
          if((bit & n)>0){
            draw_dot(y, x, 1);
          } 
          else{
            draw_dot(y, x, 0);
          }
          //draw_dot(y, x, 1);
        }
        else if(x>3 && x<7 && y<8){
          uint8_t n =numberarray[b][(x-4)] ;
          uint8_t bit = 1<<(7-y);
          if((bit & n)>0){
            draw_dot(y, x, 1);
          } 
          else{
            draw_dot(y, x, 0);
          }
          //draw_dot(y, x, 1);
         }
        else if(x<3 && y>7){
          uint8_t n = numberarray[a_bot][x] ;
          uint8_t bit = 1<<(15-y);
          if((bit & n)>0){
            draw_dot(y, x, 1);
          } 
          else{
            draw_dot(y, x, 0);
          }
        }
        else if(x>3 && x<7 && y>7){
          uint8_t n =numberarray[b_bot][(x-4)] ;
          uint8_t bit = 1<<(15-y);
          if((bit & n)>0){
            draw_dot(y, x, 1);
          } 
          else{
            draw_dot(y, x, 0);
          }
          //draw_dot(y, x, 1);
         }
        draw_dot(15,7,0);
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

  // repurpose JTAG PINS for GPIO
  rcc_periph_clock_enable(RCC_AFIO);
  gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, 0);

  // create mutex for game data
  game_data_mutex = xSemaphoreCreateMutex();
  configASSERT(game_data_mutex);

  // initialize IO
  display_init();
  btn_init();

  // test stuff befor thred stuff 
  draw_number_test(42,69);

  // add tasks and start scheduler
  xTaskCreate(task_display_refresh, "display", 100, NULL, 1, NULL);
  xTaskCreate(task_check_buttons, "buttons", 100, NULL, 1, NULL);
  xTaskCreate(task_game_logic, "game", 100, NULL, 2, NULL);
  vTaskStartScheduler();

  // this should never be reached
  while (1)
    ;
}
