
#include "benchmark/benchmark.h"

#include <cassert>

class MyFixture : public ::benchmark::Fixture
{
public:
    void SetUp() {
        data = new int(42);
    }

    void TearDown() {
        assert(data != nullptr);
        delete data;
        data = nullptr;
    }

    ~MyFixture() {
      assert(data == nullptr);
    }

    int* data;
};


BENCHMARK_F(MyFixture, Foo)(benchmark::State& st) {
    assert(data != nullptr);
    assert(*data == 42);
    while (st.KeepRunning()) {
    }
}

BENCHMARK_DEFINE_F(MyFixture, Bar)(benchmark::State& st) {
  while (st.KeepRunning()) {
  }
  st.SetItemsProcessed(st.range_x());
}
BENCHMARK_REGISTER_F(MyFixture, Bar)->Arg(42);


BENCHMARK_MAIN()
