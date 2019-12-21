#pragma once

#include <cstdint>

#define SEND_BUFFER_SIZE 256
// this is after the 64kB of the STM32F103R8 so there will not be any program
// data luckily, we actually use a 128kB chip: the page should be writeable
#define FLASH_OPERATION_ADDRESS ((uint32_t)0x08010000)
#define FLASH_PAGE_NUM_MAX 255
#define FLASH_PAGE_SIZE 0x800
#define FLASH_WRONG_DATA_WRITTEN 0x80
#define RESULT_OK 0

uint8_t get_highscore();

void set_highscore(uint8_t score);