/****************************************************************************/
/*            real mode i286 emulator by Fabrice Frances                    */
/*           (initial work based on David Hedley's pcemu)                   */
/*                                                                          */
/****************************************************************************/
#ifndef __I86_H_
#define __I86_H_

#define I8086_NMI_INT_VECTOR 2
#define INPUT_LINE_TEST 20    /* PJB 03/05 */

typedef enum { ES, CS, SS, DS } SREGS;
typedef enum { AX, CX, DX, BX, SP, BP, SI, DI } WREGS;

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#ifdef LSB_FIRST
typedef enum { AL,AH,CL,CH,DL,DH,BL,BH,SPL,SPH,BPL,BPH,SIL,SIH,DIL,DIH } BREGS;
#else
typedef enum { AH,AL,CH,CL,DH,DL,BH,BL,SPH,SPL,BPH,BPL,SIH,SIL,DIH,DIL } BREGS;
#endif

/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)			(I.TF = (x))
#define SetIF(x)			(I.IF = (x))
#define SetDF(x)			(I.DirVal = (x) ? -1 : 1)

#define SetOFW_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define SetCFB(x)			(I.CarryVal = (x) & 0x100)
#define SetCFW(x)			(I.CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)		(I.AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)			(I.SignVal = (x))
#define SetZF(x)			(I.ZeroVal = (x))
#define SetPF(x)			(I.ParityVal = (x))

#define SetSZPF_Byte(x) 	(I.ParityVal = I.SignVal = I.ZeroVal = (INT8)(x))
#define SetSZPF_Word(x) 	(I.ParityVal = I.SignVal = I.ZeroVal = (INT16)(x))

#define ADDB(dst,src) { unsigned res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW(dst,src) { unsigned res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB(dst,src) { unsigned res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW(dst,src) { unsigned res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB(dst,src) 		dst |= src; I.CarryVal = I.OverVal = I.AuxVal = 0; SetSZPF_Byte(dst)
#define ORW(dst,src) 		dst |= src; I.CarryVal = I.OverVal = I.AuxVal = 0; SetSZPF_Word(dst)

#define ANDB(dst,src) 		dst &= src; I.CarryVal = I.OverVal = I.AuxVal = 0; SetSZPF_Byte(dst)
#define ANDW(dst,src) 		dst &= src; I.CarryVal = I.OverVal = I.AuxVal = 0; SetSZPF_Word(dst)

#define XORB(dst,src) 		dst ^= src; I.CarryVal = I.OverVal = I.AuxVal = 0; SetSZPF_Byte(dst)
#define XORW(dst,src) 		dst ^= src; I.CarryVal = I.OverVal = I.AuxVal = 0; SetSZPF_Word(dst)

#define CF					(I.CarryVal != 0)
#define SF					(I.SignVal < 0)
#define ZF					(I.ZeroVal == 0)
#define PF					parity_table[I.ParityVal]
#define AF					(I.AuxVal != 0)
#define OF					(I.OverVal != 0)
#define DF					(I.DirVal < 0)

/************************************************************************/

#define read_byte(a)			(*I.mem.rbyte)(a)
#define read_word(a)			(*I.mem.rword)(a)
#define write_byte(a,d)			(*I.mem.wbyte)((a),(d))
#define write_word(a,d)			(*I.mem.wword)((a),(d))

#define read_port_byte(a)		(*I.mem.rbyte_port)(a)
#define read_port_word(a)		(*I.mem.rword_port)(a)
#define write_port_byte(a,d)	(*I.mem.wbyte_port)((a),(d))
#define write_port_word(a,d)	(*I.mem.wword_port)((a),(d))

/************************************************************************/

#define SegBase(Seg) 			(I.sregs[Seg] << 4)

#define DefaultBase(Seg) 		((seg_prefix && (Seg == DS || Seg == SS)) ? prefix_base : I.base[Seg])

#define GetMemB(Seg,Off)		(read_byte((DefaultBase(Seg) + (Off)) & AMASK))
#define GetMemW(Seg,Off)		(read_word((DefaultBase(Seg) + (Off)) & AMASK))
#define PutMemB(Seg,Off,x)		write_byte((DefaultBase(Seg) + (Off)) & AMASK, (x))
#define PutMemW(Seg,Off,x)		write_word((DefaultBase(Seg) + (Off)) & AMASK, (x))

#define PEEKBYTE(ea) 			(read_byte((ea) & AMASK))
#define ReadByte(ea) 			(read_byte((ea) & AMASK))
#define ReadWord(ea)			(read_word((ea) & AMASK))
#define WriteByte(ea,val)		write_byte((ea) & AMASK, val);
#define WriteWord(ea,val)		write_word((ea) & AMASK, val);

#define FETCH_XOR(a)			((a) ^ I.mem.fetch_xor)
#define FETCH					(cpu_readop_arg(FETCH_XOR(I.pc++)))
#define FETCHOP					(cpu_readop(FETCH_XOR(I.pc++)))
#define PEEKOP(addr)			(cpu_readop(FETCH_XOR(addr)))
#define FETCHWORD(var) 			{ var = cpu_readop_arg(FETCH_XOR(I.pc)); var += (cpu_readop_arg(FETCH_XOR(I.pc + 1)) << 8); I.pc += 2; }
#define CHANGE_PC(addr)			change_pc(addr)
#define PUSH(val)				{ I.regs.w[SP] -= 2; WriteWord(((I.base[SS] + I.regs.w[SP]) & AMASK), val); }
#define POP(var)				{ var = ReadWord(((I.base[SS] + I.regs.w[SP]) & AMASK)); I.regs.w[SP] += 2; }

/************************************************************************/

#define CompressFlags() (WORD)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (I.TF << 8) | (I.IF << 9) \
				| (DF << 10) | (OF << 11))

#define ExpandFlags(f) \
{ \
	  I.CarryVal = (f) & 1; \
	  I.ParityVal = !((f) & 4); \
	  I.AuxVal = (f) & 16; \
	  I.ZeroVal = !((f) & 64); \
	  I.SignVal = ((f) & 128) ? -1 : 0; \
	  I.TF = ((f) & 256) >> 8; \
	  I.IF = ((f) & 512) >> 9; \
	  I.DirVal = ((f) & 1024) ? -1 : 1; \
	  I.OverVal = (f) & 2048; \
}
#endif

