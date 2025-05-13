
#include <filesystem>
#include <chrono>
#include <ctime>

#include "c74_min_api.h"


using namespace c74::min;
using namespace c74::min::ui;

unsigned power_ceil(unsigned x) {
  if (x <= 1)
    return 1;
  int power = 2;
  x--;
  while (x >>= 1)
    power <<= 1;
  return power;
}
