// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
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

  void compile(BaseCompiler& cc) override {
    compile(static_cast<x86::Compiler&>(cc));
  }

  virtual void compile(x86::Compiler& cc) = 0;
};

// x86::Compiler - X86Test_AlignBase
// =================================

class X86Test_AlignBase : public X86TestCase {
public:
  X86Test_AlignBase(uint32_t arg_count, uint32_t alignment, bool preserve_fp)
    : _arg_count(arg_count),
      _alignment(alignment),
      _preserve_fp(preserve_fp) {
    _name.assign_format("AlignBase {NumArgs=%u Alignment=%u PreserveFP=%c}", arg_count, alignment, preserve_fp ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i <= 16; i++) {
      for (uint32_t a = 16; a <= 32; a += 16) {
        app.add(new X86Test_AlignBase(i, a, true));
        app.add(new X86Test_AlignBase(i, a, false));
      }
    }
  }

  void compile(x86::Compiler& cc) override {
    uint32_t arg_count = _arg_count;

    FuncSignature signature(CallConvId::kCDecl);
    signature.set_ret_t<int>();
    for (uint32_t i = 0; i < arg_count; i++) {
      signature.add_arg_t<int>();
    }

    FuncNode* func_node = cc.add_func(signature);
    if (_preserve_fp)
      func_node->frame().set_preserved_fp();

    x86::Gp gp_var = cc.new_gp_ptr("gp_var");
    x86::Gp gp_sum;
    x86::Mem stack = cc.new_stack(_alignment, _alignment);

    // Do a sum of arguments to verify a possible relocation when misaligned.
    if (arg_count) {
      for (uint32_t i = 0; i < arg_count; i++) {
        x86::Gp gp_arg = cc.new_gp32("gp_arg%u", i);
        func_node->set_arg(i, gp_arg);

        if (i == 0)
          gp_sum = gp_arg;
        else
          cc.add(gp_sum, gp_arg);
      }
    }

    // Check alignment of the stack (has to be 16).
    cc.lea(gp_var, stack);
    cc.and_(gp_var, _alignment - 1);

    // Add a sum of all arguments to check if they are correct.
    if (arg_count)
      cc.or_(gp_var.r32(), gp_sum);

    cc.ret(gp_var);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using U = unsigned int;

    using Func0 = U (*)();
    using Func1 = U (*)(U);
    using Func2 = U (*)(U, U);
    using Func3 = U (*)(U, U, U);
    using Func4 = U (*)(U, U, U, U);
    using Func5 = U (*)(U, U, U, U, U);
    using Func6 = U (*)(U, U, U, U, U, U);
    using Func7 = U (*)(U, U, U, U, U, U, U);
    using Func8 = U (*)(U, U, U, U, U, U, U, U);
    using Func9 = U (*)(U, U, U, U, U, U, U, U, U);
    using Func10 = U (*)(U, U, U, U, U, U, U, U, U, U);
    using Func11 = U (*)(U, U, U, U, U, U, U, U, U, U, U);
    using Func12 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U);
    using Func13 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func14 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func15 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);
    using Func16 = U (*)(U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U);

    unsigned int result_ret = 0;
    unsigned int expect_ret = 0;

    switch (_arg_count) {
      case 0:
        result_ret = ptr_as_func<Func0>(_func)();
        expect_ret = 0;
        break;
      case 1:
        result_ret = ptr_as_func<Func1>(_func)(1);
        expect_ret = 1;
        break;
      case 2:
        result_ret = ptr_as_func<Func2>(_func)(1, 2);
        expect_ret = 1 + 2;
        break;
      case 3:
        result_ret = ptr_as_func<Func3>(_func)(1, 2, 3);
        expect_ret = 1 + 2 + 3;
        break;
      case 4:
        result_ret = ptr_as_func<Func4>(_func)(1, 2, 3, 4);
        expect_ret = 1 + 2 + 3 + 4;
        break;
      case 5:
        result_ret = ptr_as_func<Func5>(_func)(1, 2, 3, 4, 5);
        expect_ret = 1 + 2 + 3 + 4 + 5;
        break;
      case 6:
        result_ret = ptr_as_func<Func6>(_func)(1, 2, 3, 4, 5, 6);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6;
        break;
      case 7:
        result_ret = ptr_as_func<Func7>(_func)(1, 2, 3, 4, 5, 6, 7);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7;
        break;
      case 8:
        result_ret = ptr_as_func<Func8>(_func)(1, 2, 3, 4, 5, 6, 7, 8);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;
        break;
      case 9:
        result_ret = ptr_as_func<Func9>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9;
        break;
      case 10:
        result_ret = ptr_as_func<Func10>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;
        break;
      case 11:
        result_ret = ptr_as_func<Func11>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11;
        break;
      case 12:
        result_ret = ptr_as_func<Func12>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12;
        break;
      case 13:
        result_ret = ptr_as_func<Func13>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13;
        break;
      case 14:
        result_ret = ptr_as_func<Func14>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14;
        break;
      case 15:
        result_ret = ptr_as_func<Func15>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15;
        break;
      case 16:
        result_ret = ptr_as_func<Func16>(_func)(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
        expect_ret = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12 + 13 + 14 + 15 + 16;
        break;
    }

    result.assign_format("ret={%u, %u}", result_ret >> 28, result_ret & 0x0FFFFFFFu);
    expect.assign_format("ret={%u, %u}", expect_ret >> 28, expect_ret & 0x0FFFFFFFu);

    return result == expect;
  }

  uint32_t _arg_count;
  uint32_t _alignment;
  bool _preserve_fp;
};

// x86::Compiler - X86Test_NoCode
// ==============================

class X86Test_NoCode : public X86TestCase {
public:
  X86Test_NoCode() : X86TestCase("NoCode") {}

  static void add(TestApp& app) {
    app.add(new X86Test_NoCode());
  }

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<void>());
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    Support::maybe_unused(result, expect);

    using Func = void (*)(void);
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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<void>());
    cc.align(AlignMode::kCode, 0);
    cc.align(AlignMode::kCode, 1);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    Support::maybe_unused(result, expect);

    using Func = void (*)(void);
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

  void compile(x86::Compiler& cc) override {
    FuncNode* func = cc.add_func(FuncSignature::build<void>());
    func->add_attributes(FuncAttributes::kIndirectBranchProtection);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    Support::maybe_unused(result, expect);

    using Func = void (*)(void);
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

  void compile(x86::Compiler& cc) override {
    Label L0 = cc.new_label();
    Label L1 = cc.new_label();
    Label L2 = cc.new_label();
    Label LEnd = cc.new_label();

    x86::Gp dst = cc.new_gp_ptr("dst");
    x86::Gp val = cc.new_gp32("val");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, int*, int>());
    func_node->set_arg(0, dst);
    func_node->set_arg(1, val);

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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(int*, int);
    Func func = ptr_as_func<Func>(_func);

    int arr[5] = { -1, -1, -1, -1, -1 };
    int exp[5] = {  0,  0,  0,  3,  4 };

    for (int i = 0; i < 5; i++)
      func(&arr[i], i);

    result.assign_format("ret={%d, %d, %d, %d, %d}", arr[0], arr[1], arr[2], arr[3], arr[4]);
    expect.assign_format("ret={%d, %d, %d, %d, %d}", exp[0], exp[1], exp[2], exp[3], exp[4]);

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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<void>());

    Label L1 = cc.new_label();
    Label L2 = cc.new_label();
    Label L3 = cc.new_label();

    cc.jmp(L2);

    cc.bind(L1);
    cc.jmp(L3);

    cc.bind(L2);
    cc.jmp(L1);

    cc.bind(L3);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    Support::maybe_unused(result, expect);

    using Func = void (*)(void);
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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());
    for (uint32_t i = 0; i < 1000; i++) {
      Label L = cc.new_label();
      cc.jmp(L);
      cc.bind(L);
    }

    x86::Gp ret = cc.new_gp32("ret");
    cc.xor_(ret, ret);
    cc.ret(ret);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);

    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = 0;

    result.assign_format("ret={%d}", result_ret);
    expect.assign_format("ret={%d}", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<void>());

    Label L_1 = cc.new_label();
    Label L_2 = cc.new_label();
    Label L_3 = cc.new_label();
    Label L_4 = cc.new_label();
    Label L_5 = cc.new_label();
    Label L_6 = cc.new_label();
    Label L_7 = cc.new_label();

    x86::Gp v0 = cc.new_gp32("v0");
    x86::Gp v1 = cc.new_gp32("v1");

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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void);
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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<void>());

    Label L_1 = cc.new_label();
    Label L_2 = cc.new_label();

    x86::Gp v0 = cc.new_gp32("v0");
    x86::Gp v1 = cc.new_gp32("v1");

    cc.jmp(L_1);
    cc.bind(L_2);
    cc.mov(v0, 1);
    cc.mov(v1, 2);
    cc.cmp(v0, v1);
    cc.jz(L_2);
    cc.jmp(L_1);

    cc.bind(L_1);
    cc.ret();
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void);
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
    _name.assign_format("JumpTable {%s}", annotated ? "Annotated" : "Unknown Reg/Mem");
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

  void compile(x86::Compiler& cc) override {
    x86::Vec a = cc.new_xmm_ss("a");
    x86::Vec b = cc.new_xmm_ss("b");
    x86::Gp op = cc.new_gp32("op");
    x86::Gp target = cc.new_gp_ptr("target");
    x86::Gp offset = cc.new_gp_ptr("offset");

    Label L_Table = cc.new_label();
    Label L_Add = cc.new_label();
    Label L_Sub = cc.new_label();
    Label L_Mul = cc.new_label();
    Label L_Div = cc.new_label();
    Label L_End = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<float, float, float, uint32_t>());
    func_node->set_arg(0, a);
    func_node->set_arg(1, b);
    func_node->set_arg(2, op);

    cc.lea(offset, x86::ptr(L_Table));
    if (cc.is_64bit())
      cc.movsxd(target, x86::dword_ptr(offset, op.clone_as(offset), 2));
    else
      cc.mov(target, x86::dword_ptr(offset, op.clone_as(offset), 2));
    cc.add(target, offset);

    // JumpAnnotation allows to annotate all possible jump targets of
    // instructions where it cannot be deduced from operands.
    if (_annotated) {
      JumpAnnotation* annotation = cc.new_jump_annotation();
      annotation->add_label(L_Add);
      annotation->add_label(L_Sub);
      annotation->add_label(L_Mul);
      annotation->add_label(L_Div);
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

    cc.end_func();

    cc.bind(L_Table);
    cc.embed_label_delta(L_Add, L_Table, 4);
    cc.embed_label_delta(L_Sub, L_Table, 4);
    cc.embed_label_delta(L_Mul, L_Table, 4);
    cc.embed_label_delta(L_Div, L_Table, 4);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = float (*)(float, float, uint32_t);
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

    result.assign_format("ret={%f, %f, %f, %f}", double(results[0]), double(results[1]), double(results[2]), double(results[3]));
    expect.assign_format("ret={%f, %f, %f, %f}", double(expected[0]), double(expected[1]), double(expected[2]), double(expected[3]));

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

  void compile(x86::Compiler& cc) override {
    x86::Gp result = cc.new_gp32("result");
    x86::Gp value = cc.new_gp32("value");
    x86::Gp target = cc.new_gp_ptr("target");
    x86::Gp offset = cc.new_gp_ptr("offset");

    Label L_Table = cc.new_label();
    Label L_Begin = cc.new_label();
    Label L_Case0 = cc.new_label();
    Label L_Case1 = cc.new_label();
    Label L_End = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int>());
    func_node->set_arg(0, value);

    cc.bind(L_Begin);
    cc.lea(offset, x86::ptr(L_Table));
    if (cc.is_64bit())
      cc.movsxd(target, x86::dword_ptr(offset, value.clone_as(offset), 2));
    else
      cc.mov(target, x86::dword_ptr(offset, value.clone_as(offset), 2));
    cc.add(target, offset);

    {
      JumpAnnotation* annotation = cc.new_jump_annotation();
      annotation->add_label(L_Case0);
      annotation->add_label(L_Case1);
      annotation->add_label(L_Begin); // Never used, just for the purpose of the test.
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

    cc.end_func();

    cc.bind(L_Table);
    cc.embed_label_delta(L_Case0, L_Table, 4);
    cc.embed_label_delta(L_Case1, L_Table, 4);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int);
    Func func = ptr_as_func<Func>(_func);

    int results[2];
    int expected[2];

    results[0] = func(0);
    results[1] = func(1);

    expected[0] = 0;
    expected[1] = 1;

    result.assign_format("ret={%d, %d}", results[0], results[1]);
    expect.assign_format("ret={%d, %d}", expected[0], expected[1]);

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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    Label L_Target = cc.new_label();
    x86::Gp target = cc.new_gp_ptr("target");
    x86::Gp result = cc.new_gp32("result");

    JumpAnnotation* annotation = cc.new_jump_annotation();
    annotation->add_label(L_Target);

    cc.lea(target, x86::ptr(L_Target));
    cc.jmp(target, annotation);

    cc.bind(L_Target);
    cc.mov(result, 1234);
    cc.ret(result);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int out = func();
    int expected = 1234;

    result.assign_format("ret=%d", out);
    expect.assign_format("ret=%d", expected);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp result = cc.new_gp32("result");
    x86::Gp condition = cc.new_gp32("condition");

    FuncNode* func = cc.add_func(FuncSignature::build<int, int>());
    func->set_arg(0, condition);

    Label L_NonZero = cc.new_label();
    cc.test(condition, condition);
    cc.jnz(L_NonZero);

    {
      JumpAnnotation* annotation = cc.new_jump_annotation();
      Label L_Target = cc.new_label();
      annotation->add_label(L_Target);

      x86::Gp target = cc.new_gp_ptr("target");
      cc.lea(target, x86::ptr(L_Target));
      cc.jmp(target, annotation);
      cc.bind(L_Target);
      cc.mov(result, 1234);
      cc.ret(result);
    }

    {
      JumpAnnotation* annotation = cc.new_jump_annotation();
      Label L_Target1 = cc.new_label();
      Label L_Target2 = cc.new_label();
      annotation->add_label(L_Target1);
      annotation->add_label(L_Target2);

      cc.bind(L_NonZero);
      x86::Gp target = cc.new_gp_ptr("target");
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

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int);
    Func func = ptr_as_func<Func>(_func);

    int results[2] = { func(0), func(1) };
    int expected[2] = { 1234, 4321 };

    result.assign_format("ret={%d, %d}", results[0], results[1]);
    expect.assign_format("ret={%d, %d}", expected[0], expected[1]);

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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    x86::Gp v0 = cc.new_gp32("v0");
    x86::Gp v1 = cc.new_gp32("v1");
    x86::Gp v2 = cc.new_gp32("v2");
    x86::Gp v3 = cc.new_gp32("v3");
    x86::Gp v4 = cc.new_gp32("v4");

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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = 1 + 2 + 3 + 4;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocMany1
// ==================================

class X86Test_AllocMany1 : public X86TestCase {
public:
  X86Test_AllocMany1() : X86TestCase("AllocMany1") {}

  static inline constexpr uint32_t kCount = 8;

  static void add(TestApp& app) {
    app.add(new X86Test_AllocMany1());
  }

  void compile(x86::Compiler& cc) override {
    x86::Gp a0 = cc.new_gp_ptr("a0");
    x86::Gp a1 = cc.new_gp_ptr("a1");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, int*, int*>());
    func_node->set_arg(0, a0);
    func_node->set_arg(1, a1);

    // Create some variables.
    x86::Gp t = cc.new_gp32("t");
    x86::Gp x[kCount];

    uint32_t i;

    // Setup variables (use mov with reg/imm to se if register allocator works).
    for (i = 0; i < kCount; i++) x[i] = cc.new_gp32("x%u", i);
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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(int*, int*);
    Func func = ptr_as_func<Func>(_func);

    int result_x = 0;
    int result_y = 0;

    int expect_x =  36;
    int expect_y = -36;

    func(&result_x, &result_y);

    result.assign_format("ret={x=%d, y=%d}", result_x, result_y);
    expect.assign_format("ret={x=%d, y=%d}", expect_x, expect_y);

    return result_x == expect_x && result_y == expect_y;
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

  void compile(x86::Compiler& cc) override {
    x86::Gp a = cc.new_gp_ptr("a");
    x86::Gp v[32];

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, uint32_t*>());
    func_node->set_arg(0, a);

    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) v[i] = cc.new_gp32("v%d", i);
    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) cc.xor_(v[i], v[i]);

    x86::Gp x = cc.new_gp32("x");
    Label L = cc.new_label();

    cc.mov(x, 32);
    cc.bind(L);
    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) cc.add(v[i], i);

    cc.dec(x);
    cc.jnz(L);
    for (uint32_t i = 0; i < ASMJIT_ARRAY_SIZE(v); i++) cc.mov(x86::dword_ptr(a, int(i * 4)), v[i]);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(uint32_t*);
    Func func = ptr_as_func<Func>(_func);

    uint32_t i;
    uint32_t result_buf[32] {};
    uint32_t expect_buf[32] {};

    for (i = 0; i < ASMJIT_ARRAY_SIZE(result_buf); i++)
      expect_buf[i] = i * 32;
    func(result_buf);

    for (i = 0; i < ASMJIT_ARRAY_SIZE(result_buf); i++) {
      if (i != 0) {
        result.append(',');
        expect.append(',');
      }

      result.append_format("%u", result_buf[i]);
      expect.append_format("%u", expect_buf[i]);
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

  void compile(x86::Compiler& cc) override {
    x86::Gp x = cc.new_gp8("x");
    x86::Gp y = cc.new_gp32("y");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int8_t>());
    func_node->set_arg(0, x);

    cc.movsx(y, x);

    cc.ret(y);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int8_t);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(int8_t(-13));
    int expect_ret = -13;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp x = cc.new_gp32("x");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int, int>());
    func_node->set_arg(2, x);

    cc.ret(x);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(42, 155, 199);
    int expect_ret = 199;

    result.assign_format("ret={%d}", result_ret);
    expect.assign_format("ret={%d}", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, void*, void*, void*, void*, void*, void*, void*>());
    x86::Gp var[8];

    for (uint32_t i = 0; i < 8; i++) {
      var[i] = cc.new_gp_ptr("var%u", i);
      func_node->set_arg(i, var[i]);
    }

    for (uint32_t i = 0; i < 8; i++) {
      cc.add(var[i], int(i + 1));
    }

    // Move some data into buffer provided by arguments so we can verify if it
    // really works without looking into assembler output.
    for (uint32_t i = 0; i < 8; i++) {
      cc.add(x86::byte_ptr(var[i]), int(i + 1));
    }

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, void*, void*, void*, void*, void*, void*, void*);
    Func func = ptr_as_func<Func>(_func);

    uint8_t result_buf[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t expect_buf[9] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };

    func(result_buf, result_buf, result_buf, result_buf,
         result_buf, result_buf, result_buf, result_buf);

    result.assign_format("buf={%d, %d, %d, %d, %d, %d, %d, %d, %d}",
      result_buf[0], result_buf[1], result_buf[2], result_buf[3],
      result_buf[4], result_buf[5], result_buf[6], result_buf[7],
      result_buf[8]);

    expect.assign_format("buf={%d, %d, %d, %d, %d, %d, %d, %d, %d}",
      expect_buf[0], expect_buf[1], expect_buf[2], expect_buf[3],
      expect_buf[4], expect_buf[5], expect_buf[6], expect_buf[7],
      expect_buf[8]);

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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, float, float, float, float, float, float, float, void*>());

    x86::Gp p = cc.new_gp_ptr("p");
    x86::Vec xv[7];

    for (uint32_t i = 0; i < 7; i++) {
      xv[i] = cc.new_xmm_ss("xv%u", i);
      func_node->set_arg(i, xv[i]);
    }

    func_node->set_arg(7, p);

    cc.addss(xv[0], xv[1]);
    cc.addss(xv[0], xv[2]);
    cc.addss(xv[0], xv[3]);
    cc.addss(xv[0], xv[4]);
    cc.addss(xv[0], xv[5]);
    cc.addss(xv[0], xv[6]);

    cc.movss(x86::ptr(p), xv[0]);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(float, float, float, float, float, float, float, float*);
    Func func = ptr_as_func<Func>(_func);

    float result_ret = 0;
    float expect_ret = 1.0f + 2.0f + 3.0f + 4.0f + 5.0f + 6.0f + 7.0f;

    func(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, &result_ret);

    result.assign_format("ret={%g}", double(result_ret));
    expect.assign_format("ret={%g}", double(expect_ret));

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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, double, double, double, double, double, double, double, void*>());

    x86::Gp p = cc.new_gp_ptr("p");
    x86::Vec xv[7];

    for (uint32_t i = 0; i < 7; i++) {
      xv[i] = cc.new_xmm_sd("xv%u", i);
      func_node->set_arg(i, xv[i]);
    }

    func_node->set_arg(7, p);

    cc.addsd(xv[0], xv[1]);
    cc.addsd(xv[0], xv[2]);
    cc.addsd(xv[0], xv[3]);
    cc.addsd(xv[0], xv[4]);
    cc.addsd(xv[0], xv[5]);
    cc.addsd(xv[0], xv[6]);

    cc.movsd(x86::ptr(p), xv[0]);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(double, double, double, double, double, double, double, double*);
    Func func = ptr_as_func<Func>(_func);

    double result_ret = 0;
    double expect_ret = 1.0 + 2.0 + 3.0 + 4.0 + 5.0 + 6.0 + 7.0;

    func(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, &result_ret);

    result.assign_format("ret={%g}", result_ret);
    expect.assign_format("ret={%g}", expect_ret);

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
    Support::maybe_unused(app);
#endif
  }

  void compile(x86::Compiler& cc) override {
    x86::Vec a = cc.new_xmm("xmm_a");
    x86::Vec b = cc.new_xmm("xmm_b");

    FuncNode* func_node = cc.add_func(FuncSignature::build<Type::Vec128, Type::Vec128, Type::Vec128>());
    func_node->set_arg(0, a);
    func_node->set_arg(1, b);

    cc.paddb(a, b);
    cc.ret(a);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = __m128i (*)(__m128i, __m128i);
    Func func = ptr_as_func<Func>(_func);

    uint8_t a_data[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    uint8_t b_data[16] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

    uint8_t r_data[16] {};
    uint8_t e_data[16] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };

    __m128i a_vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(a_data));
    __m128i b_vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(b_data));

    __m128i r_vec = func(a_vec, b_vec);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(r_data), r_vec);

    result.append_hex(r_data, 16);
    expect.append_hex(e_data, 16);

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

  void compile(x86::Compiler& cc) override {
    x86::Vec x = cc.new_xmm_ss("x");

    FuncNode* func_node = cc.add_func(FuncSignature::build<float, float>());
    func_node->set_arg(0, x);

    cc.ret(x);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = float (*)(float);
    Func func = ptr_as_func<Func>(_func);

    float result_ret = func(42.0f);
    float expect_ret = 42.0f;

    result.assign_format("ret={%g}", double(result_ret));
    expect.assign_format("ret={%g}", double(expect_ret));

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

  void compile(x86::Compiler& cc) override {
    x86::Vec x = cc.new_xmm_ss("x");
    x86::Vec y = cc.new_xmm_ss("y");

    FuncNode* func_node = cc.add_func(FuncSignature::build<float, float, float>());
    func_node->set_arg(0, x);
    func_node->set_arg(1, y);

    cc.addss(x, y);
    cc.ret(x);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = float (*)(float, float);
    Func func = ptr_as_func<Func>(_func);

    float result_ret = func(1.0f, 2.0f);
    float expect_ret = 1.0f + 2.0f;

    result.assign_format("ret={%g}", double(result_ret));
    expect.assign_format("ret={%g}", double(expect_ret));

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

  void compile(x86::Compiler& cc) override {
    x86::Vec x = cc.new_xmm_sd("x");

    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double>());
    func_node->set_arg(0, x);

    cc.ret(x);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double);
    Func func = ptr_as_func<Func>(_func);

    double result_ret = func(42.0);
    double expect_ret = 42.0;

    result.assign_format("ret={%g}", result_ret);
    expect.assign_format("ret={%g}", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Vec x = cc.new_xmm_sd("x");
    x86::Vec y = cc.new_xmm_sd("y");

    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double, double>());
    func_node->set_arg(0, x);
    func_node->set_arg(1, y);

    cc.addsd(x, y);
    cc.ret(x);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double result_ret = func(1.0, 2.0);
    double expect_ret = 1.0 + 2.0;

    result.assign_format("ret={%g}", result_ret);
    expect.assign_format("ret={%g}", expect_ret);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AllocStack
// ==================================

class X86Test_AllocStack : public X86TestCase {
public:
  X86Test_AllocStack() : X86TestCase("AllocStack") {}

  static inline constexpr uint32_t kSize = 256u;

  static void add(TestApp& app) {
    app.add(new X86Test_AllocStack());
  }

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    x86::Mem stack = cc.new_stack(kSize, 1);
    stack.set_size(1);

    x86::Gp i = cc.new_gp_ptr("i");
    x86::Gp a = cc.new_gp32("a");
    x86::Gp b = cc.new_gp32("b");

    Label L_1 = cc.new_label();
    Label L_2 = cc.new_label();

    // Fill stack by sequence [0, 1, 2, 3 ... 255].
    cc.xor_(i, i);

    x86::Mem stack_with_index = stack.clone();
    stack_with_index.set_index(i, 0);

    cc.bind(L_1);
    cc.mov(stack_with_index, i.r8());
    cc.inc(i);
    cc.cmp(i, 255);
    cc.jle(L_1);

    // Sum sequence in stack.
    cc.xor_(i, i);
    cc.xor_(a, a);

    cc.bind(L_2);
    cc.movzx(b, stack_with_index);
    cc.add(a, b);
    cc.inc(i);
    cc.cmp(i, 255);
    cc.jle(L_2);

    cc.ret(a);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = 32640;

    result.assign_int(result_ret);
    expect.assign_int(expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp dst_hi = cc.new_gp_ptr("dst_hi");
    x86::Gp dst_lo = cc.new_gp_ptr("dst_lo");

    x86::Gp v_hi = cc.new_gp32("v_hi");
    x86::Gp v_lo = cc.new_gp32("v_lo");
    x86::Gp src = cc.new_gp32("src");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, int*, int*, int, int>());
    func_node->set_arg(0, dst_hi);
    func_node->set_arg(1, dst_lo);
    func_node->set_arg(2, v_lo);
    func_node->set_arg(3, src);

    cc.imul(v_hi, v_lo, src);
    cc.mov(x86::dword_ptr(dst_hi), v_hi);
    cc.mov(x86::dword_ptr(dst_lo), v_lo);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(int*, int*, int, int);
    Func func = ptr_as_func<Func>(_func);

    int v0 = 4;
    int v1 = 4;

    int result_hi = 0;
    int result_lo = 0;

    int expect_hi = 0;
    int expect_lo = v0 * v1;

    func(&result_hi, &result_lo, v0, v1);

    result.assign_format("hi=%d, lo=%d", result_hi, result_lo);
    expect.assign_format("hi=%d, lo=%d", expect_hi, expect_lo);

    return result_hi == expect_hi && result_lo == expect_lo;
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

  void compile(x86::Compiler& cc) override {
    x86::Gp dst = cc.new_gp_ptr("dst");
    x86::Gp src = cc.new_gp_ptr("src");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, int*, const int*>());
    func_node->set_arg(0, dst);
    func_node->set_arg(1, src);

    for (unsigned int i = 0; i < 4; i++) {
      x86::Gp x  = cc.new_gp32("x");
      x86::Gp y  = cc.new_gp32("y");
      x86::Gp hi = cc.new_gp32("hi");

      cc.mov(x, x86::dword_ptr(src, 0));
      cc.mov(y, x86::dword_ptr(src, 4));

      cc.imul(hi, x, y);
      cc.add(x86::dword_ptr(dst, 0), hi);
      cc.add(x86::dword_ptr(dst, 4), x);
    }

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(int*, const int*);
    Func func = ptr_as_func<Func>(_func);

    int src[2] = { 4, 9 };
    int result_ret[2] = { 0, 0 };
    int expect_ret[2] = { 0, (4 * 9) * 4 };

    func(result_ret, src);

    result.assign_format("ret={%d, %d}", result_ret[0], result_ret[1]);
    expect.assign_format("ret={%d, %d}", expect_ret[0], expect_ret[1]);

    return result_ret[0] == expect_ret[0] && result_ret[1] == expect_ret[1];
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

  void compile(x86::Compiler& cc) override {
    x86::Gp a = cc.new_gp32("a");
    x86::Gp b = cc.new_gp32("b");
    x86::Gp dummy = cc.new_gp32("dummy");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int>());
    func_node->set_arg(0, a);
    func_node->set_arg(1, b);

    cc.xor_(dummy, dummy);
    cc.idiv(dummy, a, b);

    cc.ret(a);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int v0 = 2999;
    int v1 = 245;

    int result_ret = func(v0, v1);
    int expect_ret = 2999 / 245;

    result.assign_format("result=%d", result_ret);
    expect.assign_format("result=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp src0 = cc.new_gp32("src0");
    x86::Gp src1 = cc.new_gp32("src1");
    x86::Gp dst0 = cc.new_gp_ptr("dst0");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, int, int, char*>());
    func_node->set_arg(0, src0);
    func_node->set_arg(1, src1);
    func_node->set_arg(2, dst0);

    cc.cmp(src0, src1);
    cc.setz(x86::byte_ptr(dst0));

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(int, int, char*);
    Func func = ptr_as_func<Func>(_func);

    char result_buf[4] {};
    char expect_buf[4] = { 1, 0, 0, 1 };

    func(0, 0, &result_buf[0]); // We are expecting 1 (0 == 0).
    func(0, 1, &result_buf[1]); // We are expecting 0 (0 != 1).
    func(1, 0, &result_buf[2]); // We are expecting 0 (1 != 0).
    func(1, 1, &result_buf[3]); // We are expecting 1 (1 == 1).

    result.assign_format("out={%d, %d, %d, %d}", result_buf[0], result_buf[1], result_buf[2], result_buf[3]);
    expect.assign_format("out={%d, %d, %d, %d}", expect_buf[0], expect_buf[1], expect_buf[2], expect_buf[3]);

    return result_buf[0] == expect_buf[0] &&
           result_buf[1] == expect_buf[1] &&
           result_buf[2] == expect_buf[2] &&
           result_buf[3] == expect_buf[3] ;
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

  void compile(x86::Compiler& cc) override {
    x86::Gp dst = cc.new_gp_ptr("dst");
    x86::Gp var = cc.new_gp32("var");
    x86::Gp v_shl_param = cc.new_gp32("v_shl_param");
    x86::Gp v_ror_param = cc.new_gp32("v_ror_param");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, int*, int, int, int>());
    func_node->set_arg(0, dst);
    func_node->set_arg(1, var);
    func_node->set_arg(2, v_shl_param);
    func_node->set_arg(3, v_ror_param);

    cc.shl(var, v_shl_param);
    cc.ror(var, v_ror_param);
    cc.mov(x86::dword_ptr(dst), var);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(int*, int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int v0 = 0x000000FF;

    int result_ret = 0;
    int expect_ret = 0x0000FF00;

    func(&result_ret, v0, 16, 8);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }
};

// x86::Compiler - X86Test_GpbLo
// =============================

class X86Test_GpbLo1 : public X86TestCase {
public:
  X86Test_GpbLo1() : X86TestCase("GpbLo1") {}

  static inline constexpr uint32_t kCount = 32u;

  static void add(TestApp& app) {
    app.add(new X86Test_GpbLo1());
  }

  void compile(x86::Compiler& cc) override {
    x86::Gp reg_Ptr = cc.new_gp_ptr("reg_Ptr");
    x86::Gp sum = cc.new_gp32("sum");
    x86::Gp x[kCount];

    FuncNode* func_node = cc.add_func(FuncSignature::build<uint32_t, uint32_t*>());
    func_node->set_arg(0, reg_Ptr);

    for (uint32_t i = 0; i < kCount; i++) {
      x[i] = cc.new_gp32("x%u", i);
    }

    // Init pseudo-regs with values from our array.
    for (uint32_t i = 0; i < kCount; i++) {
      cc.mov(x[i], x86::dword_ptr(reg_Ptr, int(i * 4)));
    }

    for (uint32_t i = 2; i < kCount; i++) {
      // Add and truncate to 8 bit; no purpose, just mess with jit.
      cc.add  (x[i  ], x[i-1]);
      cc.movzx(x[i  ], x[i  ].r8());
      cc.movzx(x[i-2], x[i-1].r8());
      cc.movzx(x[i-1], x[i-2].r8());
    }

    // Sum up all computed values.
    cc.mov(sum, 0);
    for (uint32_t i = 0; i < kCount; i++) {
      cc.add(sum, x[i]);
    }

    // Return the sum.
    cc.ret(sum);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint32_t*);
    Func func = ptr_as_func<Func>(_func);

    uint32_t i;
    uint32_t buf[kCount];
    uint32_t result_ret = 0;
    uint32_t expect_ret = 0;

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
      expect_ret += buf[i];
    }

    for (i = 0; i < kCount; i++) {
      buf[i] = 1;
    }
    result_ret = func(buf);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp v = cc.new_gp32("v");

    FuncNode* func_node = cc.add_func(FuncSignature::build<uint32_t, uint32_t>());
    func_node->set_arg(0, v);

    cc.mov(v.r8(), 0xFF);
    cc.ret(v);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t result_ret = func(0x12345678u);
    uint32_t expect_ret = 0x123456FFu;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp dst = cc.new_gp_ptr("dst");
    x86::Gp src = cc.new_gp_ptr("src");
    x86::Gp cnt = cc.new_gp_ptr("cnt");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, void*, size_t>());
    func_node->set_arg(0, dst);
    func_node->set_arg(1, src);
    func_node->set_arg(2, cnt);

    cc.rep(cnt).movs(x86::byte_ptr(dst), x86::byte_ptr(src));
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, void*, size_t);
    Func func = ptr_as_func<Func>(_func);

    char dst[20] = { 0 };
    char src[20] = "Hello AsmJit!";
    func(dst, src, strlen(src) + 1);

    result.assign_format("ret=\"%s\"", dst);
    expect.assign_format("ret=\"%s\"", src);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp v1 = cc.new_gp32("v1");
    x86::Gp v2 = cc.new_gp32("v2");

    Label L_1 = cc.new_label();
    Label L_2 = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int>());
    func_node->set_arg(0, v1);
    func_node->set_arg(1, v2);

    cc.cmp(v1, v2);
    cc.jg(L_1);

    cc.mov(v1, 1);
    cc.jmp(L_2);

    cc.bind(L_1);
    cc.mov(v1, 2);

    cc.bind(L_2);
    cc.ret(v1);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.append_format("ret={%d, %d}", a, b);
    expect.append_format("ret={%d, %d}", 1, 2);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp v1 = cc.new_gp32("v1");
    x86::Gp v2 = cc.new_gp32("v2");

    Label L_1 = cc.new_label();
    Label L_2 = cc.new_label();
    Label L_3 = cc.new_label();
    Label L_4 = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int>());
    func_node->set_arg(0, v1);
    func_node->set_arg(1, v2);

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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.append_format("ret={%d, %d}", a, b);
    expect.append_format("ret={%d, %d}", 1, 2);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp v1 = cc.new_gp32("v1");
    x86::Gp v2 = cc.new_gp32("v2");
    x86::Gp counter = cc.new_gp32("counter");

    Label L_1 = cc.new_label();
    Label L_Loop = cc.new_label();
    Label L_Exit = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int>());
    func_node->set_arg(0, v1);
    func_node->set_arg(1, v2);

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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.append_format("ret={%d, %d}", a, b);
    expect.append_format("ret={%d, %d}", 1, 2);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp v1 = cc.new_gp32("v1");
    x86::Gp v2 = cc.new_gp32("v2");
    x86::Gp counter = cc.new_gp32("counter");

    Label L_1 = cc.new_label();
    Label L_Loop1 = cc.new_label();
    Label L_Loop2 = cc.new_label();
    Label L_Exit = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int>());
    func_node->set_arg(0, v1);
    func_node->set_arg(1, v2);

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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int a = func(0, 1);
    int b = func(1, 0);

    result.append_format("ret={%d, %d}", a, b);
    expect.append_format("ret={%d, %d}", 1, 2);

    return result == expect;
  }
};

// x86::Compiler - X86Test_Memcpy
// ==============================

class X86Test_Memcpy : public X86TestCase {
public:
  X86Test_Memcpy() : X86TestCase("Memcpy") {}

  static inline constexpr uint32_t kCount = 32u;

  static void add(TestApp& app) {
    app.add(new X86Test_Memcpy());
  }

  void compile(x86::Compiler& cc) override {
    x86::Gp dst = cc.new_gp_ptr("dst");
    x86::Gp src = cc.new_gp_ptr("src");
    x86::Gp cnt = cc.new_gp_ptr("cnt");

    Label L_Loop = cc.new_label();                   // Create base labels we use
    Label L_Exit = cc.new_label();                   // in our function.

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, uint32_t*, const uint32_t*, size_t>());
    func_node->set_arg(0, dst);
    func_node->set_arg(1, src);
    func_node->set_arg(2, cnt);

    cc.test(cnt, cnt);                              // Exit if the size is zero.
    cc.jz(L_Exit);

    cc.bind(L_Loop);                                // Bind the loop label here.

    x86::Gp tmp = cc.new_gp32("tmp");               // Copy a single dword (4 bytes).
    cc.mov(tmp, x86::dword_ptr(src));
    cc.mov(x86::dword_ptr(dst), tmp);

    cc.add(src, 4);                                 // Increment dst/src pointers.
    cc.add(dst, 4);

    cc.dec(cnt);                                    // Loop until cnt isn't zero.
    cc.jnz(L_Loop);

    cc.bind(L_Exit);                                // Bind the exit label here.
    cc.end_func();                                   // End of function.
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(uint32_t*, const uint32_t*, size_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t i;

    uint32_t dst_buffer[kCount];
    uint32_t src_buffer[kCount];

    for (i = 0; i < kCount; i++) {
      dst_buffer[i] = 0;
      src_buffer[i] = i;
    }

    func(dst_buffer, src_buffer, kCount);

    result.assign("buf={");
    expect.assign("buf={");

    for (i = 0; i < kCount; i++) {
      if (i != 0) {
        result.append(", ");
        expect.append(", ");
      }

      result.append_format("%u", unsigned(dst_buffer[i]));
      expect.append_format("%u", unsigned(src_buffer[i]));
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

  void compile(x86::Compiler& cc) override {
    x86::Gp cond = cc.new_gp32("cond");
    x86::Gp ret = cc.new_gp32("ret");
    x86::Gp a = cc.new_gp32("a");
    x86::Gp b = cc.new_gp32("b");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int, int>());
    func_node->set_arg(0, cond);
    func_node->set_arg(1, a);
    func_node->set_arg(2, b);

    Label L_Ret = cc.new_label();
    Label L_Extra = cc.new_label();

    cc.test(cond, cond);
    cc.jnz(L_Extra);

    cc.mov(ret, a);
    cc.add(ret, b);

    cc.bind(L_Ret);
    cc.ret(ret);

    // Emit code sequence at the end of the function.
    BaseNode* prev_cursor = cc.set_cursor(func_node->end_node()->prev());
    cc.bind(L_Extra);
    cc.mov(ret, a);
    cc.sub(ret, b);
    cc.jmp(L_Ret);
    cc.set_cursor(prev_cursor);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int ret1 = func(0, 4, 5);
    int ret2 = func(1, 4, 5);

    int exp1 = 4 + 5;
    int exp2 = 4 - 5;

    result.assign_format("ret={%d, %d}", ret1, ret2);
    expect.assign_format("ret={%d, %d}", exp1, exp2);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AlphaBlend
// ==================================

class X86Test_AlphaBlend : public X86TestCase {
public:
  X86Test_AlphaBlend() : X86TestCase("AlphaBlend") {}

  static inline constexpr uint32_t kCount = 17u;

  static void add(TestApp& app) {
    app.add(new X86Test_AlphaBlend());
  }

  static uint32_t blend_src_over(uint32_t d, uint32_t s) {
    uint32_t sa_inv = ~s >> 24;

    uint32_t d_20 = (d     ) & 0x00FF00FF;
    uint32_t d_31 = (d >> 8) & 0x00FF00FF;

    d_20 *= sa_inv;
    d_31 *= sa_inv;

    d_20 = ((d_20 + ((d_20 >> 8) & 0x00FF00FFu) + 0x00800080u) & 0xFF00FF00u) >> 8;
    d_31 = ((d_31 + ((d_31 >> 8) & 0x00FF00FFu) + 0x00800080u) & 0xFF00FF00u);

    return d_20 + d_31 + s;
  }

  void compile(x86::Compiler& cc) override {
    asmtest::generate_sse_alpha_blend(cc, true);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*, size_t);
    Func func = ptr_as_func<Func>(_func);

    static const uint32_t dst_const_data[] = { 0x00000000, 0x10101010, 0x20100804, 0x30200003, 0x40204040, 0x5000004D, 0x60302E2C, 0x706F6E6D, 0x807F4F2F, 0x90349001, 0xA0010203, 0xB03204AB, 0xC023AFBD, 0xD0D0D0C0, 0xE0AABBCC, 0xFFFFFFFF, 0xF8F4F2F1 };
    static const uint32_t src_const_data[] = { 0xE0E0E0E0, 0xA0008080, 0x341F1E1A, 0xFEFEFEFE, 0x80302010, 0x49490A0B, 0x998F7798, 0x00000000, 0x01010101, 0xA0264733, 0xBAB0B1B9, 0xFF000000, 0xDAB0A0C1, 0xE0BACFDA, 0x99887766, 0xFFFFFF80, 0xEE0A5FEC };

    uint32_t dst_buffer_storage[kCount + 3];
    uint32_t src_buffer_storage[kCount + 3];

    // Has to be aligned.
    uint32_t* dst_buffer = (uint32_t*)Support::align_up<intptr_t>((intptr_t)dst_buffer_storage, 16);
    uint32_t* src_buffer = (uint32_t*)Support::align_up<intptr_t>((intptr_t)src_buffer_storage, 16);

    memcpy(dst_buffer, dst_const_data, sizeof(dst_const_data));
    memcpy(src_buffer, src_const_data, sizeof(src_const_data));

    uint32_t i;
    uint32_t exp_buffer[kCount];

    for (i = 0; i < kCount; i++) {
      exp_buffer[i] = blend_src_over(dst_buffer[i], src_buffer[i]);
    }

    func(dst_buffer, src_buffer, kCount);

    result.assign("buf={");
    expect.assign("buf={");

    for (i = 0; i < kCount; i++) {
      if (i != 0) {
        result.append(", ");
        expect.append(", ");
      }

      result.append_format("%08X", unsigned(dst_buffer[i]));
      expect.append_format("%08X", unsigned(exp_buffer[i]));
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
    const CpuInfo& cpu_info = CpuInfo::host();

    if (cpu_info.features().x86().has_avx512_f()) {
      app.add(new X86Test_AVX512_KK());
    }
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<uint32_t, const void*, const void*, uint32_t>());

    x86::Gp a = cc.new_gp_ptr("a");
    x86::Gp b = cc.new_gp_ptr("b");
    x86::Gp pred = cc.new_gp32("pred");
    x86::Gp result = cc.new_gp32("result");

    x86::Vec va = cc.new_zmm("va");
    x86::Vec vb = cc.new_zmm("vb");
    x86::KReg k_in = cc.new_kd("k_in");
    x86::KReg k_out = cc.new_kd("k_out");

    func_node->set_arg(0, a);
    func_node->set_arg(1, b);
    func_node->set_arg(2, pred);

    cc.vmovdqu32(va, x86::ptr(a));
    cc.vmovdqu32(vb, x86::ptr(b));
    cc.kmovd(k_in, pred);
    cc.k(k_in).vpcmpeqd(k_out, va, vb);
    cc.kmovd(result, k_out);
    cc.ret(result);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(const void*, const void*, uint32_t pred_k);
    Func func = ptr_as_func<Func>(_func);

    static const uint32_t src_a[16] = { 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 };
    static const uint32_t src_b[16] = { 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1 };

    uint32_t ret = func(src_a, src_b, 0xF0F0);

    result.assign_format("0x%08X", ret);
    expect.assign_format("0x%08X", 0xA040u);

    return result == expect;
  }
};

// x86::Compiler - X86Test_AVX512_TernLog
// ======================================

class X86Test_AVX512_TernLog : public X86TestCase {
public:
  X86Test_AVX512_TernLog() : X86TestCase("AVX512_TernLog") {}

  static void add(TestApp& app) {
    const CpuInfo& cpu_info = CpuInfo::host();

    if (cpu_info.features().x86().has_avx512_f()) {
      app.add(new X86Test_AVX512_TernLog());
    }
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*>());

    x86::Gp out_ptr = cc.new_gp_ptr("out_ptr");
    x86::Vec vec = cc.new_zmm("vec");

    func_node->set_arg(0, out_ptr);

    cc.vpternlogd(vec, vec, vec, 0xFFu);
    cc.vmovdqu8(x86::ptr(out_ptr), vec);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*);
    Func func = ptr_as_func<Func>(_func);

    uint32_t out[16] {};
    func(out);

    result.assign("{");
    expect.assign("{");

    for (uint32_t i = 0; i < 16; i++) {
      if (i) {
        result.append(", ");
        expect.append(", ");
      }
      result.append_format("0x%08X", out[i]);
      expect.append_format("0x%08X", 0xFFFFFFFFu);
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

  void compile(x86::Compiler& cc) override {
    x86::Gp v0 = cc.new_gp32("v0");
    x86::Gp v1 = cc.new_gp32("v1");

    FuncNode* func_node = cc.add_func(FuncSignature::build<unsigned, uint8_t, uint8_t, uint32_t>());
    func_node->set_arg(0, v0);
    func_node->set_arg(1, v1);

    cc.add(v0, v1);

    cc.ret(v0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint8_t, uint8_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t arg = uint32_t(uintptr_t(_func) & 0xFFFFFFFF);

    unsigned result_ret = func(uint8_t(arg & 0xFF), uint8_t(arg & 0xFF), arg);
    unsigned expect_ret = (arg & 0xFF) * 2;

    result.assign_format("ret=%u", result_ret);
    expect.assign_format("ret=%u", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp v0 = cc.new_gp32("v0");
    x86::Gp v1 = cc.new_gp32("v1");
    x86::Gp v2 = cc.new_gp32("v2");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int, int>());
    func_node->set_arg(0, v0);
    func_node->set_arg(1, v1);
    func_node->set_arg(2, v2);

    // Just do something.
    cc.shl(v0, 1);
    cc.shl(v1, 1);
    cc.shl(v2, 1);

    // Call a function.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), imm((void*)called_fn), FuncSignature::build<int, int, int, int>());
    invoke_node->set_arg(0, v2);
    invoke_node->set_arg(1, v1);
    invoke_node->set_arg(2, v0);
    invoke_node->set_ret(0, v0);

    cc.ret(v0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(3, 2, 1);
    int expect_ret = 36;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }

  static int called_fn(int a, int b, int c) { return (a + b) * c; }
};

// x86::Compiler - X86Test_FuncCallBase2
// =====================================

class X86Test_FuncCallBase2 : public X86TestCase {
public:
  X86Test_FuncCallBase2() : X86TestCase("FuncCallBase2") {}

  static inline constexpr uint32_t kSize = 256u;

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallBase2());
  }

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    const int kTokenSize = 32;

    x86::Mem s1 = cc.new_stack(kTokenSize, 32);
    x86::Mem s2 = cc.new_stack(kTokenSize, 32);

    x86::Gp p1 = cc.new_gp_ptr("p1");
    x86::Gp p2 = cc.new_gp_ptr("p2");

    x86::Gp ret = cc.new_gp32("ret");
    Label L_Exit = cc.new_label();

    static const char token[kTokenSize] = "-+:|abcdefghijklmnopqrstuvwxyz|";
    InvokeNode* invoke_node;

    cc.lea(p1, s1);
    cc.lea(p2, s2);

    // Try to corrupt the stack if wrongly allocated.
    cc.invoke(Out(invoke_node), imm((void*)memcpy), FuncSignature::build<void*, void*, void*, size_t>());
    invoke_node->set_arg(0, p1);
    invoke_node->set_arg(1, imm(token));
    invoke_node->set_arg(2, imm(kTokenSize));
    invoke_node->set_ret(0, p1);

    cc.invoke(Out(invoke_node), imm((void*)memcpy), FuncSignature::build<void*, void*, void*, size_t>());
    invoke_node->set_arg(0, p2);
    invoke_node->set_arg(1, imm(token));
    invoke_node->set_arg(2, imm(kTokenSize));
    invoke_node->set_ret(0, p2);

    cc.invoke(Out(invoke_node), imm((void*)memcmp), FuncSignature::build<int, void*, void*, size_t>());
    invoke_node->set_arg(0, p1);
    invoke_node->set_arg(1, p2);
    invoke_node->set_arg(2, imm(kTokenSize));
    invoke_node->set_ret(0, ret);

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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = 0; // Must be zero, stack addresses must be different.

    result.assign_int(result_ret);
    expect.assign_int(expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp x = cc.new_gp32("x");
    x86::Gp y = cc.new_gp32("y");
    x86::Gp z = cc.new_gp32("z");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int, int>());
    func_node->set_arg(0, x);
    func_node->set_arg(1, y);
    func_node->set_arg(2, z);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)called_fn),
      FuncSignature::build<int, int, int, int>(CallConvId::kStdCall));
    invoke_node->set_arg(0, x);
    invoke_node->set_arg(1, y);
    invoke_node->set_arg(2, z);
    invoke_node->set_ret(0, x);

    cc.ret(x);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(1, 42, 3);
    int expect_ret = called_fn(1, 42, 3);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }

  // STDCALL function that is called inside the generated one.
  static int ASMJIT_STDCALL called_fn(int a, int b, int c) noexcept {
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

  void compile(x86::Compiler& cc) override {
    x86::Gp var = cc.new_gp32("var");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int>());
    func_node->set_arg(0, var);

    InvokeNode* invoke_node;

    cc.invoke(Out(invoke_node), imm((void*)called_fn), FuncSignature::build<int, int>(CallConvId::kFastCall));
    invoke_node->set_arg(0, var);
    invoke_node->set_ret(0, var);

    cc.invoke(Out(invoke_node), imm((void*)called_fn), FuncSignature::build<int, int>(CallConvId::kFastCall));
    invoke_node->set_arg(0, var);
    invoke_node->set_ret(0, var);

    cc.ret(var);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(9);
    int expect_ret = (9 * 9) * (9 * 9);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }

  // FASTCALL function that is called inside the generated one.
  static int ASMJIT_FASTCALL called_fn(int a) noexcept {
    return a * a;
  }
};

// x86::Compiler - X86Test_FuncCallSIMD
// ====================================

#if ASMJIT_ARCH_X86
class X86Test_FuncCallSIMD : public X86TestCase {
public:
  bool _use_vector_call;

  X86Test_FuncCallSIMD(bool use_vector_call)
    : X86TestCase(),
      _use_vector_call(use_vector_call) {
    _name.assign_format("FuncCallSIMD {%s}", _use_vector_call ? "__vectorcall" : "__cdecl");
  }

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallSIMD(false));
#ifdef _MSC_VER
    app.add(new X86Test_FuncCallSIMD(true));
#endif
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, const void*, const void*>());

    x86::Gp result_ptr = cc.new_gp_ptr("result_ptr");
    x86::Gp a_ptr = cc.new_gp_ptr("a_ptr");
    x86::Gp b_ptr = cc.new_gp_ptr("b_ptr");
    x86::Gp p_fn = cc.new_gp_ptr("p_fn");

    x86::Vec xmm_a = cc.new_xmm("xmm_a");
    x86::Vec xmm_b = cc.new_xmm("xmm_b");

    func_node->set_arg(0, result_ptr);
    func_node->set_arg(1, a_ptr);
    func_node->set_arg(2, b_ptr);

    CallConvId call_conv_id = CallConvId::kCDecl;
    Imm p_fn_imm = imm((void*)called_fn_cdecl);

#ifdef _MSC_VER
    if (_use_vector_call) {
      call_conv_id = CallConvId::kVectorCall;
      p_fn_imm = imm((void*)called_fn_vectorcall);
    }
#endif

    cc.mov(p_fn, p_fn_imm);
    cc.movdqu(xmm_a, x86::ptr(a_ptr));
    cc.movdqu(xmm_b, x86::ptr(b_ptr));

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), p_fn, FuncSignature::build<Type::Vec128, Type::Vec128, Type::Vec128>(call_conv_id));

    invoke_node->set_arg(0, xmm_a);
    invoke_node->set_arg(1, xmm_b);
    invoke_node->set_ret(0, xmm_a);

    cc.movdqu(x86::ptr(result_ptr), xmm_a);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*, const void*);
    Func func = ptr_as_func<Func>(_func);

    uint8_t a_data[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    uint8_t b_data[16] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 };

    uint8_t r_data[16] {};
    uint8_t e_data[16] = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15 };

    func(r_data, a_data, b_data);

    result.append_hex(r_data, 16);
    expect.append_hex(e_data, 16);

    return result == expect;
  }

  static __m128i called_fn_cdecl(__m128i a, __m128i b) {
    return _mm_add_epi8(a, b);
  }

#ifdef _MSC_VER
  static __m128i __vectorcall called_fn_vectorcall(__m128i a, __m128i b) {
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

  void compile(x86::Compiler& cc) override {
    FuncSignature f1_signature = FuncSignature::build<void, const void*, const void*, const void*, const void*, void*>();
    FuncSignature f2_signature = FuncSignature::build<Type::Vec128, Type::Vec128, Type::Vec128>(CallConvId::kLightCall2);

    FuncNode* f1_node = cc.new_func(f1_signature);
    FuncNode* f2_node = cc.new_func(f2_signature);

    {
      x86::Gp a_ptr = cc.new_gp_ptr("a_ptr");
      x86::Gp b_ptr = cc.new_gp_ptr("b_ptr");
      x86::Gp c_ptr = cc.new_gp_ptr("c_ptr");
      x86::Gp d_ptr = cc.new_gp_ptr("d_ptr");
      x86::Gp p_out = cc.new_gp_ptr("p_out");

      x86::Vec xmm_a = cc.new_xmm("xmm_a");
      x86::Vec xmm_b = cc.new_xmm("xmm_b");
      x86::Vec xmm_c = cc.new_xmm("xmm_c");
      x86::Vec xmm_d = cc.new_xmm("xmm_d");

      cc.add_func(f1_node);
      f1_node->set_arg(0, a_ptr);
      f1_node->set_arg(1, b_ptr);
      f1_node->set_arg(2, c_ptr);
      f1_node->set_arg(3, d_ptr);
      f1_node->set_arg(4, p_out);

      cc.movups(xmm_a, x86::ptr(a_ptr));
      cc.movups(xmm_b, x86::ptr(b_ptr));
      cc.movups(xmm_c, x86::ptr(c_ptr));
      cc.movups(xmm_d, x86::ptr(d_ptr));

      x86::Vec xmm_x = cc.new_xmm("xmm_x");
      x86::Vec xmm_y = cc.new_xmm("xmm_y");

      InvokeNode* invoke_node;

      cc.invoke(Out(invoke_node), f2_node->label(), f2_signature);
      invoke_node->set_arg(0, xmm_a);
      invoke_node->set_arg(1, xmm_b);
      invoke_node->set_ret(0, xmm_x);

      cc.invoke(Out(invoke_node), f2_node->label(), f2_signature);
      invoke_node->set_arg(0, xmm_c);
      invoke_node->set_arg(1, xmm_d);
      invoke_node->set_ret(0, xmm_y);

      cc.pmullw(xmm_x, xmm_y);
      cc.movups(x86::ptr(p_out), xmm_x);

      cc.end_func();
    }

    {
      x86::Vec xmm_a = cc.new_xmm("xmm_a");
      x86::Vec xmm_b = cc.new_xmm("xmm_b");

      cc.add_func(f2_node);
      f2_node->set_arg(0, xmm_a);
      f2_node->set_arg(1, xmm_b);
      cc.paddw(xmm_a, xmm_b);
      cc.ret(xmm_a);
      cc.end_func();
    }
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(const void*, const void*, const void*, const void*, void*);

    Func func = ptr_as_func<Func>(_func);

    int16_t a[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    int16_t b[8] = { 7, 6, 5, 4, 3, 2, 1, 0 };
    int16_t c[8] = { 1, 3, 9, 7, 5, 4, 2, 1 };
    int16_t d[8] = { 2, 0,-6,-4,-2,-1, 1, 2 };

    int16_t o[8] {};
    int oexp = 7 * 3;

    func(a, b, c, d, o);

    result.assign_format("ret={%02X %02X %02X %02X %02X %02X %02X %02X}", o[0], o[1], o[2], o[3], o[4], o[5], o[6], o[7]);
    expect.assign_format("ret={%02X %02X %02X %02X %02X %02X %02X %02X}", oexp, oexp, oexp, oexp, oexp, oexp, oexp, oexp);

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

  static int called_fn(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    // Prepare.
    x86::Gp va = cc.new_gp32("va");
    x86::Gp vb = cc.new_gp32("vb");
    x86::Gp vc = cc.new_gp32("vc");
    x86::Gp vd = cc.new_gp32("vd");
    x86::Gp ve = cc.new_gp32("ve");
    x86::Gp vf = cc.new_gp32("vf");
    x86::Gp vg = cc.new_gp32("vg");
    x86::Gp vh = cc.new_gp32("vh");
    x86::Gp vi = cc.new_gp32("vi");
    x86::Gp vj = cc.new_gp32("vj");

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
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)called_fn),
      FuncSignature::build<int, int, int, int, int, int, int, int, int, int, int>());
    invoke_node->set_arg(0, va);
    invoke_node->set_arg(1, vb);
    invoke_node->set_arg(2, vc);
    invoke_node->set_arg(3, vd);
    invoke_node->set_arg(4, ve);
    invoke_node->set_arg(5, vf);
    invoke_node->set_arg(6, vg);
    invoke_node->set_arg(7, vh);
    invoke_node->set_arg(8, vi);
    invoke_node->set_arg(9, vj);
    invoke_node->set_ret(0, va);

    cc.ret(va);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = called_fn(0x03, 0x12, 0xA0, 0x0B, 0x2F, 0x02, 0x0C, 0x12, 0x18, 0x1E);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  static int called_fn(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return (a * b * c * d * e) + (f * g * h * i * j);
  }

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    // Prepare.
    x86::Gp a = cc.new_gp32("a");
    cc.mov(a, 3);

    // Call function.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)called_fn),
      FuncSignature::build<int, int, int, int, int, int, int, int, int, int, int>());
    invoke_node->set_arg(0, a);
    invoke_node->set_arg(1, a);
    invoke_node->set_arg(2, a);
    invoke_node->set_arg(3, a);
    invoke_node->set_arg(4, a);
    invoke_node->set_arg(5, a);
    invoke_node->set_arg(6, a);
    invoke_node->set_arg(7, a);
    invoke_node->set_arg(8, a);
    invoke_node->set_arg(9, a);
    invoke_node->set_ret(0, a);

    cc.ret(a);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = called_fn(3, 3, 3, 3, 3, 3, 3, 3, 3, 3);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    // Prepare.
    x86::Gp rv = cc.new_gp32("rv");

    // Call function.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)X86Test_FuncCallManyArgs::called_fn),
      FuncSignature::build<int, int, int, int, int, int, int, int, int, int, int>());

    invoke_node->set_arg(0, imm(0x03));
    invoke_node->set_arg(1, imm(0x12));
    invoke_node->set_arg(2, imm(0xA0));
    invoke_node->set_arg(3, imm(0x0B));
    invoke_node->set_arg(4, imm(0x2F));
    invoke_node->set_arg(5, imm(0x02));
    invoke_node->set_arg(6, imm(0x0C));
    invoke_node->set_arg(7, imm(0x12));
    invoke_node->set_arg(8, imm(0x18));
    invoke_node->set_arg(9, imm(0x1E));
    invoke_node->set_ret(0, rv);

    cc.ret(rv);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = X86Test_FuncCallManyArgs::called_fn(0x03, 0x12, 0xA0, 0x0B, 0x2F, 0x02, 0x0C, 0x12, 0x18, 0x1E);

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  static int called_fn(void* a, void* b, void* c, void* d, void* e, void* f, void* g, void* h, void* i, void* j) {
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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    // Prepare.
    x86::Gp rv = cc.new_gp32("rv");

    // Call function.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)called_fn),
      FuncSignature::build<int, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*>());

    invoke_node->set_arg(0, imm(0x01));
    invoke_node->set_arg(1, imm(0x02));
    invoke_node->set_arg(2, imm(0x03));
    invoke_node->set_arg(3, imm(0x04));
    invoke_node->set_arg(4, imm(0x05));
    invoke_node->set_arg(5, imm(0x06));
    invoke_node->set_arg(6, imm(0x07));
    invoke_node->set_arg(7, imm(0x08));
    invoke_node->set_arg(8, imm(0x09));
    invoke_node->set_arg(9, imm(0x0A));
    invoke_node->set_ret(0, rv);

    cc.ret(rv);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = 55;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  static int called_fn(int& a, int& b, int& c, int& d) {
    a += a;
    b += b;
    c += c;
    d += d;
    return a + b + c + d;
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int&, int&, int&, int&>());

    // Prepare.
    x86::Gp arg1 = cc.new_gp32();
    x86::Gp arg2 = cc.new_gp32();
    x86::Gp arg3 = cc.new_gp32();
    x86::Gp arg4 = cc.new_gp32();
    x86::Gp rv = cc.new_gp32("rv");

    func_node->set_arg(0, arg1);
    func_node->set_arg(1, arg2);
    func_node->set_arg(2, arg3);
    func_node->set_arg(3, arg4);

    // Call function.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)called_fn),
      FuncSignature::build<int, int&, int&, int&, int&>());

    invoke_node->set_arg(0, arg1);
    invoke_node->set_arg(1, arg2);
    invoke_node->set_arg(2, arg3);
    invoke_node->set_arg(3, arg4);
    invoke_node->set_ret(0, rv);

    cc.ret(rv);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int&, int&, int&, int&);
    Func func = ptr_as_func<Func>(_func);

    int inputs[4] = { 1, 2, 3, 4 };
    int outputs[4] = { 2, 4, 6, 8 };
    int result_ret = func(inputs[0], inputs[1], inputs[2], inputs[3]);
    int expect_ret = 20;

    result.assign_format("ret={%08X %08X %08X %08X %08X}", result_ret, inputs[0], inputs[1], inputs[2], inputs[3]);
    expect.assign_format("ret={%08X %08X %08X %08X %08X}", expect_ret, outputs[0], outputs[1], outputs[2], outputs[3]);

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

  static float called_fn(float a, float b) {
    return a * b;
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<float, float, float>());

    x86::Vec a = cc.new_xmm_ss("a");
    x86::Vec b = cc.new_xmm_ss("b");
    x86::Vec ret = cc.new_xmm_ss("ret");

    func_node->set_arg(0, a);
    func_node->set_arg(1, b);

    // Call function.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), imm((void*)called_fn), FuncSignature::build<float, float, float>());
    invoke_node->set_arg(0, a);
    invoke_node->set_arg(1, b);
    invoke_node->set_ret(0, ret);

    cc.ret(ret);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = float (*)(float, float);
    Func func = ptr_as_func<Func>(_func);

    float result_ret = func(15.5f, 2.0f);
    float expect_ret = called_fn(15.5f, 2.0f);

    result.assign_format("ret=%g", double(result_ret));
    expect.assign_format("ret=%g", double(expect_ret));

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

  static double called_fn(double a, double b) {
    return a * b;
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double, double>());

    x86::Vec a = cc.new_xmm_sd("a");
    x86::Vec b = cc.new_xmm_sd("b");
    x86::Vec ret = cc.new_xmm_sd("ret");

    func_node->set_arg(0, a);
    func_node->set_arg(1, b);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), imm((void*)called_fn), FuncSignature::build<double, double, double>());
    invoke_node->set_arg(0, a);
    invoke_node->set_arg(1, b);
    invoke_node->set_ret(0, ret);

    cc.ret(ret);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double result_ret = func(15.5, 2.0);
    double expect_ret = called_fn(15.5, 2.0);

    result.assign_format("ret=%g", result_ret);
    expect.assign_format("ret=%g", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp x = cc.new_gp32("x");
    x86::Gp y = cc.new_gp32("y");
    x86::Gp op = cc.new_gp32("op");

    InvokeNode* invoke_node;
    x86::Gp result;

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int, int>());
    func_node->set_arg(0, x);
    func_node->set_arg(1, y);
    func_node->set_arg(2, op);

    Label op_add = cc.new_label();
    Label op_mul = cc.new_label();

    cc.cmp(op, 0);
    cc.jz(op_add);
    cc.cmp(op, 1);
    cc.jz(op_mul);

    result = cc.new_gp32("result_0");
    cc.mov(result, 0);
    cc.ret(result);

    cc.bind(op_add);
    result = cc.new_gp32("result_1");

    cc.invoke(Out(invoke_node), (uint64_t)called_fn_add, FuncSignature::build<int, int, int>());
    invoke_node->set_arg(0, x);
    invoke_node->set_arg(1, y);
    invoke_node->set_ret(0, result);
    cc.ret(result);

    cc.bind(op_mul);
    result = cc.new_gp32("result_2");

    cc.invoke(Out(invoke_node), (uint64_t)called_fn_mul, FuncSignature::build<int, int, int>());
    invoke_node->set_arg(0, x);
    invoke_node->set_arg(1, y);
    invoke_node->set_ret(0, result);

    cc.ret(result);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int arg1 = 4;
    int arg2 = 8;

    int result_add = func(arg1, arg2, 0);
    int expect_add = called_fn_add(arg1, arg2);

    int result_mul = func(arg1, arg2, 1);
    int expect_mul = called_fn_mul(arg1, arg2);

    result.assign_format("ret={add=%d, mul=%d}", result_add, result_mul);
    expect.assign_format("ret={add=%d, mul=%d}", expect_add, expect_mul);

    return (result_add == expect_add) && (result_mul == expect_mul);
  }

  static int called_fn_add(int x, int y) { return x + y; }
  static int called_fn_mul(int x, int y) { return x * y; }
};

// x86::Compiler - X86Test_FuncCallMultiple
// ========================================

class X86Test_FuncCallMultiple : public X86TestCase {
public:
  X86Test_FuncCallMultiple() : X86TestCase("FuncCallMultiple") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMultiple());
  }

  static int ASMJIT_FASTCALL called_fn(int* p_int, int index) {
    return p_int[index];
  }

  void compile(x86::Compiler& cc) override {
    unsigned int i;

    x86::Gp buf = cc.new_gp_ptr("buf");
    x86::Gp acc0 = cc.new_gp32("acc0");
    x86::Gp acc1 = cc.new_gp32("acc1");

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int*>());
    func_node->set_arg(0, buf);

    cc.mov(acc0, 0);
    cc.mov(acc1, 0);

    for (i = 0; i < 4; i++) {
      x86::Gp ret = cc.new_gp32("ret");
      x86::Gp ptr = cc.new_gp_ptr("ptr");
      x86::Gp idx = cc.new_gp32("idx");
      InvokeNode* invoke_node;

      cc.mov(ptr, buf);
      cc.mov(idx, int(i));

      cc.invoke(Out(invoke_node), (uint64_t)called_fn, FuncSignature::build<int, int*, int>(CallConvId::kFastCall));
      invoke_node->set_arg(0, ptr);
      invoke_node->set_arg(1, idx);
      invoke_node->set_ret(0, ret);

      cc.add(acc0, ret);

      cc.mov(ptr, buf);
      cc.mov(idx, int(i));

      cc.invoke(Out(invoke_node), (uint64_t)called_fn, FuncSignature::build<int, int*, int>(CallConvId::kFastCall));
      invoke_node->set_arg(0, ptr);
      invoke_node->set_arg(1, idx);
      invoke_node->set_ret(0, ret);

      cc.sub(acc1, ret);
    }

    cc.add(acc0, acc1);
    cc.ret(acc0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int*);
    Func func = ptr_as_func<Func>(_func);

    int buffer[4] = { 127, 87, 23, 17 };

    int result_ret = func(buffer);
    int expect_ret = 0;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    x86::Gp val = cc.new_gp32("val");
    Label skip = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int>());
    func_node->set_arg(0, val);

    cc.cmp(val, 1);
    cc.jle(skip);

    x86::Gp tmp = cc.new_gp32("tmp");
    cc.mov(tmp, val);
    cc.dec(tmp);

    InvokeNode* invoke_node;

    cc.invoke(Out(invoke_node), func_node->label(), FuncSignature::build<int, int>());
    invoke_node->set_arg(0, tmp);
    invoke_node->set_ret(0, tmp);
    cc.mul(cc.new_gp32(), val, tmp);

    cc.bind(skip);
    cc.ret(val);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(5);
    int expect_ret = 1 * 2 * 3 * 4 * 5;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int, int, int>());

    x86::Gp a0 = cc.new_gp32("a0");
    x86::Gp a1 = cc.new_gp32("a1");
    x86::Gp a2 = cc.new_gp32("a2");
    x86::Gp a3 = cc.new_gp32("a3");

    func_node->set_arg(0, a0);
    func_node->set_arg(1, a1);
    func_node->set_arg(2, a2);
    func_node->set_arg(3, a3);

    // We call `int func(size_t, ...)`
    //   - The `va_index` must be 1 (first argument after size_t).
    //   - The full signature of varargs (int, int, int, int) must follow.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)called_fn),
      FuncSignature::build<int, size_t, int, int, int, int>(CallConvId::kCDecl, 1));
    invoke_node->set_arg(0, imm(4));
    invoke_node->set_arg(1, a0);
    invoke_node->set_arg(2, a1);
    invoke_node->set_arg(3, a2);
    invoke_node->set_arg(4, a3);
    invoke_node->set_ret(0, a0);

    cc.ret(a0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int, int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(1, 2, 3, 4);
    int expect_ret = 1 + 2 + 3 + 4;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }

  static int called_fn(size_t n, ...) {
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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double, double, double, double>());

    x86::Vec a0 = cc.new_xmm_sd("a0");
    x86::Vec a1 = cc.new_xmm_sd("a1");
    x86::Vec a2 = cc.new_xmm_sd("a2");
    x86::Vec a3 = cc.new_xmm_sd("a3");

    func_node->set_arg(0, a0);
    func_node->set_arg(1, a1);
    func_node->set_arg(2, a2);
    func_node->set_arg(3, a3);

    // We call `double func(size_t, ...)`
    //   - The `va_index` must be 1 (first argument after size_t).
    //   - The full signature of varargs (double, double, double, double) must follow.
    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)called_fn),
      FuncSignature::build<double, size_t, double, double, double, double>(CallConvId::kCDecl, 1));
    invoke_node->set_arg(0, imm(4));
    invoke_node->set_arg(1, a0);
    invoke_node->set_arg(2, a1);
    invoke_node->set_arg(3, a2);
    invoke_node->set_arg(4, a3);
    invoke_node->set_ret(0, a0);

    cc.ret(a0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double, double, double, double);
    Func func = ptr_as_func<Func>(_func);

    double result_ret = func(1.0, 2.0, 3.0, 4.0);
    double expect_ret = 1.0 + 2.0 + 3.0 + 4.0;

    result.assign_format("ret=%f", result_ret);
    expect.assign_format("ret=%f", expect_ret);

    return result == expect;
  }

  static double called_fn(size_t n, ...) {
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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<uint64_t, uint64_t>());

    if (cc.is_64bit()) {
      x86::Gp reg = cc.new_gp64();
      func_node->set_arg(0, reg);
      cc.add(reg, 1);
      cc.ret(reg);
    }
    else {
      x86::Gp hi = cc.new_gp32("hi");
      x86::Gp lo = cc.new_gp32("lo");

      func_node->set_arg(0, 0, lo);
      func_node->set_arg(0, 1, hi);

      cc.add(lo, 1);
      cc.adc(hi, 0);
      cc.ret(lo, hi);
    }

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint64_t (*)(uint64_t);
    Func func = ptr_as_func<Func>(_func);

    uint64_t result_ret = func(uint64_t(0xFFFFFFFF));
    uint64_t expect_ret = 0x100000000;

    result.assign_format("ret=%llu", (unsigned long long)result_ret);
    expect.assign_format("ret=%llu", (unsigned long long)expect_ret);

    return result == expect;
  }

  static double called_fn(size_t n, ...) {
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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int>());

    x86::Gp a = cc.new_gp32("a");
    x86::Gp b = cc.new_gp32("b");
    x86::Gp r = cc.new_gp32("r");

    func_node->set_arg(0, a);
    func_node->set_arg(1, b);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)dummy),
      FuncSignature::build<void, int, int>());
    invoke_node->set_arg(0, a);
    invoke_node->set_arg(1, b);

    cc.lea(r, x86::ptr(a, b));
    cc.ret(r);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(44, 199);
    int expect_ret = 243;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, const double*>());

    x86::Gp p = cc.new_gp_ptr("p");
    x86::Vec arg = cc.new_xmm_sd("arg");
    x86::Vec ret = cc.new_xmm_sd("ret");

    func_node->set_arg(0, p);
    cc.movsd(arg, x86::ptr(p));

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)op),
      FuncSignature::build<double, double>());
    invoke_node->set_arg(0, arg);
    invoke_node->set_ret(0, ret);

    cc.ret(ret);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(const double*);
    Func func = ptr_as_func<Func>(_func);

    double arg = 2;

    double result_ret = func(&arg);
    double expect_ret = op(arg);

    result.assign_format("ret=%g", result_ret);
    expect.assign_format("ret=%g", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, const double*>());

    x86::Gp p = cc.new_gp_ptr("p");
    x86::Vec arg = cc.new_xmm_sd("arg");
    x86::Vec ret = cc.new_xmm_sd("ret");

    func_node->set_arg(0, p);
    cc.movsd(arg, x86::ptr(p));

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node),
      imm((void*)op),
      FuncSignature::build<double, double>());
    invoke_node->set_arg(0, arg);
    invoke_node->set_ret(0, ret);

    cc.xorps(arg, arg);
    cc.subsd(arg, ret);

    cc.ret(arg);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(const double*);
    Func func = ptr_as_func<Func>(_func);

    double arg = 2;

    double result_ret = func(&arg);
    double expect_ret = -op(arg);

    result.assign_format("ret=%g", result_ret);
    expect.assign_format("ret=%g", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    InvokeNode* invoke_node;

    FuncSignature func_signature;
    func_signature.set_call_conv_id(CallConvId::kCDecl);
    func_signature.set_ret(TypeId::kFloat64);
    cc.add_func(func_signature);

    FuncSignature invoke_signature;
    invoke_signature.set_call_conv_id(CallConvId::kCDecl);
    invoke_signature.set_ret(TypeId::kFloat64);

    cc.invoke(Out(invoke_node), imm((void*)called_fn), invoke_signature);
    x86::Vec ret = cc.new_xmm_sd("ret");
    invoke_node->set_ret(0, ret);
    cc.ret(ret);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(void);
    Func func = ptr_as_func<Func>(_func);

    double result_ret = func();
    double expect_ret = 3.14;

    result.assign_format("ret=%g", result_ret);
    expect.assign_format("ret=%g", expect_ret);

    return result == expect;
  }

  static double called_fn() { return 3.14; }
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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    x86::Gp p_fn = cc.new_gp_ptr("p_fn");
    x86::Gp vars[16];

    uint32_t i, reg_count = cc.arch() == Arch::kX86 ? 8 : 16;
    ASMJIT_ASSERT(reg_count <= ASMJIT_ARRAY_SIZE(vars));

    cc.mov(p_fn, imm((void*)called_fn));

    for (i = 0; i < reg_count; i++) {
      if (i == x86::Gp::kIdBp || i == x86::Gp::kIdSp)
        continue;

      vars[i] = cc.new_gp32("%%%u", unsigned(i));
      cc.mov(vars[i], 1);
    }

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), p_fn, FuncSignature::build<void>());

    for (i = 1; i < reg_count; i++)
      if (vars[i].is_valid())
        cc.add(vars[0], vars[i]);
    cc.ret(vars[0]);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = sizeof(void*) == 4 ? 6 : 14;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }

  static void called_fn() {}
};

// x86::Compiler - X86Test_FuncCallMisc6
// =====================================

class X86Test_FuncCallMisc6 : public X86TestCase {
public:
  X86Test_FuncCallMisc6() : X86TestCase("FuncCallMisc6") {}

  static void add(TestApp& app) {
    app.add(new X86Test_FuncCallMisc6());
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<uint32_t, uint32_t>());

    constexpr uint32_t kCount = 16;

    x86::Gp v[kCount];
    x86::Gp arg_val = cc.new_gp32("arg_val");
    x86::Gp ret_val = cc.new_gp32("ret_val");

    func_node->set_arg(0, arg_val);
    cc.add(arg_val, 1);

    for (uint32_t i = 0; i < kCount; i++) {
      v[i] = cc.new_gp32("v%u", i);
    }

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), imm((void*)called_fn), FuncSignature::build<uint32_t, uint32_t>());
    invoke_node->set_arg(0, arg_val);
    invoke_node->set_ret(0, ret_val);

    for (uint32_t i = 0; i < kCount; i++) {
      cc.mov(v[i], i + 1);
    }

    for (uint32_t i = 0; i < kCount; i++) {
      cc.add(arg_val, v[i]);
    }

    cc.add(ret_val, arg_val);
    cc.ret(ret_val);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint32_t x);
    Func func = ptr_as_func<Func>(_func);

    uint32_t result_ret = func(111);
    uint32_t expect_ret = 111 + 112 + 2 + (1 + 16) * 8;

    result.assign_format("ret=%u", result_ret);
    expect.assign_format("ret=%u", expect_ret);

    return result == expect;
  }

  static uint32_t called_fn(uint32_t x) { return x + 1; }
};

// x86::Compiler - X86Test_FuncCallAVXClobber
// ==========================================

class X86Test_FuncCallAVXClobber : public X86TestCase {
public:
  X86Test_FuncCallAVXClobber() : X86TestCase("FuncCallAVXClobber") {}

  static void add(TestApp& app) {
    const CpuInfo& cpu_info = CpuInfo::host();

    if (cpu_info.features().x86().has_avx2() && sizeof(void*) == 8) {
      app.add(new X86Test_FuncCallAVXClobber());
    }
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* main_func = cc.add_func(FuncSignature::build<void, void*, const void*, const void*>());
    main_func->frame().set_avx_enabled();
    main_func->frame().set_avx_cleanup();

    // We need a Windows calling convention to test this properly also on a non-Windows machine.
    FuncNode* helper_func = cc.new_func(FuncSignature::build<void, void*, const void*>(CallConvId::kX64Windows));
    helper_func->frame().set_avx_enabled();
    helper_func->frame().set_avx_cleanup();

    {
      size_t i;

      x86::Gp d_ptr = cc.new_gp_ptr("d_ptr");
      x86::Gp a_ptr = cc.new_gp_ptr("a_ptr");
      x86::Gp b_ptr = cc.new_gp_ptr("b_ptr");
      x86::Gp t_ptr = cc.new_gp_ptr("t_ptr");
      x86::Vec acc[8];
      x86::Mem stack = cc.new_stack(32, 1, "stack");

      main_func->set_arg(0, d_ptr);
      main_func->set_arg(1, a_ptr);
      main_func->set_arg(2, b_ptr);

      cc.lea(t_ptr, stack);
      for (i = 0; i < 8; i++) {
        acc[i] = cc.new_ymm("acc%zu", i);
        cc.vmovdqu(acc[i], x86::ptr(a_ptr));
      }

      InvokeNode* invoke_node;
      cc.invoke(Out(invoke_node),
        helper_func->label(),
        FuncSignature::build<void, void*, const void*>(CallConvId::kX64Windows));
      invoke_node->set_arg(0, t_ptr);
      invoke_node->set_arg(1, b_ptr);

      for (i = 1; i < 8; i++) {
        cc.vpaddd(acc[0], acc[0], acc[i]);
      }

      cc.vpaddd(acc[0], acc[0], x86::ptr(t_ptr));
      cc.vmovdqu(x86::ptr(d_ptr), acc[0]);

      cc.end_func();
    }

    {
      cc.add_func(helper_func);

      x86::Gp d_ptr = cc.new_gp_ptr("d_ptr");
      x86::Gp a_ptr = cc.new_gp_ptr("a_ptr");

      helper_func->set_arg(0, d_ptr);
      helper_func->set_arg(1, a_ptr);

      x86::Gp tmp = cc.new_gp_ptr("tmp");
      x86::Vec acc = cc.new_ymm("acc");

      cc.mov(tmp, 1);
      cc.vmovd(acc.xmm(), tmp);
      cc.vpbroadcastd(acc, acc.xmm());
      cc.vpaddd(acc, acc, x86::ptr(a_ptr));
      cc.vmovdqu(x86::ptr(d_ptr), acc);

      cc.end_func();
    }
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*, const void*);
    Func func = ptr_as_func<Func>(_func);

    size_t i;

    static const uint32_t a_data[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    static const uint32_t b_data[8] = { 6, 3, 5, 9, 1, 8, 7, 2 };

    uint32_t result_data[8] {};
    uint32_t expect_data[8] {};

    for (i = 0; i < 8; i++)
      expect_data[i] = a_data[i] * 8 + b_data[i] + 1;

    func(result_data, a_data, b_data);

    result.assign("{");
    expect.assign("{");

    for (i = 0; i < 8; i++) {
      result.append_format("%u", result_data[i]);
      expect.append_format("%u", expect_data[i]);

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
  static inline constexpr uint32_t kVecCount = 64;

  X86Test_VecToScalar() : X86TestCase("VecToScalar") {}

  static void add(TestApp& app) {
    app.add(new X86Test_VecToScalar());
  }

  void compile(x86::Compiler& cc) override {
    FuncNode* func = cc.add_func(FuncSignature::build<uint32_t, uint32_t>());

    x86::Gp x = cc.new_gp32("x");
    x86::Gp t = cc.new_gp32("t");
    x86::Vec v[kVecCount];

    func->set_arg(0, x);

    for (size_t i = 0; i < kVecCount; i++) {
      v[i] = cc.new_xmm("v%d", i);
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
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t result_ret = func(1);
    uint32_t expect_ret = 2080; // 1 + 2 + 3 + ... + 64

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    x86::Gp v0 = cc.new_gp32("v0");
    x86::Gp v1 = cc.new_gp32("v1");

    x86::Mem c0 = cc.new_int32_const(ConstPoolScope::kLocal, 200);
    x86::Mem c1 = cc.new_int32_const(ConstPoolScope::kLocal, 33);

    cc.mov(v0, c0);
    cc.mov(v1, c1);
    cc.add(v0, v1);

    cc.ret(v0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = 233;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    x86::Gp v0 = cc.new_gp32("v0");
    x86::Gp v1 = cc.new_gp32("v1");

    x86::Mem c0 = cc.new_int32_const(ConstPoolScope::kGlobal, 200);
    x86::Mem c1 = cc.new_int32_const(ConstPoolScope::kGlobal, 33);

    cc.mov(v0, c0);
    cc.mov(v1, c1);
    cc.add(v0, v1);

    cc.ret(v0);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = func();
    int expect_ret = 233;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

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

  void compile(x86::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, int, int>());

    x86::Gp op = cc.new_gp32("op");
    x86::Gp a = cc.new_gp32("a");
    x86::Gp b = cc.new_gp32("b");

    Label L_Zero = cc.new_label();
    Label L_Add = cc.new_label();
    Label L_Sub = cc.new_label();
    Label L_Mul = cc.new_label();
    Label L_Div = cc.new_label();

    func_node->set_arg(0, op);
    func_node->set_arg(1, a);
    func_node->set_arg(2, b);

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

    x86::Gp zero = cc.new_gp32("zero");
    cc.xor_(zero, zero);
    cc.idiv(zero, a, b);
    cc.ret(a);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int, int);

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

    result.assign_format("ret={%d %d %d %d}", r0, r1, r2, r3);
    expect.assign_format("ret={%d %d %d %d}", e0, e1, e2, e3);

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

  void compile(x86::Compiler& cc) override {
    FuncNode* f1_node = cc.new_func(FuncSignature::build<int, int, int>());
    FuncNode* f2_node = cc.new_func(FuncSignature::build<int, int, int>());

    {
      x86::Gp a = cc.new_gp32("a");
      x86::Gp b = cc.new_gp32("b");

      cc.add_func(f1_node);
      f1_node->set_arg(0, a);
      f1_node->set_arg(1, b);

      InvokeNode* invoke_node;
      cc.invoke(Out(invoke_node), f2_node->label(), FuncSignature::build<int, int, int>());
      invoke_node->set_arg(0, a);
      invoke_node->set_arg(1, b);
      invoke_node->set_ret(0, a);

      cc.ret(a);
      cc.end_func();
    }

    {
      x86::Gp a = cc.new_gp32("a");
      x86::Gp b = cc.new_gp32("b");

      cc.add_func(f2_node);
      f2_node->set_arg(0, a);
      f2_node->set_arg(1, b);

      cc.add(a, b);
      cc.ret(a);
      cc.end_func();
    }
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(int, int);

    Func func = ptr_as_func<Func>(_func);

    int result_ret = func(56, 22);
    int expect_ret = 56 + 22;

    result.assign_format("ret=%d", result_ret);
    expect.assign_format("ret=%d", expect_ret);

    return result == expect;
  }
};

// x86::Compiler - X86Test_MiscUnfollow
// ====================================

// Global (I didn't find a better way to test this).
static jmp_buf global_jmp_buf;

class X86Test_MiscUnfollow : public X86TestCase {
public:
  X86Test_MiscUnfollow() : X86TestCase("MiscUnfollow") {}

  static void add(TestApp& app) {
    app.add(new X86Test_MiscUnfollow());
  }

  void compile(x86::Compiler& cc) override {
    // NOTE: Fastcall calling convention is the most appropriate here as all arguments are passed via registers and
    // there won't be any stack misalignment in the `handler()`. This was failing on MacOS when targeting 32-bit mode.
    x86::Gp a = cc.new_gp32("a");
    x86::Gp b = cc.new_gp_ptr("b");
    Label tramp = cc.new_label();

    FuncNode* func_node = cc.add_func(FuncSignature::build<int, int, void*>(CallConvId::kFastCall));
    func_node->set_arg(0, a);
    func_node->set_arg(1, b);

    cc.cmp(a, 0);
    cc.jz(tramp);
    cc.ret(a);
    cc.bind(tramp);
    cc.unfollow().jmp(b);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (ASMJIT_FASTCALL*)(int, void*);
    Func func = ptr_as_func<Func>(_func);

    int result_ret = 0;
    int expect_ret = 1;

    if (!setjmp(global_jmp_buf))
      result_ret = func(0, (void*)handler);
    else
      result_ret = 1;

    result.assign_format("ret={%d}", result_ret);
    expect.assign_format("ret={%d}", expect_ret);

    return result == expect;
  }

  static void ASMJIT_FASTCALL handler() { longjmp(global_jmp_buf, 1); }
};

// x86::Compiler - Tests
// =====================

void compiler_add_x86_tests(TestApp& app) {
  // Base tests.
  app.add_t<X86Test_NoCode>();
  app.add_t<X86Test_NoAlign>();
  app.add_t<X86Test_IndirectBranchProtection>();
  app.add_t<X86Test_AlignBase>();

  // Jump tests.
  app.add_t<X86Test_JumpMerge>();
  app.add_t<X86Test_JumpCross>();
  app.add_t<X86Test_JumpMany>();
  app.add_t<X86Test_JumpUnreachable1>();
  app.add_t<X86Test_JumpUnreachable2>();
  app.add_t<X86Test_JumpTable1>();
  app.add_t<X86Test_JumpTable2>();
  app.add_t<X86Test_JumpTable3>();
  app.add_t<X86Test_JumpTable4>();

  // Alloc and instruction tests.
  app.add_t<X86Test_AllocBase>();
  app.add_t<X86Test_AllocMany1>();
  app.add_t<X86Test_AllocMany2>();
  app.add_t<X86Test_AllocInt8>();
  app.add_t<X86Test_AllocUnhandledArg>();
  app.add_t<X86Test_AllocArgsIntPtr>();
  app.add_t<X86Test_AllocArgsFloat>();
  app.add_t<X86Test_AllocArgsDouble>();
#if ASMJIT_ARCH_X86
  app.add_t<X86Test_AllocArgsVec>();
#endif
  app.add_t<X86Test_AllocRetFloat1>();
  app.add_t<X86Test_AllocRetFloat2>();
  app.add_t<X86Test_AllocRetDouble1>();
  app.add_t<X86Test_AllocRetDouble2>();
  app.add_t<X86Test_AllocStack>();
  app.add_t<X86Test_Imul1>();
  app.add_t<X86Test_Imul2>();
  app.add_t<X86Test_Idiv1>();
  app.add_t<X86Test_Setz>();
  app.add_t<X86Test_ShlRor>();
  app.add_t<X86Test_GpbLo1>();
  app.add_t<X86Test_GpbLo2>();
  app.add_t<X86Test_RepMovsb>();
  app.add_t<X86Test_IfElse1>();
  app.add_t<X86Test_IfElse2>();
  app.add_t<X86Test_IfElse3>();
  app.add_t<X86Test_IfElse4>();
  app.add_t<X86Test_Memcpy>();
  app.add_t<X86Test_ExtraBlock>();
  app.add_t<X86Test_AlphaBlend>();
  app.add_t<X86Test_AVX512_KK>();
  app.add_t<X86Test_AVX512_TernLog>();

  // Function arguments handling tests.
  app.add_t<X86Test_FuncArgInt8>();

  // Function call tests.
  app.add_t<X86Test_FuncCallBase1>();
  app.add_t<X86Test_FuncCallBase2>();
  app.add_t<X86Test_FuncCallStd>();
  app.add_t<X86Test_FuncCallFast>();
#if ASMJIT_ARCH_X86
  app.add_t<X86Test_FuncCallSIMD>();
#endif
  app.add_t<X86Test_FuncCallLight>();
  app.add_t<X86Test_FuncCallManyArgs>();
  app.add_t<X86Test_FuncCallDuplicateArgs>();
  app.add_t<X86Test_FuncCallImmArgs>();
  app.add_t<X86Test_FuncCallPtrArgs>();
  app.add_t<X86Test_FuncCallRefArgs>();
  app.add_t<X86Test_FuncCallFloatAsXmmRet>();
  app.add_t<X86Test_FuncCallDoubleAsXmmRet>();
  app.add_t<X86Test_FuncCallConditional>();
  app.add_t<X86Test_FuncCallMultiple>();
  app.add_t<X86Test_FuncCallRecursive>();
  app.add_t<X86Test_FuncCallVarArg1>();
  app.add_t<X86Test_FuncCallVarArg2>();
  app.add_t<X86Test_FuncCallInt64Arg>();
  app.add_t<X86Test_FuncCallMisc1>();
  app.add_t<X86Test_FuncCallMisc2>();
  app.add_t<X86Test_FuncCallMisc3>();
  app.add_t<X86Test_FuncCallMisc4>();
  app.add_t<X86Test_FuncCallMisc5>();
  app.add_t<X86Test_FuncCallMisc6>();
  app.add_t<X86Test_FuncCallAVXClobber>();

  // Miscellaneous tests.
  app.add_t<X86Test_VecToScalar>();
  app.add_t<X86Test_MiscLocalConstPool>();
  app.add_t<X86Test_MiscGlobalConstPool>();
  app.add_t<X86Test_MiscMultiRet>();
  app.add_t<X86Test_MiscMultiFunc>();
  app.add_t<X86Test_MiscUnfollow>();
}

#endif // !ASMJIT_NO_X86 && !ASMJIT_NO_COMPILER
