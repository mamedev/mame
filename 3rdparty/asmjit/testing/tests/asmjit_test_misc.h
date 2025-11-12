// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#ifndef ASMJIT_TEST_MISC_H_INCLUDED
#define ASMJIT_TEST_MISC_H_INCLUDED

#include <asmjit/x86.h>

namespace asmtest {

using namespace asmjit;

// Generates a typical alpha blend function that uses SSE2 instruction set.
// This function combines emitting instructions with control flow constructs
// like binding Labels and jumping to them. This should be pretty representative.
template<typename Emitter>
static void generate_sse_alpha_blend_internal(
  Emitter& cc,
  const x86::Gp& dst, const x86::Gp& src, const x86::Gp& n,
  const x86::Gp& gp0,
  const x86::Vec& simd0, const x86::Vec& simd1, const x86::Vec& simd2, const x86::Vec& simd3,
  const x86::Vec& simd4, const x86::Vec& simd5, const x86::Vec& simd6, const x86::Vec& simd7) {

  x86::Gp i = n;
  x86::Gp j = gp0;

  x86::Vec vzero = simd0;
  x86::Vec v0080 = simd1;
  x86::Vec v0101 = simd2;

  Label L_SmallLoop = cc.new_label();
  Label L_SmallEnd  = cc.new_label();
  Label L_LargeLoop = cc.new_label();
  Label L_LargeEnd  = cc.new_label();
  Label L_Done = cc.new_label();

  // Load SIMD Constants.
  cc.xorps(vzero, vzero);
  cc.mov(gp0.r32(), 0x00800080);
  cc.movd(v0080, gp0.r32());
  cc.mov(gp0.r32(), 0x01010101);
  cc.movd(v0101, gp0.r32());
  cc.pshufd(v0080, v0080, x86::shuffle_imm(0, 0, 0, 0));
  cc.pshufd(v0101, v0101, x86::shuffle_imm(0, 0, 0, 0));

  // How many pixels have to be processed to make the loop aligned.
  cc.xor_(j, j);
  cc.sub(j, dst);
  cc.and_(j, 15);
  cc.shr(j, 2);
  cc.jz(L_SmallEnd);

  cc.cmp(j, i);
  cc.cmovg(j, i); // j = min(i, j)
  cc.sub(i, j);   // i -= j

  // Small loop.
  cc.bind(L_SmallLoop);
  {
    x86::Vec x0 = simd3;
    x86::Vec y0 = simd4;
    x86::Vec a0 = simd5;

    cc.movd(y0, x86::ptr(src));
    cc.movd(x0, x86::ptr(dst));

    cc.pcmpeqb(a0, a0);
    cc.pxor(a0, y0);
    cc.psrlw(a0, 8);
    cc.punpcklbw(x0, vzero);

    cc.pshuflw(a0, a0, x86::shuffle_imm(1, 1, 1, 1));
    cc.punpcklbw(y0, vzero);

    cc.pmullw(x0, a0);
    cc.paddsw(x0, v0080);
    cc.pmulhuw(x0, v0101);

    cc.paddw(x0, y0);
    cc.packuswb(x0, x0);

    cc.movd(x86::ptr(dst), x0);

    cc.add(dst, 4);
    cc.add(src, 4);

    cc.dec(j);
    cc.jnz(L_SmallLoop);
  }

  // Second section, prepare for an aligned loop.
  cc.bind(L_SmallEnd);

  cc.test(i, i);
  cc.mov(j, i);
  cc.jz(L_Done);

  cc.and_(j, 3);
  cc.shr(i, 2);
  cc.jz(L_LargeEnd);

  // Aligned loop.
  cc.bind(L_LargeLoop);
  {
    x86::Vec x0 = simd3;
    x86::Vec x1 = simd4;
    x86::Vec y0 = simd5;
    x86::Vec a0 = simd6;
    x86::Vec a1 = simd7;

    cc.movups(y0, x86::ptr(src));
    cc.movaps(x0, x86::ptr(dst));

    cc.pcmpeqb(a0, a0);
    cc.xorps(a0, y0);
    cc.movaps(x1, x0);

    cc.psrlw(a0, 8);
    cc.punpcklbw(x0, vzero);

    cc.movaps(a1, a0);
    cc.punpcklwd(a0, a0);

    cc.punpckhbw(x1, vzero);
    cc.punpckhwd(a1, a1);

    cc.pshufd(a0, a0, x86::shuffle_imm(3, 3, 1, 1));
    cc.pshufd(a1, a1, x86::shuffle_imm(3, 3, 1, 1));

    cc.pmullw(x0, a0);
    cc.pmullw(x1, a1);

    cc.paddsw(x0, v0080);
    cc.paddsw(x1, v0080);

    cc.pmulhuw(x0, v0101);
    cc.pmulhuw(x1, v0101);

    cc.add(src, 16);
    cc.packuswb(x0, x1);

    cc.paddw(x0, y0);
    cc.movaps(x86::ptr(dst), x0);

    cc.add(dst, 16);

    cc.dec(i);
    cc.jnz(L_LargeLoop);
  }

  cc.bind(L_LargeEnd);
  cc.test(j, j);
  cc.jnz(L_SmallLoop);

  cc.bind(L_Done);
}

static void generate_sse_alpha_blend(asmjit::BaseEmitter& emitter, bool emit_prolog_epilog) {
  using namespace asmjit::x86;

#ifndef ASMJIT_NO_COMPILER
  if (emitter.is_compiler()) {
    Compiler& cc = *emitter.as<Compiler>();

    Gp dst = cc.new_gp_ptr("dst");
    Gp src = cc.new_gp_ptr("src");
    Gp i = cc.new_gp_ptr("i");
    Gp j = cc.new_gp_ptr("j");

    Vec v0 = cc.new_xmm("v0");
    Vec v1 = cc.new_xmm("v1");
    Vec v2 = cc.new_xmm("v2");
    Vec v3 = cc.new_xmm("v3");
    Vec v4 = cc.new_xmm("v4");
    Vec v5 = cc.new_xmm("v5");
    Vec v6 = cc.new_xmm("v6");
    Vec v7 = cc.new_xmm("v7");

    FuncNode* func_node = cc.add_func(FuncSignature::build<void, void*, const void*, size_t>());
    func_node->set_arg(0, dst);
    func_node->set_arg(1, src);
    func_node->set_arg(2, i);
    generate_sse_alpha_blend_internal(cc, dst, src, i, j, v0, v1, v2, v3, v4, v5, v6, v7);
    cc.end_func();

    return;
  }
#endif

#ifndef ASMJIT_NO_BUILDER
  if (emitter.is_builder()) {
    Builder& cc = *emitter.as<Builder>();

    x86::Gp dst = cc.zax();
    x86::Gp src = cc.zcx();
    x86::Gp i = cc.zdx();
    x86::Gp j = cc.zdi();

    if (emit_prolog_epilog) {
      FuncDetail func;
      func.init(FuncSignature::build<void, void*, const void*, size_t>(), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.add_dirty_regs(dst, src, i, j);
      frame.add_dirty_regs(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);

      FuncArgsAssignment args(&func);
      args.assign_all(dst, src, i);
      args.update_func_frame(frame);
      frame.finalize();

      cc.emit_prolog(frame);
      cc.emit_args_assignment(frame, args);
      generate_sse_alpha_blend_internal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
      cc.emit_epilog(frame);
    }
    else {
      generate_sse_alpha_blend_internal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
    }

    return;
  }
#endif

  if (emitter.is_assembler()) {
    Assembler& cc = *emitter.as<Assembler>();

    x86::Gp dst = cc.zax();
    x86::Gp src = cc.zcx();
    x86::Gp i = cc.zdx();
    x86::Gp j = cc.zdi();

    if (emit_prolog_epilog) {
      FuncDetail func;
      func.init(FuncSignature::build<void, void*, const void*, size_t>(), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.add_dirty_regs(dst, src, i, j);
      frame.add_dirty_regs(xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);

      FuncArgsAssignment args(&func);
      args.assign_all(dst, src, i);
      args.update_func_frame(frame);
      frame.finalize();

      cc.emit_prolog(frame);
      cc.emit_args_assignment(frame, args);
      generate_sse_alpha_blend_internal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
      cc.emit_epilog(frame);
    }
    else {
      generate_sse_alpha_blend_internal(cc, dst, src, i, j, xmm0, xmm1, xmm2, xmm3, xmm4, xmm5, xmm6, xmm7);
    }

    return;
  }
}

} // {asmtest}

#endif // ASMJIT_TEST_MISC_H_INCLUDED
