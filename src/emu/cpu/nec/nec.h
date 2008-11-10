#include "cpuintrf.h"

typedef struct _nec_config nec_config;
struct _nec_config
{
	const UINT8*	v25v35_decryptiontable; // internal decryption table
};


typedef enum { DS1, PS, SS, DS0 } SREGS;
typedef enum { AW, CW, DW, BW, SP, BP, IX, IY } WREGS;

#define NEC_NMI_INT_VECTOR	2
#define NEC_INPUT_LINE_POLL 20

/* Cpu types, steps of 8 to help the cycle count calculation */
#define V33 0
#define V30 8
#define V20 16

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

#ifdef LSB_FIRST
typedef enum { AL,AH,CL,CH,DL,DH,BL,BH,SPL,SPH,BPL,BPH,IXL,IXH,IYL,IYH } BREGS;
#else
typedef enum { AH,AL,CH,CL,DH,DL,BH,BL,SPH,SPL,BPH,BPL,IXH,IXL,IYH,IYL } BREGS;
#endif

/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)		(I.TF = (x))
#define SetIF(x)		(I.IF = (x))
#define SetDF(x)		(I.DF = (x))
#define SetMD(x)		(I.MF = (x))	/* OB [19.07.99] Mode Flag V30 */

#define SetCFB(x)		(I.CarryVal = (x) & 0x100)
#define SetCFW(x)		(I.CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)	(I.AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)		(I.SignVal = (x))
#define SetZF(x)		(I.ZeroVal = (x))
#define SetPF(x)		(I.ParityVal = (x))

#define SetSZPF_Byte(x) (I.SignVal=I.ZeroVal=I.ParityVal=(INT8)(x))
#define SetSZPF_Word(x) (I.SignVal=I.ZeroVal=I.ParityVal=(INT16)(x))

#define SetOFW_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define ADDB { UINT32 res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW { UINT32 res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB { UINT32 res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW { UINT32 res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB dst|=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define ORW dst|=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define ANDB dst&=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define ANDW dst&=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define XORB dst^=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define XORW dst^=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define CF		(I.CarryVal!=0)
#define SF		(I.SignVal<0)
#define ZF		(I.ZeroVal==0)
#define PF		parity_table[(BYTE)I.ParityVal]
#define AF		(I.AuxVal!=0)
#define OF		(I.OverVal!=0)
#define MD		(I.MF!=0)

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

#define CHANGE_PC do { EMPTY_PREFETCH(); change_pc((I.sregs[PS]<<4) + I.ip); } while (0)

#define SegBase(Seg) (I.sregs[Seg] << 4)

#define DefaultBase(Seg) ((I.seg_prefix && (Seg==DS0 || Seg==SS)) ? I.prefix_base : I.sregs[Seg] << 4)

#define GetMemB(Seg,Off) (read_byte(DefaultBase(Seg) + (Off)))
#define GetMemW(Seg,Off) (read_word(DefaultBase(Seg) + (Off)))

#define PutMemB(Seg,Off,x) { write_byte(DefaultBase(Seg) + (Off), (x)); }
#define PutMemW(Seg,Off,x) { write_word(DefaultBase(Seg) + (Off), (x)); }

/* prefetch timing */

#define FETCH() 			fetch()
#define FETCH_XOR(a)		((a) ^ I.mem.fetch_xor)
#define FETCHWORD()			fetchword()
#define EMPTY_PREFETCH()	I.prefetch_reset = 1


#define PUSH(val) { I.regs.w[SP]-=2; write_word((((I.sregs[SS]<<4)+I.regs.w[SP])),val); }
#define POP(var) { var = read_word((((I.sregs[SS]<<4)+I.regs.w[SP]))); I.regs.w[SP]+=2; }

#define GetModRM UINT32 ModRM=FETCH()

/* Cycle count macros:
    CLK  - cycle count is the same on all processors
    CLKS - cycle count differs between processors, list all counts
    CLKW - cycle count for word read/write differs for odd/even source/destination address
    CLKM - cycle count for reg/mem instructions
    CLKR - cycle count for reg/mem instructions with different counts for odd/even addresses


    Prefetch & buswait time is not emulated.
    Extra cycles for PUSH'ing or POP'ing registers to odd addresses is not emulated.
*/

#define CLK(all) nec_ICount-=all
#define CLKS(v20,v30,v33) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33; nec_ICount-=(ccount>>I.chip_type)&0x7f; }
#define CLKW(v20o,v30o,v33o,v20e,v30e,v33e,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; nec_ICount-=(addr&1)?((ocount>>I.chip_type)&0x7f):((ecount>>I.chip_type)&0x7f); }
#define CLKM(v20,v30,v33,v20m,v30m,v33m) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33, mcount=(v20m<<16)|(v30m<<8)|v33m; nec_ICount-=( ModRM >=0xc0 )?((ccount>>I.chip_type)&0x7f):((mcount>>I.chip_type)&0x7f); }
#define CLKR(v20o,v30o,v33o,v20e,v30e,v33e,vall,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; if (ModRM >=0xc0) nec_ICount-=vall; else nec_ICount-=(addr&1)?((ocount>>I.chip_type)&0x7f):((ecount>>I.chip_type)&0x7f); }

/************************************************************************/
#define CompressFlags() (WORD)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (I.TF << 8) | (I.IF << 9) \
				| (I.DF << 10) | (OF << 11)| (MD << 15))

#define ExpandFlags(f) \
{ \
	I.CarryVal = (f) & 1; \
	I.ParityVal = !((f) & 4); \
	I.AuxVal = (f) & 16; \
	I.ZeroVal = !((f) & 64); \
	I.SignVal = (f) & 128 ? -1 : 0; \
	I.TF = ((f) & 256) == 256; \
	I.IF = ((f) & 512) == 512; \
	I.DF = ((f) & 1024) == 1024; \
	I.OverVal = (f) & 2048; \
	I.MF = ((f) & 0x8000) == 0x8000; \
}

#define IncWordReg(Reg) 					\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	I.OverVal = (tmp == 0x7fff); 			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1

#define DecWordReg(Reg) 					\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
    unsigned tmp1 = tmp-1; 					\
	I.OverVal = (tmp == 0x8000); 			\
    SetAF(tmp1,tmp,1); 						\
    SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1

#define JMP(flag)							\
	int tmp;								\
	EMPTY_PREFETCH();						\
	tmp = (int)((INT8)FETCH());				\
	if (flag)								\
	{										\
		static const UINT8 table[3]={3,10,10}; 	\
		I.ip = (WORD)(I.ip+tmp);			\
		nec_ICount-=table[I.chip_type/8];	\
		CHANGE_PC;							\
		return;								\
	}

#define ADJ4(param1,param2)					\
	if (AF || ((I.regs.b[AL] & 0xf) > 9))	\
	{										\
		UINT16 tmp;							\
		tmp = I.regs.b[AL] + param1;		\
		I.regs.b[AL] = tmp;					\
		I.AuxVal = 1;						\
		I.CarryVal |= tmp & 0x100;			\
	}										\
	if (CF || (I.regs.b[AL]>0x9f))			\
	{										\
		I.regs.b[AL] += param2;				\
		I.CarryVal = 1;						\
	}										\
	SetSZPF_Byte(I.regs.b[AL])

#define ADJB(param1,param2)					\
	if (AF || ((I.regs.b[AL] & 0xf) > 9))	\
    {										\
		I.regs.b[AL] += param1;				\
		I.regs.b[AH] += param2;				\
		I.AuxVal = 1;						\
		I.CarryVal = 1;						\
    }										\
	else									\
	{										\
		I.AuxVal = 0;						\
		I.CarryVal = 0;						\
    }										\
	I.regs.b[AL] &= 0x0F

#define BITOP_BYTE							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=I.regs.b[Mod_RM.RM.b[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])();					\
		tmp=read_byte(EA);					\
    }

#define BITOP_WORD							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=I.regs.w[Mod_RM.RM.w[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])();					\
		tmp=read_word(EA);					\
    }

#define BIT_NOT								\
	if (tmp & (1<<tmp2))					\
		tmp &= ~(1<<tmp2);					\
	else									\
		tmp |= (1<<tmp2)

#define XchgAWReg(Reg) 						\
    WORD tmp; 								\
	tmp = I.regs.w[Reg]; 					\
	I.regs.w[Reg] = I.regs.w[AW]; 			\
	I.regs.w[AW] = tmp

#define ROL_BYTE I.CarryVal = dst & 0x80; dst = (dst << 1)+CF
#define ROL_WORD I.CarryVal = dst & 0x8000; dst = (dst << 1)+CF
#define ROR_BYTE I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_WORD I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#define ROLC_BYTE dst = (dst << 1) + CF; SetCFB(dst)
#define ROLC_WORD dst = (dst << 1) + CF; SetCFW(dst)
#define RORC_BYTE dst = (CF<<8)+dst; I.CarryVal = dst & 0x01; dst >>= 1
#define RORC_WORD dst = (CF<<16)+dst; I.CarryVal = dst & 0x01; dst >>= 1
#define SHL_BYTE(c) nec_ICount-=c; dst <<= c;	SetCFB(dst); SetSZPF_Byte(dst);	PutbackRMByte(ModRM,(BYTE)dst)
#define SHL_WORD(c) nec_ICount-=c; dst <<= c;	SetCFW(dst); SetSZPF_Word(dst);	PutbackRMWord(ModRM,(WORD)dst)
#define SHR_BYTE(c) nec_ICount-=c; dst >>= c-1; I.CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHR_WORD(c) nec_ICount-=c; dst >>= c-1; I.CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)
#define SHRA_BYTE(c) nec_ICount-=c; dst = ((INT8)dst) >> (c-1);	I.CarryVal = dst & 0x1;	dst = ((INT8)((BYTE)dst)) >> 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHRA_WORD(c) nec_ICount-=c; dst = ((INT16)dst) >> (c-1);	I.CarryVal = dst & 0x1;	dst = ((INT16)((WORD)dst)) >> 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)

#define DIVUB												\
	uresult = I.regs.w[AW];									\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xff) {							\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.b[AL] = uresult;								\
		I.regs.b[AH] = uresult2;							\
	}

#define DIVB												\
	result = (INT16)I.regs.w[AW];							\
	result2 = result % (INT16)((INT8)tmp);					\
	if ((result /= (INT16)((INT8)tmp)) > 0xff) {			\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.b[AL] = result;								\
		I.regs.b[AH] = result2;								\
	}

#define DIVUW												\
	uresult = (((UINT32)I.regs.w[DW]) << 16) | I.regs.w[AW];\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xffff) {						\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.w[AW]=uresult;								\
		I.regs.w[DW]=uresult2;								\
	}

#define DIVW												\
	result = ((UINT32)I.regs.w[DW] << 16) + I.regs.w[AW];	\
	result2 = result % (INT32)((INT16)tmp);					\
	if ((result /= (INT32)((INT16)tmp)) > 0xffff) {			\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.w[AW]=result;								\
		I.regs.w[DW]=result2;								\
	}

#define ADD4S {												\
	int i,v1,v2,result;										\
	int count = (I.regs.b[CL]+1)/2;							\
	unsigned di = I.regs.w[IY];								\
	unsigned si = I.regs.w[IX];								\
	static const UINT8 table[3]={18,19,19};	 				\
	if (I.seg_prefix) logerror("%06x: Warning: seg_prefix defined for add4s\n",cpu_get_pc(Machine->activecpu));	\
	I.ZeroVal = I.CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_ICount-=table[I.chip_type/8];					\
		tmp = GetMemB(DS0, si);								\
		tmp2 = GetMemB(DS1, di);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		result = v1+v2+I.CarryVal;							\
		I.CarryVal = result > 99 ? 1 : 0;					\
		result = result % 100;								\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(DS1, di,v1);								\
		if (v1) I.ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define SUB4S {												\
	int count = (I.regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = I.regs.w[IY];								\
	unsigned si = I.regs.w[IX];								\
	static const UINT8 table[3]={18,19,19};					\
	if (I.seg_prefix) logerror("%06x: Warning: seg_prefix defined for sub4s\n",cpu_get_pc(Machine->activecpu));	\
	I.ZeroVal = I.CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_ICount-=table[I.chip_type/8];					\
		tmp = GetMemB(DS1, di);								\
		tmp2 = GetMemB(DS0, si);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+I.CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 1;									\
		} else {											\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(DS1, di,v1);								\
		if (v1) I.ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define CMP4S {												\
	int count = (I.regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = I.regs.w[IY];								\
	unsigned si = I.regs.w[IX];								\
	static const UINT8 table[3]={14,19,19};					\
	if (I.seg_prefix) logerror("%06x: Warning: seg_prefix defined for cmp4s\n",cpu_get_pc(Machine->activecpu));	\
	I.ZeroVal = I.CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_ICount-=table[I.chip_type/8];					\
		tmp = GetMemB(DS1, di);								\
		tmp2 = GetMemB(DS0, si);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+I.CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 1;									\
		} else {											\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		if (v1) I.ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}
