#include "benchmark/benchmark.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>

#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {

class TestReporter : public benchmark::ConsoleReporter {
 public:
  virtual bool ReportContext(const Context& context) {
    return ConsoleReporter::ReportContext(context);
  };

  virtual void ReportRuns(const std::vector<Run>& report) {
    ++count_;
    ConsoleReporter::ReportRuns(report);
  };

  TestReporter() : count_(0) {}

  virtual ~TestReporter() {}

  size_t GetCount() const {
    return count_;
  }

 private:
  mutable size_t count_;
};

}  // end namespace


static void NoPrefix(benchmark::State& state) {
  while (state.KeepRunning()) {}
}
BENCHMARK(NoPrefix);

static void BM_Foo(benchmark::State& state) {
  while (state.KeepRunning()) {}
}
BENCHMARK(BM_Foo);


static void BM_Bar(benchmark::State& state) {
  while (state.KeepRunning()) {}
}
BENCHMARK(BM_Bar);


static void BM_FooBar(benchmark::State& state) {
  while (state.KeepRunning()) {}
}
BENCHMARK(BM_FooBar);


static void BM_FooBa(benchmark::State& state) {
  while (state.KeepRunning()) {}
}
BENCHMARK(BM_FooBa);



int main(int argc, char* argv[]) {
  benchmark::Initialize(&argc, argv);

  TestReporter test_reporter;
  benchmark::RunSpecifiedBenchmarks(&test_reporter);

  if (argc == 2) {
    // Make sure we ran all of the tests
    std::stringstream ss(argv[1]);
    size_t expected;
    ss >> expected;

    const size_t count = test_reporter.GetCount();
    if (count != expected) {
      std::cerr << "ERROR: Expected " << expected << " tests to be ran but only "
                << count << " completed" << std::endl;
      return -1;
    }
  }
  return 0;
}
