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
#include <iostream>
#include <string>
#include <vector>

#include "string_util.h"
#include "walltime.h"

// File format reference: http://edoceo.com/utilitas/csv-file-format.

namespace benchmark {

bool CSVReporter::ReportContext(const Context& context) {
  std::cerr << "Run on (" << context.num_cpus << " X " << context.mhz_per_cpu
            << " MHz CPU " << ((context.num_cpus > 1) ? "s" : "") << ")\n";

  std::cerr << LocalDateTimeString() << "\n";

  if (context.cpu_scaling_enabled) {
    std::cerr << "***WARNING*** CPU scaling is enabled, the benchmark "
                 "real time measurements may be noisy and will incure extra "
                 "overhead.\n";
  }

#ifndef NDEBUG
  std::cerr << "***WARNING*** Library was built as DEBUG. Timings may be "
               "affected.\n";
#endif
  std::cout << "name,iterations,real_time,cpu_time,bytes_per_second,"
               "items_per_second,label\n";
  return true;
}

void CSVReporter::ReportRuns(std::vector<Run> const& reports) {
  if (reports.empty()) {
    return;
  }

  std::vector<Run> reports_cp = reports;
  if (reports.size() >= 2) {
    Run mean_data;
    Run stddev_data;
    BenchmarkReporter::ComputeStats(reports, &mean_data, &stddev_data);
    reports_cp.push_back(mean_data);
    reports_cp.push_back(stddev_data);
  }
  for (auto it = reports_cp.begin(); it != reports_cp.end(); ++it) {
    PrintRunData(*it);
  }
}

void CSVReporter::PrintRunData(Run const& run) {
  double const multiplier = 1e9;  // nano second multiplier
  double cpu_time = run.cpu_accumulated_time * multiplier;
  double real_time = run.real_accumulated_time * multiplier;
  if (run.iterations != 0) {
    real_time = real_time / static_cast<double>(run.iterations);
    cpu_time = cpu_time / static_cast<double>(run.iterations);
  }

  // Field with embedded double-quote characters must be doubled and the field
  // delimited with double-quotes.
  std::string name = run.benchmark_name;
  ReplaceAll(&name, "\"", "\"\"");
  std::cout << "\"" << name << "\",";

  std::cout << run.iterations << ",";
  std::cout << real_time << ",";
  std::cout << cpu_time << ",";

  if (run.bytes_per_second > 0.0) {
    std::cout << run.bytes_per_second;
  }
  std::cout << ",";
  if (run.items_per_second > 0.0) {
    std::cout << run.items_per_second;
  }
  std::cout << ",";
  if (!run.report_label.empty()) {
    // Field with embedded double-quote characters must be doubled and the field
    // delimited with double-quotes.
    std::string label = run.report_label;
    ReplaceAll(&label, "\"", "\"\"");
    std::cout << "\"" << label << "\"";
  }
  std::cout << '\n';
}

}  // end namespace benchmark
