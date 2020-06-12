// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <asmjit/x86.h>
#include <stdio.h>
#include <string.h>

#include "./asmjit_test_opcode.h"

#ifndef ASMJIT_NO_COMPILER
  #include "./asmjit_test_misc.h"
#endif

using namespace asmjit;

// ============================================================================
// [Configuration]
// ============================================================================

static constexpr uint32_t kNumRepeats = 20;
static constexpr uint32_t kNumIterations = 1000;

// ============================================================================
// [BenchUtils]
// ============================================================================

namespace BenchUtils {
  class Performance {
  public:
    inline Performance() noexcept { reset(); }

    inline void reset() noexcept {
      tick = 0u;
      best = 0xFFFFFFFFu;
    }

    inline uint32_t start() noexcept { return (tick = now()); }
    inline uint32_t diff() const noexcept { return now() - tick; }

    inline uint32_t end() noexcept {
      tick = diff();
      if (best > tick)
        best = tick;
      return tick;
    }

    static inline uint32_t now() noexcept {
      return OSUtils::getTickCount();
    }

    uint32_t tick;
    uint32_t best;
  };

  static double mbps(uint32_t time, uint64_t outputSize) noexcept {
    if (!time) return 0.0;

    double bytesTotal = double(outputSize);
    return (bytesTotal * 1000) / (double(time) * 1024 * 1024);
  }

  template<typename EmitterT, typename FuncT>
  static void bench(CodeHolder& code, uint32_t arch, const char* testName, const FuncT& func) noexcept {
    EmitterT emitter;

    const char* archName =
      arch == Environment::kArchX86 ? "X86" :
      arch == Environment::kArchX64 ? "X64" : "???";

    const char* emitterName =
      emitter.isAssembler() ? "Assembler" :
      emitter.isCompiler()  ? "Compiler"  :
      emitter.isBuilder()   ? "Builder"   : "Unknown";

    Performance perf;
    uint64_t codeSize = 0;

    Environment env(arch);

    for (uint32_t r = 0; r < kNumRepeats; r++) {
      perf.start();
      codeSize = 0;
      for (uint32_t i = 0; i < kNumIterations; i++) {
        code.init(env);
        code.attach(&emitter);

        func(emitter);
        codeSize += code.codeSize();

        code.reset();
      }
      perf.end();
    }

    printf("[%s] %-9s %-10s | Time:%6u [ms] | ", archName, emitterName, testName, perf.best);
    if (codeSize)
      printf("Speed: %7.3f [MB/s]", mbps(perf.best, codeSize));
    else
      printf("Speed: N/A");
    printf("\n");
  }
}

// ============================================================================
// [Main]
// ============================================================================

#ifdef ASMJIT_BUILD_X86
static void benchX86(uint32_t arch) noexcept {
  CodeHolder code;

  BenchUtils::bench<x86::Assembler>(code, arch, "[fast]", [](x86::Assembler& a) {
    asmtest::generateOpcodes(a.as<x86::Emitter>());
  });

  BenchUtils::bench<x86::Assembler>(code, arch, "[validate]", [](x86::Assembler& a) {
    a.addValidationOptions(BaseEmitter::kValidationOptionAssembler);
    asmtest::generateOpcodes(a.as<x86::Emitter>());
  });

#ifndef ASMJIT_NO_BUILDER
  BenchUtils::bench<x86::Builder>(code, arch, "[no-asm]", [](x86::Builder& cb) {
    asmtest::generateOpcodes(cb.as<x86::Emitter>());
  });

  BenchUtils::bench<x86::Builder>(code, arch, "[asm]", [](x86::Builder& cb) {
    asmtest::generateOpcodes(cb.as<x86::Emitter>());
    cb.finalize();
  });
#endif

#ifndef ASMJIT_NO_COMPILER
  BenchUtils::bench<x86::Compiler>(code, arch, "[no-asm]", [](x86::Compiler& cc) {
    asmtest::generateAlphaBlend(cc);
  });

  BenchUtils::bench<x86::Compiler>(code, arch, "[asm]", [](x86::Compiler& cc) {
    asmtest::generateAlphaBlend(cc);
    cc.finalize();
  });
#endif
}
#endif

int main() {
#ifdef ASMJIT_BUILD_X86
  benchX86(Environment::kArchX86);
  benchX86(Environment::kArchX64);
#endif

  return 0;
}
