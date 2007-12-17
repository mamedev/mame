static struct {
	struct {
		WREGS w[256];
		BREGS b[256];
	} reg;
	struct {
		WREGS w[256];
		BREGS b[256];
	} RM;
} Mod_RM;

#define RegWord(ModRM) I.regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) I.regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? I.regs.w[Mod_RM.RM.w[ModRM]] : ( (*GetEA[ModRM])(), ReadWord( EA ) ))

#define PutbackRMWord(ModRM,val) \
{ \
	if (ModRM >= 0xc0) I.regs.w[Mod_RM.RM.w[ModRM]]=val; \
    else WriteWord(EA,val); \
}

#define GetnextRMWord ReadWord(EA+2)

#define GetRMWordOffset(offs) \
		ReadWord(EA-EO+(UINT16)(EO+offs))

#define GetRMByteOffset(offs) \
		ReadByte(EA-EO+(UINT16)(EO+offs))

#define PutRMWord(ModRM,val)				\
{											\
	if (ModRM >= 0xc0)						\
		I.regs.w[Mod_RM.RM.w[ModRM]]=val;	\
	else {									\
		(*GetEA[ModRM])();					\
		WriteWord( EA ,val);				\
	}										\
}

#define PutRMWordOffset(offs, val) \
		WriteWord( EA-EO+(UINT16)(EO+offs), val)

#define PutRMByteOffset(offs, val) \
		WriteByte( EA-EO+(UINT16)(EO+offs), val)

#define PutImmRMWord(ModRM) 				\
{											\
	WORD val;								\
	if (ModRM >= 0xc0)						\
		FETCHWORD(I.regs.w[Mod_RM.RM.w[ModRM]]) \
	else {									\
		(*GetEA[ModRM])();					\
		FETCHWORD(val)						\
		WriteWord( EA , val);				\
	}										\
}

#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? I.regs.b[Mod_RM.RM.b[ModRM]] : ReadByte( (*GetEA[ModRM])() ))

#define PutRMByte(ModRM,val)				\
{											\
	if (ModRM >= 0xc0)						\
		I.regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else									\
		WriteByte( (*GetEA[ModRM])() ,val); \
}

#define PutImmRMByte(ModRM) 				\
{											\
	if (ModRM >= 0xc0)						\
		I.regs.b[Mod_RM.RM.b[ModRM]]=FETCH; \
	else {									\
		(*GetEA[ModRM])();					\
		WriteByte( EA , FETCH );			\
	}										\
}

#define PutbackRMByte(ModRM,val)			\
{											\
	if (ModRM >= 0xc0)						\
		I.regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else									\
		WriteByte(EA,val);					\
}

#define DEF_br8(dst,src)					\
	unsigned ModRM = FETCHOP;				\
	unsigned src = RegByte(ModRM);			\
    unsigned dst = GetRMByte(ModRM)

#define DEF_wr16(dst,src)					\
	unsigned ModRM = FETCHOP;				\
	unsigned src = RegWord(ModRM);			\
    unsigned dst = GetRMWord(ModRM)

#define DEF_r8b(dst,src)					\
	unsigned ModRM = FETCHOP;				\
	unsigned dst = RegByte(ModRM);			\
    unsigned src = GetRMByte(ModRM)

#define DEF_r16w(dst,src)					\
	unsigned ModRM = FETCHOP;				\
	unsigned dst = RegWord(ModRM);			\
    unsigned src = GetRMWord(ModRM)

#define DEF_ald8(dst,src)					\
	unsigned src = FETCHOP; 				\
	unsigned dst = I.regs.b[AL]

#define DEF_axd16(dst,src)					\
	unsigned src = FETCHOP; 				\
	unsigned dst = I.regs.w[AX];			\
    src += (FETCH << 8)


