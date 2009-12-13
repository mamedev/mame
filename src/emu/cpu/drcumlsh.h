/***************************************************************************

    drcumlsh.h

    Shorthand definitions for the universal machine language.

    Copyright Aaron Giles
    Released for general non-commercial use under the MAME license
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __DRCUMLSH_H__
#define __DRCUMLSH_H__

#include "drcuml.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

/* shorthand for conditions */
#define IF_Z		DRCUML_COND_Z
#define IF_NZ		DRCUML_COND_NZ
#define IF_S		DRCUML_COND_S
#define IF_NS		DRCUML_COND_NS
#define IF_C		DRCUML_COND_C
#define IF_NC		DRCUML_COND_NC
#define IF_V		DRCUML_COND_V
#define IF_NV		DRCUML_COND_NV
#define IF_U		DRCUML_COND_U
#define IF_NU		DRCUML_COND_NU
#define IF_E		DRCUML_COND_E
#define IF_NE		DRCUML_COND_NE
#define IF_A		DRCUML_COND_A
#define IF_AE		DRCUML_COND_AE
#define IF_B		DRCUML_COND_B
#define IF_BE		DRCUML_COND_BE
#define IF_G		DRCUML_COND_G
#define IF_GE		DRCUML_COND_GE
#define IF_L		DRCUML_COND_L
#define IF_LE		DRCUML_COND_LE
#define IF_ALWAYS	DRCUML_COND_ALWAYS



/***************************************************************************
    MACROS
***************************************************************************/

/* macros for wrapping parameters */
#define NONE				DRCUML_PTYPE_NONE, 0
#define IMM(x)				DRCUML_PTYPE_IMMEDIATE, (x)
#define IREG(x)				DRCUML_PTYPE_INT_REGISTER, (DRCUML_REG_I0 + (x))
#define FREG(x)				DRCUML_PTYPE_FLOAT_REGISTER, (DRCUML_REG_F0 + (x))
#define MVAR(x)				DRCUML_PTYPE_MAPVAR, (DRCUML_MAPVAR_M0 + (x))
#define MEM(x)				DRCUML_PTYPE_MEMORY, (FPTR)(x)
#define PARAM(t,v)			(t), (v)



/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

/* ----- Compile-time Opcodes ----- */
#define UML_HANDLE(block, handle)							do { drcuml_block_append_1(block, DRCUML_OP_HANDLE,  4, IF_ALWAYS,  MEM(handle)); } while (0)
#define UML_HASH(block, mode, pc)							do { drcuml_block_append_2(block, DRCUML_OP_HASH,    4, IF_ALWAYS,  IMM(mode), IMM(pc)); } while (0)
#define UML_LABEL(block, label)								do { drcuml_block_append_1(block, DRCUML_OP_LABEL,   4, IF_ALWAYS,  IMM(label)); } while (0)
#define UML_COMMENT/*(block, format, ...)*/					drcuml_add_comment
#define UML_MAPVAR(block, mapvar, value)					do { drcuml_block_append_2(block, DRCUML_OP_MAPVAR,  4, IF_ALWAYS,  mapvar, IMM(value)); } while (0)


/* ----- Control Flow Operations ----- */
#define UML_NOP(block)										do { drcuml_block_append_0(block, DRCUML_OP_NOP,     4, IF_ALWAYS); } while (0)
#define UML_DEBUG(block, pc)								do { drcuml_block_append_1(block, DRCUML_OP_DEBUG,   4, IF_ALWAYS,  pc); } while (0)
#define UML_EXIT(block, param)								do { drcuml_block_append_1(block, DRCUML_OP_EXIT,    4, IF_ALWAYS,  param); } while (0)
#define UML_EXITc(block, cond, param)						do { drcuml_block_append_1(block, DRCUML_OP_EXIT,    4, cond,       param); } while (0)
#define UML_HASHJMP(block, mode, pc, handle)				do { drcuml_block_append_3(block, DRCUML_OP_HASHJMP, 4, IF_ALWAYS,  mode, pc, MEM(handle)); } while (0)
#define UML_JMP(block, label)								do { drcuml_block_append_1(block, DRCUML_OP_JMP,     4, IF_ALWAYS,  IMM(label)); } while (0)
#define UML_JMPc(block, cond, label)						do { drcuml_block_append_1(block, DRCUML_OP_JMP,     4, cond,       IMM(label)); } while (0)
#define UML_JMPH(block, handle)								do { drcuml_block_append_1(block, DRCUML_OP_JMPH,    4, IF_ALWAYS,  MEM(handle)); } while (0)
#define UML_JMPHc(block, cond, handle)						do { drcuml_block_append_1(block, DRCUML_OP_JMPH,    4, cond,       MEM(handle)); } while (0)
#define UML_EXH(block, handle, param)						do { drcuml_block_append_2(block, DRCUML_OP_EXH,     4, IF_ALWAYS,  MEM(handle), param); } while (0)
#define UML_EXHc(block, cond, handle, param)				do { drcuml_block_append_2(block, DRCUML_OP_EXH,     4, cond,       MEM(handle), param); } while (0)
#define UML_CALLH(block, handle)							do { drcuml_block_append_1(block, DRCUML_OP_CALLH,   4, IF_ALWAYS,  MEM(handle)); } while (0)
#define UML_CALLHc(block, cond, handle)						do { drcuml_block_append_1(block, DRCUML_OP_CALLH,   4, cond,       MEM(handle)); } while (0)
#define UML_RET(block)										do { drcuml_block_append_0(block, DRCUML_OP_RET,     4, IF_ALWAYS); } while (0)
#define UML_RETc(block, cond)								do { drcuml_block_append_0(block, DRCUML_OP_RET,     4, cond); } while (0)
#define UML_CALLC(block, func, ptr)							do { drcuml_block_append_2(block, DRCUML_OP_CALLC,   4, IF_ALWAYS,  MEM(func), MEM(ptr)); } while (0)
#define UML_CALLCc(block, cond, func, ptr)					do { drcuml_block_append_2(block, DRCUML_OP_CALLC,   4, cond,       MEM(func), MEM(ptr)); } while (0)
#define UML_RECOVER(block, dst, mapvar)						do { drcuml_block_append_2(block, DRCUML_OP_RECOVER, 4, IF_ALWAYS,  dst, mapvar); } while (0)


/* ----- Internal Register Operations ----- */
#define UML_SETFMOD(block, mode)							do { drcuml_block_append_1(block, DRCUML_OP_SETFMOD, 4, IF_ALWAYS,  mode); } while (0)
#define UML_GETFMOD(block, dst)								do { drcuml_block_append_1(block, DRCUML_OP_GETFMOD, 4, IF_ALWAYS,  dst); } while (0)
#define UML_GETEXP(block, dst)								do { drcuml_block_append_1(block, DRCUML_OP_GETEXP,  4, IF_ALWAYS,  dst); } while (0)
#define UML_GETFLGS(block, dst, flags)						do { drcuml_block_append_2(block, DRCUML_OP_GETFLGS, 4, IF_ALWAYS,  dst, IMM(flags)); } while (0)
#define UML_SAVE(block, dst)								do { drcuml_block_append_1(block, DRCUML_OP_SAVE,    4, IF_ALWAYS,  MEM(dst)); } while (0)
#define UML_RESTORE(block, src)								do { drcuml_block_append_1(block, DRCUML_OP_RESTORE, 4, IF_ALWAYS,  MEM(src)); } while (0)


/* ----- 32-Bit Integer Operations ----- */
#define UML_LOAD(block, dst, base, index, size)				do { drcuml_block_append_4(block, DRCUML_OP_LOAD,    4, IF_ALWAYS,  dst, MEM(base), index, IMM(DRCUML_SCSIZE_##size)); } while (0)
#define UML_LOADS(block, dst, base, index, size)			do { drcuml_block_append_4(block, DRCUML_OP_LOADS,   4, IF_ALWAYS,  dst, MEM(base), index, IMM(DRCUML_SCSIZE_##size)); } while (0)
#define UML_STORE(block, base, index, src1, size)			do { drcuml_block_append_4(block, DRCUML_OP_STORE,   4, IF_ALWAYS,  MEM(base), index, src1, IMM(DRCUML_SCSIZE_##size)); } while (0)
#define UML_READ(block, dst, src1, spsize)					do { drcuml_block_append_3(block, DRCUML_OP_READ,    4, IF_ALWAYS,  dst, src1, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_READM(block, dst, src1, mask, spsize)			do { drcuml_block_append_4(block, DRCUML_OP_READM,   4, IF_ALWAYS,  dst, src1, mask, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_WRITE(block, dst, src1, spsize)					do { drcuml_block_append_3(block, DRCUML_OP_WRITE,   4, IF_ALWAYS,  dst, src1, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_WRITEM(block, dst, src1, mask, spsize)			do { drcuml_block_append_4(block, DRCUML_OP_WRITEM,  4, IF_ALWAYS,  dst, src1, mask, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_CARRY(block, src, bitnum)						do { drcuml_block_append_2(block, DRCUML_OP_CARRY,   4, IF_ALWAYS,  src, bitnum); } while (0)
#define UML_SETc(block, cond, dst)							do { drcuml_block_append_1(block, DRCUML_OP_SET,     4, cond,       dst); } while (0)
#define UML_MOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_MOV,     4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_MOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_MOV,     4, cond,       dst, src1); } while (0)
#define UML_SEXT(block, dst, src1, size)					do { drcuml_block_append_3(block, DRCUML_OP_SEXT,    4, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size)); } while (0)
#define UML_ROLAND(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_ROLAND,  4, IF_ALWAYS,  dst, src, shift, mask); } while (0)
#define UML_ROLINS(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_ROLINS,  4, IF_ALWAYS,  dst, src, shift, mask); } while (0)
#define UML_ADD(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_ADD,     4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_ADDC(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_ADDC,    4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_SUB(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_SUB,     4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_SUBB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_SUBB,    4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_CMP(block, src1, src2)							do { drcuml_block_append_2(block, DRCUML_OP_CMP,     4, IF_ALWAYS,  src1, src2); } while (0)
#define UML_MULU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULU,    4, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_MULS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULS,    4, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_DIVU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVU,    4, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_DIVS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVS,    4, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_AND(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_AND,     4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_TEST(block, src1, src2)							do { drcuml_block_append_2(block, DRCUML_OP_TEST,    4, IF_ALWAYS,  src1, src2); } while (0)
#define UML_OR(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_OR,      4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_XOR(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_XOR,     4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_LZCNT(block, dst, src)							do { drcuml_block_append_2(block, DRCUML_OP_LZCNT,   4, IF_ALWAYS,  dst, src); } while (0)
#define UML_BSWAP(block, dst, src)							do { drcuml_block_append_2(block, DRCUML_OP_BSWAP,   4, IF_ALWAYS,  dst, src); } while (0)
#define UML_SHL(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_SHL,     4, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_SHR(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_SHR,     4, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_SAR(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_SAR,     4, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_ROL(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_ROL,     4, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_ROLC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROLC,    4, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_ROR(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_ROR,     4, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_RORC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_RORC,    4, IF_ALWAYS,  dst, src, count); } while (0)


/* ----- 64-Bit Integer Operations ----- */
#define UML_DLOAD(block, dst, base, index, size)			do { drcuml_block_append_4(block, DRCUML_OP_LOAD,    8, IF_ALWAYS,  dst, MEM(base), index, IMM(DRCUML_SCSIZE_##size)); } while (0)
#define UML_DLOADS(block, dst, base, index, size)			do { drcuml_block_append_4(block, DRCUML_OP_LOADS,   8, IF_ALWAYS,  dst, MEM(base), index, IMM(DRCUML_SCSIZE_##size)); } while (0)
#define UML_DSTORE(block, base, index, src1, size)			do { drcuml_block_append_4(block, DRCUML_OP_STORE,   8, IF_ALWAYS,  MEM(base), index, src1, IMM(DRCUML_SCSIZE_##size)); } while (0)
#define UML_DREAD(block, dst, src1, spsize)					do { drcuml_block_append_3(block, DRCUML_OP_READ,    8, IF_ALWAYS,  dst, src1, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_DREADM(block, dst, src1, mask, spsize)			do { drcuml_block_append_4(block, DRCUML_OP_READM,   8, IF_ALWAYS,  dst, src1, mask, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_DWRITE(block, dst, src1, spsize)				do { drcuml_block_append_3(block, DRCUML_OP_WRITE,   8, IF_ALWAYS,  dst, src1, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_DWRITEM(block, dst, src1, mask, spsize)			do { drcuml_block_append_4(block, DRCUML_OP_WRITEM,  8, IF_ALWAYS,  dst, src1, mask, IMM(DRCUML_SPSIZE_##spsize)); } while (0)
#define UML_DCARRY(block, src, bitnum)						do { drcuml_block_append_2(block, DRCUML_OP_CARRY,   8, IF_ALWAYS,  src, bitnum); } while (0)
#define UML_DSETc(block, cond, dst)							do { drcuml_block_append_1(block, DRCUML_OP_SET,     8, cond,       dst); } while (0)
#define UML_DMOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_MOV,     8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_DMOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_MOV,     8, cond,       dst, src1); } while (0)
#define UML_DSEXT(block, dst, src1, size)					do { drcuml_block_append_3(block, DRCUML_OP_SEXT,    8, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size)); } while (0)
#define UML_DROLAND(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_ROLAND,  8, IF_ALWAYS,  dst, src, shift, mask); } while (0)
#define UML_DROLINS(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_ROLINS,  8, IF_ALWAYS,  dst, src, shift, mask); } while (0)
#define UML_DADD(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_ADD,     8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_DADDC(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_ADDC,    8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_DSUB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_SUB,     8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_DSUBB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_SUBB,    8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_DCMP(block, src1, src2)							do { drcuml_block_append_2(block, DRCUML_OP_CMP,     8, IF_ALWAYS,  src1, src2); } while (0)
#define UML_DMULU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULU,    8, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_DMULS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULS,    8, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_DDIVU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVU,    8, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_DDIVS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVS,    8, IF_ALWAYS,  dst, edst, src1, src2); } while (0)
#define UML_DAND(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_AND,     8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_DTEST(block, src1, src2)						do { drcuml_block_append_2(block, DRCUML_OP_TEST,    8, IF_ALWAYS,  src1, src2); } while (0)
#define UML_DOR(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_OR,      8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_DXOR(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_XOR,     8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_DSHL(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_SHL,     8, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_DSHR(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_SHR,     8, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_DSAR(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_SAR,     8, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_DROL(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROL,     8, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_DROLC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROLC,    8, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_DROR(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROR,     8, IF_ALWAYS,  dst, src, count); } while (0)
#define UML_DRORC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_RORC,    8, IF_ALWAYS,  dst, src, count); } while (0)


/* ----- 32-bit Floating Point Arithmetic Operations ----- */
#define UML_FSLOAD(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_FLOAD,   4, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_FSSTORE(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_FSTORE,  4, IF_ALWAYS,  MEM(base), index, src1); } while (0)
#define UML_FSREAD(block, dst, src1, space)					do { drcuml_block_append_3(block, DRCUML_OP_FREAD,   4, IF_ALWAYS,  dst, src1, IMM(ADDRESS_SPACE_##space)); } while (0)
#define UML_FSWRITE(block, dst, src1, space)				do { drcuml_block_append_3(block, DRCUML_OP_FWRITE,  4, IF_ALWAYS,  dst, src1, IMM(ADDRESS_SPACE_##space)); } while (0)
#define UML_FSMOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSMOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    4, cond,       dst, src1); } while (0)
#define UML_FSTOINT(block, dst, src1, size, round)			do { drcuml_block_append_4(block, DRCUML_OP_FTOINT,  4, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size), IMM(DRCUML_FMOD_##round)); } while (0)
#define UML_FSFRINT(block, dst, src1, size)					do { drcuml_block_append_3(block, DRCUML_OP_FFRINT,  4, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size)); } while (0)
#define UML_FSFRFLT(block, dst, src1, size)					do { drcuml_block_append_3(block, DRCUML_OP_FFRFLT,  4, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size)); } while (0)
#define UML_FSADD(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FADD,    4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FSSUB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FSUB,    4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FSCMP(block, src1, src2)						do { drcuml_block_append_2(block, DRCUML_OP_FCMP,    4, IF_ALWAYS,  src1, src2); } while (0)
#define UML_FSMUL(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FMUL,    4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FSDIV(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FDIV,    4, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FSNEG(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FNEG,    4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSABS(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FABS,    4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FSQRT,   4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSRECIP(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRECIP,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSRSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRSQRT,  4, IF_ALWAYS,  dst, src1); } while (0)


/* ----- 64-bit Floating Point Arithmetic Operations ----- */
#define UML_FDLOAD(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_FLOAD,   8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_FDSTORE(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_FSTORE,  8, IF_ALWAYS,  MEM(base), index, src1); } while (0)
#define UML_FDREAD(block, dst, src1, space)					do { drcuml_block_append_3(block, DRCUML_OP_FREAD,   8, IF_ALWAYS,  dst, src1, IMM(ADDRESS_SPACE_##space)); } while (0)
#define UML_FDWRITE(block, dst, src1, space)				do { drcuml_block_append_3(block, DRCUML_OP_FWRITE,  8, IF_ALWAYS,  dst, src1, IMM(ADDRESS_SPACE_##space)); } while (0)
#define UML_FDMOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDMOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    8, cond,       dst, src1); } while (0)
#define UML_FDTOINT(block, dst, src1, size, round)			do { drcuml_block_append_4(block, DRCUML_OP_FTOINT,  8, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size), IMM(DRCUML_FMOD_##round)); } while (0)
#define UML_FDFRINT(block, dst, src1, size)					do { drcuml_block_append_3(block, DRCUML_OP_FFRINT,  8, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size)); } while (0)
#define UML_FDFRFLT(block, dst, src1, size)					do { drcuml_block_append_3(block, DRCUML_OP_FFRFLT,  8, IF_ALWAYS,  dst, src1, IMM(DRCUML_SIZE_##size)); } while (0)
#define UML_FDRNDS(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRNDS,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDADD(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FADD,    8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FDSUB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FSUB,    8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FDCMP(block, src1, src2)						do { drcuml_block_append_2(block, DRCUML_OP_FCMP,    8, IF_ALWAYS,  src1, src2); } while (0)
#define UML_FDMUL(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FMUL,    8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FDDIV(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FDIV,    8, IF_ALWAYS,  dst, src1, src2); } while (0)
#define UML_FDNEG(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FNEG,    8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDABS(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FABS,    8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FSQRT,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDRECIP(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRECIP,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDRSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRSQRT,  8, IF_ALWAYS,  dst, src1); } while (0)


#endif /* __DRCUMLSH_H__ */
