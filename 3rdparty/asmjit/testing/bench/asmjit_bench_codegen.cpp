// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../commons/asmjitutils.h"
#include "../commons/cmdline.h"

using namespace asmjit;

static void print_app_info() noexcept {
  printf("AsmJit Benchmark CodeGen v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );
}

#if !defined(ASMJIT_NO_X86)
void benchmark_x86_emitters(uint32_t num_iterations, bool test_x86, bool test_x64) noexcept;
#endif

#if !defined(ASMJIT_NO_AARCH64)
void benchmark_aarch64_emitters(uint32_t num_iterations);
#endif

int main(int argc, char* argv[]) {
  CmdLine cmd_line(argc, argv);
  uint32_t num_iterations = 100000;

  print_app_info();

  printf("Usage:\n");
  printf("  --help         Show usage only\n");
  printf("  --quick        Decrease the number of iterations to make tests quicker\n");
  printf("  --arch=<ARCH>  Select architecture(s) to run ('all' by default)\n");
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

  if (cmd_line.has_arg("--help"))
    return 0;

  if (cmd_line.has_arg("--quick"))
    num_iterations = 1000;

  const char* arch = cmd_line.value_of("--arch", "all");

#if !defined(ASMJIT_NO_X86)
  bool test_x86 = strcmp(arch, "all") == 0 || strcmp(arch, "x86") == 0;
  bool test_x64 = strcmp(arch, "all") == 0 || strcmp(arch, "x64") == 0;

  if (test_x86 || test_x64) {
    benchmark_x86_emitters(num_iterations, test_x86, test_x64);
  }
#endif

#if !defined(ASMJIT_NO_AARCH64)
  bool test_aarch64 = strcmp(arch, "all") == 0 || strcmp(arch, "aarch64") == 0;

  if (test_aarch64) {
    benchmark_aarch64_emitters(num_iterations);
  }
#endif

  return 0;
}
