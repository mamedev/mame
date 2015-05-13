// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud, hap
/* ======================================================================== */
/* ============================= CONFIGURATION ============================ */
/* ======================================================================== */

#undef FLAG_SET_M
#undef FLAG_SET_X
#undef m37710i_set_flag_mx
#undef m37710i_set_reg_p

#if EXECUTION_MODE == EXECUTION_MODE_M0X0
#define FLAG_SET_M 0
#define FLAG_SET_X 0
#define m37710i_set_flag_mx m37710i_set_flag_m0x0
#define m37710i_set_reg_p m37710i_set_reg_p_m0x0
#elif EXECUTION_MODE == EXECUTION_MODE_M0X1
#define FLAG_SET_M 0
#define FLAG_SET_X 1
#define m37710i_set_flag_mx m37710i_set_flag_m0x1
#define m37710i_set_reg_p m37710i_set_reg_p_m0x1
#elif EXECUTION_MODE == EXECUTION_MODE_M1X0
#define FLAG_SET_M 1
#define FLAG_SET_X 0
#define m37710i_set_flag_mx m37710i_set_flag_m1x0
#define m37710i_set_reg_p m37710i_set_reg_p_m1x0
#elif EXECUTION_MODE == EXECUTION_MODE_M1X1
#define FLAG_SET_M 1
#define FLAG_SET_X 1
#define m37710i_set_flag_mx m37710i_set_flag_m1x1
#define m37710i_set_reg_p m37710i_set_reg_p_m1x1
#endif

/* ======================================================================== */
/* ============================ STATUS REGISTER =========================== */
/* ======================================================================== */

/* note: difference from 65816.  when switching to 8-bit X/Y, X and Y are *not* truncated
   to 8 bits! */

void m37710_cpu_device::m37710i_set_flag_mx(UINT32 value)
{
#if FLAG_SET_M
	if(!(value & FLAGPOS_M))
	{
		REG_A |= REG_B;
		REG_B = 0;
		REG_BA |= REG_BB;
		REG_BB = 0;
		FLAG_M = MFLAG_CLEAR;
	}
#else
	if(value & FLAGPOS_M)
	{
		REG_B = REG_A & 0xff00;
		REG_A = MAKE_UINT_8(REG_A);
		REG_BB = REG_BA & 0xff00;
		REG_BA = MAKE_UINT_8(REG_BA);
		FLAG_M = MFLAG_SET;
	}
#endif
#if FLAG_SET_X
	if(!(value & FLAGPOS_X))
	{
		REG_X |= REG_XH;
		REG_XH = 0;
		REG_Y |= REG_YH;
		REG_YH = 0;
		FLAG_X = XFLAG_CLEAR;
	}
#else
	if(value & FLAGPOS_X)
	{
		REG_XH = REG_X & 0xff00;
		REG_X = MAKE_UINT_8(REG_X);
		REG_YH = REG_Y & 0xff00;
		REG_Y = MAKE_UINT_8(REG_Y);
		FLAG_X = XFLAG_SET;
	}
#endif
	m37710i_set_execution_mode((FLAG_M>>4) | (FLAG_X>>4));
}


void m37710_cpu_device::m37710i_set_reg_p(UINT32 value)
{
	FLAG_N = value;
	FLAG_V = value << 1;
	FLAG_D = value & FLAGPOS_D;
	FLAG_Z = !(value & FLAGPOS_Z);
	FLAG_C = value << 8;
	m37710i_set_flag_mx(value);
	FLAG_I = value & FLAGPOS_I;
}


/* ======================================================================== */
/* =========================== OPERATION MACROS =========================== */
/* ======================================================================== */

/* M37710  Push all */
#undef OP_PSH

#if FLAG_SET_M
#if FLAG_SET_X
#define OP_PSH(MODE)    \
	SRC = OPER_8_##MODE();  \
	CLK(12); \
	if (SRC&0x1)    \
		{ m37710i_push_8(REG_A);  CLK(2); } \
	if (SRC&0x2)    \
		{ m37710i_push_8(REG_BA); CLK(2); }  \
	if (SRC&0x4)    \
		{ m37710i_push_8(REG_X);  CLK(2); }  \
	if (SRC&0x8)    \
		{ m37710i_push_8(REG_Y);  CLK(2); }  \
	if (SRC&0x10)   \
		{ m37710i_push_16(REG_D); CLK(2); }  \
	if (SRC&0x20)   \
		{ m37710i_push_8(REG_DB>>16); CLK(1); }  \
	if (SRC&0x40)   \
		{ m37710i_push_8(REG_PB>>16); CLK(1); }  \
	if (SRC&0x80)   \
		{ m37710i_push_8(m_ipl); m37710i_push_8(m37710i_get_reg_p()); CLK(2); }
#else   // FLAG_SET_X
#define OP_PSH(MODE)    \
	SRC = OPER_8_##MODE();  \
	CLK(12); \
	if (SRC&0x1)    \
		{ m37710i_push_8(REG_A); CLK(2);  }  \
	if (SRC&0x2)    \
		{ m37710i_push_8(REG_BA); CLK(2); }  \
	if (SRC&0x4)    \
		{ m37710i_push_16(REG_X); CLK(2); }  \
	if (SRC&0x8)    \
		{ m37710i_push_16(REG_Y); CLK(2); }  \
	if (SRC&0x10)   \
		{ m37710i_push_16(REG_D); CLK(2); }  \
	if (SRC&0x20)   \
		{ m37710i_push_8(REG_DB>>16); CLK(1); }  \
	if (SRC&0x40)   \
		{ m37710i_push_8(REG_PB>>16); CLK(1); }  \
	if (SRC&0x80)   \
		{ m37710i_push_8(m_ipl); m37710i_push_8(m37710i_get_reg_p()); CLK(2); }
#endif  // FLAG_SET_X
#else   // FLAG_SET_M
#if FLAG_SET_X
#define OP_PSH(MODE)    \
	SRC = OPER_8_##MODE();  \
	CLK(12); \
	if (SRC&0x1)    \
		{ m37710i_push_16(REG_A); CLK(2); }  \
	if (SRC&0x2)    \
		{ m37710i_push_16(REG_BA); CLK(2); } \
	if (SRC&0x4)    \
		{ m37710i_push_8(REG_X); CLK(2); }   \
	if (SRC&0x8)    \
		{ m37710i_push_8(REG_Y); CLK(2); }   \
	if (SRC&0x10)   \
		{ m37710i_push_16(REG_D); CLK(2); }  \
	if (SRC&0x20)   \
		{ m37710i_push_8(REG_DB>>16); CLK(1); }  \
	if (SRC&0x40)   \
		{ m37710i_push_8(REG_PB>>16); CLK(1); }  \
	if (SRC&0x80)   \
		{ m37710i_push_8(m_ipl); m37710i_push_8(m37710i_get_reg_p()); CLK(2); }
#else   // FLAG_SET_X
#define OP_PSH(MODE)    \
	SRC = OPER_8_##MODE();  \
	CLK(12); \
	if (SRC&0x1)    \
		{ m37710i_push_16(REG_A); CLK(2); }  \
	if (SRC&0x2)    \
		{ m37710i_push_16(REG_BA); CLK(2);}  \
	if (SRC&0x4)    \
		{ m37710i_push_16(REG_X); CLK(2); }  \
	if (SRC&0x8)    \
		{ m37710i_push_16(REG_Y); CLK(2); }  \
	if (SRC&0x10)   \
		{ m37710i_push_16(REG_D); CLK(2); }  \
	if (SRC&0x20)   \
		{ m37710i_push_8(REG_DB>>16); CLK(1); }  \
	if (SRC&0x40)   \
		{ m37710i_push_8(REG_PB>>16); CLK(1); }  \
	if (SRC&0x80)   \
		{ m37710i_push_8(m_ipl); m37710i_push_8(m37710i_get_reg_p()); CLK(2); }
#endif  // FLAG_SET_X
#endif  // FLAG_SET_M

/* M37710  Pull all */
/* Unusual behavior: bit 6 has no effect */
#undef OP_PUL
#define OP_PUL(MODE)    \
	SRC = OPER_8_##MODE();  \
	CLK(14); \
	if (SRC&0x80)   \
		{ m37710i_set_reg_p(m37710i_pull_8()); m37710i_set_reg_ipl(m37710i_pull_8()); CLK(3); } \
	if (SRC&0x20)   \
		{ REG_DB = m37710i_pull_8() << 16; CLK(3); }   \
	if (SRC&0x10)   \
		{ REG_D  = m37710i_pull_16(); CLK(4); }   \
	if (m37710i_get_reg_p() & XFLAG_SET) \
	{ \
		if (SRC&0x8)    \
			{ REG_Y = m37710i_pull_8(); CLK(3); }  \
		if (SRC&0x4)    \
			{ REG_X = m37710i_pull_8(); CLK(3); }  \
	}  \
	else \
	{ \
		if (SRC&0x8)    \
			{ REG_Y = m37710i_pull_16(); CLK(3); } \
		if (SRC&0x4)    \
			{ REG_X = m37710i_pull_16(); CLK(3); } \
	} \
	if (m37710i_get_reg_p() & MFLAG_SET) \
	{ \
		if (SRC&0x2)    \
			{ REG_BA = m37710i_pull_8(); CLK(3); } \
		if (SRC&0x1)    \
			{ REG_A = m37710i_pull_8(); CLK(3); } \
	}  \
	else \
	{ \
		if (SRC&0x2)    \
			{ REG_BA = m37710i_pull_16(); CLK(3); } \
		if (SRC&0x1)    \
			{ REG_A = m37710i_pull_16(); CLK(3); } \
	}   \
	m37710i_update_irqs()

/* M37710   Multiply */
#undef OP_MPY
#if FLAG_SET_M
#define OP_MPY(MODE)                                                        \
	CLK(CLK_OP + CLK_R8 + CLK_##MODE + 14); \
	SRC = OPER_8_##MODE();          \
	{ UINT16 temp = SRC * (REG_A&0xff);  REG_A = temp & 0xff; REG_BA = (temp>>8)&0xff; FLAG_Z = temp; FLAG_N = (temp & 0x8000) ? 1 : 0; FLAG_C = 0; }
#else
#define OP_MPY(MODE)                                                        \
	CLK(CLK_OP + CLK_R16 + CLK_##MODE + 14+8);  \
	SRC = OPER_16_##MODE();         \
	{ UINT32 temp = SRC * REG_A;  REG_A = temp & 0xffff;  REG_BA = (temp>>16)&0xffff; FLAG_Z = temp; FLAG_N = (temp & 0x80000000) ? 1 : 0; FLAG_C = 0; }
#endif

/* M37710   Divide */
#undef OP_DIV
#if FLAG_SET_M
#define OP_DIV(MODE)    \
	CLK(CLK_OP + CLK_R8 + CLK_##MODE + 17); \
	SRC = (REG_BA&0xff)<<8 | (REG_A & 0xff);    \
	DST = OPER_8_##MODE();          \
	if (DST != 0)   \
	{       \
		UINT16 tempa = SRC / DST; UINT16 tempb = SRC % DST;     \
		FLAG_V = ((tempa | tempb) & 0xff00) ? VFLAG_SET : 0;    \
		FLAG_C = FLAG_V ? CFLAG_SET : 0;    \
		if (!FLAG_V) { FLAG_N = (tempa & 0x80) ? 1 : 0; }       \
		FLAG_Z = REG_A = tempa & 0xff; REG_BA = tempb & 0xff;   \
		CLK(8);     \
	} else m37710i_interrupt_software(0xfffc)
#else
#define OP_DIV(MODE)    \
	CLK(CLK_OP + CLK_R16 + CLK_##MODE + 17);    \
	SRC = (REG_BA<<16) | REG_A; \
	DST = OPER_16_##MODE();     \
	if (DST != 0)   \
	{       \
		UINT32 tempa = SRC / DST; UINT32 tempb = SRC % DST;     \
		FLAG_V = ((tempa | tempb) & 0xffff0000) ? VFLAG_SET : 0;    \
		FLAG_C = FLAG_V ? CFLAG_SET : 0;    \
		if (!FLAG_V) { FLAG_N = (tempa & 0x8000) ? 1 : 0; }     \
		FLAG_Z = REG_A = tempa & 0xffff; REG_BA = tempb & 0xffff;   \
		CLK(8+15);  \
	} else m37710i_interrupt_software(0xfffc)
#endif

/* M37710   Add With Carry */
#undef OP_ADC
#if FLAG_SET_M
#define OP_ADC(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			SRC    = OPER_8_##MODE();                                       \
			FLAG_C = REG_A + SRC + CFLAG_AS_1();                        \
			if(FLAG_D)                                                      \
			{                                                               \
				if((FLAG_C & 0xf) > 9)                                      \
					FLAG_C+=6;                                              \
				if((FLAG_C & 0xf0) > 0x90)                                  \
					FLAG_C+=0x60;                                           \
			}                                                               \
			FLAG_V = VFLAG_ADD_8(SRC, REG_A, FLAG_C);                   \
			FLAG_N = FLAG_Z = REG_A = MAKE_UINT_8(FLAG_C)
#else
#define OP_ADC(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			SRC    = OPER_16_##MODE();                                      \
			if(!FLAG_D)                                                     \
			{                                                               \
				FLAG_C = REG_A + SRC + CFLAG_AS_1();                        \
				FLAG_V = VFLAG_ADD_16(SRC, REG_A, FLAG_C);                  \
				FLAG_Z = REG_A = MAKE_UINT_16(FLAG_C);                      \
				FLAG_N = NFLAG_16(REG_A);                                   \
				FLAG_C = CFLAG_16(FLAG_C);                                  \
				BREAKOUT;                                                   \
			}                                                               \
			FLAG_C = MAKE_UINT_8(REG_A) + MAKE_UINT_8(SRC) + CFLAG_AS_1();  \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C+=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C+=0x60;                                               \
			FLAG_Z = MAKE_UINT_8(FLAG_C);                                   \
																			\
			FLAG_C = MAKE_UINT_8(REG_A>>8) + MAKE_UINT_8(SRC>>8) + CFLAG_AS_1();    \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C+=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C+=0x60;                                               \
			FLAG_Z |= MAKE_UINT_8(FLAG_C) << 8;                             \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			FLAG_V = VFLAG_ADD_16(SRC, REG_A, FLAG_C);                  \
			REG_A  = FLAG_Z
#endif

/* M37710   Add With Carry - B accumulator*/
#undef OP_ADCB
#if FLAG_SET_M
#define OP_ADCB(MODE)                                                       \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			SRC    = OPER_8_##MODE();                                       \
			FLAG_C = REG_BA + SRC + CFLAG_AS_1();                       \
			if(FLAG_D)                                                      \
			{                                                               \
				if((FLAG_C & 0xf) > 9)                                      \
					FLAG_C+=6;                                              \
				if((FLAG_C & 0xf0) > 0x90)                                  \
					FLAG_C+=0x60;                                           \
			}                                                               \
			FLAG_V = VFLAG_ADD_8(SRC, REG_BA, FLAG_C);                  \
			FLAG_N = FLAG_Z = REG_BA = MAKE_UINT_8(FLAG_C)
#else
#define OP_ADCB(MODE)                                                       \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			SRC    = OPER_16_##MODE();                                      \
			if(!FLAG_D)                                                     \
			{                                                               \
				FLAG_C = REG_BA + SRC + CFLAG_AS_1();                       \
				FLAG_V = VFLAG_ADD_16(SRC, REG_BA, FLAG_C);                 \
				FLAG_Z = REG_BA = MAKE_UINT_16(FLAG_C);                     \
				FLAG_N = NFLAG_16(REG_BA);                                  \
				FLAG_C = CFLAG_16(FLAG_C);                                  \
				BREAKOUT;                                                   \
			}                                                               \
			FLAG_C = MAKE_UINT_8(REG_BA) + MAKE_UINT_8(SRC) + CFLAG_AS_1(); \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C+=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C+=0x60;                                               \
			FLAG_Z = MAKE_UINT_8(FLAG_C);                                   \
																			\
			FLAG_C = MAKE_UINT_8(REG_BA>>8) + MAKE_UINT_8(SRC>>8) + CFLAG_AS_1();   \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C+=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C+=0x60;                                               \
			FLAG_Z |= MAKE_UINT_8(FLAG_C) << 8;                             \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			FLAG_V = VFLAG_ADD_16(SRC, REG_BA, FLAG_C);                 \
			REG_BA  = FLAG_Z
#endif

/* M37710   Logical AND with accumulator */
#undef OP_AND
#if FLAG_SET_M
#define OP_AND(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_A &= OPER_8_##MODE()
#else
#define OP_AND(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_A &= OPER_16_##MODE();                             \
			FLAG_N = NFLAG_16(REG_A)
#endif

/* M37710   Logical AND with B accumulator */
#undef OP_ANDB
#if FLAG_SET_M
#define OP_ANDB(MODE)                                                       \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_BA &= OPER_8_##MODE()
#else
#define OP_ANDB(MODE)                                                       \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_BA &= OPER_16_##MODE();                                \
			FLAG_N = NFLAG_16(REG_BA)
#endif

/* M37710   Arithmetic Shift Left accumulator */
#undef OP_ASL
#if FLAG_SET_M
#define OP_ASL()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = REG_A << 1;                                            \
			FLAG_N = FLAG_Z = REG_A = MAKE_UINT_8(FLAG_C)
#else
#define OP_ASL()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = REG_A << 1;                                            \
			FLAG_Z = REG_A = MAKE_UINT_16(FLAG_C);                          \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = CFLAG_16(FLAG_C)
#endif

/* M37710   Arithmetic Shift Left B accumulator */
#undef OP_BSL
#if FLAG_SET_M
#define OP_BSL()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = REG_BA << 1;                                           \
			FLAG_N = FLAG_Z = REG_BA = MAKE_UINT_8(FLAG_C)
#else
#define OP_BSL()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = REG_BA << 1;                                           \
			FLAG_Z = REG_BA = MAKE_UINT_16(FLAG_C);                         \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = CFLAG_16(FLAG_C)
#endif

/* M37710   Arithmetic Shift Left operand */
#undef OP_ASLM
#if FLAG_SET_M
#define OP_ASLM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST    = EA_##MODE();                                           \
			FLAG_C = read_8_##MODE(DST) << 1;                               \
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);                          \
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_ASLM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST    = EA_##MODE();                                           \
			FLAG_C = read_16_##MODE(DST) << 1;                              \
			FLAG_Z = MAKE_UINT_16(FLAG_C);                                  \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = CFLAG_16(FLAG_C);                                      \
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M37710   Branch on Condition Code */
#undef OP_BCC
#define OP_BCC(COND)                                                        \
			DST = OPER_8_IMM();                                             \
			if(COND)                                                        \
			{                                                               \
				CLK(CLK_OP + CLK_RELATIVE_8 + 1);                           \
				m37710i_branch_8(DST);                                        \
				BREAKOUT;                                                   \
			}                                                               \
			CLK(CLK_OP + CLK_RELATIVE_8);
/* M37710   Cause a Break interrupt */
#undef OP_BRK
#define OP_BRK()                                                            \
			REG_PC++; CLK(CLK_OP + CLK_R8 + CLK_IMM);                       \
			logerror("error M37710: BRK at PC=%06x\n", REG_PB|REG_PC);      \
			m37710i_interrupt_software(0xfffa)

/* M37710  Branch Always */
#undef OP_BRA
#define OP_BRA()                                                            \
			CLK(CLK_OP + CLK_IMPLIED + CLK_RELATIVE_8);                     \
			m37710i_branch_8(OPER_8_IMM())

/* M37710  Branch Always Long */
#undef OP_BRL
#define OP_BRL()                                                            \
			CLK(CLK_OP + CLK_IMPLIED + CLK_RELATIVE_16);                    \
			m37710i_branch_16(OPER_16_IMM())

/* M37710   Clear Carry flag */
#undef OP_CLC
#define OP_CLC()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = CFLAG_CLEAR

/* M37710   Clear Interrupt Mask flag */
#undef OP_CLI
#define OP_CLI()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_I = IFLAG_CLEAR;                      \
			m37710i_update_irqs()

/* M37710   Clear oVerflow flag */
#undef OP_CLV
#define OP_CLV()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_V = VFLAG_CLEAR

/* M37710   Compare operand to accumulator */
/* Unusual behavior: C flag is inverted */
#undef OP_CMP
#if FLAG_SET_M
#define OP_CMP(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_C = REG_A - OPER_8_##MODE();                               \
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);                          \
			FLAG_C ^= CFLAG_SET
#else
#define OP_CMP(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_C = REG_A - OPER_16_##MODE();                              \
			FLAG_Z = MAKE_UINT_16(FLAG_C);                                  \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = ~CFLAG_16(FLAG_C)
#endif

/* M37710   Compare operand to B accumulator */
/* Unusual behavior: C flag is inverted */
#undef OP_CMPB
#if FLAG_SET_M
#define OP_CMPB(MODE)                                                       \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_C = REG_BA - OPER_8_##MODE();                              \
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);                          \
			FLAG_C ^= CFLAG_SET
#else
#define OP_CMPB(MODE)                                                       \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_C = REG_BA - OPER_16_##MODE();                             \
			FLAG_Z = MAKE_UINT_16(FLAG_C);                                  \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = ~CFLAG_16(FLAG_C)
#endif

/* M37710   Compare operand to index register */
/* Unusual behavior: C flag is inverted */
#undef OP_CMPX
#if FLAG_SET_X
#define OP_CMPX(REG, MODE)                                                  \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_C = REG - OPER_8_##MODE();                                 \
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);                          \
			FLAG_C ^= CFLAG_SET
#else
#define OP_CMPX(REG, MODE)                                                  \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_C = REG - OPER_16_##MODE();                                \
			FLAG_Z = MAKE_UINT_16(FLAG_C);                                  \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = ~CFLAG_16(FLAG_C)
#endif

/* M37710   Decrement accumulator */
#undef OP_DEC
#if FLAG_SET_M
#define OP_DEC()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = FLAG_Z = REG_A = MAKE_UINT_8(REG_A - 1)
#else
#define OP_DEC()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_A = MAKE_UINT_16(REG_A - 1);                       \
			FLAG_N = NFLAG_16(REG_A)
#endif

/* M37710   Decrement B accumulator */
#undef OP_DECB
#if FLAG_SET_M
#define OP_DECB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = FLAG_Z = REG_BA = MAKE_UINT_8(REG_BA - 1)
#else
#define OP_DECB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_BA = MAKE_UINT_16(REG_BA - 1);                     \
			FLAG_N = NFLAG_16(REG_BA)
#endif

/* M37710   Decrement operand */
#undef OP_DECM
#if FLAG_SET_M
#define OP_DECM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST    = EA_##MODE();                                           \
			FLAG_N = FLAG_Z = MAKE_UINT_8(read_8_##MODE(DST) - 1);          \
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_DECM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST    = EA_##MODE();                                           \
			FLAG_Z = MAKE_UINT_16(read_16_##MODE(DST) - 1);                 \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M37710   Decrement index register */
#undef OP_DECX
#if FLAG_SET_X
#define OP_DECX(REG)                                                        \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = FLAG_Z = REG = MAKE_UINT_8(REG - 1)
#else
#define OP_DECX(REG)                                                        \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = MAKE_UINT_16(REG - 1);                           \
			FLAG_N = NFLAG_16(REG)
#endif

/* M37710   Exclusive Or operand to accumulator */
#undef OP_EOR
#if FLAG_SET_M
#define OP_EOR(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_A ^= OPER_8_##MODE()
#else
#define OP_EOR(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_A ^= OPER_16_##MODE();                             \
			FLAG_N = NFLAG_16(REG_A)
#endif

/* M37710   Exclusive Or operand to accumulator B */
#undef OP_EORB
#if FLAG_SET_M
#define OP_EORB(MODE)                                                       \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_BA ^= OPER_8_##MODE()
#else
#define OP_EORB(MODE)                                                       \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_BA ^= OPER_16_##MODE();                                \
			FLAG_N = NFLAG_16(REG_BA)
#endif

/* M37710   Increment accumulator */
#undef OP_INC
#if FLAG_SET_M
#define OP_INC()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = FLAG_Z = REG_A = MAKE_UINT_8(REG_A + 1)
#else
#define OP_INC()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_A = MAKE_UINT_16(REG_A + 1);                       \
			FLAG_N = NFLAG_16(REG_A)
#endif

/* M37710   Increment B accumulator */
#undef OP_INCB
#if FLAG_SET_M
#define OP_INCB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = FLAG_Z = REG_BA = MAKE_UINT_8(REG_BA + 1)
#else
#define OP_INCB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_BA = MAKE_UINT_16(REG_BA + 1);                     \
			FLAG_N = NFLAG_16(REG_BA)
#endif

/* M37710   Increment operand */
#undef OP_INCM
#if FLAG_SET_M
#define OP_INCM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST    = EA_##MODE();                                           \
			FLAG_N = FLAG_Z = MAKE_UINT_8(read_8_##MODE(DST) + 1);          \
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_INCM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST    = EA_##MODE();                                           \
			FLAG_Z = MAKE_UINT_16(read_16_##MODE(DST) + 1);                 \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M37710   Increment index register */
#undef OP_INCX
#if FLAG_SET_X
#define OP_INCX(REG)                                                        \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = FLAG_Z = REG = MAKE_UINT_8(REG + 1)
#else
#define OP_INCX(REG)                                                        \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = MAKE_UINT_16(REG + 1);                           \
			FLAG_N = NFLAG_16(REG)
#endif

/* M37710  Jump Long */
#undef OP_JMLAI
#define OP_JMLAI()                                                          \
			CLK(CLK_OP + CLK_AI + 1);                                       \
			m37710i_jump_24(read_24_A(OPER_16_IMM()))

/* M37710   Jump */
#undef OP_JMP
#define OP_JMP(MODE)                                                        \
			CLK(CLK_OP + CLK_##MODE);                                       \
			m37710i_jump_16(EA_##MODE())

/* M37710   Jump absolute indexed indirect */
#undef OP_JMPAXI
#define OP_JMPAXI()                                                         \
			CLK(CLK_OP + CLK_AXI);                                          \
			m37710i_jump_16(read_16_AXI(REG_PB | (MAKE_UINT_16(OPER_16_IMM() + REG_X))))

/* M37710  Jump absolute long */
#undef OP_JMPAL
#define OP_JMPAL()                                                          \
			CLK(CLK_OP + CLK_AL);                                           \
			m37710i_jump_24(EA_AL())

/* M37710  Jump to Subroutine Long */
#undef OP_JSL
#define OP_JSL(MODE)                                                        \
			CLK(CLK_OP + CLK_W24 + CLK_##MODE + 1);                         \
			DST = EA_##MODE();                                              \
			m37710i_push_8(REG_PB>>16);                                       \
			m37710i_push_16(REG_PC);                                      \
			m37710i_jump_24(DST)

/* M37710   Jump to Subroutine */
#undef OP_JSR
#define OP_JSR(MODE)                                                        \
			CLK(CLK_OP + CLK_W16 + CLK_##MODE);                             \
			DST = EA_##MODE();                                              \
			m37710i_push_16(REG_PC);                                      \
			m37710i_jump_16(DST)

/* M37710   Jump to Subroutine */
#undef OP_JSRAXI
#define OP_JSRAXI()                                                         \
			CLK(CLK_OP + CLK_W16 + CLK_AXI);                                \
			DST = read_16_AXI(REG_PB | (MAKE_UINT_16(OPER_16_IMM() + REG_X))); \
			m37710i_push_16(REG_PC);                                      \
			m37710i_jump_16(DST)

/* M37710   Load accumulator with operand */
#undef OP_LDA
#if FLAG_SET_M
#define OP_LDA(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_A = OPER_8_##MODE()
#else
#define OP_LDA(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_A = OPER_16_##MODE();                              \
			FLAG_N = NFLAG_16(REG_A)
#endif

/* M37710   Load B accumulator with operand */
#undef OP_LDB
#if FLAG_SET_M
#define OP_LDB(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_BA = OPER_8_##MODE()
#else
#define OP_LDB(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_BA = OPER_16_##MODE();                             \
			FLAG_N = NFLAG_16(REG_BA)
#endif

/* M37710   Load memory with operand */
#undef OP_LDM
#if FLAG_SET_M
#define OP_LDM(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);  \
			REG_IM2 = EA_##MODE();      \
			REG_IM = read_8_IMM(REG_PB | REG_PC);        \
			REG_PC++;               \
			write_8_##MODE(REG_IM2, REG_IM)
#else
#define OP_LDM(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE); \
			REG_IM2 = EA_##MODE();      \
			REG_IM = read_16_IMM(REG_PB | REG_PC);       \
			REG_PC+=2;              \
			write_16_##MODE(REG_IM2, REG_IM)
#endif

/* M37710   Branch if bits set */
#undef OP_BBS
#if FLAG_SET_M
#define OP_BBS(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);  \
			REG_IM2 = read_8_NORM(EA_##MODE());     \
			REG_IM = read_8_IMM(REG_PB | REG_PC);      \
			REG_PC++;               \
			DST = OPER_8_IMM();         \
			if ((REG_IM2 & REG_IM) == REG_IM)   \
			{                   \
				CLK(CLK_OP + CLK_RELATIVE_8 + 1);   \
				m37710i_branch_8(DST);        \
				BREAKOUT;           \
			}
#else
#define OP_BBS(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE); \
			REG_IM2 = read_16_NORM(EA_##MODE());    \
			REG_IM = read_16_IMM(REG_PB | REG_PC);     \
			REG_PC++;               \
			REG_PC++;               \
			DST = OPER_8_IMM();         \
			if ((REG_IM2 & REG_IM) == REG_IM)   \
			{                   \
				CLK(CLK_OP + CLK_RELATIVE_8 + 1);   \
				m37710i_branch_8(DST);        \
				BREAKOUT;           \
			}
#endif

/* M37710   Branch if bits clear */
#undef OP_BBC
#if FLAG_SET_M
#define OP_BBC(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);  \
			REG_IM2 = read_8_NORM(EA_##MODE());     \
			REG_IM = read_8_IMM(REG_PB | REG_PC);      \
			REG_PC++;               \
			DST = OPER_8_IMM();         \
			if ((REG_IM2 & REG_IM) == 0)        \
			{                   \
				CLK(CLK_OP + CLK_RELATIVE_8 + 1);   \
				m37710i_branch_8(DST);        \
				BREAKOUT;           \
			}
#else
#define OP_BBC(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE); \
			REG_IM2 = read_16_NORM(EA_##MODE());    \
			REG_IM = read_16_IMM(REG_PB | REG_PC);     \
			REG_PC++;               \
			REG_PC++;               \
			DST = OPER_8_IMM();         \
			if ((REG_IM2 & REG_IM) == 0)        \
			{                   \
				CLK(CLK_OP + CLK_RELATIVE_8 + 1);   \
				m37710i_branch_8(DST);        \
				BREAKOUT;           \
			}
#endif

/* M37710   Swap accumulators */
#undef OP_XAB
#if FLAG_SET_M
#define OP_XAB()                                                        \
			CLK(6);                     \
			DST = REG_A;                    \
			FLAG_N = FLAG_Z = REG_A = REG_BA;       \
			REG_BA = DST;
#else
#define OP_XAB()                                                        \
			CLK(6);                     \
			DST = REG_A;                    \
			FLAG_Z = REG_A = REG_BA;            \
			FLAG_N = NFLAG_16(REG_A);           \
			REG_BA = DST;
#endif

/* M37710   Load index register with operand */
#undef OP_LDX
#if FLAG_SET_X
#define OP_LDX(REG, MODE)                                                   \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG = OPER_8_##MODE()
#else
#define OP_LDX(REG, MODE)                                                   \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG = OPER_16_##MODE();                                \
			FLAG_N = NFLAG_16(REG)
#endif

/* M37710   Logical Shift Right accumulator */
#undef OP_LSR
#if FLAG_SET_M
#define OP_LSR()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = 0;                                                     \
			FLAG_C = REG_A << 8;                                            \
			FLAG_Z = REG_A >>= 1
#else
#define OP_LSR()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = 0;                                                     \
			FLAG_C = REG_A << 8;                                            \
			FLAG_Z = REG_A >>= 1
#endif

/* M37710   Logical Shift Right B accumulator */
#undef OP_LSRB
#if FLAG_SET_M
#define OP_LSRB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = 0;                                                     \
			FLAG_C = REG_BA << 8;                                           \
			FLAG_Z = REG_BA >>= 1
#else
#define OP_LSRB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_N = 0;                                                     \
			FLAG_C = REG_BA << 8;                                           \
			FLAG_Z = REG_BA >>= 1
#endif

/* M37710   Logical Shift Right operand */
#undef OP_LSRM
#if FLAG_SET_M
#define OP_LSRM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST    = EA_##MODE();                                           \
			FLAG_N = 0;                                                     \
			FLAG_Z = read_8_##MODE(DST);                                    \
			FLAG_C = FLAG_Z << 8;                                           \
			FLAG_Z >>= 1;                                                   \
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_LSRM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST    = EA_##MODE();                                           \
			FLAG_N = 0;                                                     \
			FLAG_Z = read_16_##MODE(DST);                                   \
			FLAG_C = FLAG_Z << 8;                                           \
			FLAG_Z >>= 1;                                                   \
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M37710  Move Block Negative */
#undef OP_MVN
#if FLAG_SET_X
#define OP_MVN()                                                            \
			DST = OPER_8_IMM()<<16;                                         \
			SRC = OPER_8_IMM()<<16;                                         \
			REG_DB = DST;   \
			REG_A |= REG_B;                                                 \
			CLK(7);                                             \
			if (REG_A > 0)                              \
			{                                                               \
				write_8_NORM(DST | REG_Y, read_8_NORM(SRC | REG_X));        \
				REG_X = MAKE_UINT_8(REG_X+1);                               \
				REG_Y = MAKE_UINT_8(REG_Y+1);                               \
				REG_A--;                                \
				if ((REG_A&0xffff) != 0) \
				{\
					REG_PC -= 3; \
				}\
				else \
				{ \
					if (FLAG_M) \
					{ \
						REG_A = 0xff; \
						REG_B = 0xff00; \
					}                                                               \
					else \
					{                                                               \
						REG_A = 0xffff;                                             \
					}                                                               \
				} \
			}
#else
#define OP_MVN()                                                            \
			DST = OPER_8_IMM()<<16;                                         \
			SRC = OPER_8_IMM()<<16;                                         \
			REG_DB = DST;   \
			REG_A |= REG_B;                                                 \
			CLK(7);                                             \
			if (REG_A > 0)                              \
			{                                                               \
				write_8_NORM(DST | REG_Y, read_8_NORM(SRC | REG_X));        \
				REG_X = MAKE_UINT_16(REG_X+1);                              \
				REG_Y = MAKE_UINT_16(REG_Y+1);                              \
				REG_A--;                                \
				if ((REG_A&0xffff) != 0) \
				{\
					REG_PC -= 3; \
				}\
				else \
				{ \
					if (FLAG_M) \
					{ \
						REG_A = 0xff; \
						REG_B = 0xff00; \
					}                                                               \
					else \
					{                                                               \
						REG_A = 0xffff;                                             \
					}                                                               \
				} \
			}
#endif

/* M37710  Move Block Positive */
#undef OP_MVP
#if FLAG_SET_X
#define OP_MVP()                                                            \
			DST = OPER_8_IMM()<<16;                                         \
			SRC = OPER_8_IMM()<<16;                                         \
			REG_DB = DST;   \
			REG_A |= REG_B;                                                 \
			CLK(7);                                             \
			if (REG_A > 0)                                  \
			{                                                               \
				write_8_NORM(DST | REG_Y, read_8_NORM(SRC | REG_X));        \
				REG_X = MAKE_UINT_8(REG_X-1);                               \
				REG_Y = MAKE_UINT_8(REG_Y-1);                               \
				REG_A--;                                \
				if ((REG_A&0xffff) != 0) \
				{\
					REG_PC -= 3; \
				}\
				else \
				{ \
					if (FLAG_M) \
					{ \
						REG_A = 0xff; \
						REG_B = 0xff00; \
					}                                                               \
					else \
					{                                                               \
						REG_A = 0xffff;                                             \
					}                                                               \
				} \
			}
#else
#define OP_MVP()                                                            \
			DST = OPER_8_IMM()<<16;                                         \
			SRC = OPER_8_IMM()<<16;                                         \
			REG_DB = DST;   \
			REG_A |= REG_B;                                                 \
			CLK(7);                                             \
			if (REG_A > 0)                                  \
			{                                                               \
				write_8_NORM(DST | REG_Y, read_8_NORM(SRC | REG_X));        \
				REG_X = MAKE_UINT_16(REG_X-1);                              \
				REG_Y = MAKE_UINT_16(REG_Y-1);                              \
				REG_A--;                                \
				if ((REG_A&0xffff) != 0) \
				{\
					REG_PC -= 3; \
				}\
				else \
				{ \
					if (FLAG_M) \
					{ \
						REG_A = 0xff; \
						REG_B = 0xff00; \
					}                                                               \
					else \
					{                                                               \
						REG_A = 0xffff;                                             \
					}                                                               \
				} \
			}
#endif

/* M37710   No Operation */
#undef OP_NOP
#define OP_NOP()                                                            \
			CLK(CLK_OP + CLK_IMPLIED)

/* M37710   Logical OR operand to accumulator */
#undef OP_ORA
#if FLAG_SET_M
#define OP_ORA(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_A |= OPER_8_ ## MODE()
#else
#define OP_ORA(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_A |= OPER_16_##MODE();                             \
			FLAG_N = NFLAG_16(REG_A)
#endif

/* M37710   Logical OR operand to B accumulator */
#undef OP_ORB
#if FLAG_SET_M
#define OP_ORB(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			FLAG_N = FLAG_Z = REG_BA |= OPER_8_ ## MODE()
#else
#define OP_ORB(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			FLAG_Z = REG_BA |= OPER_16_##MODE();                                \
			FLAG_N = NFLAG_16(REG_BA)
#endif

/* M37710  Push Effective Address */
#undef OP_PEA
#define OP_PEA()                                                            \
			CLK(CLK_OP + CLK_R16 + CLK_W16);                                \
			m37710i_push_16(OPER_16_IMM())

/* M37710  Push Effective Indirect Address */
#undef OP_PEI
#define OP_PEI()                                                            \
			CLK(CLK_OP + CLK_R16 + CLK_W16 + CLK_D);                        \
			m37710i_push_16(EA_DI())

/* M37710  Push Effective PC-Relative Address */
#undef OP_PER
#define OP_PER()                                                            \
			CLK(CLK_OP + CLK_R16 + CLK_W16 + 1);                            \
			SRC = OPER_16_IMM();                                            \
			m37710i_push_16(REG_PC + SRC)

/* M37710   Push accumulator to the stack */
#undef OP_PHA
#if FLAG_SET_M
#define OP_PHA()                                                            \
			CLK(CLK_OP + CLK_W8 + 1);                                       \
			m37710i_push_8(REG_A)
#else
#define OP_PHA()                                                            \
			CLK(CLK_OP + CLK_W16 + 1);                                      \
			m37710i_push_16(REG_A)
#endif

/* M37710   Push B accumulator to the stack */
#undef OP_PHAB
#if FLAG_SET_M
#define OP_PHAB()                                                           \
			CLK(CLK_OP + CLK_W8 + 1);                                       \
			m37710i_push_8(REG_BA)
#else
#define OP_PHAB()                                                           \
			CLK(CLK_OP + CLK_W16 + 1);                                      \
			m37710i_push_16(REG_BA)
#endif

/* M37710   Push index register to the stack */
#undef OP_PHX
#if FLAG_SET_X
#define OP_PHX(REG)                                                         \
			CLK(CLK_OP + CLK_W8 + 1);                                       \
			m37710i_push_8(REG)
#else
#define OP_PHX(REG)                                                         \
			CLK(CLK_OP + CLK_W16 + 1);                                      \
			m37710i_push_16(REG)
#endif

/* M37710  Push data bank register */
#undef OP_PHT
#define OP_PHT()                                                            \
			CLK(CLK_OP + CLK_W8 + 1);                                       \
			m37710i_push_8(REG_DB>>16)

/* M37710  Push direct register */
#undef OP_PHD
#define OP_PHD()                                                            \
			CLK(CLK_OP + CLK_W16 + 1);                                      \
			m37710i_push_16(REG_D)

/* M37710  Push program bank register */
#undef OP_PHK
#define OP_PHK()                                                            \
			CLK(CLK_OP + CLK_W8 + 1);                                       \
			m37710i_push_8(REG_PB>>16)

/* M37710   Push the Processor Status Register to the stack */
#undef OP_PHP
#define OP_PHP()                                                            \
			CLK(CLK_OP + CLK_W8 + 1);                                       \
			m37710i_push_8(m_ipl);                                    \
			m37710i_push_8(m37710i_get_reg_p())

/* M37710   Pull accumulator from the stack */
#undef OP_PLA
#if FLAG_SET_M
#define OP_PLA()                                                            \
			CLK(CLK_OP + CLK_R8 + 2);                                       \
			FLAG_N = FLAG_Z = REG_A = m37710i_pull_8()
#else
#define OP_PLA()                                                            \
			CLK(CLK_OP + CLK_R16 + 2);                                      \
			FLAG_Z = REG_A = m37710i_pull_16();                             \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710   Pull B accumulator from the stack */
#undef OP_PLAB
#if FLAG_SET_M
#define OP_PLAB()                                                           \
			CLK(CLK_OP + CLK_R8 + 2);                                       \
			FLAG_N = FLAG_Z = REG_BA = m37710i_pull_8()
#else
#define OP_PLAB()                                                           \
			CLK(CLK_OP + CLK_R16 + 2);                                      \
			FLAG_Z = REG_BA = m37710i_pull_16();                                \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710   Pull index register from the stack */
#undef OP_PLX
#if FLAG_SET_X
#define OP_PLX(REG)                                                         \
			CLK(CLK_OP + CLK_R8 + 2);                                       \
			FLAG_N = FLAG_Z = REG = m37710i_pull_8()
#else
#define OP_PLX(REG)                                                         \
			CLK(CLK_OP + CLK_R16 + 2);                                      \
			FLAG_Z = REG = m37710i_pull_16();                               \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710  Pull data bank register */
#undef OP_PLT
#define OP_PLT()                                                            \
			CLK(CLK_OP + CLK_R8 + 2);                                       \
			FLAG_N = FLAG_Z = m37710i_pull_8();                             \
			REG_DB = FLAG_Z << 16

/* M37710  Pull direct register */
#undef OP_PLD
#define OP_PLD()                                                            \
			CLK(CLK_OP + CLK_R16 + 2);                                      \
			REG_D = m37710i_pull_16()

/* M37710   Pull the Processor Status Register from the stack */
#undef OP_PLP
#define OP_PLP()                                                            \
			CLK(CLK_OP + CLK_R8 + 2);                                       \
			m37710i_set_reg_p(m37710i_pull_8());          \
			m37710i_set_reg_ipl(m37710i_pull_8());        \
			m37710i_update_irqs()

/* M37710  Reset Program status word */
#undef OP_REP
#define OP_REP()                                                            \
			CLK(CLK_OP + CLK_R8 + 1);                                       \
			m37710i_set_reg_p(m37710i_get_reg_p() & ~OPER_8_IMM());   \
			m37710i_update_irqs()

/* M37710  Clear "M" status bit */
#undef OP_CLM
#define OP_CLM()                                                            \
			CLK(CLK_OP + CLK_R8 + 1);                                       \
			m37710i_set_reg_p(m37710i_get_reg_p() & ~FLAGPOS_M)

/* M37710   Rotate Left the accumulator */
#undef OP_ROL
#if FLAG_SET_M
#define OP_ROL()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = (REG_A<<1) | CFLAG_AS_1();                             \
			FLAG_N = FLAG_Z = REG_A = MAKE_UINT_8(FLAG_C)
#else
#define OP_ROL()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = (REG_A<<1) | CFLAG_AS_1();                             \
			FLAG_Z = REG_A = MAKE_UINT_16(FLAG_C);                          \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = CFLAG_16(FLAG_C)
#endif

/* M37710   Rotate Left the B accumulator */
#undef OP_ROLB
#if FLAG_SET_M
#define OP_ROLB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = (REG_BA<<1) | CFLAG_AS_1();                                \
			FLAG_N = FLAG_Z = REG_BA = MAKE_UINT_8(FLAG_C)
#else
#define OP_ROLB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = (REG_BA<<1) | CFLAG_AS_1();                                \
			FLAG_Z = REG_BA = MAKE_UINT_16(FLAG_C);                         \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = CFLAG_16(FLAG_C)
#endif

/* M37710   Rotate Left the accumulator by a specified amount */
#undef OP_RLA
#if FLAG_SET_M
#define OP_RLA(MODE)                                                        \
	{ int cnt = OPER_8_##MODE(); while (cnt > 0) { CLK(6); REG_A=((REG_A<<1)|(REG_A>>7&1))&0xff; cnt--; } }
#else
#define OP_RLA(MODE)                                                        \
	{ int cnt = OPER_16_##MODE(); while (cnt > 0) { CLK(6); REG_A=((REG_A<<1)|(REG_A>>15&1))&0xffff; cnt--; } }
#endif

/* M37710   Rotate Left an operand */
#undef OP_ROLM
#if FLAG_SET_M
#define OP_ROLM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST = EA_##MODE();                                              \
			FLAG_C = (read_8_##MODE(DST)<<1) | CFLAG_AS_1();                \
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);                          \
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_ROLM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST = EA_##MODE();                                              \
			FLAG_C = (read_16_##MODE(DST)<<1) | CFLAG_AS_1();               \
			FLAG_Z = MAKE_UINT_16(FLAG_C);                                  \
			FLAG_N = NFLAG_16(FLAG_C);                                      \
			FLAG_C = CFLAG_16(FLAG_C);                                      \
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M37710   Rotate Right the accumulator */
#undef OP_ROR
#if FLAG_SET_M
#define OP_ROR()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_A |= FLAG_C & 0x100;                                        \
			FLAG_C = REG_A << 8;                                            \
			FLAG_N = FLAG_Z = REG_A >>= 1
#else
#define OP_ROR()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_A |= (FLAG_C<<8) & 0x10000;                                 \
			FLAG_C = REG_A << 8;                                            \
			FLAG_Z = REG_A >>= 1;                                           \
			FLAG_N = NFLAG_16(REG_A)
#endif

/* M37710   Rotate Right the B accumulator */
#undef OP_RORB
#if FLAG_SET_M
#define OP_RORB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_BA |= FLAG_C & 0x100;                                       \
			FLAG_C = REG_BA << 8;                                           \
			FLAG_N = FLAG_Z = REG_BA >>= 1
#else
#define OP_RORB()                                                           \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_BA |= (FLAG_C<<8) & 0x10000;                                    \
			FLAG_C = REG_BA << 8;                                           \
			FLAG_Z = REG_BA >>= 1;                                          \
			FLAG_N = NFLAG_16(REG_BA)
#endif

/* M37710   Rotate Right an operand */
#undef OP_RORM
#if FLAG_SET_M
#define OP_RORM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST = EA_##MODE();                                              \
			FLAG_Z = read_8_##MODE(DST) | (FLAG_C & 0x100);                 \
			FLAG_C = FLAG_Z << 8;                                           \
			FLAG_N = FLAG_Z >>= 1;                                          \
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_RORM(MODE)                                                       \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST = EA_##MODE();                                              \
			FLAG_Z = read_16_##MODE(DST) | ((FLAG_C<<8) & 0x10000);         \
			FLAG_C = FLAG_Z << 8;                                           \
			FLAG_Z >>= 1;                                                   \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M37710   Return from Interrupt */
#undef OP_RTI
#define OP_RTI()                                                            \
			CLK(8);                                                         \
			m37710i_set_reg_p(m37710i_pull_8());                  \
			m37710i_set_reg_ipl(m37710i_pull_8());                    \
			m37710i_jump_16(m37710i_pull_16());                   \
			REG_PB = m37710i_pull_8() << 16;                    \
			m37710i_update_irqs()

/* M37710  Return from Subroutine Long */
#undef OP_RTL
#define OP_RTL()                                                            \
			CLK(6);                                                         \
			m37710i_jump_24(m37710i_pull_24())

/* M37710   Return from Subroutine */
#undef OP_RTS
#define OP_RTS()                                                            \
			CLK(6);     \
			DST = m37710i_pull_16();                                                    \
			m37710i_jump_16(DST)

/* M37710   Subtract with Carry */
/* Unusual behavior: C flag is inverted */
#undef OP_SBC
#if FLAG_SET_M
#define OP_SBC(MODE)                                                        \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			SRC = OPER_8_##MODE();                                          \
			FLAG_C = ~FLAG_C;                                               \
			if(!FLAG_D)                                                     \
			{                                                               \
				FLAG_C = REG_A - SRC - CFLAG_AS_1();                        \
				FLAG_V = VFLAG_SUB_8(SRC, REG_A, FLAG_C);                   \
				FLAG_N = FLAG_Z = REG_A = MAKE_UINT_8(FLAG_C);              \
				FLAG_C = ~FLAG_C;                                           \
				BREAKOUT;                                                   \
			}                                                               \
			DST = CFLAG_AS_1();                                             \
			FLAG_C = REG_A - SRC - DST;                                     \
			FLAG_V = VFLAG_SUB_8(SRC, REG_A, FLAG_C);                       \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C-=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C-=0x60;                                               \
			FLAG_N = FLAG_Z = REG_A = MAKE_UINT_8(FLAG_C);                  \
			FLAG_C = ~FLAG_C
#else
#define OP_SBC(MODE)                                                        \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			SRC = OPER_16_##MODE();                                         \
			FLAG_C = ~FLAG_C;                                               \
			if(!FLAG_D)                                                     \
			{                                                               \
				FLAG_C = REG_A - SRC - CFLAG_AS_1();                        \
				FLAG_V = VFLAG_SUB_16(SRC, REG_A, FLAG_C);                  \
				FLAG_Z = REG_A = MAKE_UINT_16(FLAG_C);                      \
				FLAG_N = NFLAG_16(REG_A);                                   \
				FLAG_C = ~CFLAG_16(FLAG_C);                                 \
				BREAKOUT;                                                   \
			}                                                               \
			DST    = CFLAG_AS_1();                                          \
			FLAG_C = MAKE_UINT_8(REG_A) - MAKE_UINT_8(SRC) - DST;           \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C-=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C-=0x60;                                               \
			FLAG_Z = MAKE_UINT_8(FLAG_C);                                   \
			DST    = CFLAG_AS_1();                                          \
			FLAG_C = MAKE_UINT_8(REG_A>>8) - MAKE_UINT_8(SRC>>8) - DST;     \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C-=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C-=0x60;                                               \
			FLAG_Z |= MAKE_UINT_8(FLAG_C) << 8;                             \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			FLAG_V = VFLAG_SUB_16(SRC, REG_A, FLAG_Z);                      \
			REG_A  = FLAG_Z;                                                \
			FLAG_C = ~FLAG_C
#endif

/* M37710   Subtract with Carry - B accumulator */
/* Unusual behavior: C flag is inverted */
#undef OP_SBCB
#if FLAG_SET_M
#define OP_SBCB(MODE)                                                       \
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);                              \
			SRC = OPER_8_##MODE();                                          \
			FLAG_C = ~FLAG_C;                                               \
			if(!FLAG_D)                                                     \
			{                                                               \
				FLAG_C = REG_BA - SRC - CFLAG_AS_1();                       \
				FLAG_V = VFLAG_SUB_8(SRC, REG_BA, FLAG_C);                  \
				FLAG_N = FLAG_Z = REG_BA = MAKE_UINT_8(FLAG_C);             \
				FLAG_C = ~FLAG_C;                                           \
				BREAKOUT;                                                   \
			}                                                               \
			DST = CFLAG_AS_1();                                             \
			FLAG_C = REG_BA - SRC - DST;                                        \
			FLAG_V = VFLAG_SUB_8(SRC, REG_BA, FLAG_C);                      \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C-=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C-=0x60;                                               \
			FLAG_N = FLAG_Z = REG_BA = MAKE_UINT_8(FLAG_C);                 \
			FLAG_C = ~FLAG_C
#else
#define OP_SBCB(MODE)                                                       \
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);                             \
			SRC = OPER_16_##MODE();                                         \
			FLAG_C = ~FLAG_C;                                               \
			if(!FLAG_D)                                                     \
			{                                                               \
				FLAG_C = REG_BA - SRC - CFLAG_AS_1();                       \
				FLAG_V = VFLAG_SUB_16(SRC, REG_BA, FLAG_C);                 \
				FLAG_Z = REG_BA = MAKE_UINT_16(FLAG_C);                     \
				FLAG_N = NFLAG_16(REG_BA);                                  \
				FLAG_C = ~CFLAG_16(FLAG_C);                                 \
				BREAKOUT;                                                   \
			}                                                               \
			DST    = CFLAG_AS_1();                                          \
			FLAG_C = MAKE_UINT_8(REG_BA) - MAKE_UINT_8(SRC) - DST;          \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C-=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C-=0x60;                                               \
			FLAG_Z = MAKE_UINT_8(FLAG_C);                                   \
			DST    = CFLAG_AS_1();                                          \
			FLAG_C = MAKE_UINT_8(REG_A>>8) - MAKE_UINT_8(SRC>>8) - DST;     \
			if((FLAG_C & 0xf) > 9)                                          \
				FLAG_C-=6;                                                  \
			if((FLAG_C & 0xf0) > 0x90)                                      \
				FLAG_C-=0x60;                                               \
			FLAG_Z |= MAKE_UINT_8(FLAG_C) << 8;                             \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			FLAG_V = VFLAG_SUB_16(SRC, REG_BA, FLAG_Z);                     \
			REG_BA = FLAG_Z;                                                \
			FLAG_C = ~FLAG_C
#endif

/* M37710   Set Carry flag */
#undef OP_SEC
#define OP_SEC()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_C = CFLAG_SET

/* M37710   Set Interrupt Mask flag */
#undef OP_SEI
#define OP_SEI()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_I = IFLAG_SET

/* M37710  Set Program status word */
#undef OP_SEP
#define OP_SEP()                                                            \
			CLK(CLK_OP + CLK_R8 + 1);                                       \
			m37710i_set_reg_p(m37710i_get_reg_p() | OPER_8_IMM())

/* M37710  Set "M" status bit */
#undef OP_SEM
#define OP_SEM()                                                            \
			CLK(CLK_OP + CLK_R8 + 1);                                       \
			m37710i_set_reg_p(m37710i_get_reg_p() | FLAGPOS_M)

/* M37710   Store accumulator to memory */
#undef OP_STA
#if FLAG_SET_M
#define OP_STA(MODE)                                                        \
			CLK(CLK_OP + CLK_W8 + CLK_W_##MODE);                                \
			write_8_##MODE(EA_##MODE(), REG_A)
#else
#define OP_STA(MODE)                                                        \
			CLK(CLK_OP + CLK_W16 + CLK_W_##MODE);                               \
			write_16_##MODE(EA_##MODE(), REG_A)
#endif

/* M37710   Store B accumulator to memory */
#undef OP_STB
#if FLAG_SET_M
#define OP_STB(MODE)                                                        \
			CLK(CLK_OP + CLK_W8 + CLK_W_##MODE);                                \
			write_8_##MODE(EA_##MODE(), REG_BA)
#else
#define OP_STB(MODE)                                                        \
			CLK(CLK_OP + CLK_W16 + CLK_W_##MODE);                               \
			write_16_##MODE(EA_##MODE(), REG_BA)
#endif

/* M37710   Store index register to memory */
#undef OP_STX
#if FLAG_SET_X
#define OP_STX(REG, MODE)                                                   \
			CLK(CLK_OP + CLK_W8 + CLK_W_##MODE);                                \
			write_8_##MODE(EA_##MODE(), REG)
#else
#define OP_STX(REG, MODE)                                                   \
			CLK(CLK_OP + CLK_W16 + CLK_W_##MODE);                               \
			write_16_##MODE(EA_##MODE(), REG)
#endif

/* M37710  Stop the clock */
#undef OP_STP
#define OP_STP()                                                            \
			USE_ALL_CLKS();                                                 \
			CPU_STOPPED |= STOP_LEVEL_STOP

/* M37710   Transfer accumulator to index */
#undef OP_TAX
#if FLAG_SET_M
#if FLAG_SET_X
#define OP_TAX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = REG_A;                                           \
			FLAG_N = NFLAG_8(FLAG_Z)
#else /* FLAG_SET_X */
#define OP_TAX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = REG_B | REG_A;                                   \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif /* FLAG_SET_X */
#else /* FLAG_SET_M */
#if FLAG_SET_X
#define OP_TAX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = MAKE_UINT_8(REG_A);                              \
			FLAG_N = NFLAG_8(FLAG_Z)
#else /* FLAG_SET_X */
#define OP_TAX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = REG_A;                                           \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif /* FLAG_SET_X */
#endif /* FLAG_SET_M */


/* M37710   Transfer accumulator B to index */
#undef OP_TBX
#if FLAG_SET_M
#if FLAG_SET_X
#define OP_TBX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = REG_BA;                                          \
			FLAG_N = NFLAG_8(FLAG_Z)
#else /* FLAG_SET_X */
#define OP_TBX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = REG_BB | REG_BA;                                 \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif /* FLAG_SET_X */
#else /* FLAG_SET_M */
#if FLAG_SET_X
#define OP_TBX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = MAKE_UINT_8(REG_BA);                             \
			FLAG_N = NFLAG_8(FLAG_Z)
#else /* FLAG_SET_X */
#define OP_TBX(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG = REG_BA;                                          \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif /* FLAG_SET_X */
#endif /* FLAG_SET_M */

/* M37710   Transfer index to accumulator */
#undef OP_TXA
#if FLAG_SET_M
#define OP_TXA(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_A = MAKE_UINT_8(REG);                              \
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TXA(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_A = REG;                                           \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710   Transfer index to accumulator B */
#undef OP_TXB
#if FLAG_SET_M
#define OP_TXB(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_BA = MAKE_UINT_8(REG);                             \
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TXB(REG)                                                         \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_BA = REG;                                          \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710  Transfer accumulator to direct register */
#undef OP_TAD
#if FLAG_SET_M
#define OP_TAD()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_D = REG_A | REG_B
#else
#define OP_TAD()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_D = REG_A
#endif

/* M37710  Transfer accumulator B to direct register */
#undef OP_TBD
#if FLAG_SET_M
#define OP_TBD()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_D = REG_BA | REG_BB
#else
#define OP_TBD()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_D = REG_BA
#endif

/* M37710  Transfer direct register to accumulator */
#undef OP_TDA
#if FLAG_SET_M
#define OP_TDA()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_D;                                                 \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			REG_A = MAKE_UINT_8(REG_D);                                     \
			REG_B = REG_D & 0xff00
#else
#define OP_TDA()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_A = REG_D;                                         \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710  Transfer direct register to accumulator B */
#undef OP_TDB
#if FLAG_SET_M
#define OP_TDB()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_D;                                                 \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			REG_BA = MAKE_UINT_8(REG_D);                                    \
			REG_BB = REG_D & 0xff00
#else
#define OP_TDB()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_BA = REG_D;                                        \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710  Transfer accumulator to stack pointer */
#undef OP_TAS
#if FLAG_SET_M
#define OP_TAS()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_S = REG_A | REG_B
#else
#define OP_TAS()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_S = REG_A
#endif

/* M37710  Transfer accumulator B to stack pointer */
#undef OP_TBS
#if FLAG_SET_M
#define OP_TBS()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_S = REG_BA | REG_BB
#else
#define OP_TBS()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_S = REG_BA
#endif

/* M37710  Transfer stack pointer to accumulator */
#undef OP_TSA
#if FLAG_SET_M
#define OP_TSA()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_S;                                                 \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			REG_A = MAKE_UINT_8(REG_S);                                     \
			REG_B = REG_S & 0xff00
#else
#define OP_TSA()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_A = REG_S;                                         \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710  Transfer stack pointer to accumulator B */
#undef OP_TSB
#if FLAG_SET_M
#define OP_TSB()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_S;                                                 \
			FLAG_N = NFLAG_16(FLAG_Z);                                      \
			REG_BA = MAKE_UINT_8(REG_S);                                    \
			REG_BB = REG_S & 0xff00
#else
#define OP_TSB()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_BA = REG_S;                                        \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710   Transfer stack pointer to X */
#undef OP_TSX
#if FLAG_SET_X
#define OP_TSX()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_X = MAKE_UINT_8(REG_S);                            \
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TSX()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_X = REG_S;                                         \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710   Transfer X to stack pointer */
#undef OP_TXS
#if FLAG_SET_X
#define OP_TXS()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_S = MAKE_UINT_8(REG_X)
#else
#define OP_TXS()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			REG_S = REG_X
#endif

/* M37710  Transfer X to Y */
#undef OP_TXY
#if FLAG_SET_X
#define OP_TXY()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_Y = REG_X;                                         \
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TXY()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_Y = REG_X;                                         \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710  Transfer Y to X */
#undef OP_TYX
#if FLAG_SET_X
#define OP_TYX()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_X = REG_Y;                                         \
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TYX()                                                            \
			CLK(CLK_OP + CLK_IMPLIED);                                      \
			FLAG_Z = REG_X = REG_Y;                                         \
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M37710  clear bit */
#undef OP_CLB
#if FLAG_SET_M
#define OP_CLB(MODE)                                                        \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST    = EA_##MODE();                                           \
			REG_IM = read_8_##MODE(DST);                                    \
			REG_IM2 = read_8_IMM(REG_PB | REG_PC); \
			REG_PC++;           \
			write_8_##MODE(DST, REG_IM & ~REG_IM2);
#else
#define OP_CLB(MODE)                                                        \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST    = EA_##MODE();                                           \
			REG_IM = read_16_##MODE(DST);                                   \
			REG_IM2 = read_16_IMM(REG_PB | REG_PC);    \
			REG_PC+=2;          \
			write_16_##MODE(DST, REG_IM & ~REG_IM2);
#endif

/* M37710  set bit */
#undef OP_SEB
#if FLAG_SET_M
#define OP_SEB(MODE)                                                        \
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);                          \
			DST    = EA_##MODE();                                           \
			REG_IM = read_8_##MODE(DST);                                    \
			REG_IM2 = read_8_IMM(REG_PB | REG_PC); \
			REG_PC++;           \
			write_8_##MODE(DST, REG_IM | REG_IM2);
#else
#define OP_SEB(MODE)                                                        \
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);                         \
			DST    = EA_##MODE();                                           \
			REG_IM = read_16_##MODE(DST);                                   \
			REG_IM2 = read_16_IMM(REG_PB | REG_PC);    \
			REG_PC+=2;          \
			write_16_##MODE(DST, REG_IM | REG_IM2);
#endif

/* M37710  Wait for interrupt */
#undef OP_WAI
#define OP_WAI()                                                            \
			USE_ALL_CLKS();                                                 \
			CPU_STOPPED |= STOP_LEVEL_WAI

/* M37710 load data bank register */
#undef OP_LDT
#define OP_LDT(MODE)    \
	CLK(CLK_OP + CLK_R8 + CLK_##MODE);  \
	REG_DB = OPER_8_##MODE()<<16;


/* M37710  prefix for B accumulator (0x42) */
/* There is a 2 cycle penalty for all instructions using this prefix */
#undef OP_PFB
#define OP_PFB()                                                            \
			CLK(2);     \
			REG_IR = read_8_IMM(REG_PB | REG_PC);   \
			REG_PC++;   \
			(this->*m_opcodes42[REG_IR])();


/* M37710  prefix for multiply / divide instructions (0x89) */
#undef OP_PFXM
#define OP_PFXM()                                                           \
			REG_IR = read_8_IMM(REG_PB | REG_PC);   \
			REG_PC++;   \
			(this->*m_opcodes89[REG_IR])();


/* M37710 unimplemented opcode */
#undef OP_UNIMP
#define OP_UNIMP()                                                          \
	logerror("error M37710: UNIMPLEMENTED OPCODE!  K=%x PC=%x\n", REG_PB, REG_PPC);

/* ======================================================================== */
/* ======================== OPCODE & FUNCTION TABLES ====================== */
/* ======================================================================== */

#undef OP
#undef O
#undef TABLE_OPCODES
#undef TABLE_FUNCTION

#if !FLAG_SET_M && !FLAG_SET_X
#define OP(CODE, OPERATION) void m37710_cpu_device::m37710i_ ## CODE ## _M0X0() {OPERATION;}
#define O(CODE) &m37710_cpu_device::m37710i_ ## CODE ## _M0X0
#define TABLE_OPCODES const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes_M0X0[256]
#define TABLE_OPCODES2 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes42_M0X0[256]
#define TABLE_OPCODES3 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes89_M0X0[256]
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)   RTYPE m37710_cpu_device::m37710i_ ## NAME ## _M0X0 ARGS

#elif !FLAG_SET_M && FLAG_SET_X

#define OP(CODE, OPERATION) void m37710_cpu_device::m37710i_ ## CODE ## _M0X1() {OPERATION;}
#define O(CODE) &m37710_cpu_device::m37710i_ ## CODE ## _M0X1
#define TABLE_OPCODES const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes_M0X1[256]
#define TABLE_OPCODES2 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes42_M0X1[256]
#define TABLE_OPCODES3 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes89_M0X1[256]
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)   RTYPE m37710_cpu_device::m37710i_ ## NAME ## _M0X1 ARGS

#elif FLAG_SET_M && !FLAG_SET_X

#define OP(CODE, OPERATION) void m37710_cpu_device::m37710i_ ## CODE ## _M1X0() {OPERATION;}
#define O(CODE) &m37710_cpu_device::m37710i_ ## CODE ## _M1X0
#define TABLE_OPCODES const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes_M1X0[256]
#define TABLE_OPCODES2 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes42_M1X0[256]
#define TABLE_OPCODES3 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes89_M1X0[256]
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)   RTYPE m37710_cpu_device::m37710i_ ## NAME ## _M1X0 ARGS

#elif FLAG_SET_M && FLAG_SET_X

#define OP(CODE, OPERATION) void m37710_cpu_device::m37710i_ ## CODE ## _M1X1() {OPERATION;}
#define O(CODE) &m37710_cpu_device::m37710i_ ## CODE ## _M1X1
#define TABLE_OPCODES const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes_M1X1[256]
#define TABLE_OPCODES2 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes42_M1X1[256]
#define TABLE_OPCODES3 const m37710_cpu_device::opcode_func m37710_cpu_device::m37710i_opcodes89_M1X1[256]
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)   RTYPE m37710_cpu_device::m37710i_ ## NAME ## _M1X1 ARGS

#endif

#define BREAKOUT return

/* OP  FUNCTION                     Comment     */
OP(00, OP_BRK  (             ) ) /* BRK         */
OP(01, OP_ORA  ( DXI         ) ) /* ORA dxi     */
OP(02, OP_NOP  (             ) ) /* unused      */
OP(03, OP_ORA  ( S           ) ) /* ORA s   (G) */
OP(04, OP_SEB  ( D           ) ) /* SEB d   (C) */
OP(05, OP_ORA  ( D           ) ) /* ORA d       */
OP(06, OP_ASLM ( D           ) ) /* ASL d       */
OP(07, OP_ORA  ( DLI         ) ) /* ORA dli (G) */
OP(08, OP_PHP  (             ) ) /* PHP         */
OP(09, OP_ORA  ( IMM         ) ) /* ORA imm     */
OP(0a, OP_ASL  (             ) ) /* ASL acc     */
OP(0b, OP_PHD  (             ) ) /* PHD     (G) */
OP(0c, OP_SEB  ( A           ) ) /* SEB a   (C) */
OP(0d, OP_ORA  ( A           ) ) /* ORA a       */
OP(0e, OP_ASLM ( A           ) ) /* ASL a       */
OP(0f, OP_ORA  ( AL          ) ) /* ORA al  (G) */
OP(10, OP_BCC  ( COND_PL()   ) ) /* BPL         */
OP(11, OP_ORA  ( DIY         ) ) /* ORA diy     */
OP(12, OP_ORA  ( DI          ) ) /* ORA di  (C) */
OP(13, OP_ORA  ( SIY         ) ) /* ORA siy (G) */
OP(14, OP_CLB  ( D           ) ) /* CLB d   (C) */
OP(15, OP_ORA  ( DX          ) ) /* ORA dx      */
OP(16, OP_ASLM ( DX          ) ) /* ASL dx      */
OP(17, OP_ORA  ( DLIY        ) ) /* ORA dliy(C) */
OP(18, OP_CLC  (             ) ) /* CLC         */
OP(19, OP_ORA  ( AY          ) ) /* ORA ay      */
OP(1a, OP_DEC  (             ) ) /* DEA     (C) */
OP(1b, OP_TAS  (             ) ) /* TAS     (G) */
OP(1c, OP_CLB  ( A           ) ) /* CLB a   (C) */
OP(1d, OP_ORA  ( AX          ) ) /* ORA ax      */
OP(1e, OP_ASLM ( AX          ) ) /* ASL ax      */
OP(1f, OP_ORA  ( ALX         ) ) /* ORA alx (G) */
OP(20, OP_JSR  ( A           ) ) /* JSR a       */
OP(21, OP_AND  ( DXI         ) ) /* AND dxi     */
OP(22, OP_JSL  ( AL          ) ) /* JSL al  (G) */
OP(23, OP_AND  ( S           ) ) /* AND s   (G) */
OP(24, OP_BBS  ( D           ) ) /* BBS d       */
OP(25, OP_AND  ( D           ) ) /* AND d       */
OP(26, OP_ROLM ( D           ) ) /* ROL d       */
OP(27, OP_AND  ( DLI         ) ) /* AND dli (G) */
OP(28, OP_PLP  (             ) ) /* PLP         */
OP(29, OP_AND  ( IMM         ) ) /* AND imm     */
OP(2a, OP_ROL  (             ) ) /* ROL acc     */
OP(2b, OP_PLD  (             ) ) /* PLD     (G) */
OP(2c, OP_BBS  ( A           ) ) /* BBS a       */
OP(2d, OP_AND  ( A           ) ) /* AND a       */
OP(2e, OP_ROLM ( A           ) ) /* ROL a       */
OP(2f, OP_AND  ( AL          ) ) /* AND al  (G) */
OP(30, OP_BCC  ( COND_MI()   ) ) /* BMI         */
OP(31, OP_AND  ( DIY         ) ) /* AND diy     */
OP(32, OP_AND  ( DI          ) ) /* AND di  (C) */
OP(33, OP_AND  ( SIY         ) ) /* AND siy     */
OP(34, OP_BBC  ( D           ) ) /* BBC d       */
OP(35, OP_AND  ( DX          ) ) /* AND dx      */
OP(36, OP_ROLM ( DX          ) ) /* ROL dx      */
OP(37, OP_AND  ( DLIY        ) ) /* AND dliy(G) */
OP(38, OP_SEC  (             ) ) /* SEC         */
OP(39, OP_AND  ( AY          ) ) /* AND ay      */
OP(3a, OP_INC  (             ) ) /* INA     (C) */
OP(3b, OP_TSA  (             ) ) /* TSA     (G) */
OP(3c, OP_BBC  ( A           ) ) /* BBC a       */
OP(3d, OP_AND  ( AX          ) ) /* AND ax      */
OP(3e, OP_ROLM ( AX          ) ) /* ROL ax      */
OP(3f, OP_AND  ( ALX         ) ) /* AND alx (G) */
OP(40, OP_RTI  (             ) ) /* RTI         */
OP(41, OP_EOR  ( DXI         ) ) /* EOR dxi     */
OP(42, OP_PFB  (             ) ) /* prefix for "B" accumulator */
OP(43, OP_EOR  ( S           ) ) /* EOR s   (G) */
OP(44, OP_MVP  (             ) ) /* MVP     (G) */
OP(45, OP_EOR  ( D           ) ) /* EOR d       */
OP(46, OP_LSRM ( D           ) ) /* LSR d       */
OP(47, OP_EOR  ( DLI         ) ) /* EOR dli (G) */
OP(48, OP_PHA  (             ) ) /* PHA         */
OP(49, OP_EOR  ( IMM         ) ) /* EOR imm     */
OP(4a, OP_LSR  (             ) ) /* LSR acc     */
OP(4b, OP_PHK  (             ) ) /* PHK     (G) */
OP(4c, OP_JMP  ( A           ) ) /* JMP a       */
OP(4d, OP_EOR  ( A           ) ) /* EOR a       */
OP(4e, OP_LSRM ( A           ) ) /* LSR a       */
OP(4f, OP_EOR  ( AL          ) ) /* EOR al  (G) */
OP(50, OP_BCC  ( COND_VC()   ) ) /* BVC         */
OP(51, OP_EOR  ( DIY         ) ) /* EOR diy     */
OP(52, OP_EOR  ( DI          ) ) /* EOR di  (C) */
OP(53, OP_EOR  ( SIY         ) ) /* EOR siy (G) */
OP(54, OP_MVN  (             ) ) /* MVN     (G) */
OP(55, OP_EOR  ( DX          ) ) /* EOR dx      */
OP(56, OP_LSRM ( DX          ) ) /* LSR dx      */
OP(57, OP_EOR  ( DLIY        ) ) /* EOR dliy(G) */
OP(58, OP_CLI  (             ) ) /* CLI         */
OP(59, OP_EOR  ( AY          ) ) /* EOR ay      */
OP(5a, OP_PHX  ( REG_Y       ) ) /* PHY     (C) */
OP(5b, OP_TAD  (             ) ) /* TAD     (G) */
OP(5c, OP_JMPAL(             ) ) /* JMP al  (G) */
OP(5d, OP_EOR  ( AX          ) ) /* EOR ax      */
OP(5e, OP_LSRM ( AX          ) ) /* LSR ax      */
OP(5f, OP_EOR  ( ALX         ) ) /* EOR alx (G) */
OP(60, OP_RTS  (             ) ) /* RTS         */
OP(61, OP_ADC  ( DXI         ) ) /* ADC dxi     */
OP(62, OP_PER  (             ) ) /* PER     (G) */
OP(63, OP_ADC  ( S           ) ) /* ADC s   (G) */
OP(64, OP_LDM  ( D           ) ) /* LDM d   (C) */
OP(65, OP_ADC  ( D           ) ) /* ADC d       */
OP(66, OP_RORM ( D           ) ) /* ROR d       */
OP(67, OP_ADC  ( DLI         ) ) /* ADC dli (G) */
OP(68, OP_PLA  (             ) ) /* PLA         */
OP(69, OP_ADC  ( IMM         ) ) /* ADC imm     */
OP(6a, OP_ROR  (             ) ) /* ROR acc     */
OP(6b, OP_RTL  (             ) ) /* RTL     (G) */
OP(6c, OP_JMP  ( AI          ) ) /* JMP ai      */
OP(6d, OP_ADC  ( A           ) ) /* ADC a       */
OP(6e, OP_RORM ( A           ) ) /* ROR a       */
OP(6f, OP_ADC  ( AL          ) ) /* ADC al  (G) */
OP(70, OP_BCC  ( COND_VS()   ) ) /* BVS         */
OP(71, OP_ADC  ( DIY         ) ) /* ADC diy     */
OP(72, OP_ADC  ( DI          ) ) /* ADC di  (G) */
OP(73, OP_ADC  ( SIY         ) ) /* ADC siy (G) */
OP(74, OP_LDM  ( DX          ) ) /* LDM dx  (C) */
OP(75, OP_ADC  ( DX          ) ) /* ADC dx      */
OP(76, OP_RORM ( DX          ) ) /* ROR dx      */
OP(77, OP_ADC  ( DLIY        ) ) /* ADC dliy(G) */
OP(78, OP_SEI  (             ) ) /* SEI         */
OP(79, OP_ADC  ( AY          ) ) /* ADC ay      */
OP(7a, OP_PLX  ( REG_Y       ) ) /* PLY     (C) */
OP(7b, OP_TDA  (             ) ) /* TDA     (G) */
OP(7c, OP_JMPAXI(            ) ) /* JMP axi (C) */
OP(7d, OP_ADC  ( AX          ) ) /* ADC ax      */
OP(7e, OP_RORM ( AX          ) ) /* ROR ax      */
OP(7f, OP_ADC  ( ALX         ) ) /* ADC alx (G) */
OP(80, OP_BRA  (             ) ) /* BRA     (C) */
OP(81, OP_STA  ( DXI         ) ) /* STA dxi     */
OP(82, OP_BRL  (             ) ) /* BRL     (G) */
OP(83, OP_STA  ( S           ) ) /* STA s   (G) */
OP(84, OP_STX  ( REG_Y, D    ) ) /* STY d       */
OP(85, OP_STA  ( D           ) ) /* STA d       */
OP(86, OP_STX  ( REG_X, D    ) ) /* STX d       */
OP(87, OP_STA  ( DLI         ) ) /* STA dli (G) */
OP(88, OP_DECX ( REG_Y       ) ) /* DEY         */
OP(89, OP_PFXM (             ) ) /* prefix for mul/div insns */
OP(8a, OP_TXA  ( REG_X       ) ) /* TXA         */
OP(8b, OP_PHT  (             ) ) /* PHT     (G) */
OP(8c, OP_STX  ( REG_Y, A    ) ) /* STY a       */
OP(8d, OP_STA  ( A           ) ) /* STA a       */
OP(8e, OP_STX  ( REG_X, A    ) ) /* STX a       */
OP(8f, OP_STA  ( AL          ) ) /* STA al  (G) */
OP(90, OP_BCC  ( COND_CC()   ) ) /* BCC         */
OP(91, OP_STA  ( DIY         ) ) /* STA diy     */
OP(92, OP_STA  ( DI          ) ) /* STA di  (C) */
OP(93, OP_STA  ( SIY         ) ) /* STA siy (G) */
OP(94, OP_STX  ( REG_Y, DX   ) ) /* STY dx      */
OP(95, OP_STA  ( DX          ) ) /* STA dx      */
OP(96, OP_STX  ( REG_X, DY   ) ) /* STX dy      */
OP(97, OP_STA  ( DLIY        ) ) /* STA dliy(G) */
OP(98, OP_TXA  ( REG_Y       ) ) /* TYA         */
OP(99, OP_STA  ( AY          ) ) /* STA ay      */
OP(9a, OP_TXS  (             ) ) /* TXS         */
OP(9b, OP_TXY  (             ) ) /* TXY     (G) */
OP(9c, OP_LDM  ( A           ) ) /* LDM a   (C) */
OP(9d, OP_STA  ( AX          ) ) /* STA ax      */
OP(9e, OP_LDM  ( AX          ) ) /* LDM ax  (C) */
OP(9f, OP_STA  ( ALX         ) ) /* STA alx (G) */
OP(a0, OP_LDX  ( REG_Y, IMM  ) ) /* LDY imm     */
OP(a1, OP_LDA  ( DXI         ) ) /* LDA dxi     */
OP(a2, OP_LDX  ( REG_X, IMM  ) ) /* LDX imm     */
OP(a3, OP_LDA  ( S           ) ) /* LDA s   (G) */
OP(a4, OP_LDX  ( REG_Y, D    ) ) /* LDY d       */
OP(a5, OP_LDA  ( D           ) ) /* LDA d       */
OP(a6, OP_LDX  ( REG_X, D    ) ) /* LDX d       */
OP(a7, OP_LDA  ( DLI         ) ) /* LDA dli (G) */
OP(a8, OP_TAX  ( REG_Y       ) ) /* TAY         */
OP(a9, OP_LDA  ( IMM         ) ) /* LDA imm     */
OP(aa, OP_TAX  ( REG_X       ) ) /* TAX         */
OP(ab, OP_PLT  (             ) ) /* PLT     (G) */
OP(ac, OP_LDX  ( REG_Y, A    ) ) /* LDY a       */
OP(ad, OP_LDA  ( A           ) ) /* LDA a       */
OP(ae, OP_LDX  ( REG_X, A    ) ) /* LDX a       */
OP(af, OP_LDA  ( AL          ) ) /* LDA al  (G) */
OP(b0, OP_BCC  ( COND_CS()   ) ) /* BCS         */
OP(b1, OP_LDA  ( DIY         ) ) /* LDA diy     */
OP(b2, OP_LDA  ( DI          ) ) /* LDA di  (C) */
OP(b3, OP_LDA  ( SIY         ) ) /* LDA siy (G) */
OP(b4, OP_LDX  ( REG_Y, DX   ) ) /* LDY dx      */
OP(b5, OP_LDA  ( DX          ) ) /* LDA dx      */
OP(b6, OP_LDX  ( REG_X, DY   ) ) /* LDX dy      */
OP(b7, OP_LDA  ( DLIY        ) ) /* LDA dliy(G) */
OP(b8, OP_CLV  (             ) ) /* CLV         */
OP(b9, OP_LDA  ( AY          ) ) /* LDA ay      */
OP(ba, OP_TSX  (             ) ) /* TSX         */
OP(bb, OP_TYX  (             ) ) /* TYX     (G) */
OP(bc, OP_LDX  ( REG_Y, AX   ) ) /* LDY ax      */
OP(bd, OP_LDA  ( AX          ) ) /* LDA ax      */
OP(be, OP_LDX  ( REG_X, AY   ) ) /* LDX ay      */
OP(bf, OP_LDA  ( ALX         ) ) /* LDA alx (G) */
OP(c0, OP_CMPX ( REG_Y, IMM  ) ) /* CPY imm     */
OP(c1, OP_CMP  ( DXI         ) ) /* CMP dxi     */
OP(c2, OP_REP  (             ) ) /* REP     (G) */
OP(c3, OP_CMP  ( S           ) ) /* CMP s   (G) */
OP(c4, OP_CMPX ( REG_Y, D    ) ) /* CPY d       */
OP(c5, OP_CMP  ( D           ) ) /* CMP d       */
OP(c6, OP_DECM ( D           ) ) /* DEC d       */
OP(c7, OP_CMP  ( DLI         ) ) /* CMP dli (G) */
OP(c8, OP_INCX ( REG_Y       ) ) /* INY         */
OP(c9, OP_CMP  ( IMM         ) ) /* CMP imm     */
OP(ca, OP_DECX ( REG_X       ) ) /* DEX         */
OP(cb, OP_WAI  (             ) ) /* WAI     (G) */
OP(cc, OP_CMPX ( REG_Y, A    ) ) /* CPY a       */
OP(cd, OP_CMP  ( A           ) ) /* CMP a       */
OP(ce, OP_DECM ( A           ) ) /* DEC a       */
OP(cf, OP_CMP  ( AL          ) ) /* CMP al  (G) */
OP(d0, OP_BCC  ( COND_NE()   ) ) /* BNE         */
OP(d1, OP_CMP  ( DIY         ) ) /* CMP diy     */
OP(d2, OP_CMP  ( DI          ) ) /* CMP di  (C) */
OP(d3, OP_CMP  ( SIY         ) ) /* CMP siy (G) */
OP(d4, OP_PEI  (             ) ) /* PEI     (G) */
OP(d5, OP_CMP  ( DX          ) ) /* CMP dx      */
OP(d6, OP_DECM ( DX          ) ) /* DEC dx      */
OP(d7, OP_CMP  ( DLIY        ) ) /* CMP dliy(G) */
OP(d8, OP_CLM  (             ) ) /* CLM         */
OP(d9, OP_CMP  ( AY          ) ) /* CMP ay      */
OP(da, OP_PHX  ( REG_X       ) ) /* PHX     (C) */
OP(db, OP_STP  (             ) ) /* STP     (G) */
OP(dc, OP_JMLAI(             ) ) /* JML ai  (G) */
OP(dd, OP_CMP  ( AX          ) ) /* CMP ax      */
OP(de, OP_DECM ( AX          ) ) /* DEC ax      */
OP(df, OP_CMP  ( ALX         ) ) /* CMP alx (G) */
OP(e0, OP_CMPX ( REG_X, IMM  ) ) /* CPX imm     */
OP(e1, OP_SBC  ( DXI         ) ) /* SBC dxi     */
OP(e2, OP_SEP  (             ) ) /* SEP imm (G) */
OP(e3, OP_SBC  ( S           ) ) /* SBC s   (G) */
OP(e4, OP_CMPX ( REG_X, D    ) ) /* CPX d       */
OP(e5, OP_SBC  ( D           ) ) /* SBC d       */
OP(e6, OP_INCM ( D           ) ) /* INC d       */
OP(e7, OP_SBC  ( DLI         ) ) /* SBC dli (G) */
OP(e8, OP_INCX ( REG_X       ) ) /* INX         */
OP(e9, OP_SBC  ( IMM         ) ) /* SBC imm     */
OP(ea, OP_NOP  (             ) ) /* NOP         */
OP(eb, OP_PSH  ( IMM         ) ) /* PSH imm     */
OP(ec, OP_CMPX ( REG_X, A    ) ) /* CPX a       */
OP(ed, OP_SBC  ( A           ) ) /* SBC a       */
OP(ee, OP_INCM ( A           ) ) /* INC a       */
OP(ef, OP_SBC  ( AL          ) ) /* SBC al  (G) */
OP(f0, OP_BCC  ( COND_EQ()   ) ) /* BEQ         */
OP(f1, OP_SBC  ( DIY         ) ) /* SBC diy     */
OP(f2, OP_SBC  ( DI          ) ) /* SBC di  (C) */
OP(f3, OP_SBC  ( SIY         ) ) /* SBC siy (G) */
OP(f4, OP_PEA  (             ) ) /* PEA     (G) */
OP(f5, OP_SBC  ( DX          ) ) /* SBC dx      */
OP(f6, OP_INCM ( DX          ) ) /* INC dx      */
OP(f7, OP_SBC  ( DLIY        ) ) /* SBC dliy(G) */
OP(f8, OP_SEM  (             ) ) /* SEM         */
OP(f9, OP_SBC  ( AY          ) ) /* SBC ay      */
OP(fa, OP_PLX  ( REG_X       ) ) /* PLX     (C) */
OP(fb, OP_PUL  ( IMM         ) ) /* PUL imm     */
OP(fc, OP_JSRAXI(            ) ) /* JSR axi (G) */
OP(fd, OP_SBC  ( AX          ) ) /* SBC ax      */
OP(fe, OP_INCM ( AX          ) ) /* INC ax      */
OP(ff, OP_SBC  ( ALX         ) ) /* SBC alx (G) */

/* B accumulator */
OP(101,OP_ORB  ( DXI         ) ) /* ORB dxi     */
OP(103,OP_ORB  ( S           ) ) /* ORB s       */
OP(105,OP_ORB  ( D           ) ) /* ORB d       */
OP(107,OP_ORB  ( DLI         ) ) /* ORB dli     */
OP(109,OP_ORB  ( IMM         ) ) /* ORB imm     */
OP(10a,OP_BSL  (             ) ) /* BSL acc     */
OP(10d,OP_ORB  ( A           ) ) /* ORB a       */
OP(10f,OP_ORB  ( AL          ) ) /* ORB al      */
OP(111,OP_ORB  ( DIY         ) ) /* ORB diy     */
OP(112,OP_ORB  ( DI          ) ) /* ORB di      */
OP(113,OP_ORB  ( SIY         ) ) /* ORB siy     */
OP(115,OP_ORB  ( DX          ) ) /* ORB dx      */
OP(117,OP_ORB  ( DLIY        ) ) /* ORB dliy    */
OP(119,OP_ORB  ( AY          ) ) /* ORB ay      */
OP(11a,OP_DECB (             ) ) /* DEB         */
OP(11b,OP_TBS  (             ) ) /* TBS         */
OP(11d,OP_ORB  ( AX          ) ) /* ORB ax      */
OP(11f,OP_ORB  ( ALX         ) ) /* ORB alx     */
OP(121,OP_ANDB ( DXI         ) ) /* ANDB dxi    */
OP(123,OP_ANDB ( S           ) ) /* ANDB s      */
OP(125,OP_ANDB ( D           ) ) /* ANDB d      */
OP(127,OP_ANDB ( DLI         ) ) /* ANDB dli    */
OP(129,OP_ANDB ( IMM         ) ) /* ANDB imm    */
OP(12a,OP_ROLB (             ) ) /* ROL Bacc    */
OP(12d,OP_ANDB ( A           ) ) /* ANDB a      */
OP(12f,OP_ANDB ( AL          ) ) /* ANDB al     */
OP(131,OP_ANDB ( DIY         ) ) /* ANDB diy    */
OP(132,OP_ANDB ( DI          ) ) /* ANDB di     */
OP(133,OP_ANDB ( SIY         ) ) /* ANDB siy    */
OP(135,OP_ANDB ( DX          ) ) /* ANDB dx     */
OP(137,OP_ANDB ( DLIY        ) ) /* ANDB dliy   */
OP(139,OP_ANDB ( AY          ) ) /* ANDB ay     */
OP(13a,OP_INCB (             ) ) /* INB         */
OP(13b,OP_TSB  (             ) ) /* TSB         */
OP(13d,OP_ANDB ( AX          ) ) /* ANDB ax     */
OP(13f,OP_ANDB ( ALX         ) ) /* ANDB alx    */
OP(141,OP_EORB ( DXI         ) ) /* EORB dxi    */
OP(143,OP_EORB ( S           ) ) /* EORB s      */
OP(145,OP_EORB ( D           ) ) /* EORB d      */
OP(147,OP_EORB ( DLI         ) ) /* EORB dli    */
OP(148,OP_PHAB (             ) ) /* PHB         */
OP(149,OP_EORB ( IMM         ) ) /* EORB imm    */
OP(14a,OP_LSRB (             ) ) /* LSRB acc    */
OP(14d,OP_EORB ( A           ) ) /* EORB a      */
OP(14f,OP_EORB ( AL          ) ) /* EORB al     */
OP(151,OP_EORB ( DIY         ) ) /* EORB diy    */
OP(152,OP_EORB ( DI          ) ) /* EORB di     */
OP(153,OP_EORB ( SIY         ) ) /* EORB siy    */
OP(155,OP_EORB ( DX          ) ) /* EORB dx     */
OP(157,OP_EORB ( DLIY        ) ) /* EORB dliy   */
OP(159,OP_EORB ( AY          ) ) /* EORB ay     */
OP(15b,OP_TBD  (             ) ) /* TBD         */
OP(15d,OP_EORB ( AX          ) ) /* EORB ax     */
OP(15f,OP_EORB ( ALX         ) ) /* EORB alx    */
OP(161,OP_ADCB ( DXI         ) ) /* ADCB dxi    */
OP(163,OP_ADCB ( S           ) ) /* ADCB s      */
OP(165,OP_ADCB ( D           ) ) /* ADCB d      */
OP(167,OP_ADCB ( DLI         ) ) /* ADCB dli    */
OP(168,OP_PLAB (             ) ) /* PLB         */
OP(169,OP_ADCB ( IMM         ) ) /* ADCB imm    */
OP(16a,OP_RORB (             ) ) /* ROR Bacc    */
OP(16d,OP_ADCB ( A           ) ) /* ADCB a      */
OP(16f,OP_ADCB ( AL          ) ) /* ADCB al     */
OP(171,OP_ADCB ( DIY         ) ) /* ADCB diy    */
OP(172,OP_ADCB ( DI          ) ) /* ADCB di     */
OP(173,OP_ADCB ( SIY         ) ) /* ADCB siy    */
OP(175,OP_ADCB ( DX          ) ) /* ADCB dx     */
OP(177,OP_ADCB ( DLIY        ) ) /* ADCB dliy   */
OP(179,OP_ADCB ( AY          ) ) /* ADCB ay     */
OP(17b,OP_TDB  (             ) ) /* TDB         */
OP(17d,OP_ADCB ( AX          ) ) /* ADCB ax     */
OP(17f,OP_ADCB ( ALX         ) ) /* ADCB alx    */
OP(181,OP_STB  ( DXI         ) ) /* STB dxi     */
OP(183,OP_STB  ( S           ) ) /* STB s       */
OP(185,OP_STB  ( D           ) ) /* STB d       */
OP(187,OP_STB  ( DLI         ) ) /* STB dli     */
OP(18a,OP_TXB  ( REG_X       ) ) /* TXB         */
OP(18d,OP_STB  ( A           ) ) /* STB a       */
OP(18f,OP_STB  ( AL          ) ) /* STB al      */
OP(191,OP_STB  ( DIY         ) ) /* STB diy     */
OP(192,OP_STB  ( DI          ) ) /* STB di      */
OP(193,OP_STB  ( SIY         ) ) /* STB siy     */
OP(195,OP_STB  ( DX          ) ) /* STB dx      */
OP(197,OP_STB  ( DLIY        ) ) /* STB dliy    */
OP(198,OP_TXB  ( REG_Y       ) ) /* TYB         */
OP(199,OP_STB  ( AY          ) ) /* STB ay      */
OP(19d,OP_STB  ( AX          ) ) /* STB ax      */
OP(19f,OP_STB  ( ALX         ) ) /* STB alx     */
OP(1a1,OP_LDB  ( DXI         ) ) /* LDB dxi     */
OP(1a3,OP_LDB  ( S           ) ) /* LDB s       */
OP(1a5,OP_LDB  ( D           ) ) /* LDB d       */
OP(1a7,OP_LDB  ( DLI         ) ) /* LDB dli     */
OP(1a8,OP_TBX  ( REG_Y       ) ) /* TBY         */
OP(1a9,OP_LDB  ( IMM         ) ) /* LDB imm     */
OP(1aa,OP_TBX  ( REG_X       ) ) /* TBX         */
OP(1ad,OP_LDB  ( A           ) ) /* LDB a       */
OP(1af,OP_LDB  ( AL          ) ) /* LDB al      */
OP(1b1,OP_LDB  ( DIY         ) ) /* LDB diy     */
OP(1b2,OP_LDB  ( DI          ) ) /* LDB di      */
OP(1b3,OP_LDB  ( SIY         ) ) /* LDB siy     */
OP(1b5,OP_LDB  ( DX          ) ) /* LDB dx      */
OP(1b7,OP_LDB  ( DLIY        ) ) /* LDB dliy    */
OP(1b9,OP_LDB  ( AY          ) ) /* LDB ay      */
OP(1bd,OP_LDB  ( AX          ) ) /* LDB ax      */
OP(1bf,OP_LDB  ( ALX         ) ) /* LDB alx     */
OP(1c1,OP_CMPB ( DXI         ) ) /* CMPB dxi    */
OP(1c3,OP_CMPB ( S           ) ) /* CMPB s      */
OP(1c5,OP_CMPB ( D           ) ) /* CMPB d      */
OP(1c7,OP_CMPB ( DLI         ) ) /* CMPB dli    */
OP(1c9,OP_CMPB ( IMM         ) ) /* CMPB imm    */
OP(1cd,OP_CMPB ( A           ) ) /* CMPB a      */
OP(1cf,OP_CMPB ( AL          ) ) /* CMPB al     */
OP(1d1,OP_CMPB ( DIY         ) ) /* CMPB diy    */
OP(1d2,OP_CMPB ( DI          ) ) /* CMPB di     */
OP(1d3,OP_CMPB ( SIY         ) ) /* CMPB siy    */
OP(1d5,OP_CMPB ( DX          ) ) /* CMPB dx     */
OP(1d7,OP_CMPB ( DLIY        ) ) /* CMPB dliy   */
OP(1d9,OP_CMPB ( AY          ) ) /* CMPB ay     */
OP(1dd,OP_CMPB ( AX          ) ) /* CMPB ax     */
OP(1df,OP_CMPB ( ALX         ) ) /* CMPB alx    */
OP(1e1,OP_SBCB ( DXI         ) ) /* SBCB dxi    */
OP(1e3,OP_SBCB ( S           ) ) /* SBCB s      */
OP(1e5,OP_SBCB ( D           ) ) /* SBCB d      */
OP(1e7,OP_SBCB ( DLI         ) ) /* SBCB dli    */
OP(1e9,OP_SBCB ( IMM         ) ) /* SBCB imm    */
OP(1ed,OP_SBCB ( A           ) ) /* SBCB a      */
OP(1ef,OP_SBCB ( AL          ) ) /* SBCB al     */
OP(1f1,OP_SBCB ( DIY         ) ) /* SBCB diy    */
OP(1f2,OP_SBCB ( DI          ) ) /* SBCB di     */
OP(1f3,OP_SBCB ( SIY         ) ) /* SBCB siy    */
OP(1f5,OP_SBCB ( DX          ) ) /* SBCB dx     */
OP(1f7,OP_SBCB ( DLIY        ) ) /* SBCB dliy   */
OP(1f9,OP_SBCB ( AY          ) ) /* SBCB ay     */
OP(1fd,OP_SBCB ( AX          ) ) /* SBCB ax     */
OP(1ff,OP_SBCB ( ALX         ) ) /* SBCB alx    */

OP(200,OP_UNIMP(             ) ) /* unimplemented */

/* multiply/divide */
OP(201,OP_MPY  ( DXI         ) ) /* MPY dxi     */
OP(203,OP_MPY  ( S           ) ) /* MPY s       */
OP(205,OP_MPY  ( D           ) ) /* MPY d       */
OP(207,OP_MPY  ( DLI         ) ) /* MPY dli     */
OP(209,OP_MPY  ( IMM         ) ) /* MPY imm     */
OP(20d,OP_MPY  ( A           ) ) /* MPY a       */
OP(20f,OP_MPY  ( AL          ) ) /* MPY al      */
OP(211,OP_MPY  ( DIY         ) ) /* MPY diy     */
OP(212,OP_MPY  ( DI          ) ) /* MPY di      */
OP(213,OP_MPY  ( SIY         ) ) /* MPY siy     */
OP(215,OP_MPY  ( DX          ) ) /* MPY dx      */
OP(217,OP_MPY  ( DLIY        ) ) /* MPY dliy    */
OP(219,OP_MPY  ( AY          ) ) /* MPY ay      */
OP(21d,OP_MPY  ( AX          ) ) /* MPY ax      */
OP(21f,OP_MPY  ( ALX         ) ) /* MPY alx     */
OP(221,OP_DIV  ( DXI         ) ) /* DIV dxi     */
OP(223,OP_DIV  ( S           ) ) /* DIV s       */
OP(225,OP_DIV  ( D           ) ) /* DIV d       */
OP(227,OP_DIV  ( DLI         ) ) /* DIV dli     */
OP(228,OP_XAB  (             ) ) /* XAB         */
OP(229,OP_DIV  ( IMM         ) ) /* DIV imm     */
OP(22d,OP_DIV  ( A           ) ) /* DIV a       */
OP(22f,OP_DIV  ( AL          ) ) /* DIV al      */
OP(231,OP_DIV  ( DIY         ) ) /* DIV diy     */
OP(232,OP_DIV  ( DI          ) ) /* DIV di      */
OP(233,OP_DIV  ( SIY         ) ) /* DIV siy     */
OP(235,OP_DIV  ( DX          ) ) /* DIV dx      */
OP(237,OP_DIV  ( DLIY        ) ) /* DIV dliy    */
OP(239,OP_DIV  ( AY          ) ) /* DIV ay      */
OP(23d,OP_DIV  ( AX          ) ) /* DIV ax      */
OP(23f,OP_DIV  ( ALX         ) ) /* DIV alx     */
OP(249,OP_RLA  ( IMM         ) ) /* RLA imm     */
OP(2c2,OP_LDT  ( IMM         ) ) /* LDT imm     */
// note: block 28x-2bx is for 7750 opcodes, not implemented yet

TABLE_OPCODES =
//    00     01     02     03     04     05     06     07
//    08     09     0a     0b     0c     0d     0e     0f
{
	O(00), O(01), O(02), O(03), O(04), O(05), O(06), O(07),     // 00
	O(08), O(09), O(0a), O(0b), O(0c), O(0d), O(0e), O(0f),
	O(10), O(11), O(12), O(13), O(14), O(15), O(16), O(17),     // 10
	O(18), O(19), O(1a), O(1b), O(1c), O(1d), O(1e), O(1f),
	O(20), O(21), O(22), O(23), O(24), O(25), O(26), O(27),     // 20
	O(28), O(29), O(2a), O(2b), O(2c), O(2d), O(2e), O(2f),
	O(30), O(31), O(32), O(33), O(34), O(35), O(36), O(37),     // 30
	O(38), O(39), O(3a), O(3b), O(3c), O(3d), O(3e), O(3f),
	O(40), O(41), O(42), O(43), O(44), O(45), O(46), O(47),     // 40
	O(48), O(49), O(4a), O(4b), O(4c), O(4d), O(4e), O(4f),
	O(50), O(51), O(52), O(53), O(54), O(55), O(56), O(57),     // 50
	O(58), O(59), O(5a), O(5b), O(5c), O(5d), O(5e), O(5f),
	O(60), O(61), O(62), O(63), O(64), O(65), O(66), O(67),     // 60
	O(68), O(69), O(6a), O(6b), O(6c), O(6d), O(6e), O(6f),
	O(70), O(71), O(72), O(73), O(74), O(75), O(76), O(77),     // 70
	O(78), O(79), O(7a), O(7b), O(7c), O(7d), O(7e), O(7f),
	O(80), O(81), O(82), O(83), O(84), O(85), O(86), O(87),     // 80
	O(88), O(89), O(8a), O(8b), O(8c), O(8d), O(8e), O(8f),
	O(90), O(91), O(92), O(93), O(94), O(95), O(96), O(97),     // 90
	O(98), O(99), O(9a), O(9b), O(9c), O(9d), O(9e), O(9f),
	O(a0), O(a1), O(a2), O(a3), O(a4), O(a5), O(a6), O(a7),     // a0
	O(a8), O(a9), O(aa), O(ab), O(ac), O(ad), O(ae), O(af),
	O(b0), O(b1), O(b2), O(b3), O(b4), O(b5), O(b6), O(b7),     // b0
	O(b8), O(b9), O(ba), O(bb), O(bc), O(bd), O(be), O(bf),
	O(c0), O(c1), O(c2), O(c3), O(c4), O(c5), O(c6), O(c7),     // c0
	O(c8), O(c9), O(ca), O(cb), O(cc), O(cd), O(ce), O(cf),
	O(d0), O(d1), O(d2), O(d3), O(d4), O(d5), O(d6), O(d7),     // d0
	O(d8), O(d9), O(da), O(db), O(dc), O(dd), O(de), O(df),
	O(e0), O(e1), O(e2), O(e3), O(e4), O(e5), O(e6), O(e7),     // e0
	O(e8), O(e9), O(ea), O(eb), O(ec), O(ed), O(ee), O(ef),
	O(f0), O(f1), O(f2), O(f3), O(f4), O(f5), O(f6), O(f7),     // f0
	O(f8), O(f9), O(fa), O(fb), O(fc), O(fd), O(fe), O(ff)
};

TABLE_OPCODES2 =
//    00     01     02     03     04     05     06     07
//    08     09     0a     0b     0c     0d     0e     0f
{
	O(200),O(101),O(200),O(103),O(200),O(105),O(200),O(107),    // 00
	O(200),O(109),O(10a),O(200),O(200),O(10d),O(200),O(10f),
	O(200),O(111),O(112),O(113),O(200),O(115),O(200),O(117),    // 10
	O(200),O(119),O(11a),O(11b),O(200),O(11d),O(200),O(11f),
	O(200),O(121),O(200),O(123),O(200),O(125),O(200),O(127),    // 20
	O(200),O(129),O(12a),O(200),O(200),O(12d),O(200),O(12f),
	O(200),O(131),O(132),O(133),O(200),O(135),O(200),O(137),    // 30
	O(200),O(139),O(13a),O(13b),O(200),O(13d),O(200),O(13f),
	O(200),O(141),O(200),O(143),O(200),O(145),O(200),O(147),    // 40
	O(148),O(149),O(14a),O(200),O(200),O(14d),O(200),O(14f),
	O(200),O(151),O(152),O(153),O(200),O(155),O(200),O(157),    // 50
	O(200),O(159),O(200),O(15b),O(200),O(15d),O(200),O(15f),
	O(200),O(161),O(200),O(163),O(200),O(165),O(200),O(167),    // 60
	O(168),O(169),O(16a),O(200),O(200),O(16d),O(200),O(16f),
	O(200),O(171),O(172),O(173),O(200),O(175),O(200),O(177),    // 70
	O(200),O(179),O(200),O(17b),O(200),O(17d),O(200),O(17f),
	O(200),O(181),O(200),O(183),O(200),O(185),O(200),O(187),    // 80
	O(200),O(200),O(18a),O(200),O(200),O(18d),O(200),O(18f),
	O(200),O(191),O(192),O(193),O(200),O(195),O(200),O(197),    // 90
	O(198),O(199),O(200),O(200),O(200),O(19d),O(200),O(19f),
	O(200),O(1a1),O(200),O(1a3),O(200),O(1a5),O(200),O(1a7),    // a0
	O(1a8),O(1a9),O(1aa),O(200),O(200),O(1ad),O(200),O(1af),
	O(200),O(1b1),O(1b2),O(1b3),O(200),O(1b5),O(200),O(1b7),    // b0
	O(200),O(1b9),O(200),O(200),O(200),O(1bd),O(200),O(1bf),
	O(200),O(1c1),O(200),O(1c3),O(200),O(1c5),O(200),O(1c7),    // c0
	O(200),O(1c9),O(200),O(200),O(200),O(1cd),O(200),O(1cf),
	O(200),O(1d1),O(1d2),O(1d3),O(200),O(1d5),O(200),O(1d7),    // d0
	O(200),O(1d9),O(200),O(200),O(200),O(1dd),O(200),O(1df),
	O(200),O(1e1),O(200),O(1e3),O(200),O(1e5),O(200),O(1e7),    // e0
	O(200),O(1e9),O(200),O(200),O(200),O(1ed),O(200),O(1ef),
	O(200),O(1f1),O(1f2),O(1f3),O(200),O(1f5),O(200),O(1f7),    // f0
	O(200),O(1f9),O(200),O(200),O(200),O(1fd),O(200),O(1ff)
};

TABLE_OPCODES3 =
//    00     01     02     03     04     05     06     07
//    08     09     0a     0b     0c     0d     0e     0f
{
	O(200),O(201),O(200),O(203),O(200),O(205),O(200),O(207),    // 00
	O(200),O(209),O(200),O(200),O(200),O(20d),O(200),O(20f),
	O(200),O(211),O(212),O(213),O(200),O(215),O(200),O(217),    // 10
	O(200),O(219),O(200),O(200),O(200),O(21d),O(200),O(21f),
	O(200),O(221),O(200),O(223),O(200),O(225),O(200),O(227),    // 20
	O(228),O(229),O(200),O(200),O(200),O(22d),O(200),O(22f),
	O(200),O(231),O(232),O(233),O(200),O(235),O(200),O(237),    // 30
	O(200),O(239),O(200),O(200),O(200),O(23d),O(200),O(23f),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // 40
	O(200),O(249),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // 50
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // 60
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // 70
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // 80
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // 90
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // a0
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // b0
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(2c2),O(200),O(200),O(200),O(200),O(200),    // c0
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // d0
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // e0
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200),    // f0
	O(200),O(200),O(200),O(200),O(200),O(200),O(200),O(200)
};


/* Assert or clear a line on the CPU */
TABLE_FUNCTION(void, set_line, (int line, int state))
{
	switch(line)
	{
		// maskable interrupts
		case M37710_LINE_ADC:
		case M37710_LINE_UART1XMIT:
		case M37710_LINE_UART1RECV:
		case M37710_LINE_UART0XMIT:
		case M37710_LINE_UART0RECV:
		case M37710_LINE_TIMERB2:
		case M37710_LINE_TIMERB1:
		case M37710_LINE_TIMERB0:
		case M37710_LINE_TIMERA4:
		case M37710_LINE_TIMERA3:
		case M37710_LINE_TIMERA2:
		case M37710_LINE_TIMERA1:
		case M37710_LINE_TIMERA0:
		case M37710_LINE_IRQ2:
		case M37710_LINE_IRQ1:
		case M37710_LINE_IRQ0:
			switch(state)
			{
				case CLEAR_LINE:
					LINE_IRQ &= ~(1 << line);
					if (m37710_irq_levels[line])
					{
						m_m37710_regs[m37710_irq_levels[line]] &= ~8;
					}
					break;

				case ASSERT_LINE:
				case PULSE_LINE:
				case HOLD_LINE:
					LINE_IRQ |= (1 << line);
					if (m37710_irq_levels[line])
					{
						m_m37710_regs[m37710_irq_levels[line]] |= 8;
					}
					break;

				default: break;
			}
			break;

		default: break;
	}
}



/* Get a register from the CPU core */
TABLE_FUNCTION(UINT32, get_reg, (int regnum))
{
	switch(regnum)
	{
		case M37710_A: return REG_B | REG_A;
		case M37710_B: return REG_BB | REG_BA;
		case M37710_X: return REG_X;
		case M37710_Y: return REG_Y;
		case M37710_S: return REG_S;
		case M37710_PC: return REG_PC;
		case M37710_PB: return REG_PB >> 16;
		case M37710_DB: return REG_DB >> 16;
		case M37710_D: return REG_D;
		case M37710_P: return m37710i_get_reg_p();
		case M37710_IRQ_STATE: return LINE_IRQ;
		case STATE_GENPCBASE: return REG_PPC;
	}
	return 0;
}

TABLE_FUNCTION(void, set_reg, (int regnum, UINT32 val))
{
	switch(regnum)
	{
		case M37710_PC: REG_PC = MAKE_UINT_16(val); break;
		case M37710_S: REG_S = MAKE_UINT_16(val); break;
		case M37710_P: m37710i_set_reg_p(val); break;
#if FLAG_SET_M
		case M37710_A: REG_A = MAKE_UINT_8(val); REG_B = val&0xff00; break;
		case M37710_B: REG_BA = MAKE_UINT_8(val); REG_BB = val&0xff00; break;
#else
		case M37710_A: REG_A = MAKE_UINT_16(val); break;
		case M37710_B: REG_BA = MAKE_UINT_16(val); break;
#endif
#if FLAG_SET_X
		case M37710_X: REG_X = MAKE_UINT_8(val); break;
		case M37710_Y: REG_Y = MAKE_UINT_8(val); break;
#else
		case M37710_X: REG_X = MAKE_UINT_16(val); break;
		case M37710_Y: REG_Y = MAKE_UINT_16(val); break;
#endif
		case M37710_IRQ_STATE: (this->*FTABLE_SET_LINE)(M37710_LINE_IRQ0, val == 0 ? CLEAR_LINE : ASSERT_LINE); break;
	}
}

TABLE_FUNCTION(int, execute, (int clocks))
{
	if(!CPU_STOPPED)
	{
		CLOCKS = clocks;
		do
		{
			REG_PPC = REG_PC;
			M37710_CALL_DEBUGGER(REG_PB | REG_PC);
			REG_PC++;
			REG_IR = read_8_IMM(REG_PB | REG_PPC);
			(this->*m_opcodes[REG_IR])();
		} while(CLOCKS > 0);
		return clocks - CLOCKS;
	}
	return clocks;
}


/* ======================================================================== */
/* ================================== EOF ================================= */
/* ======================================================================== */
