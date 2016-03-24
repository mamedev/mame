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
#ifndef BENCHMARK_REPORTER_H_
#define BENCHMARK_REPORTER_H_

#include <string>
#include <utility>
#include <vector>

#include "benchmark_api.h" // For forward declaration of BenchmarkReporter

namespace benchmark {

// Interface for custom benchmark result printers.
// By default, benchmark reports are printed to stdout. However an application
// can control the destination of the reports by calling
// RunSpecifiedBenchmarks and passing it a custom reporter object.
// The reporter object must implement the following interface.
class BenchmarkReporter {
 public:
  struct Context {
    int num_cpus;
    double mhz_per_cpu;
    bool cpu_scaling_enabled;

    // The number of chars in the longest benchmark name.
    size_t name_field_width;
  };

  struct Run {
    Run() :
      iterations(1),
      real_accumulated_time(0),
      cpu_accumulated_time(0),
      bytes_per_second(0),
      items_per_second(0),
      max_heapbytes_used(0) {}

    std::string benchmark_name;
    std::string report_label;  // Empty if not set by benchmark.
    int64_t iterations;
    double real_accumulated_time;
    double cpu_accumulated_time;

    // Zero if not set by benchmark.
    double bytes_per_second;
    double items_per_second;

    // This is set to 0.0 if memory tracing is not enabled.
    double max_heapbytes_used;
  };

  // Called once for every suite of benchmarks run.
  // The parameter "context" contains information that the
  // reporter may wish to use when generating its report, for example the
  // platform under which the benchmarks are running. The benchmark run is
  // never started if this function returns false, allowing the reporter
  // to skip runs based on the context information.
  virtual bool ReportContext(const Context& context) = 0;

  // Called once for each group of benchmark runs, gives information about
  // cpu-time and heap memory usage during the benchmark run.
  // Note that all the grouped benchmark runs should refer to the same
  // benchmark, thus have the same name.
  virtual void ReportRuns(const std::vector<Run>& report) = 0;

  // Called once and only once after ever group of benchmarks is run and
  // reported.
  virtual void Finalize();

  virtual ~BenchmarkReporter();
protected:
    static void ComputeStats(std::vector<Run> const& reports, Run* mean, Run* stddev);
};

// Simple reporter that outputs benchmark data to the console. This is the
// default reporter used by RunSpecifiedBenchmarks().
class ConsoleReporter : public BenchmarkReporter {
 public:
  virtual bool ReportContext(const Context& context);
  virtual void ReportRuns(const std::vector<Run>& reports);
protected:
  virtual void PrintRunData(const Run& report);

  size_t name_field_width_;
};

class JSONReporter : public BenchmarkReporter {
public:
  JSONReporter() : first_report_(true) {}
  virtual bool ReportContext(const Context& context);
  virtual void ReportRuns(const std::vector<Run>& reports);
  virtual void Finalize();

private:
  void PrintRunData(const Run& report);

  bool first_report_;
};

class CSVReporter : public BenchmarkReporter {
public:
  virtual bool ReportContext(const Context& context);
  virtual void ReportRuns(const std::vector<Run>& reports);

private:
  void PrintRunData(const Run& report);
};

} // end namespace benchmark
#endif // BENCHMARK_REPORTER_H_
