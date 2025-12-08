// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <vector>
#include <chrono>

#if !defined(ASMJIT_NO_COMPILER)

#include "../commons/asmjitutils.h"
#include "../commons/cmdline.h"
#include "../commons/performancetimer.h"

#include "asmjit_test_compiler.h"

#if !defined(ASMJIT_NO_X86)
  #include <asmjit/x86.h>
  void compiler_add_x86_tests(TestApp& app);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  #include <asmjit/a64.h>
  void compiler_add_a64_tests(TestApp& app);
#endif // !ASMJIT_NO_AARCH64

using namespace asmjit;

int TestApp::handle_args(int argc, const char* const* argv) {
  CmdLine cmd(argc, argv);
  _arch = cmd.value_of("--arch", "all");
  _filter = cmd.value_of("--filter", nullptr);

  if (cmd.has_arg("--help")) _help_only = true;
  if (cmd.has_arg("--verbose")) _verbose = true;

#ifndef ASMJIT_NO_LOGGING
  if (cmd.has_arg("--dump-asm")) _dump_asm = true;
#endif // !ASMJIT_NO_LOGGING

  if (cmd.has_arg("--dump-hex")) _dump_hex = true;

  return 0;
}

void TestApp::show_info() {
  printf("AsmJit Compiler Test-Suite v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  printf("Usage:\n");
  printf("  --help          Show usage only\n");
  printf("  --arch=<NAME>   Select architecture to run ('all' by default)\n");
  printf("  --filter=<NAME> Use a filter to restrict which test is called\n");
  printf("  --verbose       Verbose output\n");
  printf("  --dump-asm      Assembler output\n");
  printf("  --dump-hex      Hexadecimal output (relocated, only for host arch)\n");
  printf("\n");
}

#ifndef ASMJIT_NO_LOGGING
class IndentedStdoutLogger : public Logger {
public:
  ASMJIT_NONCOPYABLE(IndentedStdoutLogger)

  size_t _indentation = 0;

  explicit IndentedStdoutLogger(size_t indentation) noexcept
    : _indentation(indentation) {}

  Error _log(const char* data, size_t size = SIZE_MAX) noexcept override {
    asmjit::Support::maybe_unused(size);
    print_indented(data, _indentation);
    return Error::kOk;
  }
};
#endif // !ASMJIT_NO_LOGGING

bool TestApp::should_run(const TestCase* tc) {
  if (!_filter)
    return true;

  return strstr(tc->name(), _filter) != nullptr;
}

int TestApp::run() {
#ifndef ASMJIT_NO_LOGGING
  FormatOptions format_options;
  format_options.add_flags(
    FormatFlags::kMachineCode |
    FormatFlags::kShowAliases |
    FormatFlags::kExplainImms |
    FormatFlags::kRegCasts   );
  format_options.set_indentation(FormatIndentationGroup::kCode, 2);

  IndentedStdoutLogger print_logger(4);
  print_logger.set_options(format_options);

  StringLogger string_logger;
  string_logger.set_options(format_options);

  auto print_string_logger_content = [&]() {
    if (!_verbose) {
      printf("%s", string_logger.data());
      fflush(stdout);
    }
  };
#else
  auto print_string_logger_content = [&]() {};
#endif // !ASMJIT_NO_LOGGING

  // maybe unused...
  Support::maybe_unused(print_string_logger_content);

#ifndef ASMJIT_NO_JIT
  JitRuntime runtime;
#endif // !ASMJIT_NO_JIT

  PerformanceTimer compile_timer;
  PerformanceTimer finalize_timer;

  double compile_time = 0;
  double finalize_time = 0;

  for (std::unique_ptr<TestCase>& test : _tests) {
    if (!should_run(test.get()))
      continue;

    _num_tests++;

    for (uint32_t pass = 0; pass < 2; pass++) {
      bool runnable = false;
      CodeHolder code;
      SimpleErrorHandler error_handler;

      const char* status_separator = " ";

      // Filter architecture to run.
      if (strcmp(_arch, "all") != 0) {
        switch (test->arch()) {
          case Arch::kX86:
            if (strcmp(_arch, "x86") == 0)
              break;
            continue;
          case Arch::kX64:
            if (strcmp(_arch, "x64") == 0)
              break;
            continue;
          case Arch::kAArch64:
            if (strcmp(_arch, "aarch64") == 0)
              break;
            continue;
          default:
            continue;
        }
      }

      // Use platform environment and CPU features when the test can run on the arch.
#ifndef ASMJIT_NO_JIT
      if (runtime.arch() == test->arch()) {
        code.init(runtime.environment(), runtime.cpu_features());
        runnable = true;
      }
#endif // !ASMJIT_NO_JIT

      if (!code.is_initialized()) {
        Environment custom_env;
        CpuFeatures features;

        switch (test->arch()) {
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
                         CpuFeatures::X86::kSSE4_2);
            break;

          case Arch::kAArch64:
            features.add(CpuFeatures::ARM::kAES,
                         CpuFeatures::ARM::kASIMD,
                         CpuFeatures::ARM::kIDIVA,
                         CpuFeatures::ARM::kIDIVT,
                         CpuFeatures::ARM::kPMULL);
            break;

          default:
            break;
        }

        custom_env.init(test->arch());
        code.init(custom_env, features);
      }

      code.set_error_handler(&error_handler);

      if (pass != 0) {
        printf("[Test:%s] %s", asmjit_arch_as_string(test->arch()), test->name());
        fflush(stdout);

#ifndef ASMJIT_NO_LOGGING
        if (_verbose || _dump_asm || _dump_hex) {
          printf("\n");
          status_separator = "  ";
        }

        if (_verbose) {
          printf("  [Log]\n");
          code.set_logger(&print_logger);
        }
        else {
          string_logger.clear();
          code.set_logger(&string_logger);
        }
#endif // !ASMJIT_NO_LOGGING
      }

      std::unique_ptr<BaseCompiler> cc;

#ifndef ASMJIT_NO_X86
      if (code.arch() == Arch::kX86 || code.arch() == Arch::kX64)
        cc = std::make_unique<x86::Compiler>(&code);
#endif // !ASMJIT_NO_X86

#ifndef ASMJIT_NO_AARCH64
      if (code.arch() == Arch::kAArch64)
        cc = std::make_unique<a64::Compiler>(&code);
#endif // !ASMJIT_NO_AARCH64

      if (!cc)
        continue;

#ifndef ASMJIT_NO_LOGGING
      cc->add_diagnostic_options(DiagnosticOptions::kRAAnnotate | DiagnosticOptions::kRADebugAll);
#endif // !ASMJIT_NO_LOGGING

      compile_timer.start();
      test->compile(*cc);
      compile_timer.stop();

      Error err = error_handler._err;
      if (err == Error::kOk) {
        finalize_timer.start();
        err = cc->finalize();
        finalize_timer.stop();
      }

      // The first pass is only used for timing of serialization and compilation, because otherwise it would be
      // biased by logging, which takes much more time than finalize() does. We want to benchmark Compiler the
      // way it would be used in the production.
      if (pass == 0) {
        _output_size += code.code_size();
        compile_time += compile_timer.duration();
        finalize_time += finalize_timer.duration();
        continue;
      }

#ifndef ASMJIT_NO_LOGGING
      if (_dump_asm) {
        String sb;
        Formatter::format_node_list(sb, format_options, cc.get());
        printf("  [Assembly]\n");
        print_indented(sb.data(), 4);
      }
#endif // !ASMJIT_NO_LOGGING

#ifndef ASMJIT_NO_JIT
      if (runnable) {
        void* func = nullptr;
        if (err == Error::kOk)
          err = runtime.add(&func, &code);

        if (err == Error::kOk && _dump_hex) {
          String sb;
          sb.append_hex((void*)func, code.code_size());
          printf("  [Hex Dump]:\n");
          for (size_t i = 0; i < sb.size(); i += 76) {
            printf("    %.60s\n", sb.data() + i);
          }
        }

        if (_verbose)
          fflush(stdout);

        if (err == Error::kOk) {
          StringTmp<128> result;
          StringTmp<128> expect;

          if (test->run(func, result, expect)) {
            if (!_verbose)
              printf("%s[RUN OK]\n", status_separator);
          }
          else {
            if (!_verbose)
              printf("%s[RUN FAILED]\n", status_separator);

            print_string_logger_content();
            printf("  [Output]\n");
            printf("    Returned: %s\n", result.data());
            printf("    Expected: %s\n", expect.data());
            _num_failed++;
          }

          if (_dump_asm)
            printf("\n");

          runtime.release(func);
        }
        else {
          if (!_verbose)
            printf("%s[COMPILE FAILED]\n", status_separator);

          print_string_logger_content();
          printf("  [Status]\n");
          printf("    ERROR 0x%08X: %s\n", unsigned(err), error_handler._message.data());
          _num_failed++;
        }
      }
#endif // !ASMJIT_NO_JIT

      if (!runnable) {
        if (err != Error::kOk) {
          printf("  [Status]\n");
          printf("    ERROR 0x%08X: %s\n", unsigned(err), error_handler._message.data());
          _num_failed++;
        }
        else {
          printf("%s[COMPILE OK]\n", status_separator);
        }
      }

#ifndef ASMJIT_NO_LOGGING
      if (_verbose || _dump_asm || _dump_hex) {
        printf("\n");
      }
#endif // !ASMJIT_NO_LOGGING
    }
  }

  printf("\n");
  printf("Summary:\n");
  printf("  OutputSize: %zu bytes\n", _output_size);
  printf("  CompileTime: %.2f ms\n", compile_time);
  printf("  FinalizeTime: %.2f ms\n", finalize_time);
  printf("\n");

  if (_num_failed == 0)
    printf("** SUCCESS: All %u tests passed **\n", _num_tests);
  else
    printf("** FAILURE: %u of %u tests failed **\n", _num_failed, _num_tests);

  return _num_failed == 0 ? 0 : 1;
}

int main(int argc, char* argv[]) {
  TestApp app;

  app.handle_args(argc, argv);
  app.show_info();

#if !defined(ASMJIT_NO_X86)
  compiler_add_x86_tests(app);
#endif // !ASMJIT_NO_X86

#if !defined(ASMJIT_NO_AARCH64)
  compiler_add_a64_tests(app);
#endif // !ASMJIT_NO_AARCH64

  return app.run();
}

#else

int main(int argc, char* argv[]) {
  Support::maybe_unused(argc, argv);

  printf("AsmJit Compiler Test suite is disabled when compiling with ASMJIT_NO_COMPILER\n\n");
  return 0;
}

#endif // !ASMJIT_NO_COMPILER
