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

/* shorthand for flags */
#define FLAGS_ALLI	(DRCUML_FLAG_C | DRCUML_FLAG_V | DRCUML_FLAG_Z | DRCUML_FLAG_S)
#define FLAGS_ALLF	(DRCUML_FLAG_C | DRCUML_FLAG_Z | DRCUML_FLAG_U)
#define FLAGS_NONE	(0)

/* flags needed for condition codes */
#define FLAGS_Z		(DRCUML_FLAG_Z)
#define FLAGS_NZ	(DRCUML_FLAG_Z)
#define FLAGS_E		(DRCUML_FLAG_Z)
#define FLAGS_NE	(DRCUML_FLAG_Z)
#define FLAGS_S		(DRCUML_FLAG_S)
#define FLAGS_NS	(DRCUML_FLAG_S)
#define FLAGS_C		(DRCUML_FLAG_C)
#define FLAGS_NC	(DRCUML_FLAG_C)
#define FLAGS_V		(DRCUML_FLAG_V)
#define FLAGS_NV	(DRCUML_FLAG_V)
#define FLAGS_U		(DRCUML_FLAG_U)
#define FLAGS_NU	(DRCUML_FLAG_U)
#define FLAGS_A		(DRCUML_FLAG_C | DRCUML_FLAG_Z)
#define FLAGS_BE	(DRCUML_FLAG_C | DRCUML_FLAG_Z)
#define FLAGS_B		(DRCUML_FLAG_C)
#define FLAGS_AE	(DRCUML_FLAG_C)
#define FLAGS_G		(DRCUML_FLAG_S | DRCUML_FLAG_V | DRCUML_FLAG_Z)
#define FLAGS_LE	(DRCUML_FLAG_S | DRCUML_FLAG_V | DRCUML_FLAG_Z)
#define FLAGS_L		(DRCUML_FLAG_S | DRCUML_FLAG_V)
#define FLAGS_GE	(DRCUML_FLAG_S | DRCUML_FLAG_V)



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
#define UML_SAVE(block, dst)								do { drcuml_block_append_1(block, DRCUML_OP_SAVE,    4, IF_ALWAYS,  MEM(dst)); } while (0)
#define UML_RESTORE(block, src)								do { drcuml_block_append_1(block, DRCUML_OP_RESTORE, 4, IF_ALWAYS,  MEM(src)); } while (0)


/* ----- 32-Bit Integer Operations ----- */
#define UML_LOAD1U(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_LOAD1U,  4, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_LOAD1S(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_LOAD1S,  4, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_LOAD2U(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_LOAD2U,  4, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_LOAD2S(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_LOAD2S,  4, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_LOAD4(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_LOAD4U,  4, IF_ALWAYS,  dst, MEM(base), index); } while (0)

#define UML_STORE1(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_STORE1,  4, IF_ALWAYS,  MEM(base), index, src1); } while (0)
#define UML_STORE2(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_STORE2,  4, IF_ALWAYS,  MEM(base), index, src1); } while (0)
#define UML_STORE4(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_STORE4,  4, IF_ALWAYS,  MEM(base), index, src1); } while (0)

#define UML_READ1U(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_READ1U,  4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_READ1S(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_READ1S,  4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_READ2U(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_READ2U,  4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_READ2S(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_READ2S,  4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_READ2M(block, dst, space, src1, mask)			do { drcuml_block_append_4(block, DRCUML_OP_READ2M,  4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1, mask); } while (0)
#define UML_READ4(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_READ4U,  4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_READ4M(block, dst, space, src1, mask)			do { drcuml_block_append_4(block, DRCUML_OP_READ4M,  4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1, mask); } while (0)

#define UML_WRITE1(block, space, dst, src1)					do { drcuml_block_append_3(block, DRCUML_OP_WRITE1,  4, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)
#define UML_WRITE2(block, space, dst, src1)					do { drcuml_block_append_3(block, DRCUML_OP_WRITE2,  4, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)
#define UML_WRIT2M(block, space, dst, mask, src1)			do { drcuml_block_append_4(block, DRCUML_OP_WRIT2M,  4, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, mask, src1); } while (0)
#define UML_WRITE4(block, space, dst, src1)					do { drcuml_block_append_3(block, DRCUML_OP_WRITE4,  4, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)
#define UML_WRIT4M(block, space, dst, mask, src1)			do { drcuml_block_append_4(block, DRCUML_OP_WRIT4M,  4, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, mask, src1); } while (0)

#define UML_FLAGS(block, dst, mask, table)					do { drcuml_block_append_3(block, DRCUML_OP_FLAGS,   4, IF_ALWAYS,  dst, IMM(mask), MEM(table)); } while (0)
#define UML_SETC(block, src, bitnum)						do { drcuml_block_append_2(block, DRCUML_OP_SETC,    4, IF_ALWAYS,  src, bitnum); } while (0)

#define UML_MOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_MOV,     4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_MOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_MOV,     4, cond,       dst, src1); } while (0)

#define UML_ZEXT1(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_ZEXT1,   4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_ZEXT2(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_ZEXT2,   4, IF_ALWAYS,  dst, src1); } while (0)

#define UML_SEXT1(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_SEXT1,   4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_SEXT2(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_SEXT2,   4, IF_ALWAYS,  dst, src1); } while (0)

#define UML_XTRACT(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_XTRACT,  4, IF_ALWAYS,  dst, src, shift, mask); } while (0)
#define UML_INSERT(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_INSERT,  4, IF_ALWAYS,  dst, src, shift, mask); } while (0)

#define UML_NEG(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_NEG,     4, FLAGS_NONE, dst, src1); } while (0)
#define UML_NEGf(block, dst, src1, flags)					do { drcuml_block_append_2(block, DRCUML_OP_NEG,     4, flags,      dst, src1); } while (0)

#define UML_ADD(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_ADD,     4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_ADDf(block, dst, src1, src2, flags)				do { drcuml_block_append_3(block, DRCUML_OP_ADD,     4, flags,      dst, src1, src2); } while (0)
#define UML_ADDC(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_ADDC,    4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_ADDCf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_ADDC,    4, flags,      dst, src1, src2); } while (0)

#define UML_SUB(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_SUB,     4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_SUBf(block, dst, src1, src2, flags)				do { drcuml_block_append_3(block, DRCUML_OP_SUB,     4, flags,      dst, src1, src2); } while (0)
#define UML_SUBB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_SUBB,    4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_SUBBf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_SUBB,    4, flags,      dst, src1, src2); } while (0)

#define UML_CMP(block, src1, src2)							do { drcuml_block_append_2(block, DRCUML_OP_CMP,     4, FLAGS_ALLI, src1, src2); } while (0)
#define UML_CMPf(block, src1, src2, flags)					do { drcuml_block_append_2(block, DRCUML_OP_CMP,     4, flags,      src1, src2); } while (0)

#define UML_MULU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULU,    4, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_MULUf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_MULU,    4, flags,      dst, edst, src1, src2); } while (0)
#define UML_MULS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULS,    4, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_MULSf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_MULS,    4, flags,      dst, edst, src1, src2); } while (0)

#define UML_DIVU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVU,    4, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_DIVUf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_DIVU,    4, flags,      dst, edst, src1, src2); } while (0)
#define UML_DIVS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVS,    4, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_DIVSf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_DIVS,    4, flags,      dst, edst, src1, src2); } while (0)

#define UML_AND(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_AND,     4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_ANDf(block, dst, src1, src2, flags)				do { drcuml_block_append_3(block, DRCUML_OP_AND,     4, flags,      dst, src1, src2); } while (0)

#define UML_TEST(block, src1, src2)							do { drcuml_block_append_2(block, DRCUML_OP_TEST,    4, FLAGS_S|FLAGS_Z,src1, src2); } while (0)
#define UML_TESTf(block, src1, src2, flags)					do { drcuml_block_append_2(block, DRCUML_OP_TEST,    4, flags,      src1, src2); } while (0)

#define UML_OR(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_OR,      4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_ORf(block, dst, src1, src2, flags)				do { drcuml_block_append_3(block, DRCUML_OP_OR,      4, flags,      dst, src1, src2); } while (0)

#define UML_XOR(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_XOR,     4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_XORf(block, dst, src1, src2, flags)				do { drcuml_block_append_3(block, DRCUML_OP_XOR,     4, flags,      dst, src1, src2); } while (0)

#define UML_LZCNT(block, dst, src)							do { drcuml_block_append_2(block, DRCUML_OP_LZCNT,   4, FLAGS_NONE, dst, src); } while (0)
#define UML_BSWAP(block, dst, src)							do { drcuml_block_append_2(block, DRCUML_OP_BSWAP,   4, FLAGS_NONE, dst, src); } while (0)

#define UML_SHL(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_SHL,     4, FLAGS_NONE, dst, src, count); } while (0)
#define UML_SHLf(block, dst, src, count, flags)				do { drcuml_block_append_3(block, DRCUML_OP_SHL,     4, flags,      dst, src, count); } while (0)

#define UML_SHR(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_SHR,     4, FLAGS_NONE, dst, src, count); } while (0)
#define UML_SHRf(block, dst, src, count, flags)				do { drcuml_block_append_3(block, DRCUML_OP_SHR,     4, flags,      dst, src, count); } while (0)

#define UML_SAR(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_SAR,     4, FLAGS_NONE, dst, src, count); } while (0)
#define UML_SARf(block, dst, src, count, flags)				do { drcuml_block_append_3(block, DRCUML_OP_SAR,     4, flags,      dst, src, count); } while (0)

#define UML_ROL(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_ROL,     4, FLAGS_NONE, dst, src, count); } while (0)
#define UML_ROLf(block, dst, src, count, flags)				do { drcuml_block_append_3(block, DRCUML_OP_ROL,     4, flags,      dst, src, count); } while (0)
#define UML_ROLC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROLC,    4, FLAGS_NONE, dst, src, count); } while (0)
#define UML_ROLCf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_ROLC,    4, flags,      dst, src, count); } while (0)

#define UML_ROR(block, dst, src, count)						do { drcuml_block_append_3(block, DRCUML_OP_ROR,     4, FLAGS_NONE, dst, src, count); } while (0)
#define UML_RORf(block, dst, src, count, flags)				do { drcuml_block_append_3(block, DRCUML_OP_ROR,     4, flags,      dst, src, count); } while (0)
#define UML_RORC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_RORC,    4, FLAGS_NONE, dst, src, count); } while (0)
#define UML_RORCf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_RORC,    4, flags,      dst, src, count); } while (0)


/* ----- 64-Bit Integer Operations ----- */
#define UML_DLOAD1U(block, dst, base, index)				do { drcuml_block_append_3(block, DRCUML_OP_LOAD1U,  8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_DLOAD1S(block, dst, base, index)				do { drcuml_block_append_3(block, DRCUML_OP_LOAD1S,  8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_DLOAD2U(block, dst, base, index)				do { drcuml_block_append_3(block, DRCUML_OP_LOAD2U,  8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_DLOAD2S(block, dst, base, index)				do { drcuml_block_append_3(block, DRCUML_OP_LOAD2S,  8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_DLOAD4U(block, dst, base, index)				do { drcuml_block_append_3(block, DRCUML_OP_LOAD4U,  8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_DLOAD4S(block, dst, base, index)				do { drcuml_block_append_3(block, DRCUML_OP_LOAD4S,  8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_DLOAD8(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_LOAD8U,  8, IF_ALWAYS,  dst, MEM(base), index); } while (0)

#define UML_DSTORE1(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_STORE1,  8, IF_ALWAYS,  MEM(base), index, src1); } while (0)
#define UML_DSTORE2(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_STORE2,  8, IF_ALWAYS,  MEM(base), index, src1); } while (0)
#define UML_DSTORE4(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_STORE4,  8, IF_ALWAYS,  MEM(base), index, src1); } while (0)
#define UML_DSTORE8(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_STORE8,  8, IF_ALWAYS,  MEM(base), index, src1); } while (0)

#define UML_DREAD1U(block, dst, space, src1)				do { drcuml_block_append_3(block, DRCUML_OP_READ1U,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_DREAD1S(block, dst, space, src1)				do { drcuml_block_append_3(block, DRCUML_OP_READ1S,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_DREAD2U(block, dst, space, src1)				do { drcuml_block_append_3(block, DRCUML_OP_READ2U,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_DREAD2S(block, dst, space, src1)				do { drcuml_block_append_3(block, DRCUML_OP_READ2S,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_DREAD2M(block, dst, space, src1, mask)			do { drcuml_block_append_4(block, DRCUML_OP_READ2M,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1, mask); } while (0)
#define UML_DREAD4U(block, dst, space, src1)				do { drcuml_block_append_3(block, DRCUML_OP_READ4U,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_DREAD4S(block, dst, space, src1)				do { drcuml_block_append_3(block, DRCUML_OP_READ4S,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_DREAD4M(block, dst, space, src1, mask)			do { drcuml_block_append_4(block, DRCUML_OP_READ4M,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1, mask); } while (0)
#define UML_DREAD8(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_READ8U,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_DREAD8M(block, dst, space, src1, mask)			do { drcuml_block_append_4(block, DRCUML_OP_READ8M,  8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1, mask); } while (0)

#define UML_DWRITE1(block, space, dst, src1)				do { drcuml_block_append_3(block, DRCUML_OP_WRITE1,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)
#define UML_DWRITE2(block, space, dst, src1)				do { drcuml_block_append_3(block, DRCUML_OP_WRITE2,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)
#define UML_DWRIT2M(block, space, dst, mask, src1)			do { drcuml_block_append_4(block, DRCUML_OP_WRIT2M,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, mask, src1); } while (0)
#define UML_DWRITE4(block, space, dst, src1)				do { drcuml_block_append_3(block, DRCUML_OP_WRITE4,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)
#define UML_DWRIT4M(block, space, dst, mask, src1)			do { drcuml_block_append_4(block, DRCUML_OP_WRIT4M,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, mask, src1); } while (0)
#define UML_DWRITE8(block, space, dst, src1)				do { drcuml_block_append_3(block, DRCUML_OP_WRITE8,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)
#define UML_DWRIT8M(block, space, dst, mask, src1)			do { drcuml_block_append_4(block, DRCUML_OP_WRIT8M,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, mask, src1); } while (0)

#define UML_DFLAGS(block, dst, mask, table)					do { drcuml_block_append_3(block, DRCUML_OP_FLAGS,   8, IF_ALWAYS,  dst, IMM(mask), MEM(table)); } while (0)

#define UML_DMOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_MOV,     8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_DMOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_MOV,     8, cond,       dst, src1); } while (0)

#define UML_DZEXT1(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_ZEXT1,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_DZEXT2(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_ZEXT2,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_DZEXT4(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_ZEXT4,   8, IF_ALWAYS,  dst, src1); } while (0)

#define UML_DSEXT1(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_SEXT1,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_DSEXT2(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_SEXT2,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_DSEXT4(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_SEXT4,   8, IF_ALWAYS,  dst, src1); } while (0)

#define UML_DXTRACT(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_XTRACT,  8, IF_ALWAYS,  dst, src, shift, mask); } while (0)
#define UML_DINSERT(block, dst, src, shift, mask)			do { drcuml_block_append_4(block, DRCUML_OP_INSERT,  8, IF_ALWAYS,  dst, src, shift, mask); } while (0)

#define UML_DNEG(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_NEG,     8, FLAGS_NONE, dst, src1); } while (0)
#define UML_DNEGf(block, dst, src1, flags)					do { drcuml_block_append_2(block, DRCUML_OP_NEG,     8, flags,      dst, src1); } while (0)

#define UML_DADD(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_ADD,     8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_DADDf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_ADD,     8, flags,      dst, src1, src2); } while (0)
#define UML_DADDC(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_ADDC,    8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_DADDCf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_ADDC,    8, flags,      dst, src1, src2); } while (0)

#define UML_DSUB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_SUB,     8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_DSUBf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_SUB,     8, flags,      dst, src1, src2); } while (0)
#define UML_DSUBB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_SUBB,    8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_DSUBBf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_SUBB,    8, flags,      dst, src1, src2); } while (0)

#define UML_DCMP(block, src1, src2)							do { drcuml_block_append_2(block, DRCUML_OP_CMP,     8, FLAGS_ALLI, src1, src2); } while (0)
#define UML_DCMPf(block, src1, src2, flags)					do { drcuml_block_append_2(block, DRCUML_OP_CMP,     8, flags,      src1, src2); } while (0)

#define UML_DMULU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULU,    8, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_DMULUf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_MULU,    8, flags,      dst, edst, src1, src2); } while (0)
#define UML_DMULS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_MULS,    8, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_DMULSf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_MULS,    8, flags,      dst, edst, src1, src2); } while (0)

#define UML_DDIVU(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVU,    8, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_DDIVUf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_DIVU,    8, flags,      dst, edst, src1, src2); } while (0)
#define UML_DDIVS(block, dst, edst, src1, src2)				do { drcuml_block_append_4(block, DRCUML_OP_DIVS,    8, FLAGS_NONE, dst, edst, src1, src2); } while (0)
#define UML_DDIVSf(block, dst, edst, src1, src2, flags)		do { drcuml_block_append_4(block, DRCUML_OP_DIVS,    8, flags,      dst, edst, src1, src2); } while (0)

#define UML_DAND(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_AND,     8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_DANDf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_AND,     8, flags,      dst, src1, src2); } while (0)

#define UML_DTEST(block, src1, src2)						do { drcuml_block_append_2(block, DRCUML_OP_TEST,    8, FLAGS_S|FLAGS_Z,src1, src2); } while (0)
#define UML_DTESTf(block, src1, src2, flags)				do { drcuml_block_append_2(block, DRCUML_OP_TEST,    8, flags,      src1, src2); } while (0)

#define UML_DOR(block, dst, src1, src2)						do { drcuml_block_append_3(block, DRCUML_OP_OR,      8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_DORf(block, dst, src1, src2, flags)				do { drcuml_block_append_3(block, DRCUML_OP_OR,      8, flags,      dst, src1, src2); } while (0)

#define UML_DXOR(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_XOR,     8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_DXORf(block, dst, src1, src2, flags)			do { drcuml_block_append_3(block, DRCUML_OP_XOR,     8, flags,      dst, src1, src2); } while (0)

#define UML_DSHL(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_SHL,     8, FLAGS_NONE, dst, src, count); } while (0)
#define UML_DSHLf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_SHL,     8, flags,      dst, src, count); } while (0)

#define UML_DSHR(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_SHR,     8, FLAGS_NONE, dst, src, count); } while (0)
#define UML_DSHRf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_SHR,     8, flags,      dst, src, count); } while (0)

#define UML_DSAR(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_SAR,     8, FLAGS_NONE, dst, src, count); } while (0)
#define UML_DSARf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_SAR,     8, flags,      dst, src, count); } while (0)

#define UML_DROL(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROL,     8, FLAGS_NONE, dst, src, count); } while (0)
#define UML_DROLf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_ROL,     8, flags,      dst, src, count); } while (0)
#define UML_DROLC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROLC,    8, FLAGS_NONE, dst, src, count); } while (0)
#define UML_DROLCf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_ROLC,    8, flags,      dst, src, count); } while (0)

#define UML_DROR(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_ROR,     8, FLAGS_NONE, dst, src, count); } while (0)
#define UML_DRORf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_ROR,     8, flags,      dst, src, count); } while (0)
#define UML_DRORC(block, dst, src, count)					do { drcuml_block_append_3(block, DRCUML_OP_RORC,    8, FLAGS_NONE, dst, src, count); } while (0)
#define UML_DRORCf(block, dst, src, count, flags)			do { drcuml_block_append_3(block, DRCUML_OP_RORC,    8, flags,      dst, src, count); } while (0)


/* ----- 32-bit Floating Point Arithmetic Operations ----- */
#define UML_FSLOAD(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_FLOAD,   4, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_FSSTORE(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_FSTORE,  4, IF_ALWAYS,  MEM(base), index, src1); } while (0)

#define UML_FSREAD(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_FREAD,   4, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_FSWRITE(block, space, dst, src1)				do { drcuml_block_append_3(block, DRCUML_OP_FWRITE,  4, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)

#define UML_FSMOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSMOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    4, cond,       dst, src1); } while (0)

#define UML_FSTOI4(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4,   4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI4T(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4T,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI4R(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4R,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI4C(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4C,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI4F(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4F,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI8(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8,   4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI8T(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8T,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI8R(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8R,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI8C(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8C,  4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSTOI8F(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8F,  4, IF_ALWAYS,  dst, src1); } while (0)

#define UML_FSFRFD(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FFRFD,   4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSFRI4(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FFRI4,   4, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FSFRI8(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FFRI8,   4, IF_ALWAYS,  dst, src1); } while (0)

#define UML_FSADD(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FADD,    4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FSSUB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FSUB,    4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FSCMP(block, src1, src2)						do { drcuml_block_append_2(block, DRCUML_OP_FCMP,    4, FLAGS_ALLF, src1, src2); } while (0)
#define UML_FSMUL(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FMUL,    4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FSDIV(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FDIV,    4, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FSNEG(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FNEG,    4, FLAGS_NONE, dst, src1); } while (0)
#define UML_FSABS(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FABS,    4, FLAGS_NONE, dst, src1); } while (0)
#define UML_FSSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FSQRT,   4, FLAGS_NONE, dst, src1); } while (0)
#define UML_FSRECIP(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRECIP,  4, FLAGS_NONE, dst, src1); } while (0)
#define UML_FSRSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRSQRT,  4, FLAGS_NONE, dst, src1); } while (0)


/* ----- 64-bit Floating Point Arithmetic Operations ----- */
#define UML_FDLOAD(block, dst, base, index)					do { drcuml_block_append_3(block, DRCUML_OP_FLOAD,   8, IF_ALWAYS,  dst, MEM(base), index); } while (0)
#define UML_FDSTORE(block, base, index, src1)				do { drcuml_block_append_3(block, DRCUML_OP_FSTORE,  8, IF_ALWAYS,  MEM(base), index, src1); } while (0)

#define UML_FDREAD(block, dst, space, src1)					do { drcuml_block_append_3(block, DRCUML_OP_FREAD,   8, IF_ALWAYS,  dst, IMM(ADDRESS_SPACE_##space), src1); } while (0)
#define UML_FDWRITE(block, space, dst, src1)				do { drcuml_block_append_3(block, DRCUML_OP_FWRITE,  8, IF_ALWAYS,  IMM(ADDRESS_SPACE_##space), dst, src1); } while (0)

#define UML_FDMOV(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDMOVc(block, cond, dst, src1)					do { drcuml_block_append_2(block, DRCUML_OP_FMOV,    8, cond,       dst, src1); } while (0)

#define UML_FDTOI4(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI4T(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4T,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI4R(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4R,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI4C(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4C,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI4F(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI4F,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI8(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI8T(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8T,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI8R(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8R,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI8C(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8C,  8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDTOI8F(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FTOI8F,  8, IF_ALWAYS,  dst, src1); } while (0)

#define UML_FDFRFS(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FFRFS,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDFRI4(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FFRI4,   8, IF_ALWAYS,  dst, src1); } while (0)
#define UML_FDFRI8(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FFRI8,   8, IF_ALWAYS,  dst, src1); } while (0)

#define UML_FDADD(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FADD,    8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FDSUB(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FSUB,    8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FDCMP(block, src1, src2)						do { drcuml_block_append_2(block, DRCUML_OP_FCMP,    8, FLAGS_ALLF, src1, src2); } while (0)
#define UML_FDMUL(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FMUL,    8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FDDIV(block, dst, src1, src2)					do { drcuml_block_append_3(block, DRCUML_OP_FDIV,    8, FLAGS_NONE, dst, src1, src2); } while (0)
#define UML_FDNEG(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FNEG,    8, FLAGS_NONE, dst, src1); } while (0)
#define UML_FDABS(block, dst, src1)							do { drcuml_block_append_2(block, DRCUML_OP_FABS,    8, FLAGS_NONE, dst, src1); } while (0)
#define UML_FDSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FSQRT,   8, FLAGS_NONE, dst, src1); } while (0)
#define UML_FDRECIP(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRECIP,  8, FLAGS_NONE, dst, src1); } while (0)
#define UML_FDRSQRT(block, dst, src1)						do { drcuml_block_append_2(block, DRCUML_OP_FRSQRT,  8, FLAGS_NONE, dst, src1); } while (0)


#endif
