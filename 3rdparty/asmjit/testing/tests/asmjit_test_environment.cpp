// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86 != 0
  #include <asmjit/x86.h>
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
  #include <asmjit/a64.h>
#endif

#include "../commons/asmjitutils.h"

using namespace asmjit;

static void print_app_info() {
  printf("AsmJit Environment Test v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );

  printf("This application can be used to verify AsmJit build options and to verify the\n");
  printf("environment where it runs. For example to check CPU extensions available, system\n");
  printf("hardening (RWX restrictions), large page support, and virtual memory allocations.\n");
  printf("\n");
}

const char* stringify_bool(bool b) noexcept { return b ? "true" : "false"; };
const char* stringify_result(Error err) noexcept { return err == Error::kOk ? "success" : DebugUtils::error_as_string(err); };

using VoidFunc = void (ASMJIT_CDECL*)(void);

#if !defined(ASMJIT_NO_JIT)

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86 != 0
#define TEST_ENVIRONMENT_HAS_JIT

static void emit_void_function(CodeHolder& code) noexcept {
  x86::Assembler a(&code);
  a.ret();
}
#endif

#if !defined(ASMJIT_NO_AARCH64) && ASMJIT_ARCH_ARM == 64
#define TEST_ENVIRONMENT_HAS_JIT

static void emit_void_function(CodeHolder& code) noexcept {
  a64::Assembler a(&code);
  a.ret(a64::x30);
}
#endif

#if defined(TEST_ENVIRONMENT_HAS_JIT)
static void* offset_pointer(void* ptr, size_t offset) noexcept {
  return static_cast<void*>(static_cast<uint8_t*>(ptr) + offset);
}

static size_t write_empty_function_at(void* ptr, size_t size) noexcept {
  printf("  Write JIT code at addr  : %p\n", ptr);

  CodeHolder code;
  Error err = code.init(Environment::host());
  if (err != Error::kOk) {
    printf(  "Failed to initialize CodeHolder (%s)\n", DebugUtils::error_as_string(err));
    return 0;
  }

  emit_void_function(code);
  code.flatten();
  code.copy_flattened_data(ptr, size);

  return code.code_size();
}

static void flush_instruction_cache(void* ptr, size_t size) noexcept {
  printf("  Flush JIT code at addr  : %p [size=%zu]\n", ptr, size);
  VirtMem::flush_instruction_cache(ptr, size);
}

static void invoke_void_function(void* ptr) noexcept {
  printf("  Invoke JIT code at addr : %p\n", ptr);

  // In case it crashes, we want to have the output flushed.
  fflush(stdout);

  VoidFunc func = reinterpret_cast<VoidFunc>(ptr);
  func();
}
#endif

static void print_virt_mem_info_and_test_execution() noexcept {
  using MemoryFlags = VirtMem::MemoryFlags;
  using HardenedRuntimeInfo = VirtMem::HardenedRuntimeInfo;
  using HardenedRuntimeFlags = VirtMem::HardenedRuntimeFlags;

  // Size of a virtual memory allocation.
  constexpr size_t kVMemAllocSize = 65536;

  // Offset to the first function to execute (must be greater than 8 for UBSAN to work).
  [[maybe_unused]]
  constexpr size_t kVirtFuncOffset = 64;

  size_t large_page_size = VirtMem::large_page_size();
  HardenedRuntimeInfo rti = VirtMem::hardened_runtime_info();

  printf("Large/Huge Pages Info:\n");
  printf("  Large pages supported   : %s\n", stringify_bool(large_page_size != 0u));
  if (large_page_size >= 1024 * 1024) {
    printf("  Large page size         : %zu MiB\n", large_page_size / (1024u * 1024u));
  }
  else if (large_page_size) {
    printf("  Large page size         : %zu KiB\n", large_page_size / 1024u);
  }
  printf("\n");

  printf("Hardened Environment Info:\n");
  printf("  Hardening was detected  : %s\n", stringify_bool(rti.has_flag(HardenedRuntimeFlags::kEnabled    )));
  printf("  MAP_JIT is available    : %s\n", stringify_bool(rti.has_flag(HardenedRuntimeFlags::kMapJit     )));
  printf("  DualMapping is available: %s\n", stringify_bool(rti.has_flag(HardenedRuntimeFlags::kDualMapping)));
  printf("\n");

  if (!rti.has_flag(HardenedRuntimeFlags::kEnabled)) {
    printf("Virtual Memory Allocation (RWX):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRWX);
    printf("  Alloc virt memory (RWX) : %s\n", stringify_result(result));

    if (result == Error::kOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* func_ptr = offset_pointer(ptr, kVirtFuncOffset);
      size_t func_size = write_empty_function_at(func_ptr, kVMemAllocSize);

      if (func_size) {
        flush_instruction_cache(func_ptr, func_size);
        invoke_void_function(func_ptr);
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory     : %s\n", stringify_result(result));
    }

    printf("\n");
  }

  {
    printf("Virtual Memory Allocation (RW - Flipping Permissions RW<->RX):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRW | MemoryFlags::kMMapMaxAccessRWX);
    printf("  Alloc virt memory (RW)  : %s (allocation uses kMMapMaxAccessRWX)\n", stringify_result(result));

    if (result == Error::kOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* func_ptr = offset_pointer(ptr, kVirtFuncOffset);
      size_t func_size = write_empty_function_at(func_ptr, kVMemAllocSize);
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRX);
      printf("  Protect virt memory (RX): %s\n", stringify_result(result));

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      if (func_size) {
        flush_instruction_cache(func_ptr, func_size);
        invoke_void_function(func_ptr);
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::protect(ptr, kVMemAllocSize, MemoryFlags::kAccessRW);
      printf("  Protect virt memory (RW): %s\n", stringify_result(result));

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory (RW): %s\n", stringify_result(result));
    }

    printf("\n");
  }

  if (rti.has_flag(HardenedRuntimeFlags::kMapJit)) {
    printf("Virtual Memory Allocation (MAP_JIT):\n");

    void* ptr = nullptr;
    Error result = VirtMem::alloc(&ptr, kVMemAllocSize, MemoryFlags::kAccessRWX | MemoryFlags::kMMapEnableMapJit);
    printf("  Alloc virt mem (RWX)    : %s (allocation uses kMMapEnableMapJit)\n", stringify_result(result));

    if (result == Error::kOk) {
      printf("  Protect JIT Memory (RW) : (per-thread protection)\n");
      VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadWrite);

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      void* func_ptr = offset_pointer(ptr, kVirtFuncOffset);
      size_t func_size = write_empty_function_at(func_ptr, kVMemAllocSize);
#endif // TEST_ENVIRONMENT_HAS_JIT

      printf("  Protect JIT Memory (RX) : (per-thread protection)\n");
      VirtMem::protect_jit_memory(VirtMem::ProtectJitAccess::kReadExecute);

#if defined(TEST_ENVIRONMENT_HAS_JIT)
      if (func_size) {
        flush_instruction_cache(func_ptr, func_size);
        invoke_void_function(func_ptr);
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release(ptr, kVMemAllocSize);
      printf("  Release virt memory     : %s\n", stringify_result(result));
    }

    printf("\n");
  }

  if (rti.has_flag(HardenedRuntimeFlags::kDualMapping)) {
    printf("Virtual Memory Allocation (Dual Mapping):\n");

    VirtMem::DualMapping dm {};
    Error result = VirtMem::alloc_dual_mapping(Out(dm), kVMemAllocSize, MemoryFlags::kAccessRWX);
    printf("  Alloc dual mem (RW+RX)  : %s\n", stringify_result(result));

    if (result == Error::kOk) {
#if defined(TEST_ENVIRONMENT_HAS_JIT)
      size_t func_size = write_empty_function_at(offset_pointer(dm.rw, kVirtFuncOffset), kVMemAllocSize);
      if (func_size) {
        flush_instruction_cache(offset_pointer(dm.rx, kVirtFuncOffset), func_size);
        invoke_void_function(offset_pointer(dm.rx, kVirtFuncOffset));
      }
#endif // TEST_ENVIRONMENT_HAS_JIT

      result = VirtMem::release_dual_mapping(dm, kVMemAllocSize);
      printf("  Release dual mem (RW+RX): %s\n", stringify_result(result));
    }

    printf("\n");
  }
}

#if defined(TEST_ENVIRONMENT_HAS_JIT)
static void print_jit_runtime_info_and_test_execution_with_params(const JitAllocator::CreateParams* params, const char* params_name) noexcept {
  printf("JitRuntime (%s):\n", params_name);

  JitRuntime rt(params);
  CodeHolder code;

  Error result = code.init(rt.environment());
  printf("  CodeHolder init result  : %s\n", stringify_result(result));

  if (result != Error::kOk) {
    return;
  }

  emit_void_function(code);
  VoidFunc fn;

  result = rt.add(&fn, &code);
  printf("  Runtime.add() result    : %s\n", stringify_result(result));

  if (result == Error::kOk) {
    invoke_void_function((void*)fn);

    result = rt.release(fn);
    printf("  Runtime.release() result: %s\n", stringify_result(result));
  }

  printf("\n");
}

static void print_jit_runtime_info_and_test_execution() noexcept {
  print_jit_runtime_info_and_test_execution_with_params(nullptr, "<no params>");

  if (VirtMem::large_page_size()) {
    JitAllocator::CreateParams p{};
    p.options = JitAllocatorOptions::kUseLargePages;

    print_jit_runtime_info_and_test_execution_with_params(&p, "large pages");
  }
}
#endif // TEST_ENVIRONMENT_HAS_JIT

#endif // !ASMJIT_NO_JIT

int main() {
  print_app_info();
  print_build_options();
  print_cpu_info();

#if !defined(ASMJIT_NO_JIT)
  print_virt_mem_info_and_test_execution();
#endif // !ASMJIT_NO_JIT

#if !defined(ASMJIT_NO_JIT) && defined(TEST_ENVIRONMENT_HAS_JIT)
  print_jit_runtime_info_and_test_execution();
#endif // !ASMJIT_NO_JIT && TEST_ENVIRONMENT_HAS_JIT

  return 0;
}
