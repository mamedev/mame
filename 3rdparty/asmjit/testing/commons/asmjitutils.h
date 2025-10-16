// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJITUTILS_H_INCLUDED
#define ASMJITUTILS_H_INCLUDED

#include <asmjit/core.h>

namespace {

[[maybe_unused]]
static const char* asmjit_build_type() noexcept {
#if defined(ASMJIT_BUILD_DEBUG)
  static const char build_type[] = "Debug";
#else
  static const char build_type[] = "Release";
#endif
  return build_type;
}

[[maybe_unused]]
static const char* asmjit_arch_as_string(asmjit::Arch arch) noexcept {
  switch (arch) {
    case asmjit::Arch::kX86       : return "X86";
    case asmjit::Arch::kX64       : return "X64";

    case asmjit::Arch::kRISCV32   : return "RISCV32";
    case asmjit::Arch::kRISCV64   : return "RISCV64";

    case asmjit::Arch::kARM       : return "ARM";
    case asmjit::Arch::kAArch64   : return "AArch64";
    case asmjit::Arch::kThumb     : return "Thumb";

    case asmjit::Arch::kMIPS32_LE : return "MIPS_LE";
    case asmjit::Arch::kMIPS64_LE : return "MIPS64_LE";

    case asmjit::Arch::kARM_BE    : return "ARM_BE";
    case asmjit::Arch::kThumb_BE  : return "Thumb_BE";
    case asmjit::Arch::kAArch64_BE: return "AArch64_BE";

    case asmjit::Arch::kMIPS32_BE : return "MIPS_BE";
    case asmjit::Arch::kMIPS64_BE : return "MIPS64_BE";

    default:
      return "<Unknown>";
  }
}

[[maybe_unused]]
static void print_indented(const char* str, size_t indent) noexcept {
  const char* start = str;
  while (*str) {
    if (*str == '\n') {
      size_t size = (size_t)(str - start);
      printf("%*s%.*s\n", size ? int(indent) : 0, "", int(size), start);
      start = str + 1;
    }
    str++;
  }

  size_t size = (size_t)(str - start);
  if (size)
    printf("%*s%.*s\n", int(indent), "", int(size), start);
}

[[maybe_unused]]
static void print_cpu_info() noexcept {
  const asmjit::CpuInfo& cpu = asmjit::CpuInfo::host();

  // CPU Information
  // ---------------

  printf("CPU Info:\n");
  printf("  Vendor                  : %s\n", cpu.vendor());
  printf("  Brand                   : %s\n", cpu.brand());
  printf("  Model ID                : 0x%08X (%u)\n", cpu.model_id(), cpu.model_id());
  printf("  Brand ID                : 0x%08X (%u)\n", cpu.brand_id(), cpu.brand_id());
  printf("  Family ID               : 0x%08X (%u)\n", cpu.family_id(), cpu.family_id());
  printf("  Stepping                : %u\n", cpu.stepping());
  printf("  Processor Type          : %u\n", cpu.processor_type());
  printf("  Max logical Processors  : %u\n", cpu.max_logical_processors());
  printf("  Cache-Line Size         : %u\n", cpu.cache_line_size());
  printf("  HW-Thread Count         : %u\n", cpu.hw_thread_count());
  printf("\n");

  // CPU Features
  // ------------

  using asmjit::CpuHints;

#ifndef ASMJIT_NO_LOGGING
  printf("CPU Features:\n");
  asmjit::CpuFeatures::Iterator it(cpu.features().iterator());
  while (it.has_next()) {
    uint32_t feature_id = uint32_t(it.next());
    asmjit::StringTmp<64> feature_string;
    asmjit::Formatter::format_feature(feature_string, cpu.arch(), feature_id);
    printf("  %s\n", feature_string.data());
  };
  printf("\n");
#endif // !ASMJIT_NO_LOGGING

  // CPU Hints
  // ---------

  printf("CPU Hints:\n");
  auto print_hint = [&](CpuHints hint, const char* name) {
    if ((cpu.hints() & hint) != CpuHints::kNone) {
      printf("  %s\n", name);
    }
  };

  print_hint(CpuHints::kVecMaskedOps8  , "VecMaskedOps8"  );
  print_hint(CpuHints::kVecMaskedOps16 , "VecMaskedOps16" );
  print_hint(CpuHints::kVecMaskedOps32 , "VecMaskedOps32" );
  print_hint(CpuHints::kVecMaskedOps64 , "VecMaskedOps64" );
  print_hint(CpuHints::kVecFastIntMul32, "VecFastIntMul32");
  print_hint(CpuHints::kVecFastIntMul64, "VecFastIntMul64");
  print_hint(CpuHints::kVecFastGather  , "VecFastGather"  );
  print_hint(CpuHints::kVecMaskedStore , "VecMaskedStore" );

  printf("\n");
}

[[maybe_unused]]
static void print_build_options() {
  auto stringify_build_definition = [](bool b) { return b ? "defined" : "(not defined)"; };

#if defined(ASMJIT_NO_X86)
  constexpr bool no_x86 = true;
#else
  constexpr bool no_x86 = false;
#endif

#if defined(ASMJIT_NO_AARCH64)
  constexpr bool no_aarch64 = true;
#else
  constexpr bool no_aarch64 = false;
#endif

#if defined(ASMJIT_NO_FOREIGN)
  constexpr bool no_foreign = true;
#else
  constexpr bool no_foreign = false;
#endif

#if defined(ASMJIT_NO_DEPRECATED)
  constexpr bool no_deprecated = true;
#else
  constexpr bool no_deprecated = false;
#endif

#if defined(ASMJIT_NO_ABI_NAMESPACE)
  constexpr bool no_abi_namespace = true;
#else
  constexpr bool no_abi_namespace = false;
#endif

#if defined(ASMJIT_NO_SHM_OPEN)
  constexpr bool no_shm_open = true;
#else
  constexpr bool no_shm_open = false;
#endif

#if defined(ASMJIT_NO_JIT)
  constexpr bool no_jit = true;
#else
  constexpr bool no_jit = false;
#endif

#if defined(ASMJIT_NO_TEXT)
  constexpr bool no_text = true;
#else
  constexpr bool no_text = false;
#endif

#if defined(ASMJIT_NO_LOGGING)
  constexpr bool no_logging = true;
#else
  constexpr bool no_logging = false;
#endif

#if defined(ASMJIT_NO_VALIDATION)
  constexpr bool no_validation = true;
#else
  constexpr bool no_validation = false;
#endif

#if defined(ASMJIT_NO_INTROSPECTION)
  constexpr bool no_introspection = true;
#else
  constexpr bool no_introspection = false;
#endif

#if defined(ASMJIT_NO_BUILDER)
  constexpr bool no_builder = true;
#else
  constexpr bool no_builder = false;
#endif

#if defined(ASMJIT_NO_COMPILER)
  constexpr bool no_compiler = true;
#else
  constexpr bool no_compiler = false;
#endif

#if defined(ASMJIT_NO_UJIT)
  constexpr bool no_ujit = true;
#else
  constexpr bool no_ujit = false;
#endif

  printf("Build Options:\n");
  printf("  BUILD_TYPE             : %s\n", asmjit_build_type());
  printf("  ASMJIT_NO_DEPRECATED   : %s\n", stringify_build_definition(no_deprecated));
  printf("  ASMJIT_NO_ABI_NAMESPACE: %s\n", stringify_build_definition(no_abi_namespace));
  printf("\n");

  printf("Build Backends:\n");
  printf("  ASMJIT_NO_X86          : %s\n", stringify_build_definition(no_x86));
  printf("  ASMJIT_NO_AARCH64      : %s\n", stringify_build_definition(no_aarch64));
  printf("  ASMJIT_NO_FOREIGN      : %s\n", stringify_build_definition(no_foreign));
  printf("\n");

  printf("Build Features:\n");
  printf("  ASMJIT_NO_SHM_OPEN     : %s\n", stringify_build_definition(no_shm_open));
  printf("  ASMJIT_NO_JIT          : %s\n", stringify_build_definition(no_jit));
  printf("  ASMJIT_NO_TEXT         : %s\n", stringify_build_definition(no_text));
  printf("  ASMJIT_NO_LOGGING      : %s\n", stringify_build_definition(no_logging));
  printf("  ASMJIT_NO_VALIDATION   : %s\n", stringify_build_definition(no_validation));
  printf("  ASMJIT_NO_INTROSPECTION: %s\n", stringify_build_definition(no_introspection));
  printf("  ASMJIT_NO_BUILDER      : %s\n", stringify_build_definition(no_builder));
  printf("  ASMJIT_NO_COMPILER     : %s\n", stringify_build_definition(no_compiler));
  printf("  ASMJIT_NO_UJIT         : %s\n", stringify_build_definition(no_ujit));
  printf("\n");
}

} // {anonymous}

#endif // ASMJITUTILS_H_INCLUDED
