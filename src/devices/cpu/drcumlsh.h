// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    drcumlsh.h

    Shorthand definitions for the universal machine language.

***************************************************************************/
#ifndef MAME_CPU_DRCUMLSH_H
#define MAME_CPU_DRCUMLSH_H

#pragma once

#include "drcuml.h"



/***************************************************************************
    MACROS
***************************************************************************/

/* ----- Compile-time Opcodes ----- */
#define UML_HANDLE(block, handle_)                          do { using namespace uml; block.append().handle(handle_); } while (0)
#define UML_HASH(block, mode, pc)                           do { using namespace uml; block.append().hash(mode, pc); } while (0)
#define UML_LABEL(block, label_)                            do { using namespace uml; block.append().label(label_); } while (0)
#define UML_MAPVAR(block, mapvar_, value)                   do { using namespace uml; block.append().mapvar(mapvar_, value); } while (0)


/* ----- Control Flow Operations ----- */
#define UML_NOP(block)                                      do { using namespace uml; block.append().nop(); } while (0)
#define UML_BREAK(block)                                    do { using namespace uml; block.append().break_(); } while (0)
#define UML_DEBUG(block, pc)                                do { using namespace uml; block.append().debug(pc); } while (0)
#define UML_EXIT(block, param)                              do { using namespace uml; block.append().exit(param); } while (0)
#define UML_EXITc(block, cond, param)                       do { using namespace uml; block.append().exit(param, cond); } while (0)
#define UML_HASHJMP(block, mode, pc, handle)                do { using namespace uml; block.append().hashjmp(mode, pc, handle); } while (0)
#define UML_JMP(block, label)                               do { using namespace uml; block.append().jmp(label); } while (0)
#define UML_JMPc(block, cond, label)                        do { using namespace uml; block.append().jmp(cond, label); } while (0)
#define UML_JMPH(block, handle)                             do { using namespace uml; block.append().jmph(handle); } while (0)
#define UML_JMPHc(block, cond, handle)                      do { using namespace uml; block.append().jmph(cond, handle); } while (0)
#define UML_EXH(block, handle, param)                       do { using namespace uml; block.append().exh(handle, param); } while (0)
#define UML_EXHc(block, cond, handle, param)                do { using namespace uml; block.append().exh(cond, handle, param); } while (0)
#define UML_CALLH(block, handle)                            do { using namespace uml; block.append().callh(handle); } while (0)
#define UML_CALLHc(block, cond, handle)                     do { using namespace uml; block.append().callh(cond, handle); } while (0)
#define UML_RET(block)                                      do { using namespace uml; block.append().ret(); } while (0)
#define UML_RETc(block, cond)                               do { using namespace uml; block.append().ret(cond); } while (0)
#define UML_CALLC(block, func, ptr)                         do { using namespace uml; block.append().callc(func, ptr); } while (0)
#define UML_CALLCc(block, cond, func, ptr)                  do { using namespace uml; block.append().callc(cond, func, ptr); } while (0)
#define UML_RECOVER(block, dst, mapvar)                     do { using namespace uml; block.append().recover(dst, mapvar); } while (0)


/* ----- Internal Register Operations ----- */
#define UML_SETFMOD(block, mode)                            do { using namespace uml; block.append().setfmod(mode); } while (0)
#define UML_GETFMOD(block, dst)                             do { using namespace uml; block.append().getfmod(dst); } while (0)
#define UML_GETEXP(block, dst)                              do { using namespace uml; block.append().getexp(dst); } while (0)
#define UML_GETFLGS(block, dst, flags)                      do { using namespace uml; block.append().getflgs(dst, flags); } while (0)
#define UML_SETFLGS(block, flags)                           do { using namespace uml; block.append().setflgs(flags); } while (0)
#define UML_SAVE(block, dst)                                do { using namespace uml; block.append().save(dst); } while (0)
#define UML_RESTORE(block, src)                             do { using namespace uml; block.append().restore(src); } while (0)


/* ----- 32-Bit Integer Operations ----- */
#define UML_LOAD(block, dst, base, index, size, scale)      do { using namespace uml; block.append().load(dst, base, index, size, scale); } while (0)
#define UML_LOADS(block, dst, base, index, size, scale)     do { using namespace uml; block.append().loads(dst, base, index, size, scale); } while (0)
#define UML_STORE(block, base, index, src1, size, scale)    do { using namespace uml; block.append().store(base, index, src1, size, scale); } while (0)
#define UML_READ(block, dst, src1, size, space)             do { using namespace uml; block.append().read(dst, src1, size, space); } while (0)
#define UML_READM(block, dst, src1, mask, size, space)      do { using namespace uml; block.append().readm(dst, src1, mask, size, space); } while (0)
#define UML_WRITE(block, dst, src1, size, space)            do { using namespace uml; block.append().write(dst, src1, size, space); } while (0)
#define UML_WRITEM(block, dst, src1, mask, size, space)     do { using namespace uml; block.append().writem(dst, src1, mask, size, space); } while (0)
#define UML_CARRY(block, src, bitnum)                       do { using namespace uml; block.append().carry(src, bitnum); } while (0)
#define UML_SETc(block, cond, dst)                          do { using namespace uml; block.append().set(cond, dst); } while (0)
#define UML_MOV(block, dst, src1)                           do { using namespace uml; block.append().mov(dst, src1); } while (0)
#define UML_MOVc(block, cond, dst, src1)                    do { using namespace uml; block.append().mov(cond, dst, src1); } while (0)
#define UML_SEXT(block, dst, src1, size)                    do { using namespace uml; block.append().sext(dst, src1, size); } while (0)
#define UML_ROLAND(block, dst, src, shift, mask)            do { using namespace uml; block.append().roland(dst, src, shift, mask); } while (0)
#define UML_ROLINS(block, dst, src, shift, mask)            do { using namespace uml; block.append().rolins(dst, src, shift, mask); } while (0)
#define UML_ADD(block, dst, src1, src2)                     do { using namespace uml; block.append().add(dst, src1, src2); } while (0)
#define UML_ADDC(block, dst, src1, src2)                    do { using namespace uml; block.append().addc(dst, src1, src2); } while (0)
#define UML_SUB(block, dst, src1, src2)                     do { using namespace uml; block.append().sub(dst, src1, src2); } while (0)
#define UML_SUBB(block, dst, src1, src2)                    do { using namespace uml; block.append().subb(dst, src1, src2); } while (0)
#define UML_CMP(block, src1, src2)                          do { using namespace uml; block.append().cmp(src1, src2); } while (0)
#define UML_MULU(block, dst, edst, src1, src2)              do { using namespace uml; block.append().mulu(dst, edst, src1, src2); } while (0)
#define UML_MULULW(block, dst, src1, src2)                  do { using namespace uml; block.append().mululw(dst, src1, src2); } while (0)
#define UML_MULS(block, dst, edst, src1, src2)              do { using namespace uml; block.append().muls(dst, edst, src1, src2); } while (0)
#define UML_MULSLW(block, dst, src1, src2)                  do { using namespace uml; block.append().mulslw(dst, src1, src2); } while (0)
#define UML_DIVU(block, dst, edst, src1, src2)              do { using namespace uml; block.append().divu(dst, edst, src1, src2); } while (0)
#define UML_DIVS(block, dst, edst, src1, src2)              do { using namespace uml; block.append().divs(dst, edst, src1, src2); } while (0)
#define UML_AND(block, dst, src1, src2)                     do { using namespace uml; block.append()._and(dst, src1, src2); } while (0)
#define UML_TEST(block, src1, src2)                         do { using namespace uml; block.append().test(src1, src2); } while (0)
#define UML_OR(block, dst, src1, src2)                      do { using namespace uml; block.append()._or(dst, src1, src2); } while (0)
#define UML_XOR(block, dst, src1, src2)                     do { using namespace uml; block.append()._xor(dst, src1, src2); } while (0)
#define UML_LZCNT(block, dst, src)                          do { using namespace uml; block.append().lzcnt(dst, src); } while (0)
#define UML_TZCNT(block, dst, src)                          do { using namespace uml; block.append().tzcnt(dst, src); } while (0)
#define UML_BSWAP(block, dst, src)                          do { using namespace uml; block.append().bswap(dst, src); } while (0)
#define UML_SHL(block, dst, src, count)                     do { using namespace uml; block.append().shl(dst, src, count); } while (0)
#define UML_SHR(block, dst, src, count)                     do { using namespace uml; block.append().shr(dst, src, count); } while (0)
#define UML_SAR(block, dst, src, count)                     do { using namespace uml; block.append().sar(dst, src, count); } while (0)
#define UML_ROL(block, dst, src, count)                     do { using namespace uml; block.append().rol(dst, src, count); } while (0)
#define UML_ROLC(block, dst, src, count)                    do { using namespace uml; block.append().rolc(dst, src, count); } while (0)
#define UML_ROR(block, dst, src, count)                     do { using namespace uml; block.append().ror(dst, src, count); } while (0)
#define UML_RORC(block, dst, src, count)                    do { using namespace uml; block.append().rorc(dst, src, count); } while (0)


/* ----- 64-Bit Integer Operations ----- */
#define UML_DLOAD(block, dst, base, index, size, scale)     do { using namespace uml; block.append().dload(dst, base, index, size, scale); } while (0)
#define UML_DLOADS(block, dst, base, index, size, scale)    do { using namespace uml; block.append().dloads(dst, base, index, size, scale); } while (0)
#define UML_DSTORE(block, base, index, src1, size, scale)   do { using namespace uml; block.append().dstore(base, index, src1, size, scale); } while (0)
#define UML_DREAD(block, dst, src1, size, space)            do { using namespace uml; block.append().dread(dst, src1, size, space); } while (0)
#define UML_DREADM(block, dst, src1, mask, size, space)     do { using namespace uml; block.append().dreadm(dst, src1, mask, size, space); } while (0)
#define UML_DWRITE(block, dst, src1, size, space)           do { using namespace uml; block.append().dwrite(dst, src1, size, space); } while (0)
#define UML_DWRITEM(block, dst, src1, mask, size, space)    do { using namespace uml; block.append().dwritem(dst, src1, mask, size, space); } while (0)
#define UML_DCARRY(block, src, bitnum)                      do { using namespace uml; block.append().dcarry(src, bitnum); } while (0)
#define UML_DSETc(block, cond, dst)                         do { using namespace uml; block.append().dset(cond, dst); } while (0)
#define UML_DMOV(block, dst, src1)                          do { using namespace uml; block.append().dmov(dst, src1); } while (0)
#define UML_DMOVc(block, cond, dst, src1)                   do { using namespace uml; block.append().dmov(cond, dst, src1); } while (0)
#define UML_DSEXT(block, dst, src1, size)                   do { using namespace uml; block.append().dsext(dst, src1, size); } while (0)
#define UML_DROLAND(block, dst, src, shift, mask)           do { using namespace uml; block.append().droland(dst, src, shift, mask); } while (0)
#define UML_DROLINS(block, dst, src, shift, mask)           do { using namespace uml; block.append().drolins(dst, src, shift, mask); } while (0)
#define UML_DADD(block, dst, src1, src2)                    do { using namespace uml; block.append().dadd(dst, src1, src2); } while (0)
#define UML_DADDC(block, dst, src1, src2)                   do { using namespace uml; block.append().daddc(dst, src1, src2); } while (0)
#define UML_DSUB(block, dst, src1, src2)                    do { using namespace uml; block.append().dsub(dst, src1, src2); } while (0)
#define UML_DSUBB(block, dst, src1, src2)                   do { using namespace uml; block.append().dsubb(dst, src1, src2); } while (0)
#define UML_DCMP(block, src1, src2)                         do { using namespace uml; block.append().dcmp(src1, src2); } while (0)
#define UML_DMULU(block, dst, edst, src1, src2)             do { using namespace uml; block.append().dmulu(dst, edst, src1, src2); } while (0)
#define UML_DMULULW(block, dst, src1, src2)                 do { using namespace uml; block.append().dmululw(dst, src1, src2); } while (0)
#define UML_DMULS(block, dst, edst, src1, src2)             do { using namespace uml; block.append().dmuls(dst, edst, src1, src2); } while (0)
#define UML_DMULSLW(block, dst, src1, src2)                 do { using namespace uml; block.append().dmulslw(dst, src1, src2); } while (0)
#define UML_DDIVU(block, dst, edst, src1, src2)             do { using namespace uml; block.append().ddivu(dst, edst, src1, src2); } while (0)
#define UML_DDIVS(block, dst, edst, src1, src2)             do { using namespace uml; block.append().ddivs(dst, edst, src1, src2); } while (0)
#define UML_DAND(block, dst, src1, src2)                    do { using namespace uml; block.append().dand(dst, src1, src2); } while (0)
#define UML_DTEST(block, src1, src2)                        do { using namespace uml; block.append().dtest(src1, src2); } while (0)
#define UML_DOR(block, dst, src1, src2)                     do { using namespace uml; block.append().dor(dst, src1, src2); } while (0)
#define UML_DXOR(block, dst, src1, src2)                    do { using namespace uml; block.append().dxor(dst, src1, src2); } while (0)
#define UML_DLZCNT(block, dst, src)                         do { using namespace uml; block.append().dlzcnt(dst, src); } while (0)
#define UML_DTZCNT(block, dst, src)                         do { using namespace uml; block.append().dtzcnt(dst, src); } while (0)
#define UML_DBSWAP(block, dst, src)                         do { using namespace uml; block.append().dbswap(dst, src); } while (0)
#define UML_DSHL(block, dst, src, count)                    do { using namespace uml; block.append().dshl(dst, src, count); } while (0)
#define UML_DSHR(block, dst, src, count)                    do { using namespace uml; block.append().dshr(dst, src, count); } while (0)
#define UML_DSAR(block, dst, src, count)                    do { using namespace uml; block.append().dsar(dst, src, count); } while (0)
#define UML_DROL(block, dst, src, count)                    do { using namespace uml; block.append().drol(dst, src, count); } while (0)
#define UML_DROLC(block, dst, src, count)                   do { using namespace uml; block.append().drolc(dst, src, count); } while (0)
#define UML_DROR(block, dst, src, count)                    do { using namespace uml; block.append().dror(dst, src, count); } while (0)
#define UML_DRORC(block, dst, src, count)                   do { using namespace uml; block.append().drorc(dst, src, count); } while (0)


/* ----- 32-bit Floating Point Arithmetic Operations ----- */
#define UML_FSLOAD(block, dst, base, index)                 do { using namespace uml; block.append().fsload(dst, base, index); } while (0)
#define UML_FSSTORE(block, base, index, src1)               do { using namespace uml; block.append().fsstore(base, index, src1); } while (0)
#define UML_FSREAD(block, dst, src1, space)                 do { using namespace uml; block.append().fsread(dst, src1, space); } while (0)
#define UML_FSWRITE(block, dst, src1, space)                do { using namespace uml; block.append().fswrite(dst, src1, space); } while (0)
#define UML_FSMOV(block, dst, src1)                         do { using namespace uml; block.append().fsmov(dst, src1); } while (0)
#define UML_FSMOVc(block, cond, dst, src1)                  do { using namespace uml; block.append().fsmov(cond, dst, src1); } while (0)
#define UML_FSTOINT(block, dst, src1, size, round)          do { using namespace uml; block.append().fstoint(dst, src1, size, round); } while (0)
#define UML_FSFRINT(block, dst, src1, size)                 do { using namespace uml; block.append().fsfrint(dst, src1, size); } while (0)
#define UML_FSFRFLT(block, dst, src1, size)                 do { using namespace uml; block.append().fsfrflt(dst, src1, size); } while (0)
#define UML_FSADD(block, dst, src1, src2)                   do { using namespace uml; block.append().fsadd(dst, src1, src2); } while (0)
#define UML_FSSUB(block, dst, src1, src2)                   do { using namespace uml; block.append().fssub(dst, src1, src2); } while (0)
#define UML_FSCMP(block, src1, src2)                        do { using namespace uml; block.append().fscmp(src1, src2); } while (0)
#define UML_FSMUL(block, dst, src1, src2)                   do { using namespace uml; block.append().fsmul(dst, src1, src2); } while (0)
#define UML_FSDIV(block, dst, src1, src2)                   do { using namespace uml; block.append().fsdiv(dst, src1, src2); } while (0)
#define UML_FSNEG(block, dst, src1)                         do { using namespace uml; block.append().fsneg(dst, src1); } while (0)
#define UML_FSABS(block, dst, src1)                         do { using namespace uml; block.append().fsabs(dst, src1); } while (0)
#define UML_FSSQRT(block, dst, src1)                        do { using namespace uml; block.append().fssqrt(dst, src1); } while (0)
#define UML_FSRECIP(block, dst, src1)                       do { using namespace uml; block.append().fsrecip(dst, src1); } while (0)
#define UML_FSRSQRT(block, dst, src1)                       do { using namespace uml; block.append().fsrsqrt(dst, src1); } while (0)
#define UML_FSCOPYI(block, dst, src)                        do { using namespace uml; block.append().fscopyi(dst, src); } while (0)
#define UML_ICOPYFS(block, dst, src)                        do { using namespace uml; block.append().icopyfs(dst, src); } while (0)


/* ----- 64-bit Floating Point Arithmetic Operations ----- */
#define UML_FDLOAD(block, dst, base, index)                 do { using namespace uml; block.append().fdload(dst, base, index); } while (0)
#define UML_FDSTORE(block, base, index, src1)               do { using namespace uml; block.append().fdstore(base, index, src1); } while (0)
#define UML_FDREAD(block, dst, src1, space)                 do { using namespace uml; block.append().fdread(dst, src1, space); } while (0)
#define UML_FDWRITE(block, dst, src1, space)                do { using namespace uml; block.append().fdwrite(dst, src1, space); } while (0)
#define UML_FDMOV(block, dst, src1)                         do { using namespace uml; block.append().fdmov(dst, src1); } while (0)
#define UML_FDMOVc(block, cond, dst, src1)                  do { using namespace uml; block.append().fdmov(cond, dst, src1); } while (0)
#define UML_FDTOINT(block, dst, src1, size, round)          do { using namespace uml; block.append().fdtoint(dst, src1, size, round); } while (0)
#define UML_FDFRINT(block, dst, src1, size)                 do { using namespace uml; block.append().fdfrint(dst, src1, size); } while (0)
#define UML_FDFRFLT(block, dst, src1, size)                 do { using namespace uml; block.append().fdfrflt(dst, src1, size); } while (0)
#define UML_FDRNDS(block, dst, src1)                        do { using namespace uml; block.append().fdrnds(dst, src1); } while (0)
#define UML_FDADD(block, dst, src1, src2)                   do { using namespace uml; block.append().fdadd(dst, src1, src2); } while (0)
#define UML_FDSUB(block, dst, src1, src2)                   do { using namespace uml; block.append().fdsub(dst, src1, src2); } while (0)
#define UML_FDCMP(block, src1, src2)                        do { using namespace uml; block.append().fdcmp(src1, src2); } while (0)
#define UML_FDMUL(block, dst, src1, src2)                   do { using namespace uml; block.append().fdmul(dst, src1, src2); } while (0)
#define UML_FDDIV(block, dst, src1, src2)                   do { using namespace uml; block.append().fddiv(dst, src1, src2); } while (0)
#define UML_FDNEG(block, dst, src1)                         do { using namespace uml; block.append().fdneg(dst, src1); } while (0)
#define UML_FDABS(block, dst, src1)                         do { using namespace uml; block.append().fdabs(dst, src1); } while (0)
#define UML_FDSQRT(block, dst, src1)                        do { using namespace uml; block.append().fdsqrt(dst, src1); } while (0)
#define UML_FDRECIP(block, dst, src1)                       do { using namespace uml; block.append().fdrecip(dst, src1); } while (0)
#define UML_FDRSQRT(block, dst, src1)                       do { using namespace uml; block.append().fdrsqrt(dst, src1); } while (0)
#define UML_FDCOPYI(block, dst, src)                        do { using namespace uml; block.append().fdcopyi(dst, src); } while (0)
#define UML_ICOPYFD(block, dst, src)                        do { using namespace uml; block.append().icopyfd(dst, src); } while (0)


#endif // MAME_CPU_DRCUMLSH_H
