// Copyright 2015 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "colorprint.h"

#include <cstdarg>
#include <cstdio>

#include "commandlineflags.h"
#include "internal_macros.h"

#ifdef BENCHMARK_OS_WINDOWS
#include <Windows.h>
#endif

DECLARE_bool(color_print);

namespace benchmark {
namespace {
#ifdef BENCHMARK_OS_WINDOWS
typedef WORD PlatformColorCode;
#else
typedef const char* PlatformColorCode;
#endif

PlatformColorCode GetPlatformColorCode(LogColor color) {
#ifdef BENCHMARK_OS_WINDOWS
  switch (color) {
    case COLOR_RED:
      return FOREGROUND_RED;
    case COLOR_GREEN:
      return FOREGROUND_GREEN;
    case COLOR_YELLOW:
      return FOREGROUND_RED | FOREGROUND_GREEN;
    case COLOR_BLUE:
      return FOREGROUND_BLUE;
    case COLOR_MAGENTA:
      return FOREGROUND_BLUE | FOREGROUND_RED;
    case COLOR_CYAN:
      return FOREGROUND_BLUE | FOREGROUND_GREEN;
    case COLOR_WHITE:  // fall through to default
    default:
      return 0;
  }
#else
  switch (color) {
    case COLOR_RED:
      return "1";
    case COLOR_GREEN:
      return "2";
    case COLOR_YELLOW:
      return "3";
    case COLOR_BLUE:
      return "4";
    case COLOR_MAGENTA:
      return "5";
    case COLOR_CYAN:
      return "6";
    case COLOR_WHITE:
      return "7";
    default:
      return nullptr;
  };
#endif
}
}  // end namespace

void ColorPrintf(LogColor color, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  if (!FLAGS_color_print) {
    vprintf(fmt, args);
    va_end(args);
    return;
  }

#ifdef BENCHMARK_OS_WINDOWS
  const HANDLE stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

  // Gets the current text color.
  CONSOLE_SCREEN_BUFFER_INFO buffer_info;
  GetConsoleScreenBufferInfo(stdout_handle, &buffer_info);
  const WORD old_color_attrs = buffer_info.wAttributes;

  // We need to flush the stream buffers into the console before each
  // SetConsoleTextAttribute call lest it affect the text that is already
  // printed but has not yet reached the console.
  fflush(stdout);
  SetConsoleTextAttribute(stdout_handle,
                          GetPlatformColorCode(color) | FOREGROUND_INTENSITY);
  vprintf(fmt, args);

  fflush(stdout);
  // Restores the text color.
  SetConsoleTextAttribute(stdout_handle, old_color_attrs);
#else
  const char* color_code = GetPlatformColorCode(color);
  if (color_code) fprintf(stdout, "\033[0;3%sm", color_code);
  vprintf(fmt, args);
  printf("\033[m");  // Resets the terminal to default.
#endif
  va_end(args);
}
}  // end namespace benchmark
