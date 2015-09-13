// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)        (m_TF = (x))
#define SetIF(x)        (m_IF = (x))
#define SetDF(x)        (m_DF = (x))
#define SetMD(x)        (m_MF = (x))   /* OB [19.07.99] Mode Flag V30 */

#define SetCFB(x)       (m_CarryVal = (x) & 0x100)
#define SetCFW(x)       (m_CarryVal = (x) & 0x10000)
#define SetAF(x,y,z)    (m_AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)
#define SetSF(x)        (m_SignVal = (x))
#define SetZF(x)        (m_ZeroVal = (x))
#define SetPF(x)        (m_ParityVal = (x))

#define SetSZPF_Byte(x) (m_SignVal=m_ZeroVal=m_ParityVal=(INT8)(x))
#define SetSZPF_Word(x) (m_SignVal=m_ZeroVal=m_ParityVal=(INT16)(x))

#define SetOFW_Add(x,y,z)   (m_OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)   (m_OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)   (m_OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)   (m_OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define ADDB { UINT32 res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW { UINT32 res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB { UINT32 res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW { UINT32 res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB dst|=src; m_CarryVal=m_OverVal=m_AuxVal=0; SetSZPF_Byte(dst)
#define ORW dst|=src; m_CarryVal=m_OverVal=m_AuxVal=0; SetSZPF_Word(dst)

#define ANDB dst&=src; m_CarryVal=m_OverVal=m_AuxVal=0; SetSZPF_Byte(dst)
#define ANDW dst&=src; m_CarryVal=m_OverVal=m_AuxVal=0; SetSZPF_Word(dst)

#define XORB dst^=src; m_CarryVal=m_OverVal=m_AuxVal=0; SetSZPF_Byte(dst)
#define XORW dst^=src; m_CarryVal=m_OverVal=m_AuxVal=0; SetSZPF_Word(dst)

#define IncWordReg(Reg)                     \
	unsigned tmp = (unsigned)Wreg(Reg); \
	unsigned tmp1 = tmp+1;                  \
	m_OverVal = (tmp == 0x7fff);           \
	SetAF(tmp1,tmp,1);                      \
	SetSZPF_Word(tmp1);                     \
	Wreg(Reg)=tmp1

#define DecWordReg(Reg)                     \
	unsigned tmp = (unsigned)Wreg(Reg); \
	unsigned tmp1 = tmp-1;                  \
	m_OverVal = (tmp == 0x8000);           \
	SetAF(tmp1,tmp,1);                      \
	SetSZPF_Word(tmp1);                     \
	Wreg(Reg)=tmp1

#define JMP(flag)                           \
	int tmp;                                \
	EMPTY_PREFETCH();                       \
	tmp = (int)((INT8)FETCH());             \
	if (flag)                               \
	{                                       \
		static const UINT8 table[3]={3,10,10};  \
		m_ip = (WORD)(m_ip+tmp);          \
		m_icount-=table[m_chip_type/8];   \
		CHANGE_PC;                          \
		return;                             \
	}

#define ADJ4(param1,param2)                 \
	if (AF || ((Breg(AL) & 0xf) > 9))   \
	{                                       \
		UINT16 tmp;                         \
		tmp = Breg(AL) + param1;        \
		Breg(AL) = tmp;                 \
		m_AuxVal = 1;                      \
		m_CarryVal |= tmp & 0x100;         \
	}                                       \
	if (CF || (Breg(AL)>0x9f))          \
	{                                       \
		Breg(AL) += param2;             \
		m_CarryVal = 1;                        \
	}                                       \
	SetSZPF_Byte(Breg(AL))

#define ADJB(param1,param2)                 \
	if (AF || ((Breg(AL) & 0xf) > 9))   \
	{                                       \
		Breg(AL) += param1;             \
		Breg(AH) += param2;             \
		m_AuxVal = 1;                      \
		m_CarryVal = 1;                        \
	}                                       \
	else                                    \
	{                                       \
		m_AuxVal = 0;                      \
		m_CarryVal = 0;                        \
	}                                       \
	Breg(AL) &= 0x0F

#define BITOP_BYTE                          \
	ModRM = FETCH();                            \
	if (ModRM >= 0xc0) {                    \
		tmp=Breg(Mod_RM.RM.b[ModRM]);   \
	}                                       \
	else {                                  \
		(this->*s_GetEA[ModRM])();                 \
		tmp=read_mem_byte(m_EA);                  \
	}

#define BITOP_WORD                          \
	ModRM = FETCH();                            \
	if (ModRM >= 0xc0) {                    \
		tmp=Wreg(Mod_RM.RM.w[ModRM]);   \
	}                                       \
	else {                                  \
		(this->*s_GetEA[ModRM])();                 \
		tmp=read_mem_word(m_EA);                  \
	}

#define BIT_NOT                             \
	if (tmp & (1<<tmp2))                    \
		tmp &= ~(1<<tmp2);                  \
	else                                    \
		tmp |= (1<<tmp2)

#define XchgAWReg(Reg)                      \
	WORD tmp;                               \
	tmp = Wreg(Reg);                    \
	Wreg(Reg) = Wreg(AW);           \
	Wreg(AW) = tmp

#define ROL_BYTE m_CarryVal = dst & 0x80; dst = (dst << 1)+CF
#define ROL_WORD m_CarryVal = dst & 0x8000; dst = (dst << 1)+CF
#define ROR_BYTE m_CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_WORD m_CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#define ROLC_BYTE dst = (dst << 1) + CF; SetCFB(dst)
#define ROLC_WORD dst = (dst << 1) + CF; SetCFW(dst)
#define RORC_BYTE dst = (CF<<8)+dst; m_CarryVal = dst & 0x01; dst >>= 1
#define RORC_WORD dst = (CF<<16)+dst; m_CarryVal = dst & 0x01; dst >>= 1
#define SHL_BYTE(c) m_icount-=c; dst <<= c;    SetCFB(dst); SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHL_WORD(c) m_icount-=c; dst <<= c;    SetCFW(dst); SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)
#define SHR_BYTE(c) m_icount-=c; dst >>= c-1; m_CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHR_WORD(c) m_icount-=c; dst >>= c-1; m_CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)
#define SHRA_BYTE(c) m_icount-=c; dst = ((INT8)dst) >> (c-1);  m_CarryVal = dst & 0x1;    dst = ((INT8)((BYTE)dst)) >> 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHRA_WORD(c) m_icount-=c; dst = ((INT16)dst) >> (c-1); m_CarryVal = dst & 0x1;    dst = ((INT16)((WORD)dst)) >> 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)

#define DIVUB                                               \
	uresult = Wreg(AW);                                 \
	uresult2 = uresult % tmp;                               \
	if ((uresult /= tmp) > 0xff) {                          \
		nec_interrupt(NEC_DIVIDE_VECTOR, BRK); break;                            \
	} else {                                                \
		Breg(AL) = uresult;                             \
		Breg(AH) = uresult2;                            \
	}

#define DIVB                                                \
	result = (INT16)Wreg(AW);                           \
	result2 = result % (INT16)((INT8)tmp);                  \
	if ((result /= (INT16)((INT8)tmp)) > 0xff) {            \
		nec_interrupt(NEC_DIVIDE_VECTOR, BRK); break;                            \
	} else {                                                \
		Breg(AL) = result;                              \
		Breg(AH) = result2;                             \
	}

#define DIVUW                                               \
	uresult = (((UINT32)Wreg(DW)) << 16) | Wreg(AW);\
	uresult2 = uresult % tmp;                               \
	if ((uresult /= tmp) > 0xffff) {                        \
		nec_interrupt(NEC_DIVIDE_VECTOR, BRK); break;                            \
	} else {                                                \
		Wreg(AW)=uresult;                               \
		Wreg(DW)=uresult2;                              \
	}

#define DIVW                                                \
	result = ((UINT32)Wreg(DW) << 16) + Wreg(AW);   \
	result2 = result % (INT32)((INT16)tmp);                 \
	if ((result /= (INT32)((INT16)tmp)) > 0xffff) {         \
		nec_interrupt(NEC_DIVIDE_VECTOR, BRK); break;                            \
	} else {                                                \
		Wreg(AW)=result;                                \
		Wreg(DW)=result2;                               \
	}

#define ADD4S {                                             \
	int i,v1,v2,result;                                     \
	int count = (Breg(CL)+1)/2;                         \
	unsigned di = Wreg(IY);                             \
	unsigned si = Wreg(IX);                             \
	static const UINT8 table[3]={18,19,19};                 \
	if (m_seg_prefix) logerror("%06x: Warning: seg_prefix defined for add4s\n",PC()); \
	m_ZeroVal = m_CarryVal = 0;                               \
	for (i=0;i<count;i++) {                                 \
		m_icount-=table[m_chip_type/8];                   \
		tmp = GetMemB(DS0, si);                             \
		tmp2 = GetMemB(DS1, di);                            \
		v1 = (tmp>>4)*10 + (tmp&0xf);                       \
		v2 = (tmp2>>4)*10 + (tmp2&0xf);                     \
		result = v1+v2+m_CarryVal;                         \
		m_CarryVal = result > 99 ? 1 : 0;                  \
		result = result % 100;                              \
		v1 = ((result/10)<<4) | (result % 10);              \
		PutMemB(DS1, di,v1);                                \
		if (v1) m_ZeroVal = 1;                             \
		si++;                                               \
		di++;                                               \
	}                                                       \
}

#define SUB4S {                                             \
	int count = (Breg(CL)+1)/2;                         \
	int i,v1,v2,result;                                     \
	unsigned di = Wreg(IY);                             \
	unsigned si = Wreg(IX);                             \
	static const UINT8 table[3]={18,19,19};                 \
	if (m_seg_prefix) logerror("%06x: Warning: seg_prefix defined for sub4s\n",PC()); \
	m_ZeroVal = m_CarryVal = 0;                               \
	for (i=0;i<count;i++) {                                 \
		m_icount-=table[m_chip_type/8];                   \
		tmp = GetMemB(DS1, di);                             \
		tmp2 = GetMemB(DS0, si);                            \
		v1 = (tmp>>4)*10 + (tmp&0xf);                       \
		v2 = (tmp2>>4)*10 + (tmp2&0xf);                     \
		if (v1 < (v2+m_CarryVal)) {                            \
			v1+=100;                                        \
			result = v1-(v2+m_CarryVal);                   \
			m_CarryVal = 1;                                    \
		} else {                                            \
			result = v1-(v2+m_CarryVal);                   \
			m_CarryVal = 0;                                    \
		}                                                   \
		v1 = ((result/10)<<4) | (result % 10);              \
		PutMemB(DS1, di,v1);                                \
		if (v1) m_ZeroVal = 1;                             \
		si++;                                               \
		di++;                                               \
	}                                                       \
}

#define CMP4S {                                             \
	int count = (Breg(CL)+1)/2;                         \
	int i,v1,v2,result;                                     \
	unsigned di = Wreg(IY);                             \
	unsigned si = Wreg(IX);                             \
	static const UINT8 table[3]={14,19,19};                 \
	if (m_seg_prefix) logerror("%06x: Warning: seg_prefix defined for cmp4s\n",PC()); \
	m_ZeroVal = m_CarryVal = 0;                               \
	for (i=0;i<count;i++) {                                 \
		m_icount-=table[m_chip_type/8];                   \
		tmp = GetMemB(DS1, di);                             \
		tmp2 = GetMemB(DS0, si);                            \
		v1 = (tmp>>4)*10 + (tmp&0xf);                       \
		v2 = (tmp2>>4)*10 + (tmp2&0xf);                     \
		if (v1 < (v2+m_CarryVal)) {                            \
			v1+=100;                                        \
			result = v1-(v2+m_CarryVal);                   \
			m_CarryVal = 1;                                    \
		} else {                                            \
			result = v1-(v2+m_CarryVal);                   \
			m_CarryVal = 0;                                    \
		}                                                   \
		v1 = ((result/10)<<4) | (result % 10);              \
		if (v1) m_ZeroVal = 1;                             \
		si++;                                               \
		di++;                                               \
	}                                                       \
}
