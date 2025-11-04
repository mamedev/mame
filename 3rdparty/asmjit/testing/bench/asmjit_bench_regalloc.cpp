// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86.h>
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/a64.h>
#endif // !ASMJIT_NO_AARCH64

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <vector>

#include "../commons/asmjitutils.h"

#if !defined(ASMJIT_NO_COMPILER)
  #include "../commons/cmdline.h"
  #include "../commons/performancetimer.h"
  #include "../commons/random.h"
#endif

using namespace asmjit;

static void print_app_info() {
  printf("AsmJit Benchmark RegAlloc v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );
}

#if !defined(ASMJIT_NO_COMPILER)

class BenchRegAllocApp {
public:
  const char* _arch = nullptr;
  bool _help_only = false;
  bool _verbose = false;
  uint32_t _maximum_complexity = 65536;

  BenchRegAllocApp() noexcept
    : _arch("all") {}
  ~BenchRegAllocApp() noexcept {}

  template<class T>
  inline void add_t() { T::add(*this); }

  int handle_args(int argc, const char* const* argv);
  void show_info();

  bool should_run_arch(Arch arch) const noexcept;
  void emit_code(BaseCompiler* cc, uint32_t complexity, uint32_t reg_count);

#if !defined(ASMJIT_NO_X86)
  void emit_code_x86(x86::Compiler* cc, uint32_t complexity, uint32_t reg_count);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  void emit_code_aarch64(a64::Compiler* cc, uint32_t complexity, uint32_t reg_count);
#endif // !ASMJIT_NO_AARCH64

  int run();
  bool run_arch(Arch arch);
};

int BenchRegAllocApp::handle_args(int argc, const char* const* argv) {
  CmdLine cmd(argc, argv);
  _arch = cmd.value_of("--arch", "all");
  _maximum_complexity = cmd.value_as_uint("--complexity", _maximum_complexity);

  if (cmd.has_arg("--help")) _help_only = true;
  if (cmd.has_arg("--verbose")) _verbose = true;

  return 0;
}

void BenchRegAllocApp::show_info() {
  print_app_info();

  printf("Usage:\n");
  printf("  asmjit_bench_regalloc [arguments]\n");
  printf("\n");

  printf("Arguments:\n");
  printf("  --help           Show usage only\n");
  printf("  --arch=<NAME>    Select architecture to run ('all' by default)\n");
  printf("  --verbose        Verbose output\n");
  printf("  --complexity=<n> Maximum complexity to test (%u)\n", _maximum_complexity);
  printf("\n");

  printf("Architectures:\n");
#if !defined(ASMJIT_NO_X86)
  printf("  --arch=x86       32-bit X86 architecture (X86)\n");
  printf("  --arch=x64       64-bit X86 architecture (X86_64)\n");
#endif
#if !defined(ASMJIT_NO_AARCH64)
  printf("  --arch=aarch64   64-bit ARM architecture (AArch64)\n");
#endif
  printf("\n");
}

bool BenchRegAllocApp::should_run_arch(Arch arch) const noexcept {
  if (strcmp(_arch, "all") == 0) {
    return true;
  }

  if (strcmp(_arch, "x86") == 0 && arch == Arch::kX86) {
    return true;
  }

  if (strcmp(_arch, "x64") == 0 && arch == Arch::kX64) {
    return true;
  }

  if (strcmp(_arch, "aarch64") == 0 && arch == Arch::kAArch64) {
    return true;
  }

  return false;
}

void BenchRegAllocApp::emit_code(BaseCompiler* cc, uint32_t complexity, uint32_t reg_count) {
#if !defined(ASMJIT_NO_X86)
  if (cc->arch() == Arch::kX86 || cc->arch() == Arch::kX64) {
    emit_code_x86(cc->as<x86::Compiler>(), complexity, reg_count);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if (cc->arch() == Arch::kAArch64) {
    emit_code_aarch64(cc->as<a64::Compiler>(), complexity, reg_count);
  }
#endif
}

constexpr size_t kLocalRegCount = 3;
constexpr size_t kLocalOpCount = 15;

#if !defined(ASMJIT_NO_X86)
void BenchRegAllocApp::emit_code_x86(x86::Compiler* cc, uint32_t complexity, uint32_t reg_count) {
  TestUtils::Random rnd(0x1234);

  std::vector<Label> labels;
  std::vector<uint32_t> used_labels;
  std::vector<x86::Vec> virt_regs;

  x86::Gp arg_ptr = cc->new_gp_ptr("arg_ptr");
  x86::Gp counter = cc->new_gp_ptr("counter");

  for (size_t i = 0; i < complexity; i++) {
    labels.push_back(cc->new_label());
    used_labels.push_back(0u);
  }

  for (size_t i = 0; i < reg_count; i++) {
    virt_regs.push_back(cc->new_xmm_sd("v%u", unsigned(i)));
  }

  FuncNode* func = cc->add_func(FuncSignature::build<void, size_t, void*>());
  func->add_attributes(FuncAttributes::kX86_AVXEnabled);
  func->set_arg(0, counter);
  func->set_arg(1, arg_ptr);

  for (size_t i = 0; i < reg_count; i++) {
    cc->vmovsd(virt_regs[i], x86::ptr_64(arg_ptr, int32_t(i * 8)));
  }

  auto next_label = [&]() {
    uint32_t id = rnd.next_uint32() % complexity;
    if (used_labels[id] > 1) {
      id = 0;
      do {
        if (++id >= complexity) {
          id = 0;
        }
      } while (used_labels[id] != 0);
    }

    used_labels[id]++;
    return labels[id];
  };

  for (size_t i = 0; i < labels.size(); i++) {
    cc->bind(labels[i]);

    x86::Vec locals[kLocalRegCount];
    for (size_t j = 0; j < kLocalRegCount; j++) {
      locals[j] = cc->new_xmm_sd("local%u", unsigned(j));
    }

    size_t local_op_threshold = kLocalOpCount - kLocalRegCount;

    for (size_t j = 0; j < 15; j++) {
      uint32_t op = rnd.next_uint32() % 6u;
      uint32_t id1 = rnd.next_uint32() % reg_count;
      uint32_t id2 = rnd.next_uint32() % reg_count;

      x86::Vec v0 = virt_regs[id1];
      x86::Vec v1 = virt_regs[id1];
      x86::Vec v2 = virt_regs[id2];

      if (j < kLocalRegCount) {
        v0 = locals[j];
      }

      if (j >= local_op_threshold) {
        v2 = locals[j - local_op_threshold];
      }

      switch (op) {
        case 0: cc->vaddsd(v0, v1, v2); break;
        case 1: cc->vsubsd(v0, v1, v2); break;
        case 2: cc->vmulsd(v0, v1, v2); break;
        case 3: cc->vdivsd(v0, v1, v2); break;
        case 4: cc->vminsd(v0, v1, v2); break;
        case 5: cc->vmaxsd(v0, v1, v2); break;
      }
    }

    cc->sub(counter, 1);
    cc->jns(next_label());
  }

  for (size_t i = 0; i < reg_count; i++) {
    cc->vmovsd(x86::ptr_64(arg_ptr, int32_t(i * 8)), virt_regs[i]);
  }

  cc->end_func();
}
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
void BenchRegAllocApp::emit_code_aarch64(a64::Compiler* cc, uint32_t complexity, uint32_t reg_count) {
  TestUtils::Random rnd(0x1234);

  std::vector<Label> labels;
  std::vector<uint32_t> used_labels;
  std::vector<a64::Vec> virt_regs;

  a64::Gp arg_ptr = cc->new_gp_ptr("arg_ptr");
  a64::Gp counter = cc->new_gp_ptr("counter");

  for (size_t i = 0; i < complexity; i++) {
    labels.push_back(cc->new_label());
    used_labels.push_back(0u);
  }

  for (size_t i = 0; i < reg_count; i++) {
    virt_regs.push_back(cc->new_vec_d("v%u", unsigned(i)));
  }

  FuncNode* func = cc->add_func(FuncSignature::build<void, size_t, void*>());
  func->add_attributes(FuncAttributes::kX86_AVXEnabled);
  func->set_arg(0, counter);
  func->set_arg(1, arg_ptr);

  for (size_t i = 0; i < reg_count; i++) {
    cc->ldr(virt_regs[i].d(), a64::ptr(arg_ptr, int32_t(i * 8) & 1023));
  }

  auto next_label = [&]() {
    uint32_t id = rnd.next_uint32() % complexity;
    if (used_labels[id] > 1) {
      id = 0;
      do {
        if (++id >= complexity) {
          id = 0;
        }
      } while (used_labels[id] != 0);
    }

    used_labels[id]++;
    return labels[id];
  };

  for (size_t i = 0; i < labels.size(); i++) {
    cc->bind(labels[i]);

    a64::Vec locals[kLocalRegCount];
    for (size_t j = 0; j < kLocalRegCount; j++) {
      locals[j] = cc->new_vec_d("local%u", unsigned(j));
    }

    size_t local_op_threshold = kLocalOpCount - kLocalRegCount;

    for (size_t j = 0; j < 15; j++) {
      uint32_t op = rnd.next_uint32() % 6;
      uint32_t id1 = rnd.next_uint32() % reg_count;
      uint32_t id2 = rnd.next_uint32() % reg_count;

      a64::Vec v0 = virt_regs[id1];
      a64::Vec v1 = virt_regs[id1];
      a64::Vec v2 = virt_regs[id2];

      if (j < kLocalRegCount) {
        v0 = locals[j];
      }

      if (j >= local_op_threshold) {
        v2 = locals[j - local_op_threshold];
      }

      switch (op) {
        case 0: cc->fadd(v0.d(), v1.d(), v2.d()); break;
        case 1: cc->fsub(v0.d(), v1.d(), v2.d()); break;
        case 2: cc->fmul(v0.d(), v1.d(), v2.d()); break;
        case 3: cc->fdiv(v0.d(), v1.d(), v2.d()); break;
        case 4: cc->fmin(v0.d(), v1.d(), v2.d()); break;
        case 5: cc->fmax(v0.d(), v1.d(), v2.d()); break;
      }
    }

    cc->subs(counter, counter, 1);
    cc->b_hi(next_label());
  }

  for (size_t i = 0; i < reg_count; i++) {
    cc->str(virt_regs[i].d(), a64::ptr(arg_ptr, int32_t(i * 8) & 1023));
  }

  cc->end_func();
}
#endif // !ASMJIT_NO_AARCH64

int BenchRegAllocApp::run() {
  if (should_run_arch(Arch::kX64) && !run_arch(Arch::kX64)) {
    return 1;
  }

  if (should_run_arch(Arch::kAArch64) && !run_arch(Arch::kAArch64)) {
    return 1;
  }

  return 0;
}

bool BenchRegAllocApp::run_arch(Arch arch) {
  Environment custom_env;
  CpuFeatures features;

  switch (arch) {
    case Arch::kX86:
    case Arch::kX64:
      features.add(CpuFeatures::X86::kADX,
                   CpuFeatures::X86::kAVX,
                   CpuFeatures::X86::kAVX2,
                   CpuFeatures::X86::kBMI,
                   CpuFeatures::X86::kBMI2,
                   CpuFeatures::X86::kCMOV,
                   CpuFeatures::X86::kF16C,
                   CpuFeatures::X86::kFMA,
                   CpuFeatures::X86::kFPU,
                   CpuFeatures::X86::kI486,
                   CpuFeatures::X86::kLZCNT,
                   CpuFeatures::X86::kMMX,
                   CpuFeatures::X86::kMMX2,
                   CpuFeatures::X86::kPOPCNT,
                   CpuFeatures::X86::kSSE,
                   CpuFeatures::X86::kSSE2,
                   CpuFeatures::X86::kSSE3,
                   CpuFeatures::X86::kSSSE3,
                   CpuFeatures::X86::kSSE4_1,
                   CpuFeatures::X86::kSSE4_2,
                   CpuFeatures::X86::kAVX,
                   CpuFeatures::X86::kAVX2);
      break;

    case Arch::kAArch64:
      features.add(CpuFeatures::ARM::kAES,
                   CpuFeatures::ARM::kASIMD,
                   CpuFeatures::ARM::kIDIVA,
                   CpuFeatures::ARM::kIDIVT,
                   CpuFeatures::ARM::kPMULL);
      break;

    default:
      return false;
  }

  CodeHolder code;

  custom_env.init(arch);
  code.init(custom_env, features);

  std::unique_ptr<BaseCompiler> cc;

#ifndef ASMJIT_NO_X86
  if (code.arch() == Arch::kX86 || code.arch() == Arch::kX64) {
    cc = std::make_unique<x86::Compiler>();
  }
#endif // !ASMJIT_NO_X86

#ifndef ASMJIT_NO_AARCH64
  if (code.arch() == Arch::kAArch64) {
    cc = std::make_unique<a64::Compiler>();
  }
#endif // !ASMJIT_NO_AARCH64

  if (!cc)
    return false;

  PerformanceTimer emit_timer;
  PerformanceTimer finalize_timer;

  uint32_t reg_count = 35;

  code.reinit();
  code.attach(cc.get());

  // Dry run to not benchmark allocs on the first run.
  emit_code(cc.get(), 0, reg_count);
  cc->finalize();
  code.reinit();

#if !defined(ASMJIT_NO_LOGGING)
  StringLogger logger;
  if (_verbose) {
    code.set_logger(&logger);
    cc->add_diagnostic_options(DiagnosticOptions::kRAAnnotate | DiagnosticOptions::kRADebugAll);
  }
#endif // !ASMJIT_NO_LOGGING

  printf("+-----------------------------------------+-----------+-----------------------------------+--------------+--------------+\n");
  printf("|           Input Configuration           |   Output  |        Reserved Memory [KiB]      |      Time Elapsed [ms]      |\n");
  printf("+--------+------------+--------+----------+-----------+-----------+-----------+-----------+--------------+--------------+\n");
  printf("| Arch   | Complexity | Labels | RegCount |  CodeSize | Code Hold.| Compiler  | Pass Temp.|   Emit Time  |  Reg. Alloc  |\n");
  printf("+--------+------------+--------+----------+-----------+-----------+-----------+-----------+--------------+--------------+\n");

  for (uint32_t complexity = 1u; complexity <= _maximum_complexity; complexity *= 2u) {
    emit_timer.start();
    emit_code(cc.get(), complexity + 1, reg_count);
    emit_timer.stop();

    finalize_timer.start();
    Error err = cc->finalize();
    finalize_timer.stop();

#if !defined(ASMJIT_NO_LOGGING)
    if (_verbose) {
      printf("%s\n", logger.data());
      logger.clear();
    }
#endif

    code.flatten();

    double emit_time = emit_timer.duration();
    double finalize_time = finalize_timer.duration();
    size_t code_size = code.code_size();
    size_t label_count = code.label_count();
    size_t virt_reg_count = cc->virt_regs().size();

    ArenaStatistics code_holder_stats = code._arena.statistics();
    ArenaStatistics compiler_stats = cc->_builder_arena.statistics();
    ArenaStatistics pass_stats = cc->_pass_arena.statistics();

    printf(
      "| %-7s| %10u | %6zu | %8zu | %9zu | %9zu | %9zu | %9zu | %12.3f | %12.3f |",
      asmjit_arch_as_string(arch),
      complexity,
      label_count,
      virt_reg_count,
      code_size,
      (code_holder_stats.reserved_size() + 1023) / 1024,
      (compiler_stats.reserved_size() + 1023) / 1024,
      (pass_stats.reserved_size() + 1023) / 1024,
      emit_time,
      finalize_time
    );

    if (err != Error::kOk) {
      printf(" (err: %s)", DebugUtils::error_as_string(err));
    }

    printf("\n");

    code.reinit();
  }

  printf("+--------+------------+--------+----------+-----------+-----------+-----------+-----------+--------------+--------------+\n");
  printf("\n");

  return true;
}

int main(int argc, char* argv[]) {
  BenchRegAllocApp app;

  app.handle_args(argc, argv);
  app.show_info();

  if (app._help_only)
    return 0;

  return app.run();
}

#else

int main() {
  print_app_info();
  printf("!! This Benchmark is disabled: <ASMJIT_NO_JIT> or unsuitable target architecture !!\n");
  return 0;
}

#endif // !ASMJIT_NO_COMPILER
