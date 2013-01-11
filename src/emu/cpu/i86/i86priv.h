/****************************************************************************/
/*            real mode i286 emulator by Fabrice Frances                    */
/*           (initial work based on David Hedley's pcemu)                   */
/*                                                                          */
/****************************************************************************/
#pragma once

#ifndef __I86_H__
#define __I86_H__

#define I8086_NMI_INT_VECTOR 2

enum SREGS { ES, CS, SS, DS };
enum WREGS { AX, CX, DX, BX, SP, BP, SI, DI };

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif


enum BREGS {
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
};

/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)            (cpustate->TF = (x))
#define SetIF(x)            (cpustate->IF = (x))
#define SetDF(x)            (cpustate->DirVal = (x) ? -1 : 1)

#define SetOFW_Add(x,y,z)   (cpustate->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)   (cpustate->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)   (cpustate->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)   (cpustate->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define SetCFB(x)           (cpustate->CarryVal = (x) & 0x100)
#define SetCFW(x)           (cpustate->CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)        (cpustate->AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)            (cpustate->SignVal = (x))
#define SetZF(x)            (cpustate->ZeroVal = (x))
#define SetPF(x)            (cpustate->ParityVal = (x))

#define SetSZPF_Byte(x)     (cpustate->ParityVal = cpustate->SignVal = cpustate->ZeroVal = (INT8)(x))
#define SetSZPF_Word(x)     (cpustate->ParityVal = cpustate->SignVal = cpustate->ZeroVal = (INT16)(x))

#define ADDB(dst,src) { unsigned res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW(dst,src) { unsigned res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB(dst,src) { unsigned res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW(dst,src) { unsigned res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

// don't modify CF in case fault occurs
#define ADCB(dst,src,tmpcf) { unsigned res=dst+src; tmpcf = res & 0x100; SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADCW(dst,src,tmpcf) { unsigned res=dst+src; tmpcf = res & 0x10000; SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SBBB(dst,src,tmpcf) { unsigned res=dst-src; tmpcf = res & 0x100; SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SBBW(dst,src,tmpcf) { unsigned res=dst-src; tmpcf = res & 0x10000; SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB(dst,src)        dst |= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Byte(dst)
#define ORW(dst,src)        dst |= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Word(dst)

#define ANDB(dst,src)       dst &= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Byte(dst)
#define ANDW(dst,src)       dst &= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Word(dst)

#define XORB(dst,src)       dst ^= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Byte(dst)
#define XORW(dst,src)       dst ^= src; cpustate->CarryVal = cpustate->OverVal = cpustate->AuxVal = 0; SetSZPF_Word(dst)

#define CF                  (int)(cpustate->CarryVal != 0)
#define SF                  (int)(cpustate->SignVal < 0)
#define ZF                  (int)(cpustate->ZeroVal == 0)
#define PF                  parity_table[cpustate->ParityVal&0xff]
#define AF                  (int)(cpustate->AuxVal != 0)
#define OF                  (int)(cpustate->OverVal != 0)
#define DF                  (int)(cpustate->DirVal < 0)

/************************************************************************/

#define read_mem_byte(a)            cpustate->program->read_byte(a)
#define read_mem_word(a)            cpustate->program->read_word_unaligned(a)
#define write_mem_byte(a,d)         cpustate->program->write_byte((a),(d))
#define write_mem_word(a,d)         cpustate->program->write_word_unaligned((a),(d))

#define read_port_byte(a)       cpustate->io->read_byte(a)
#define read_port_word(a)       cpustate->io->read_word_unaligned(a)
#define write_port_byte(a,d)    cpustate->io->write_byte((a),(d))
#define write_port_word(a,d)    cpustate->io->write_word_unaligned((a),(d))

/************************************************************************/

#define SegBase(Seg)            (cpustate->sregs[Seg] << 4)

#define DefaultSeg(Seg)         ((cpustate->seg_prefix && (Seg == DS || Seg == SS)) ? cpustate->prefix_seg : Seg)
#define DefaultBase(Seg)        ((cpustate->seg_prefix && (Seg == DS || Seg == SS)) ? cpustate->base[cpustate->prefix_seg] : cpustate->base[Seg])

#ifdef I80286
#define GetMemB(Seg,Off)        (read_mem_byte(GetMemAddr(cpustate,Seg,Off,1,I80286_READ)))
#define GetMemW(Seg,Off)        (read_mem_word(GetMemAddr(cpustate,Seg,Off,2,I80286_READ)))
#define PutMemB(Seg,Off,x)      write_mem_byte(GetMemAddr(cpustate,Seg,Off,1,I80286_WRITE), (x))
#define PutMemW(Seg,Off,x)      write_mem_word(GetMemAddr(cpustate,Seg,Off,2,I80286_WRITE), (x))
#else
#define GetMemB(Seg,Off)        (read_mem_byte((DefaultBase(Seg) + (Off)) & AMASK))
#define GetMemW(Seg,Off)        (read_mem_word((DefaultBase(Seg) + (Off)) & AMASK))
#define PutMemB(Seg,Off,x)      write_mem_byte((DefaultBase(Seg) + (Off)) & AMASK, (x))
#define PutMemW(Seg,Off,x)      write_mem_word((DefaultBase(Seg) + (Off)) & AMASK, (x))
#endif

#define PEEKBYTE(ea)            (read_mem_byte((ea) & AMASK))
#define ReadByte(ea)            (read_mem_byte((ea) & AMASK))
#define ReadWord(ea)            (read_mem_word((ea) & AMASK))
#define WriteByte(ea,val)       write_mem_byte((ea) & AMASK, val);
#define WriteWord(ea,val)       write_mem_word((ea) & AMASK, val);

#define FETCH                   (cpustate->direct->read_raw_byte(cpustate->pc++, cpustate->fetch_xor))
#define FETCHOP                 (cpustate->direct->read_decrypted_byte(cpustate->pc++, cpustate->fetch_xor))
#define PEEKOP(addr)            (cpustate->direct->read_decrypted_byte(addr, cpustate->fetch_xor))
#define FETCHWORD(var)          { var = cpustate->direct->read_raw_byte(cpustate->pc, cpustate->fetch_xor); var += (cpustate->direct->read_raw_byte(cpustate->pc + 1, cpustate->fetch_xor) << 8); cpustate->pc += 2; }
#define CHANGE_PC(addr)
#ifdef I80286
#define PUSH(val)               { if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP]-2, I80286_WORD, I80286_WRITE); cpustate->regs.w[SP] -= 2; WriteWord(((cpustate->base[SS] + cpustate->regs.w[SP]) & AMASK), val); }
#define POP(var)                { if(PM) i80286_check_permission(cpustate, SS, cpustate->regs.w[SP], I80286_WORD, I80286_READ); cpustate->regs.w[SP] += 2; var = ReadWord(((cpustate->base[SS] + ((cpustate->regs.w[SP]-2) & 0xffff)) & AMASK)); }
#else
#define PUSH(val)               { cpustate->regs.w[SP] -= 2; WriteWord(((cpustate->base[SS] + cpustate->regs.w[SP]) & AMASK), val); }
#define POP(var)                { cpustate->regs.w[SP] += 2; var = ReadWord(((cpustate->base[SS] + ((cpustate->regs.w[SP]-2) & 0xffff)) & AMASK)); }
#endif
/************************************************************************/
#ifdef I80286
#define IOPL ((cpustate->flags&0x3000)>>12)
#define NT ((cpustate->flags&0x4000)>>14)
#define xF (0)
#else
#define IOPL (3)
#define NT (1)
#define xF (1)
#endif

#define CompressFlags() (WORD)(CF | 2 |(PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (cpustate->TF << 8) | (cpustate->IF << 9) \
				| (DF << 10) | (OF << 11) | (IOPL << 12) | (NT << 14) | (xF << 15))

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
