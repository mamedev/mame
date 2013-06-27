static struct
{
	struct
	{
		WREGS w[256];
		BREGS b[256];
	} reg;
	struct
	{
		WREGS w[256];
		BREGS b[256];
	} RM;
} Mod_RM;

#define RegWord(ModRM) cpustate->regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) cpustate->regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? cpustate->regs.w[Mod_RM.RM.w[ModRM]] : ( (*GetEA[ModRM])(cpustate), i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_WORD, I80286_READ), ReadWord( cpustate->ea ) ))

#define PutbackRMWord(ModRM,val)                                                                        \
{                                                                                                       \
	if (ModRM >= 0xc0) cpustate->regs.w[Mod_RM.RM.w[ModRM]]=val;                                        \
	else                                                                                                \
	{                                                                                                   \
		i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_WORD, I80286_WRITE);   \
		WriteWord(cpustate->ea,val);                                                                    \
	}                                                                                                   \
}

#define GetnextRMWord                                                                                                                   \
	(                                                                                                                                   \
	i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->ea + 2 - cpustate->base[cpustate->ea_seg], I80286_WORD, I80286_READ), \
	ReadWord(cpustate->ea+2)                                                                                                            \
	)

#define GetRMWordOffset(offs)                                                                                       \
		(                                                                                                           \
		i80286_check_permission(cpustate, cpustate->ea_seg, (UINT16)(cpustate->eo+offs), I80286_WORD, I80286_READ), \
		ReadWord(cpustate->ea-cpustate->eo+(UINT16)(cpustate->eo+offs))                                             \
		)

#define GetRMByteOffset(offs)                                                                                       \
		(                                                                                                           \
		i80286_check_permission(cpustate, cpustate->ea_seg, (UINT16)(cpustate->eo+offs), I80286_BYTE, I80286_READ), \
		ReadByte(cpustate->ea-cpustate->eo+(UINT16)(cpustate->eo+offs))                                             \
		)

#define PutRMWord(ModRM,val)                                                                            \
{                                                                                                       \
	if (ModRM >= 0xc0)                                                                                  \
		cpustate->regs.w[Mod_RM.RM.w[ModRM]]=val;                                                       \
	else {                                                                                              \
		(*GetEA[ModRM])(cpustate);                                                                      \
		i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_WORD, I80286_WRITE);   \
		WriteWord( cpustate->ea ,val);                                                                  \
	}                                                                                                   \
}

#define PutRMWordOffset(offs, val)                                                                                      \
		i80286_check_permission(cpustate, cpustate->ea_seg, (UINT16)(cpustate->eo+offs), I80286_WORD, I80286_WRITE);    \
		WriteWord( cpustate->ea-cpustate->eo+(UINT16)(cpustate->eo+offs), val)

#define PutRMByteOffset(offs, val)                                                                                      \
		i80286_check_permission(cpustate, cpustate->ea_seg, (UINT16)(cpustate->eo+offs), I80286_BYTE, I80286_WRITE);    \
		WriteByte( cpustate->ea-cpustate->eo+(UINT16)(cpustate->eo+offs), val)

#define PutImmRMWord(ModRM)                                                                             \
{                                                                                                       \
	WORD val;                                                                                           \
	if (ModRM >= 0xc0)                                                                                  \
		FETCHWORD(cpustate->regs.w[Mod_RM.RM.w[ModRM]])                                                 \
	else {                                                                                              \
		(*GetEA[ModRM])(cpustate);                                                                      \
		i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_WORD, I80286_WRITE);   \
		FETCHWORD(val)                                                                                  \
		WriteWord( cpustate->ea , val);                                                                 \
	}                                                                                                   \
}

#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? cpustate->regs.b[Mod_RM.RM.b[ModRM]] : ( (*GetEA[ModRM])(cpustate), i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_BYTE, I80286_READ), ReadByte( cpustate->ea )) )

#define PutRMByte(ModRM,val)                                                                            \
{                                                                                                       \
	if (ModRM >= 0xc0)                                                                                  \
		cpustate->regs.b[Mod_RM.RM.b[ModRM]]=val;                                                       \
	else                                                                                                \
	{                                                                                                   \
		(*GetEA[ModRM])(cpustate);                                                                      \
		i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_BYTE, I80286_WRITE);   \
		WriteByte( cpustate->ea,val);                                                                   \
	}                                                                                                   \
}

#define PutImmRMByte(ModRM)                                                                             \
{                                                                                                       \
	if (ModRM >= 0xc0)                                                                                  \
		cpustate->regs.b[Mod_RM.RM.b[ModRM]]=FETCH;                                                     \
	else {                                                                                              \
		(*GetEA[ModRM])(cpustate);                                                                      \
		i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_BYTE, I80286_WRITE);   \
		WriteByte( cpustate->ea , FETCH );                                                              \
	}                                                                                                   \
}

#define PutbackRMByte(ModRM,val)                                                                        \
{                                                                                                       \
	if (ModRM >= 0xc0)                                                                                  \
		cpustate->regs.b[Mod_RM.RM.b[ModRM]]=val;                                                       \
	else                                                                                                \
	{                                                                                                   \
		i80286_check_permission(cpustate, cpustate->ea_seg, cpustate->eo, I80286_BYTE, I80286_WRITE);   \
		WriteByte(cpustate->ea,val);                                                                    \
	}                                                                                                   \
}

#define DEF_br8(dst,src)                    \
	unsigned ModRM = FETCHOP;               \
	unsigned src = RegByte(ModRM);          \
	unsigned dst = GetRMByte(ModRM)

#define DEF_wr16(dst,src)                   \
	unsigned ModRM = FETCHOP;               \
	unsigned src = RegWord(ModRM);          \
	unsigned dst = GetRMWord(ModRM)

#define DEF_r8b(dst,src)                    \
	unsigned ModRM = FETCHOP;               \
	unsigned dst = RegByte(ModRM);          \
	unsigned src = GetRMByte(ModRM)

#define DEF_r16w(dst,src)                   \
	unsigned ModRM = FETCHOP;               \
	unsigned dst = RegWord(ModRM);          \
	unsigned src = GetRMWord(ModRM)

#define DEF_ald8(dst,src)                   \
	unsigned src = FETCHOP;                 \
	unsigned dst = cpustate->regs.b[AL]

#define DEF_axd16(dst,src)                  \
	unsigned src = FETCHOP;                 \
	unsigned dst = cpustate->regs.w[AX];    \
	src += (FETCH << 8)
