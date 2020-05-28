// AsmJit - Machine code generation for C++
//
//  * Official AsmJit Home Page: https://asmjit.com
//  * Official Github Repository: https://github.com/asmjit/asmjit
//
// Copyright (c) 2008-2020 The AsmJit Authors
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef ASMJIT_TEST_MISC_H_INCLUDED
#define ASMJIT_TEST_MISC_H_INCLUDED

#include <asmjit/x86.h>

namespace asmtest {

// Generate a typical alpha blend function using SSE2 instruction set. Used
// for benchmarking and also in test86. The generated code should be stable
// and fully functional.
static void generateAlphaBlend(asmjit::x86::Compiler& cc) {
  using namespace asmjit;
  using namespace asmjit::x86;

  Gp dst = cc.newIntPtr("dst");
  Gp src = cc.newIntPtr("src");

  Gp i = cc.newIntPtr("i");
  Gp j = cc.newIntPtr("j");
  Gp t = cc.newIntPtr("t");

  Xmm vzero = cc.newXmm("vzero");
  Xmm v0080 = cc.newXmm("v0080");
  Xmm v0101 = cc.newXmm("v0101");

  Label L_SmallLoop = cc.newLabel();
  Label L_SmallEnd  = cc.newLabel();
  Label L_LargeLoop = cc.newLabel();
  Label L_LargeEnd  = cc.newLabel();
  Label L_DataPool  = cc.newLabel();

  cc.addFunc(FuncSignatureT<void, void*, const void*, size_t>(cc.codeInfo().cdeclCallConv()));

  cc.setArg(0, dst);
  cc.setArg(1, src);
  cc.setArg(2, i);

  // How many pixels have to be processed to make the loop aligned.
  cc.lea(t, x86::ptr(L_DataPool));
  cc.xorps(vzero, vzero);
  cc.movaps(v0080, x86::ptr(t, 0));
  cc.movaps(v0101, x86::ptr(t, 16));

  cc.xor_(j, j);
  cc.sub(j, dst);
  cc.and_(j, 15);
  cc.shr(j, 2);
  cc.jz(L_SmallEnd);

  cc.cmp(j, i);
  cc.cmovg(j, i); // j = min(i, j).
  cc.sub(i, j);   // i -= j.

  // Small loop.
  cc.bind(L_SmallLoop);
  {
    Xmm x0 = cc.newXmm("x0");
    Xmm y0 = cc.newXmm("y0");
    Xmm a0 = cc.newXmm("a0");

    cc.movd(y0, x86::ptr(src));
    cc.movd(x0, x86::ptr(dst));

    cc.pcmpeqb(a0, a0);
    cc.pxor(a0, y0);
    cc.psrlw(a0, 8);
    cc.punpcklbw(x0, vzero);

    cc.pshuflw(a0, a0, x86::Predicate::shuf(1, 1, 1, 1));
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
  cc.jz(cc.func()->exitLabel());

  cc.and_(j, 3);
  cc.shr(i, 2);
  cc.jz(L_LargeEnd);

  // Aligned loop.
  cc.bind(L_LargeLoop);
  {
    Xmm x0 = cc.newXmm("x0");
    Xmm x1 = cc.newXmm("x1");
    Xmm y0 = cc.newXmm("y0");
    Xmm a0 = cc.newXmm("a0");
    Xmm a1 = cc.newXmm("a1");

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

    cc.pshufd(a0, a0, x86::Predicate::shuf(3, 3, 1, 1));
    cc.pshufd(a1, a1, x86::Predicate::shuf(3, 3, 1, 1));

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

  cc.endFunc();

  // Data.
  cc.align(kAlignData, 16);
  cc.bind(L_DataPool);
  cc.dxmm(Data128::fromI16(0x0080));
  cc.dxmm(Data128::fromI16(0x0101));
}

} // {asmtest}

#endif // ASMJIT_TEST_MISC_H_INCLUDED
