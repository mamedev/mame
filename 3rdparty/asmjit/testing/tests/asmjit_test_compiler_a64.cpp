// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>
#if !defined(ASMJIT_NO_COMPILER) && !defined(ASMJIT_NO_AARCH64)

#include <asmjit/a64.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asmjit_test_compiler.h"

using namespace asmjit;

// a64::Compiler - A64TestCase
// ===========================

class A64TestCase : public TestCase {
public:
  A64TestCase(const char* name = nullptr)
    : TestCase(name, Arch::kAArch64) {}

  void compile(BaseCompiler& cc) override {
    compile(static_cast<a64::Compiler&>(cc));
  }

  virtual void compile(a64::Compiler& cc) = 0;
};

// a64::Compiler - A64Test_GpArgs
// ==============================

class A64Test_GpArgs : public A64TestCase {
public:
  uint32_t _arg_count;
  bool _preserve_fp;

  A64Test_GpArgs(uint32_t arg_count, bool preserve_fp)
    : _arg_count(arg_count),
      _preserve_fp(preserve_fp) {
    _name.assign_format("GpArgs {NumArgs=%u PreserveFP=%c}", arg_count, preserve_fp ? 'Y' : 'N');
  }

  static void add(TestApp& app) {
    for (uint32_t i = 0; i <= 16; i++) {
      app.add(new A64Test_GpArgs(i, true));
      app.add(new A64Test_GpArgs(i, false));
    }
  }

  void compile(a64::Compiler& cc) override {
    uint32_t arg_count = _arg_count;
    FuncSignature signature;

    signature.set_ret_t<int>();
    for (uint32_t i = 0; i < arg_count; i++) {
      signature.add_arg_t<int>();
    }

    FuncNode* func_node = cc.add_func(signature);
    if (_preserve_fp)
      func_node->frame().set_preserved_fp();

    a64::Gp sum;

    if (arg_count) {
      for (uint32_t i = 0; i < arg_count; i++) {
        a64::Gp i_reg = cc.new_gp32("i%u", i);
        func_node->set_arg(i, i_reg);

        if (i == 0)
          sum = i_reg;
        else
          cc.add(sum, sum, i_reg);
      }
    }
    else {
      sum = cc.new_gp32("i");
      cc.mov(sum, 0);
    }

    cc.ret(sum);
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
};

// a64::Compiler - A64Test_Simd1
// =============================

class A64Test_Simd1 : public A64TestCase {
public:
  A64Test_Simd1()
    : A64TestCase("Simd1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Simd1());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, const void*, const void*>());

    a64::Gp dst = cc.new_gp_ptr("dst");
    a64::Gp src1 = cc.new_gp_ptr("src1");
    a64::Gp src2 = cc.new_gp_ptr("src2");

    func_node->set_arg(0, dst);
    func_node->set_arg(1, src1);
    func_node->set_arg(2, src2);

    a64::Vec v1 = cc.new_vec_q("vec1");
    a64::Vec v2 = cc.new_vec_q("vec2");
    a64::Vec v3 = cc.new_vec_q("vec3");

    cc.ldr(v2, a64::ptr(src1));
    cc.ldr(v3, a64::ptr(src2));
    cc.add(v1.b16(), v2.b16(), v3.b16());
    cc.str(v1, a64::ptr(dst));

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void*, const void*, const void*);

    uint32_t dst[4];
    uint32_t a_src[4] = { 0 , 1 , 2 , 255 };
    uint32_t b_src[4] = { 99, 17, 33, 1   };

    // NOTE: It's a byte-add, so uint8_t(255+1) == 0.
    uint32_t ref[4] = { 99, 18, 35, 0 };

    ptr_as_func<Func>(_func)(dst, a_src, b_src);

    result.assign_format("ret={%u, %u, %u, %u}", dst[0], dst[1], dst[2], dst[3]);
    expect.assign_format("ret={%u, %u, %u, %u}", ref[0], ref[1], ref[2], ref[3]);

    return result == expect;
  }
};

// a64::Compiler - A64Test_ManyRegs
// ================================

class A64Test_ManyRegs : public A64TestCase {
public:
  uint32_t _reg_count;

  A64Test_ManyRegs(uint32_t n)
    : A64TestCase(),
      _reg_count(n) {
    _name.assign_format("GpRegs {NumRegs=%u}", n);
  }

  static void add(TestApp& app) {
    for (uint32_t i = 2; i < 64; i++)
      app.add(new A64Test_ManyRegs(i));
  }

  void compile(a64::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    a64::Gp* regs = static_cast<a64::Gp*>(malloc(_reg_count * sizeof(a64::Gp)));

    for (uint32_t i = 0; i < _reg_count; i++) {
      regs[i] = cc.new_gp32("reg%u", i);
      cc.mov(regs[i], i + 1);
    }

    a64::Gp sum = cc.new_gp32("sum");
    cc.mov(sum, 0);

    for (uint32_t i = 0; i < _reg_count; i++) {
      cc.add(sum, sum, regs[i]);
    }

    cc.ret(sum);
    cc.end_func();

    free(regs);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assign_format("ret={%d}", func());
    expect.assign_format("ret={%d}", calc_sum());

    return result == expect;
  }

  uint32_t calc_sum() const {
    return (_reg_count | 1) * ((_reg_count + 1) / 2);
  }
};

// a64::Compiler - A64Test_Adr
// ===========================

class A64Test_Adr : public A64TestCase {
public:
  A64Test_Adr()
    : A64TestCase("Adr") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Adr());
  }

  void compile(a64::Compiler& cc) override {
    cc.add_func(FuncSignature::build<int>());

    a64::Gp addr = cc.new_gp_ptr("addr");
    a64::Gp val = cc.new_gp_ptr("val");

    Label L_Table = cc.new_label();

    cc.adr(addr, L_Table);
    cc.ldrsw(val, a64::ptr(addr, 8));
    cc.ret(val);
    cc.end_func();

    cc.bind(L_Table);
    cc.embed_int32(1);
    cc.embed_int32(2);
    cc.embed_int32(3);
    cc.embed_int32(4);
    cc.embed_int32(5);
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = int (*)(void);
    Func func = ptr_as_func<Func>(_func);

    result.assign_format("ret={%d}", func());
    expect.assign_format("ret={%d}", 3);

    return result == expect;
  }
};

// a64::Compiler - A64Test_Branch1
// ===============================

class A64Test_Branch1 : public A64TestCase {
public:
  A64Test_Branch1()
    : A64TestCase("Branch1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Branch1());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, size_t>());

    a64::Gp p = cc.new_gp_ptr("p");
    a64::Gp count = cc.new_gp_ptr("count");
    a64::Gp i = cc.new_gp_ptr("i");
    Label L = cc.new_label();

    func_node->set_arg(0, p);
    func_node->set_arg(1, count);

    cc.mov(i, 0);

    cc.bind(L);
    cc.strb(i.w(), a64::ptr(p, i));
    cc.add(i, i, 1);
    cc.cmp(i, count);
    cc.b_ne(L);

    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = void (*)(void* p, size_t n);
    Func func = ptr_as_func<Func>(_func);

    uint8_t array[16];
    func(array, 16);

    expect.assign("ret={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}");

    result.assign("ret={");
    for (size_t i = 0; i < 16; i++) {
      if (i)
        result.append(", ");
      result.append_format("%d", int(array[i]));
    }
    result.append("}");

    return result == expect;
  }
};

// a64::Compiler - A64Test_Invoke1
// ===============================

class A64Test_Invoke1 : public A64TestCase {
public:
  A64Test_Invoke1()
    : A64TestCase("Invoke1") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke1());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<uint32_t, uint32_t, uint32_t>());

    a64::Gp x = cc.new_gp32("x");
    a64::Gp y = cc.new_gp32("y");
    a64::Gp r = cc.new_gp32("r");
    a64::Gp fn = cc.new_gp_ptr("fn");

    func_node->set_arg(0, x);
    func_node->set_arg(1, y);

    cc.mov(fn, (uint64_t)called_fn);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), fn, FuncSignature::build<uint32_t, uint32_t, uint32_t>());
    invoke_node->set_arg(0, x);
    invoke_node->set_arg(1, y);
    invoke_node->set_ret(0, r);

    cc.ret(r);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = uint32_t (*)(uint32_t, uint32_t);
    Func func = ptr_as_func<Func>(_func);

    uint32_t x = 49;
    uint32_t y = 7;

    result.assign_format("ret={%u}", func(x, y));
    expect.assign_format("ret={%u}", x - y);

    return result == expect;
  }

  static uint32_t called_fn(uint32_t x, uint32_t y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_Invoke2
// ===============================

class A64Test_Invoke2 : public A64TestCase {
public:
  A64Test_Invoke2()
    : A64TestCase("Invoke2") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke2());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double, double>());

    a64::Vec x = cc.new_vec_d("x");
    a64::Vec y = cc.new_vec_d("y");
    a64::Vec r = cc.new_vec_d("r");
    a64::Gp fn = cc.new_gp_ptr("fn");

    func_node->set_arg(0, x);
    func_node->set_arg(1, y);
    cc.mov(fn, (uint64_t)called_fn);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), fn, FuncSignature::build<double, double, double>());
    invoke_node->set_arg(0, x);
    invoke_node->set_arg(1, y);
    invoke_node->set_ret(0, r);

    cc.ret(r);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double x = 49;
    double y = 7;

    result.assign_format("ret={%f}", func(x, y));
    expect.assign_format("ret={%f}", called_fn(x, y));

    return result == expect;
  }

  static double called_fn(double x, double y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_Invoke3
// ===============================

class A64Test_Invoke3 : public A64TestCase {
public:
  A64Test_Invoke3()
    : A64TestCase("Invoke3") {}

  static void add(TestApp& app) {
    app.add(new A64Test_Invoke3());
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<double, double, double>());

    a64::Vec x = cc.new_vec_d("x");
    a64::Vec y = cc.new_vec_d("y");
    a64::Vec r = cc.new_vec_d("r");
    a64::Gp fn = cc.new_gp_ptr("fn");

    func_node->set_arg(0, x);
    func_node->set_arg(1, y);
    cc.mov(fn, (uint64_t)called_fn);

    InvokeNode* invoke_node;
    cc.invoke(Out(invoke_node), fn, FuncSignature::build<double, double, double>());
    invoke_node->set_arg(0, y);
    invoke_node->set_arg(1, x);
    invoke_node->set_ret(0, r);

    cc.ret(r);
    cc.end_func();
  }

  bool run(void* _func, String& result, String& expect) override {
    using Func = double (*)(double, double);
    Func func = ptr_as_func<Func>(_func);

    double x = 49;
    double y = 7;

    result.assign_format("ret={%f}", func(x, y));
    expect.assign_format("ret={%f}", called_fn(y, x));

    return result == expect;
  }

  static double called_fn(double x, double y) {
    return x - y;
  }
};

// a64::Compiler - A64Test_JumpTable
// =================================

class A64Test_JumpTable : public A64TestCase {
public:
  bool _annotated;

  A64Test_JumpTable(bool annotated)
    : A64TestCase("A64Test_JumpTable"),
      _annotated(annotated) {
    _name.assign_format("JumpTable {%s}", annotated ? "Annotated" : "Unknown Target");
  }

  enum Operator {
    kOperatorAdd = 0,
    kOperatorSub = 1,
    kOperatorMul = 2,
    kOperatorDiv = 3
  };

  static void add(TestApp& app) {
    app.add(new A64Test_JumpTable(false));
    app.add(new A64Test_JumpTable(true));
  }

  void compile(a64::Compiler& cc) override {
    FuncNode* func_node = cc.add_func(FuncSignature::build<float, float, float, uint32_t>());

    a64::Vec a = cc.new_vec_s("a");
    a64::Vec b = cc.new_vec_s("b");
    a64::Gp op = cc.new_gp32("op");

    a64::Gp target = cc.new_gp_ptr("target");
    a64::Gp offset = cc.new_gp_ptr("offset");

    Label L_End = cc.new_label();

    Label L_Table = cc.new_label();
    Label L_Add = cc.new_label();
    Label L_Sub = cc.new_label();
    Label L_Mul = cc.new_label();
    Label L_Div = cc.new_label();

    func_node->set_arg(0, a);
    func_node->set_arg(1, b);
    func_node->set_arg(2, op);

    cc.adr(target, L_Table);
    cc.ldrsw(offset, a64::ptr(target, op, a64::sxtw(2)));
    cc.add(target, target, offset);

    // JumpAnnotation allows to annotate all possible jump targets of
    // instructions where it cannot be deduced from operands.
    if (_annotated) {
      JumpAnnotation* annotation = cc.new_jump_annotation();
      annotation->add_label(L_Add);
      annotation->add_label(L_Sub);
      annotation->add_label(L_Mul);
      annotation->add_label(L_Div);
      cc.br(target, annotation);
    }
    else {
      cc.br(target);
    }

    cc.bind(L_Add);
    cc.fadd(a, a, b);
    cc.b(L_End);

    cc.bind(L_Sub);
    cc.fsub(a, a, b);
    cc.b(L_End);

    cc.bind(L_Mul);
    cc.fmul(a, a, b);
    cc.b(L_End);

    cc.bind(L_Div);
    cc.fdiv(a, a, b);

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

    float dst[4];
    float ref[4];

    dst[0] = func(33.0f, 14.0f, kOperatorAdd);
    dst[1] = func(33.0f, 14.0f, kOperatorSub);
    dst[2] = func(10.0f, 6.0f, kOperatorMul);
    dst[3] = func(80.0f, 8.0f, kOperatorDiv);

    ref[0] = 47.0f;
    ref[1] = 19.0f;
    ref[2] = 60.0f;
    ref[3] = 10.0f;

    result.assign_format("ret={%f, %f, %f, %f}", double(dst[0]), double(dst[1]), double(dst[2]), double(dst[3]));
    expect.assign_format("ret={%f, %f, %f, %f}", double(ref[0]), double(ref[1]), double(ref[2]), double(ref[3]));

    return result == expect;
  }
};

// a64::Compiler - Export
// ======================

void compiler_add_a64_tests(TestApp& app) {
  app.add_t<A64Test_GpArgs>();
  app.add_t<A64Test_ManyRegs>();
  app.add_t<A64Test_Simd1>();
  app.add_t<A64Test_Adr>();
  app.add_t<A64Test_Branch1>();
  app.add_t<A64Test_Invoke1>();
  app.add_t<A64Test_Invoke2>();
  app.add_t<A64Test_Invoke3>();
  app.add_t<A64Test_JumpTable>();
}

#endif // !ASMJIT_NO_COMPILER && !ASMJIT_NO_AARCH64
