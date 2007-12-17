#ifndef V810_H
#define V810_H

#include "cpuintrf.h"

extern int v810_ICount;

void v810_get_info(UINT32, cpuinfo*);

#define R_B(addr) (program_read_byte_32le(addr))
#define R_H(addr) (program_read_word_32le(addr))
#define R_W(addr) (program_read_dword_32le(addr))


#define W_B(addr, val) (program_write_byte_32le(addr,val))
#define W_H(addr, val) (program_write_word_32le(addr,val))
#define W_W(addr, val) (program_write_dword_32le(addr,val))


#define RIO_B(addr) (io_read_byte_32le(addr))
#define RIO_H(addr) (io_read_word_32le(addr))
#define RIO_W(addr) (io_read_dword_32le(addr))


#define WIO_B(addr, val) (io_write_byte_32le(addr,val))
#define WIO_H(addr, val) (io_write_word_32le(addr,val))
#define WIO_W(addr, val) (io_write_dword_32le(addr,val))

#define R_OP(addr)	(R_H(addr))

#define GET1 (OP&0x1f)
#define GET2 ((OP>>5)&0x1f)
#define I5(x) (((x)&0x1f)|(((x)&0x10)?0xffffffe0:0))
#define UI5(x) ((x)&0x1f)
#define I16(x) (((x)&0xffff)|(((x)&0x8000)?0xffff0000:0))
#define UI16(x) ((x)&0xffff)
#define D16(x) (((x)&0xffff)|(((x)&0x8000)?0xffff0000:0))
#define D26(x,y) ((y)|((x&0x3ff)<<16 )|((x&0x200)?0xfc000000:0))
#define D9(x) ((x&0x1ff)|((x&0x100)?0xfffffe00:0))
#define SO(opcode) ((opcode)&0xfc00)>>10)

#define CHECK_CY(x)	PSW=(PSW & ~8)|(((x) & (((UINT64)1) << 32)) ? 8 : 0)
#define CHECK_OVADD(x,y,z)	PSW=(PSW & ~0x00000004) |(( ((x) ^ (z)) & ((y) ^ (z)) & 0x80000000) ? 4: 0)
#define CHECK_OVSUB(x,y,z)	PSW=(PSW & ~0x00000004) |(( ((y) ^ (z)) & ((x) ^ (y)) & 0x80000000) ? 4: 0)
#define CHECK_ZS(x)	PSW=(PSW & ~3)|((UINT32)(x)==0)|(((x)&0x80000000) ? 2: 0)


#define ADD(dst, src)		{ UINT64 res=(UINT64)(dst)+(UINT64)(src); SetCF(res); SetOF_Add(res,src,dst); SetSZPF(res); dst=(UINT32)res; }
#define SUB(dst, src)		{ UINT64 res=(UINT64)(dst)-(INT64)(src); SetCF(res); SetOF_Sub(res,src,dst); SetSZPF(res); dst=(UINT32)res; }

#ifdef MAME_DEBUG
offs_t v810_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

enum
{
	V810_R0=1,
	V810_R1,
	V810_R2,  /* R2 - handler stack pointer */
	V810_SP,  /* R3 - stack pointer */
	V810_R4,  /* R4 - global pointer */
	V810_R5,  /* R5 - text pointer */
	V810_R6,
	V810_R7,
	V810_R8,
	V810_R9,
	V810_R10,
	V810_R11,
	V810_R12,
	V810_R13,
	V810_R14,
	V810_R15,
	V810_R16,
	V810_R17,
	V810_R18,
	V810_R19,
	V810_R20,
	V810_R21,
	V810_R22,
	V810_R23,
	V810_R24,
	V810_R25,
	V810_R26,
	V810_R27,
	V810_R28,
	V810_R29,
	V810_R30,
	V810_R31, /* R31 - link pointer */

	/* System Registers */
	V810_EIPC, /* Exception/interrupt  saving - PC */
	V810_EIPSW,/* Exception/interrupt  saving - PSW */
	V810_FEPC, /* Duplexed exception/NMI  saving - PC */
	V810_FEPSW,/* Duplexed exception/NMI  saving - PSW */
	V810_ECR,  /* Exception cause register */
	V810_PSW,  /* Program status word */
	V810_PIR,  /* Processor ID register */
	V810_TKCW, /* Task control word */
	V810_res08,
	V810_res09,
	V810_res10,
	V810_res11,
	V810_res12,
	V810_res13,
	V810_res14,
	V810_res15,
	V810_res16,
	V810_res17,
	V810_res18,
	V810_res19,
	V810_res20,
	V810_res21,
	V810_res22,
	V810_res23,
	V810_CHCW,  /* Cache control word */
	V810_ADTRE, /* Address trap register */
	V810_res26,
	V810_res27,
	V810_res28,
	V810_res29,
	V810_res30,
	V810_res31,

	V810_PC
};
#endif


