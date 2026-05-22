// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

// ----------------------------------------------------------------------------
// This is a working example that demonstrates how multiple sections can be
// used in a JIT-based code generator. It shows also the necessary tooling
// that is expected to be done by the user when the feature is used. It's
// important to handle the following cases:
//
//   - Assign offsets to sections when the code generation is finished.
//   - Tell the CodeHolder to resolve unresolved fixups and check whether
//     all fixups were resolved.
//   - Relocate the code
//   - Copy the code to the destination address.
// ----------------------------------------------------------------------------

#include <asmjit/core.h>
#if ASMJIT_ARCH_X86 && !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_JIT)

#include <asmjit/x86.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

// The generated function is very simple, it only accesses the built-in data
// (from .data section) at the index as provided by its first argument. This
// data is inlined into the resulting function so we can use it this array
// for verification that the function returns correct values.
static const uint8_t data_array[] = { 2, 9, 4, 7, 1, 3, 8, 5, 6, 0 };

static void fail(const char* message, Error err) {
  printf("** FAILURE: %s (%s) **\n", message, DebugUtils::error_as_string(err));
  exit(1);
}

int main() {
  printf("AsmJit X86 Sections Test\n\n");

  Environment env = Environment::host();
  JitAllocator allocator;

#ifndef ASMJIT_NO_LOGGING
  FileLogger logger(stdout);
  logger.set_indentation(FormatIndentationGroup::kCode, 2);
#endif

  CodeHolder code;
  code.init(env);

#ifndef ASMJIT_NO_LOGGING
  code.set_logger(&logger);
#endif

  Section* data_section;
  Error err = code.new_section(Out(data_section), ".data", SIZE_MAX, SectionFlags::kNone, 8);

  if (err != Error::kOk) {
    fail("Failed to create a .data section", err);
  }
  else {
    printf("Generating code:\n");
    x86::Assembler a(&code);
    x86::Gp idx = a.zax();
    x86::Gp addr = a.zcx();

    Label data = a.new_label();

    FuncDetail func;
    func.init(FuncSignature::build<size_t, size_t>(), code.environment());

    FuncFrame frame;
    frame.init(func);
    frame.add_dirty_regs(idx, addr);

    FuncArgsAssignment args(&func);
    args.assign_all(idx);
    args.update_func_frame(frame);
    frame.finalize();

    a.emit_prolog(frame);
    a.emit_args_assignment(frame, args);

    a.lea(addr, x86::ptr(data));
    a.movzx(idx, x86::byte_ptr(addr, idx));

    a.emit_epilog(frame);

    a.section(data_section);
    a.bind(data);

    a.embed(data_array, sizeof(data_array));
  }

  // Manually change he offsets of each section, start at 0. This code is very similar to
  // what `CodeHolder::flatten()` does, however, it's shown here how to do it explicitly.
  printf("\nCalculating section offsets:\n");
  uint64_t offset = 0;
  for (Section* section : code.sections_by_order()) {
    offset = Support::align_up(offset, section->alignment());
    section->set_offset(offset);
    offset += section->real_size();

    printf("  [0x%08X %s] {Id=%u Size=%u}\n",
           uint32_t(section->offset()),
           section->name(),
           section->section_id(),
           uint32_t(section->real_size()));
  }
  size_t code_size = size_t(offset);
  printf("  Final code size: %zu\n", code_size);

  // Resolve cross-section fixups (if any). On 32-bit X86 this is not necessary
  // as this is handled through relocations as the addressing is different.
  if (code.has_unresolved_fixups()) {
    printf("\nResolving cross-section fixups:\n");
    printf("  Before 'resolve_cross_section_fixups()': %zu\n", code.unresolved_fixup_count());

    err = code.resolve_cross_section_fixups();
    if (err != Error::kOk) {
      fail("Failed to resolve cross-section fixups", err);
    }
    printf("  After 'resolve_cross_section_fixups()': %zu\n", code.unresolved_fixup_count());
  }

  // Allocate memory for the function and relocate it there.
  JitAllocator::Span span;
  err = allocator.alloc(Out(span), code_size);
  if (err != Error::kOk)
    fail("Failed to allocate executable memory", err);

  // Relocate to the base-address of the allocated memory.
  code.relocate_to_base(uint64_t(uintptr_t(span.rx())));

  allocator.write(span, [&](JitAllocator::Span& span) noexcept -> Error {
    // Copy the flattened code into `mem.rw`. There are two ways. You can either copy
    // everything manually by iterating over all sections or use `copy_flattened_data`.
    // This code is similar to what `copy_flattened_data(p, code_size, 0)` would do:
    for (Section* section : code.sections_by_order())
      memcpy(static_cast<uint8_t*>(span.rw()) + size_t(section->offset()), section->data(), section->buffer_size());
    return Error::kOk;
  });

  // Execute the function and test whether it works.
  using Func = size_t (*)(size_t idx);
  Func fn = (Func)span.rx();

  printf("\n");
  if (fn(0) != data_array[0] ||
      fn(3) != data_array[3] ||
      fn(6) != data_array[6] ||
      fn(9) != data_array[9] ) {
    printf("** FAILURE: The generated function returned incorrect result(s) **\n");
    return 1;
  }

  printf("** SUCCESS **\n");
  return 0;
}

#else
int main() {
  printf("!! This test is disabled: ASMJIT_NO_JIT or unsuitable target architecture !!\n\n");
  return 0;
}
#endif // ASMJIT_ARCH_X86 && !ASMJIT_NO_X86 && !ASMJIT_NO_JIT
