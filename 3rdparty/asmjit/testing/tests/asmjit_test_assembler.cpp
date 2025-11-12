// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmjit_test_assembler.h"

#include "../commons/asmjitutils.h"
#include "../commons/cmdline.h"

using namespace asmjit;

#if !defined(ASMJIT_NO_X86)
bool test_x86_assembler(const TestSettings& settings) noexcept;
bool test_x64_assembler(const TestSettings& settings) noexcept;
#endif

#if !defined(ASMJIT_NO_AARCH64)
bool test_aarch64_assembler(const TestSettings& settings) noexcept;
#endif

static void print_app_info() noexcept {
  printf("AsmJit Assembler Test-Suite v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );
}

static void print_app_usage(const TestSettings& settings) noexcept {
  printf("Usage:\n");
  printf("  --help         Show usage only\n");
  printf("  --verbose      Show only assembling errors [%s]\n", settings.verbose ? "x" : " ");
  printf("  --validate     Use instruction validation [%s]\n", settings.validate ? "x" : " ");
  printf("  --arch=<ARCH>  Select architecture to run ('all' by default)\n");
  printf("\n");

  printf("Architectures:\n");
#if !defined(ASMJIT_NO_X86)
  printf("  --arch=x86     32-bit X86 architecture (X86)\n");
  printf("  --arch=x64     64-bit X86 architecture (X86_64)\n");
#endif
#if !defined(ASMJIT_NO_AARCH64)
  printf("  --arch=aarch64 64-bit ARM architecture (AArch64)\n");
#endif
  printf("\n");
}

int main(int argc, char* argv[]) {
  CmdLine cmd_line(argc, argv);

  TestSettings settings {};
  settings.verbose = cmd_line.has_arg("--verbose");
  settings.validate = cmd_line.has_arg("--validate");

  print_app_info();
  print_app_usage(settings);

  if (cmd_line.has_arg("--help")) {
    return 0;
  }

  const char* arch = cmd_line.value_of("--arch", "all");
  bool x86_failed = false;
  bool x64_failed = false;
  bool aarch64_failed = false;

#if !defined(ASMJIT_NO_X86)
  if ((strcmp(arch, "all") == 0 || strcmp(arch, "x86") == 0))
    x86_failed = !test_x86_assembler(settings);

  if ((strcmp(arch, "all") == 0 || strcmp(arch, "x64") == 0))
    x64_failed = !test_x64_assembler(settings);
#endif

#if !defined(ASMJIT_NO_AARCH64)
  if ((strcmp(arch, "all") == 0 || strcmp(arch, "aarch64") == 0))
    aarch64_failed = !test_aarch64_assembler(settings);
#endif

  bool failed = x86_failed || x64_failed || aarch64_failed;

  if (failed) {
    if (x86_failed)
      printf("** X86 test suite failed **\n");

    if (x64_failed)
      printf("** X64 test suite failed **\n");

    if (aarch64_failed)
      printf("** AArch64 test suite failed **\n");

    printf("** FAILURE **\n");
  }
  else {
    printf("** SUCCESS **\n");
  }

  return failed ? 1 : 0;
}
