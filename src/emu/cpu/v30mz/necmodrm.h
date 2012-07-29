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

#define RegWord(ModRM) cpustate->regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) cpustate->regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? cpustate->regs.w[Mod_RM.RM.w[ModRM]] : ( (*GetEA[ModRM])(cpustate), ReadWord( cpustate->ea ) ))

#define PutbackRMWord(ModRM,val)			     \
{							     \
	if (ModRM >= 0xc0) cpustate->regs.w[Mod_RM.RM.w[ModRM]]=val; \
    else WriteWord(cpustate->ea,val);  \
}

#define GetnextRMWord ReadWord((cpustate->ea&0xf0000)|((cpustate->ea+2)&0xffff))

#define PutRMWord(ModRM,val)				\
{							\
	if (ModRM >= 0xc0)				\
		cpustate->regs.w[Mod_RM.RM.w[ModRM]]=val;	\
	else {						\
		(*GetEA[ModRM])(cpustate);			\
		WriteWord( cpustate->ea ,val);			\
	}						\
}

#define PutImmRMWord(ModRM) 				\
{							\
	UINT16 val;					\
	if (ModRM >= 0xc0)				\
		FETCHWORD(cpustate->regs.w[Mod_RM.RM.w[ModRM]]) \
	else {						\
		(*GetEA[ModRM])(cpustate);			\
		FETCHWORD(val)				\
		WriteWord( cpustate->ea , val);			\
	}						\
}

#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? cpustate->regs.b[Mod_RM.RM.b[ModRM]] : ReadByte( (*GetEA[ModRM])(cpustate) ))

#define PutRMByte(ModRM,val)				\
{							\
	if (ModRM >= 0xc0)				\
		cpustate->regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else						\
		WriteByte( (*GetEA[ModRM])(cpustate) ,val); 	\
}

#define PutImmRMByte(ModRM) 				\
{							\
	if (ModRM >= 0xc0)				\
		cpustate->regs.b[Mod_RM.RM.b[ModRM]]=FETCH; 	\
	else {						\
		(*GetEA[ModRM])(cpustate);			\
		WriteByte( cpustate->ea , FETCH );		\
	}						\
}

#define PutbackRMByte(ModRM,val)			\
{							\
	if (ModRM >= 0xc0)				\
		cpustate->regs.b[Mod_RM.RM.b[ModRM]]=val;	\
	else						\
		WriteByte(cpustate->ea,val);			\
}

#define DEF_br8							\
	UINT32 ModRM = FETCH,src,dst;		\
	src = RegByte(ModRM);				\
    dst = GetRMByte(ModRM)

#define DEF_wr16						\
	UINT32 ModRM = FETCH,src,dst;		\
	src = RegWord(ModRM);				\
    dst = GetRMWord(ModRM)

#define DEF_r8b							\
	UINT32 ModRM = FETCH,src,dst;		\
	dst = RegByte(ModRM);				\
    src = GetRMByte(ModRM)

#define DEF_r16w						\
	UINT32 ModRM = FETCH,src,dst;		\
	dst = RegWord(ModRM);				\
    src = GetRMWord(ModRM)

#define DEF_ald8						\
	UINT32 src = FETCH;					\
	UINT32 dst = cpustate->regs.b[AL]

#define DEF_axd16						\
	UINT32 src = FETCH; 				\
	UINT32 dst = cpustate->regs.w[AW];			\
    src += (FETCH << 8)
