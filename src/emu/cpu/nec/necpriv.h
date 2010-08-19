
typedef enum { DS1, PS, SS, DS0 } SREGS;
typedef enum { AW, CW, DW, BW, SP, BP, IX, IY } WREGS;

#define NEC_NMI_INT_VECTOR	2

/* Cpu types, steps of 8 to help the cycle count calculation */
#define V33_TYPE 0
#define V30_TYPE 8
#define V20_TYPE 16

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
  IXL = NATIVE_ENDIAN_VALUE_LE_BE(0xc, 0xd),
  IXH = NATIVE_ENDIAN_VALUE_LE_BE(0xd, 0xc),
  IYL = NATIVE_ENDIAN_VALUE_LE_BE(0xe, 0xf),
  IYH = NATIVE_ENDIAN_VALUE_LE_BE(0xf, 0xe)
} BREGS;

/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)		(nec_state->TF = (x))
#define SetIF(x)		(nec_state->IF = (x))
#define SetDF(x)		(nec_state->DF = (x))
#define SetMD(x)		(nec_state->MF = (x))	/* OB [19.07.99] Mode Flag V30 */

#define SetCFB(x)		(nec_state->CarryVal = (x) & 0x100)
#define SetCFW(x)		(nec_state->CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)	(nec_state->AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)		(nec_state->SignVal = (x))
#define SetZF(x)		(nec_state->ZeroVal = (x))
#define SetPF(x)		(nec_state->ParityVal = (x))

#define SetSZPF_Byte(x) (nec_state->SignVal=nec_state->ZeroVal=nec_state->ParityVal=(INT8)(x))
#define SetSZPF_Word(x) (nec_state->SignVal=nec_state->ZeroVal=nec_state->ParityVal=(INT16)(x))

#define SetOFW_Add(x,y,z)	(nec_state->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(nec_state->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(nec_state->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(nec_state->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define ADDB { UINT32 res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW { UINT32 res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB { UINT32 res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW { UINT32 res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB dst|=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Byte(dst)
#define ORW dst|=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Word(dst)

#define ANDB dst&=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Byte(dst)
#define ANDW dst&=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Word(dst)

#define XORB dst^=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Byte(dst)
#define XORW dst^=src; nec_state->CarryVal=nec_state->OverVal=nec_state->AuxVal=0; SetSZPF_Word(dst)

#define CF		(nec_state->CarryVal!=0)
#define SF		(nec_state->SignVal<0)
#define ZF		(nec_state->ZeroVal==0)
#define PF		parity_table[(BYTE)nec_state->ParityVal]
#define AF		(nec_state->AuxVal!=0)
#define OF		(nec_state->OverVal!=0)
#define MD		(nec_state->MF!=0)

/************************************************************************/

#define read_mem_byte(a)			(*nec_state->mem.rbyte)(nec_state->program, a)
#define read_mem_word(a)			(*nec_state->mem.rword)(nec_state->program, a)
#define write_mem_byte(a,d)			(*nec_state->mem.wbyte)(nec_state->program, (a),(d))
#define write_mem_word(a,d)			(*nec_state->mem.wword)(nec_state->program, (a),(d))

#define read_port_byte(a)		(*nec_state->mem.rbyte)(nec_state->io, a)
#define read_port_word(a)		(*nec_state->mem.rword)(nec_state->io, a)
#define write_port_byte(a,d)	(*nec_state->mem.wbyte)(nec_state->io, (a),(d))
#define write_port_word(a,d)	(*nec_state->mem.wword)(nec_state->io, (a),(d))

/************************************************************************/

#define CHANGE_PC do { EMPTY_PREFETCH(); } while (0)

#define SegBase(Seg) (nec_state->sregs[Seg] << 4)

#define DefaultBase(Seg) ((nec_state->seg_prefix && (Seg==DS0 || Seg==SS)) ? nec_state->prefix_base : nec_state->sregs[Seg] << 4)

#define GetMemB(Seg,Off) (read_mem_byte(DefaultBase(Seg) + (Off)))
#define GetMemW(Seg,Off) (read_mem_word(DefaultBase(Seg) + (Off)))

#define PutMemB(Seg,Off,x) { write_mem_byte(DefaultBase(Seg) + (Off), (x)); }
#define PutMemW(Seg,Off,x) { write_mem_word(DefaultBase(Seg) + (Off), (x)); }

/* prefetch timing */

#define FETCH() 			fetch(nec_state)
#define FETCH_XOR(a)		((a) ^ nec_state->mem.fetch_xor)
#define FETCHWORD()			fetchword(nec_state)
#define EMPTY_PREFETCH()	nec_state->prefetch_reset = 1


#define PUSH(val) { nec_state->regs.w[SP]-=2; write_mem_word((((nec_state->sregs[SS]<<4)+nec_state->regs.w[SP])),val); }
#define POP(var) { var = read_mem_word((((nec_state->sregs[SS]<<4)+nec_state->regs.w[SP]))); nec_state->regs.w[SP]+=2; }

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

#define CLK(all) nec_state->icount-=all
#define CLKS(v20,v30,v33) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33; nec_state->icount-=(ccount>>nec_state->chip_type)&0x7f; }
#define CLKW(v20o,v30o,v33o,v20e,v30e,v33e,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; nec_state->icount-=(addr&1)?((ocount>>nec_state->chip_type)&0x7f):((ecount>>nec_state->chip_type)&0x7f); }
#define CLKM(v20,v30,v33,v20m,v30m,v33m) { const UINT32 ccount=(v20<<16)|(v30<<8)|v33, mcount=(v20m<<16)|(v30m<<8)|v33m; nec_state->icount-=( ModRM >=0xc0 )?((ccount>>nec_state->chip_type)&0x7f):((mcount>>nec_state->chip_type)&0x7f); }
#define CLKR(v20o,v30o,v33o,v20e,v30e,v33e,vall,addr) { const UINT32 ocount=(v20o<<16)|(v30o<<8)|v33o, ecount=(v20e<<16)|(v30e<<8)|v33e; if (ModRM >=0xc0) nec_state->icount-=vall; else nec_state->icount-=(addr&1)?((ocount>>nec_state->chip_type)&0x7f):((ecount>>nec_state->chip_type)&0x7f); }

/************************************************************************/
#define CompressFlags() (WORD)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (nec_state->TF << 8) | (nec_state->IF << 9) \
				| (nec_state->DF << 10) | (OF << 11)| (MD << 15))

#define ExpandFlags(f) \
{ \
	nec_state->CarryVal = (f) & 1; \
	nec_state->ParityVal = !((f) & 4); \
	nec_state->AuxVal = (f) & 16; \
	nec_state->ZeroVal = !((f) & 64); \
	nec_state->SignVal = (f) & 128 ? -1 : 0; \
	nec_state->TF = ((f) & 256) == 256; \
	nec_state->IF = ((f) & 512) == 512; \
	nec_state->DF = ((f) & 1024) == 1024; \
	nec_state->OverVal = (f) & 2048; \
	nec_state->MF = ((f) & 0x8000) == 0x8000; \
}

#define IncWordReg(Reg) 					\
	unsigned tmp = (unsigned)nec_state->regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	nec_state->OverVal = (tmp == 0x7fff);			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	nec_state->regs.w[Reg]=tmp1

#define DecWordReg(Reg) 					\
	unsigned tmp = (unsigned)nec_state->regs.w[Reg]; \
    unsigned tmp1 = tmp-1;					\
	nec_state->OverVal = (tmp == 0x8000);			\
    SetAF(tmp1,tmp,1);						\
    SetSZPF_Word(tmp1); 					\
	nec_state->regs.w[Reg]=tmp1

#define JMP(flag)							\
	int tmp;								\
	EMPTY_PREFETCH();						\
	tmp = (int)((INT8)FETCH());				\
	if (flag)								\
	{										\
		static const UINT8 table[3]={3,10,10};	\
		nec_state->ip = (WORD)(nec_state->ip+tmp);			\
		nec_state->icount-=table[nec_state->chip_type/8];	\
		CHANGE_PC;							\
		return;								\
	}

#define ADJ4(param1,param2)					\
	if (AF || ((nec_state->regs.b[AL] & 0xf) > 9))	\
	{										\
		UINT16 tmp;							\
		tmp = nec_state->regs.b[AL] + param1;		\
		nec_state->regs.b[AL] = tmp;					\
		nec_state->AuxVal = 1;						\
		nec_state->CarryVal |= tmp & 0x100;			\
	}										\
	if (CF || (nec_state->regs.b[AL]>0x9f))			\
	{										\
		nec_state->regs.b[AL] += param2;				\
		nec_state->CarryVal = 1;						\
	}										\
	SetSZPF_Byte(nec_state->regs.b[AL])

#define ADJB(param1,param2)					\
	if (AF || ((nec_state->regs.b[AL] & 0xf) > 9))	\
    {										\
		nec_state->regs.b[AL] += param1;				\
		nec_state->regs.b[AH] += param2;				\
		nec_state->AuxVal = 1;						\
		nec_state->CarryVal = 1;						\
    }										\
	else									\
	{										\
		nec_state->AuxVal = 0;						\
		nec_state->CarryVal = 0;						\
    }										\
	nec_state->regs.b[AL] &= 0x0F

#define BITOP_BYTE							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=nec_state->regs.b[Mod_RM.RM.b[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])(nec_state);					\
		tmp=read_mem_byte(EA);					\
    }

#define BITOP_WORD							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=nec_state->regs.w[Mod_RM.RM.w[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])(nec_state);					\
		tmp=read_mem_word(EA);					\
    }

#define BIT_NOT								\
	if (tmp & (1<<tmp2))					\
		tmp &= ~(1<<tmp2);					\
	else									\
		tmp |= (1<<tmp2)

#define XchgAWReg(Reg)						\
    WORD tmp;								\
	tmp = nec_state->regs.w[Reg];					\
	nec_state->regs.w[Reg] = nec_state->regs.w[AW]; 			\
	nec_state->regs.w[AW] = tmp

#define ROL_BYTE nec_state->CarryVal = dst & 0x80; dst = (dst << 1)+CF
#define ROL_WORD nec_state->CarryVal = dst & 0x8000; dst = (dst << 1)+CF
#define ROR_BYTE nec_state->CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_WORD nec_state->CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#define ROLC_BYTE dst = (dst << 1) + CF; SetCFB(dst)
#define ROLC_WORD dst = (dst << 1) + CF; SetCFW(dst)
#define RORC_BYTE dst = (CF<<8)+dst; nec_state->CarryVal = dst & 0x01; dst >>= 1
#define RORC_WORD dst = (CF<<16)+dst; nec_state->CarryVal = dst & 0x01; dst >>= 1
#define SHL_BYTE(c) nec_state->icount-=c; dst <<= c;	SetCFB(dst); SetSZPF_Byte(dst);	PutbackRMByte(ModRM,(BYTE)dst)
#define SHL_WORD(c) nec_state->icount-=c; dst <<= c;	SetCFW(dst); SetSZPF_Word(dst);	PutbackRMWord(ModRM,(WORD)dst)
#define SHR_BYTE(c) nec_state->icount-=c; dst >>= c-1; nec_state->CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHR_WORD(c) nec_state->icount-=c; dst >>= c-1; nec_state->CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)
#define SHRA_BYTE(c) nec_state->icount-=c; dst = ((INT8)dst) >> (c-1);	nec_state->CarryVal = dst & 0x1;	dst = ((INT8)((BYTE)dst)) >> 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHRA_WORD(c) nec_state->icount-=c; dst = ((INT16)dst) >> (c-1);	nec_state->CarryVal = dst & 0x1;	dst = ((INT16)((WORD)dst)) >> 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)

#define DIVUB												\
	uresult = nec_state->regs.w[AW];									\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xff) {							\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		nec_state->regs.b[AL] = uresult;								\
		nec_state->regs.b[AH] = uresult2;							\
	}

#define DIVB												\
	result = (INT16)nec_state->regs.w[AW];							\
	result2 = result % (INT16)((INT8)tmp);					\
	if ((result /= (INT16)((INT8)tmp)) > 0xff) {			\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		nec_state->regs.b[AL] = result;								\
		nec_state->regs.b[AH] = result2;								\
	}

#define DIVUW												\
	uresult = (((UINT32)nec_state->regs.w[DW]) << 16) | nec_state->regs.w[AW];\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xffff) {						\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		nec_state->regs.w[AW]=uresult;								\
		nec_state->regs.w[DW]=uresult2;								\
	}

#define DIVW												\
	result = ((UINT32)nec_state->regs.w[DW] << 16) + nec_state->regs.w[AW];	\
	result2 = result % (INT32)((INT16)tmp);					\
	if ((result /= (INT32)((INT16)tmp)) > 0xffff) {			\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		nec_state->regs.w[AW]=result;								\
		nec_state->regs.w[DW]=result2;								\
	}

#define ADD4S {												\
	int i,v1,v2,result;										\
	int count = (nec_state->regs.b[CL]+1)/2;							\
	unsigned di = nec_state->regs.w[IY];								\
	unsigned si = nec_state->regs.w[IX];								\
	static const UINT8 table[3]={18,19,19};					\
	if (nec_state->seg_prefix) logerror("%06x: Warning: seg_prefix defined for add4s\n",PC(nec_state));	\
	nec_state->ZeroVal = nec_state->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_state->icount-=table[nec_state->chip_type/8];					\
		tmp = GetMemB(DS0, si);								\
		tmp2 = GetMemB(DS1, di);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		result = v1+v2+nec_state->CarryVal;							\
		nec_state->CarryVal = result > 99 ? 1 : 0;					\
		result = result % 100;								\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(DS1, di,v1);								\
		if (v1) nec_state->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define SUB4S {												\
	int count = (nec_state->regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = nec_state->regs.w[IY];								\
	unsigned si = nec_state->regs.w[IX];								\
	static const UINT8 table[3]={18,19,19};					\
	if (nec_state->seg_prefix) logerror("%06x: Warning: seg_prefix defined for sub4s\n",PC(nec_state));	\
	nec_state->ZeroVal = nec_state->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_state->icount-=table[nec_state->chip_type/8];					\
		tmp = GetMemB(DS1, di);								\
		tmp2 = GetMemB(DS0, si);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+nec_state->CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 1;									\
		} else {											\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(DS1, di,v1);								\
		if (v1) nec_state->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define CMP4S {												\
	int count = (nec_state->regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = nec_state->regs.w[IY];								\
	unsigned si = nec_state->regs.w[IX];								\
	static const UINT8 table[3]={14,19,19};					\
	if (nec_state->seg_prefix) logerror("%06x: Warning: seg_prefix defined for cmp4s\n",PC(nec_state));	\
	nec_state->ZeroVal = nec_state->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		nec_state->icount-=table[nec_state->chip_type/8];					\
		tmp = GetMemB(DS1, di);								\
		tmp2 = GetMemB(DS0, si);							\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+nec_state->CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 1;									\
		} else {											\
			result = v1-(v2+nec_state->CarryVal);					\
			nec_state->CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		if (v1) nec_state->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}
