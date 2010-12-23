#define NEC_NMI_INT_VECTOR	2

/* Cpu types, steps of 8 to help the cycle count calculation */
#define V33_TYPE 0
#define V30_TYPE 8
#define V20_TYPE 16

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

/* NEC registers */
typedef union
{                   /* eight general registers */
    UINT16 w[8];    /* viewed as 16 bits registers */
    UINT8  b[16];   /* or as 8 bit registers */
} necbasicregs;

typedef struct _nec_state_t nec_state_t;
struct _nec_state_t
{
	necbasicregs regs;
	offs_t	fetch_xor;
	UINT16	sregs[4];

	UINT16	ip;

	INT32	SignVal;
	UINT32	AuxVal, OverVal, ZeroVal, CarryVal, ParityVal;	/* 0 or non-0 valued flags */
	UINT8	TF, IF, DF, MF;	/* 0 or 1 valued flags */
	UINT32	int_vector;
	UINT32	pending_irq;
	UINT32	nmi_state;
	UINT32	irq_state;
	UINT32	poll_state;
	UINT8	no_interrupt;

	device_irq_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	int		icount;

	UINT8	prefetch_size;
	UINT8	prefetch_cycles;
	INT8	prefetch_count;
	UINT8	prefetch_reset;
	UINT32	chip_type;

	UINT32	prefix_base;	/* base address of the latest prefix segment */
	UINT8	seg_prefix;		/* prefix segment indicator */
};

typedef enum { DS1, PS, SS, DS0 } SREGS;
typedef enum { AW, CW, DW, BW, SP, BP, IX, IY } WREGS;
typedef enum {
   AL = NATIVE_ENDIAN_VALUE_LE_BE(0x0, 0x1),
   AH = NATIVE_ENDIAN_VALUE_LE_BE(0x1, 0x0),
   CL = NATIVE_ENDIAN_VALUE_LE_BE(0x2, 0x3),
   CH = NATIVE_ENDIAN_VALUE_LE_BE(0x3, 0x2),
   DL = NATIVE_ENDIAN_VALUE_LE_BE(0x4, 0x5),
   DH = NATIVE_ENDIAN_VALUE_LE_BE(0x5, 0x4),
   BL = NATIVE_ENDIAN_VALUE_LE_BE(0x6, 0x7),
   BH = NATIVE_ENDIAN_VALUE_LE_BE(0x7, 0x6),
} BREGS;

#define Sreg(x)			nec_state->sregs[x]
#define Wreg(x)			nec_state->regs.w[x]
#define Breg(x)			nec_state->regs.b[x]

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

/************************************************************************/

#define read_mem_byte(a)			nec_state->program->read_byte(a)
#define read_mem_word(a)			nec_state->program->read_word_unaligned(a)
#define write_mem_byte(a,d)			nec_state->program->write_byte((a),(d))
#define write_mem_word(a,d)			nec_state->program->write_word_unaligned((a),(d))

#define read_port_byte(a)		nec_state->io->read_byte(a)
#define read_port_word(a)		nec_state->io->read_word_unaligned(a)
#define write_port_byte(a,d)	nec_state->io->write_byte((a),(d))
#define write_port_word(a,d)	nec_state->io->write_word_unaligned((a),(d))

/************************************************************************/

#define CHANGE_PC do { EMPTY_PREFETCH(); } while (0)

#define SegBase(Seg) (Sreg(Seg) << 4)

#define DefaultBase(Seg) ((nec_state->seg_prefix && (Seg==DS0 || Seg==SS)) ? nec_state->prefix_base : Sreg(Seg) << 4)

#define GetMemB(Seg,Off) (read_mem_byte(DefaultBase(Seg) + (Off)))
#define GetMemW(Seg,Off) (read_mem_word(DefaultBase(Seg) + (Off)))

#define PutMemB(Seg,Off,x) { write_mem_byte(DefaultBase(Seg) + (Off), (x)); }
#define PutMemW(Seg,Off,x) { write_mem_word(DefaultBase(Seg) + (Off), (x)); }

/* prefetch timing */

#define FETCH() 			fetch(nec_state)
#define FETCH_XOR(a)		((a) ^ nec_state->fetch_xor)
#define FETCHWORD()			fetchword(nec_state)
#define EMPTY_PREFETCH()	nec_state->prefetch_reset = 1


#define PUSH(val) { Wreg(SP)-=2; write_mem_word((((Sreg(SS)<<4)+Wreg(SP))),val); }
#define POP(var) { var = read_mem_word((((Sreg(SS)<<4)+Wreg(SP)))); Wreg(SP)+=2; }

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
#define CompressFlags() (WORD)(CF | 0x02 | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (nec_state->TF << 8) | (nec_state->IF << 9) \
				| (nec_state->DF << 10) | (OF << 11) | (nec_state->MF << 15))

#define ExpandFlags(f) \
{ \
	nec_state->CarryVal = (f) & 0x0001; \
	nec_state->ParityVal = !((f) & 0x0004); \
	nec_state->AuxVal = (f) & 0x0010; \
	nec_state->ZeroVal = !((f) & 0x0040); \
	nec_state->SignVal = (f) & 0x0080 ? -1 : 0; \
	nec_state->TF = ((f) & 0x0100) == 0x0100; \
	nec_state->IF = ((f) & 0x0200) == 0x0200; \
	nec_state->DF = ((f) & 0x0400) == 0x0400; \
	nec_state->OverVal = (f) & 0x0800; \
	nec_state->MF = ((f) & 0x8000) == 0x8000; \
}

#define IncWordReg(Reg) 					\
	unsigned tmp = (unsigned)Wreg(Reg); \
	unsigned tmp1 = tmp+1;					\
	nec_state->OverVal = (tmp == 0x7fff);			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	Wreg(Reg)=tmp1

#define DecWordReg(Reg) 					\
	unsigned tmp = (unsigned)Wreg(Reg); \
    unsigned tmp1 = tmp-1;					\
	nec_state->OverVal = (tmp == 0x8000);			\
    SetAF(tmp1,tmp,1);						\
    SetSZPF_Word(tmp1); 					\
	Wreg(Reg)=tmp1

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
	if (AF || ((Breg(AL) & 0xf) > 9))	\
	{										\
		UINT16 tmp;							\
		tmp = Breg(AL) + param1;		\
		Breg(AL) = tmp;					\
		nec_state->AuxVal = 1;						\
		nec_state->CarryVal |= tmp & 0x100;			\
	}										\
	if (CF || (Breg(AL)>0x9f))			\
	{										\
		Breg(AL) += param2;				\
		nec_state->CarryVal = 1;						\
	}										\
	SetSZPF_Byte(Breg(AL))

#define ADJB(param1,param2)					\
	if (AF || ((Breg(AL) & 0xf) > 9))	\
    {										\
		Breg(AL) += param1;				\
		Breg(AH) += param2;				\
		nec_state->AuxVal = 1;						\
		nec_state->CarryVal = 1;						\
    }										\
	else									\
	{										\
		nec_state->AuxVal = 0;						\
		nec_state->CarryVal = 0;						\
    }										\
	Breg(AL) &= 0x0F

#define BITOP_BYTE							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=Breg(Mod_RM.RM.b[ModRM]);	\
	}										\
	else {									\
		(*GetEA[ModRM])(nec_state);					\
		tmp=read_mem_byte(EA);					\
    }

#define BITOP_WORD							\
	ModRM = FETCH();							\
	if (ModRM >= 0xc0) {					\
		tmp=Wreg(Mod_RM.RM.w[ModRM]);	\
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
	tmp = Wreg(Reg);					\
	Wreg(Reg) = Wreg(AW);			\
	Wreg(AW) = tmp

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
	uresult = Wreg(AW);									\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xff) {							\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		Breg(AL) = uresult;								\
		Breg(AH) = uresult2;							\
	}

#define DIVB												\
	result = (INT16)Wreg(AW);							\
	result2 = result % (INT16)((INT8)tmp);					\
	if ((result /= (INT16)((INT8)tmp)) > 0xff) {			\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		Breg(AL) = result;								\
		Breg(AH) = result2;								\
	}

#define DIVUW												\
	uresult = (((UINT32)Wreg(DW)) << 16) | Wreg(AW);\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xffff) {						\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		Wreg(AW)=uresult;								\
		Wreg(DW)=uresult2;								\
	}

#define DIVW												\
	result = ((UINT32)Wreg(DW) << 16) + Wreg(AW);	\
	result2 = result % (INT32)((INT16)tmp);					\
	if ((result /= (INT32)((INT16)tmp)) > 0xffff) {			\
		nec_interrupt(nec_state, 0,0); break;							\
	} else {												\
		Wreg(AW)=result;								\
		Wreg(DW)=result2;								\
	}

#define ADD4S {												\
	int i,v1,v2,result;										\
	int count = (Breg(CL)+1)/2;							\
	unsigned di = Wreg(IY);								\
	unsigned si = Wreg(IX);								\
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
	int count = (Breg(CL)+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = Wreg(IY);								\
	unsigned si = Wreg(IX);								\
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
	int count = (Breg(CL)+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = Wreg(IY);								\
	unsigned si = Wreg(IX);								\
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
