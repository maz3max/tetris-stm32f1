#pragma once

#include <cstdint>

// initialise random number generator
void prng_init();

// return value uniformly distributed in [0, top)
uint32_t random(uint32_t top);