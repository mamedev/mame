// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_X86) && ASMJIT_ARCH_X86
#include <asmjit/x86.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace asmjit;

// Signature of the generated function.
typedef void (*SumIntsFunc)(int* dst, const int* a, const int* b);

// This function works with both x86::Assembler and x86::Builder. It shows how
// `x86::Emitter` can be used to make your code more generic.
static void makeRawFunc(x86::Emitter* emitter) noexcept {
  // Decide which registers will be mapped to function arguments. Try changing
  // registers of `dst`, `src_a`, and `src_b` and see what happens in function's
  // prolog and epilog.
  x86::Gp dst   = emitter->zax();
  x86::Gp src_a = emitter->zcx();
  x86::Gp src_b = emitter->zdx();

  // Decide which vector registers to use. We use these to keep the code generic,
  // you can switch to any other registers when needed.
  x86::Xmm vec0 = x86::xmm0;
  x86::Xmm vec1 = x86::xmm1;

  // Create and initialize `FuncDetail` and `FuncFrame`.
  FuncDetail func;
  func.init(FuncSignatureT<void, int*, const int*, const int*>(CallConvId::kHost), emitter->environment());

  FuncFrame frame;
  frame.init(func);

  // Make XMM0 and XMM1 dirty. VEC group includes XMM|YMM|ZMM registers.
  frame.addDirtyRegs(x86::xmm0, x86::xmm1);

  FuncArgsAssignment args(&func);         // Create arguments assignment context.
  args.assignAll(dst, src_a, src_b);      // Assign our registers to arguments.
  args.updateFuncFrame(frame);            // Reflect our args in FuncFrame.
  frame.finalize();

  // Emit prolog and allocate arguments to registers.
  emitter->emitProlog(frame);
  emitter->emitArgsAssignment(frame, args);

  emitter->movdqu(vec0, x86::ptr(src_a)); // Load 4 ints from [src_a] to XMM0.
  emitter->movdqu(vec1, x86::ptr(src_b)); // Load 4 ints from [src_b] to XMM1.

  emitter->paddd(vec0, vec1);             // Add 4 ints in XMM1 to XMM0.
  emitter->movdqu(x86::ptr(dst), vec0);   // Store the result to [dst].

  // Emit epilog and return.
  emitter->emitEpilog(frame);
}

#ifndef ASMJIT_NO_COMPILER
// This function works with x86::Compiler, provided for comparison.
static void makeCompiledFunc(x86::Compiler* cc) noexcept {
  x86::Gp dst = cc->newIntPtr("dst");
  x86::Gp src_a = cc->newIntPtr("src_a");
  x86::Gp src_b = cc->newIntPtr("src_b");
  x86::Xmm vec0 = cc->newXmm("vec0");
  x86::Xmm vec1 = cc->newXmm("vec1");

  FuncNode* funcNode = cc->addFunc(FuncSignatureT<void, int*, const int*, const int*>(CallConvId::kHost));
  funcNode->setArg(0, dst);
  funcNode->setArg(1, src_a);
  funcNode->setArg(2, src_b);

  cc->movdqu(vec0, x86::ptr(src_a));
  cc->movdqu(vec1, x86::ptr(src_b));
  cc->paddd(vec0, vec1);
  cc->movdqu(x86::ptr(dst), vec0);
  cc->endFunc();
}
#endif

static uint32_t testFunc(JitRuntime& rt, EmitterType emitterType) noexcept {
#ifndef ASMJIT_NO_LOGGING
  FileLogger logger(stdout);
  logger.setIndentation(FormatIndentationGroup::kCode, 2);
#endif

  CodeHolder code;
  code.init(rt.environment());

#ifndef ASMJIT_NO_LOGGING
  code.setLogger(&logger);
#endif

  Error err = kErrorOk;
  switch (emitterType) {
    case EmitterType::kNone: {
      break;
    }

    case EmitterType::kAssembler: {
      printf("Using x86::Assembler:\n");
      x86::Assembler a(&code);
      makeRawFunc(a.as<x86::Emitter>());
      break;
    }

#ifndef ASMJIT_NO_BUILDER
    case EmitterType::kBuilder: {
      printf("Using x86::Builder:\n");
      x86::Builder cb(&code);
      makeRawFunc(cb.as<x86::Emitter>());

      err = cb.finalize();
      if (err) {
        printf("** FAILURE: x86::Builder::finalize() failed (%s) **\n", DebugUtils::errorAsString(err));
        return 1;
      }
      break;
    }
#endif

#ifndef ASMJIT_NO_COMPILER
    case EmitterType::kCompiler: {
      printf("Using x86::Compiler:\n");
      x86::Compiler cc(&code);
      makeCompiledFunc(&cc);

      err = cc.finalize();
      if (err) {
        printf("** FAILURE: x86::Compiler::finalize() failed (%s) **\n", DebugUtils::errorAsString(err));
        return 1;
      }
      break;
    }
#endif
  }

  // Add the code generated to the runtime.
  SumIntsFunc fn;
  err = rt.add(&fn, &code);

  if (err) {
    printf("** FAILURE: JitRuntime::add() failed (%s) **\n", DebugUtils::errorAsString(err));
    return 1;
  }

  // Execute the generated function.
  int inA[4] = { 4, 3, 2, 1 };
  int inB[4] = { 1, 5, 2, 8 };
  int out[4];
  fn(out, inA, inB);

  // Should print {5 8 4 9}.
  printf("Result = { %d %d %d %d }\n\n", out[0], out[1], out[2], out[3]);

  rt.release(fn);
  return !(out[0] == 5 && out[1] == 8 && out[2] == 4 && out[3] == 9);
}

int main() {
  printf("AsmJit Emitters Test-Suite v%u.%u.%u\n",
    unsigned((ASMJIT_LIBRARY_VERSION >> 16)       ),
    unsigned((ASMJIT_LIBRARY_VERSION >>  8) & 0xFF),
    unsigned((ASMJIT_LIBRARY_VERSION      ) & 0xFF));
  printf("\n");

  JitRuntime rt;
  unsigned nFailed = 0;

  nFailed += testFunc(rt, EmitterType::kAssembler);

#ifndef ASMJIT_NO_BUILDER
  nFailed += testFunc(rt, EmitterType::kBuilder);
#endif

#ifndef ASMJIT_NO_COMPILER
  nFailed += testFunc(rt, EmitterType::kCompiler);
#endif

  if (!nFailed)
    printf("** SUCCESS **\n");
  else
    printf("** FAILURE - %u %s failed ** \n", nFailed, nFailed == 1 ? "test" : "tests");

  return nFailed ? 1 : 0;
}
#else
int main() {
  printf("AsmJit X86 Emitter Test is disabled on non-x86 host\n\n");
  return 0;
}
#endif // !ASMJIT_NO_X86 && ASMJIT_ARCH_X86
