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

#include <cstdlib>
#include <vector>

#include "check.h"
#include "stat.h"

namespace benchmark {

void BenchmarkReporter::ComputeStats(
    const std::vector<Run>& reports,
    Run* mean_data, Run* stddev_data) {
  CHECK(reports.size() >= 2) << "Cannot compute stats for less than 2 reports";
  // Accumulators.
  Stat1_d real_accumulated_time_stat;
  Stat1_d cpu_accumulated_time_stat;
  Stat1_d bytes_per_second_stat;
  Stat1_d items_per_second_stat;
  // All repetitions should be run with the same number of iterations so we
  // can take this information from the first benchmark.
  int64_t const run_iterations = reports.front().iterations;

  // Populate the accumulators.
  for (Run const& run : reports) {
    CHECK_EQ(reports[0].benchmark_name, run.benchmark_name);
    CHECK_EQ(run_iterations, run.iterations);
    real_accumulated_time_stat +=
        Stat1_d(run.real_accumulated_time/run.iterations, run.iterations);
    cpu_accumulated_time_stat +=
        Stat1_d(run.cpu_accumulated_time/run.iterations, run.iterations);
    items_per_second_stat += Stat1_d(run.items_per_second, run.iterations);
    bytes_per_second_stat += Stat1_d(run.bytes_per_second, run.iterations);
  }

  // Get the data from the accumulator to BenchmarkReporter::Run's.
  mean_data->benchmark_name = reports[0].benchmark_name + "_mean";
  mean_data->iterations = run_iterations;
  mean_data->real_accumulated_time = real_accumulated_time_stat.Mean() *
                                     run_iterations;
  mean_data->cpu_accumulated_time = cpu_accumulated_time_stat.Mean() *
                                    run_iterations;
  mean_data->bytes_per_second = bytes_per_second_stat.Mean();
  mean_data->items_per_second = items_per_second_stat.Mean();

  // Only add label to mean/stddev if it is same for all runs
  mean_data->report_label = reports[0].report_label;
  for (std::size_t i = 1; i < reports.size(); i++) {
    if (reports[i].report_label != reports[0].report_label) {
      mean_data->report_label = "";
      break;
    }
  }

  stddev_data->benchmark_name = reports[0].benchmark_name + "_stddev";
  stddev_data->report_label = mean_data->report_label;
  stddev_data->iterations = 0;
  stddev_data->real_accumulated_time =
      real_accumulated_time_stat.StdDev();
  stddev_data->cpu_accumulated_time =
      cpu_accumulated_time_stat.StdDev();
  stddev_data->bytes_per_second = bytes_per_second_stat.StdDev();
  stddev_data->items_per_second = items_per_second_stat.StdDev();
}

TimeUnitMultiplier BenchmarkReporter::GetTimeUnitAndMultiplier(TimeUnit unit) {
  switch (unit) {
    case kMillisecond:
      return std::make_pair("ms", 1e3);
    case kMicrosecond:
      return std::make_pair("us", 1e6);
    case kNanosecond:
    default:
      return std::make_pair("ns", 1e9);
  }
}

void BenchmarkReporter::Finalize() {
}

BenchmarkReporter::~BenchmarkReporter() {
}

} // end namespace benchmark
