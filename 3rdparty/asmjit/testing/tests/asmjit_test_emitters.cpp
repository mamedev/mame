// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asmjit/core.h>
#include "../commons/asmjitutils.h"

#if ASMJIT_ARCH_X86 != 0
  #include <asmjit/x86.h>
#endif

#if ASMJIT_ARCH_ARM == 64
  #include <asmjit/a64.h>
#endif

using namespace asmjit;

static void print_app_info() noexcept {
  printf("AsmJit Emitters Test-Suite v%u.%u.%u [Arch=%s] [Mode=%s]\n\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF),
    asmjit_arch_as_string(Arch::kHost),
    asmjit_build_type()
  );
}

#if !defined(ASMJIT_NO_JIT) && ((ASMJIT_ARCH_X86 != 0  && !defined(ASMJIT_NO_X86    )) || \
                                (ASMJIT_ARCH_ARM == 64 && !defined(ASMJIT_NO_AARCH64)) )

// Signature of the generated function.
using SumIntsFunc = void (*)(int* dst, const int* a, const int* b);

// X86 Backend
// -----------

#if ASMJIT_ARCH_X86 != 0
// This function works with both x86::Assembler and x86::Builder. It shows how
// `x86::Emitter` can be used to make your code more generic.
static void generate_func_with_emitter(x86::Emitter* emitter) noexcept {
  // Decide which registers will be mapped to function arguments. Try changing
  // registers of `dst`, `src_a`, and `src_b` and see what happens in function's
  // prolog and epilog.
  x86::Gp dst   = emitter->zax();
  x86::Gp src_a = emitter->zcx();
  x86::Gp src_b = emitter->zdx();

  // Decide which vector registers to use. We use these to keep the code generic,
  // you can switch to any other registers when needed.
  x86::Vec vec0 = x86::xmm0;
  x86::Vec vec1 = x86::xmm1;

  // Create and initialize `FuncDetail` and `FuncFrame`.
  FuncDetail func;
  func.init(FuncSignature::build<void, int*, const int*, const int*>(), emitter->environment());

  FuncFrame frame;
  frame.init(func);

  // Make or registers dirty.
  frame.add_dirty_regs(vec0, vec1);

  FuncArgsAssignment args(&func);         // Create arguments assignment context.
  args.assign_all(dst, src_a, src_b);      // Assign our registers to arguments.
  args.update_func_frame(frame);            // Reflect our args in FuncFrame.
  frame.finalize();

  // Emit prolog and allocate arguments to registers.
  emitter->emit_prolog(frame);
  emitter->emit_args_assignment(frame, args);

  emitter->movdqu(vec0, x86::ptr(src_a)); // Load 4 ints from [src_a] to XMM0.
  emitter->movdqu(vec1, x86::ptr(src_b)); // Load 4 ints from [src_b] to XMM1.

  emitter->paddd(vec0, vec1);             // Add 4 ints in XMM1 to XMM0.
  emitter->movdqu(x86::ptr(dst), vec0);   // Store the result to [dst].

  // Emit epilog and return.
  emitter->emit_epilog(frame);
}

#ifndef ASMJIT_NO_COMPILER
// This function works with x86::Compiler, provided for comparison.
static void generate_func_with_compiler(x86::Compiler* cc) noexcept {
  x86::Gp dst = cc->new_gp_ptr("dst");
  x86::Gp src_a = cc->new_gp_ptr("src_a");
  x86::Gp src_b = cc->new_gp_ptr("src_b");
  x86::Vec vec0 = cc->new_xmm("vec0");
  x86::Vec vec1 = cc->new_xmm("vec1");

  FuncNode* func_node = cc->add_func(FuncSignature::build<void, int*, const int*, const int*>());
  func_node->set_arg(0, dst);
  func_node->set_arg(1, src_a);
  func_node->set_arg(2, src_b);

  cc->movdqu(vec0, x86::ptr(src_a));
  cc->movdqu(vec1, x86::ptr(src_b));
  cc->paddd(vec0, vec1);
  cc->movdqu(x86::ptr(dst), vec0);
  cc->end_func();
}
#endif

static Error generate_func(CodeHolder& code, EmitterType emitter_type) noexcept {
  switch (emitter_type) {
    case EmitterType::kAssembler: {
      printf("Using x86::Assembler:\n");
      x86::Assembler a(&code);
      generate_func_with_emitter(a.as<x86::Emitter>());
      return Error::kOk;
    }

#ifndef ASMJIT_NO_BUILDER
    case EmitterType::kBuilder: {
      printf("Using x86::Builder:\n");
      x86::Builder cb(&code);
      generate_func_with_emitter(cb.as<x86::Emitter>());

      return cb.finalize();
    }
#endif

#ifndef ASMJIT_NO_COMPILER
    case EmitterType::kCompiler: {
      printf("Using x86::Compiler:\n");
      x86::Compiler cc(&code);
      generate_func_with_compiler(&cc);

      return cc.finalize();
    }
#endif

    default: {
      printf("** FAILURE: No emitter to use **\n");
      exit(1);
    }
  }
}
#endif

// AArch64 Backend
// ---------------

#if ASMJIT_ARCH_ARM == 64
// This function works with both a64::Assembler and a64::Builder. It shows how
// `a64::Emitter` can be used to make your code more generic.
static void generate_func_with_emitter(a64::Emitter* emitter) noexcept {
  // Decide which registers will be mapped to function arguments. Try changing
  // registers of `dst`, `src_a`, and `src_b` and see what happens in function's
  // prolog and epilog.
  a64::Gp dst   = a64::x0;
  a64::Gp src_a = a64::x1;
  a64::Gp src_b = a64::x2;

  // Decide which vector registers to use. We use these to keep the code generic,
  // you can switch to any other registers when needed.
  a64::Vec vec0 = a64::v0;
  a64::Vec vec1 = a64::v1;
  a64::Vec vec2 = a64::v2;

  // Create and initialize `FuncDetail` and `FuncFrame`.
  FuncDetail func;
  func.init(FuncSignature::build<void, int*, const int*, const int*>(), emitter->environment());

  FuncFrame frame;
  frame.init(func);

  // Make XMM0 and XMM1 dirty. VEC group includes XMM|YMM|ZMM registers.
  frame.add_dirty_regs(vec0, vec1, vec2);

  FuncArgsAssignment args(&func);                // Create arguments assignment context.
  args.assign_all(dst, src_a, src_b);             // Assign our registers to arguments.
  args.update_func_frame(frame);                   // Reflect our args in FuncFrame.
  frame.finalize();

  // Emit prolog and allocate arguments to registers.
  emitter->emit_prolog(frame);
  emitter->emit_args_assignment(frame, args);

  emitter->ld1(vec0.b16(), a64::ptr(src_a));     // Load 4 ints from [src_a] to vec0.
  emitter->ld1(vec1.b16(), a64::ptr(src_b));     // Load 4 ints from [src_b] to vec1.
  emitter->add(vec2.s4(), vec0.s4(), vec1.s4()); // Add 4 ints of vec0 and vec1 and store to vec2.
  emitter->st1(vec2.b16(), a64::ptr(dst));       // Store the result (vec2) to [dst].

  // Emit epilog and return.
  emitter->emit_epilog(frame);
}

#ifndef ASMJIT_NO_COMPILER
// This function works with x86::Compiler, provided for comparison.
static void generate_func_with_compiler(a64::Compiler* cc) noexcept {
  a64::Gp dst = cc->new_gp_ptr("dst");
  a64::Gp src_a = cc->new_gp_ptr("src_a");
  a64::Gp src_b = cc->new_gp_ptr("src_b");
  a64::Vec vec0 = cc->new_vec_q("vec0");
  a64::Vec vec1 = cc->new_vec_q("vec1");
  a64::Vec vec2 = cc->new_vec_q("vec2");

  FuncNode* func_node = cc->add_func(FuncSignature::build<void, int*, const int*, const int*>());
  func_node->set_arg(0, dst);
  func_node->set_arg(1, src_a);
  func_node->set_arg(2, src_b);

  cc->ld1(vec0.b16(), a64::ptr(src_a));          // Load 4 ints from [src_a] to vec0.
  cc->ld1(vec1.b16(), a64::ptr(src_b));          // Load 4 ints from [src_b] to vec1.
  cc->add(vec2.s4(), vec0.s4(), vec1.s4());      // Add 4 ints of vec0 and vec1 and store to vec2.
  cc->st1(vec2.b16(), a64::ptr(dst));            // Store the result (vec2) to [dst].
  cc->end_func();
}
#endif

static Error generate_func(CodeHolder& code, EmitterType emitter_type) noexcept {
  switch (emitter_type) {
    case EmitterType::kAssembler: {
      printf("Using a64::Assembler:\n");
      a64::Assembler a(&code);
      generate_func_with_emitter(a.as<a64::Emitter>());
      return Error::kOk;
    }

#ifndef ASMJIT_NO_BUILDER
    case EmitterType::kBuilder: {
      printf("Using a64::Builder:\n");
      a64::Builder cb(&code);
      generate_func_with_emitter(cb.as<a64::Emitter>());

      return cb.finalize();
    }
#endif

#ifndef ASMJIT_NO_COMPILER
    case EmitterType::kCompiler: {
      printf("Using a64::Compiler:\n");
      a64::Compiler cc(&code);
      generate_func_with_compiler(&cc);

      return cc.finalize();
    }
#endif

    default: {
      printf("** FAILURE: No emitter to use **\n");
      exit(1);
    }
  }
}
#endif

// Testing
// -------

static uint32_t test_func(JitRuntime& rt, EmitterType emitter_type) noexcept {
#ifndef ASMJIT_NO_LOGGING
  FileLogger logger(stdout);
  logger.set_indentation(FormatIndentationGroup::kCode, 2);
#endif

  CodeHolder code;
  code.init(rt.environment(), rt.cpu_features());

#ifndef ASMJIT_NO_LOGGING
  code.set_logger(&logger);
#endif

  Error err = generate_func(code, emitter_type);
  if (err != Error::kOk) {
    printf("** FAILURE: Failed to generate a function: %s **\n", DebugUtils::error_as_string(err));
    return 1;
  }

  // Add the code generated to the runtime.
  SumIntsFunc fn;
  err = rt.add(&fn, &code);

  if (err != Error::kOk) {
    printf("** FAILURE: JitRuntime::add() failed: %s **\n", DebugUtils::error_as_string(err));
    return 1;
  }

  // Execute the generated function.
  static const int in_a[4] = { 4, 3, 2, 1 };
  static const int in_b[4] = { 1, 5, 2, 8 };
  int out[4] {};
  fn(out, in_a, in_b);

  // Should print {5 8 4 9}.
  printf("Result = { %d %d %d %d }\n\n", out[0], out[1], out[2], out[3]);

  rt.release(fn);
  return out[0] == 5 && out[1] == 8 && out[2] == 4 && out[3] == 9;
}

int main() {
  print_app_info();

  JitRuntime rt;
  unsigned failed_count = 0;

  failed_count += !test_func(rt, EmitterType::kAssembler);

#ifndef ASMJIT_NO_BUILDER
  failed_count += !test_func(rt, EmitterType::kBuilder);
#endif

#ifndef ASMJIT_NO_COMPILER
  failed_count += !test_func(rt, EmitterType::kCompiler);
#endif

  if (!failed_count)
    printf("** SUCCESS **\n");
  else
    printf("** FAILURE - %u %s failed ** \n", failed_count, failed_count == 1 ? "test" : "tests");

  return failed_count ? 1 : 0;
}
#else
int main() {
  print_app_info();
  printf("!! This test is disabled: <ASMJIT_NO_JIT> or unsuitable target architecture !!\n");
  return 0;
}
#endif // ASMJIT_ARCH_X86 && !ASMJIT_NO_X86 && !ASMJIT_NO_JIT
