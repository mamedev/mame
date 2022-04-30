// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <vector>
#include <chrono>

#include "cmdline.h"
#include "asmjitutils.h"
#include "performancetimer.h"

#include "asmjit_test_compiler.h"

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86
#include <asmjit/x86.h>
void compiler_add_x86_tests(TestApp& app);
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
#include <asmjit/a64.h>
void compiler_add_a64_tests(TestApp& app);
#endif

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86
  #define ASMJIT_HAVE_WORKING_JIT
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
  #define ASMJIT_HAVE_WORKING_JIT
#endif

using namespace asmjit;

int TestApp::handleArgs(int argc, const char* const* argv) {
  CmdLine cmd(argc, argv);

  if (cmd.hasArg("--verbose")) _verbose = true;
  if (cmd.hasArg("--dump-asm")) _dumpAsm = true;
  if (cmd.hasArg("--dump-hex")) _dumpHex = true;

  return 0;
}

void TestApp::showInfo() {
  printf("AsmJit Compiler Test-Suite v%u.%u.%u (Arch=%s):\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjitArchAsString(Arch::kHost));
  printf("  [%s] Verbose (use --verbose to turn verbose output ON)\n", _verbose ? "x" : " ");
  printf("  [%s] DumpAsm (use --dump-asm to turn assembler dumps ON)\n", _dumpAsm ? "x" : " ");
  printf("  [%s] DumpHex (use --dump-hex to dump binary in hexadecimal)\n", _dumpHex ? "x" : " ");
  printf("\n");
}

int TestApp::run() {
#ifndef ASMJIT_HAVE_WORKING_JIT
  return 0;
#else
#ifndef ASMJIT_NO_LOGGING
  FormatOptions formatOptions;
  formatOptions.addFlags(
    FormatFlags::kMachineCode |
    FormatFlags::kExplainImms |
    FormatFlags::kRegCasts   );

  FileLogger fileLogger(stdout);
  fileLogger.setOptions(formatOptions);

  StringLogger stringLogger;
  stringLogger.setOptions(formatOptions);
#endif

  JitRuntime runtime;

  PerformanceTimer compileTimer;
  PerformanceTimer finalizeTimer;
  double compileTime = 0;
  double finalizeTime = 0;

  for (std::unique_ptr<TestCase>& test : _tests) {
    for (uint32_t pass = 0; pass < 2; pass++) {
      CodeHolder code;
      SimpleErrorHandler errorHandler;

      code.init(runtime.environment());
      code.setErrorHandler(&errorHandler);

      if (pass != 0) {
#ifndef ASMJIT_NO_LOGGING
        if (_verbose) {
          code.setLogger(&fileLogger);
        }
        else {
          stringLogger.clear();
          code.setLogger(&stringLogger);
        }
#endif

        printf("[Test] %s", test->name());

#ifndef ASMJIT_NO_LOGGING
        if (_verbose)
          printf("\n");
#endif
      }


#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86
      x86::Compiler cc(&code);
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
      a64::Compiler cc(&code);
#endif

#ifndef ASMJIT_NO_LOGGING
      cc.addDiagnosticOptions(DiagnosticOptions::kRAAnnotate | DiagnosticOptions::kRADebugAll);
#endif

      compileTimer.start();
      test->compile(cc);
      compileTimer.stop();

      void* func = nullptr;
      Error err = errorHandler._err;

      if (!err) {
        finalizeTimer.start();
        err = cc.finalize();
        finalizeTimer.stop();
      }

      // The first pass is only for timing serialization and compilation, because otherwise it would be biased by
      // logging, which takes much more time than finalize() does. We want to benchmark Compiler the way it would
      // be used in production.
      if (pass == 0) {
        compileTime += compileTimer.duration();
        finalizeTime += finalizeTimer.duration();
        continue;
      }

#ifndef ASMJIT_NO_LOGGING
      if (_dumpAsm) {
        if (!_verbose)
          printf("\n");

        String sb;
        Formatter::formatNodeList(sb, formatOptions, &cc);
        printf("%s", sb.data());
      }
#endif

      if (err == kErrorOk)
        err = runtime.add(&func, &code);

      if (err == kErrorOk && _dumpHex) {
        String sb;
        sb.appendHex((void*)func, code.codeSize());
        printf("\n (HEX: %s)\n", sb.data());
      }

      if (_verbose)
        fflush(stdout);

      if (err == kErrorOk) {
        _outputSize += code.codeSize();

        StringTmp<128> result;
        StringTmp<128> expect;

        if (test->run(func, result, expect)) {
          if (!_verbose)
            printf(" [OK]\n");
        }
        else {
          if (!_verbose)
            printf(" [FAILED]\n");

#ifndef ASMJIT_NO_LOGGING
          if (!_verbose)
            printf("%s", stringLogger.data());
#endif

          printf("[Status]\n");
          printf("  Returned: %s\n", result.data());
          printf("  Expected: %s\n", expect.data());

          _nFailed++;
        }

        if (_dumpAsm)
          printf("\n");

        runtime.release(func);
      }
      else {
        if (!_verbose)
          printf(" [FAILED]\n");

#ifndef ASMJIT_NO_LOGGING
        if (!_verbose)
          printf("%s", stringLogger.data());
#endif

        printf("[Status]\n");
        printf("  ERROR 0x%08X: %s\n", unsigned(err), errorHandler._message.data());

        _nFailed++;
      }
    }
  }

  printf("\n");
  printf("Summary:\n");
  printf("  OutputSize: %zu bytes\n", _outputSize);
  printf("  CompileTime: %.2f ms\n", compileTime);
  printf("  FinalizeTime: %.2f ms\n", finalizeTime);
  printf("\n");

  if (_nFailed == 0)
    printf("** SUCCESS: All %u tests passed **\n", unsigned(_tests.size()));
  else
    printf("** FAILURE: %u of %u tests failed **\n", _nFailed, unsigned(_tests.size()));

  return _nFailed == 0 ? 0 : 1;
#endif
}

int main(int argc, char* argv[]) {
  TestApp app;

  app.handleArgs(argc, argv);
  app.showInfo();

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86
  compiler_add_x86_tests(app);
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
  compiler_add_a64_tests(app);
#endif

  return app.run();
}
