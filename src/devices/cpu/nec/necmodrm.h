// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
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

#define RegWord(ModRM) Wreg(Mod_RM.reg.w[ModRM])
#define RegByte(ModRM) Breg(Mod_RM.reg.b[ModRM])

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? Wreg(Mod_RM.RM.w[ModRM]) : ((this->*s_GetEA[ModRM])(), read_mem_word(m_EA)))

#define PutbackRMWord(ModRM,val)          \
{                                         \
	if (ModRM >= 0xc0) Wreg(Mod_RM.RM.w[ModRM]) = val; \
	else write_mem_word(m_EA,val);        \
}

#define GetnextRMWord read_mem_word((m_EA & 0xf0000) | ((m_EA + 2) & 0xffff))

#define PutRMWord(ModRM,val)              \
{                                         \
	if (ModRM >= 0xc0)                    \
		Wreg(Mod_RM.RM.w[ModRM]) = val;   \
	else {                                \
		(this->*s_GetEA[ModRM])();        \
		write_mem_word(m_EA, val);        \
	}                                     \
}

#define PutImmRMWord(ModRM)               \
{                                         \
	WORD val;                             \
	if (ModRM >= 0xc0)                    \
		Wreg(Mod_RM.RM.w[ModRM]) = fetchword(); \
	else {                                \
		(this->*s_GetEA[ModRM])();        \
		val = fetchword();                \
		write_mem_word(m_EA, val);        \
	}                                     \
}

#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? Breg(Mod_RM.RM.b[ModRM]) : read_mem_byte((this->*s_GetEA[ModRM])()))

#define PutRMByte(ModRM,val)              \
{                                         \
	if (ModRM >= 0xc0)                    \
		Breg(Mod_RM.RM.b[ModRM]) = val;   \
	else                                  \
		write_mem_byte((this->*s_GetEA[ModRM])(), val); \
}

#define PutImmRMByte(ModRM)               \
{                                         \
	if (ModRM >= 0xc0)                    \
		Breg(Mod_RM.RM.b[ModRM])=fetch(); \
	else {                                \
		(this->*s_GetEA[ModRM])();        \
		write_mem_byte(m_EA, fetch());    \
	}                                     \
}

#define PutbackRMByte(ModRM,val)          \
{                                         \
	if (ModRM >= 0xc0)                    \
		Breg(Mod_RM.RM.b[ModRM])=val;     \
	else                                  \
		write_mem_byte(m_EA,val);         \
}

#define DEF_br8                           \
	uint32_t ModRM = fetch(),src,dst;     \
	src = RegByte(ModRM);                 \
	dst = GetRMByte(ModRM)

#define DEF_wr16                          \
	uint32_t ModRM = fetch(),src,dst;     \
	src = RegWord(ModRM);                 \
	dst = GetRMWord(ModRM)

#define DEF_r8b                           \
	uint32_t ModRM = fetch(),src,dst;     \
	dst = RegByte(ModRM);                 \
	src = GetRMByte(ModRM)

#define DEF_r16w                          \
	uint32_t ModRM = fetch(),src,dst;     \
	dst = RegWord(ModRM);                 \
	src = GetRMWord(ModRM)

#define DEF_ald8                          \
	uint32_t src = fetch();               \
	uint32_t dst = Breg(AL)

#define DEF_axd16                         \
	uint32_t src = fetch();               \
	uint32_t dst = Wreg(AW);              \
	src += (fetch() << 8)
