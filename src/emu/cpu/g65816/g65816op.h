/* ======================================================================== */
/* ============================= CONFIGURATION ============================ */
/* ======================================================================== */

#undef FLAG_SET_E
#undef FLAG_SET_M
#undef FLAG_SET_X
#if EXECUTION_MODE == EXECUTION_MODE_E
#define FLAG_SET_E 1
#define FLAG_SET_M 1
#define FLAG_SET_X 1
#elif EXECUTION_MODE == EXECUTION_MODE_M0X0
#define FLAG_SET_E 0
#define FLAG_SET_M 0
#define FLAG_SET_X 0
#elif EXECUTION_MODE == EXECUTION_MODE_M0X1
#define FLAG_SET_E 0
#define FLAG_SET_M 0
#define FLAG_SET_X 1
#elif EXECUTION_MODE == EXECUTION_MODE_M1X0
#define FLAG_SET_E 0
#define FLAG_SET_M 1
#define FLAG_SET_X 0
#elif EXECUTION_MODE == EXECUTION_MODE_M1X1
#define FLAG_SET_E 0
#define FLAG_SET_M 1
#define FLAG_SET_X 1
#endif

#undef VECTOR_IRQ
#undef VECTOR_NMI
#undef VECTOR_ABORT
#undef VECTOR_BRK
#undef VECTOR_COP

#if FLAG_SET_E
#define VECTOR_IRQ			VECTOR_IRQ_E
#define VECTOR_NMI			VECTOR_NMI_E
#define VECTOR_ABORT		VECTOR_ABORT_E
#define VECTOR_BRK			VECTOR_BRK_E
#define VECTOR_COP			VECTOR_COP_E
#else /* FLAG_SET_E */
#define VECTOR_IRQ			VECTOR_IRQ_N
#define VECTOR_NMI			VECTOR_NMI_N
#define VECTOR_ABORT		VECTOR_ABORT_N
#define VECTOR_BRK			VECTOR_BRK_N
#define VECTOR_COP			VECTOR_COP_N
#endif



/* ======================================================================== */
/* ================================= MEMORY =============================== */
/* ======================================================================== */

#define ADDRESS_65816(A) ((A)&0x00ffffff)

INLINE uint g65816i_read_8_normal(uint address)
{
	address = ADDRESS_65816(address);
	return g65816_read_8(address);
}

INLINE uint g65816i_read_8_immediate(uint address)
{
	address = ADDRESS_65816(address);
	return g65816_read_8_immediate(address);
}

INLINE uint g65816i_read_8_direct(uint address)
{
#if FLAG_SET_E
	/* force address into zero page */
	address = REGISTER_D + MAKE_UINT_8(address - REGISTER_D);
#else
	address = ADDRESS_65816(address);
#endif
	return g65816_read_8(address);
}

INLINE uint g65816i_read_8_vector(uint address)
{
	if (READ_VECTOR)
		return READ_VECTOR(address);
	else
		return g65816i_read_8_normal(address);
}

INLINE void g65816i_write_8_normal(uint address, uint value)
{
	address = ADDRESS_65816(address);
	g65816_write_8(address, MAKE_UINT_8(value));
}

INLINE void g65816i_write_8_direct(uint address, uint value)
{
#if FLAG_SET_E
	/* force address into zero page */
	address = REGISTER_D + MAKE_UINT_8(address - REGISTER_D);
#else
	address = ADDRESS_65816(address);
#endif
	g65816_write_8(address, MAKE_UINT_8(value));
}

INLINE uint g65816i_read_16_normal(uint address)
{
	return	 g65816i_read_8_normal(address) |
			(g65816i_read_8_normal(address+1)<<8);
}

INLINE uint g65816i_read_16_immediate(uint address)
{
	return	 g65816i_read_8_immediate(address) |
			(g65816i_read_8_immediate(address+1)<<8);
}

INLINE uint g65816i_read_16_direct(uint address)
{
	return   g65816i_read_8_direct(address) |
			(g65816i_read_8_direct(address+1)<<8);
}

INLINE uint g65816i_read_16_vector(uint address)
{
	return	 g65816i_read_8_vector(address) |
			(g65816i_read_8_vector(address+1)<<8);
}

INLINE void g65816i_write_16_normal(uint address, uint value)
{
	g65816i_write_8_normal(address, value&0xff);
	g65816i_write_8_normal(address+1, value>>8);
}

INLINE void g65816i_write_16_direct(uint address, uint value)
{
	g65816i_write_8_direct(address, value&0xff);
	g65816i_write_8_direct(address+1, value>>8);
}

INLINE uint g65816i_read_24_normal(uint address)
{
	return	 g65816i_read_8_normal(address)       |
			(g65816i_read_8_normal(address+1)<<8) |
			(g65816i_read_8_normal(address+2)<<16);
}

INLINE uint g65816i_read_24_immediate(uint address)
{
	return	 g65816i_read_8_immediate(address)       |
			(g65816i_read_8_immediate(address+1)<<8) |
			(g65816i_read_8_immediate(address+2)<<16);
}

INLINE uint g65816i_read_24_direct(uint address)
{
	return	 g65816i_read_8_direct(address)         |
			(g65816i_read_8_direct(address+1)<<8) |
			(g65816i_read_8_direct(address+2)<<16);
}



/* ======================================================================== */
/* ================================= STACK ================================ */
/* ======================================================================== */

INLINE void g65816i_push_8(uint value)
{
	g65816i_write_8_normal(REGISTER_S, value);
#if FLAG_SET_E
	REGISTER_S = MAKE_UINT_8(REGISTER_S-1) | 0x100;
#else
	REGISTER_S = MAKE_UINT_16(REGISTER_S-1);
#endif
}

INLINE uint g65816i_pull_8(void)
{
#if FLAG_SET_E
	REGISTER_S = MAKE_UINT_8(REGISTER_S+1) | 0x100;
#else
	REGISTER_S = MAKE_UINT_16(REGISTER_S+1);
#endif
	return g65816i_read_8_normal(REGISTER_S);
}

INLINE void g65816i_push_16(uint value)
{
	g65816i_push_8(value>>8);
	g65816i_push_8(value&0xff);
}

INLINE uint g65816i_pull_16(void)
{
	uint res = g65816i_pull_8();
	return res | (g65816i_pull_8() << 8);
}

INLINE void g65816i_push_24(uint value)
{
	g65816i_push_8(value>>16);
	g65816i_push_8((value>>8)&0xff);
	g65816i_push_8(value&0xff);
}

INLINE uint g65816i_pull_24(void)
{
	uint res = g65816i_pull_8();
	res |= g65816i_pull_8() << 8;
	return res | (g65816i_pull_8() << 16);
}


/* ======================================================================== */
/* ============================ PROGRAM COUNTER =========================== */
/* ======================================================================== */

INLINE void g65816i_jump_16(uint address)
{
	REGISTER_PC = MAKE_UINT_16(address);
	g65816i_jumping(REGISTER_PC);
}

INLINE void g65816i_jump_24(uint address)
{
	REGISTER_PB = address&0xff0000;
	REGISTER_PC = MAKE_UINT_16(address);
	g65816i_jumping(REGISTER_PC);
}

INLINE void g65816i_branch_8(uint offset)
{
#if FLAG_SET_E
	uint old_pc = REGISTER_PC;
	REGISTER_PC = MAKE_UINT_16(REGISTER_PC + MAKE_INT_8(offset));
	if((REGISTER_PC^old_pc)&0xff00)
		CLK(1);
#else
	REGISTER_PC = MAKE_UINT_16(REGISTER_PC + MAKE_INT_8(offset));
#endif
	g65816i_branching(REGISTER_PC);
}

INLINE void g65816i_branch_16(uint offset)
{
	REGISTER_PC = MAKE_UINT_16(REGISTER_PC + offset);
	g65816i_branching(REGISTER_PC);
}


/* ======================================================================== */
/* ============================ STATUS REGISTER =========================== */
/* ======================================================================== */

#if !FLAG_SET_E
INLINE void g65816i_set_flag_mx(uint value)
{
#if FLAG_SET_M
	if(!(value & FLAGPOS_M))
	{
		REGISTER_A |= REGISTER_B;
		REGISTER_B = 0;
		FLAG_M = MFLAG_CLEAR;
	}
#else
	if(value & FLAGPOS_M)
	{
		REGISTER_B = REGISTER_A & 0xff00;
		REGISTER_A = MAKE_UINT_8(REGISTER_A);
		FLAG_M = MFLAG_SET;
	}
#endif
#if FLAG_SET_X
	if(!(value & FLAGPOS_X))
	{
		FLAG_X = XFLAG_CLEAR;
	}
#else
	if(value & FLAGPOS_X)
	{
		REGISTER_X = MAKE_UINT_8(REGISTER_X);
		REGISTER_Y = MAKE_UINT_8(REGISTER_Y);
		FLAG_X = XFLAG_SET;
	}
#endif
	g65816i_set_execution_mode((FLAG_M>>4) | (FLAG_X>>4));
}
#endif

INLINE void g65816i_set_flag_e(uint value)
{
#if FLAG_SET_E
	if(!value)
	{
		FLAG_E = EFLAG_CLEAR;
		g65816i_set_execution_mode(EXECUTION_MODE_M1X1);
	}
#else
	if(value)
	{
#if !FLAG_SET_M
		REGISTER_B = REGISTER_A & 0xff00;
		REGISTER_A &= 0x00ff;
		FLAG_M = MFLAG_SET;
#endif
#if !FLAG_SET_X
		REGISTER_X = MAKE_UINT_8(REGISTER_X);
		REGISTER_Y = MAKE_UINT_8(REGISTER_Y);
		FLAG_X = XFLAG_SET;
#endif
		REGISTER_S = MAKE_UINT_8(REGISTER_S) | 0x100;
		FLAG_E = EFLAG_SET;
		g65816i_set_execution_mode(EXECUTION_MODE_E);
	}
#endif
}

//INLINE void g65816i_check_maskable_interrupt(void);

INLINE void g65816i_set_flag_i(uint value)
{
	value &= FLAGPOS_I;
	if(!FLAG_I || value)
	{
		FLAG_I = value;
		return;
	}
	FLAG_I = value;
//  g65816i_check_maskable_interrupt();
}




/* Get the Processor Status Register */
INLINE uint g65816i_get_reg_p(void)
{
	return	(FLAG_N&0x80)		|
			((FLAG_V>>1)&0x40)	|
			FLAG_M				|
			FLAG_X				|
			FLAG_D				|
			FLAG_I				|
			((!FLAG_Z)<<1)		|
			((FLAG_C>>8)&1);
}

INLINE void g65816i_set_reg_p(uint value)
{
#if FLAG_SET_E
	FLAG_N = value;
	FLAG_V = value << 1;
	FLAG_D = value & FLAGPOS_D;
	FLAG_Z = !(value & FLAGPOS_Z);
	FLAG_C = value << 8;
	g65816i_set_flag_i(value);
#else
	FLAG_N = value;
	FLAG_V = value << 1;
	FLAG_D = value & FLAGPOS_D;
	FLAG_Z = !(value & FLAGPOS_Z);
	FLAG_C = value << 8;
	g65816i_set_flag_mx(value);
	g65816i_set_flag_i(value);
#endif
}


/* ======================================================================== */
/* =============================== INTERRUPTS ============================= */
/* ======================================================================== */

INLINE void g65816i_interrupt_hardware(uint vector)
{
#if FLAG_SET_E
	CLK(7);
	g65816i_push_16(REGISTER_PC);
	g65816i_push_8(g65816i_get_reg_p() & ~FLAGPOS_B);
	FLAG_D = DFLAG_CLEAR;
	g65816i_set_flag_i(IFLAG_SET);
	REGISTER_PB = 0;
	g65816i_jump_16(g65816i_read_16_vector(vector));
	if(INT_ACK) INT_ACK(0);
#else
	CLK(8);
	g65816i_push_8(REGISTER_PB>>16);
	g65816i_push_16(REGISTER_PC);
	g65816i_push_8(g65816i_get_reg_p());
	FLAG_D = DFLAG_CLEAR;
	g65816i_set_flag_i(IFLAG_SET);
	REGISTER_PB = 0;
	g65816i_jump_16(g65816i_read_16_vector(vector));
	if(INT_ACK) INT_ACK(0);
#endif
}

INLINE void g65816i_interrupt_software(uint vector)
{
#if FLAG_SET_E
	CLK(7);
	g65816i_push_16(REGISTER_PC);
	g65816i_push_8(g65816i_get_reg_p());
	FLAG_D = DFLAG_CLEAR;
	g65816i_set_flag_i(IFLAG_SET);
	REGISTER_PB = 0;
	g65816i_jump_16(g65816i_read_16_normal(vector));
#else
	CLK(8);
	g65816i_push_8(REGISTER_PB>>16);
	g65816i_push_16(REGISTER_PC);
	g65816i_push_8(g65816i_get_reg_p());
	FLAG_D = DFLAG_CLEAR;
	g65816i_set_flag_i(IFLAG_SET);
	REGISTER_PB = 0;
	g65816i_jump_16(g65816i_read_16_normal(vector));
#endif
}

INLINE void g65816i_interrupt_nmi(void)
{
#if FLAG_SET_E
	CLK(7);
	g65816i_push_16(REGISTER_PC);
	g65816i_push_8(g65816i_get_reg_p() & ~FLAGPOS_B);
	FLAG_D = DFLAG_CLEAR;
	REGISTER_PB = 0;
	g65816i_jump_16(g65816i_read_16_normal(VECTOR_NMI));
#else
	CLK(8);
	g65816i_push_8(REGISTER_PB>>16);
	g65816i_push_16(REGISTER_PC);
	g65816i_push_8(g65816i_get_reg_p());
	FLAG_D = DFLAG_CLEAR;
	REGISTER_PB = 0;
	g65816i_jump_16(g65816i_read_16_normal(VECTOR_NMI));
#endif
}


INLINE void g65816i_check_maskable_interrupt(void)
{
	if(!(CPU_STOPPED & STOP_LEVEL_STOP) && LINE_IRQ && !FLAG_I)
	{
		g65816i_interrupt_hardware(VECTOR_IRQ);
		CPU_STOPPED &= ~STOP_LEVEL_WAI;
		LINE_IRQ=0;
	}
}


/* ======================================================================== */
/* ========================== EFFECTIVE ADDRESSES ========================= */
/* ======================================================================== */

/* Effective-address based memory access macros */
#define read_8_NORM(A)		g65816i_read_8_normal(A)
#define read_8_IMM(A)		g65816i_read_8_immediate(A)
#define read_8_D(A)			g65816i_read_8_direct(A)
#define read_8_A(A)			g65816i_read_8_normal(A)
#define read_8_AL(A)		g65816i_read_8_normal(A)
#define read_8_DX(A)		g65816i_read_8_direct(A)
#define read_8_DY(A)		g65816i_read_8_direct(A)
#define read_8_AX(A)		g65816i_read_8_normal(A)
#define read_8_ALX(A)		g65816i_read_8_normal(A)
#define read_8_AY(A)		g65816i_read_8_normal(A)
#define read_8_DI(A)		g65816i_read_8_normal(A)
#define read_8_DLI(A)		g65816i_read_8_normal(A)
#define read_8_AI(A)		g65816i_read_8_normal(A)
#define read_8_ALI(A)		g65816i_read_8_normal(A)
#define read_8_DXI(A)		g65816i_read_8_normal(A)
#define read_8_DIY(A)		g65816i_read_8_normal(A)
#define read_8_DLIY(A)		g65816i_read_8_normal(A)
#define read_8_AXI(A)		g65816i_read_8_normal(A)
#define read_8_S(A)			g65816i_read_8_normal(A)
#define read_8_SIY(A)		g65816i_read_8_normal(A)

#define read_16_NORM(A)		g65816i_read_16_normal(A)
#define read_16_IMM(A)		g65816i_read_16_immediate(A)
#define read_16_D(A)		g65816i_read_16_direct(A)
#define read_16_A(A)		g65816i_read_16_normal(A)
#define read_16_AL(A)		g65816i_read_16_normal(A)
#define read_16_DX(A)		g65816i_read_16_direct(A)
#define read_16_DY(A)		g65816i_read_16_direct(A)
#define read_16_AX(A)		g65816i_read_16_normal(A)
#define read_16_ALX(A)		g65816i_read_16_normal(A)
#define read_16_AY(A)		g65816i_read_16_normal(A)
#define read_16_DI(A)		g65816i_read_16_normal(A)
#define read_16_DLI(A)		g65816i_read_16_normal(A)
#define read_16_AI(A)		g65816i_read_16_normal(A)
#define read_16_ALI(A)		g65816i_read_16_normal(A)
#define read_16_DXI(A)		g65816i_read_16_normal(A)
#define read_16_DIY(A)		g65816i_read_16_normal(A)
#define read_16_DLIY(A)		g65816i_read_16_normal(A)
#define read_16_AXI(A)		g65816i_read_16_normal(A)
#define read_16_S(A)		g65816i_read_16_normal(A)
#define read_16_SIY(A)		g65816i_read_16_normal(A)

#define read_24_NORM(A)		g65816i_read_24_normal(A)
#define read_24_IMM(A)		g65816i_read_24_immediate(A)
#define read_24_D(A)		g65816i_read_24_direct(A)
#define read_24_A(A)		g65816i_read_24_normal(A)
#define read_24_AL(A)		g65816i_read_24_normal(A)
#define read_24_DX(A)		g65816i_read_24_direct(A)
#define read_24_DY(A)		g65816i_read_24_direct(A)
#define read_24_AX(A)		g65816i_read_24_normal(A)
#define read_24_ALX(A)		g65816i_read_24_normal(A)
#define read_24_AY(A)		g65816i_read_24_normal(A)
#define read_24_DI(A)		g65816i_read_24_normal(A)
#define read_24_DLI(A)		g65816i_read_24_normal(A)
#define read_24_AI(A)		g65816i_read_24_normal(A)
#define read_24_ALI(A)		g65816i_read_24_normal(A)
#define read_24_DXI(A)		g65816i_read_24_normal(A)
#define read_24_DIY(A)		g65816i_read_24_normal(A)
#define read_24_DLIY(A)		g65816i_read_24_normal(A)
#define read_24_AXI(A)		g65816i_read_24_normal(A)
#define read_24_S(A)		g65816i_read_24_normal(A)
#define read_24_SIY(A)		g65816i_read_24_normal(A)

#define write_8_NORM(A, V)	g65816i_write_8_normal(A, V)
#define write_8_D(A, V)		g65816i_write_8_direct(A, V)
#define write_8_A(A, V)		g65816i_write_8_normal(A, V)
#define write_8_AL(A, V)	g65816i_write_8_normal(A, V)
#define write_8_DX(A, V)	g65816i_write_8_direct(A, V)
#define write_8_DY(A, V)	g65816i_write_8_direct(A, V)
#define write_8_AX(A, V)	g65816i_write_8_normal(A, V)
#define write_8_ALX(A, V)	g65816i_write_8_normal(A, V)
#define write_8_AY(A, V)	g65816i_write_8_normal(A, V)
#define write_8_DI(A, V)	g65816i_write_8_normal(A, V)
#define write_8_DLI(A, V)	g65816i_write_8_normal(A, V)
#define write_8_AI(A, V)	g65816i_write_8_normal(A, V)
#define write_8_ALI(A, V)	g65816i_write_8_normal(A, V)
#define write_8_DXI(A, V)	g65816i_write_8_normal(A, V)
#define write_8_DIY(A, V)	g65816i_write_8_normal(A, V)
#define write_8_DLIY(A, V)	g65816i_write_8_normal(A, V)
#define write_8_AXI(A, V)	g65816i_write_8_normal(A, V)
#define write_8_S(A, V)		g65816i_write_8_normal(A, V)
#define write_8_SIY(A, V)	g65816i_write_8_normal(A, V)

#define write_16_NORM(A, V)	g65816i_write_16_normal(A, V)
#define write_16_D(A, V)	g65816i_write_16_direct(A, V)
#define write_16_A(A, V)	g65816i_write_16_normal(A, V)
#define write_16_AL(A, V)	g65816i_write_16_normal(A, V)
#define write_16_DX(A, V)	g65816i_write_16_direct(A, V)
#define write_16_DY(A, V)	g65816i_write_16_direct(A, V)
#define write_16_AX(A, V)	g65816i_write_16_normal(A, V)
#define write_16_ALX(A, V)	g65816i_write_16_normal(A, V)
#define write_16_AY(A, V)	g65816i_write_16_normal(A, V)
#define write_16_DI(A, V)	g65816i_write_16_normal(A, V)
#define write_16_DLI(A, V)	g65816i_write_16_normal(A, V)
#define write_16_AI(A, V)	g65816i_write_16_normal(A, V)
#define write_16_ALI(A, V)	g65816i_write_16_normal(A, V)
#define write_16_DXI(A, V)	g65816i_write_16_normal(A, V)
#define write_16_DIY(A, V)	g65816i_write_16_normal(A, V)
#define write_16_DLIY(A, V)	g65816i_write_16_normal(A, V)
#define write_16_AXI(A, V)	g65816i_write_16_normal(A, V)
#define write_16_S(A, V)	g65816i_write_16_normal(A, V)
#define write_16_SIY(A, V)	g65816i_write_16_normal(A, V)


#define OPER_8_IMM()		read_8_IMM(EA_IMM8())
#define OPER_8_D()			read_8_D(EA_D())
#define OPER_8_A()			read_8_A(EA_A())
#define OPER_8_AL()			read_8_AL(EA_AL())
#define OPER_8_DX()			read_8_DX(EA_DX())
#define OPER_8_DY()			read_8_DY(EA_DY())
#define OPER_8_AX()			read_8_AX(EA_AX())
#define OPER_8_ALX()		read_8_ALX(EA_ALX())
#define OPER_8_AY()			read_8_AY(EA_AY())
#define OPER_8_DI()			read_8_DI(EA_DI())
#define OPER_8_DLI()		read_8_DLI(EA_DLI())
#define OPER_8_AI()			read_8_AI(EA_AI())
#define OPER_8_ALI()		read_8_ALI(EA_ALI())
#define OPER_8_DXI()		read_8_DXI(EA_DXI())
#define OPER_8_DIY()		read_8_DIY(EA_DIY())
#define OPER_8_DLIY()		read_8_DLIY(EA_DLIY())
#define OPER_8_AXI()		read_8_AXI(EA_AXI())
#define OPER_8_S()			read_8_S(EA_S())
#define OPER_8_SIY()		read_8_SIY(EA_SIY())

#define OPER_16_IMM()		read_16_IMM(EA_IMM16())
#define OPER_16_D()			read_16_D(EA_D())
#define OPER_16_A()			read_16_A(EA_A())
#define OPER_16_AL()		read_16_AL(EA_AL())
#define OPER_16_DX()		read_16_DX(EA_DX())
#define OPER_16_DY()		read_16_DY(EA_DY())
#define OPER_16_AX()		read_16_AX(EA_AX())
#define OPER_16_ALX()		read_16_ALX(EA_ALX())
#define OPER_16_AY()		read_16_AY(EA_AY())
#define OPER_16_DI()		read_16_DI(EA_DI())
#define OPER_16_DLI()		read_16_DLI(EA_DLI())
#define OPER_16_AI()		read_16_AI(EA_AI())
#define OPER_16_ALI()		read_16_ALI(EA_ALI())
#define OPER_16_DXI()		read_16_DXI(EA_DXI())
#define OPER_16_DIY()		read_16_DIY(EA_DIY())
#define OPER_16_DLIY()		read_16_DLIY(EA_DLIY())
#define OPER_16_AXI()		read_16_AXI(EA_AXI())
#define OPER_16_S()			read_16_S(EA_S())
#define OPER_16_SIY()		read_16_SIY(EA_SIY())

#define OPER_24_IMM()		read_24_IMM(EA_IMM24())
#define OPER_24_D()			read_24_D(EA_D())
#define OPER_24_A()			read_24_A(EA_A())
#define OPER_24_AL()		read_24_AL(EA_AL())
#define OPER_24_DX()		read_24_DX(EA_DX())
#define OPER_24_DY()		read_24_DY(EA_DY())
#define OPER_24_AX()		read_24_AX(EA_AX())
#define OPER_24_ALX()		read_24_ALX(EA_ALX())
#define OPER_24_AY()		read_24_AY(EA_AY())
#define OPER_24_DI()		read_24_DI(EA_DI())
#define OPER_24_DLI()		read_24_DLI(EA_DLI())
#define OPER_24_AI()		read_24_AI(EA_AI())
#define OPER_24_ALI()		read_24_ALI(EA_ALI())
#define OPER_24_DXI()		read_24_DXI(EA_DXI())
#define OPER_24_DIY()		read_24_DIY(EA_DIY())
#define OPER_24_DLIY()		read_24_DLIY(EA_DLIY())
#define OPER_24_AXI()		read_24_AXI(EA_AXI())
#define OPER_24_S()			read_24_S(EA_S())
#define OPER_24_SIY()		read_24_SIY(EA_SIY())

INLINE uint EA_IMM8(void)  {REGISTER_PC += 1; return REGISTER_PB | MAKE_UINT_16(REGISTER_PC-1);}
INLINE uint EA_IMM16(void) {REGISTER_PC += 2; return REGISTER_PB | MAKE_UINT_16(REGISTER_PC-2);}
INLINE uint EA_IMM24(void) {REGISTER_PC += 3; return REGISTER_PB | MAKE_UINT_16(REGISTER_PC-3);}
INLINE uint EA_D(void)     {if(MAKE_UINT_8(REGISTER_D)) CLK(1); return MAKE_UINT_16(REGISTER_D + OPER_8_IMM());}
INLINE uint EA_A(void)     {return REGISTER_DB | OPER_16_IMM();}
INLINE uint EA_AL(void)    {return OPER_24_IMM();}
INLINE uint EA_DX(void)    {return MAKE_UINT_16(REGISTER_D + OPER_8_IMM() + REGISTER_X);}
INLINE uint EA_DY(void)    {return MAKE_UINT_16(REGISTER_D + OPER_8_IMM() + REGISTER_Y);}
INLINE uint EA_AX(void)    {uint tmp = EA_A(); if((tmp^(tmp+REGISTER_X))&0xff00) CLK(1); return tmp + REGISTER_X;}
INLINE uint EA_ALX(void)   {return EA_AL() + REGISTER_X;}
INLINE uint EA_AY(void)    {uint tmp = EA_A(); if((tmp^(tmp+REGISTER_X))&0xff00) CLK(1); return tmp + REGISTER_Y;}
INLINE uint EA_DI(void)    {return REGISTER_DB | OPER_16_D();}
INLINE uint EA_DLI(void)   {return OPER_24_D();}
INLINE uint EA_AI(void)    {return read_16_A(OPER_16_IMM());}
INLINE uint EA_ALI(void)   {return OPER_24_A();}
INLINE uint EA_DXI(void)   {return REGISTER_DB | OPER_16_DX();}
INLINE uint EA_DIY(void)   {uint tmp = REGISTER_DB | OPER_16_D(); if((tmp^(tmp+REGISTER_X))&0xff00) CLK(1); return tmp + REGISTER_Y;}
INLINE uint EA_DLIY(void)  {return OPER_24_D() + REGISTER_Y;}
INLINE uint EA_AXI(void)   {return read_16_AXI(MAKE_UINT_16(OPER_16_IMM() + REGISTER_X));}
INLINE uint EA_S(void)     {return MAKE_UINT_16(REGISTER_S + OPER_8_IMM());}
INLINE uint EA_SIY(void)   {return MAKE_UINT_16(read_16_SIY(REGISTER_S + OPER_8_IMM()) + REGISTER_Y) | REGISTER_DB;}



/* ======================================================================== */
/* =========================== OPERATION MACROS =========================== */
/* ======================================================================== */

/* M6502   Add With Carry */
#undef OP_ADC
#if FLAG_SET_M
#define OP_ADC(MODE)														\
	{																		\
			unsigned tmp16;													\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			SRC    = OPER_8_##MODE();										\
			if(FLAG_D)														\
			{																\
				unsigned tmp1, tmp2, tmpA, tmpB;							\
				tmp1 = REGISTER_A & 0x0F;									\
				tmp2 = (REGISTER_A >> 4) & 0x0F;							\
				tmpA = SRC&0x0F;											\
				tmpB = (SRC >> 4) & 0x0F;									\
				tmp1 = tmp1 + tmpA + CFLAG_AS_1();							\
				tmp2 = tmp2 + tmpB;											\
				FLAG_C = CFLAG_CLEAR;										\
				if(tmp1 > 0x09)												\
				{															\
					tmp2++;													\
					tmp1 = ((tmp1 + 6) & 0x0F);								\
				}															\
				if(tmp2 > 0x09)												\
				{															\
					tmp2=((tmp2 + 6) & 0x0F);								\
					FLAG_C = CFLAG_SET;										\
				}															\
				tmp16 = tmp1 | (tmp2 << 4);									\
				FLAG_V = VFLAG_ADD_8(SRC, REGISTER_A, tmp16);				\
			}																\
			else															\
			{																\
				FLAG_C = tmp16 = REGISTER_A + SRC + CFLAG_AS_1();			\
				FLAG_V = VFLAG_ADD_8(SRC, REGISTER_A, FLAG_C);				\
			}																\
			FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_8(tmp16);				\
	}

#else
#define OP_ADC(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			SRC    = OPER_16_##MODE();										\
			if(!FLAG_D)														\
			{																\
				FLAG_C = REGISTER_A + SRC + CFLAG_AS_1();					\
				FLAG_V = VFLAG_ADD_16(SRC, REGISTER_A, FLAG_C);				\
				FLAG_Z = REGISTER_A = MAKE_UINT_16(FLAG_C);					\
				FLAG_N = NFLAG_16(REGISTER_A);								\
				FLAG_C = CFLAG_16(FLAG_C);									\
				BREAKOUT;													\
			}																\
			else															\
			{																\
				unsigned tmp16;												\
				unsigned tmp1,tmp2,tmp3, tmp4, tmpA, tmpB, tmpC, tmpD;		\
				tmp1 = REGISTER_A & 0x0F;									\
				tmp2 = (REGISTER_A >> 4) & 0x0F;							\
				tmp3 = (REGISTER_A >> 8) & 0x0F;							\
				tmp4 = (REGISTER_A >> 12) & 0x0F;							\
				tmpA = SRC & 0x0F;											\
				tmpB = (SRC >> 4) & 0x0F;									\
				tmpC = (SRC >> 8) & 0x0F;									\
				tmpD = (SRC >> 12) & 0x0F;									\
				tmp1 = tmp1 + tmpA + CFLAG_AS_1();							\
				tmp2 = tmp2 + tmpB;											\
				tmp3 = tmp3 + tmpC;											\
				tmp4 = tmp4 + tmpD;											\
				FLAG_C = CFLAG_CLEAR;										\
				if(tmp1 > 9)												\
				{															\
					tmp2++;													\
					tmp1 = ((tmp1 + 6) & 0x0F);								\
				}															\
				if(tmp2 > 9)												\
				{															\
					tmp3++;													\
					tmp2 = ((tmp2 + 6) & 0x0F);								\
				}															\
				if(tmp3 > 9)												\
				{															\
					tmp4++;													\
					tmp3 = ((tmp3 + 6) & 0x0F);								\
				}															\
				if(tmp4 > 9)												\
				{															\
					FLAG_C = CFLAG_SET;										\
					tmp4 = ((tmp4 + 6) & 0x0F);								\
				}															\
				tmp16 = tmp1 | (tmp2 << 4) | (tmp3 << 8) | (tmp4 << 12);	\
				FLAG_V = VFLAG_ADD_16(SRC, REGISTER_A, tmp16);				\
				FLAG_Z = REGISTER_A = MAKE_UINT_16(tmp16);					\
				FLAG_N = NFLAG_16(REGISTER_A);								\
			}

#endif


/* M6502   Logical AND with accumulator */
#undef OP_AND
#if FLAG_SET_M
#define OP_AND(MODE)														\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_N = FLAG_Z = REGISTER_A &= OPER_8_##MODE()
#else
#define OP_AND(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_Z = REGISTER_A &= OPER_16_##MODE();								\
			FLAG_N = NFLAG_16(REGISTER_A)
#endif

/* M6502   Arithmetic Shift Left accumulator */
#undef OP_ASL
#if FLAG_SET_M
#define OP_ASL()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_C = REGISTER_A << 1;											\
			FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_8(FLAG_C)
#else
#define OP_ASL()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_C = REGISTER_A << 1;											\
			FLAG_Z = REGISTER_A = MAKE_UINT_16(FLAG_C);							\
			FLAG_N = NFLAG_16(FLAG_C);										\
			FLAG_C = CFLAG_16(FLAG_C)
#endif

/* M6502   Arithmetic Shift Left operand */
#undef OP_ASLM
#if FLAG_SET_M
#define OP_ASLM(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_C = read_8_##MODE(DST) << 1;								\
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);							\
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_ASLM(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_C = read_16_##MODE(DST) << 1;								\
			FLAG_Z = MAKE_UINT_16(FLAG_C);									\
			FLAG_N = NFLAG_16(FLAG_C);										\
			FLAG_C = CFLAG_16(FLAG_C);										\
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M6502   Branch on Condition Code */
#undef OP_BCC
#define OP_BCC(COND)														\
			DST = OPER_8_IMM();												\
			if(COND)														\
			{																\
				CLK(CLK_OP + CLK_RELATIVE_8 + 1);							\
				g65816i_branch_8(DST);										\
				BREAKOUT;													\
			}																\
			CLK(CLK_OP + CLK_RELATIVE_8);									\

/* M6502   Set flags according to bits */
#undef OP_BIT
#if FLAG_SET_M
#define OP_BIT(MODE)														\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_N = OPER_8_##MODE();										\
			FLAG_Z = FLAG_N & REGISTER_A;										\
			FLAG_V = FLAG_N << 1
#else
#define OP_BIT(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_N = OPER_16_##MODE();										\
			FLAG_Z = FLAG_N & REGISTER_A;										\
			FLAG_N = NFLAG_16(FLAG_N);										\
			FLAG_V = FLAG_N << 1
#endif

/* G65816  Set flags according to bits (immediate addressing mode) */
#undef OP_BITI
#if FLAG_SET_M
#define OP_BITI()															\
			CLK(CLK_OP + CLK_R8 + CLK_IMM);									\
			FLAG_Z = REGISTER_A & OPER_8_IMM()
#else
#define OP_BITI()															\
			CLK(CLK_OP + CLK_R16 + CLK_IMM);								\
			FLAG_Z = REGISTER_A & OPER_16_IMM()
#endif

/* M6502   Cause a Break interrupt */
#undef OP_BRK
#define OP_BRK()															\
			REGISTER_PC++;														\
			g65816i_interrupt_software(VECTOR_BRK)

/* G65816  Branch Always */
#undef OP_BRA
#define OP_BRA()															\
			CLK(CLK_OP + CLK_IMPLIED + CLK_RELATIVE_8);						\
			g65816i_branch_8(OPER_8_IMM())

/* G65816  Branch Always Long */
#undef OP_BRL
#define OP_BRL()															\
			CLK(CLK_OP + CLK_IMPLIED + CLK_RELATIVE_16);					\
			g65816i_branch_16(OPER_16_IMM())

/* M6502   Clear Carry flag */
#undef OP_CLC
#define OP_CLC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_C = CFLAG_CLEAR

/* M6502   Clear Decimal flag */
#undef OP_CLD
#define OP_CLD()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_D = DFLAG_CLEAR

/* M6502   Clear Interrupt Mask flag */
#undef OP_CLI
#define OP_CLI()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			g65816i_set_flag_i(IFLAG_CLEAR)

/* M6502   Clear oVerflow flag */
#undef OP_CLV
#define OP_CLV()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_V = VFLAG_CLEAR

/* M6502   Compare operand to accumulator */
/* Unusual behavior: C flag is inverted */
#undef OP_CMP
#if FLAG_SET_M
#define OP_CMP(MODE)														\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_C = REGISTER_A - OPER_8_##MODE();								\
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);							\
			FLAG_C ^= CFLAG_SET
#else
#define OP_CMP(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_C = REGISTER_A - OPER_16_##MODE();								\
			FLAG_Z = MAKE_UINT_16(FLAG_C);									\
			FLAG_N = NFLAG_16(FLAG_C);										\
			FLAG_C = ~CFLAG_16(FLAG_C)
#endif

/* M6502   Compare operand to index register */
/* Unusual behavior: C flag is inverted */
#undef OP_CMPX
#if FLAG_SET_X
#define OP_CMPX(REG, MODE)													\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_C = REG - OPER_8_##MODE();									\
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);							\
			FLAG_C ^= CFLAG_SET
#else
#define OP_CMPX(REG, MODE)													\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_C = REG - OPER_16_##MODE();								\
			FLAG_Z = MAKE_UINT_16(FLAG_C);									\
			FLAG_N = NFLAG_16(FLAG_C);										\
			FLAG_C = ~CFLAG_16(FLAG_C)
#endif

/* G65816  Coprocessor operation */
#undef OP_COP
#define OP_COP()															\
			REGISTER_PC++;														\
			g65816i_interrupt_software(VECTOR_COP)

/* M6502   Decrement accumulator */
#undef OP_DEC
#if FLAG_SET_M
#define OP_DEC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_8(REGISTER_A - 1)
#else
#define OP_DEC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_A = MAKE_UINT_16(REGISTER_A - 1);						\
			FLAG_N = NFLAG_16(REGISTER_A)
#endif

/* M6502   Decrement operand */
#undef OP_DECM
#if FLAG_SET_M
#define OP_DECM(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_N = FLAG_Z = MAKE_UINT_8(read_8_##MODE(DST) - 1);			\
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_DECM(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_Z = MAKE_UINT_16(read_16_##MODE(DST) - 1);					\
			FLAG_N = NFLAG_16(FLAG_Z);										\
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M6502   Decrement index register */
#undef OP_DECX
#if FLAG_SET_X
#define OP_DECX(REG)														\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_N = FLAG_Z = REG = MAKE_UINT_8(REG - 1)
#else
#define OP_DECX(REG)														\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REG = MAKE_UINT_16(REG - 1);							\
			FLAG_N = NFLAG_16(REG)
#endif

/* M6502   Exclusive Or operand to accumulator */
#undef OP_EOR
#if FLAG_SET_M
#define OP_EOR(MODE)														\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_N = FLAG_Z = REGISTER_A ^= OPER_8_##MODE()
#else
#define OP_EOR(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_Z = REGISTER_A ^= OPER_16_##MODE();								\
			FLAG_N = NFLAG_16(REGISTER_A)
#endif

/* M6502   Increment accumulator */
#undef OP_INC
#if FLAG_SET_M
#define OP_INC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_8(REGISTER_A + 1)
#else
#define OP_INC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_A = MAKE_UINT_16(REGISTER_A + 1);						\
			FLAG_N = NFLAG_16(REGISTER_A)
#endif

/* M6502   Increment operand */
#undef OP_INCM
#if FLAG_SET_M
#define OP_INCM(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_N = FLAG_Z = MAKE_UINT_8(read_8_##MODE(DST) + 1);			\
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_INCM(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_Z = MAKE_UINT_16(read_16_##MODE(DST) + 1);					\
			FLAG_N = NFLAG_16(FLAG_Z);										\
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M6502   Increment index register */
#undef OP_INCX
#if FLAG_SET_X
#define OP_INCX(REG)														\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_N = FLAG_Z = REG = MAKE_UINT_8(REG + 1)
#else
#define OP_INCX(REG)														\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REG = MAKE_UINT_16(REG + 1);							\
			FLAG_N = NFLAG_16(REG)
#endif

/* G65816  Jump Long */
#undef OP_JMLAI
#define OP_JMLAI()															\
			CLK(CLK_OP + CLK_AI + 1);										\
			g65816i_jump_24(read_24_A(OPER_16_IMM()))

/* M6502   Jump */
#undef OP_JMP
#define OP_JMP(MODE)														\
			CLK(CLK_OP + CLK_##MODE);										\
			g65816i_jump_16(EA_##MODE())

/* M6502   Jump absolute indexed indirect */
#undef OP_JMPAXI
#define OP_JMPAXI()															\
			CLK(CLK_OP + CLK_AXI);											\
			g65816i_jump_16(read_16_AXI(REGISTER_PB | (MAKE_UINT_16(OPER_16_IMM() + REGISTER_X))))

/* G65816  Jump absolute long */
#undef OP_JMPAL
#define OP_JMPAL()															\
			CLK(CLK_OP + CLK_AL);											\
			g65816i_jump_24(EA_AL())

/* G65816  Jump to Subroutine Long */
/* Unusual behavior: stacks PC-1 */
#undef OP_JSL
#define OP_JSL(MODE)														\
			CLK(CLK_OP + CLK_W24 + CLK_##MODE + 1);							\
			DST = EA_##MODE();												\
			g65816i_push_8(REGISTER_PB>>16);										\
			g65816i_push_16(REGISTER_PC-1);										\
			g65816i_jump_24(DST)

/* M6502   Jump to Subroutine */
/* Unusual behavior: stacks PC-1 */
#undef OP_JSR
#define OP_JSR(MODE)														\
			CLK(CLK_OP + CLK_W16 + CLK_##MODE);								\
			DST = EA_##MODE();												\
			g65816i_push_16(REGISTER_PC-1);										\
			g65816i_jump_16(DST)

/* M6502   Jump to Subroutine */
/* Unusual behavior: stacks PC-1 */
#undef OP_JSRAXI
#define OP_JSRAXI()															\
			CLK(CLK_OP + CLK_W16 + CLK_AXI);								\
			DST = read_16_AXI(REGISTER_PB | (MAKE_UINT_16(OPER_16_IMM() + REGISTER_X))); \
			g65816i_push_16(REGISTER_PC-1);										\
			g65816i_jump_16(DST)

/* M6502   Load accumulator with operand */
#undef OP_LDA
#if FLAG_SET_M
#define OP_LDA(MODE)														\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_N = FLAG_Z = REGISTER_A = OPER_8_##MODE()
#else
#define OP_LDA(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_Z = REGISTER_A = OPER_16_##MODE();								\
			FLAG_N = NFLAG_16(REGISTER_A)
#endif

/* M6502   Load index register with operand */
#undef OP_LDX
#if FLAG_SET_X
#define OP_LDX(REG, MODE)													\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_N = FLAG_Z = REG = OPER_8_##MODE()
#else
#define OP_LDX(REG, MODE)													\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_Z = REG = OPER_16_##MODE();								\
			FLAG_N = NFLAG_16(REG)
#endif

/* M6502   Logical Shift Right accumulator */
#undef OP_LSR
#if FLAG_SET_M
#define OP_LSR()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_N = 0;														\
			FLAG_C = REGISTER_A << 8;											\
			FLAG_Z = REGISTER_A >>= 1
#else
#define OP_LSR()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_N = 0;														\
			FLAG_C = REGISTER_A << 8;											\
			FLAG_Z = REGISTER_A >>= 1
#endif

/* M6502   Logical Shift Right operand */
#undef OP_LSRM
#if FLAG_SET_M
#define OP_LSRM(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_N = 0;														\
			FLAG_Z = read_8_##MODE(DST);									\
			FLAG_C = FLAG_Z << 8;											\
			FLAG_Z >>= 1;													\
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_LSRM(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_N = 0;														\
			FLAG_Z = read_16_##MODE(DST);									\
			FLAG_C = FLAG_Z << 8;											\
			FLAG_Z >>= 1;													\
			write_16_##MODE(DST, FLAG_Z)
#endif

/* G65816  Move Block Negative */
#undef OP_MVN
#if FLAG_SET_M
#if FLAG_SET_X
#define OP_MVN()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_8(REGISTER_X+1);								\
			REGISTER_Y = MAKE_UINT_8(REGISTER_Y+1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xff) != 0xff) \
			{\
				REGISTER_PC -= 3; \
			}\
			else \
			{ \
				REGISTER_B -= 0x100;\
				if ((REGISTER_B & 0xff00) != 0xff00)\
				{			  \
					REGISTER_PC -= 3;\
				}	  \
			}
#else
#define OP_MVN()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_16(REGISTER_X+1);								\
			REGISTER_Y = MAKE_UINT_16(REGISTER_Y+1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xff) != 0xff) \
			{\
				REGISTER_PC -= 3; \
			}\
			else \
			{ \
				REGISTER_B -= 0x100;\
				if ((REGISTER_B & 0xff00) != 0xff00)\
				{			  \
					REGISTER_PC -= 3;\
				}	  \
			}
#endif
#else
#if FLAG_SET_X
#define OP_MVN()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			REGISTER_A |= REGISTER_B;													\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_8(REGISTER_X+1);								\
			REGISTER_Y = MAKE_UINT_8(REGISTER_Y+1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xffff) != 0xffff) \
			{\
				REGISTER_PC -= 3; \
			}
#else
#define OP_MVN()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			REGISTER_A |= REGISTER_B;													\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_16(REGISTER_X+1);								\
			REGISTER_Y = MAKE_UINT_16(REGISTER_Y+1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xffff) != 0xffff) \
			{\
				REGISTER_PC -= 3; \
			}
#endif
#endif

/* G65816  Move Block Positive */
#undef OP_MVP
#if FLAG_SET_M
#if FLAG_SET_X
#define OP_MVP()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_8(REGISTER_X-1);								\
			REGISTER_Y = MAKE_UINT_8(REGISTER_Y-1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xff) != 0xff) \
			{\
				REGISTER_PC -= 3; \
			}\
			else \
			{ \
				REGISTER_B -= 0x100;\
				if ((REGISTER_B & 0xff00) != 0xff00)\
				{			  \
					REGISTER_PC -= 3;\
				}	  \
			}
#else
#define OP_MVP()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_16(REGISTER_X-1);								\
			REGISTER_Y = MAKE_UINT_16(REGISTER_Y-1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xff) != 0xff) \
			{\
				REGISTER_PC -= 3; \
			}\
			else \
			{ \
				REGISTER_B -= 0x100;\
				if ((REGISTER_B & 0xff00) != 0xff00)\
				{			  \
					REGISTER_PC -= 3;\
				}	  \
			}
#endif
#else
#if FLAG_SET_X
#define OP_MVP()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			REGISTER_A |= REGISTER_B;													\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_8(REGISTER_X-1);								\
			REGISTER_Y = MAKE_UINT_8(REGISTER_Y-1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xffff) != 0xffff) \
			{\
				REGISTER_PC -= 3; \
			}
#else
#define OP_MVP()															\
			DST = OPER_8_IMM()<<16;											\
			SRC = OPER_8_IMM()<<16;											\
			REGISTER_DB = DST;								\
			REGISTER_A |= REGISTER_B;													\
			CLK(7);												\
			write_8_NORM(DST | REGISTER_Y, read_8_NORM(SRC | REGISTER_X));		\
			REGISTER_X = MAKE_UINT_16(REGISTER_X-1);								\
			REGISTER_Y = MAKE_UINT_16(REGISTER_Y-1);								\
			REGISTER_A--;								\
			if ((REGISTER_A&0xffff) != 0xffff) \
			{\
				REGISTER_PC -= 3; \
			}
#endif
#endif

/* M6502   No Operation */
#undef OP_NOP
#define OP_NOP()															\
			CLK(CLK_OP + CLK_IMPLIED)

/* M6502   Logical OR operand to accumulator */
#undef OP_ORA
#if FLAG_SET_M
#define OP_ORA(MODE)														\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			FLAG_N = FLAG_Z = REGISTER_A |= OPER_8_ ## MODE()
#else
#define OP_ORA(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			FLAG_Z = REGISTER_A |= OPER_16_##MODE();								\
			FLAG_N = NFLAG_16(REGISTER_A)
#endif

/* G65816  Push Effective Address */
#undef OP_PEA
#define OP_PEA()															\
			CLK(CLK_OP + CLK_R16 + CLK_W16);								\
			g65816i_push_16(OPER_16_IMM())

/* G65816  Push Effective Indirect Address */
#undef OP_PEI
#define OP_PEI()															\
			CLK(CLK_OP + CLK_R16 + CLK_W16 + CLK_D);						\
			g65816i_push_16(EA_DI())

/* G65816  Push Effective PC-Relative Address */
#undef OP_PER
#define OP_PER()															\
			CLK(CLK_OP + CLK_R16 + CLK_W16 + 1);							\
			SRC = OPER_16_IMM();											\
			g65816i_push_16(REGISTER_PC + SRC)

/* M6502   Push accumulator to the stack */
#undef OP_PHA
#if FLAG_SET_M
#define OP_PHA()															\
			CLK(CLK_OP + CLK_W8 + 1);										\
			g65816i_push_8(REGISTER_A)
#else
#define OP_PHA()															\
			CLK(CLK_OP + CLK_W16 + 1);										\
			g65816i_push_16(REGISTER_A)
#endif

/* M6502   Push index register to the stack */
#undef OP_PHX
#if FLAG_SET_X
#define OP_PHX(REG)															\
			CLK(CLK_OP + CLK_W8 + 1);										\
			g65816i_push_8(REG)
#else
#define OP_PHX(REG)															\
			CLK(CLK_OP + CLK_W16 + 1);										\
			g65816i_push_16(REG)
#endif

/* G65816  Push data bank register */
#undef OP_PHB
#define OP_PHB()															\
			CLK(CLK_OP + CLK_W8 + 1);										\
			g65816i_push_8(REGISTER_DB>>16)

/* G65816  Push direct register */
#undef OP_PHD
#define OP_PHD()															\
			CLK(CLK_OP + CLK_W16 + 1);										\
			g65816i_push_16(REGISTER_D)

/* G65816  Push program bank register */
#undef OP_PHK
#define OP_PHK()															\
			CLK(CLK_OP + CLK_W8 + 1);										\
			g65816i_push_8(REGISTER_PB>>16)

/* M6502   Push the Processor Status Register to the stack */
#undef OP_PHP
#define OP_PHP()															\
			CLK(CLK_OP + CLK_W8 + 1);										\
			g65816i_push_8(g65816i_get_reg_p())

/* M6502   Pull accumulator from the stack */
#undef OP_PLA
#if FLAG_SET_M
#define OP_PLA()															\
			CLK(CLK_OP + CLK_R8 + 2);										\
			FLAG_N = FLAG_Z = REGISTER_A = g65816i_pull_8()
#else
#define OP_PLA()															\
			CLK(CLK_OP + CLK_R16 + 2);										\
			FLAG_Z = REGISTER_A = g65816i_pull_16();								\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M6502   Pull index register from the stack */
#undef OP_PLX
#if FLAG_SET_X
#define OP_PLX(REG)															\
			CLK(CLK_OP + CLK_R8 + 2);										\
			FLAG_N = FLAG_Z = REG = g65816i_pull_8()
#else
#define OP_PLX(REG)															\
			CLK(CLK_OP + CLK_R16 + 2);										\
			FLAG_Z = REG = g65816i_pull_16();								\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* G65816  Pull data bank register */
#undef OP_PLB
#define OP_PLB()															\
			CLK(CLK_OP + CLK_R8 + 2);										\
			FLAG_N = FLAG_Z = g65816i_pull_8();								\
			REGISTER_DB = FLAG_Z << 16

/* G65816  Pull direct register */
#undef OP_PLD
#define OP_PLD()															\
			CLK(CLK_OP + CLK_R16 + 2);										\
			FLAG_Z = REGISTER_D = g65816i_pull_16();								\
			FLAG_N = NFLAG_16(FLAG_Z)

/* M6502   Pull the Processor Status Register from the stack */
#undef OP_PLP
#define OP_PLP()															\
			CLK(CLK_OP + CLK_R8 + 2);										\
			g65816i_set_reg_p(g65816i_pull_8())

/* G65816  Reset Program status word */
#undef OP_REP
#define OP_REP()															\
			CLK(CLK_OP + CLK_R8 + 1);										\
			g65816i_set_reg_p(g65816i_get_reg_p() & ~OPER_8_IMM())

/* M6502   Rotate Left the accumulator */
#undef OP_ROL
#if FLAG_SET_M
#define OP_ROL()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_C = (REGISTER_A<<1) | CFLAG_AS_1();								\
			FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_8(FLAG_C)
#else
#define OP_ROL()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_C = (REGISTER_A<<1) | CFLAG_AS_1();								\
			FLAG_Z = REGISTER_A = MAKE_UINT_16(FLAG_C);							\
			FLAG_N = NFLAG_16(FLAG_C);										\
			FLAG_C = CFLAG_16(FLAG_C)
#endif

/* M6502   Rotate Left an operand */
#undef OP_ROLM
#if FLAG_SET_M
#define OP_ROLM(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST = EA_##MODE();												\
			FLAG_C = (read_8_##MODE(DST)<<1) | CFLAG_AS_1();				\
			FLAG_N = FLAG_Z = MAKE_UINT_8(FLAG_C);							\
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_ROLM(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST = EA_##MODE();												\
			FLAG_C = (read_16_##MODE(DST)<<1) | CFLAG_AS_1();				\
			FLAG_Z = MAKE_UINT_16(FLAG_C);									\
			FLAG_N = NFLAG_16(FLAG_C);										\
			FLAG_C = CFLAG_16(FLAG_C);										\
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M6502   Rotate Right the accumulator */
#undef OP_ROR
#if FLAG_SET_M
#define OP_ROR()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			REGISTER_A |= FLAG_C & 0x100;										\
			FLAG_C = REGISTER_A << 8;											\
			FLAG_N = FLAG_Z = REGISTER_A >>= 1
#else
#define OP_ROR()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			REGISTER_A |= (FLAG_C<<8) & 0x10000;									\
			FLAG_C = REGISTER_A << 8;											\
			FLAG_Z = REGISTER_A >>= 1;											\
			FLAG_N = NFLAG_16(REGISTER_A)
#endif

/* M6502   Rotate Right an operand */
#undef OP_RORM
#if FLAG_SET_M
#define OP_RORM(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST = EA_##MODE();												\
			FLAG_Z = read_8_##MODE(DST) | (FLAG_C & 0x100);					\
			FLAG_C = FLAG_Z << 8;											\
			FLAG_N = FLAG_Z >>= 1;											\
			write_8_##MODE(DST, FLAG_Z)
#else
#define OP_RORM(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST = EA_##MODE();												\
			FLAG_Z = read_16_##MODE(DST) | ((FLAG_C<<8) & 0x10000);			\
			FLAG_C = FLAG_Z << 8;											\
			FLAG_Z >>= 1;													\
			FLAG_N = NFLAG_16(FLAG_Z);										\
			write_16_##MODE(DST, FLAG_Z)
#endif

/* M6502   Return from Interrupt */
#undef OP_RTI
#if FLAG_SET_E
#define OP_RTI()															\
			CLK(7);															\
			g65816i_set_reg_p(g65816i_pull_8());							\
			g65816i_jump_16(g65816i_pull_16())
#else
#define OP_RTI()															\
			CLK(8);															\
			g65816i_set_reg_p(g65816i_pull_8());							\
			g65816i_jump_16(g65816i_pull_16());								\
			REGISTER_PB = g65816i_pull_8() << 16
#endif

/* G65816  Return from Subroutine Long */
/* Unusual behavior: Gets PC and increments */
#undef OP_RTL
#define OP_RTL()															\
			CLK(6);															\
			g65816i_jump_24(g65816i_pull_24()+1)

/* M6502   Return from Subroutine */
/* Unusual behavior: Gets PC and increments */
#undef OP_RTS
#define OP_RTS()															\
			CLK(6);															\
			g65816i_jump_16(g65816i_pull_16()+1)

/* M6502   Subtract with Carry */
/* Unusual behavior: C flag is inverted */
#undef OP_SBC
#if FLAG_SET_M
#define OP_SBC(MODE)														\
			CLK(CLK_OP + CLK_R8 + CLK_##MODE);								\
			SRC = OPER_8_##MODE();											\
			FLAG_C = ~FLAG_C;												\
			if(!FLAG_D)														\
			{																\
				FLAG_C = REGISTER_A - SRC - CFLAG_AS_1();					\
				FLAG_V = VFLAG_SUB_8(SRC, REGISTER_A, FLAG_C);				\
				FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_8(FLAG_C);			\
				FLAG_C = ~FLAG_C;											\
				BREAKOUT;													\
			}																\
			else															\
			{																\
				unsigned tmp16;												\
				signed tmp1, tmp2, tmpA, tmpB;							\
				DST = CFLAG_AS_1();											\
				tmp1 = REGISTER_A & 0x0F;									\
				tmp2 = (REGISTER_A >> 4) & 0x0F;							\
				tmpA = SRC&0x0F;											\
				tmpB = (SRC >> 4) & 0x0F;									\
				tmp1 = tmp1 - tmpA - DST;									\
				tmp2 = tmp2 - tmpB;											\
				FLAG_C = CFLAG_CLEAR;										\
				if(tmp1 < 0)												\
					tmp2--;													\
				if((tmp1 > 0x09) || (tmp1 < 0 ))							\
				{															\
					tmp1 = ((tmp1 + 10) & 0x0F);							\
				}															\
				if(!(tmp2 < 0))												\
					FLAG_C = CFLAG_SET;										\
				if((tmp2 > 0x09) || (tmp2 < 0))								\
				{															\
						tmp2=((tmp2 + 10) & 0x0F);							\
				}															\
				tmp16 = tmp1 | (tmp2 << 4);									\
				FLAG_V = VFLAG_SUB_8(SRC, REGISTER_A, tmp16);				\
				FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_8(tmp16);			\
			}
#else
#define OP_SBC(MODE)														\
			CLK(CLK_OP + CLK_R16 + CLK_##MODE);								\
			SRC = OPER_16_##MODE();											\
			FLAG_C = ~FLAG_C;												\
			if(!FLAG_D)														\
			{																\
				FLAG_C = REGISTER_A - SRC - CFLAG_AS_1();					\
				FLAG_V = VFLAG_SUB_16(SRC, REGISTER_A, FLAG_C);				\
				FLAG_Z = REGISTER_A = MAKE_UINT_16(FLAG_C);					\
				FLAG_N = NFLAG_16(REGISTER_A);								\
				FLAG_C = ~CFLAG_16(FLAG_C);									\
				BREAKOUT;													\
			}																\
			else															\
			{																\
				unsigned tmp16;												\
				signed tmp1,tmp2,tmp3, tmp4, tmpA, tmpB, tmpC, tmpD;		\
				DST = CFLAG_AS_1();											\
				tmp1 = REGISTER_A & 0x0F;									\
				tmp2 = (REGISTER_A >> 4) & 0x0F;							\
				tmp3 = (REGISTER_A >> 8) & 0x0F;							\
				tmp4 = (REGISTER_A >> 12) & 0x0F;							\
				tmpA = SRC & 0x0F;											\
				tmpB = (SRC >> 4) & 0x0F;									\
				tmpC = (SRC >> 8) & 0x0F;									\
				tmpD = (SRC >> 12) & 0x0F;									\
				tmp1 = tmp1 - tmpA - CFLAG_AS_1();							\
				tmp2 = tmp2 - tmpB;											\
				tmp3 = tmp3 - tmpC;											\
				tmp4 = tmp4 - tmpD;											\
				FLAG_C = CFLAG_CLEAR;										\
				if(tmp1 < 0)												\
					tmp2--;													\
				if((tmp1 > 0x09) || (tmp1 < 0 ))							\
				{															\
					tmp1 = ((tmp1 + 10) & 0x0F);							\
				}															\
				if(tmp2 < 0)												\
					tmp3--;													\
				if((tmp2 > 0x09) || (tmp2 < 0))								\
				{															\
						tmp2=((tmp2 + 10) & 0x0F);							\
				}															\
				if(tmp3 < 0)												\
					tmp4--;													\
				if((tmp3 > 0x09) || (tmp3 < 0 ))							\
				{															\
					tmp3 = ((tmp3 + 10) & 0x0F);							\
				}															\
				if(!(tmp4 < 0))												\
					FLAG_C = CFLAG_SET;										\
				if((tmp4 > 0x09) || (tmp4 < 0))								\
				{															\
					tmp4=((tmp4 + 10) & 0x0F);								\
				}															\
				tmp16 = tmp1 | (tmp2 << 4) | (tmp3 << 8) | (tmp4 << 12);	\
				FLAG_V = VFLAG_SUB_16(SRC, REGISTER_A, tmp16);				\
				FLAG_N = FLAG_Z = REGISTER_A = MAKE_UINT_16(tmp16);			\
			}
#endif


/* M6502   Set Carry flag */
#undef OP_SEC
#define OP_SEC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_C = CFLAG_SET

/* M6502   Set Decimal flag */
#undef OP_SED
#define OP_SED()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_D = DFLAG_SET

/* M6502   Set Interrupt Mask flag */
#undef OP_SEI
#define OP_SEI()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			g65816i_set_flag_i(IFLAG_SET)

/* G65816  Set Program status word */
#undef OP_SEP
#define OP_SEP()															\
			CLK(CLK_OP + CLK_R8 + 1);										\
			g65816i_set_reg_p(g65816i_get_reg_p() | OPER_8_IMM())

/* M6502   Store accumulator to memory */
#undef OP_STA
#if FLAG_SET_M
#define OP_STA(MODE)														\
			CLK(CLK_OP + CLK_W8 + CLK_W_##MODE);								\
			write_8_##MODE(EA_##MODE(), REGISTER_A)
#else
#define OP_STA(MODE)														\
			CLK(CLK_OP + CLK_W16 + CLK_W_##MODE);								\
			write_16_##MODE(EA_##MODE(), REGISTER_A)
#endif

/* M6502   Store index register to memory */
#undef OP_STX
#if FLAG_SET_X
#define OP_STX(REG, MODE)													\
			CLK(CLK_OP + CLK_W8 + CLK_W_##MODE);								\
			write_8_##MODE(EA_##MODE(), REG)
#else
#define OP_STX(REG, MODE)													\
			CLK(CLK_OP + CLK_W16 + CLK_W_##MODE);								\
			write_16_##MODE(EA_##MODE(), REG)
#endif

/* M6502   Store zero to memory */
#undef OP_STZ
#if FLAG_SET_M
#define OP_STZ(MODE)														\
			CLK(CLK_OP + CLK_W8 + CLK_W_##MODE);								\
			write_8_##MODE(EA_##MODE(), 0)
#else
#define OP_STZ(MODE)														\
			CLK(CLK_OP + CLK_W16 + CLK_W_##MODE);								\
			write_16_##MODE(EA_##MODE(), 0)
#endif

/* G65816  Stop the clock */
#undef OP_STP
#define OP_STP()															\
			USE_ALL_CLKS();													\
			CPU_STOPPED |= STOP_LEVEL_STOP

/* M6502   Transfer accumulator to index */
#undef OP_TAX
#if FLAG_SET_M
#if FLAG_SET_X
#define OP_TAX(REG)															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REG = REGISTER_A;											\
			FLAG_N = NFLAG_8(FLAG_Z)
#else /* FLAG_SET_X */
#define OP_TAX(REG)															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REG = REGISTER_B | REGISTER_A;									\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif /* FLAG_SET_X */
#else /* FLAG_SET_M */
#if FLAG_SET_X
#define OP_TAX(REG)															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REG = MAKE_UINT_8(REGISTER_A);								\
			FLAG_N = NFLAG_8(FLAG_Z)
#else /* FLAG_SET_X */
#define OP_TAX(REG)															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REG = REGISTER_A;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif /* FLAG_SET_X */
#endif /* FLAG_SET_M */


/* M6502   Transfer index to accumulator */
#undef OP_TXA
#if FLAG_SET_M
#define OP_TXA(REG)															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_A = MAKE_UINT_8(REG);								\
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TXA(REG)															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_A = REG;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* G65816  Transfer C to direct register */
#undef OP_TCD
#if FLAG_SET_M
#define OP_TCD()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_D = REGISTER_A | REGISTER_B;									\
			FLAG_N = NFLAG_16(FLAG_Z)
#else
#define OP_TCD()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_D = REGISTER_A;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* G65816  Transfer direct register to C */
#undef OP_TDC
#if FLAG_SET_M
#define OP_TDC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_D;													\
			FLAG_N = NFLAG_16(FLAG_Z);										\
			REGISTER_A = MAKE_UINT_8(REGISTER_D);										\
			REGISTER_B = REGISTER_D & 0xff00
#else
#define OP_TDC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_A = REGISTER_D;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* G65816  Transfer C to stack pointer */
#undef OP_TCS
#if FLAG_SET_E
#define OP_TCS()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			REGISTER_S = MAKE_UINT_8(REGISTER_A) | 0x100
#else
#define OP_TCS()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			REGISTER_S = REGISTER_A | REGISTER_B
#endif

/* G65816  Transfer stack pointer to C */
#undef OP_TSC
#if FLAG_SET_M
#define OP_TSC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_S;													\
			FLAG_N = NFLAG_16(FLAG_Z);										\
			REGISTER_A = MAKE_UINT_8(REGISTER_S);										\
			REGISTER_B = REGISTER_S & 0xff00
#else
#define OP_TSC()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_A = REGISTER_S;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M6502   Transfer stack pointer to X */
#undef OP_TSX
#if FLAG_SET_X
#define OP_TSX()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_X = MAKE_UINT_8(REGISTER_S);							\
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TSX()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_X = REGISTER_S;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* M6502   Transfer X to stack pointer */
#undef OP_TXS
#if FLAG_SET_E
#define OP_TXS()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			REGISTER_S = MAKE_UINT_8(REGISTER_X) | 0x100
#else
#define OP_TXS()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			REGISTER_S = REGISTER_X
#endif

/* G65816  Transfer X to Y */
#undef OP_TXY
#if FLAG_SET_X
#define OP_TXY()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_Y = REGISTER_X;											\
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TXY()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_Y = REGISTER_X;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* G65816  Transfer Y to X */
#undef OP_TYX
#if FLAG_SET_X
#define OP_TYX()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_X = REGISTER_Y;											\
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_TYX()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_X = REGISTER_Y;											\
			FLAG_N = NFLAG_16(FLAG_Z)
#endif

/* G65816  Test and reset bit */
#undef OP_TRB
#if FLAG_SET_M
#define OP_TRB(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_Z = read_8_##MODE(DST);									\
			write_8_##MODE(DST, FLAG_Z & ~REGISTER_A);							\
			FLAG_Z &= REGISTER_A
#else
#define OP_TRB(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_Z = read_16_##MODE(DST);									\
			write_16_##MODE(DST, FLAG_Z & ~REGISTER_A);							\
			FLAG_Z &= REGISTER_A
#endif

/* G65816  Test and set bit */
#undef OP_TSB
#if FLAG_SET_M
#define OP_TSB(MODE)														\
			CLK(CLK_OP + CLK_RMW8 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_Z = read_8_##MODE(DST);									\
			write_8_##MODE(DST, FLAG_Z | REGISTER_A);							\
			FLAG_Z &= REGISTER_A
#else
#define OP_TSB(MODE)														\
			CLK(CLK_OP + CLK_RMW16 + CLK_W_##MODE);							\
			DST    = EA_##MODE();											\
			FLAG_Z = read_16_##MODE(DST);									\
			write_16_##MODE(DST, FLAG_Z | REGISTER_A);							\
			FLAG_Z &= REGISTER_A
#endif

/* G65816  Wait for interrupt */
#undef OP_WAI
#define OP_WAI()															\
			USE_ALL_CLKS();													\
			CPU_STOPPED |= STOP_LEVEL_WAI

/* G65816  William D. Mensch, JR. (65816 designer) - future expansion */
#undef OP_WDM
#define OP_WDM()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			REGISTER_PC++

/* G65816  Exchange accum high and low bytes */
#undef OP_XBA
#if FLAG_SET_M
#define OP_XBA()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			FLAG_Z = REGISTER_B>>8;												\
			REGISTER_B = REGISTER_A<<8;												\
			REGISTER_A = FLAG_Z;													\
			FLAG_N = NFLAG_8(FLAG_Z)
#else
#define OP_XBA()															\
			CLK(CLK_OP + CLK_IMPLIED + 1);									\
			FLAG_Z = REGISTER_A >> 8;											\
			REGISTER_A = MAKE_UINT_16(REGISTER_A<<8) | FLAG_Z;						\
			FLAG_N = NFLAG_8(FLAG_Z)
#endif

/* G65816  Exchange carry and emulation bits */
#undef OP_XCE
#define OP_XCE()															\
			CLK(CLK_OP + CLK_IMPLIED);										\
			SRC = CFLAG_AS_1();												\
			FLAG_C = FLAG_E<<8;												\
			g65816i_set_flag_e(SRC)




/* ======================================================================== */
/* ======================== OPCODE & FUNCTION TABLES ====================== */
/* ======================================================================== */

#undef OP
#undef O
#undef TABLE_OPCODES
#undef TABLE_FUNCTION

#if FLAG_SET_E
#define OP(CODE, OPERATION) static void g65816i_ ## CODE ## _E(void) {OPERATION;}
#define O(CODE) g65816i_ ## CODE ## _E
#define TABLE_OPCODES void (*g65816i_opcodes_E[256])(void)
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)	RTYPE g65816i_ ## NAME ## _E ARGS

#else

#if !FLAG_SET_M && !FLAG_SET_X
#define OP(CODE, OPERATION) static void g65816i_ ## CODE ## _M0X0(void) {OPERATION;}
#define O(CODE) g65816i_ ## CODE ## _M0X0
#define TABLE_OPCODES void (*g65816i_opcodes_M0X0[256])(void)
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)	RTYPE g65816i_ ## NAME ## _M0X0 ARGS

#elif !FLAG_SET_M && FLAG_SET_X

#define OP(CODE, OPERATION) static void g65816i_ ## CODE ## _M0X1(void) {OPERATION;}
#define O(CODE) g65816i_ ## CODE ## _M0X1
#define TABLE_OPCODES void (*g65816i_opcodes_M0X1[256])(void)
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)	RTYPE g65816i_ ## NAME ## _M0X1 ARGS

#elif FLAG_SET_M && !FLAG_SET_X

#define OP(CODE, OPERATION) static void g65816i_ ## CODE ## _M1X0(void) {OPERATION;}
#define O(CODE) g65816i_ ## CODE ## _M1X0
#define TABLE_OPCODES void (*g65816i_opcodes_M1X0[256])(void)
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)	RTYPE g65816i_ ## NAME ## _M1X0 ARGS

#elif FLAG_SET_M && FLAG_SET_X

#define OP(CODE, OPERATION) static void g65816i_ ## CODE ## _M1X1(void) {OPERATION;}
#define O(CODE) g65816i_ ## CODE ## _M1X1
#define TABLE_OPCODES void (*g65816i_opcodes_M1X1[256])(void)
#define TABLE_FUNCTION(RTYPE, NAME, ARGS)	RTYPE g65816i_ ## NAME ## _M1X1 ARGS

#endif
#endif
#define BREAKOUT return



/* OP  FUNCTION                     Comment     */
OP(00, OP_BRK  (             ) ) /* BRK         */
OP(01, OP_ORA  ( DXI         ) ) /* ORA dxi     */
OP(02, OP_COP  (             ) ) /* COP     (G) */
OP(03, OP_ORA  ( S           ) ) /* ORA s   (G) */
OP(04, OP_TSB  ( D           ) ) /* TSB d   (C) */
OP(05, OP_ORA  ( D           ) ) /* ORA d       */
OP(06, OP_ASLM ( D           ) ) /* ASL d       */
OP(07, OP_ORA  ( DLI         ) ) /* ORA dli (G) */
OP(08, OP_PHP  (             ) ) /* PHP         */
OP(09, OP_ORA  ( IMM         ) ) /* ORA imm     */
OP(0a, OP_ASL  (             ) ) /* ASL acc     */
OP(0b, OP_PHD  (             ) ) /* PHD     (G) */
OP(0c, OP_TSB  ( A           ) ) /* TSB a   (C) */
OP(0d, OP_ORA  ( A           ) ) /* ORA a       */
OP(0e, OP_ASLM ( A           ) ) /* ASL a       */
OP(0f, OP_ORA  ( AL          ) ) /* ORA al  (G) */
OP(10, OP_BCC  ( COND_PL()   ) ) /* BPL         */
OP(11, OP_ORA  ( DIY         ) ) /* ORA diy     */
OP(12, OP_ORA  ( DI          ) ) /* ORA di  (C) */
OP(13, OP_ORA  ( SIY         ) ) /* ORA siy (G) */
OP(14, OP_TRB  ( D           ) ) /* TRB d   (C) */
OP(15, OP_ORA  ( DX          ) ) /* ORA dx      */
OP(16, OP_ASLM ( DX          ) ) /* ASL dx      */
OP(17, OP_ORA  ( DLIY        ) ) /* ORA dliy(C) */
OP(18, OP_CLC  (             ) ) /* CLC         */
OP(19, OP_ORA  ( AY          ) ) /* ORA ay      */
OP(1a, OP_INC  (             ) ) /* INA     (C) */
OP(1b, OP_TCS  (             ) ) /* TCS     (G) */
OP(1c, OP_TRB  ( A           ) ) /* TRB a   (C) */
OP(1d, OP_ORA  ( AX          ) ) /* ORA ax      */
OP(1e, OP_ASLM ( AX          ) ) /* ASL ax      */
OP(1f, OP_ORA  ( ALX         ) ) /* ORA alx (G) */
OP(20, OP_JSR  ( A           ) ) /* JSR a       */
OP(21, OP_AND  ( DXI         ) ) /* AND dxi     */
OP(22, OP_JSL  ( AL          ) ) /* JSL al  (G) */
OP(23, OP_AND  ( S           ) ) /* AND s   (G) */
OP(24, OP_BIT  ( D           ) ) /* BIT d       */
OP(25, OP_AND  ( D           ) ) /* AND d       */
OP(26, OP_ROLM ( D           ) ) /* ROL d       */
OP(27, OP_AND  ( DLI         ) ) /* AND dli (G) */
OP(28, OP_PLP  (             ) ) /* PLP         */
OP(29, OP_AND  ( IMM         ) ) /* AND imm     */
OP(2a, OP_ROL  (             ) ) /* ROL acc     */
OP(2b, OP_PLD  (             ) ) /* PLD     (G) */
OP(2c, OP_BIT  ( A           ) ) /* BIT a       */
OP(2d, OP_AND  ( A           ) ) /* AND a       */
OP(2e, OP_ROLM ( A           ) ) /* ROL a       */
OP(2f, OP_AND  ( AL          ) ) /* AND al  (G) */
OP(30, OP_BCC  ( COND_MI()   ) ) /* BMI         */
OP(31, OP_AND  ( DIY         ) ) /* AND diy     */
OP(32, OP_AND  ( DI          ) ) /* AND di  (C) */
OP(33, OP_AND  ( SIY         ) ) /* AND siy     */
OP(34, OP_BIT  ( DX          ) ) /* BIT dx  (C) */
OP(35, OP_AND  ( DX          ) ) /* AND dx      */
OP(36, OP_ROLM ( DX          ) ) /* ROL dx      */
OP(37, OP_AND  ( DLIY        ) ) /* AND dliy(G) */
OP(38, OP_SEC  (             ) ) /* SEC         */
OP(39, OP_AND  ( AY          ) ) /* AND ay      */
OP(3a, OP_DEC  (             ) ) /* DEA     (C) */
OP(3b, OP_TSC  (             ) ) /* TSC     (G) */
OP(3c, OP_BIT  ( AX          ) ) /* BIT abx (C) */
OP(3d, OP_AND  ( AX          ) ) /* AND ax      */
OP(3e, OP_ROLM ( AX          ) ) /* ROL ax      */
OP(3f, OP_AND  ( ALX         ) ) /* AND alx (G) */
OP(40, OP_RTI  (             ) ) /* RTI         */
OP(41, OP_EOR  ( DXI         ) ) /* EOR dxi     */
OP(42, OP_WDM  (             ) ) /* WDM         */
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
OP(5a, OP_PHX  ( REGISTER_Y       ) ) /* PHY     (C) */
OP(5b, OP_TCD  (             ) ) /* TCD     (G) */
OP(5c, OP_JMPAL(             ) ) /* JMP al  (G) */
OP(5d, OP_EOR  ( AX          ) ) /* EOR ax      */
OP(5e, OP_LSRM ( AX          ) ) /* LSR ax      */
OP(5f, OP_EOR  ( ALX         ) ) /* EOR alx (G) */
OP(60, OP_RTS  (             ) ) /* RTS         */
OP(61, OP_ADC  ( DXI         ) ) /* ADC dxi     */
OP(62, OP_PER  (             ) ) /* PER     (G) */
OP(63, OP_ADC  ( S           ) ) /* ADC s   (G) */
OP(64, OP_STZ  ( D           ) ) /* STZ d   (C) */
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
OP(74, OP_STZ  ( DX          ) ) /* STZ dx  (C) */
OP(75, OP_ADC  ( DX          ) ) /* ADC dx      */
OP(76, OP_RORM ( DX          ) ) /* ROR dx      */
OP(77, OP_ADC  ( DLIY        ) ) /* ADC dliy(G) */
OP(78, OP_SEI  (             ) ) /* SEI         */
OP(79, OP_ADC  ( AY          ) ) /* ADC ay      */
OP(7a, OP_PLX  ( REGISTER_Y       ) ) /* PLY     (C) */
OP(7b, OP_TDC  (             ) ) /* TDC     (G) */
OP(7c, OP_JMPAXI(            ) ) /* JMP axi (C) */
OP(7d, OP_ADC  ( AX          ) ) /* ADC ax      */
OP(7e, OP_RORM ( AX          ) ) /* ROR ax      */
OP(7f, OP_ADC  ( ALX         ) ) /* ADC alx (G) */
OP(80, OP_BRA  (             ) ) /* BRA     (C) */
OP(81, OP_STA  ( DXI         ) ) /* STA dxi     */
OP(82, OP_BRL  (             ) ) /* BRL     (G) */
OP(83, OP_STA  ( S           ) ) /* STA s   (G) */
OP(84, OP_STX  ( REGISTER_Y, D    ) ) /* STY d       */
OP(85, OP_STA  ( D           ) ) /* STA d       */
OP(86, OP_STX  ( REGISTER_X, D    ) ) /* STX d       */
OP(87, OP_STA  ( DLI         ) ) /* STA dli (G) */
OP(88, OP_DECX ( REGISTER_Y       ) ) /* DEY         */
OP(89, OP_BITI (             ) ) /* BIT imm (C) */
OP(8a, OP_TXA  ( REGISTER_X       ) ) /* TXA         */
OP(8b, OP_PHB  (             ) ) /* PHB     (G) */
OP(8c, OP_STX  ( REGISTER_Y, A    ) ) /* STY a       */
OP(8d, OP_STA  ( A           ) ) /* STA a       */
OP(8e, OP_STX  ( REGISTER_X, A    ) ) /* STX a       */
OP(8f, OP_STA  ( AL          ) ) /* STA al  (G) */
OP(90, OP_BCC  ( COND_CC()   ) ) /* BCC         */
OP(91, OP_STA  ( DIY         ) ) /* STA diy     */
OP(92, OP_STA  ( DI          ) ) /* STA di  (C) */
OP(93, OP_STA  ( SIY         ) ) /* STA siy (G) */
OP(94, OP_STX  ( REGISTER_Y, DX   ) ) /* STY dx      */
OP(95, OP_STA  ( DX          ) ) /* STA dx      */
OP(96, OP_STX  ( REGISTER_X, DY   ) ) /* STX dy      */
OP(97, OP_STA  ( DLIY        ) ) /* STA dliy(G) */
OP(98, OP_TXA  ( REGISTER_Y       ) ) /* TYA         */
OP(99, OP_STA  ( AY          ) ) /* STA ay      */
OP(9a, OP_TXS  (             ) ) /* TXS         */
OP(9b, OP_TXY  (             ) ) /* TXY     (G) */
OP(9c, OP_STZ  ( A           ) ) /* STZ a   (C) */
OP(9d, OP_STA  ( AX          ) ) /* STA ax      */
OP(9e, OP_STZ  ( AX          ) ) /* STZ ax  (C) */
OP(9f, OP_STA  ( ALX         ) ) /* STA alx (G) */
OP(a0, OP_LDX  ( REGISTER_Y, IMM  ) ) /* LDY imm     */
OP(a1, OP_LDA  ( DXI         ) ) /* LDA dxi     */
OP(a2, OP_LDX  ( REGISTER_X, IMM  ) ) /* LDX imm     */
OP(a3, OP_LDA  ( S           ) ) /* LDA s   (G) */
OP(a4, OP_LDX  ( REGISTER_Y, D    ) ) /* LDY d       */
OP(a5, OP_LDA  ( D           ) ) /* LDA d       */
OP(a6, OP_LDX  ( REGISTER_X, D    ) ) /* LDX d       */
OP(a7, OP_LDA  ( DLI         ) ) /* LDA dli (G) */
OP(a8, OP_TAX  ( REGISTER_Y       ) ) /* TAY         */
OP(a9, OP_LDA  ( IMM         ) ) /* LDA imm     */
OP(aa, OP_TAX  ( REGISTER_X       ) ) /* TAX         */
OP(ab, OP_PLB  (             ) ) /* PLB     (G) */
OP(ac, OP_LDX  ( REGISTER_Y, A    ) ) /* LDY a       */
OP(ad, OP_LDA  ( A           ) ) /* LDA a       */
OP(ae, OP_LDX  ( REGISTER_X, A    ) ) /* LDX a       */
OP(af, OP_LDA  ( AL          ) ) /* LDA al  (G) */
OP(b0, OP_BCC  ( COND_CS()   ) ) /* BCS         */
OP(b1, OP_LDA  ( DIY         ) ) /* LDA diy     */
OP(b2, OP_LDA  ( DI          ) ) /* LDA di  (C) */
OP(b3, OP_LDA  ( SIY         ) ) /* LDA siy (G) */
OP(b4, OP_LDX  ( REGISTER_Y, DX   ) ) /* LDY dx      */
OP(b5, OP_LDA  ( DX          ) ) /* LDA dx      */
OP(b6, OP_LDX  ( REGISTER_X, DY   ) ) /* LDX dy      */
OP(b7, OP_LDA  ( DLIY        ) ) /* LDA dliy(G) */
OP(b8, OP_CLV  (             ) ) /* CLV         */
OP(b9, OP_LDA  ( AY          ) ) /* LDA ay      */
OP(ba, OP_TSX  (             ) ) /* TSX         */
OP(bb, OP_TYX  (             ) ) /* TYX     (G) */
OP(bc, OP_LDX  ( REGISTER_Y, AX   ) ) /* LDY ax      */
OP(bd, OP_LDA  ( AX          ) ) /* LDA ax      */
OP(be, OP_LDX  ( REGISTER_X, AY   ) ) /* LDX ay      */
OP(bf, OP_LDA  ( ALX         ) ) /* LDA alx (G) */
OP(c0, OP_CMPX ( REGISTER_Y, IMM  ) ) /* CPY imm     */
OP(c1, OP_CMP  ( DXI         ) ) /* CMP dxi     */
OP(c2, OP_REP  (             ) ) /* REP     (G) */
OP(c3, OP_CMP  ( S           ) ) /* CMP s   (G) */
OP(c4, OP_CMPX ( REGISTER_Y, D    ) ) /* CPY d       */
OP(c5, OP_CMP  ( D           ) ) /* CMP d       */
OP(c6, OP_DECM ( D           ) ) /* DEC d       */
OP(c7, OP_CMP  ( DLI         ) ) /* CMP dli (G) */
OP(c8, OP_INCX ( REGISTER_Y       ) ) /* INY         */
OP(c9, OP_CMP  ( IMM         ) ) /* CMP imm     */
OP(ca, OP_DECX ( REGISTER_X       ) ) /* DEX         */
OP(cb, OP_WAI  (             ) ) /* WAI     (G) */
OP(cc, OP_CMPX ( REGISTER_Y, A    ) ) /* CPY a       */
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
OP(d8, OP_CLD  (             ) ) /* CLD         */
OP(d9, OP_CMP  ( AY          ) ) /* CMP ay      */
OP(da, OP_PHX  ( REGISTER_X       ) ) /* PHX     (C) */
OP(db, OP_STP  (             ) ) /* STP     (G) */
OP(dc, OP_JMLAI(             ) ) /* JML ai  (G) */
OP(dd, OP_CMP  ( AX          ) ) /* CMP ax      */
OP(de, OP_DECM ( AX          ) ) /* DEC ax      */
OP(df, OP_CMP  ( ALX         ) ) /* CMP alx (G) */
OP(e0, OP_CMPX ( REGISTER_X, IMM  ) ) /* CPX imm     */
OP(e1, OP_SBC  ( DXI         ) ) /* SBC dxi     */
OP(e2, OP_SEP  (             ) ) /* SEP imm (G) */
OP(e3, OP_SBC  ( S           ) ) /* SBC s   (G) */
OP(e4, OP_CMPX ( REGISTER_X, D    ) ) /* CPX d       */
OP(e5, OP_SBC  ( D           ) ) /* SBC d       */
OP(e6, OP_INCM ( D           ) ) /* INC d       */
OP(e7, OP_SBC  ( DLI         ) ) /* SBC dli (G) */
OP(e8, OP_INCX ( REGISTER_X       ) ) /* INX         */
OP(e9, OP_SBC  ( IMM         ) ) /* SBC imm     */
OP(ea, OP_NOP  (             ) ) /* NOP         */
OP(eb, OP_XBA  (             ) ) /* XBA     (G) */
OP(ec, OP_CMPX ( REGISTER_X, A    ) ) /* CPX a       */
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
OP(f8, OP_SED  (             ) ) /* SED         */
OP(f9, OP_SBC  ( AY          ) ) /* SBC ay      */
OP(fa, OP_PLX  ( REGISTER_X       ) ) /* PLX     (C) */
OP(fb, OP_XCE  (             ) ) /* XCE     (G) */
OP(fc, OP_JSRAXI(            ) ) /* JSR axi (G) */
OP(fd, OP_SBC  ( AX          ) ) /* SBC ax      */
OP(fe, OP_INCM ( AX          ) ) /* INC ax      */
OP(ff, OP_SBC  ( ALX         ) ) /* SBC alx (G) */



TABLE_OPCODES =
{
	O(00),O(01),O(02),O(03),O(04),O(05),O(06),O(07),
	O(08),O(09),O(0a),O(0b),O(0c),O(0d),O(0e),O(0f),
	O(10),O(11),O(12),O(13),O(14),O(15),O(16),O(17),
	O(18),O(19),O(1a),O(1b),O(1c),O(1d),O(1e),O(1f),
	O(20),O(21),O(22),O(23),O(24),O(25),O(26),O(27),
	O(28),O(29),O(2a),O(2b),O(2c),O(2d),O(2e),O(2f),
	O(30),O(31),O(32),O(33),O(34),O(35),O(36),O(37),
	O(38),O(39),O(3a),O(3b),O(3c),O(3d),O(3e),O(3f),
	O(40),O(41),O(42),O(43),O(44),O(45),O(46),O(47),
	O(48),O(49),O(4a),O(4b),O(4c),O(4d),O(4e),O(4f),
	O(50),O(51),O(52),O(53),O(54),O(55),O(56),O(57),
	O(58),O(59),O(5a),O(5b),O(5c),O(5d),O(5e),O(5f),
	O(60),O(61),O(62),O(63),O(64),O(65),O(66),O(67),
	O(68),O(69),O(6a),O(6b),O(6c),O(6d),O(6e),O(6f),
	O(70),O(71),O(72),O(73),O(74),O(75),O(76),O(77),
	O(78),O(79),O(7a),O(7b),O(7c),O(7d),O(7e),O(7f),
	O(80),O(81),O(82),O(83),O(84),O(85),O(86),O(87),
	O(88),O(89),O(8a),O(8b),O(8c),O(8d),O(8e),O(8f),
	O(90),O(91),O(92),O(93),O(94),O(95),O(96),O(97),
	O(98),O(99),O(9a),O(9b),O(9c),O(9d),O(9e),O(9f),
	O(a0),O(a1),O(a2),O(a3),O(a4),O(a5),O(a6),O(a7),
	O(a8),O(a9),O(aa),O(ab),O(ac),O(ad),O(ae),O(af),
	O(b0),O(b1),O(b2),O(b3),O(b4),O(b5),O(b6),O(b7),
	O(b8),O(b9),O(ba),O(bb),O(bc),O(bd),O(be),O(bf),
	O(c0),O(c1),O(c2),O(c3),O(c4),O(c5),O(c6),O(c7),
	O(c8),O(c9),O(ca),O(cb),O(cc),O(cd),O(ce),O(cf),
	O(d0),O(d1),O(d2),O(d3),O(d4),O(d5),O(d6),O(d7),
	O(d8),O(d9),O(da),O(db),O(dc),O(dd),O(de),O(df),
	O(e0),O(e1),O(e2),O(e3),O(e4),O(e5),O(e6),O(e7),
	O(e8),O(e9),O(ea),O(eb),O(ec),O(ed),O(ee),O(ef),
	O(f0),O(f1),O(f2),O(f3),O(f4),O(f5),O(f6),O(f7),
	O(f8),O(f9),O(fa),O(fb),O(fc),O(fd),O(fe),O(ff)
};



/* Assert or clear a line on the CPU */
TABLE_FUNCTION(void, set_line, (int line, int state))
{
	switch(line)
	{
		case G65816_LINE_IRQ:
			switch(state)
			{
				case CLEAR_LINE:
					LINE_IRQ = 0;
					return;
				case ASSERT_LINE:
				case HOLD_LINE:
					LINE_IRQ = 1;
			}
			if(FLAG_I)
			{
				if(CPU_STOPPED & STOP_LEVEL_WAI)
				{
					CPU_STOPPED &= ~STOP_LEVEL_WAI;
				}
				return;
			}
			return;
		case G65816_LINE_NMI:
			if(state == CLEAR_LINE)
			{
				LINE_NMI = 0;
				return;
			}
			if(!LINE_NMI)
			{
				LINE_NMI = state != PULSE_LINE;
				CPU_STOPPED &= ~STOP_LEVEL_WAI;
				if(!CPU_STOPPED)
					g65816i_interrupt_nmi();
			}
			return;
		case G65816_LINE_SO:
			FLAG_V = VFLAG_SET;
			break;
		case G65816_LINE_RESET:
		case G65816_LINE_ABORT:
		case G65816_LINE_RDY:
			return;
	}

	LINE_IRQ=1;
}



/* Get a register from the CPU core */
TABLE_FUNCTION(uint, get_reg, (int regnum))
{
	switch(regnum)
	{
		case G65816_A: return REGISTER_B | REGISTER_A;
		case G65816_X: return REGISTER_X;
		case G65816_Y: return REGISTER_Y;
		case REG_SP: return REGISTER_S;
		case G65816_S: return REGISTER_S;
		case REG_PC: return REGISTER_PC;
		case G65816_PC: return REGISTER_PC;
		case G65816_PB: return REGISTER_PB >> 16;
		case G65816_DB: return REGISTER_DB >> 16;
		case G65816_D: return REGISTER_D;
		case G65816_P: return g65816i_get_reg_p();
		case G65816_NMI_STATE: return LINE_NMI;
		case G65816_IRQ_STATE: return LINE_IRQ;
		case REG_PREVIOUSPC: return REGISTER_PPC;
	}
	return 0;
}



TABLE_FUNCTION(void, set_reg, (int regnum, uint val))
{
	switch(regnum)
	{
		case REG_PC: case G65816_PC: REGISTER_PC = MAKE_UINT_16(val); break;
#if FLAG_SET_E
		case REG_SP: case G65816_S: REGISTER_S = MAKE_UINT_8(val) | 0x100; break;
#else
		case REG_SP: case G65816_S: REGISTER_S = MAKE_UINT_16(val); break;
#endif
		case G65816_P: g65816i_set_reg_p(val); break;
#if FLAG_SET_M
		case G65816_A: REGISTER_A = MAKE_UINT_8(val); REGISTER_B = val&0xff00; break;
#else
		case G65816_A: REGISTER_A = MAKE_UINT_16(val); break;
#endif
#if FLAG_SET_X
		case G65816_X: REGISTER_X = MAKE_UINT_8(val); break;
		case G65816_Y: REGISTER_Y = MAKE_UINT_8(val); break;
#else
		case G65816_X: REGISTER_X = MAKE_UINT_16(val); break;
		case G65816_Y: REGISTER_Y = MAKE_UINT_16(val); break;
#endif
		case G65816_DB: REGISTER_DB = MAKE_UINT_8(val); break;
		case G65816_PB: REGISTER_PB = MAKE_UINT_8(val); break;
		case G65816_NMI_STATE: FTABLE_SET_LINE(G65816_LINE_NMI, val == 0 ? CLEAR_LINE : ASSERT_LINE); break;
		case G65816_IRQ_STATE: FTABLE_SET_LINE(G65816_LINE_IRQ, val == 0 ? CLEAR_LINE : ASSERT_LINE); break;
	 }
}



INLINE int g65816i_correct_mode(void)
{
#if EXECUTION_MODE == EXECUTION_MODE_E
	return (FLAG_E == EFLAG_SET);
#elif EXECUTION_MODE == EXECUTION_MODE_M0X0
	return (FLAG_E == EFLAG_CLEAR) && (FLAG_M == MFLAG_CLEAR) && (FLAG_X == XFLAG_CLEAR);
#elif EXECUTION_MODE == EXECUTION_MODE_M0X1
	return (FLAG_E == EFLAG_CLEAR) && (FLAG_M == MFLAG_CLEAR) && (FLAG_X == XFLAG_SET);
#elif EXECUTION_MODE == EXECUTION_MODE_M1X0
	return (FLAG_E == EFLAG_CLEAR) && (FLAG_M == MFLAG_SET) && (FLAG_X == XFLAG_CLEAR);
#elif EXECUTION_MODE == EXECUTION_MODE_M1X1
	return (FLAG_E == EFLAG_CLEAR) && (FLAG_M == MFLAG_SET) && (FLAG_X == XFLAG_SET);
#else
#error Invalid EXECUTION_MODE
#endif
}



TABLE_FUNCTION(int, execute, (int clocks))
{
	// do a check here also in case we're in STOP_WAI mode - this'll clear it when the IRQ happens
	g65816i_check_maskable_interrupt();

	if (!CPU_STOPPED)
	{
		CLOCKS = clocks;
		do
		{
			/* Note that I'm doing a per-instruction interrupt
             * check until this core is working well enough
             * to start trying fancy stuff.
             */
			g65816i_check_maskable_interrupt();

			REGISTER_PPC = REGISTER_PC;
			G65816_CALL_DEBUGGER;

			REGISTER_PC++;
			REGISTER_IR = read_8_IMM(REGISTER_PB | REGISTER_PPC);
			FTABLE_OPCODES[REGISTER_IR]();
		} while((CLOCKS > 0) && g65816i_correct_mode());
		return clocks - CLOCKS;
	}
	return clocks;
}


/* ======================================================================== */
/* ================================== EOF ================================= */
/* ======================================================================== */
