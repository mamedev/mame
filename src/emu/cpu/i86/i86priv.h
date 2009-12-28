/****************************************************************************/
/*            real mode i286 emulator by Fabrice Frances                    */
/*           (initial work based on David Hedley's pcemu)                   */
/*                                                                          */
/****************************************************************************/
#pragma once

#ifndef __I86_H__
#define __I86_H__

#define I8086_NMI_INT_VECTOR 2

typedef enum { ES, CS, SS, DS } SREGS;
typedef enum { AX, CX, DX, BX, SP, BP, SI, DI } WREGS;

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif


typedef enum {
   AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
   AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
   CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
   CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
   DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
   DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
   BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
   BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6),
  SPL = NATIVE_ENDIAN_VALUE_LE_BE(0x8, 0x9),
  SPH = NATIVE_ENDIAN_VALUE_LE_BE(0x9, 0x8),
  BPL = NATIVE_ENDIAN_VALUE_LE_BE(0xa, 0xb),
  BPH = NATIVE_ENDIAN_VALUE_LE_BE(0xb, 0xa),
  SIL = NATIVE_ENDIAN_VALUE_LE_BE(0xc, 0xd),
  SIH = NATIVE_ENDIAN_VALUE_LE_BE(0xd, 0xc),
  DIL = NATIVE_ENDIAN_VALUE_LE_BE(0xe, 0xf),
  DIH = NATIVE_ENDIAN_VALUE_LE_BE(0xf, 0xe)
} BREGS;

/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)			(cpustate->TF = (x))
#define SetIF(x)			(cpustate->IF = (x))
#define SetDF(x)			(cpustate->DirVal = (x) ? -1 : 1)

#define SetOFW_Add(x,y,z)	(cpustate->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(cpustate->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(cpustate->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(cpustate->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define SetCFB(x)			(cpustate->CarryVal = (x) & 0x100)
#define SetCFW(x)			(cpustate->CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)		(cpustate->AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)			(cpustate->SignVal = (x))
#define SetZF(x)			(cpustate->ZeroVal = (x))
#define SetPF(x)			(cpustate->ParityVal = (x))

#define SetSZPF_Byte(x) 	(cpustate->ParityVal = cpustate->SignVal = cpustate->ZeroVal = (INT8)(x))
#define SetSZPF_Word(x) 	(cpustate->ParityVal = cpustate->SignVal = cpustate->ZeroVal = (INT16)(x))

#define ADDB(dst,src) { unsigned res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW(dst,src) { unsigned res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB(dst,src) { unsigned res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW(dst,src) { unsigned res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB(dst,src)		dst |= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Byte(dst)
#define ORW(dst,src)		dst |= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Word(dst)

#define ANDB(dst,src)		dst &= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Byte(dst)
#define ANDW(dst,src)		dst &= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Word(dst)

#define XORB(dst,src)		dst ^= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Byte(dst)
#define XORW(dst,src)		dst ^= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Word(dst)

#define CF					(cpustate->CarryVal != 0)
#define SF					(cpustate->SignVal < 0)
#define ZF					(cpustate->ZeroVal == 0)
#define PF					parity_table[cpustate->ParityVal]
#define AF					(cpustate->AuxVal != 0)
#define OF					(cpustate->OverVal != 0)
#define DF					(cpustate->DirVal < 0)

/************************************************************************/

#define read_byte(a)			(*cpustate->mem.rbyte)(cpustate->program, a)
#define read_word(a)			(*cpustate->mem.rword)(cpustate->program, a)
#define write_byte(a,d)			(*cpustate->mem.wbyte)(cpustate->program, (a),(d))
#define write_word(a,d)			(*cpustate->mem.wword)(cpustate->program, (a),(d))

#define read_port_byte(a)		(*cpustate->mem.rbyte)(cpustate->io, a)
#define read_port_word(a)		(*cpustate->mem.rword)(cpustate->io, a)
#define write_port_byte(a,d)	(*cpustate->mem.wbyte)(cpustate->io, (a),(d))
#define write_port_word(a,d)	(*cpustate->mem.wword)(cpustate->io, (a),(d))

/************************************************************************/

#define SegBase(Seg)			(cpustate->sregs[Seg] << 4)

#define DefaultBase(Seg)		((cpustate->seg_prefix && (Seg == DS || Seg == SS)) ? cpustate->prefix_base : cpustate->base[Seg])

#define GetMemB(Seg,Off)		(read_byte((DefaultBase(Seg) + (Off)) & AMASK))
#define GetMemW(Seg,Off)		(read_word((DefaultBase(Seg) + (Off)) & AMASK))
#define PutMemB(Seg,Off,x)		write_byte((DefaultBase(Seg) + (Off)) & AMASK, (x))
#define PutMemW(Seg,Off,x)		write_word((DefaultBase(Seg) + (Off)) & AMASK, (x))

#define PEEKBYTE(ea)			(read_byte((ea) & AMASK))
#define ReadByte(ea)			(read_byte((ea) & AMASK))
#define ReadWord(ea)			(read_word((ea) & AMASK))
#define WriteByte(ea,val)		write_byte((ea) & AMASK, val);
#define WriteWord(ea,val)		write_word((ea) & AMASK, val);

#define FETCH_XOR(a)			((a) ^ cpustate->mem.fetch_xor)
#define FETCH					(memory_raw_read_byte(cpustate->program, FETCH_XOR(cpustate->pc++)))
#define FETCHOP					(memory_decrypted_read_byte(cpustate->program, FETCH_XOR(cpustate->pc++)))
#define PEEKOP(addr)			(memory_decrypted_read_byte(cpustate->program, FETCH_XOR(addr)))
#define FETCHWORD(var)			{ var = memory_raw_read_byte(cpustate->program, FETCH_XOR(cpustate->pc)); var += (memory_raw_read_byte(cpustate->program, FETCH_XOR(cpustate->pc + 1)) << 8); cpustate->pc += 2; }
#define CHANGE_PC(addr)
#define PUSH(val)				{ cpustate->regs.w[SP] -= 2; WriteWord(((cpustate->base[SS] + cpustate->regs.w[SP]) & AMASK), val); }
#define POP(var)				{ var = ReadWord(((cpustate->base[SS] + cpustate->regs.w[SP]) & AMASK)); cpustate->regs.w[SP] += 2; }

/************************************************************************/

#define CompressFlags() (WORD)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (cpustate->TF << 8) | (cpustate->IF << 9) \
				| (DF << 10) | (OF << 11))

#define ExpandFlags(f) \
{ \
	  cpustate->CarryVal = (f) & 1; \
	  cpustate->ParityVal = !((f) & 4); \
	  cpustate->AuxVal = (f) & 16; \
	  cpustate->ZeroVal = !((f) & 64); \
	  cpustate->SignVal = ((f) & 128) ? -1 : 0; \
	  cpustate->TF = ((f) & 256) >> 8; \
	  cpustate->IF = ((f) & 512) >> 9; \
	  cpustate->DirVal = ((f) & 1024) ? -1 : 1; \
	  cpustate->OverVal = (f) & 2048; \
}

#endif /* __I86_H__ */
