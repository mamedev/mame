// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_PERF_H_INCLUDED
#define ASMJIT_TEST_PERF_H_INCLUDED

#include <asmjit/core.h>
#include "asmjitutils.h"
#include "performancetimer.h"

class MyErrorHandler : public asmjit::ErrorHandler {
  void handleError(asmjit::Error err, const char* message, asmjit::BaseEmitter* origin) {
    (void)err;
    (void)origin;
    printf("ERROR: %s\n", message);
    abort();
  }
};

template<typename EmitterT, typename FuncT>
static void bench(asmjit::CodeHolder& code, asmjit::Arch arch, uint32_t numIterations, const char* testName, const FuncT& func) noexcept {
  EmitterT emitter;
  MyErrorHandler eh;

  const char* archName = asmjitArchAsString(arch);
  const char* emitterName =
    emitter.isAssembler() ? "Assembler" :
    emitter.isCompiler()  ? "Compiler"  :
    emitter.isBuilder()   ? "Builder"   : "Unknown";

  uint64_t codeSize = 0;
  asmjit::Environment env(arch);

  PerformanceTimer timer;
  double duration = std::numeric_limits<double>::infinity();

  for (uint32_t r = 0; r < numIterations; r++) {
    codeSize = 0;
    code.init(env);
    code.setErrorHandler(&eh);
    code.attach(&emitter);

    timer.start();
    func(emitter);
    timer.stop();

    codeSize += code.codeSize();

    code.reset();
    duration = asmjit::Support::min(duration, timer.duration());
  }

  printf("  [%s] %-9s %-16s | CodeSize:%5llu [B] | Time:%8.4f [ms]", archName, emitterName, testName, (unsigned long long)codeSize, duration);
  if (codeSize)
    printf(" | Speed:%8.3f [MB/s]", mbps(duration, codeSize));
  printf("\n");
}

#endif // ASMJIT_TEST_PERF_H_INCLUDED
