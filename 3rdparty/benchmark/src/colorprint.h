#ifndef BENCHMARK_COLORPRINT_H_
#define BENCHMARK_COLORPRINT_H_

namespace benchmark {
enum LogColor {
  COLOR_DEFAULT,
  COLOR_RED,
  COLOR_GREEN,
  COLOR_YELLOW,
  COLOR_BLUE,
  COLOR_MAGENTA,
  COLOR_CYAN,
  COLOR_WHITE
};

void ColorPrintf(LogColor color, const char* fmt, ...);
}  // end namespace benchmark

#endif  // BENCHMARK_COLORPRINT_H_
