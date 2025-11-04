// This file is part of AsmJit project <https://asmjit.com>
//
// See <asmjit/core.h> or LICENSE.md for license and copyright information
// SPDX-License-Identifier: Zlib

#include <asmjit/core.h>

#if !defined(ASMJIT_NO_AARCH64)
#include <asmjit/a64.h>

#include <limits>
#include <stdio.h>
#include <string.h>

#include "asmjit_bench_codegen.h"

using namespace asmjit;

// Generates a long sequence of GP instructions.
template<typename Emitter>
static void generate_gp_sequence_internal(
  Emitter& cc,
  const a64::Gp& a, const a64::Gp& b, const a64::Gp& c, const a64::Gp& d) {

  using namespace asmjit::a64;

  Gp wa = a.w();
  Gp wb = b.w();
  Gp wc = c.w();
  Gp wd = d.w();

  Gp xa = a.x();
  Gp xb = b.x();
  Gp xc = c.x();
  Gp xd = d.x();

  Mem m = ptr(xd);

  cc.mov(wa, 0);
  cc.mov(wb, 1);
  cc.mov(wc, 2);
  cc.mov(wd, 3);

  cc.adc(wa, wb, wc);
  cc.adc(xa, xb, xc);
  cc.adc(wa, wzr, wc);
  cc.adc(xa, xzr, xc);
  cc.adc(wzr, wb, wc);
  cc.adc(xzr, xb, xc);
  cc.adcs(wa, wb, wc);
  cc.adcs(xa, xb, xc);
  cc.add(wa, wb, wc);
  cc.add(xa, xb, xc);
  cc.add(wa, wb, wc, lsl(3));
  cc.add(xa, xb, xc, lsl(3));
  cc.add(wa, wzr, wc);
  cc.add(xa, xzr, xc);
  cc.add(wzr, wb, wc);
  cc.add(xzr, xb, xc);
  cc.add(wc, wd, 0, lsl(12));
  cc.add(xc, xd, 0, lsl(12));
  cc.add(wc, wd, 1024, lsl(12));
  cc.add(xc, xd, 1024, lsl(12));
  cc.add(wc, wd, 1024, lsl(12));
  cc.add(xc, xd, 1024, lsl(12));
  cc.adds(wa, wb, wc);
  cc.adds(xa, xb, xc);
  cc.adr(xa, 0);
  cc.adr(xa, 256);
  cc.adrp(xa, 4096);
  cc.and_(wa, wb, wc);
  cc.and_(xa, xb, xc);
  cc.and_(wa, wb, 1);
  cc.and_(xa, xb, 1);
  cc.and_(wa, wb, 15);
  cc.and_(xa, xb, 15);
  cc.and_(wa, wzr, wc);
  cc.and_(xa, xzr, xc);
  cc.and_(wzr, wb, wc);
  cc.and_(xzr, xb, xc);
  cc.and_(wa, wb, 0x1);
  cc.and_(xa, xb, 0x1);
  cc.and_(wa, wb, 0xf);
  cc.and_(xa, xb, 0xf);
  cc.ands(wa, wb, wc);
  cc.ands(xa, xb, xc);
  cc.ands(wa, wzr, wc);
  cc.ands(xa, xzr, xc);
  cc.ands(wzr, wb, wc);
  cc.ands(xzr, xb, xc);
  cc.ands(wa, wb, 0x1);
  cc.ands(xa, xb, 0x1);
  cc.ands(wa, wb, 0xf);
  cc.ands(xa, xb, 0xf);
  cc.asr(wa, wb, 15);
  cc.asr(xa, xb, 15);
  cc.asrv(wa, wb, wc);
  cc.asrv(xa, xb, xc);
  cc.bfc(wa, 8, 16);
  cc.bfc(xa, 8, 16);
  cc.bfi(wa, wb, 8, 16);
  cc.bfi(xa, xb, 8, 16);
  cc.bfm(wa, wb, 8, 16);
  cc.bfm(xa, xb, 8, 16);
  cc.bfxil(wa, wb, 8, 16);
  cc.bfxil(xa, xb, 8, 16);
  cc.bic(wa, wb, wc, lsl(4));
  cc.bic(xa, xb, xc, lsl(4));
  cc.bic(wa, wzr, wc);
  cc.bic(xa, xzr, xc);
  cc.bics(wa, wb, wc, lsl(4));
  cc.bics(xa, xb, xc, lsl(4));
  cc.bics(wa, wzr, wc);
  cc.bics(xa, xzr, xc);
  cc.cas(wa, wb, m);
  cc.cas(xa, xb, m);
  cc.casa(wa, wb, m);
  cc.casa(xa, xb, m);
  cc.casab(wa, wb, m);
  cc.casah(wa, wb, m);
  cc.casal(wa, wb, m);
  cc.casal(xa, xb, m);
  cc.casalb(wa, wb, m);
  cc.casalh(wa, wb, m);
  cc.casb(wa, wb, m);
  cc.cash(wa, wb, m);
  cc.casl(wa, wb, m);
  cc.casl(xa, xb, m);
  cc.caslb(wa, wb, m);
  cc.caslh(wa, wb, m);
  cc.ccmn(wa, wb, 3, CondCode::kEQ);
  cc.ccmn(xa, xb, 3, CondCode::kEQ);
  cc.ccmn(wa, 2, 3, CondCode::kEQ);
  cc.ccmn(xa, 2, 3, CondCode::kEQ);
  cc.ccmn(wa, wzr, 3, CondCode::kEQ);
  cc.ccmn(xa, xzr, 3, CondCode::kEQ);
  cc.ccmp(wa, wb, 3, CondCode::kEQ);
  cc.ccmp(xa, xb, 3, CondCode::kEQ);
  cc.ccmp(wa, 2, 3, CondCode::kEQ);
  cc.ccmp(xa, 2, 3, CondCode::kEQ);
  cc.ccmp(wa, wzr, 3, CondCode::kEQ);
  cc.ccmp(xa, xzr, 3, CondCode::kEQ);
  cc.cinc(wa, wb, CondCode::kEQ);
  cc.cinc(xa, xb, CondCode::kEQ);
  cc.cinc(wzr, wb, CondCode::kEQ);
  cc.cinc(wa, wzr, CondCode::kEQ);
  cc.cinc(xzr, xb, CondCode::kEQ);
  cc.cinc(xa, xzr, CondCode::kEQ);
  cc.cinv(wa, wb, CondCode::kEQ);
  cc.cinv(xa, xb, CondCode::kEQ);
  cc.cinv(wzr, wb, CondCode::kEQ);
  cc.cinv(wa, wzr, CondCode::kEQ);
  cc.cinv(xzr, xb, CondCode::kEQ);
  cc.cinv(xa, xzr, CondCode::kEQ);
  cc.cls(wa, wb);
  cc.cls(xa, xb);
  cc.cls(wa, wzr);
  cc.cls(xa, xzr);
  cc.cls(wzr, wb);
  cc.cls(xzr, xb);
  cc.clz(wa, wb);
  cc.clz(xa, xb);
  cc.clz(wa, wzr);
  cc.clz(xa, xzr);
  cc.clz(wzr, wb);
  cc.clz(xzr, xb);
  cc.cmn(wa, 33);
  cc.cmn(xa, 33);
  cc.cmn(wa, wb);
  cc.cmn(xa, xb);
  cc.cmn(wa, wb, uxtb(2));
  cc.cmn(xa, wb, uxtb(2));
  cc.cmp(wa, 33);
  cc.cmp(xa, 33);
  cc.cmp(wa, wb);
  cc.cmp(xa, xb);
  cc.cmp(wa, wb, uxtb(2));
  cc.cmp(xa, wb, uxtb(2));
  cc.crc32b(wa, wb, wc);
  cc.crc32b(wzr, wb, wc);
  cc.crc32b(wa, wzr, wc);
  cc.crc32b(wa, wb, wzr);
  cc.crc32cb(wa, wb, wc);
  cc.crc32cb(wzr, wb, wc);
  cc.crc32cb(wa, wzr, wc);
  cc.crc32cb(wa, wb, wzr);
  cc.crc32ch(wa, wb, wc);
  cc.crc32ch(wzr, wb, wc);
  cc.crc32ch(wa, wzr, wc);
  cc.crc32ch(wa, wb, wzr);
  cc.crc32cw(wa, wb, wc);
  cc.crc32cw(wzr, wb, wc);
  cc.crc32cw(wa, wzr, wc);
  cc.crc32cw(wa, wb, wzr);
  cc.crc32cx(wa, wb, xc);
  cc.crc32cx(wzr, wb, xc);
  cc.crc32cx(wa, wzr, xc);
  cc.crc32cx(wa, wb, xzr);
  cc.crc32h(wa, wb, wc);
  cc.crc32h(wzr, wb, wc);
  cc.crc32h(wa, wzr, wc);
  cc.crc32h(wa, wb, wzr);
  cc.crc32w(wa, wb, wc);
  cc.crc32w(wzr, wb, wc);
  cc.crc32w(wa, wzr, wc);
  cc.crc32w(wa, wb, wzr);
  cc.crc32x(wa, wb, xc);
  cc.crc32x(wzr, wb, xc);
  cc.crc32x(wa, wzr, xc);
  cc.crc32x(wa, wb, xzr);
  cc.csel(wa, wb, wc, CondCode::kEQ);
  cc.csel(xa, xb, xc, CondCode::kEQ);
  cc.cset(wa, CondCode::kEQ);
  cc.cset(xa, CondCode::kEQ);
  cc.cset(wa, CondCode::kEQ);
  cc.cset(xa, CondCode::kEQ);
  cc.csetm(wa, CondCode::kEQ);
  cc.csetm(xa, CondCode::kEQ);
  cc.csinc(wa, wb, wc, CondCode::kEQ);
  cc.csinc(xa, xb, xc, CondCode::kEQ);
  cc.csinv(wa, wb, wc, CondCode::kEQ);
  cc.csinv(xa, xb, xc, CondCode::kEQ);
  cc.csneg(wa, wb, wc, CondCode::kEQ);
  cc.csneg(xa, xb, xc, CondCode::kEQ);
  cc.eon(wa, wb, wc);
  cc.eon(wzr, wb, wc);
  cc.eon(wa, wzr, wc);
  cc.eon(wa, wb, wzr);
  cc.eon(wa, wb, wc, lsl(4));
  cc.eon(xa, xb, xc);
  cc.eon(xzr, xb, xc);
  cc.eon(xa, xzr, xc);
  cc.eon(xa, xb, xzr);
  cc.eon(xa, xb, xc, lsl(4));
  cc.eor(wa, wb, wc);
  cc.eor(wzr, wb, wc);
  cc.eor(wa, wzr, wc);
  cc.eor(wa, wb, wzr);
  cc.eor(xa, xb, xc);
  cc.eor(xzr, xb, xc);
  cc.eor(xa, xzr, xc);
  cc.eor(xa, xb, xzr);
  cc.eor(wa, wb, wc, lsl(4));
  cc.eor(xa, xb, xc, lsl(4));
  cc.eor(wa, wb, 0x4000);
  cc.eor(xa, xb, 0x8000);
  cc.extr(wa, wb, wc, 15);
  cc.extr(wzr, wb, wc, 15);
  cc.extr(wa, wzr, wc, 15);
  cc.extr(wa, wb, wzr, 15);
  cc.extr(xa, xb, xc, 15);
  cc.extr(xzr, xb, xc, 15);
  cc.extr(xa, xzr, xc, 15);
  cc.extr(xa, xb, xzr, 15);
  cc.ldadd(wa, wb, m);
  cc.ldadd(xa, xb, m);
  cc.ldadda(wa, wb, m);
  cc.ldadda(xa, xb, m);
  cc.ldaddab(wa, wb, m);
  cc.ldaddah(wa, wb, m);
  cc.ldaddal(wa, wb, m);
  cc.ldaddal(xa, xb, m);
  cc.ldaddalb(wa, wb, m);
  cc.ldaddalh(wa, wb, m);
  cc.ldaddb(wa, wb, m);
  cc.ldaddh(wa, wb, m);
  cc.ldaddl(wa, wb, m);
  cc.ldaddl(xa, xb, m);
  cc.ldaddlb(wa, wb, m);
  cc.ldaddlh(wa, wb, m);
  cc.ldclr(wa, wb, m);
  cc.ldclr(xa, xb, m);
  cc.ldclra(wa, wb, m);
  cc.ldclra(xa, xb, m);
  cc.ldclrab(wa, wb, m);
  cc.ldclrah(wa, wb, m);
  cc.ldclral(wa, wb, m);
  cc.ldclral(xa, xb, m);
  cc.ldclralb(wa, wb, m);
  cc.ldclralh(wa, wb, m);
  cc.ldclrb(wa, wb, m);
  cc.ldclrh(wa, wb, m);
  cc.ldclrl(wa, wb, m);
  cc.ldclrl(xa, xb, m);
  cc.ldclrlb(wa, wb, m);
  cc.ldclrlh(wa, wb, m);
  cc.ldeor(wa, wb, m);
  cc.ldeor(xa, xb, m);
  cc.ldeora(wa, wb, m);
  cc.ldeora(xa, xb, m);
  cc.ldeorab(wa, wb, m);
  cc.ldeorah(wa, wb, m);
  cc.ldeoral(wa, wb, m);
  cc.ldeoral(xa, xb, m);
  cc.ldeoralb(wa, wb, m);
  cc.ldeoralh(wa, wb, m);
  cc.ldeorb(wa, wb, m);
  cc.ldeorh(wa, wb, m);
  cc.ldeorl(wa, wb, m);
  cc.ldeorl(xa, xb, m);
  cc.ldeorlb(wa, wb, m);
  cc.ldeorlh(wa, wb, m);
  cc.ldlar(wa, m);
  cc.ldlar(xa, m);
  cc.ldlarb(wa, m);
  cc.ldlarh(wa, m);
  cc.ldnp(wa, wb, m);
  cc.ldnp(xa, xb, m);
  cc.ldp(wa, wb, m);
  cc.ldp(xa, xb, m);
  cc.ldpsw(xa, xb, m);
  cc.ldr(wa, m);
  cc.ldr(xa, m);
  cc.ldrb(wa, m);
  cc.ldrh(wa, m);
  cc.ldrsw(xa, m);
  cc.ldraa(xa, m);
  cc.ldrab(xa, m);
  cc.ldset(wa, wb, m);
  cc.ldset(xa, xb, m);
  cc.ldseta(wa, wb, m);
  cc.ldseta(xa, xb, m);
  cc.ldsetab(wa, wb, m);
  cc.ldsetah(wa, wb, m);
  cc.ldsetal(wa, wb, m);
  cc.ldsetal(xa, xb, m);
  cc.ldsetalh(wa, wb, m);
  cc.ldsetalb(wa, wb, m);
  cc.ldsetb(wa, wb, m);
  cc.ldseth(wa, wb, m);
  cc.ldsetl(wa, wb, m);
  cc.ldsetl(xa, xb, m);
  cc.ldsetlb(wa, wb, m);
  cc.ldsetlh(wa, wb, m);
  cc.ldsmax(wa, wb, m);
  cc.ldsmax(xa, xb, m);
  cc.ldsmaxa(wa, wb, m);
  cc.ldsmaxa(xa, xb, m);
  cc.ldsmaxab(wa, wb, m);
  cc.ldsmaxah(wa, wb, m);
  cc.ldsmaxal(wa, wb, m);
  cc.ldsmaxal(xa, xb, m);
  cc.ldsmaxalb(wa, wb, m);
  cc.ldsmaxalh(wa, wb, m);
  cc.ldsmaxb(wa, wb, m);
  cc.ldsmaxh(wa, wb, m);
  cc.ldsmaxl(wa, wb, m);
  cc.ldsmaxl(xa, xb, m);
  cc.ldsmaxlb(wa, wb, m);
  cc.ldsmaxlh(wa, wb, m);
  cc.ldsmin(wa, wb, m);
  cc.ldsmin(xa, xb, m);
  cc.ldsmina(wa, wb, m);
  cc.ldsmina(xa, xb, m);
  cc.ldsminab(wa, wb, m);
  cc.ldsminah(wa, wb, m);
  cc.ldsminal(wa, wb, m);
  cc.ldsminal(xa, xb, m);
  cc.ldsminalb(wa, wb, m);
  cc.ldsminalh(wa, wb, m);
  cc.ldsminb(wa, wb, m);
  cc.ldsminh(wa, wb, m);
  cc.ldsminl(wa, wb, m);
  cc.ldsminl(xa, xb, m);
  cc.ldsminlb(wa, wb, m);
  cc.ldsminlh(wa, wb, m);
  cc.ldtr(wa, m);
  cc.ldtr(xa, m);
  cc.ldtrb(wa, m);
  cc.ldtrh(wa, m);
  cc.ldtrsb(wa, m);
  cc.ldtrsh(wa, m);
  cc.ldtrsw(xa, m);
  cc.ldumax(wa, wb, m);
  cc.ldumax(xa, xb, m);
  cc.ldumaxa(wa, wb, m);
  cc.ldumaxa(xa, xb, m);
  cc.ldumaxab(wa, wb, m);
  cc.ldumaxah(wa, wb, m);
  cc.ldumaxal(wa, wb, m);
  cc.ldumaxal(xa, xb, m);
  cc.ldumaxalb(wa, wb, m);
  cc.ldumaxalh(wa, wb, m);
  cc.ldumaxb(wa, wb, m);
  cc.ldumaxh(wa, wb, m);
  cc.ldumaxl(wa, wb, m);
  cc.ldumaxl(xa, xb, m);
  cc.ldumaxlb(wa, wb, m);
  cc.ldumaxlh(wa, wb, m);
  cc.ldumin(wa, wb, m);
  cc.ldumin(xa, xb, m);
  cc.ldumina(wa, wb, m);
  cc.ldumina(xa, xb, m);
  cc.lduminab(wa, wb, m);
  cc.lduminah(wa, wb, m);
  cc.lduminal(wa, wb, m);
  cc.lduminal(xa, xb, m);
  cc.lduminalb(wa, wb, m);
  cc.lduminalh(wa, wb, m);
  cc.lduminb(wa, wb, m);
  cc.lduminh(wa, wb, m);
  cc.lduminl(wa, wb, m);
  cc.lduminl(xa, xb, m);
  cc.lduminlb(wa, wb, m);
  cc.lduminlh(wa, wb, m);
  cc.ldur(wa, m);
  cc.ldur(xa, m);
  cc.ldurb(wa, m);
  cc.ldurh(wa, m);
  cc.ldursb(wa, m);
  cc.ldursh(wa, m);
  cc.ldursw(xa, m);
  cc.ldxp(wa, wb, m);
  cc.ldxp(xa, xb, m);
  cc.ldxr(wa, m);
  cc.ldxr(xa, m);
  cc.ldxrb(wa, m);
  cc.ldxrh(wa, m);
  cc.lsl(wa, wb, wc);
  cc.lsl(xa, xb, xc);
  cc.lsl(wa, wb, 15);
  cc.lsl(xa, xb, 15);
  cc.lslv(wa, wb, wc);
  cc.lslv(xa, xb, xc);
  cc.lsr(wa, wb, wc);
  cc.lsr(xa, xb, xc);
  cc.lsr(wa, wb, 15);
  cc.lsr(xa, xb, 15);
  cc.lsrv(wa, wb, wc);
  cc.lsrv(xa, xb, xc);
  cc.madd(wa, wb, wc, wd);
  cc.madd(xa, xb, xc, xd);
  cc.mneg(wa, wb, wc);
  cc.mneg(xa, xb, xc);
  cc.mov(wa, wb);
  cc.mov(xa, xb);
  cc.mov(wa, 0);
  cc.mov(wa, 1);
  cc.mov(wa, 2);
  cc.mov(wa, 3);
  cc.mov(wa, 4);
  cc.mov(wa, 5);
  cc.mov(wa, 6);
  cc.mov(wa, 7);
  cc.mov(wa, 8);
  cc.mov(wa, 9);
  cc.mov(wa, 10);
  cc.mov(wa, 0xA234);
  cc.mov(xa, 0xA23400000000);
  cc.msub(wa, wb, wc, wd);
  cc.msub(xa, xb, xc, xd);
  cc.mul(wa, wb, wc);
  cc.mul(xa, xb, xc);
  cc.mvn_(wa, wb);
  cc.mvn_(xa, xb);
  cc.mvn_(wa, wb, lsl(4));
  cc.mvn_(xa, xb, lsl(4));
  cc.neg(wa, wb);
  cc.neg(xa, xb);
  cc.neg(wa, wb, lsl(4));
  cc.neg(xa, xb, lsl(4));
  cc.negs(wa, wb);
  cc.negs(xa, xb);
  cc.negs(wa, wb, lsl(4));
  cc.negs(xa, xb, lsl(4));
  cc.ngc(wa, wb);
  cc.ngc(xa, xb);
  cc.ngcs(wa, wb);
  cc.ngcs(xa, xb);
  cc.orn(wa, wb, wc);
  cc.orn(xa, xb, xc);
  cc.orn(wa, wb, wc, lsl(4));
  cc.orn(xa, xb, xc, lsl(4));
  cc.orr(wa, wb, wc);
  cc.orr(xa, xb, xc);
  cc.orr(wa, wb, wc, lsl(4));
  cc.orr(xa, xb, xc, lsl(4));
  cc.orr(wa, wb, 0x4000);
  cc.orr(xa, xb, 0x8000);
  cc.rbit(wa, wb);
  cc.rbit(xa, xb);
  cc.rev(wa, wb);
  cc.rev(xa, xb);
  cc.rev16(wa, wb);
  cc.rev16(xa, xb);
  cc.rev32(xa, xb);
  cc.rev64(xa, xb);
  cc.ror(wa, wb, wc);
  cc.ror(xa, xb, xc);
  cc.ror(wa, wb, 15);
  cc.ror(xa, xb, 15);
  cc.rorv(wa, wb, wc);
  cc.rorv(xa, xb, xc);
  cc.sbc(wa, wb, wc);
  cc.sbc(xa, xb, xc);
  cc.sbcs(wa, wb, wc);
  cc.sbcs(xa, xb, xc);
  cc.sbfiz(wa, wb, 5, 10);
  cc.sbfiz(xa, xb, 5, 10);
  cc.sbfm(wa, wb, 5, 10);
  cc.sbfm(xa, xb, 5, 10);
  cc.sbfx(wa, wb, 5, 10);
  cc.sbfx(xa, xb, 5, 10);
  cc.sdiv(wa, wb, wc);
  cc.sdiv(xa, xb, xc);
  cc.smaddl(xa, wb, wc, xd);
  cc.smnegl(xa, wb, wc);
  cc.smsubl(xa, wb, wc, xd);
  cc.smulh(xa, xb, xc);
  cc.smull(xa, wb, wc);
  cc.stp(wa, wb, m);
  cc.stp(xa, xb, m);
  cc.sttr(wa, m);
  cc.sttr(xa, m);
  cc.sttrb(wa, m);
  cc.sttrh(wa, m);
  cc.stur(wa, m);
  cc.stur(xa, m);
  cc.sturb(wa, m);
  cc.sturh(wa, m);
  cc.stxp(wa, wb, wc, m);
  cc.stxp(wa, xb, xc, m);
  cc.stxr(wa, wb, m);
  cc.stxr(wa, xb, m);
  cc.stxrb(wa, wb, m);
  cc.stxrh(wa, wb, m);
  cc.sub(wa, wb, wc);
  cc.sub(xa, xb, xc);
  cc.sub(wa, wb, wc, lsl(3));
  cc.sub(xa, xb, xc, lsl(3));
  cc.subg(xa, xb, 32, 11);
  cc.subp(xa, xb, xc);
  cc.subps(xa, xb, xc);
  cc.subs(wa, wb, wc);
  cc.subs(xa, xb, xc);
  cc.subs(wa, wb, wc, lsl(3));
  cc.subs(xa, xb, xc, lsl(3));
  cc.sxtb(wa, wb);
  cc.sxtb(xa, wb);
  cc.sxth(wa, wb);
  cc.sxth(xa, wb);
  cc.sxtw(xa, wb);
  cc.tst(wa, 1);
  cc.tst(xa, 1);
  cc.tst(wa, wb);
  cc.tst(xa, xb);
  cc.tst(wa, wb, lsl(4));
  cc.tst(xa, xb, lsl(4));
  cc.udiv(wa, wb, wc);
  cc.udiv(xa, xb, xc);
  cc.ubfiz(wa, wb, 5, 10);
  cc.ubfiz(xa, xb, 5, 10);
  cc.ubfm(wa, wb, 5, 10);
  cc.ubfm(xa, xb, 5, 10);
  cc.ubfx(wa, wb, 5, 10);
  cc.ubfx(xa, xb, 5, 10);
  cc.umaddl(xa, wb, wc, xd);
  cc.umnegl(xa, wb, wc);
  cc.umsubl(xa, wb, wc, xd);
  cc.umulh(xa, xb, xc);
  cc.umull(xa, wb, wc);
  cc.uxtb(wa, wb);
  cc.uxth(wa, wb);
}

static void generate_gp_sequence(BaseEmitter& emitter, bool emit_prolog_epilog) {
#ifndef ASMJIT_NO_COMPILER
  if (emitter.is_compiler()) {
    a64::Compiler& cc = *emitter.as<a64::Compiler>();

    a64::Gp a = cc.new_gp_ptr("a");
    a64::Gp b = cc.new_gp_ptr("b");
    a64::Gp c = cc.new_gp_ptr("c");
    a64::Gp d = cc.new_gp_ptr("d");

    cc.add_func(FuncSignature::build<void>());
    generate_gp_sequence_internal(cc, a, b, c, d);
    cc.end_func();

    return;
  }
#endif

#ifndef ASMJIT_NO_BUILDER
  if (emitter.is_builder()) {
    a64::Builder& cc = *emitter.as<a64::Builder>();

    a64::Gp a = a64::x0;
    a64::Gp b = a64::x1;
    a64::Gp c = a64::x2;
    a64::Gp d = a64::x3;

    if (emit_prolog_epilog) {
      FuncDetail func;
      func.init(FuncSignature::build<void, void*, const void*, size_t>(), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.add_dirty_regs(a, b, c, d);
      frame.finalize();

      cc.emit_prolog(frame);
      generate_gp_sequence_internal(cc, a, b, c, d);
      cc.emit_epilog(frame);
    }
    else {
      generate_gp_sequence_internal(cc, a, b, c, d);
    }

    return;
  }
#endif

  if (emitter.is_assembler()) {
    a64::Assembler& cc = *emitter.as<a64::Assembler>();

    a64::Gp a = a64::x0;
    a64::Gp b = a64::x1;
    a64::Gp c = a64::x2;
    a64::Gp d = a64::x3;

    if (emit_prolog_epilog) {
      FuncDetail func;
      func.init(FuncSignature::build<void, void*, const void*, size_t>(), cc.environment());

      FuncFrame frame;
      frame.init(func);
      frame.add_dirty_regs(a, b, c, d);
      frame.finalize();

      cc.emit_prolog(frame);
      generate_gp_sequence_internal(cc, a, b, c, d);
      cc.emit_epilog(frame);
    }
    else {
      generate_gp_sequence_internal(cc, a, b, c, d);
    }

    return;
  }
}

template<typename EmitterFn>
static void benchmark_a64_function(Arch arch, uint32_t num_iterations, const char* description, const EmitterFn& emitter_fn) noexcept {
  CodeHolder code;
  printf("%s:\n", description);

  uint32_t instruction_count = 0;

#ifndef ASMJIT_NO_BUILDER
  instruction_count = asmjit_perf_utils::calculate_instruction_count<a64::Builder>(code, arch, [&](a64::Builder& cc) {
    emitter_fn(cc, false);
  });
#endif

  asmjit_perf_utils::bench<a64::Assembler>(code, arch, num_iterations, "[raw]", instruction_count, [&](a64::Assembler& cc) {
    emitter_fn(cc, false);
  });

  asmjit_perf_utils::bench<a64::Assembler>(code, arch, num_iterations, "[validated]", instruction_count, [&](a64::Assembler& cc) {
    cc.add_diagnostic_options(DiagnosticOptions::kValidateAssembler);
    emitter_fn(cc, false);
  });

  asmjit_perf_utils::bench<a64::Assembler>(code, arch, num_iterations, "[prolog/epilog]", instruction_count, [&](a64::Assembler& cc) {
    cc.add_diagnostic_options(DiagnosticOptions::kValidateAssembler);
    emitter_fn(cc, true);
  });

#ifndef ASMJIT_NO_BUILDER
  asmjit_perf_utils::bench<a64::Builder>(code, arch, num_iterations, "[no-asm]", instruction_count, [&](a64::Builder& cc) {
    emitter_fn(cc, false);
  });

  asmjit_perf_utils::bench<a64::Builder>(code, arch, num_iterations, "[finalized]", instruction_count, [&](a64::Builder& cc) {
    emitter_fn(cc, false);
    cc.finalize();
  });

  asmjit_perf_utils::bench<a64::Builder>(code, arch, num_iterations, "[prolog/epilog]", instruction_count, [&](a64::Builder& cc) {
    emitter_fn(cc, true);
    cc.finalize();
  });
#endif

#ifndef ASMJIT_NO_COMPILER
  asmjit_perf_utils::bench<a64::Compiler>(code, arch, num_iterations, "[no-asm]", instruction_count, [&](a64::Compiler& cc) {
    emitter_fn(cc, true);
  });

  asmjit_perf_utils::bench<a64::Compiler>(code, arch, num_iterations, "[finalized]", instruction_count, [&](a64::Compiler& cc) {
    emitter_fn(cc, true);
    cc.finalize();
  });
#endif

  printf("\n");
}

void benchmark_aarch64_emitters(uint32_t num_iterations) {
  static const char description[] = "GpSequence (Sequence of GP instructions - reg/mem)";
  benchmark_a64_function(Arch::kAArch64, num_iterations, description, [](BaseEmitter& emitter, bool emit_prolog_epilog) {
    generate_gp_sequence(emitter, emit_prolog_epilog);
  });
}

#endif // !ASMJIT_NO_AARCH64
