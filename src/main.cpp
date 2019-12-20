#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/flash.h>

#include <atomic>
#include <cstdlib>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

#include "buttons.hpp"
#include "display.hpp"
#include "matrixFont.h"
#include "tetris.hpp"

#define PG_HEIGHT 16 // playground height
#define PG_WIDTH 8   // playground width

#define SEND_BUFFER_SIZE 256
#define FLASH_OPERATION_ADDRESS ((uint32_t)0x0800f000)
#define FLASH_PAGE_NUM_MAX 127
#define FLASH_PAGE_SIZE 0x800
#define FLASH_WRONG_DATA_WRITTEN 0x80
#define RESULT_OK 0

Tetris<PG_WIDTH, PG_HEIGHT> tetris;       // game logic object
SemaphoreHandle_t game_data_mutex = NULL; // mutex for accessing above object
SemaphoreHandle_t score_mutex = NULL;
std::atomic<bool> btn_states[NUM_BTNS] = {0}; // true if buttons are pressed + also contains old button states
std::atomic<bool> btn_flank_state[NUM_BTNS] = {0}; // contains rising edge button states
std::atomic<bool> score_display[PG_HEIGHT][PG_WIDTH] = {0}; //contains the numbers as dotmatrix to display
uint8_t highscore[4] = {0}; 

// this task has to run very regularly to cycle between all the display dots
// it will only run if the game object is not in use (mutex)
void task_display_refresh(void *args __attribute__((unused))) {
  while (1) {
    if (xSemaphoreTake(game_data_mutex, (TickType_t)10) == pdTRUE) {
      if (tetris.get_game_over_status()) {
        for (size_t y = 0; y < PG_HEIGHT / 2; ++y) {
          for (size_t x = 0; x < PG_WIDTH; ++x) {
            double_draw_dot(y, x, score_display[y][x],
                            score_display[y + (PG_HEIGHT / 2)][x]);
            __asm__("nop"); // wait for the pin status to take effect
          }
        }
      } else {
        auto *playground = tetris.get_playground();
        for (size_t y = 0; y < PG_HEIGHT / 2; ++y) {
          for (size_t x = 0; x < PG_WIDTH; ++x) {
            double_draw_dot(y, x, playground[x][y],
                            playground[x][y + (PG_HEIGHT / 2)]);
            __asm__("nop"); // wait for the pin status to take effect
          }
        }
      }
      // just clear display
      draw_dot(0, 0, 0);
      xSemaphoreGive(game_data_mutex);
    }
    taskYIELD(); // run other tasks with same priority
  }
}

// this task reads the button states into a global variable
// debounced button states are stored in btn_states
// positive flanks are stored in btn_flank_state
// debouncing is performed using a delta since the last change
// min_delta_ticks can be used to adjust debouncing aggressivity
void task_check_buttons(void *args __attribute__((unused))) {
  const TickType_t min_delta_ticks = pdMS_TO_TICKS(50);
  static TickType_t last_change[NUM_BTNS];
  static bool last_state[NUM_BTNS];
  while (1) {
    for (size_t i = 0; i < NUM_BTNS; i++) {
      bool current_state = btn_pressed(i);
      if (current_state != last_state[i]) {
        last_change[i] = xTaskGetTickCount();
        last_state[i] = current_state;
      } else {
        auto delta = xTaskGetTickCount() - last_change[i];
        if (delta > min_delta_ticks) {
          if (static_cast<bool>(btn_states[i]) != current_state) {
            btn_states[i] = current_state;
            if (current_state) {
              btn_flank_state[i] = true;
            }
          }
        }
      }
    }
    taskYIELD(); // run other tasks with same priority
  }
}


static uint32_t flash_program_data(uint32_t start_address, uint8_t *input_data, uint16_t num_elements)
{
	uint16_t iter;
	uint32_t current_address = start_address;
	uint32_t page_address = start_address;
	uint32_t flash_status = 0;

	/*check if start_address is in proper range*/
	if((start_address - FLASH_BASE) >= (FLASH_PAGE_SIZE * (FLASH_PAGE_NUM_MAX+1)))
		return 1;

	/*calculate current page address*/
	if(start_address % FLASH_PAGE_SIZE)
		page_address -= (start_address % FLASH_PAGE_SIZE);

	flash_unlock();

	/*Erasing page*/
	flash_erase_page(page_address);
	flash_status = flash_get_status_flags();
	if(flash_status != FLASH_SR_EOP)
		return flash_status;

	/*programming flash memory*/
	for(iter=0; iter<num_elements; iter += 4)
	{
		/*programming word data*/
		flash_program_word(current_address+iter, *((uint32_t*)(input_data + iter)));
		flash_status = flash_get_status_flags();
		if(flash_status != FLASH_SR_EOP)
			return flash_status;

		/*verify if correct data is programmed*/
		if(*((uint32_t*)(current_address+iter)) != *((uint32_t*)(input_data + iter)))
			return FLASH_WRONG_DATA_WRITTEN;
	}

	return 0;
}


static void flash_read_data(uint32_t start_address, uint16_t num_elements, uint8_t *output_data)
{
	uint16_t iter;
	uint32_t *memory_ptr= (uint32_t*)start_address;

	for(iter=0; iter<num_elements/4; iter++)
	{
		*(uint32_t*)output_data = *(memory_ptr + iter);
		output_data += 4;
	}
}


void draw_number_field(int nmb_top, int nmb_bot) {
  if (nmb_top > 99 || nmb_bot > 99) {
    return;
  }
  int a = nmb_top / 10;
  int b = nmb_top - (a * 10);
  int a_bot = nmb_bot / 10;
  int b_bot = nmb_bot - (a_bot * 10);
  uint8_t *numberarray[10] = {ze, on, tw, th, fo, fi, si, se, ei, ni};
  if (xSemaphoreTake(score_mutex, (TickType_t)10) == pdTRUE) {
    for (uint8_t y = 0; y < PG_HEIGHT; y++) {
      for (uint8_t x = 0; x < PG_WIDTH; x++) {
        if (x < 3 && y < 8) {
          uint8_t n = numberarray[a][x];
          uint8_t bit = 1 << (7 - y);
          if ((bit & n) > 0) {
            score_display[y][x] = 1;
          } else {
            score_display[y][x] = 0;
          }
          // draw_dot(y, x, 1);
        } else if (x > 3 && x < 7 && y < 8) {
          uint8_t n = numberarray[b][(x - 4)];
          uint8_t bit = 1 << (7 - y);
          if ((bit & n) > 0) {
            score_display[y][x] = 1;
          } else {
            score_display[y][x] = 0;
          }
          // draw_dot(y, x, 1);
        } else if (x < 3 && y > 7) {
          uint8_t n = numberarray[a_bot][x];
          uint8_t bit = 1 << (15 - y);
          if ((bit & n) > 0) {
            score_display[y][x] = 1;
          } else {
            score_display[y][x] = 0;
          }
        } else if (x > 3 && x < 7 && y > 7) {
          uint8_t n = numberarray[b_bot][(x - 4)];
          uint8_t bit = 1 << (15 - y);
          if ((bit & n) > 0) {
            score_display[y][x] = 1;
          } else {
            score_display[y][x] = 0;
          }
        }
      }
    }
    xSemaphoreGive(score_mutex);
  }
}

// this task updates the game logic
// it is delayed with a fixed delay and can therefore get a higher priority.
// due to the higher priority, it will take precedence over the display task
// and can claim the game object mutex
void task_game_logic(void *args __attribute__((unused))) {
  while (1) {
    uint8_t score = tetris.get_score();
    if (xSemaphoreTake(game_data_mutex, (TickType_t)10) == pdTRUE) {
      auto &status = tetris.get_status();

      status.left = btn_states[BTN_LEFT];
      status.right = btn_states[BTN_RIGHT];
      status.down = btn_states[BTN_DOWN];
      status.rotCW = btn_flank_state[BTN_A];
      status.rotCCW = btn_flank_state[BTN_B];

      if (status.ending) {
        flash_read_data(FLASH_OPERATION_ADDRESS, 4, highscore);
        if(score > highscore[0]){
          highscore[0] = score;
          flash_program_data(FLASH_OPERATION_ADDRESS, highscore, 4);
        }
        draw_number_field(score, highscore[0]);
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
    int new_delay = 100 - (score * 5);
    if (new_delay < 20) {
      new_delay = 20;
    }
    vTaskDelay(pdMS_TO_TICKS(new_delay));
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
  score_mutex = xSemaphoreCreateMutex();

  configASSERT(game_data_mutex);
  configASSERT(score_mutex);

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
