// most of this is taken from the flash_rw example in libopencm3-examples:
// https://github.com/libopencm3/libopencm3-examples/tree/master/examples/stm32/f1/stm32-h107/flash_rw_example
#include "flash_rw.hpp"

#include <libopencm3/stm32/flash.h>

static uint32_t flash_program_data(uint32_t start_address, uint8_t *input_data,
                                   uint16_t num_elements);
static void flash_read_data(uint32_t start_address, uint16_t num_elements,
                            uint8_t *output_data);

uint8_t get_highscore() {
  uint8_t highscore[4];
  flash_read_data(FLASH_OPERATION_ADDRESS, 4, highscore);
  return highscore[0];
}

void set_highscore(uint8_t score) {
  uint8_t highscore[4] = {score, 0, 0, 0};
  flash_program_data(FLASH_OPERATION_ADDRESS, highscore, 4);
}

static uint32_t flash_program_data(uint32_t start_address, uint8_t *input_data,
                                   uint16_t num_elements) {
  uint16_t iter;
  uint32_t current_address = start_address;
  uint32_t page_address = start_address;
  uint32_t flash_status = 0;

  /*check if start_address is in proper range*/
  if ((start_address - FLASH_BASE) >=
      (FLASH_PAGE_SIZE * (FLASH_PAGE_NUM_MAX + 1)))
    return 1;

  /*calculate current page address*/
  if (start_address % FLASH_PAGE_SIZE)
    page_address -= (start_address % FLASH_PAGE_SIZE);

  flash_unlock();

  /*Erasing page*/
  flash_erase_page(page_address);
  flash_status = flash_get_status_flags();
  if (flash_status != FLASH_SR_EOP)
    return flash_status;

  /*programming flash memory*/
  for (iter = 0; iter < num_elements; iter += 4) {
    /*programming word data*/
    flash_program_word(current_address + iter,
                       *((uint32_t *)(input_data + iter)));
    flash_status = flash_get_status_flags();
    if (flash_status != FLASH_SR_EOP)
      return flash_status;

    /*verify if correct data is programmed*/
    if (*((uint32_t *)(current_address + iter)) !=
        *((uint32_t *)(input_data + iter)))
      return FLASH_WRONG_DATA_WRITTEN;
  }

  return 0;
}

static void flash_read_data(uint32_t start_address, uint16_t num_elements,
                            uint8_t *output_data) {
  uint16_t iter;
  uint32_t *memory_ptr = (uint32_t *)start_address;

  for (iter = 0; iter < num_elements / 4; iter++) {
    *(uint32_t *)output_data = *(memory_ptr + iter);
    output_data += 4;
  }
}