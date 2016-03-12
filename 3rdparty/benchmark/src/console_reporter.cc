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

#include "benchmark/reporter.h"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "check.h"
#include "colorprint.h"
#include "string_util.h"
#include "walltime.h"

namespace benchmark {

bool ConsoleReporter::ReportContext(const Context& context) {
  name_field_width_ = context.name_field_width;

  std::cerr << "Run on (" << context.num_cpus << " X " << context.mhz_per_cpu
            << " MHz CPU " << ((context.num_cpus > 1) ? "s" : "") << ")\n";

  std::cerr << LocalDateTimeString() << "\n";

  if (context.cpu_scaling_enabled) {
    std::cerr << "***WARNING*** CPU scaling is enabled, the benchmark "
                 "real time measurements may be noisy and will incur extra "
                 "overhead.\n";
  }

#ifndef NDEBUG
  std::cerr << "***WARNING*** Library was built as DEBUG. Timings may be "
               "affected.\n";
#endif

  int output_width = fprintf(stdout, "%-*s %10s %10s %10s\n",
                             static_cast<int>(name_field_width_), "Benchmark",
                             "Time(ns)", "CPU(ns)", "Iterations");
  std::cout << std::string(output_width - 1, '-') << "\n";

  return true;
}

void ConsoleReporter::ReportRuns(const std::vector<Run>& reports) {
  if (reports.empty()) {
    return;
  }

  for (Run const& run : reports) {
    CHECK_EQ(reports[0].benchmark_name, run.benchmark_name);
    PrintRunData(run);
  }

  if (reports.size() < 2) {
    // We don't report aggregated data if there was a single run.
    return;
  }

  Run mean_data;
  Run stddev_data;
  BenchmarkReporter::ComputeStats(reports, &mean_data, &stddev_data);

  // Output using PrintRun.
  PrintRunData(mean_data);
  PrintRunData(stddev_data);
}

void ConsoleReporter::PrintRunData(const Run& result) {
  // Format bytes per second
  std::string rate;
  if (result.bytes_per_second > 0) {
    rate = StrCat(" ", HumanReadableNumber(result.bytes_per_second), "B/s");
  }

  // Format items per second
  std::string items;
  if (result.items_per_second > 0) {
    items = StrCat(" ", HumanReadableNumber(result.items_per_second),
                   " items/s");
  }

  double const multiplier = 1e9; // nano second multiplier
  ColorPrintf(COLOR_GREEN, "%-*s ",
              name_field_width_, result.benchmark_name.c_str());
  if (result.iterations == 0) {
    ColorPrintf(COLOR_YELLOW, "%10.0f %10.0f ",
                result.real_accumulated_time * multiplier,
                result.cpu_accumulated_time * multiplier);
  } else {
    ColorPrintf(COLOR_YELLOW, "%10.0f %10.0f ",
                (result.real_accumulated_time * multiplier) /
                    (static_cast<double>(result.iterations)),
                (result.cpu_accumulated_time * multiplier) /
                    (static_cast<double>(result.iterations)));
  }
  ColorPrintf(COLOR_CYAN, "%10lld", result.iterations);
  ColorPrintf(COLOR_DEFAULT, "%*s %*s %s\n",
              13, rate.c_str(),
              18, items.c_str(),
              result.report_label.c_str());
}

}  // end namespace benchmark
