// This file is part of AsmJit project <https://asmjit.com>
//
// See asmjit.h or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#if !defined(ASMJIT_NO_X86) && !defined(ASMJIT_NO_COMPILER)

#include <asmjit/x86.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if ASMJIT_ARCH_X86
  // Required for function tests that pass / return XMM registers.
  #include <emmintrin.h>
#endif

#include "asmjit_test_misc.h"
#include "asmjit_test_compiler.h"

#ifdef _MSC_VER
// Interaction between '_setjmp' and C++ object destruction is non-portable.
#pragma warning(disable: 4611)
#endif

using namespace asmjit;

// x86::Compiler - X86TestCase
// ===========================

class X86TestCase : public TestCase {
public:
  X86TestCase(const char* name = nullptr)
    : TestCase(name, Arch::kHost == Arch::kX86 ? Arch::kX86 : Arch::kX64) {}

  virtual void compile(BaseCompiler& cc) override {
    compile(static_cast<x86::Compiler&>(cc));
  }

  virtual void compile(x86::Compiler& cc) = 0;
};

// x86::Compiler - X86Test_AlignBase
// =================================

class X86Test_AlignBase : public X86TestCase {
public:
  X86Test_AlignBase(uint32_t argCount, uint32_t alignment, bool preserveFP)
    : _argCount(argCount),
      _alignment(alignment),
      _preserveFP(preserveFP) {
    _name.assignFormat("AlignBase {NumArgs=%u Alignment=%u PreserveFP=%c}", argCount, alignment, preserveFP ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i <= 16; i++) {
      for (uint32_t a = 16; a <= 32; a += 16) {
        app.add(new X86Test_AlignBase(i, a, true));
        app.add(new X86Test_AlignBase(i, a, false));
      }
    }
  }

  virtual void compile(x86::Compiler& cc) {
    uint32_t i;
    uint32_t argCount = _argCount;

    FuncSignature signature(CallConvId::kCDecl);
    signature.setRetT<int>();
    for (i = 0; i < argCount; i++)
      signature.addArgT<int>();

    FuncNode* funcNode = cc.addFunc(signature);
    if (_preserveFP)
      funcNode->frame().setPreservedFP();

    x86::Gp gpVar = cc.newIntPtr("gpVar");
    x86::Gp gpSum;
    x86::Mem stack = cc.newStack(_alignment, _alignment);

    // Do a sum of arguments to verify a possible relocation when misaligned.
    if (argCount) {
      for (i = 0; i < argCount; i++) {
        x86::Gp gpArg = cc.newInt32("gpArg%u", i);
        funcNode->setArg(i, gpArg);

        if (i == 0)
          gpSum = gpArg;
        else
          cc.add(gpSum, gpArg);
      }
    }

    // Check alignment of xmmVar (has to be 16).
    cc.lea(gpVar, stack);
    cc.and_(gpVar, _alignment - 1);

    // Add a sum of all arguments to check if they are correct.
    if (argCount)
      cc.or_(gpVar.r32(), gpSum);

    cc.ret(gpVar);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef unsigned int U;

    typedef U (*Func0)();
    typedef U (*Func1)(U);
    typedef U (*Func2)(U, U);
    typedef U (*Func3)(U, U, U);
    typedef U (*Func4)(U, U, U, U);
    typedef U (*Func5)(U, U, U, U, U);
    typedef U (*Func6)(U, U, U, U, U, U);
    typedef U (*Func7)(U, U, U, U, U, U, U);
    typedef U (*Func8)(U, U, U, U, U, U, U, U);
    typedef U (*Func9)(U, U, U, U, U, U, U, U, U);
    typedef U (*Func10)(U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func11)(U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func12)(U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func13)(U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func14)(U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func15)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    typedef U (*Func16)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);

    unsigned int resultRet = 0;
    unsigned int expectRet = 0;

    switch (_argCount) {
      case 0:
        resultRet = ptr_as_func<Func0>(_func)();
        expectRet = 0;
        break;
      case 1:
        resultRet = ptr_as_func<Func1>(_func)(1);
        expectRet = 1;
        break;
      case 2:
        resultRet = ptr_as_func<Func2>(_func)(1, 2);
        expectRet = 1 + 2;
        break;
      case 3:
        resultRet = ptr_as_func<Func3>(_func)(1, 2, 3);
        expectRet = 1 + 2 + 3;
        break;
      case 4:
        resultRet = ptr_as_func<Func4>(_func)(1, 2, 3, 4);
        expectRet = 1 + 2 + 3 + 4;
        break;
      case 5:
        resultRet = ptr_as_func<Func5>(_func)(1, 2, 3, 4, 5);
        expectRet = 1 + 2 + 3 + 4 + 5;
        break;
      case 6:
        resultRet = ptr_as_func<Func6>(_func)(1, 2, 3, 4, 5, 6);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6;
        break;
      case 7:
        resultRet = ptr_as_func<Func7>(_func)(1, 2, 3, 4, 5, 6, 7);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7;
        break;
      case 8:
        resultRet = ptr_as_func<Func8>(_func)(1, 2, 3, 4, 5, 6, 7, 8);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;
        break;
      case 9:
        resultRet = ptr_as_func<Func9>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
        break;
      case 10:
        resultRet = ptr_as_func<Func10>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;
        break;
      case 11:
        resultRet = ptr_as_func<Func11>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
        break;
      case 12:
        resultRet = ptr_as_func<Func12>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12;
        break;
      case 13:
        resultRet = ptr_as_func<Func13>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13;
        break;
      case 14:
        resultRet = ptr_as_func<Func14>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14;
        break;
      case 15:
        resultRet = ptr_as_func<Func15>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15;
        break;
      case 16:
        resultRet = ptr_as_func<Func16>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        expectRet = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
        break;
    }

    result.assignFormat("ret={%u, %u}", resultRet >> 28, resultRet & 0x0FFFFFFFu);
    expect.assignFormat("ret={%u, %u}", expectRet >> 28, expectRet & 0x0FFFFFFFu);

    return result == expect;
  }

  uint32_t _argCount;
  uint32_t _alignment;
  bool _preserveFP;
};

// x86::Compiler - X86Test_NoCode
// ==============================

class X86Test_NoCode : public X86TestCase {
public:
  X86Test_NoCode() : X86TestCase("NoCode") {}

  static void add(TestApp& app) {
    app.add(new X86Test_NoCode());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<void>());
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    DebugUtils::unused(result, expect);

    typedef void(*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();
    return true;
  }
};

// x86::Compiler - X86Test_NoAlign
// ===============================

class X86Test_NoAlign : public X86TestCase {
public:
  X86Test_NoAlign() : X86TestCase("NoAlign") {}

  static void add(TestApp& app) {
    app.add(new X86Test_NoAlign());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<void>());
    cc.align(AlignMode::kCode, 0);
    cc.align(AlignMode::kCode, 1);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    DebugUtils::unused(result, expect);

    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();
    return true;
  }
};

// x86::Compiler - X86Test_IndirectBranchProtection
// ================================================

class X86Test_IndirectBranchProtection : public X86TestCase {
public:
  X86Test_IndirectBranchProtection() : X86TestCase("IndirectBranchProtection") {}

  static void add(TestApp& app) {
    app.add(new X86Test_IndirectBranchProtection());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* func = cc.addFunc(FuncSignature::build<void>());
    func->addAttributes(FuncAttributes::kIndirectBranchProtection);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    DebugUtils::unused(result, expect);

    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();
    return true;
  }
};


// x86::Compiler - X86Test_JumpMerge
// =================================

class X86Test_JumpMerge : public X86TestCase {
public:
  X86Test_JumpMerge() : X86TestCase("JumpMerge") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpMerge());
  }

  virtual void compile(x86::Compiler& cc) {
    Label L0 = cc.newLabel();
    Label L1 = cc.newLabel();
    Label L2 = cc.newLabel();
    Label LEnd = cc.newLabel();

    x86::Gp dst = cc.newIntPtr("dst");
    x86::Gp val = cc.newInt32("val");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, int*, int>());
    funcNode->setArg(0, dst);
    funcNode->setArg(1, val);

    cc.cmp(val, 0);
    cc.je(L2);

    cc.cmp(val, 1);
    cc.je(L1);

    cc.cmp(val, 2);
    cc.je(L0);

    cc.mov(x86::dword_ptr(dst), val);
    cc.jmp(LEnd);

    // On purpose. This tests whether the CFG constructs a single basic-block
    // from multiple labels next to each other.
    cc.bind(L0);
    cc.bind(L1);
    cc.bind(L2);
    cc.mov(x86::dword_ptr(dst), 0);

    cc.bind(LEnd);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void(*Func)(int*, int);
    Func func = ptr_as_func<Func>(_func);

    int arr[5] = { -1, -1, -1, -1, -1 };
    int exp[5] = {  0,  0,  0,  3,  4 };

    for (int i = 0; i < 5; i++)
      func(&arr[i], i);

    result.assignFormat("ret={%d, %d, %d, %d, %d}", arr[0], arr[1], arr[2], arr[3], arr[4]);
    expect.assignFormat("ret={%d, %d, %d, %d, %d}", exp[0], exp[1], exp[2], exp[3], exp[4]);

    return result == expect;
  }
};

// x86::Compiler - X86Test_JumpCross
// =================================

class X86Test_JumpCross : public X86TestCase {
public:
  X86Test_JumpCross() : X86TestCase("JumpCross") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpCross());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<void>());

    Label L1 = cc.newLabel();
    Label L2 = cc.newLabel();
    Label L3 = cc.newLabel();

    cc.jmp(L2);

    cc.bind(L1);
    cc.jmp(L3);

    cc.bind(L2);
    cc.jmp(L1);

    cc.bind(L3);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    DebugUtils::unused(result, expect);

    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();
    return true;
  }
};

// x86::Compiler - X86Test_JumpMany
// ================================

class X86Test_JumpMany : public X86TestCase {
public:
  X86Test_JumpMany() : X86TestCase("JumpMany") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpMany());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());
    for (uint32_t i = 0; i < 1000; i++) {
      Label L = cc.newLabel();
      cc.jmp(L);
      cc.bind(L);
    }

    x86::Gp ret = cc.newInt32("ret");
    cc.xor_(ret, ret);
    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);

    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 0;

    result.assignFormat("ret={%d}", resultRet);
    expect.assignFormat("ret={%d}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_JumpUnreachable1
// ========================================

class X86Test_JumpUnreachable1 : public X86TestCase {
public:
  X86Test_JumpUnreachable1() : X86TestCase("JumpUnreachable1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpUnreachable1());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<void>());

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();
    Label L_3 = cc.newLabel();
    Label L_4 = cc.newLabel();
    Label L_5 = cc.newLabel();
    Label L_6 = cc.newLabel();
    Label L_7 = cc.newLabel();

    x86::Gp v0 = cc.newUInt32("v0");
    x86::Gp v1 = cc.newUInt32("v1");

    cc.bind(L_2);
    cc.bind(L_3);

    cc.jmp(L_1);

    cc.bind(L_5);
    cc.mov(v0, 0);

    cc.bind(L_6);
    cc.jmp(L_3);
    cc.mov(v1, 1);
    cc.jmp(L_1);

    cc.bind(L_4);
    cc.jmp(L_2);
    cc.bind(L_7);
    cc.add(v0, v1);

    cc.align(AlignMode::kCode, 16);
    cc.bind(L_1);
    cc.ret();
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();

    result.append("ret={}");
    expect.append("ret={}");

    return true;
  }
};

// x86::Compiler - X86Test_JumpUnreachable2
// ========================================

class X86Test_JumpUnreachable2 : public X86TestCase {
public:
  X86Test_JumpUnreachable2() : X86TestCase("JumpUnreachable2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpUnreachable2());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<void>());

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();

    x86::Gp v0 = cc.newUInt32("v0");
    x86::Gp v1 = cc.newUInt32("v1");

    cc.jmp(L_1);
    cc.bind(L_2);
    cc.mov(v0, 1);
    cc.mov(v1, 2);
    cc.cmp(v0, v1);
    cc.jz(L_2);
    cc.jmp(L_1);

    cc.bind(L_1);
    cc.ret();
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    func();

    result.append("ret={}");
    expect.append("ret={}");

    return true;
  }
};

// x86::Compiler - X86Test_JumpTable1
// ==================================

class X86Test_JumpTable1 : public X86TestCase {
public:
  bool _annotated;

  X86Test_JumpTable1(bool annotated)
    : X86TestCase("X86Test_JumpTable1"),
      _annotated(annotated) {
    _name.assignFormat("JumpTable {%s}", annotated ? "Annotated" : "Unknown Reg/Mem");
  }

  enum Operator {
    kOperatorAdd = 0,
    kOperatorSub = 1,
    kOperatorMul = 2,
    kOperatorDiv = 3
  };

  static void add(TestApp& app) {
    app.add(new X86Test_JumpTable1(false));
    app.add(new X86Test_JumpTable1(true));
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Xmm a = cc.newXmmSs("a");
    x86::Xmm b = cc.newXmmSs("b");
    x86::Gp op = cc.newUInt32("op");
    x86::Gp target = cc.newIntPtr("target");
    x86::Gp offset = cc.newIntPtr("offset");

    Label L_Table = cc.newLabel();
    Label L_Add = cc.newLabel();
    Label L_Sub = cc.newLabel();
    Label L_Mul = cc.newLabel();
    Label L_Div = cc.newLabel();
    Label L_End = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<float, float, float, uint32_t>());
    funcNode->setArg(0, a);
    funcNode->setArg(1, b);
    funcNode->setArg(2, op);

    cc.lea(offset, x86::ptr(L_Table));
    if (cc.is64Bit())
      cc.movsxd(target, x86::dword_ptr(offset, op.cloneAs(offset), 2));
    else
      cc.mov(target, x86::dword_ptr(offset, op.cloneAs(offset), 2));
    cc.add(target, offset);

    // JumpAnnotation allows to annotate all possible jump targets of
    // instructions where it cannot be deduced from operands.
    if (_annotated) {
      JumpAnnotation* annotation = cc.newJumpAnnotation();
      annotation->addLabel(L_Add);
      annotation->addLabel(L_Sub);
      annotation->addLabel(L_Mul);
      annotation->addLabel(L_Div);
      cc.jmp(target, annotation);
    }
    else {
      cc.jmp(target);
    }

    cc.bind(L_Add);
    cc.addss(a, b);
    cc.jmp(L_End);

    cc.bind(L_Sub);
    cc.subss(a, b);
    cc.jmp(L_End);

    cc.bind(L_Mul);
    cc.mulss(a, b);
    cc.jmp(L_End);

    cc.bind(L_Div);
    cc.divss(a, b);

    cc.bind(L_End);
    cc.ret(a);

    cc.endFunc();

    cc.bind(L_Table);
    cc.embedLabelDelta(L_Add, L_Table, 4);
    cc.embedLabelDelta(L_Sub, L_Table, 4);
    cc.embedLabelDelta(L_Mul, L_Table, 4);
    cc.embedLabelDelta(L_Div, L_Table, 4);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef float (*Func)(float, float, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    float results[4];
    float expected[4];

    results[0] = func(33.0f, 14.0f, kOperatorAdd);
    results[1] = func(33.0f, 14.0f, kOperatorSub);
    results[2] = func(10.0f, 6.0f, kOperatorMul);
    results[3] = func(80.0f, 8.0f, kOperatorDiv);

    expected[0] = 47.0f;
    expected[1] = 19.0f;
    expected[2] = 60.0f;
    expected[3] = 10.0f;

    result.assignFormat("ret={%f, %f, %f, %f}", results[0], results[1], results[2], results[3]);
    expect.assignFormat("ret={%f, %f, %f, %f}", expected[0], expected[1], expected[2], expected[3]);

    return result == expect;
  }
};

// x86::Compiler - X86Test_JumpTable2
// ==================================

class X86Test_JumpTable2 : public X86TestCase {
public:
  X86Test_JumpTable2()
    : X86TestCase("JumpTable {Jumping to Begin}") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpTable2());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp result = cc.newUInt32("result");
    x86::Gp value = cc.newUInt32("value");
    x86::Gp target = cc.newIntPtr("target");
    x86::Gp offset = cc.newIntPtr("offset");

    Label L_Table = cc.newLabel();
    Label L_Begin = cc.newLabel();
    Label L_Case0 = cc.newLabel();
    Label L_Case1 = cc.newLabel();
    Label L_End = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int>());
    funcNode->setArg(0, value);

    cc.bind(L_Begin);
    cc.lea(offset, x86::ptr(L_Table));
    if (cc.is64Bit())
      cc.movsxd(target, x86::dword_ptr(offset, value.cloneAs(offset), 2));
    else
      cc.mov(target, x86::dword_ptr(offset, value.cloneAs(offset), 2));
    cc.add(target, offset);

    {
      JumpAnnotation* annotation = cc.newJumpAnnotation();
      annotation->addLabel(L_Case0);
      annotation->addLabel(L_Case1);
      annotation->addLabel(L_Begin); // Never used, just for the purpose of the test.
      cc.jmp(target, annotation);

      cc.bind(L_Case0);
      cc.mov(result, 0);
      cc.jmp(L_End);

      cc.bind(L_Case1);
      cc.mov(result, 1);
      cc.jmp(L_End);
    }

    cc.bind(L_End);
    cc.ret(result);

    cc.endFunc();

    cc.bind(L_Table);
    cc.embedLabelDelta(L_Case0, L_Table, 4);
    cc.embedLabelDelta(L_Case1, L_Table, 4);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int);
    Func func = ptr_as_func<Func>(_func);

    int results[2];
    int expected[2];

    results[0] = func(0);
    results[1] = func(1);

    expected[0] = 0;
    expected[1] = 1;

    result.assignFormat("ret={%d, %d}", results[0], results[1]);
    expect.assignFormat("ret={%d, %d}", expected[0], expected[1]);

    return result == expect;
  }
};

// x86::Compiler - X86Test_JumpTable3
// ==================================

class X86Test_JumpTable3 : public X86TestCase {
public:
  X86Test_JumpTable3()
    : X86TestCase("JumpTable {Jumping to a single label}") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpTable3());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    Label L_Target = cc.newLabel();
    x86::Gp target = cc.newUIntPtr("target");
    x86::Gp result = cc.newUInt32("result");

    JumpAnnotation* annotation = cc.newJumpAnnotation();
    annotation->addLabel(L_Target);

    cc.lea(target, x86::ptr(L_Target));
    cc.jmp(target, annotation);

    cc.bind(L_Target);
    cc.mov(result, 1234);
    cc.ret(result);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int out = func();
    int expected = 1234;

    result.assignFormat("ret=%d", out);
    expect.assignFormat("ret=%d", expected);

    return result == expect;
  }
};

// x86::Compiler - X86Test_JumpTable4
// ==================================

class X86Test_JumpTable4 : public X86TestCase {
public:
  X86Test_JumpTable4()
    : X86TestCase("JumpTable {Jumping to a single label and multiple labels}") {}

  static void add(TestApp& app) {
    app.add(new X86Test_JumpTable4());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp result = cc.newUInt32("result");
    x86::Gp condition = cc.newUInt32("condition");

    FuncNode* func = cc.addFunc(FuncSignature::build<int, int>());
    func->setArg(0, condition);

    Label L_NonZero = cc.newLabel();
    cc.test(condition, condition);
    cc.jnz(L_NonZero);

    {
      JumpAnnotation* annotation = cc.newJumpAnnotation();
      Label L_Target = cc.newLabel();
      annotation->addLabel(L_Target);

      x86::Gp target = cc.newUIntPtr("target");
      cc.lea(target, x86::ptr(L_Target));
      cc.jmp(target, annotation);
      cc.bind(L_Target);
      cc.mov(result, 1234);
      cc.ret(result);
    }

    {
      JumpAnnotation* annotation = cc.newJumpAnnotation();
      Label L_Target1 = cc.newLabel();
      Label L_Target2 = cc.newLabel();
      annotation->addLabel(L_Target1);
      annotation->addLabel(L_Target2);

      cc.bind(L_NonZero);
      x86::Gp target = cc.newUIntPtr("target");
      cc.lea(target, x86::ptr(L_Target1));
      cc.jmp(target, annotation);

      cc.bind(L_Target1);
      cc.mov(result, 4321);
      cc.ret(result);

      // Never executed.
      cc.bind(L_Target2);
      cc.mov(result, 0);
      cc.ret(result);
    }

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int);
    Func func = ptr_as_func<Func>(_func);

    int results[2] = { func(0), func(1) };
    int expected[2] = { 1234, 4321 };

    result.assignFormat("ret={%d, %d}", results[0], results[1]);
    expect.assignFormat("ret={%d, %d}", expected[0], expected[1]);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocBase
// =================================

class X86Test_AllocBase : public X86TestCase {
public:
  X86Test_AllocBase() : X86TestCase("AllocBase") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocBase());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    x86::Gp v0 = cc.newInt32("v0");
    x86::Gp v1 = cc.newInt32("v1");
    x86::Gp v2 = cc.newInt32("v2");
    x86::Gp v3 = cc.newInt32("v3");
    x86::Gp v4 = cc.newInt32("v4");

    cc.xor_(v0, v0);

    cc.mov(v1, 1);
    cc.mov(v2, 2);
    cc.mov(v3, 3);
    cc.mov(v4, 4);

    cc.add(v0, v1);
    cc.add(v0, v2);
    cc.add(v0, v3);
    cc.add(v0, v4);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 1 + 2 + 3 + 4;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocMany1
// ==================================

class X86Test_AllocMany1 : public X86TestCase {
public:
  X86Test_AllocMany1() : X86TestCase("AllocMany1") {}

  enum { kCount = 8 };

  static void add(TestApp& app) {
    app.add(new X86Test_AllocMany1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp a0 = cc.newIntPtr("a0");
    x86::Gp a1 = cc.newIntPtr("a1");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, int*, int*>());
    funcNode->setArg(0, a0);
    funcNode->setArg(1, a1);

    // Create some variables.
    x86::Gp t = cc.newInt32("t");
    x86::Gp x[kCount];

    uint32_t i;

    // Setup variables (use mov with reg/imm to se if register allocator works).
    for (i = 0; i < kCount; i++) x[i] = cc.newInt32("x%u", i);
    for (i = 0; i < kCount; i++) cc.mov(x[i], int(i + 1));

    // Make sum (addition).
    cc.xor_(t, t);
    for (i = 0; i < kCount; i++) cc.add(t, x[i]);

    // Store result to a given pointer in first argument.
    cc.mov(x86::dword_ptr(a0), t);

    // Clear t.
    cc.xor_(t, t);

    // Make sum (subtraction).
    for (i = 0; i < kCount; i++) cc.sub(t, x[i]);

    // Store result to a given pointer in second argument.
    cc.mov(x86::dword_ptr(a1), t);

    // End of function.
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(int*, int*);
    Func func = ptr_as_func<Func>(_func);

    int resultX = 0;
    int resultY = 0;

    int expectX =  36;
    int expectY = -36;

    func(&resultX, &resultY);

    result.assignFormat("ret={x=%d, y=%d}", resultX, resultY);
    expect.assignFormat("ret={x=%d, y=%d}", expectX, expectY);

    return resultX == expectX && resultY == expectY;
  }
};

// x86::Compiler - X86Test_AllocMany2
// ==================================

class X86Test_AllocMany2 : public X86TestCase {
public:
  X86Test_AllocMany2() : X86TestCase("AllocMany2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocMany2());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp a = cc.newIntPtr("a");
    x86::Gp v[32];

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, uint32_t*>());
    funcNode->setArg(0, a);

    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) v[i] = cc.newInt32("v%d", i);
    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) cc.xor_(v[i], v[i]);

    x86::Gp x = cc.newInt32("x");
    Label L = cc.newLabel();

    cc.mov(x, 32);
    cc.bind(L);
    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) cc.add(v[i], i);

    cc.dec(x);
    cc.jnz(L);
    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) cc.mov(x86::dword_ptr(a, int(i * 4)), v[i]);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(uint32_t*);
    Func func = ptr_as_func<Func>(_func);

    uint32_t i;
    uint32_t resultBuf[32] {};
    uint32_t expectBuf[32] {};

    for (i = 0; i < ASMJIT_ARRAY_SIZE(resultBuf); i++)
      expectBuf[i] = i * 32;
    func(resultBuf);

    for (i = 0; i < ASMJIT_ARRAY_SIZE(resultBuf); i++) {
      if (i != 0) {
        result.append(',');
        expect.append(',');
      }

      result.appendFormat("%u", resultBuf[i]);
      expect.appendFormat("%u", expectBuf[i]);
    }

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocInt8
// =================================

class X86Test_AllocInt8 : public X86TestCase {
public:
  X86Test_AllocInt8() : X86TestCase("AllocInt8") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocInt8());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp x = cc.newInt8("x");
    x86::Gp y = cc.newInt32("y");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int8_t>());
    funcNode->setArg(0, x);

    cc.movsx(y, x);

    cc.ret(y);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int8_t);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(int8_t(-13));
    int expectRet = -13;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocUnhandledArg
// =========================================

class X86Test_AllocUnhandledArg : public X86TestCase {
public:
  X86Test_AllocUnhandledArg() : X86TestCase("AllocUnhandledArg") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocUnhandledArg());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp x = cc.newInt32("x");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int, int>());
    funcNode->setArg(2, x);

    cc.ret(x);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(42, 155, 199);
    int expectRet = 199;

    result.assignFormat("ret={%d}", resultRet);
    expect.assignFormat("ret={%d}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocArgsIntPtr
// =======================================

class X86Test_AllocArgsIntPtr : public X86TestCase {
public:
  X86Test_AllocArgsIntPtr() : X86TestCase("AllocArgsIntPtr") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocArgsIntPtr());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*, void*, void*, void*, void*, void*, void*, void*>());
    x86::Gp var[8];

    for (uint32_t i = 0; i < 8; i++) {
      var[i] = cc.newIntPtr("var%u", i);
      funcNode->setArg(i, var[i]);
    }

    for (uint32_t i = 0; i < 8; i++) {
      cc.add(var[i], int(i + 1));
    }

    // Move some data into buffer provided by arguments so we can verify if it
    // really works without looking into assembler output.
    for (uint32_t i = 0; i < 8; i++) {
      cc.add(x86::byte_ptr(var[i]), int(i + 1));
    }

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*, void*, void*, void*, void*, void*, void*, void*);
    Func func = ptr_as_func<Func>(_func);

    uint8_t resultBuf[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t expectBuf[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

    func(resultBuf, resultBuf, resultBuf, resultBuf,
         resultBuf, resultBuf, resultBuf, resultBuf);

    result.assignFormat("buf={%d, %d, %d, %d, %d, %d, %d, %d, %d}",
      resultBuf[0], resultBuf[1], resultBuf[2], resultBuf[3],
      resultBuf[4], resultBuf[5], resultBuf[6], resultBuf[7],
      resultBuf[8]);
    expect.assignFormat("buf={%d, %d, %d, %d, %d, %d, %d, %d, %d}",
      expectBuf[0], expectBuf[1], expectBuf[2], expectBuf[3],
      expectBuf[4], expectBuf[5], expectBuf[6], expectBuf[7],
      expectBuf[8]);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocArgsFloat
// ======================================

class X86Test_AllocArgsFloat : public X86TestCase {
public:
  X86Test_AllocArgsFloat() : X86TestCase("AllocArgsFloat") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocArgsFloat());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, float, float, float, float, float, float, float, void*>());

    x86::Gp p = cc.newIntPtr("p");
    x86::Xmm xv[7];

    for (uint32_t i = 0; i < 7; i++) {
      xv[i] = cc.newXmmSs("xv%u", i);
      funcNode->setArg(i, xv[i]);
    }

    funcNode->setArg(7, p);

    cc.addss(xv[0], xv[1]);
    cc.addss(xv[0], xv[2]);
    cc.addss(xv[0], xv[3]);
    cc.addss(xv[0], xv[4]);
    cc.addss(xv[0], xv[5]);
    cc.addss(xv[0], xv[6]);

    cc.movss(x86::ptr(p), xv[0]);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(float, float, float, float, float, float, float, float*);
    Func func = ptr_as_func<Func>(_func);

    float resultRet = 0;
    float expectRet = 1.0f + 2.0f + 3.0f + 4.0f + 5.0f + 6.0f + 7.0f;

    func(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, &resultRet);

    result.assignFormat("ret={%g}", resultRet);
    expect.assignFormat("ret={%g}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocArgsDouble
// =======================================

class X86Test_AllocArgsDouble : public X86TestCase {
public:
  X86Test_AllocArgsDouble() : X86TestCase("AllocArgsDouble") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocArgsDouble());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, double, double, double, double, double, double, double, void*>());

    x86::Gp p = cc.newIntPtr("p");
    x86::Xmm xv[7];

    for (uint32_t i = 0; i < 7; i++) {
      xv[i] = cc.newXmmSd("xv%u", i);
      funcNode->setArg(i, xv[i]);
    }

    funcNode->setArg(7, p);

    cc.addsd(xv[0], xv[1]);
    cc.addsd(xv[0], xv[2]);
    cc.addsd(xv[0], xv[3]);
    cc.addsd(xv[0], xv[4]);
    cc.addsd(xv[0], xv[5]);
    cc.addsd(xv[0], xv[6]);

    cc.movsd(x86::ptr(p), xv[0]);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(double, double, double, double, double, double, double, double*);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = 0;
    double expectRet = 1.0 + 2.0 + 3.0 + 4.0 + 5.0 + 6.0 + 7.0;

    func(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, &resultRet);

    result.assignFormat("ret={%g}", resultRet);
    expect.assignFormat("ret={%g}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocArgsVec
// ====================================

#if ASMJIT_ARCH_X86
class X86Test_AllocArgsVec : public X86TestCase {
public:
  X86Test_AllocArgsVec() : X86TestCase("AllocArgsVec") {}

  static void add(TestApp& app) {
    // Not supported on Windows.
#ifndef _WIN32
    app.add(new X86Test_AllocArgsVec());
#else
    DebugUtils::unused(app);
#endif
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Xmm a = cc.newXmm("aXmm");
    x86::Xmm b = cc.newXmm("bXmm");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<x86::Xmm, x86::Xmm, x86::Xmm>());
    funcNode->setArg(0, a);
    funcNode->setArg(1, b);

    cc.paddb(a, b);
    cc.ret(a);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef __m128i (*Func)(__m128i, __m128i);
    Func func = ptr_as_func<Func>(_func);

    uint8_t aData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    uint8_t bData[16] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

    uint8_t rData[16] {};
    uint8_t eData[16] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };

    __m128i aVec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(aData));
    __m128i bVec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(bData));

    __m128i rVec = func(aVec, bVec);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(rData), rVec);

    result.appendHex(rData, 16);
    expect.appendHex(eData, 16);

    return result == expect;
  }
};
#endif // ASMJIT_ARCH_X86

// x86::Compiler - X86Test_AllocRetFloat1
// ======================================

class X86Test_AllocRetFloat1 : public X86TestCase {
public:
  X86Test_AllocRetFloat1() : X86TestCase("AllocRetFloat1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocRetFloat1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Xmm x = cc.newXmmSs("x");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<float, float>());
    funcNode->setArg(0, x);

    cc.ret(x);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef float (*Func)(float);
    Func func = ptr_as_func<Func>(_func);

    float resultRet = func(42.0f);
    float expectRet = 42.0f;

    result.assignFormat("ret={%g}", resultRet);
    expect.assignFormat("ret={%g}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocRetFloat2
// ======================================

class X86Test_AllocRetFloat2 : public X86TestCase {
public:
  X86Test_AllocRetFloat2() : X86TestCase("AllocRetFloat2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocRetFloat2());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Xmm x = cc.newXmmSs("x");
    x86::Xmm y = cc.newXmmSs("y");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<float, float, float>());
    funcNode->setArg(0, x);
    funcNode->setArg(1, y);

    cc.addss(x, y);
    cc.ret(x);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef float (*Func)(float, float);
    Func func = ptr_as_func<Func>(_func);

    float resultRet = func(1.0f, 2.0f);
    float expectRet = 1.0f + 2.0f;

    result.assignFormat("ret={%g}", resultRet);
    expect.assignFormat("ret={%g}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocRetDouble1
// =======================================

class X86Test_AllocRetDouble1 : public X86TestCase {
public:
  X86Test_AllocRetDouble1() : X86TestCase("AllocRetDouble1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocRetDouble1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Xmm x = cc.newXmmSd("x");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, double>());
    funcNode->setArg(0, x);

    cc.ret(x);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(double);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = func(42.0);
    double expectRet = 42.0;

    result.assignFormat("ret={%g}", resultRet);
    expect.assignFormat("ret={%g}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocRetDouble2
// =======================================

class X86Test_AllocRetDouble2 : public X86TestCase {
public:
  X86Test_AllocRetDouble2() : X86TestCase("AllocRetDouble2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_AllocRetDouble2());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Xmm x = cc.newXmmSd("x");
    x86::Xmm y = cc.newXmmSd("y");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, double, double>());
    funcNode->setArg(0, x);
    funcNode->setArg(1, y);

    cc.addsd(x, y);
    cc.ret(x);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = func(1.0, 2.0);
    double expectRet = 1.0 + 2.0;

    result.assignFormat("ret={%g}", resultRet);
    expect.assignFormat("ret={%g}", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocStack
// ==================================

class X86Test_AllocStack : public X86TestCase {
public:
  X86Test_AllocStack() : X86TestCase("AllocStack") {}

  enum { kSize = 256 };

  static void add(TestApp& app) {
    app.add(new X86Test_AllocStack());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    x86::Mem stack = cc.newStack(kSize, 1);
    stack.setSize(1);

    x86::Gp i = cc.newIntPtr("i");
    x86::Gp a = cc.newInt32("a");
    x86::Gp b = cc.newInt32("b");

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();

    // Fill stack by sequence [0, 1, 2, 3 ... 255].
    cc.xor_(i, i);

    x86::Mem stackWithIndex = stack.clone();
    stackWithIndex.setIndex(i, 0);

    cc.bind(L_1);
    cc.mov(stackWithIndex, i.r8());
    cc.inc(i);
    cc.cmp(i, 255);
    cc.jle(L_1);

    // Sum sequence in stack.
    cc.xor_(i, i);
    cc.xor_(a, a);

    cc.bind(L_2);
    cc.movzx(b, stackWithIndex);
    cc.add(a, b);
    cc.inc(i);
    cc.cmp(i, 255);
    cc.jle(L_2);

    cc.ret(a);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 32640;

    result.assignInt(resultRet);
    expect.assignInt(expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_Imul1
// =============================

class X86Test_Imul1 : public X86TestCase {
public:
  X86Test_Imul1() : X86TestCase("Imul1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_Imul1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp dstHi = cc.newIntPtr("dstHi");
    x86::Gp dstLo = cc.newIntPtr("dstLo");

    x86::Gp vHi = cc.newInt32("vHi");
    x86::Gp vLo = cc.newInt32("vLo");
    x86::Gp src = cc.newInt32("src");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, int*, int*, int, int>());
    funcNode->setArg(0, dstHi);
    funcNode->setArg(1, dstLo);
    funcNode->setArg(2, vLo);
    funcNode->setArg(3, src);

    cc.imul(vHi, vLo, src);
    cc.mov(x86::dword_ptr(dstHi), vHi);
    cc.mov(x86::dword_ptr(dstLo), vLo);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(int*, int*, int, int);
    Func func = ptr_as_func<Func>(_func);

    int v0 = 4;
    int v1 = 4;

    int resultHi = 0;
    int resultLo = 0;

    int expectHi = 0;
    int expectLo = v0 * v1;

    func(&resultHi, &resultLo, v0, v1);

    result.assignFormat("hi=%d, lo=%d", resultHi, resultLo);
    expect.assignFormat("hi=%d, lo=%d", expectHi, expectLo);

    return resultHi == expectHi && resultLo == expectLo;
  }
};

// x86::Compiler - X86Test_Imul2
// =============================

class X86Test_Imul2 : public X86TestCase {
public:
  X86Test_Imul2() : X86TestCase("Imul2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_Imul2());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp dst = cc.newIntPtr("dst");
    x86::Gp src = cc.newIntPtr("src");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, int*, const int*>());
    funcNode->setArg(0, dst);
    funcNode->setArg(1, src);

    for (unsigned int i = 0; i < 4; i++) {
      x86::Gp x  = cc.newInt32("x");
      x86::Gp y  = cc.newInt32("y");
      x86::Gp hi = cc.newInt32("hi");

      cc.mov(x, x86::dword_ptr(src, 0));
      cc.mov(y, x86::dword_ptr(src, 4));

      cc.imul(hi, x, y);
      cc.add(x86::dword_ptr(dst, 0), hi);
      cc.add(x86::dword_ptr(dst, 4), x);
    }

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(int*, const int*);
    Func func = ptr_as_func<Func>(_func);

    int src[2] = { 4, 9 };
    int resultRet[2] = { 0, 0 };
    int expectRet[2] = { 0, (4 * 9) * 4 };

    func(resultRet, src);

    result.assignFormat("ret={%d, %d}", resultRet[0], resultRet[1]);
    expect.assignFormat("ret={%d, %d}", expectRet[0], expectRet[1]);

    return resultRet[0] == expectRet[0] && resultRet[1] == expectRet[1];
  }
};

// x86::Compiler - X86Test_Idiv1
// =============================

class X86Test_Idiv1 : public X86TestCase {
public:
  X86Test_Idiv1() : X86TestCase("Idiv1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_Idiv1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp a = cc.newInt32("a");
    x86::Gp b = cc.newInt32("b");
    x86::Gp dummy = cc.newInt32("dummy");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int>());
    funcNode->setArg(0, a);
    funcNode->setArg(1, b);

    cc.xor_(dummy, dummy);
    cc.idiv(dummy, a, b);

    cc.ret(a);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int v0 = 2999;
    int v1 = 245;

    int resultRet = func(v0, v1);
    int expectRet = 2999 / 245;

    result.assignFormat("result=%d", resultRet);
    expect.assignFormat("result=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_Setz
// ============================

class X86Test_Setz : public X86TestCase {
public:
  X86Test_Setz() : X86TestCase("Setz") {}

  static void add(TestApp& app) {
    app.add(new X86Test_Setz());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp src0 = cc.newInt32("src0");
    x86::Gp src1 = cc.newInt32("src1");
    x86::Gp dst0 = cc.newIntPtr("dst0");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, int, int, char*>());
    funcNode->setArg(0, src0);
    funcNode->setArg(1, src1);
    funcNode->setArg(2, dst0);

    cc.cmp(src0, src1);
    cc.setz(x86::byte_ptr(dst0));

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(int, int, char*);
    Func func = ptr_as_func<Func>(_func);

    char resultBuf[4] {};
    char expectBuf[4] = { 1, 0, 0, 1 };

    func(0, 0, &resultBuf[0]); // We are expecting 1 (0 == 0).
    func(0, 1, &resultBuf[1]); // We are expecting 0 (0 != 1).
    func(1, 0, &resultBuf[2]); // We are expecting 0 (1 != 0).
    func(1, 1, &resultBuf[3]); // We are expecting 1 (1 == 1).

    result.assignFormat("out={%d, %d, %d, %d}", resultBuf[0], resultBuf[1], resultBuf[2], resultBuf[3]);
    expect.assignFormat("out={%d, %d, %d, %d}", expectBuf[0], expectBuf[1], expectBuf[2], expectBuf[3]);

    return resultBuf[0] == expectBuf[0] &&
           resultBuf[1] == expectBuf[1] &&
           resultBuf[2] == expectBuf[2] &&
           resultBuf[3] == expectBuf[3] ;
  }
};

// x86::Compiler - X86Test_ShlRor
// ==============================

class X86Test_ShlRor : public X86TestCase {
public:
  X86Test_ShlRor() : X86TestCase("ShlRor") {}

  static void add(TestApp& app) {
    app.add(new X86Test_ShlRor());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp dst = cc.newIntPtr("dst");
    x86::Gp var = cc.newInt32("var");
    x86::Gp vShlParam = cc.newInt32("vShlParam");
    x86::Gp vRorParam = cc.newInt32("vRorParam");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, int*, int, int, int>());
    funcNode->setArg(0, dst);
    funcNode->setArg(1, var);
    funcNode->setArg(2, vShlParam);
    funcNode->setArg(3, vRorParam);

    cc.shl(var, vShlParam);
    cc.ror(var, vRorParam);
    cc.mov(x86::dword_ptr(dst), var);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(int*, int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int v0 = 0x000000FF;

    int resultRet = 0;
    int expectRet = 0x0000FF00;

    func(&resultRet, v0, 16, 8);

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_GpbLo
// =============================

class X86Test_GpbLo1 : public X86TestCase {
public:
  X86Test_GpbLo1() : X86TestCase("GpbLo1") {}

  enum : uint32_t { kCount = 32 };

  static void add(TestApp& app) {
    app.add(new X86Test_GpbLo1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp rPtr = cc.newUIntPtr("rPtr");
    x86::Gp rSum = cc.newUInt32("rSum");
    x86::Gp x[kCount];

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<uint32_t, uint32_t*>());
    funcNode->setArg(0, rPtr);

    for (uint32_t i = 0; i < kCount; i++) {
      x[i] = cc.newUInt32("x%u", i);
    }

    // Init pseudo-regs with values from our array.
    for (uint32_t i = 0; i < kCount; i++) {
      cc.mov(x[i], x86::dword_ptr(rPtr, int(i * 4)));
    }

    for (uint32_t i = 2; i < kCount; i++) {
      // Add and truncate to 8 bit; no purpose, just mess with jit.
      cc.add  (x[i  ], x[i-1]);
      cc.movzx(x[i  ], x[i  ].r8());
      cc.movzx(x[i-2], x[i-1].r8());
      cc.movzx(x[i-1], x[i-2].r8());
    }

    // Sum up all computed values.
    cc.mov(rSum, 0);
    for (uint32_t i = 0; i < kCount; i++) {
      cc.add(rSum, x[i]);
    }

    // Return the sum.
    cc.ret(rSum);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(uint32_t*);
    Func func = ptr_as_func<Func>(_func);

    uint32_t i;
    uint32_t buf[kCount];
    uint32_t resultRet = 0;
    uint32_t expectRet = 0;

    for (i = 0; i < kCount; i++) {
      buf[i] = 1;
    }

    for (i = 2; i < kCount; i++) {
      buf[i  ]+= buf[i-1];
      buf[i  ] = buf[i  ] & 0xFF;
      buf[i-2] = buf[i-1] & 0xFF;
      buf[i-1] = buf[i-2] & 0xFF;
    }

    for (i = 0; i < kCount; i++) {
      expectRet += buf[i];
    }

    for (i = 0; i < kCount; i++) {
      buf[i] = 1;
    }
    resultRet = func(buf);

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_GpbLo2
// ==============================

class X86Test_GpbLo2 : public X86TestCase {
public:
  X86Test_GpbLo2() : X86TestCase("GpbLo2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_GpbLo2());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp v = cc.newUInt32("v");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<uint32_t, uint32_t>());
    funcNode->setArg(0, v);

    cc.mov(v.r8(), 0xFF);
    cc.ret(v);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t resultRet = func(0x12345678u);
    uint32_t expectRet = 0x123456FFu;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_RepMovsb
// ================================

class X86Test_RepMovsb : public X86TestCase {
public:
  X86Test_RepMovsb() : X86TestCase("RepMovsb") {}

  static void add(TestApp& app) {
    app.add(new X86Test_RepMovsb());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp dst = cc.newIntPtr("dst");
    x86::Gp src = cc.newIntPtr("src");
    x86::Gp cnt = cc.newIntPtr("cnt");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*, void*, size_t>());
    funcNode->setArg(0, dst);
    funcNode->setArg(1, src);
    funcNode->setArg(2, cnt);

    cc.rep(cnt).movs(x86::byte_ptr(dst), x86::byte_ptr(src));
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*, void*, size_t);
    Func func = ptr_as_func<Func>(_func);

    char dst[20] = { 0 };
    char src[20] = "Hello AsmJit!";
    func(dst, src, strlen(src) + 1);

    result.assignFormat("ret=\"%s\"", dst);
    expect.assignFormat("ret=\"%s\"", src);

    return result == expect;
  }
};

// x86::Compiler - X86Test_IfElse1
// ===============================

class X86Test_IfElse1 : public X86TestCase {
public:
  X86Test_IfElse1() : X86TestCase("IfElse1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_IfElse1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp v1 = cc.newInt32("v1");
    x86::Gp v2 = cc.newInt32("v2");

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int>());
    funcNode->setArg(0, v1);
    funcNode->setArg(1, v2);

    cc.cmp(v1, v2);
    cc.jg(L_1);

    cc.mov(v1, 1);
    cc.jmp(L_2);

    cc.bind(L_1);
    cc.mov(v1, 2);

    cc.bind(L_2);
    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    expect.appendFormat("ret={%d, %d}", 1, 2);

    return result == expect;
  }
};

// x86::Compiler - X86Test_IfElse2
// ===============================

class X86Test_IfElse2 : public X86TestCase {
public:
  X86Test_IfElse2() : X86TestCase("IfElse2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_IfElse2());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp v1 = cc.newInt32("v1");
    x86::Gp v2 = cc.newInt32("v2");

    Label L_1 = cc.newLabel();
    Label L_2 = cc.newLabel();
    Label L_3 = cc.newLabel();
    Label L_4 = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int>());
    funcNode->setArg(0, v1);
    funcNode->setArg(1, v2);

    cc.jmp(L_1);
    cc.bind(L_2);
    cc.jmp(L_4);
    cc.bind(L_1);

    cc.cmp(v1, v2);
    cc.jg(L_3);

    cc.mov(v1, 1);
    cc.jmp(L_2);

    cc.bind(L_3);
    cc.mov(v1, 2);
    cc.jmp(L_2);

    cc.bind(L_4);

    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    expect.appendFormat("ret={%d, %d}", 1, 2);

    return result == expect;
  }
};

// x86::Compiler - X86Test_IfElse3
// ===============================

class X86Test_IfElse3 : public X86TestCase {
public:
  X86Test_IfElse3() : X86TestCase("IfElse3") {}

  static void add(TestApp& app) {
    app.add(new X86Test_IfElse3());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp v1 = cc.newInt32("v1");
    x86::Gp v2 = cc.newInt32("v2");
    x86::Gp counter = cc.newInt32("counter");

    Label L_1 = cc.newLabel();
    Label L_Loop = cc.newLabel();
    Label L_Exit = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int>());
    funcNode->setArg(0, v1);
    funcNode->setArg(1, v2);

    cc.cmp(v1, v2);
    cc.jg(L_1);

    cc.mov(counter, 0);

    cc.bind(L_Loop);
    cc.mov(v1, counter);

    cc.inc(counter);
    cc.cmp(counter, 1);
    cc.jle(L_Loop);
    cc.jmp(L_Exit);

    cc.bind(L_1);
    cc.mov(v1, 2);

    cc.bind(L_Exit);
    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    expect.appendFormat("ret={%d, %d}", 1, 2);

    return result == expect;
  }
};

// x86::Compiler - X86Test_IfElse4
// ===============================

class X86Test_IfElse4 : public X86TestCase {
public:
  X86Test_IfElse4() : X86TestCase("IfElse4") {}

  static void add(TestApp& app) {
    app.add(new X86Test_IfElse4());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp v1 = cc.newInt32("v1");
    x86::Gp v2 = cc.newInt32("v2");
    x86::Gp counter = cc.newInt32("counter");

    Label L_1 = cc.newLabel();
    Label L_Loop1 = cc.newLabel();
    Label L_Loop2 = cc.newLabel();
    Label L_Exit = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int>());
    funcNode->setArg(0, v1);
    funcNode->setArg(1, v2);

    cc.mov(counter, 0);
    cc.cmp(v1, v2);
    cc.jg(L_1);

    cc.bind(L_Loop1);
    cc.mov(v1, counter);

    cc.inc(counter);
    cc.cmp(counter, 1);
    cc.jle(L_Loop1);
    cc.jmp(L_Exit);

    cc.bind(L_1);
    cc.bind(L_Loop2);
    cc.mov(v1, counter);
    cc.inc(counter);
    cc.cmp(counter, 2);
    cc.jle(L_Loop2);

    cc.bind(L_Exit);
    cc.ret(v1);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.appendFormat("ret={%d, %d}", a, b);
    expect.appendFormat("ret={%d, %d}", 1, 2);

    return result == expect;
  }
};

// x86::Compiler - X86Test_Memcpy
// ==============================

class X86Test_Memcpy : public X86TestCase {
public:
  X86Test_Memcpy() : X86TestCase("Memcpy") {}

  enum { kCount = 32 };

  static void add(TestApp& app) {
    app.add(new X86Test_Memcpy());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp dst = cc.newIntPtr("dst");
    x86::Gp src = cc.newIntPtr("src");
    x86::Gp cnt = cc.newUIntPtr("cnt");

    Label L_Loop = cc.newLabel();                   // Create base labels we use
    Label L_Exit = cc.newLabel();                   // in our function.

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, uint32_t*, const uint32_t*, size_t>());
    funcNode->setArg(0, dst);
    funcNode->setArg(1, src);
    funcNode->setArg(2, cnt);

    cc.test(cnt, cnt);                              // Exit if the size is zero.
    cc.jz(L_Exit);

    cc.bind(L_Loop);                                // Bind the loop label here.

    x86::Gp tmp = cc.newInt32("tmp");               // Copy a single dword (4 bytes).
    cc.mov(tmp, x86::dword_ptr(src));
    cc.mov(x86::dword_ptr(dst), tmp);

    cc.add(src, 4);                                 // Increment dst/src pointers.
    cc.add(dst, 4);

    cc.dec(cnt);                                    // Loop until cnt isn't zero.
    cc.jnz(L_Loop);

    cc.bind(L_Exit);                                // Bind the exit label here.
    cc.endFunc();                                   // End of function.
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(uint32_t*, const uint32_t*, size_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t i;

    uint32_t dstBuffer[kCount];
    uint32_t srcBuffer[kCount];

    for (i = 0; i < kCount; i++) {
      dstBuffer[i] = 0;
      srcBuffer[i] = i;
    }

    func(dstBuffer, srcBuffer, kCount);

    result.assign("buf={");
    expect.assign("buf={");

    for (i = 0; i < kCount; i++) {
      if (i != 0) {
        result.append(", ");
        expect.append(", ");
      }

      result.appendFormat("%u", unsigned(dstBuffer[i]));
      expect.appendFormat("%u", unsigned(srcBuffer[i]));
    }

    result.append("}");
    expect.append("}");

    return result == expect;
  }
};

// x86::Compiler - X86Test_ExtraBlock
// ==================================

class X86Test_ExtraBlock : public X86TestCase {
public:
  X86Test_ExtraBlock() : X86TestCase("ExtraBlock") {}

  static void add(TestApp& app) {
    app.add(new X86Test_ExtraBlock());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp cond = cc.newInt32("cond");
    x86::Gp ret = cc.newInt32("ret");
    x86::Gp a = cc.newInt32("a");
    x86::Gp b = cc.newInt32("b");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int, int>());
    funcNode->setArg(0, cond);
    funcNode->setArg(1, a);
    funcNode->setArg(2, b);

    Label L_Ret = cc.newLabel();
    Label L_Extra = cc.newLabel();

    cc.test(cond, cond);
    cc.jnz(L_Extra);

    cc.mov(ret, a);
    cc.add(ret, b);

    cc.bind(L_Ret);
    cc.ret(ret);

    // Emit code sequence at the end of the function.
    BaseNode* prevCursor = cc.setCursor(funcNode->endNode()->prev());
    cc.bind(L_Extra);
    cc.mov(ret, a);
    cc.sub(ret, b);
    cc.jmp(L_Ret);
    cc.setCursor(prevCursor);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int ret1 = func(0, 4, 5);
    int ret2 = func(1, 4, 5);

    int exp1 = 4 + 5;
    int exp2 = 4 - 5;

    result.assignFormat("ret={%d, %d}", ret1, ret2);
    expect.assignFormat("ret={%d, %d}", exp1, exp2);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AlphaBlend
// ==================================

class X86Test_AlphaBlend : public X86TestCase {
public:
  X86Test_AlphaBlend() : X86TestCase("AlphaBlend") {}

  enum { kCount = 17 };

  static void add(TestApp& app) {
    app.add(new X86Test_AlphaBlend());
  }

  static uint32_t blendSrcOver(uint32_t d, uint32_t s) {
    uint32_t saInv = ~s >> 24;

    uint32_t d_20 = (d     ) & 0x00FF00FF;
    uint32_t d_31 = (d >> 8) & 0x00FF00FF;

    d_20 *= saInv;
    d_31 *= saInv;

    d_20 = ((d_20 + ((d_20 >> 8) & 0x00FF00FFu) + 0x00800080u) & 0xFF00FF00u) >> 8;
    d_31 = ((d_31 + ((d_31 >> 8) & 0x00FF00FFu) + 0x00800080u) & 0xFF00FF00u);

    return d_20 + d_31 + s;
  }

  virtual void compile(x86::Compiler& cc) {
    asmtest::generateSseAlphaBlend(cc, true);
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*, const void*, size_t);
    Func func = ptr_as_func<Func>(_func);

    static const uint32_t dstConstData[] = { 0x00000000, 0x10101010, 0x20100804, 0x30200003, 0x40204040, 0x5000004D, 0x60302E2C, 0x706F6E6D, 0x807F4F2F, 0x90349001, 0xA0010203, 0xB03204AB, 0xC023AFBD, 0xD0D0D0C0, 0xE0AABBCC, 0xFFFFFFFF, 0xF8F4F2F1 };
    static const uint32_t srcConstData[] = { 0xE0E0E0E0, 0xA0008080, 0x341F1E1A, 0xFEFEFEFE, 0x80302010, 0x49490A0B, 0x998F7798, 0x00000000, 0x01010101, 0xA0264733, 0xBAB0B1B9, 0xFF000000, 0xDAB0A0C1, 0xE0BACFDA, 0x99887766, 0xFFFFFF80, 0xEE0A5FEC };

    uint32_t _dstBuffer[kCount + 3];
    uint32_t _srcBuffer[kCount + 3];

    // Has to be aligned.
    uint32_t* dstBuffer = (uint32_t*)Support::alignUp<intptr_t>((intptr_t)_dstBuffer, 16);
    uint32_t* srcBuffer = (uint32_t*)Support::alignUp<intptr_t>((intptr_t)_srcBuffer, 16);

    memcpy(dstBuffer, dstConstData, sizeof(dstConstData));
    memcpy(srcBuffer, srcConstData, sizeof(srcConstData));

    uint32_t i;
    uint32_t expBuffer[kCount];

    for (i = 0; i < kCount; i++) {
      expBuffer[i] = blendSrcOver(dstBuffer[i], srcBuffer[i]);
    }

    func(dstBuffer, srcBuffer, kCount);

    result.assign("buf={");
    expect.assign("buf={");

    for (i = 0; i < kCount; i++) {
      if (i != 0) {
        result.append(", ");
        expect.append(", ");
      }

      result.appendFormat("%08X", unsigned(dstBuffer[i]));
      expect.appendFormat("%08X", unsigned(expBuffer[i]));
    }

    result.append("}");
    expect.append("}");

    return result == expect;
  }
};

// x86::Compiler - X86Test_AVX512_KK
// =================================

class X86Test_AVX512_KK : public X86TestCase {
public:
  X86Test_AVX512_KK() : X86TestCase("AVX512_KK") {}

  static void add(TestApp& app) {
    const CpuInfo& cpuInfo = CpuInfo::host();

    if (cpuInfo.features().x86().hasAVX512_F()) {
      app.add(new X86Test_AVX512_KK());
    }
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<uint32_t, const void*, const void*, uint32_t>());

    x86::Gp a = cc.newIntPtr("a");
    x86::Gp b = cc.newIntPtr("b");
    x86::Gp pred = cc.newInt32("pred");
    x86::Gp result = cc.newInt32("result");

    x86::Vec va = cc.newZmm("va");
    x86::Vec vb = cc.newZmm("vb");
    x86::KReg kIn = cc.newKd("k_in");
    x86::KReg kOut = cc.newKd("k_out");

    funcNode->setArg(0, a);
    funcNode->setArg(1, b);
    funcNode->setArg(2, pred);

    cc.vmovdqu32(va, x86::ptr(a));
    cc.vmovdqu32(vb, x86::ptr(b));
    cc.kmovd(kIn, pred);
    cc.k(kIn).vpcmpeqd(kOut, va, vb);
    cc.kmovd(result, kOut);
    cc.ret(result);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(const void*, const void*, uint32_t prevK);
    Func func = ptr_as_func<Func>(_func);

    static const uint32_t srcA[16] = { 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 };
    static const uint32_t srcB[16] = { 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1 };

    uint32_t ret = func(srcA, srcB, 0xF0F0);

    result.assignFormat("0x%08X", ret);
    expect.assignFormat("0x%08X", 0xA040u);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AVX512_TernLog
// ======================================

class X86Test_AVX512_TernLog : public X86TestCase {
public:
  X86Test_AVX512_TernLog() : X86TestCase("AVX512_TernLog") {}

  static void add(TestApp& app) {
    const CpuInfo& cpuInfo = CpuInfo::host();

    if (cpuInfo.features().x86().hasAVX512_F()) {
      app.add(new X86Test_AVX512_TernLog());
    }
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*>());

    x86::Gp out = cc.newIntPtr("outPtr");
    x86::Vec vec = cc.newZmm("vec");

    funcNode->setArg(0, out);

    cc.vpternlogd(vec, vec, vec, 0xFFu);
    cc.vmovdqu8(x86::ptr(out), vec);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*);
    Func func = ptr_as_func<Func>(_func);

    uint32_t out[16];
    func(out);

    result.assign("{");
    expect.assign("{");

    for (uint32_t i = 0; i < 16; i++) {
      if (i) {
        result.append(", ");
        expect.append(", ");
      }
      result.appendFormat("0x%08X", out[i]);
      expect.appendFormat("0x%08X", 0xFFFFFFFFu);
    }

    result.append("}");
    expect.append("}");

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncArgInt8
// ===================================

class X86Test_FuncArgInt8 : public X86TestCase {
public:
  X86Test_FuncArgInt8() : X86TestCase("FuncArgInt8") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncArgInt8());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp v0 = cc.newUInt32("v0");
    x86::Gp v1 = cc.newUInt32("v1");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<unsigned, uint8_t, uint8_t, uint32_t>());
    funcNode->setArg(0, v0);
    funcNode->setArg(1, v1);

    cc.add(v0, v1);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(uint8_t, uint8_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t arg = uint32_t(uintptr_t(_func) & 0xFFFFFFFF);

    unsigned resultRet = func(uint8_t(arg & 0xFF), uint8_t(arg & 0xFF), arg);
    unsigned expectRet = (arg & 0xFF) * 2;

    result.assignFormat("ret=%u", resultRet);
    expect.assignFormat("ret=%u", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallBase1
// =====================================

class X86Test_FuncCallBase1 : public X86TestCase {
public:
  X86Test_FuncCallBase1() : X86TestCase("FuncCallBase1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallBase1());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp v0 = cc.newInt32("v0");
    x86::Gp v1 = cc.newInt32("v1");
    x86::Gp v2 = cc.newInt32("v2");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int, int>());
    funcNode->setArg(0, v0);
    funcNode->setArg(1, v1);
    funcNode->setArg(2, v2);

    // Just do something.
    cc.shl(v0, 1);
    cc.shl(v1, 1);
    cc.shl(v2, 1);

    // Call a function.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, imm((void*)calledFunc), FuncSignature::build<int, int, int, int>());
    invokeNode->setArg(0, v2);
    invokeNode->setArg(1, v1);
    invokeNode->setArg(2, v0);
    invokeNode->setRet(0, v0);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(3, 2, 1);
    int expectRet = 36;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }

  static int calledFunc(int a, int b, int c) { return (a + b) * c; }
};

// x86::Compiler - X86Test_FuncCallBase2
// =====================================

class X86Test_FuncCallBase2 : public X86TestCase {
public:
  X86Test_FuncCallBase2() : X86TestCase("FuncCallBase2") {}

  enum { kSize = 256 };

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallBase2());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    const int kTokenSize = 32;

    x86::Mem s1 = cc.newStack(kTokenSize, 32);
    x86::Mem s2 = cc.newStack(kTokenSize, 32);

    x86::Gp p1 = cc.newIntPtr("p1");
    x86::Gp p2 = cc.newIntPtr("p2");

    x86::Gp ret = cc.newInt32("ret");
    Label L_Exit = cc.newLabel();

    static const char token[kTokenSize] = "-+:|abcdefghijklmnopqrstuvwxyz|";
    InvokeNode* invokeNode;

    cc.lea(p1, s1);
    cc.lea(p2, s2);

    // Try to corrupt the stack if wrongly allocated.
    cc.invoke(&invokeNode, imm((void*)memcpy), FuncSignature::build<void*, void*, void*, size_t>());
    invokeNode->setArg(0, p1);
    invokeNode->setArg(1, imm(token));
    invokeNode->setArg(2, imm(kTokenSize));
    invokeNode->setRet(0, p1);

    cc.invoke(&invokeNode, imm((void*)memcpy), FuncSignature::build<void*, void*, void*, size_t>());
    invokeNode->setArg(0, p2);
    invokeNode->setArg(1, imm(token));
    invokeNode->setArg(2, imm(kTokenSize));
    invokeNode->setRet(0, p2);

    cc.invoke(&invokeNode, imm((void*)memcmp), FuncSignature::build<int, void*, void*, size_t>());
    invokeNode->setArg(0, p1);
    invokeNode->setArg(1, p2);
    invokeNode->setArg(2, imm(kTokenSize));
    invokeNode->setRet(0, ret);

    // This should be 0 on success, however, if both `p1` and `p2` were
    // allocated in the same address this check will still pass.
    cc.cmp(ret, 0);
    cc.jnz(L_Exit);

    // Checks whether `p1` and `p2` are different (must be).
    cc.xor_(ret, ret);
    cc.cmp(p1, p2);
    cc.setz(ret.r8());

    cc.bind(L_Exit);
    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 0; // Must be zero, stack addresses must be different.

    result.assignInt(resultRet);
    expect.assignInt(expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallStd
// ===================================

class X86Test_FuncCallStd : public X86TestCase {
public:
  X86Test_FuncCallStd() : X86TestCase("FuncCallStd") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallStd());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp x = cc.newInt32("x");
    x86::Gp y = cc.newInt32("y");
    x86::Gp z = cc.newInt32("z");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int, int>());
    funcNode->setArg(0, x);
    funcNode->setArg(1, y);
    funcNode->setArg(2, z);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)calledFunc),
      FuncSignature::build<int, int, int, int>(CallConvId::kStdCall));
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setArg(2, z);
    invokeNode->setRet(0, x);

    cc.ret(x);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(1, 42, 3);
    int expectRet = calledFunc(1, 42, 3);

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }

  // STDCALL function that is called inside the generated one.
  static int ASMJIT_STDCALL calledFunc(int a, int b, int c) noexcept {
    return (a + b) * c;
  }
};

// x86::Compiler - X86Test_FuncCallFast
// ====================================

class X86Test_FuncCallFast : public X86TestCase {
public:
  X86Test_FuncCallFast() : X86TestCase("FuncCallFast") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallFast());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp var = cc.newInt32("var");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int>());
    funcNode->setArg(0, var);

    InvokeNode* invokeNode;

    cc.invoke(&invokeNode, imm((void*)calledFunc), FuncSignature::build<int, int>(CallConvId::kFastCall));
    invokeNode->setArg(0, var);
    invokeNode->setRet(0, var);

    cc.invoke(&invokeNode, imm((void*)calledFunc), FuncSignature::build<int, int>(CallConvId::kFastCall));
    invokeNode->setArg(0, var);
    invokeNode->setRet(0, var);

    cc.ret(var);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(9);
    int expectRet = (9 * 9) * (9 * 9);

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }

  // FASTCALL function that is called inside the generated one.
  static int ASMJIT_FASTCALL calledFunc(int a) noexcept {
    return a * a;
  }
};

// x86::Compiler - X86Test_FuncCallSIMD
// ====================================

#if ASMJIT_ARCH_X86
class X86Test_FuncCallSIMD : public X86TestCase {
public:
  bool _useVectorCall;

  X86Test_FuncCallSIMD(bool useVectorCall)
    : X86TestCase(),
      _useVectorCall(useVectorCall) {
    _name.assignFormat("FuncCallSIMD {%s}", _useVectorCall ? "__vectorcall" : "__cdecl");
  }

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallSIMD(false));
#ifdef _MSC_VER
    app.add(new X86Test_FuncCallSIMD(true));
#endif
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<void, void*, const void*, const void*>());

    x86::Gp resultPtr = cc.newIntPtr("resultPtr");
    x86::Gp aPtr = cc.newIntPtr("aPtr");
    x86::Gp bPtr = cc.newIntPtr("bPtr");
    x86::Gp pFn = cc.newIntPtr("pFn");

    x86::Xmm aXmm = cc.newXmm("aXmm");
    x86::Xmm bXmm = cc.newXmm("bXmm");

    funcNode->setArg(0, resultPtr);
    funcNode->setArg(1, aPtr);
    funcNode->setArg(2, bPtr);

    CallConvId ccId = CallConvId::kCDecl;
    Imm pFnImm = imm((void*)calledFunc_cdecl);

#ifdef _MSC_VER
    if (_useVectorCall) {
      ccId = CallConvId::kVectorCall;
      pFnImm = imm((void*)calledFunc_vcall);
    }
#endif

    cc.mov(pFn, pFnImm);
    cc.movdqu(aXmm, x86::ptr(aPtr));
    cc.movdqu(bXmm, x86::ptr(bPtr));

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, pFn, FuncSignature::build<x86::Xmm, x86::Xmm, x86::Xmm>(ccId));

    invokeNode->setArg(0, aXmm);
    invokeNode->setArg(1, bXmm);
    invokeNode->setRet(0, aXmm);

    cc.movdqu(x86::ptr(resultPtr), aXmm);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*, const void*, const void*);
    Func func = ptr_as_func<Func>(_func);

    uint8_t aData[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    uint8_t bData[16] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

    uint8_t rData[16] {};
    uint8_t eData[16] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };

    func(rData, aData, bData);

    result.appendHex(rData, 16);
    expect.appendHex(eData, 16);

    return result == expect;
  }

  static __m128i calledFunc_cdecl(__m128i a, __m128i b) {
    return _mm_add_epi8(a, b);
  }

#ifdef _MSC_VER
  static __m128i __vectorcall calledFunc_vcall(__m128i a, __m128i b) {
    return _mm_add_epi8(a, b);
  }
#endif
};
#endif // ASMJIT_ARCH_X86

// x86::Compiler - X86Test_FuncCallLight
// =====================================

class X86Test_FuncCallLight : public X86TestCase {
public:
  X86Test_FuncCallLight() : X86TestCase("FuncCallLight") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallLight());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncSignature f1Sig = FuncSignature::build<void, const void*, const void*, const void*, const void*, void*>();
    FuncSignature f2Sig = FuncSignature::build<x86::Xmm, x86::Xmm, x86::Xmm>(CallConvId::kLightCall2);

    FuncNode* f1Node = cc.newFunc(f1Sig);
    FuncNode* f2Node = cc.newFunc(f2Sig);

    {
      x86::Gp aPtr = cc.newIntPtr("aPtr");
      x86::Gp bPtr = cc.newIntPtr("bPtr");
      x86::Gp cPtr = cc.newIntPtr("cPtr");
      x86::Gp dPtr = cc.newIntPtr("dPtr");
      x86::Gp pOut = cc.newIntPtr("pOut");

      x86::Xmm aXmm = cc.newXmm("aXmm");
      x86::Xmm bXmm = cc.newXmm("bXmm");
      x86::Xmm cXmm = cc.newXmm("cXmm");
      x86::Xmm dXmm = cc.newXmm("dXmm");

      cc.addFunc(f1Node);
      f1Node->setArg(0, aPtr);
      f1Node->setArg(1, bPtr);
      f1Node->setArg(2, cPtr);
      f1Node->setArg(3, dPtr);
      f1Node->setArg(4, pOut);

      cc.movups(aXmm, x86::ptr(aPtr));
      cc.movups(bXmm, x86::ptr(bPtr));
      cc.movups(cXmm, x86::ptr(cPtr));
      cc.movups(dXmm, x86::ptr(dPtr));

      x86::Xmm xXmm = cc.newXmm("xXmm");
      x86::Xmm yXmm = cc.newXmm("yXmm");

      InvokeNode* invokeNode;

      cc.invoke(&invokeNode, f2Node->label(), f2Sig);
      invokeNode->setArg(0, aXmm);
      invokeNode->setArg(1, bXmm);
      invokeNode->setRet(0, xXmm);

      cc.invoke(&invokeNode, f2Node->label(), f2Sig);
      invokeNode->setArg(0, cXmm);
      invokeNode->setArg(1, dXmm);
      invokeNode->setRet(0, yXmm);

      cc.pmullw(xXmm, yXmm);
      cc.movups(x86::ptr(pOut), xXmm);

      cc.endFunc();
    }

    {
      x86::Xmm aXmm = cc.newXmm("aXmm");
      x86::Xmm bXmm = cc.newXmm("bXmm");

      cc.addFunc(f2Node);
      f2Node->setArg(0, aXmm);
      f2Node->setArg(1, bXmm);
      cc.paddw(aXmm, bXmm);
      cc.ret(aXmm);
      cc.endFunc();
    }
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(const void*, const void*, const void*, const void*, void*);

    Func func = ptr_as_func<Func>(_func);

    int16_t a[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    int16_t b[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
    int16_t c[8] = { 1, 3, 9, 7, 5, 4, 2, 1 };
    int16_t d[8] = { 2, 0,-6,-4,-2,-1, 1, 2 };

    int16_t o[8] {};
    int oExp = 7 * 3;

    func(a, b, c, d, o);

    result.assignFormat("ret={%02X %02X %02X %02X %02X %02X %02X %02X}", o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7]);
    expect.assignFormat("ret={%02X %02X %02X %02X %02X %02X %02X %02X}", oExp, oExp, oExp, oExp, oExp, oExp, oExp, oExp);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallManyArgs
// ========================================

class X86Test_FuncCallManyArgs : public X86TestCase {
public:
  X86Test_FuncCallManyArgs() : X86TestCase("FuncCallManyArgs") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallManyArgs());
  }

  static int calledFunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    // Prepare.
    x86::Gp va = cc.newInt32("va");
    x86::Gp vb = cc.newInt32("vb");
    x86::Gp vc = cc.newInt32("vc");
    x86::Gp vd = cc.newInt32("vd");
    x86::Gp ve = cc.newInt32("ve");
    x86::Gp vf = cc.newInt32("vf");
    x86::Gp vg = cc.newInt32("vg");
    x86::Gp vh = cc.newInt32("vh");
    x86::Gp vi = cc.newInt32("vi");
    x86::Gp vj = cc.newInt32("vj");

    cc.mov(va, 0x03);
    cc.mov(vb, 0x12);
    cc.mov(vc, 0xA0);
    cc.mov(vd, 0x0B);
    cc.mov(ve, 0x2F);
    cc.mov(vf, 0x02);
    cc.mov(vg, 0x0C);
    cc.mov(vh, 0x12);
    cc.mov(vi, 0x18);
    cc.mov(vj, 0x1E);

    // Function call.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)calledFunc),
      FuncSignature::build<int, int, int, int, int, int, int, int, int, int, int>());
    invokeNode->setArg(0, va);
    invokeNode->setArg(1, vb);
    invokeNode->setArg(2, vc);
    invokeNode->setArg(3, vd);
    invokeNode->setArg(4, ve);
    invokeNode->setArg(5, vf);
    invokeNode->setArg(6, vg);
    invokeNode->setArg(7, vh);
    invokeNode->setArg(8, vi);
    invokeNode->setArg(9, vj);
    invokeNode->setRet(0, va);

    cc.ret(va);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = calledFunc(0x03, 0x12, 0xA0, 0x0B, 0x2F, 0x02, 0x0C, 0x12, 0x18, 0x1E);

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallDuplicateArgs
// =============================================

class X86Test_FuncCallDuplicateArgs : public X86TestCase {
public:
  X86Test_FuncCallDuplicateArgs() : X86TestCase("FuncCallDuplicateArgs") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallDuplicateArgs());
  }

  static int calledFunc(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    // Prepare.
    x86::Gp a = cc.newInt32("a");
    cc.mov(a, 3);

    // Call function.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)calledFunc),
      FuncSignature::build<int, int, int, int, int, int, int, int, int, int, int>());
    invokeNode->setArg(0, a);
    invokeNode->setArg(1, a);
    invokeNode->setArg(2, a);
    invokeNode->setArg(3, a);
    invokeNode->setArg(4, a);
    invokeNode->setArg(5, a);
    invokeNode->setArg(6, a);
    invokeNode->setArg(7, a);
    invokeNode->setArg(8, a);
    invokeNode->setArg(9, a);
    invokeNode->setRet(0, a);

    cc.ret(a);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = calledFunc(3, 3, 3, 3, 3, 3, 3, 3, 3, 3);

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallImmArgs
// =======================================

class X86Test_FuncCallImmArgs : public X86TestCase {
public:
  X86Test_FuncCallImmArgs() : X86TestCase("FuncCallImmArgs") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallImmArgs());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    // Prepare.
    x86::Gp rv = cc.newInt32("rv");

    // Call function.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)X86Test_FuncCallManyArgs::calledFunc),
      FuncSignature::build<int, int, int, int, int, int, int, int, int, int, int>());

    invokeNode->setArg(0, imm(0x03));
    invokeNode->setArg(1, imm(0x12));
    invokeNode->setArg(2, imm(0xA0));
    invokeNode->setArg(3, imm(0x0B));
    invokeNode->setArg(4, imm(0x2F));
    invokeNode->setArg(5, imm(0x02));
    invokeNode->setArg(6, imm(0x0C));
    invokeNode->setArg(7, imm(0x12));
    invokeNode->setArg(8, imm(0x18));
    invokeNode->setArg(9, imm(0x1E));
    invokeNode->setRet(0, rv);

    cc.ret(rv);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = X86Test_FuncCallManyArgs::calledFunc(0x03, 0x12, 0xA0, 0x0B, 0x2F, 0x02, 0x0C, 0x12, 0x18, 0x1E);

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallPtrArgs
// =======================================

class X86Test_FuncCallPtrArgs : public X86TestCase {
public:
  X86Test_FuncCallPtrArgs() : X86TestCase("FuncCallPtrArgs") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallPtrArgs());
  }

  static int calledFunc(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j) {
    return int((intptr_t)a) +
           int((intptr_t)b) +
           int((intptr_t)c) +
           int((intptr_t)d) +
           int((intptr_t)e) +
           int((intptr_t)f) +
           int((intptr_t)g) +
           int((intptr_t)h) +
           int((intptr_t)i) +
           int((intptr_t)j) ;
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    // Prepare.
    x86::Gp rv = cc.newInt32("rv");

    // Call function.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)calledFunc),
      FuncSignature::build<int, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*>());

    invokeNode->setArg(0, imm(0x01));
    invokeNode->setArg(1, imm(0x02));
    invokeNode->setArg(2, imm(0x03));
    invokeNode->setArg(3, imm(0x04));
    invokeNode->setArg(4, imm(0x05));
    invokeNode->setArg(5, imm(0x06));
    invokeNode->setArg(6, imm(0x07));
    invokeNode->setArg(7, imm(0x08));
    invokeNode->setArg(8, imm(0x09));
    invokeNode->setArg(9, imm(0x0A));
    invokeNode->setRet(0, rv);

    cc.ret(rv);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 55;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallRefArgs
// =======================================

class X86Test_FuncCallRefArgs : public X86TestCase {
public:
  X86Test_FuncCallRefArgs() : X86TestCase("FuncCallRefArgs") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallRefArgs());
  }

  static int calledFunc(int& a, int& b, int& c, int& d) {
    a += a;
    b += b;
    c += c;
    d += d;
    return a + b + c + d;
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int&, int&, int&, int&>());

    // Prepare.
    x86::Gp arg1 = cc.newInt32();
    x86::Gp arg2 = cc.newInt32();
    x86::Gp arg3 = cc.newInt32();
    x86::Gp arg4 = cc.newInt32();
    x86::Gp rv = cc.newInt32("rv");

    funcNode->setArg(0, arg1);
    funcNode->setArg(1, arg2);
    funcNode->setArg(2, arg3);
    funcNode->setArg(3, arg4);

    // Call function.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)calledFunc),
      FuncSignature::build<int, int&, int&, int&, int&>());

    invokeNode->setArg(0, arg1);
    invokeNode->setArg(1, arg2);
    invokeNode->setArg(2, arg3);
    invokeNode->setArg(3, arg4);
    invokeNode->setRet(0, rv);

    cc.ret(rv);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int&, int&, int&, int&);
    Func func = ptr_as_func<Func>(_func);

    int inputs[4] = { 1, 2, 3, 4 };
    int outputs[4] = { 2, 4, 6, 8 };
    int resultRet = func(inputs[0], inputs[1], inputs[2], inputs[3]);
    int expectRet = 20;

    result.assignFormat("ret={%08X %08X %08X %08X %08X}", resultRet, inputs[0], inputs[1], inputs[2], inputs[3]);
    expect.assignFormat("ret={%08X %08X %08X %08X %08X}", expectRet, outputs[0], outputs[1], outputs[2], outputs[3]);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallFloatAsXmmRet
// =============================================

class X86Test_FuncCallFloatAsXmmRet : public X86TestCase {
public:
  X86Test_FuncCallFloatAsXmmRet() : X86TestCase("FuncCallFloatAsXmmRet") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallFloatAsXmmRet());
  }

  static float calledFunc(float a, float b) {
    return a * b;
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<float, float, float>());

    x86::Xmm a = cc.newXmmSs("a");
    x86::Xmm b = cc.newXmmSs("b");
    x86::Xmm ret = cc.newXmmSs("ret");

    funcNode->setArg(0, a);
    funcNode->setArg(1, b);

    // Call function.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, imm((void*)calledFunc), FuncSignature::build<float, float, float>());
    invokeNode->setArg(0, a);
    invokeNode->setArg(1, b);
    invokeNode->setRet(0, ret);

    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef float (*Func)(float, float);
    Func func = ptr_as_func<Func>(_func);

    float resultRet = func(15.5f, 2.0f);
    float expectRet = calledFunc(15.5f, 2.0f);

    result.assignFormat("ret=%g", resultRet);
    expect.assignFormat("ret=%g", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallDoubleAsXmmRet
// ==============================================

class X86Test_FuncCallDoubleAsXmmRet : public X86TestCase {
public:
  X86Test_FuncCallDoubleAsXmmRet() : X86TestCase("FuncCallDoubleAsXmmRet") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallDoubleAsXmmRet());
  }

  static double calledFunc(double a, double b) {
    return a * b;
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, double, double>());

    x86::Xmm a = cc.newXmmSd("a");
    x86::Xmm b = cc.newXmmSd("b");
    x86::Xmm ret = cc.newXmmSd("ret");

    funcNode->setArg(0, a);
    funcNode->setArg(1, b);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, imm((void*)calledFunc), FuncSignature::build<double, double, double>());
    invokeNode->setArg(0, a);
    invokeNode->setArg(1, b);
    invokeNode->setRet(0, ret);

    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = func(15.5, 2.0);
    double expectRet = calledFunc(15.5, 2.0);

    result.assignFormat("ret=%g", resultRet);
    expect.assignFormat("ret=%g", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallConditional
// ===========================================

class X86Test_FuncCallConditional : public X86TestCase {
public:
  X86Test_FuncCallConditional() : X86TestCase("FuncCallConditional") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallConditional());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp x = cc.newInt32("x");
    x86::Gp y = cc.newInt32("y");
    x86::Gp op = cc.newInt32("op");

    InvokeNode* invokeNode;
    x86::Gp result;

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int, int>());
    funcNode->setArg(0, x);
    funcNode->setArg(1, y);
    funcNode->setArg(2, op);

    Label opAdd = cc.newLabel();
    Label opMul = cc.newLabel();

    cc.cmp(op, 0);
    cc.jz(opAdd);
    cc.cmp(op, 1);
    cc.jz(opMul);

    result = cc.newInt32("result_0");
    cc.mov(result, 0);
    cc.ret(result);

    cc.bind(opAdd);
    result = cc.newInt32("result_1");

    cc.invoke(&invokeNode, (uint64_t)calledFuncAdd, FuncSignature::build<int, int, int>());
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setRet(0, result);
    cc.ret(result);

    cc.bind(opMul);
    result = cc.newInt32("result_2");

    cc.invoke(&invokeNode, (uint64_t)calledFuncMul, FuncSignature::build<int, int, int>());
    invokeNode->setArg(0, x);
    invokeNode->setArg(1, y);
    invokeNode->setRet(0, result);

    cc.ret(result);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int arg1 = 4;
    int arg2 = 8;

    int resultAdd = func(arg1, arg2, 0);
    int expectAdd = calledFuncAdd(arg1, arg2);

    int resultMul = func(arg1, arg2, 1);
    int expectMul = calledFuncMul(arg1, arg2);

    result.assignFormat("ret={add=%d, mul=%d}", resultAdd, resultMul);
    expect.assignFormat("ret={add=%d, mul=%d}", expectAdd, expectMul);

    return (resultAdd == expectAdd) && (resultMul == expectMul);
  }

  static int calledFuncAdd(int x, int y) { return x + y; }
  static int calledFuncMul(int x, int y) { return x * y; }
};

// x86::Compiler - X86Test_FuncCallMultiple
// ========================================

class X86Test_FuncCallMultiple : public X86TestCase {
public:
  X86Test_FuncCallMultiple() : X86TestCase("FuncCallMultiple") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMultiple());
  }

  static int ASMJIT_FASTCALL calledFunc(int* pInt, int index) {
    return pInt[index];
  }

  virtual void compile(x86::Compiler& cc) {
    unsigned int i;

    x86::Gp buf = cc.newIntPtr("buf");
    x86::Gp acc0 = cc.newInt32("acc0");
    x86::Gp acc1 = cc.newInt32("acc1");

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int*>());
    funcNode->setArg(0, buf);

    cc.mov(acc0, 0);
    cc.mov(acc1, 0);

    for (i = 0; i < 4; i++) {
      x86::Gp ret = cc.newInt32("ret");
      x86::Gp ptr = cc.newIntPtr("ptr");
      x86::Gp idx = cc.newInt32("idx");
      InvokeNode* invokeNode;

      cc.mov(ptr, buf);
      cc.mov(idx, int(i));

      cc.invoke(&invokeNode, (uint64_t)calledFunc, FuncSignature::build<int, int*, int>(CallConvId::kFastCall));
      invokeNode->setArg(0, ptr);
      invokeNode->setArg(1, idx);
      invokeNode->setRet(0, ret);

      cc.add(acc0, ret);

      cc.mov(ptr, buf);
      cc.mov(idx, int(i));

      cc.invoke(&invokeNode, (uint64_t)calledFunc, FuncSignature::build<int, int*, int>(CallConvId::kFastCall));
      invokeNode->setArg(0, ptr);
      invokeNode->setArg(1, idx);
      invokeNode->setRet(0, ret);

      cc.sub(acc1, ret);
    }

    cc.add(acc0, acc1);
    cc.ret(acc0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int*);
    Func func = ptr_as_func<Func>(_func);

    int buffer[4] = { 127, 87, 23, 17 };

    int resultRet = func(buffer);
    int expectRet = 0;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallRecursive
// =========================================

class X86Test_FuncCallRecursive : public X86TestCase {
public:
  X86Test_FuncCallRecursive() : X86TestCase("FuncCallRecursive") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallRecursive());
  }

  virtual void compile(x86::Compiler& cc) {
    x86::Gp val = cc.newInt32("val");
    Label skip = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int>());
    funcNode->setArg(0, val);

    cc.cmp(val, 1);
    cc.jle(skip);

    x86::Gp tmp = cc.newInt32("tmp");
    cc.mov(tmp, val);
    cc.dec(tmp);

    InvokeNode* invokeNode;

    cc.invoke(&invokeNode, funcNode->label(), FuncSignature::build<int, int>());
    invokeNode->setArg(0, tmp);
    invokeNode->setRet(0, tmp);
    cc.mul(cc.newInt32(), val, tmp);

    cc.bind(skip);
    cc.ret(val);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(5);
    int expectRet = 1 * 2 * 3 * 4 * 5;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallVarArg1
// =======================================

class X86Test_FuncCallVarArg1 : public X86TestCase {
public:
  X86Test_FuncCallVarArg1() : X86TestCase("FuncCallVarArg1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallVarArg1());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int, int, int>());

    x86::Gp a0 = cc.newInt32("a0");
    x86::Gp a1 = cc.newInt32("a1");
    x86::Gp a2 = cc.newInt32("a2");
    x86::Gp a3 = cc.newInt32("a3");

    funcNode->setArg(0, a0);
    funcNode->setArg(1, a1);
    funcNode->setArg(2, a2);
    funcNode->setArg(3, a3);

    // We call `int func(size_t, ...)`
    //   - The `vaIndex` must be 1 (first argument after size_t).
    //   - The full signature of varargs (int, int, int, int) must follow.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)calledFunc),
      FuncSignature::build<int, size_t, int, int, int, int>(CallConvId::kCDecl, 1));
    invokeNode->setArg(0, imm(4));
    invokeNode->setArg(1, a0);
    invokeNode->setArg(2, a1);
    invokeNode->setArg(3, a2);
    invokeNode->setArg(4, a3);
    invokeNode->setRet(0, a0);

    cc.ret(a0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(1, 2, 3, 4);
    int expectRet = 1 + 2 + 3 + 4;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }

  static int calledFunc(size_t n, ...) {
    int sum = 0;
    va_list ap;
    va_start(ap, n);
    for (size_t i = 0; i < n; i++) {
      int arg = va_arg(ap, int);
      sum += arg;
    }
    va_end(ap);
    return sum;
  }
};

// x86::Compiler - X86Test_FuncCallVarArg2
// =======================================

class X86Test_FuncCallVarArg2 : public X86TestCase {
public:
  X86Test_FuncCallVarArg2() : X86TestCase("FuncCallVarArg2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallVarArg2());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, double, double, double, double>());

    x86::Xmm a0 = cc.newXmmSd("a0");
    x86::Xmm a1 = cc.newXmmSd("a1");
    x86::Xmm a2 = cc.newXmmSd("a2");
    x86::Xmm a3 = cc.newXmmSd("a3");

    funcNode->setArg(0, a0);
    funcNode->setArg(1, a1);
    funcNode->setArg(2, a2);
    funcNode->setArg(3, a3);

    // We call `double func(size_t, ...)`
    //   - The `vaIndex` must be 1 (first argument after size_t).
    //   - The full signature of varargs (double, double, double, double) must follow.
    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)calledFunc),
      FuncSignature::build<double, size_t, double, double, double, double>(CallConvId::kCDecl, 1));
    invokeNode->setArg(0, imm(4));
    invokeNode->setArg(1, a0);
    invokeNode->setArg(2, a1);
    invokeNode->setArg(3, a2);
    invokeNode->setArg(4, a3);
    invokeNode->setRet(0, a0);

    cc.ret(a0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(double, double, double, double);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = func(1.0, 2.0, 3.0, 4.0);
    double expectRet = 1.0 + 2.0 + 3.0 + 4.0;

    result.assignFormat("ret=%f", resultRet);
    expect.assignFormat("ret=%f", expectRet);

    return result == expect;
  }

  static double calledFunc(size_t n, ...) {
    double sum = 0;
    va_list ap;
    va_start(ap, n);
    for (size_t i = 0; i < n; i++) {
      double arg = va_arg(ap, double);
      sum += arg;
    }
    va_end(ap);
    return sum;
  }
};

// x86::Compiler - X86Test_FuncCallInt64Arg
// ========================================

class X86Test_FuncCallInt64Arg : public X86TestCase {
public:
  X86Test_FuncCallInt64Arg() : X86TestCase("FuncCallInt64Arg") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallInt64Arg());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<uint64_t, uint64_t>());

    if (cc.is64Bit()) {
      x86::Gp reg = cc.newUInt64();
      funcNode->setArg(0, reg);
      cc.add(reg, 1);
      cc.ret(reg);
    }
    else {
      x86::Gp hi = cc.newUInt32("hi");
      x86::Gp lo = cc.newUInt32("lo");

      funcNode->setArg(0, 0, lo);
      funcNode->setArg(0, 1, hi);

      cc.add(lo, 1);
      cc.adc(hi, 0);
      cc.ret(lo, hi);
    }

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint64_t (*Func)(uint64_t);
    Func func = ptr_as_func<Func>(_func);

    uint64_t resultRet = func(uint64_t(0xFFFFFFFF));
    uint64_t expectRet = 0x100000000;

    result.assignFormat("ret=%llu", (unsigned long long)resultRet);
    expect.assignFormat("ret=%llu", (unsigned long long)expectRet);

    return result == expect;
  }

  static double calledFunc(size_t n, ...) {
    double sum = 0;
    va_list ap;
    va_start(ap, n);
    for (size_t i = 0; i < n; i++) {
      double arg = va_arg(ap, double);
      sum += arg;
    }
    va_end(ap);
    return sum;
  }
};

// x86::Compiler - X86Test_FuncCallMisc1
// =====================================

class X86Test_FuncCallMisc1 : public X86TestCase {
public:
  X86Test_FuncCallMisc1() : X86TestCase("FuncCallMisc1") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMisc1());
  }

  static void dummy(int, int) {}

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int>());

    x86::Gp a = cc.newInt32("a");
    x86::Gp b = cc.newInt32("b");
    x86::Gp r = cc.newInt32("r");

    funcNode->setArg(0, a);
    funcNode->setArg(1, b);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)dummy),
      FuncSignature::build<void, int, int>());
    invokeNode->setArg(0, a);
    invokeNode->setArg(1, b);

    cc.lea(r, x86::ptr(a, b));
    cc.ret(r);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(44, 199);
    int expectRet = 243;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_FuncCallMisc2
// =====================================

class X86Test_FuncCallMisc2 : public X86TestCase {
public:
  X86Test_FuncCallMisc2() : X86TestCase("FuncCallMisc2") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMisc2());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, const double*>());

    x86::Gp p = cc.newIntPtr("p");
    x86::Xmm arg = cc.newXmmSd("arg");
    x86::Xmm ret = cc.newXmmSd("ret");

    funcNode->setArg(0, p);
    cc.movsd(arg, x86::ptr(p));

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)op),
      FuncSignature::build<double, double>());
    invokeNode->setArg(0, arg);
    invokeNode->setRet(0, ret);

    cc.ret(ret);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(const double*);
    Func func = ptr_as_func<Func>(_func);

    double arg = 2;

    double resultRet = func(&arg);
    double expectRet = op(arg);

    result.assignFormat("ret=%g", resultRet);
    expect.assignFormat("ret=%g", expectRet);

    return result == expect;
  }

  static double op(double a) { return a * a; }
};

// x86::Compiler - X86Test_FuncCallMisc3
// =====================================

class X86Test_FuncCallMisc3 : public X86TestCase {
public:
  X86Test_FuncCallMisc3() : X86TestCase("FuncCallMisc3") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMisc3());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<double, const double*>());

    x86::Gp p = cc.newIntPtr("p");
    x86::Xmm arg = cc.newXmmSd("arg");
    x86::Xmm ret = cc.newXmmSd("ret");

    funcNode->setArg(0, p);
    cc.movsd(arg, x86::ptr(p));

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode,
      imm((void*)op),
      FuncSignature::build<double, double>());
    invokeNode->setArg(0, arg);
    invokeNode->setRet(0, ret);

    cc.xorps(arg, arg);
    cc.subsd(arg, ret);

    cc.ret(arg);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(const double*);
    Func func = ptr_as_func<Func>(_func);

    double arg = 2;

    double resultRet = func(&arg);
    double expectRet = -op(arg);

    result.assignFormat("ret=%g", resultRet);
    expect.assignFormat("ret=%g", expectRet);

    return result == expect;
  }

  static double op(double a) { return a * a; }
};

// x86::Compiler - X86Test_FuncCallMisc4
// =====================================

class X86Test_FuncCallMisc4 : public X86TestCase {
public:
  X86Test_FuncCallMisc4() : X86TestCase("FuncCallMisc4") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMisc4());
  }

  virtual void compile(x86::Compiler& cc) {
    InvokeNode* invokeNode;

    FuncSignature funcSignature;
    funcSignature.setCallConvId(CallConvId::kCDecl);
    funcSignature.setRet(TypeId::kFloat64);
    cc.addFunc(funcSignature);

    FuncSignature invokeSignature;
    invokeSignature.setCallConvId(CallConvId::kCDecl);
    invokeSignature.setRet(TypeId::kFloat64);

    cc.invoke(&invokeNode, imm((void*)calledFunc), invokeSignature);
    x86::Xmm ret = cc.newXmmSd("ret");
    invokeNode->setRet(0, ret);
    cc.ret(ret);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef double (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    double resultRet = func();
    double expectRet = 3.14;

    result.assignFormat("ret=%g", resultRet);
    expect.assignFormat("ret=%g", expectRet);

    return result == expect;
  }

  static double calledFunc() { return 3.14; }
};

// x86::Compiler - X86Test_FuncCallMisc5
// =====================================

// The register allocator should clobber the register used by the `call` itself.
class X86Test_FuncCallMisc5 : public X86TestCase {
public:
  X86Test_FuncCallMisc5() : X86TestCase("FuncCallMisc5") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMisc5());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    x86::Gp pFn = cc.newIntPtr("pFn");
    x86::Gp vars[16];

    uint32_t i, regCount = cc.arch() == Arch::kX86 ? 8 : 16;
    ASMJIT_ASSERT(regCount <= ASMJIT_ARRAY_SIZE(vars));

    cc.mov(pFn, imm((void*)calledFunc));

    for (i = 0; i < regCount; i++) {
      if (i == x86::Gp::kIdBp || i == x86::Gp::kIdSp)
        continue;

      vars[i] = cc.newInt32("%%%u", unsigned(i));
      cc.mov(vars[i], 1);
    }

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, pFn, FuncSignature::build<void>());

    for (i = 1; i < regCount; i++)
      if (vars[i].isValid())
        cc.add(vars[0], vars[i]);
    cc.ret(vars[0]);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = sizeof(void*) == 4 ? 6 : 14;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }

  static void calledFunc() {}
};

// x86::Compiler - X86Test_FuncCallMisc6
// =====================================

class X86Test_FuncCallMisc6 : public X86TestCase {
public:
  X86Test_FuncCallMisc6() : X86TestCase("FuncCallMisc6") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMisc6());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<uint32_t, uint32_t>());

    constexpr uint32_t kCount = 16;

    x86::Gp v[kCount];
    x86::Gp argVal = cc.newUInt32("argVal");
    x86::Gp retVal = cc.newUInt32("retVal");
    uint32_t i;

    funcNode->setArg(0, argVal);
    cc.add(argVal, 1);

    for (i = 0; i < kCount; i++)
      v[i] = cc.newUInt32("v%u", i);

    InvokeNode* invokeNode;
    cc.invoke(&invokeNode, imm((void*)calledFunc), FuncSignature::build<uint32_t, uint32_t>());
    invokeNode->setArg(0, argVal);
    invokeNode->setRet(0, retVal);

    for (i = 0; i < kCount; i++)
      cc.mov(v[i], i + 1);

    for (i = 0; i < kCount; i++)
      cc.add(argVal, v[i]);

    cc.add(retVal, argVal);
    cc.ret(retVal);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(uint32_t x);
    Func func = ptr_as_func<Func>(_func);

    uint32_t resultRet = func(111);
    uint32_t expectRet = 111 + 112 + 2 + (1 + 16) * 8;

    result.assignFormat("ret=%u", resultRet);
    expect.assignFormat("ret=%u", expectRet);

    return result == expect;
  }

  static uint32_t calledFunc(uint32_t x) { return x + 1; }
};

// x86::Compiler - X86Test_FuncCallAVXClobber
// ==========================================

class X86Test_FuncCallAVXClobber : public X86TestCase {
public:
  X86Test_FuncCallAVXClobber() : X86TestCase("FuncCallAVXClobber") {}

  static void add(TestApp& app) {
    const CpuInfo& cpuInfo = CpuInfo::host();

    if (cpuInfo.features().x86().hasAVX2() && sizeof(void*) == 8) {
      app.add(new X86Test_FuncCallAVXClobber());
    }
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* mainFunc = cc.addFunc(FuncSignature::build<void, void*, const void*, const void*>());
    mainFunc->frame().setAvxEnabled();
    mainFunc->frame().setAvxCleanup();

    // We need a Windows calling convention to test this properly also on a non-Windows machine.
    FuncNode* helperFunc = cc.newFunc(FuncSignature::build<void, void*, const void*>(CallConvId::kX64Windows));
    helperFunc->frame().setAvxEnabled();
    helperFunc->frame().setAvxCleanup();

    {
      size_t i;

      x86::Gp dPtr = cc.newIntPtr("dPtr");
      x86::Gp aPtr = cc.newIntPtr("aPtr");
      x86::Gp bPtr = cc.newIntPtr("bPtr");
      x86::Gp tPtr = cc.newIntPtr("tPtr");
      x86::Ymm acc[8];
      x86::Mem stack = cc.newStack(32, 1, "stack");

      mainFunc->setArg(0, dPtr);
      mainFunc->setArg(1, aPtr);
      mainFunc->setArg(2, bPtr);

      cc.lea(tPtr, stack);
      for (i = 0; i < 8; i++) {
        acc[i] = cc.newYmm("acc%zu", i);
        cc.vmovdqu(acc[i], x86::ptr(aPtr));
      }

      InvokeNode* invokeNode;
      cc.invoke(&invokeNode,
        helperFunc->label(),
        FuncSignature::build<void, void*, const void*>(CallConvId::kX64Windows));
      invokeNode->setArg(0, tPtr);
      invokeNode->setArg(1, bPtr);

      for (i = 1; i < 8; i++) {
        cc.vpaddd(acc[0], acc[0], acc[i]);
      }

      cc.vpaddd(acc[0], acc[0], x86::ptr(tPtr));
      cc.vmovdqu(x86::ptr(dPtr), acc[0]);

      cc.endFunc();
    }

    {
      cc.addFunc(helperFunc);

      x86::Gp dPtr = cc.newIntPtr("dPtr");
      x86::Gp aPtr = cc.newIntPtr("aPtr");

      helperFunc->setArg(0, dPtr);
      helperFunc->setArg(1, aPtr);

      x86::Gp tmp = cc.newIntPtr("tmp");
      x86::Ymm acc = cc.newYmm("acc");

      cc.mov(tmp, 1);
      cc.vmovd(acc.xmm(), tmp);
      cc.vpbroadcastd(acc, acc.xmm());
      cc.vpaddd(acc, acc, x86::ptr(aPtr));
      cc.vmovdqu(x86::ptr(dPtr), acc);

      cc.endFunc();
    }
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef void (*Func)(void*, const void*, const void*);
    Func func = ptr_as_func<Func>(_func);

    size_t i;

    static const uint32_t aData[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    static const uint32_t bData[8] = { 6, 3, 5, 9, 1, 8, 7, 2 };

    uint32_t resultData[8] {};
    uint32_t expectData[8] {};

    for (i = 0; i < 8; i++)
      expectData[i] = aData[i] * 8 + bData[i] + 1;

    func(resultData, aData, bData);

    result.assign("{");
    expect.assign("{");

    for (i = 0; i < 8; i++) {
      result.appendFormat("%u", resultData[i]);
      expect.appendFormat("%u", expectData[i]);

      if (i != 7) result.append(", ");
      if (i != 7) expect.append(", ");
    }

    result.append("}");
    expect.append("}");

    return result == expect;
  }
};

// x86::Compiler - X86Test_VecToScalar
// ===================================

class X86Test_VecToScalar : public X86TestCase {
public:
  static constexpr uint32_t kVecCount = 64;

  X86Test_VecToScalar() : X86TestCase("VecToScalar") {}

  static void add(TestApp& app) {
    app.add(new X86Test_VecToScalar());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* func = cc.addFunc(FuncSignature::build<uint32_t, uint32_t>());

    x86::Gp x = cc.newInt32("x");
    x86::Gp t = cc.newInt32("t");
    x86::Xmm v[kVecCount];

    func->setArg(0, x);

    for (size_t i = 0; i < kVecCount; i++) {
      v[i] = cc.newXmm("v%d", i);
      if (i != 0)
        cc.add(x, 1);
      cc.movd(v[i], x);
    }

    cc.xor_(x, x);

    for (size_t i = 0; i < kVecCount; i++) {
      cc.movd(t, v[i]);
      cc.add(x, t);
    }

    cc.ret(x);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef uint32_t (*Func)(uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t resultRet = func(1);
    uint32_t expectRet = 2080; // 1 + 2 + 3 + ... + 64

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_MiscLocalConstPool
// ==========================================

class X86Test_MiscLocalConstPool : public X86TestCase {
public:
  X86Test_MiscLocalConstPool() : X86TestCase("MiscLocalConstPool") {}

  static void add(TestApp& app) {
    app.add(new X86Test_MiscLocalConstPool());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    x86::Gp v0 = cc.newInt32("v0");
    x86::Gp v1 = cc.newInt32("v1");

    x86::Mem c0 = cc.newInt32Const(ConstPoolScope::kLocal, 200);
    x86::Mem c1 = cc.newInt32Const(ConstPoolScope::kLocal, 33);

    cc.mov(v0, c0);
    cc.mov(v1, c1);
    cc.add(v0, v1);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 233;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_MiscGlobalConstPool
// ===========================================

class X86Test_MiscGlobalConstPool : public X86TestCase {
public:
  X86Test_MiscGlobalConstPool() : X86TestCase("MiscGlobalConstPool") {}

  static void add(TestApp& app) {
    app.add(new X86Test_MiscGlobalConstPool());
  }

  virtual void compile(x86::Compiler& cc) {
    cc.addFunc(FuncSignature::build<int>());

    x86::Gp v0 = cc.newInt32("v0");
    x86::Gp v1 = cc.newInt32("v1");

    x86::Mem c0 = cc.newInt32Const(ConstPoolScope::kGlobal, 200);
    x86::Mem c1 = cc.newInt32Const(ConstPoolScope::kGlobal, 33);

    cc.mov(v0, c0);
    cc.mov(v1, c1);
    cc.add(v0, v1);

    cc.ret(v0);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(void);
    Func func = ptr_as_func<Func>(_func);

    int resultRet = func();
    int expectRet = 233;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_MiscMultiRet
// ====================================

struct X86Test_MiscMultiRet : public X86TestCase {
  X86Test_MiscMultiRet() : X86TestCase("MiscMultiRet") {}

  static void add(TestApp& app) {
    app.add(new X86Test_MiscMultiRet());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, int, int>());

    x86::Gp op = cc.newInt32("op");
    x86::Gp a = cc.newInt32("a");
    x86::Gp b = cc.newInt32("b");

    Label L_Zero = cc.newLabel();
    Label L_Add = cc.newLabel();
    Label L_Sub = cc.newLabel();
    Label L_Mul = cc.newLabel();
    Label L_Div = cc.newLabel();

    funcNode->setArg(0, op);
    funcNode->setArg(1, a);
    funcNode->setArg(2, b);

    cc.cmp(op, 0);
    cc.jz(L_Add);

    cc.cmp(op, 1);
    cc.jz(L_Sub);

    cc.cmp(op, 2);
    cc.jz(L_Mul);

    cc.cmp(op, 3);
    cc.jz(L_Div);

    cc.bind(L_Zero);
    cc.xor_(a, a);
    cc.ret(a);

    cc.bind(L_Add);
    cc.add(a, b);
    cc.ret(a);

    cc.bind(L_Sub);
    cc.sub(a, b);
    cc.ret(a);

    cc.bind(L_Mul);
    cc.imul(a, b);
    cc.ret(a);

    cc.bind(L_Div);
    cc.cmp(b, 0);
    cc.jz(L_Zero);

    x86::Gp zero = cc.newInt32("zero");
    cc.xor_(zero, zero);
    cc.idiv(zero, a, b);
    cc.ret(a);

    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int, int);

    Func func = ptr_as_func<Func>(_func);

    int a = 44;
    int b = 3;

    int r0 = func(0, a, b);
    int r1 = func(1, a, b);
    int r2 = func(2, a, b);
    int r3 = func(3, a, b);
    int e0 = a + b;
    int e1 = a - b;
    int e2 = a * b;
    int e3 = a / b;

    result.assignFormat("ret={%d %d %d %d}", r0, r1, r2, r3);
    expect.assignFormat("ret={%d %d %d %d}", e0, e1, e2, e3);

    return result == expect;
  }
};

// x86::Compiler - X86Test_MiscMultiFunc
// =====================================

class X86Test_MiscMultiFunc : public X86TestCase {
public:
  X86Test_MiscMultiFunc() : X86TestCase("MiscMultiFunc") {}

  static void add(TestApp& app) {
    app.add(new X86Test_MiscMultiFunc());
  }

  virtual void compile(x86::Compiler& cc) {
    FuncNode* f1Node = cc.newFunc(FuncSignature::build<int, int, int>());
    FuncNode* f2Node = cc.newFunc(FuncSignature::build<int, int, int>());

    {
      x86::Gp a = cc.newInt32("a");
      x86::Gp b = cc.newInt32("b");

      cc.addFunc(f1Node);
      f1Node->setArg(0, a);
      f1Node->setArg(1, b);

      InvokeNode* invokeNode;
      cc.invoke(&invokeNode, f2Node->label(), FuncSignature::build<int, int, int>());
      invokeNode->setArg(0, a);
      invokeNode->setArg(1, b);
      invokeNode->setRet(0, a);

      cc.ret(a);
      cc.endFunc();
    }

    {
      x86::Gp a = cc.newInt32("a");
      x86::Gp b = cc.newInt32("b");

      cc.addFunc(f2Node);
      f2Node->setArg(0, a);
      f2Node->setArg(1, b);

      cc.add(a, b);
      cc.ret(a);
      cc.endFunc();
    }
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (*Func)(int, int);

    Func func = ptr_as_func<Func>(_func);

    int resultRet = func(56, 22);
    int expectRet = 56 + 22;

    result.assignFormat("ret=%d", resultRet);
    expect.assignFormat("ret=%d", expectRet);

    return result == expect;
  }
};

// x86::Compiler - X86Test_MiscUnfollow
// ====================================

// Global (I didn't find a better way to test this).
static jmp_buf globalJmpBuf;

class X86Test_MiscUnfollow : public X86TestCase {
public:
  X86Test_MiscUnfollow() : X86TestCase("MiscUnfollow") {}

  static void add(TestApp& app) {
    app.add(new X86Test_MiscUnfollow());
  }

  virtual void compile(x86::Compiler& cc) {
    // NOTE: Fastcall calling convention is the most appropriate here as all arguments are passed via registers and
    // there won't be any stack misalignment in the `handler()`. This was failing on MacOS when targeting 32-bit mode.
    x86::Gp a = cc.newInt32("a");
    x86::Gp b = cc.newIntPtr("b");
    Label tramp = cc.newLabel();

    FuncNode* funcNode = cc.addFunc(FuncSignature::build<int, int, void*>(CallConvId::kFastCall));
    funcNode->setArg(0, a);
    funcNode->setArg(1, b);

    cc.cmp(a, 0);
    cc.jz(tramp);
    cc.ret(a);
    cc.bind(tramp);
    cc.unfollow().jmp(b);
    cc.endFunc();
  }

  virtual bool run(void* _func, String& result, String& expect) {
    typedef int (ASMJIT_FASTCALL *Func)(int, void*);

    Func func = ptr_as_func<Func>(_func);

    int resultRet = 0;
    int expectRet = 1;

    if (!setjmp(globalJmpBuf))
      resultRet = func(0, (void*)handler);
    else
      resultRet = 1;

    result.assignFormat("ret={%d}", resultRet);
    expect.assignFormat("ret={%d}", expectRet);

    return result == expect;
  }

  static void ASMJIT_FASTCALL handler() { longjmp(globalJmpBuf, 1); }
};

// x86::Compiler - Tests
// =====================

void compiler_add_x86_tests(TestApp& app) {
  // Base tests.
  app.addT<X86Test_NoCode>();
  app.addT<X86Test_NoAlign>();
  app.addT<X86Test_IndirectBranchProtection>();
  app.addT<X86Test_AlignBase>();

  // Jump tests.
  app.addT<X86Test_JumpMerge>();
  app.addT<X86Test_JumpCross>();
  app.addT<X86Test_JumpMany>();
  app.addT<X86Test_JumpUnreachable1>();
  app.addT<X86Test_JumpUnreachable2>();
  app.addT<X86Test_JumpTable1>();
  app.addT<X86Test_JumpTable2>();
  app.addT<X86Test_JumpTable3>();
  app.addT<X86Test_JumpTable4>();

  // Alloc and instruction tests.
  app.addT<X86Test_AllocBase>();
  app.addT<X86Test_AllocMany1>();
  app.addT<X86Test_AllocMany2>();
  app.addT<X86Test_AllocInt8>();
  app.addT<X86Test_AllocUnhandledArg>();
  app.addT<X86Test_AllocArgsIntPtr>();
  app.addT<X86Test_AllocArgsFloat>();
  app.addT<X86Test_AllocArgsDouble>();
#if ASMJIT_ARCH_X86
  app.addT<X86Test_AllocArgsVec>();
#endif
  app.addT<X86Test_AllocRetFloat1>();
  app.addT<X86Test_AllocRetFloat2>();
  app.addT<X86Test_AllocRetDouble1>();
  app.addT<X86Test_AllocRetDouble2>();
  app.addT<X86Test_AllocStack>();
  app.addT<X86Test_Imul1>();
  app.addT<X86Test_Imul2>();
  app.addT<X86Test_Idiv1>();
  app.addT<X86Test_Setz>();
  app.addT<X86Test_ShlRor>();
  app.addT<X86Test_GpbLo1>();
  app.addT<X86Test_GpbLo2>();
  app.addT<X86Test_RepMovsb>();
  app.addT<X86Test_IfElse1>();
  app.addT<X86Test_IfElse2>();
  app.addT<X86Test_IfElse3>();
  app.addT<X86Test_IfElse4>();
  app.addT<X86Test_Memcpy>();
  app.addT<X86Test_ExtraBlock>();
  app.addT<X86Test_AlphaBlend>();
  app.addT<X86Test_AVX512_KK>();
  app.addT<X86Test_AVX512_TernLog>();

  // Function arguments handling tests.
  app.addT<X86Test_FuncArgInt8>();

  // Function call tests.
  app.addT<X86Test_FuncCallBase1>();
  app.addT<X86Test_FuncCallBase2>();
  app.addT<X86Test_FuncCallStd>();
  app.addT<X86Test_FuncCallFast>();
#if ASMJIT_ARCH_X86
  app.addT<X86Test_FuncCallSIMD>();
#endif
  app.addT<X86Test_FuncCallLight>();
  app.addT<X86Test_FuncCallManyArgs>();
  app.addT<X86Test_FuncCallDuplicateArgs>();
  app.addT<X86Test_FuncCallImmArgs>();
  app.addT<X86Test_FuncCallPtrArgs>();
  app.addT<X86Test_FuncCallRefArgs>();
  app.addT<X86Test_FuncCallFloatAsXmmRet>();
  app.addT<X86Test_FuncCallDoubleAsXmmRet>();
  app.addT<X86Test_FuncCallConditional>();
  app.addT<X86Test_FuncCallMultiple>();
  app.addT<X86Test_FuncCallRecursive>();
  app.addT<X86Test_FuncCallVarArg1>();
  app.addT<X86Test_FuncCallVarArg2>();
  app.addT<X86Test_FuncCallInt64Arg>();
  app.addT<X86Test_FuncCallMisc1>();
  app.addT<X86Test_FuncCallMisc2>();
  app.addT<X86Test_FuncCallMisc3>();
  app.addT<X86Test_FuncCallMisc4>();
  app.addT<X86Test_FuncCallMisc5>();
  app.addT<X86Test_FuncCallMisc6>();
  app.addT<X86Test_FuncCallAVXClobber>();

  // Miscellaneous tests.
  app.addT<X86Test_VecToScalar>();
  app.addT<X86Test_MiscLocalConstPool>();
  app.addT<X86Test_MiscGlobalConstPool>();
  app.addT<X86Test_MiscMultiRet>();
  app.addT<X86Test_MiscMultiFunc>();
  app.addT<X86Test_MiscUnfollow>();
}

#endif // !ASMJIT_NO_X86 && !ASMJIT_NO_COMPILER
