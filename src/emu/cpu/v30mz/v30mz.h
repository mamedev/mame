typedef enum { ES, CS, SS, DS } SREGS;
typedef enum { AW, CW, DW, BW, SP, BP, IX, IY } WREGS;

#define NEC_NMI_INT_VECTOR 2

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

#define SetTF(x)		(cpustate->TF = (x))
#define SetIF(x)		(cpustate->IF = (x))
#define SetDF(x)		(cpustate->DF = (x))
#define SetMD(x)		(cpustate->MF = (x))	/* OB [19.07.99] Mode Flag V30 */

#define SetCFB(x)		(cpustate->CarryVal = (x) & 0x100)
#define SetCFW(x)		(cpustate->CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)	(cpustate->AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)		(cpustate->SignVal = (x))
#define SetZF(x)		(cpustate->ZeroVal = (x))
#define SetPF(x)		(cpustate->ParityVal = (x))

#define SetSZPF_Byte(x) (cpustate->SignVal=cpustate->ZeroVal=cpustate->ParityVal=(INT8)(x))
#define SetSZPF_Word(x) (cpustate->SignVal=cpustate->ZeroVal=cpustate->ParityVal=(INT16)(x))

#define SetOFW_Add(x,y,z)	(cpustate->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(cpustate->OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(cpustate->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(cpustate->OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define ADDB { UINT32 res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(UINT8)res; }
#define ADDW { UINT32 res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(UINT16)res; }

#define SUBB { UINT32 res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(UINT8)res; }
#define SUBW { UINT32 res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(UINT16)res; }

#define ORB dst|=src; cpustate->CarryVal=cpustate->OverVal=cpustate->AuxVal=0; SetSZPF_Byte(dst)
#define ORW dst|=src; cpustate->CarryVal=cpustate->OverVal=cpustate->AuxVal=0; SetSZPF_Word(dst)

#define ANDB dst&=src; cpustate->CarryVal=cpustate->OverVal=cpustate->AuxVal=0; SetSZPF_Byte(dst)
#define ANDW dst&=src; cpustate->CarryVal=cpustate->OverVal=cpustate->AuxVal=0; SetSZPF_Word(dst)

#define XORB dst^=src; cpustate->CarryVal=cpustate->OverVal=cpustate->AuxVal=0; SetSZPF_Byte(dst)
#define XORW dst^=src; cpustate->CarryVal=cpustate->OverVal=cpustate->AuxVal=0; SetSZPF_Word(dst)

#define CF		(cpustate->CarryVal!=0)
#define SF		(cpustate->SignVal<0)
#define ZF		(cpustate->ZeroVal==0)
#define PF		parity_table[(UINT8)cpustate->ParityVal]
#define AF		(cpustate->AuxVal!=0)
#define OF		(cpustate->OverVal!=0)
#define MD		(cpustate->MF!=0)

/************************************************************************/

#define SegBase(Seg) (cpustate->sregs[Seg] << 4)

#define DefaultBase(Seg) ((cpustate->seg_prefix && (Seg==DS || Seg==SS)) ? cpustate->prefix_base : cpustate->sregs[Seg] << 4)

#define GetMemB(Seg,Off) ((UINT8)cpustate->program->read_byte((DefaultBase(Seg)+(Off))))
#define GetMemW(Seg,Off) ((UINT16) cpustate->program->read_byte((DefaultBase(Seg)+(Off))) + (cpustate->program->read_byte((DefaultBase(Seg)+((Off)+1)))<<8) )

#define PutMemB(Seg,Off,x) { cpustate->program->write_byte((DefaultBase(Seg)+(Off)),(x)); }
#define PutMemW(Seg,Off,x) { PutMemB(Seg,Off,(x)&0xff); PutMemB(Seg,(Off)+1,(UINT8)((x)>>8)); }

/* Todo:  Remove these later - plus readword could overflow */
#define ReadByte(ea) ((UINT8)cpustate->program->read_byte((ea)))
#define ReadWord(ea) (cpustate->program->read_byte((ea))+(cpustate->program->read_byte(((ea)+1))<<8))
#define WriteByte(ea,val) { cpustate->program->write_byte((ea),val); }
#define WriteWord(ea,val) { cpustate->program->write_byte((ea),(UINT8)(val)); cpustate->program->write_byte(((ea)+1),(val)>>8); }

#define read_port(port) cpustate->io->read_byte(port)
#define write_port(port,val) cpustate->io->write_byte(port,val)

#define FETCH (cpustate->direct->read_raw_byte((cpustate->sregs[CS]<<4)+cpustate->ip++))
#define FETCHOP (cpustate->direct->read_decrypted_byte((cpustate->sregs[CS]<<4)+cpustate->ip++))
#define FETCHWORD(var) { var=cpustate->direct->read_raw_byte((((cpustate->sregs[CS]<<4)+cpustate->ip)))+(cpustate->direct->read_raw_byte((((cpustate->sregs[CS]<<4)+cpustate->ip+1)))<<8); cpustate->ip+=2; }
#define PUSH(val) { cpustate->regs.w[SP]-=2; WriteWord((((cpustate->sregs[SS]<<4)+cpustate->regs.w[SP])),val); }
#define POP(var) { var = ReadWord((((cpustate->sregs[SS]<<4)+cpustate->regs.w[SP]))); cpustate->regs.w[SP]+=2; }
#define PEEK(addr) ((UINT8)cpustate->direct->read_raw_byte(addr))
#define PEEKOP(addr) ((UINT8)cpustate->direct->read_decrypted_byte(addr))

#define GetModRM UINT32 ModRM=cpustate->direct->read_raw_byte((cpustate->sregs[CS]<<4)+cpustate->ip++)

/* Cycle count macros:
    CLK  - cycle count is the same on all processors
    CLKS - cycle count differs between processors, list all counts
    CLKW - cycle count for word read/write differs for odd/even source/destination address
    CLKM - cycle count for reg/mem instructions
    CLKR - cycle count for reg/mem instructions with different counts for odd/even addresses


    Prefetch & buswait time is not emulated.
    Extra cycles for PUSH'ing or POP'ing registers to odd addresses is not emulated.
*/

#define CLK(v30mz) { cpustate->icount-=v30mz; }
#define CLKM(v30mz,v30mzm) { cpustate->icount-=( ModRM >=0xc0 )?v30mz:v30mzm; }

/************************************************************************/
#define CompressFlags() (UINT16)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (cpustate->TF << 8) | (cpustate->IF << 9) \
				| (cpustate->DF << 10) | (OF << 11)| (MD << 15))

#define ExpandFlags(f) \
{ \
	cpustate->CarryVal = (f) & 1; \
	cpustate->ParityVal = !((f) & 4); \
	cpustate->AuxVal = (f) & 16; \
	cpustate->ZeroVal = !((f) & 64); \
	cpustate->SignVal = (f) & 128 ? -1 : 0; \
	cpustate->TF = ((f) & 256) == 256; \
	cpustate->IF = ((f) & 512) == 512; \
	cpustate->DF = ((f) & 1024) == 1024; \
	cpustate->OverVal = (f) & 2048; \
	cpustate->MF = ((f) & 0x8000) == 0x8000; \
}

#define IncWordReg(Reg) 					\
	unsigned tmp = (unsigned)cpustate->regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	cpustate->OverVal = (tmp == 0x7fff);			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	cpustate->regs.w[Reg]=tmp1

#define DecWordReg(Reg) 					\
	unsigned tmp = (unsigned)cpustate->regs.w[Reg]; \
    unsigned tmp1 = tmp-1;					\
	cpustate->OverVal = (tmp == 0x8000);			\
    SetAF(tmp1,tmp,1);						\
    SetSZPF_Word(tmp1); 					\
	cpustate->regs.w[Reg]=tmp1

#define JMP(flag)							\
	int tmp = (int)((INT8)FETCH);			\
	if (flag)								\
	{										\
		cpustate->ip = (UINT16)(cpustate->ip+tmp);			\
		cpustate->icount-=10;		\
		return;								\
	}

#define ADJ4(param1,param2)					\
	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))	\
	{										\
		UINT16 tmp;							\
		tmp = cpustate->regs.b[AL] + param1;		\
		cpustate->regs.b[AL] = tmp;					\
		cpustate->AuxVal = 1;						\
		cpustate->CarryVal |= tmp & 0x100;			\
	}										\
	if (CF || (cpustate->regs.b[AL]>0x9f))			\
	{										\
		cpustate->regs.b[AL] += param2;				\
		cpustate->CarryVal = 1;						\
	}										\
	SetSZPF_Byte(cpustate->regs.b[AL])

#define ADJB(param1,param2)					\
	if (AF || ((cpustate->regs.b[AL] & 0xf) > 9))	\
    {										\
		cpustate->regs.b[AL] += param1;				\
		cpustate->regs.b[AH] += param2;				\
		cpustate->AuxVal = 1;						\
		cpustate->CarryVal = 1;						\
    }										\
	else									\
	{										\
		cpustate->AuxVal = 0;						\
		cpustate->CarryVal = 0;						\
    }										\
	cpustate->regs.b[AL] &= 0x0F

#define BITOP_BYTE							\
	ModRM = FETCH;							\
	if (ModRM >= 0xc0) {					\
		tmp=cpustate->regs.b[Mod_RM.RM.b[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])(cpustate);					\
		tmp=ReadByte(cpustate->ea);					\
    }

#define BITOP_WORD							\
	ModRM = FETCH;							\
	if (ModRM >= 0xc0) {					\
		tmp=cpustate->regs.w[Mod_RM.RM.w[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])(cpustate);					\
		tmp=ReadWord(cpustate->ea);					\
    }

#define BIT_NOT								\
	if (tmp & (1<<tmp2))					\
		tmp &= ~(1<<tmp2);					\
	else									\
		tmp |= (1<<tmp2)

#define XchgAWReg(Reg)						\
    UINT16 tmp;								\
	tmp = cpustate->regs.w[Reg];					\
	cpustate->regs.w[Reg] = cpustate->regs.w[AW];			\
	cpustate->regs.w[AW] = tmp

#define ROL_BYTE cpustate->CarryVal = dst & 0x80; dst = (dst << 1)+CF
#define ROL_WORD cpustate->CarryVal = dst & 0x8000; dst = (dst << 1)+CF
#define ROR_BYTE cpustate->CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_WORD cpustate->CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#define ROLC_BYTE dst = (dst << 1) + CF; SetCFB(dst)
#define ROLC_WORD dst = (dst << 1) + CF; SetCFW(dst)
#define RORC_BYTE dst = (CF<<8)+dst; cpustate->CarryVal = dst & 0x01; dst >>= 1
#define RORC_WORD dst = (CF<<16)+dst; cpustate->CarryVal = dst & 0x01; dst >>= 1
#define SHL_BYTE(c) cpustate->icount-=c; dst <<= c;	SetCFB(dst); SetSZPF_Byte(dst);	PutbackRMByte(ModRM,(UINT8)dst)
#define SHL_WORD(c) cpustate->icount-=c; dst <<= c;	SetCFW(dst); SetSZPF_Word(dst);	PutbackRMWord(ModRM,(UINT16)dst)
#define SHR_BYTE(c) cpustate->icount-=c; dst >>= c-1; cpustate->CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(UINT8)dst)
#define SHR_WORD(c) cpustate->icount-=c; dst >>= c-1; cpustate->CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(UINT16)dst)
#define SHRA_BYTE(c) cpustate->icount-=c; dst = ((INT8)dst) >> (c-1);	cpustate->CarryVal = dst & 0x1;	dst = ((INT8)((UINT8)dst)) >> 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(UINT8)dst)
#define SHRA_WORD(c) cpustate->icount-=c; dst = ((INT16)dst) >> (c-1);	cpustate->CarryVal = dst & 0x1;	dst = ((INT16)((UINT16)dst)) >> 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(UINT16)dst)

#define DIVUB												\
	uresult = cpustate->regs.w[AW];									\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xff) {							\
		nec_interrupt(cpustate,0); break;							\
	} else {												\
		cpustate->regs.b[AL] = uresult;								\
		cpustate->regs.b[AH] = uresult2;							\
	}

#define DIVB												\
	result = (INT16)cpustate->regs.w[AW];							\
	result2 = result % (INT16)((INT8)tmp);					\
	if ((result /= (INT16)((INT8)tmp)) > 0xff) {			\
		nec_interrupt(cpustate,0); break;							\
	} else {												\
		cpustate->regs.b[AL] = result;								\
		cpustate->regs.b[AH] = result2;								\
	}

#define DIVUW												\
	uresult = (((UINT32)cpustate->regs.w[DW]) << 16) | cpustate->regs.w[AW];\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xffff) {						\
		nec_interrupt(cpustate,0); break;							\
	} else {												\
		cpustate->regs.w[AW]=uresult;								\
		cpustate->regs.w[DW]=uresult2;								\
	}

#define DIVW												\
	result = ((UINT32)cpustate->regs.w[DW] << 16) + cpustate->regs.w[AW];	\
	result2 = result % (INT32)((INT16)tmp);					\
	if ((result /= (INT32)((INT16)tmp)) > 0xffff) {			\
		nec_interrupt(cpustate,0); break;							\
	} else {												\
		cpustate->regs.w[AW]=result;								\
		cpustate->regs.w[DW]=result2;								\
	}

#define ADD4S {												\
	int i,v1,v2,result;										\
	int count = (cpustate->regs.b[CL]+1)/2;							\
	unsigned di = cpustate->regs.w[IY];								\
	unsigned si = cpustate->regs.w[IX];								\
	if (cpustate->seg_prefix) logerror("%06x: Warning: seg_prefix defined for add4s\n",PC(cpustate));	\
	cpustate->ZeroVal = cpustate->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		cpustate->icount-=19;						\
		tmp = GetMemB(DS, si);								\
		tmp2 = GetMemB(ES, di);								\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		result = v1+v2+cpustate->CarryVal;							\
		cpustate->CarryVal = result > 99 ? 1 : 0;					\
		result = result % 100;								\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(ES, di,v1);									\
		if (v1) cpustate->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define SUB4S {												\
	int count = (cpustate->regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = cpustate->regs.w[IY];								\
	unsigned si = cpustate->regs.w[IX];								\
	if (cpustate->seg_prefix) logerror("%06x: Warning: seg_prefix defined for sub4s\n",PC(cpustate));	\
	cpustate->ZeroVal = cpustate->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		cpustate->icount-=19;						\
		tmp = GetMemB(ES, di);								\
		tmp2 = GetMemB(DS, si);								\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+cpustate->CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+cpustate->CarryVal);					\
			cpustate->CarryVal = 1;									\
		} else {											\
			result = v1-(v2+cpustate->CarryVal);					\
			cpustate->CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(ES, di,v1);									\
		if (v1) cpustate->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define CMP4S {												\
	int count = (cpustate->regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = cpustate->regs.w[IY];								\
	unsigned si = cpustate->regs.w[IX];								\
	if (cpustate->seg_prefix) logerror("%06x: Warning: seg_prefix defined for cmp4s\n",PC(cpustate));	\
	cpustate->ZeroVal = cpustate->CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		cpustate->icount-=19;						\
		tmp = GetMemB(ES, di);								\
		tmp2 = GetMemB(DS, si);								\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+cpustate->CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+cpustate->CarryVal);					\
			cpustate->CarryVal = 1;									\
		} else {											\
			result = v1-(v2+cpustate->CarryVal);					\
			cpustate->CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		if (v1) cpustate->ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}
