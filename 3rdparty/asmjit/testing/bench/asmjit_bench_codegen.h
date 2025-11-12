// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_PERF_H_INCLUDED
#define ASMJIT_TEST_PERF_H_INCLUDED

#include <asmjit/core.h>

#include "../commons/asmjitutils.h"
#include "../commons/performancetimer.h"

namespace asmjit_perf_utils {

class TestErrorHandler : public asmjit::ErrorHandler {
  void handle_error(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) override {
    (void)err;
    (void)origin;
    printf("ERROR: %s\n", message);
    abort();
  }
};

#ifndef ASMJIT_NO_BUILDER
template<typename BuilderT, typename FuncT>
static uint32_t calculate_instruction_count(asmjit::CodeHolder& code, asmjit::Arch arch, const FuncT& func) noexcept {
  BuilderT builder;
  TestErrorHandler eh;

  asmjit::Environment env(arch);
  code.init(env);
  code.set_error_handler(&eh);
  code.attach(&builder);
  func(builder);

  uint32_t count = 0;
  asmjit::BaseNode* node = builder.first_node();

  while (node) {
    count += uint32_t(node->is_inst());
    node = node->next();
  }

  code.reset();
  return count;
}
#endif

static inline double calculate_mbps(double duration_us, uint64_t output_size) noexcept {
  if (duration_us == 0)
    return 0.0;

  double bytes_total = double(output_size);
  return (bytes_total * 1000000) / (duration_us * 1024 * 1024);
}

static inline double calculate_mips(double duration, uint64_t instruction_count) noexcept {
  if (duration == 0)
    return 0.0;

  return double(instruction_count) * 1000000.0 / (duration * 1e6);
}

template<typename EmitterT, typename FuncT>
static void bench(asmjit::CodeHolder& code, asmjit::Arch arch, uint32_t num_iterations, const char* test_name, uint32_t instruction_count, const FuncT& func) noexcept {
  EmitterT emitter;
  TestErrorHandler eh;

  const char* arch_name = asmjit_arch_as_string(arch);
  const char* emitter_name =
    emitter.is_assembler() ? "Assembler" :
    emitter.is_compiler()  ? "Compiler"  :
    emitter.is_builder()   ? "Builder"   : "Unknown";

  uint64_t code_size = 0;
  asmjit::Environment env(arch);

  PerformanceTimer timer;
  double duration = std::numeric_limits<double>::infinity();

  code.init(env);
  code.set_error_handler(&eh);
  code.attach(&emitter);

  for (uint32_t r = 0; r < num_iterations; r++) {
    code_size = 0;

    timer.start();
    func(emitter);
    code_size += code.code_size();

    code.reinit();
    timer.stop();

    duration = asmjit::Support::min(duration, timer.duration() * 1000);
  }

  printf("  [%-7s] %-9s %-16s | CodeSize:%5llu [B] | Time:%7.3f [us]", arch_name, emitter_name, test_name, (unsigned long long)code_size, duration);
  if (code_size) {
    printf(" | Speed:%7.1f [MiB/s]", calculate_mbps(duration, code_size));
  }
  else {
    printf(" | Speed:    N/A        ");
  }

  if (instruction_count) {
    printf(", %8.1f [MInst/s]", calculate_mips(duration, instruction_count));
  }

  printf("\n");
  code.reset();
}

} // {asmjit_perf_utils}

#endif // ASMJIT_TEST_PERF_H_INCLUDED
