#include "prng.hpp"

#include <random>

static std::mt19937 generator;

void prng_init() { generator.seed(0); }

uint32_t random(uint32_t top) {
  std::uniform_int_distribution<> dis(0, top - 1);
  return dis(generator);
}