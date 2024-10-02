// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari
/***************************************************************************

    TMS34010: Portable Texas Instruments TMS34010 emulator

    Copyright Alex Pasadyn/Zsolt Vasvari
    Parts based on code by Aaron Giles

***************************************************************************/



/***************************************************************************
    MISC MACROS
***************************************************************************/

#define ZEXTEND(val,width) if (width) (val) &= ((uint32_t)0xffffffff >> (32 - (width)))
#define SEXTEND(val,width) if (width) (val) = (int32_t)((val) << (32 - (width))) >> (32 - (width))

#define SXYTOL(val)   ((((int16_t)(val).y * m_convsp) + ((int16_t)(val).x << m_pixelshift)) + OFFSET())
#define DXYTOL(val)   ((((int16_t)(val).y * m_convdp) + ((int16_t)(val).x << m_pixelshift)) + OFFSET())
#define MXYTOL(val)   ((((int16_t)(val).y * m_convmp) + ((int16_t)(val).x << m_pixelshift)) + OFFSET())

#define COUNT_CYCLES(x)   m_icount -= x
#define COUNT_UNKNOWN_CYCLES(x) COUNT_CYCLES(x)

#define CORRECT_ODD_PC(x) do { if (m_pc & 0x0f) logerror("%s to PC=%08X\n", x, m_pc); m_pc &= ~0x0f; } while (0)



/***************************************************************************
    FLAG HANDLING MACROS
***************************************************************************/

#define SIGN(val)           ((val) & 0x80000000)

#define CLR_Z()                m_st &= ~STBIT_Z
#define CLR_V()                m_st &= ~STBIT_V
#define CLR_C()                m_st &= ~STBIT_C
#define CLR_N()                m_st &= ~STBIT_N
#define CLR_NZ()               m_st &= ~(STBIT_N | STBIT_Z)
#define CLR_CZ()               m_st &= ~(STBIT_C | STBIT_Z)
#define CLR_ZV()               m_st &= ~(STBIT_Z | STBIT_V)
#define CLR_NZV()              m_st &= ~(STBIT_N | STBIT_Z | STBIT_V)
#define CLR_NCZ()              m_st &= ~(STBIT_N | STBIT_C | STBIT_Z)
#define CLR_NCZV()             m_st &= ~(STBIT_N | STBIT_C | STBIT_Z | STBIT_V)

#define SET_V_BIT_LO(val,bit) m_st |= ((val) << (28 - (bit))) & STBIT_V
#define SET_V_BIT_HI(val,bit) m_st |= ((val) >> ((bit) - 28)) & STBIT_V
#define SET_V_LOG(val)        m_st |= (val) << 28
#define SET_Z_BIT_LO(val,bit) m_st |= ((val) << (29 - (bit))) & STBIT_Z
#define SET_Z_BIT_HI(val,bit) m_st |= ((val) >> ((bit) - 29)) & STBIT_Z
#define SET_Z_LOG(val)        m_st |= (val) << 29
#define SET_C_BIT_LO(val,bit) m_st |= ((val) << (30 - (bit))) & STBIT_C
#define SET_C_BIT_HI(val,bit) m_st |= ((val) >> ((bit) - 30)) & STBIT_C
#define SET_C_LOG(val)        m_st |= (val) << 30
#define SET_N_BIT(val,bit)    m_st |= ((val) << (31 - (bit))) & STBIT_N
#define SET_N_LOG(val)        m_st |= (val) << 31

#define SET_Z_VAL(val)        SET_Z_LOG((val) == 0)
#define SET_N_VAL(val)        SET_N_BIT(val, 31)
#define SET_NZ_VAL(val)       SET_Z_VAL(val); SET_N_VAL(val)
#define SET_V_SUB(a,b,r)      SET_V_BIT_HI(((a) ^ (b)) & ((a) ^ (r)), 31)
#define SET_V_ADD(a,b,r)      SET_V_BIT_HI(~((a) ^ (b)) & ((a) ^ (r)), 31)
#define SET_C_SUB(a,b)        SET_C_LOG((uint32_t)(b) > (uint32_t)(a))
#define SET_C_ADD(a,b)        SET_C_LOG((uint32_t)~(a) < (uint32_t)(b))
#define SET_NZV_SUB(a,b,r)    SET_NZ_VAL(r); SET_V_SUB(a,b,r)
#define SET_NZCV_SUB(a,b,r)   SET_NZV_SUB(a,b,r); SET_C_SUB(a,b)
#define SET_NZCV_ADD(a,b,r)   SET_NZ_VAL(r); SET_V_ADD(a,b,r); SET_C_ADD(a,b)

static const uint8_t fw_inc[32] = { 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };


/***************************************************************************
    UNIMPLEMENTED INSTRUCTION
***************************************************************************/

void tms340x0_device::unimpl(uint16_t op)
{
	// Not all illegal opcodes cause TRAP 30 exceptions on TMS34010, and some games only work because of this:

	// 9 Ball Shootout calls to FFDF7468, expecting it
	// to execute the next instruction from FFDF7470
	// but the instruction at FFDF7460 is an 0x0001

	// Super High Impact executes 0x0007 at various entry points

	logerror("TMS34010 reserved opcode %04Xh encountered at %08x\n", op, m_ppc);
	COUNT_CYCLES(1);
}

void tms340x0_device::illop(uint16_t op)
{
	PUSH(m_pc);
	PUSH(m_st);
	RESET_ST();
	m_pc = RLONG(0xfffffc20);
	COUNT_UNKNOWN_CYCLES(16);

	/* extra check to prevent bad things */
#if 0
	if (m_pc == 0 || s_opcode_table[space(AS_PROGRAM).read_word(m_pc) >> 4] == &tms34010_device::unimpl)
	{
		set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		machine().debug_break();
	}
#endif
}


/***************************************************************************
    X/Y OPERATIONS
***************************************************************************/

#define ADD_XY(R)                               \
{                                               \
	XY  a =  R##REG_XY(SRCREG(op));                 \
	XY *b = &R##REG_XY(DSTREG(op));                 \
	CLR_NCZV();                                  \
	b->x += a.x;                                \
	b->y += a.y;                                \
	SET_N_LOG(b->x == 0);                      \
	SET_C_BIT_LO(b->y, 15);                        \
	SET_Z_LOG(b->y == 0);                      \
	SET_V_BIT_LO(b->x, 15);                        \
	COUNT_CYCLES(1);                            \
}
void tms340x0_device::add_xy_a(uint16_t op) { ADD_XY(A); }
void tms340x0_device::add_xy_b(uint16_t op) { ADD_XY(B); }

#define SUB_XY(R)                               \
{                                               \
	XY  a =  R##REG_XY(SRCREG(op));                 \
	XY *b = &R##REG_XY(DSTREG(op));                 \
	CLR_NCZV();                                  \
	SET_N_LOG(a.x == b->x);                        \
	SET_C_LOG(a.y > b->y);                     \
	SET_Z_LOG(a.y == b->y);                        \
	SET_V_LOG(a.x > b->x);                     \
	b->x -= a.x;                                \
	b->y -= a.y;                                \
	COUNT_CYCLES(1);                            \
}
void tms340x0_device::sub_xy_a(uint16_t op) { SUB_XY(A); }
void tms340x0_device::sub_xy_b(uint16_t op) { SUB_XY(B); }

#define CMP_XY(R)                               \
{                                               \
	int16_t res;                                  \
	XY a = R##REG_XY(DSTREG(op));                   \
	XY b = R##REG_XY(SRCREG(op));                   \
	CLR_NCZV();                                  \
	res = a.x-b.x;                              \
	SET_N_LOG(res == 0);                       \
	SET_V_BIT_LO(res, 15);                     \
	res = a.y-b.y;                              \
	SET_Z_LOG(res == 0);                       \
	SET_C_BIT_LO(res, 15);                     \
	COUNT_CYCLES(1);                            \
}
void tms340x0_device::cmp_xy_a(uint16_t op) { CMP_XY(A); }
void tms340x0_device::cmp_xy_b(uint16_t op) { CMP_XY(B); }

#define CPW(R)                                  \
{                                               \
	int32_t res = 0;                              \
	int16_t x = R##REG_X(SRCREG(op));                 \
	int16_t y = R##REG_Y(SRCREG(op));                 \
												\
	CLR_V();                                     \
	res |= ((WSTART_X() > x) ? 0x20  : 0);       \
	res |= ((x > WEND_X())   ? 0x40  : 0);       \
	res |= ((WSTART_Y() > y) ? 0x80  : 0);       \
	res |= ((y > WEND_Y())   ? 0x100 : 0);       \
	R##REG(DSTREG(op)) = res;                       \
	SET_V_LOG(res != 0);                       \
	COUNT_CYCLES(1);                            \
}
void tms340x0_device::cpw_a(uint16_t op) { CPW(A); }
void tms340x0_device::cpw_b(uint16_t op) { CPW(B); }

// TODO: Handle cycles properly, currently we assume power of two
#define CVXYL(R)                                    \
{                                                   \
	R##REG(DSTREG(op)) = DXYTOL(R##REG_XY(SRCREG(op)));     \
	COUNT_CYCLES(3);                                \
}
void tms340x0_device::cvxyl_a(uint16_t op) { CVXYL(A); }
void tms340x0_device::cvxyl_b(uint16_t op) { CVXYL(B); }

#define MOVX(R)                                     \
{                                                   \
	R##REG(DSTREG(op)) = (R##REG(DSTREG(op)) & 0xffff0000) | (uint16_t)R##REG(SRCREG(op));    \
	COUNT_CYCLES(1);                                                                    \
}
void tms340x0_device::movx_a(uint16_t op) { MOVX(A); }
void tms340x0_device::movx_b(uint16_t op) { MOVX(B); }

#define MOVY(R)                                     \
{                                                   \
	R##REG(DSTREG(op)) = (R##REG(SRCREG(op)) & 0xffff0000) | (uint16_t)R##REG(DSTREG(op));    \
	COUNT_CYCLES(1);                                                                    \
}
void tms340x0_device::movy_a(uint16_t op) { MOVY(A); }
void tms340x0_device::movy_b(uint16_t op) { MOVY(B); }



/***************************************************************************
    PIXEL TRANSFER OPERATIONS
***************************************************************************/

#define PIXT_RI(R)                                  \
{                                                   \
	WPIXEL(R##REG(DSTREG(op)),R##REG(SRCREG(op)));  \
	COUNT_UNKNOWN_CYCLES(2);                        \
}
void tms340x0_device::pixt_ri_a(uint16_t op) { PIXT_RI(A); }
void tms340x0_device::pixt_ri_b(uint16_t op) { PIXT_RI(B); }

#define PIXT_RIXY(R)                                                                \
{                                                                                   \
	if (WINDOW_CHECKING() != 0)                                                      \
	{                                                                               \
		CLR_V();                                                                     \
		if (R##REG_X(DSTREG(op)) < WSTART_X() || R##REG_X(DSTREG(op)) > WEND_X() ||               \
			R##REG_Y(DSTREG(op)) < WSTART_Y() || R##REG_Y(DSTREG(op)) > WEND_Y())             \
		{                                                                           \
			SET_V_LOG(1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING() == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(DXYTOL(R##REG_XY(DSTREG(op))),R##REG(SRCREG(op)));                               \
skip:                                                                               \
	COUNT_UNKNOWN_CYCLES(4);                                                        \
}
void tms340x0_device::pixt_rixy_a(uint16_t op) { PIXT_RIXY(A); }
void tms340x0_device::pixt_rixy_b(uint16_t op) { PIXT_RIXY(B); }

#define PIXT_IR(R)                                  \
{                                                   \
	int32_t temp = RPIXEL(R##REG(SRCREG(op)));            \
	CLR_V();                                         \
	R##REG(DSTREG(op)) = temp;                          \
	SET_V_LOG(temp != 0);                          \
	COUNT_CYCLES(4);                                \
}
void tms340x0_device::pixt_ir_a(uint16_t op) { PIXT_IR(A); }
void tms340x0_device::pixt_ir_b(uint16_t op) { PIXT_IR(B); }

#define PIXT_II(R)                                  \
{                                                   \
	WPIXEL(R##REG(DSTREG(op)),RPIXEL(R##REG(SRCREG(op))));  \
	COUNT_UNKNOWN_CYCLES(4);                        \
}
void tms340x0_device::pixt_ii_a(uint16_t op) { PIXT_II(A); }
void tms340x0_device::pixt_ii_b(uint16_t op) { PIXT_II(B); }

#define PIXT_IXYR(R)                                \
{                                                   \
	int32_t temp = RPIXEL(SXYTOL(R##REG_XY(SRCREG(op)))); \
	CLR_V();                                         \
	R##REG(DSTREG(op)) = temp;                          \
	SET_V_LOG(temp != 0);                          \
	COUNT_CYCLES(6);                                \
}
void tms340x0_device::pixt_ixyr_a(uint16_t op) { PIXT_IXYR(A); }
void tms340x0_device::pixt_ixyr_b(uint16_t op) { PIXT_IXYR(B); }

#define PIXT_IXYIXY(R)                                                              \
{                                                                                   \
	if (WINDOW_CHECKING() != 0)                                                      \
	{                                                                               \
		CLR_V();                                                                     \
		if (R##REG_X(DSTREG(op)) < WSTART_X() || R##REG_X(DSTREG(op)) > WEND_X() ||               \
			R##REG_Y(DSTREG(op)) < WSTART_Y() || R##REG_Y(DSTREG(op)) > WEND_Y())             \
		{                                                                           \
			SET_V_LOG(1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING() == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(DXYTOL(R##REG_XY(DSTREG(op))),RPIXEL(SXYTOL(R##REG_XY(SRCREG(op)))));            \
skip:                                                                               \
	COUNT_UNKNOWN_CYCLES(7);                                                        \
}
void tms340x0_device::pixt_ixyixy_a(uint16_t op) { PIXT_IXYIXY(A); }
void tms340x0_device::pixt_ixyixy_b(uint16_t op) { PIXT_IXYIXY(B); }

#define DRAV(R)                                                                     \
{                                                                                   \
	if (WINDOW_CHECKING() != 0)                                                      \
	{                                                                               \
		CLR_V();                                                                     \
		if (R##REG_X(DSTREG(op)) < WSTART_X() || R##REG_X(DSTREG(op)) > WEND_X() ||               \
			R##REG_Y(DSTREG(op)) < WSTART_Y() || R##REG_Y(DSTREG(op)) > WEND_Y())             \
		{                                                                           \
			SET_V_LOG(1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING() == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(DXYTOL(R##REG_XY(DSTREG(op))),COLOR1());                                      \
skip:                                                                               \
	R##REG_X(DSTREG(op)) += R##REG_X(SRCREG(op));                                           \
	R##REG_Y(DSTREG(op)) += R##REG_Y(SRCREG(op));                                           \
	COUNT_UNKNOWN_CYCLES(4);                                                        \
}
void tms340x0_device::drav_a(uint16_t op) { DRAV(A); }
void tms340x0_device::drav_b(uint16_t op) { DRAV(B); }



/***************************************************************************
    ARITHMETIC OPERATIONS
***************************************************************************/

#define ABS(R)                                              \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t r = 0 - *rd;                                      \
	CLR_NZV();                                               \
	if (r > 0) *rd = r;                                     \
	SET_NZ_VAL(r);                                         \
	SET_V_LOG(r == (int32_t)0x80000000);                     \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::abs_a(uint16_t op) { ABS(A); }
void tms340x0_device::abs_b(uint16_t op) { ABS(B); }

#define ADD(R)                                              \
{                                                           \
	int32_t a = R##REG(SRCREG(op));                               \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::add_a(uint16_t op) { ADD(A); }
void tms340x0_device::add_b(uint16_t op) { ADD(B); }

#define ADDC(R)                                             \
{                                                           \
	/* I'm not sure to which side the carry is added to, should */  \
	/* verify it against the examples */                    \
	int32_t a = R##REG(SRCREG(op));                               \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b + (C_FLAG() ? 1 : 0);                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::addc_a(uint16_t op) { ADDC(A); }
void tms340x0_device::addc_b(uint16_t op) { ADDC(B); }

#define ADDI_W(R)                                           \
{                                                           \
	int32_t a = PARAM_WORD();                              \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(2);                                        \
}
void tms340x0_device::addi_w_a(uint16_t op) { ADDI_W(A); }
void tms340x0_device::addi_w_b(uint16_t op) { ADDI_W(B); }

#define ADDI_L(R)                                           \
{                                                           \
	int32_t a = PARAM_LONG();                              \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(3);                                        \
}
void tms340x0_device::addi_l_a(uint16_t op) { ADDI_L(A); }
void tms340x0_device::addi_l_b(uint16_t op) { ADDI_L(B); }

#define ADDK(R)                                             \
{                                                           \
	int32_t a = fw_inc[PARAM_K(op)];                              \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t b = *rd;                                          \
	int32_t r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::addk_a(uint16_t op) { ADDK(A); }
void tms340x0_device::addk_b(uint16_t op) { ADDK(B); }

#define AND(R)                                              \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= R##REG(SRCREG(op));                                  \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::and_a(uint16_t op) { AND(A); }
void tms340x0_device::and_b(uint16_t op) { AND(B); }

#define ANDI(R)                                             \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= ~PARAM_LONG();                                \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(3);                                        \
}
void tms340x0_device::andi_a(uint16_t op) { ANDI(A); }
void tms340x0_device::andi_b(uint16_t op) { ANDI(B); }

#define ANDN(R)                                             \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= ~R##REG(SRCREG(op));                                 \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::andn_a(uint16_t op) { ANDN(A); }
void tms340x0_device::andn_b(uint16_t op) { ANDN(B); }

#define BTST_K(R)                                           \
{                                                           \
	int bit = 31 - PARAM_K(op);                                 \
	CLR_Z();                                                 \
	if (bit <= 29)                                          \
		SET_Z_BIT_LO(~R##REG(DSTREG(op)), bit);                    \
	else                                                    \
		SET_Z_BIT_HI(~R##REG(DSTREG(op)), bit);                    \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::btst_k_a(uint16_t op) { BTST_K(A); }
void tms340x0_device::btst_k_b(uint16_t op) { BTST_K(B); }

#define BTST_R(R)                                           \
{                                                           \
	int bit = R##REG(SRCREG(op)) & 0x1f;                        \
	CLR_Z();                                                 \
	if (bit <= 29)                                          \
		SET_Z_BIT_LO(~R##REG(DSTREG(op)), bit);                    \
	else                                                    \
		SET_Z_BIT_HI(~R##REG(DSTREG(op)), bit);                    \
	COUNT_CYCLES(2);                                        \
}
void tms340x0_device::btst_r_a(uint16_t op) { BTST_R(A); }
void tms340x0_device::btst_r_b(uint16_t op) { BTST_R(B); }

void tms340x0_device::clrc(uint16_t op)
{
	CLR_C();
	COUNT_CYCLES(1);
}

#define CMP(R)                                              \
{                                                           \
	int32_t *rs = &R##REG(SRCREG(op));                            \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t r = *rd - *rs;                                    \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,*rs,r);                                \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::cmp_a(uint16_t op) { CMP(A); }
void tms340x0_device::cmp_b(uint16_t op) { CMP(B); }

#define CMPI_W(R)                                           \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t t = (int16_t)~PARAM_WORD();                      \
	int32_t r = *rd - t;                                      \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(2);                                        \
}
void tms340x0_device::cmpi_w_a(uint16_t op) { CMPI_W(A); }
void tms340x0_device::cmpi_w_b(uint16_t op) { CMPI_W(B); }

#define CMPI_L(R)                                           \
{                                                           \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t t = ~PARAM_LONG();                             \
	int32_t r = *rd - t;                                      \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(3);                                        \
}
void tms340x0_device::cmpi_l_a(uint16_t op) { CMPI_L(A); }
void tms340x0_device::cmpi_l_b(uint16_t op) { CMPI_L(B); }

void tms340x0_device::dint(uint16_t op)
{
	m_st &= ~STBIT_IE;
	COUNT_CYCLES(3);
}

#define DIVS(R)                                             \
{                                                           \
	int32_t *rs  = &R##REG(SRCREG(op));                           \
	int32_t *rd1 = &R##REG(DSTREG(op));                           \
	CLR_NZV();                                               \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			int32_t *rd2 = &R##REG(DSTREG(op)+1);                 \
			int64_t dividend = ((uint64_t)*rd1 << 32) | (uint32_t)*rd2; \
			int64_t quotient = dividend / *rs;                \
			int32_t remainder = dividend % *rs;               \
			if ((int64_t)(int32_t)quotient != quotient)     \
			{                                               \
				SET_V_LOG(1);                              \
			}                                               \
			else                                            \
			{                                               \
				*rd1 = quotient;                            \
				*rd2 = remainder;                           \
				SET_NZ_VAL(*rd1);                          \
			}                                               \
		}                                                   \
		COUNT_CYCLES(40);                                   \
	}                                                       \
	else                                                    \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			*rd1 /= *rs;                                    \
			SET_NZ_VAL(*rd1);                              \
		}                                                   \
		COUNT_CYCLES(39);                                   \
	}                                                       \
}
void tms340x0_device::divs_a(uint16_t op) { DIVS(A); }
void tms340x0_device::divs_b(uint16_t op) { DIVS(B); }

#define DIVU(R)                                             \
{                                                           \
	int32_t *rs  = &R##REG(SRCREG(op));                           \
	int32_t *rd1 = &R##REG(DSTREG(op));                           \
	CLR_ZV();                                                    \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			int32_t *rd2 = &R##REG(DSTREG(op)+1);                 \
			uint64_t dividend  = ((uint64_t)*rd1 << 32) | (uint32_t)*rd2; \
			uint64_t quotient  = dividend / (uint32_t)*rs;      \
			uint32_t remainder = dividend % (uint32_t)*rs;      \
			if (quotient > 0xffffffff)                      \
			{                                               \
				SET_V_LOG(1);                              \
			}                                               \
			else                                            \
			{                                               \
				*rd1 = quotient;                            \
				*rd2 = remainder;                           \
				SET_Z_VAL(*rd1);                           \
			}                                               \
		}                                                   \
	}                                                       \
	else                                                    \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			*rd1 = (uint32_t)*rd1 / (uint32_t)*rs;              \
			SET_Z_VAL(*rd1);                               \
		}                                                   \
	}                                                       \
	COUNT_CYCLES(37);                                       \
}
void tms340x0_device::divu_a(uint16_t op) { DIVU(A); }
void tms340x0_device::divu_b(uint16_t op) { DIVU(B); }

void tms340x0_device::eint(uint16_t op)
{
	m_st |= STBIT_IE;
	check_interrupt();
	COUNT_CYCLES(3);
}

#define EXGF(F,R)                                               \
{                                                               \
	uint8_t shift = F ? 6 : 0;                                    \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t temp = (m_st >> shift) & 0x3f;                    \
	m_st &= ~(0x3f << shift);                                \
	m_st |= (*rd & 0x3f) << shift;                           \
	*rd = temp;                                                 \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::exgf0_a(uint16_t op) { EXGF(0,A); }
void tms340x0_device::exgf0_b(uint16_t op) { EXGF(0,B); }
void tms340x0_device::exgf1_a(uint16_t op) { EXGF(1,A); }
void tms340x0_device::exgf1_b(uint16_t op) { EXGF(1,B); }

#define LMO(R)                                                  \
{                                                               \
	uint32_t res = 0;                                             \
	uint32_t rs  = R##REG(SRCREG(op));                                \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	SET_Z_VAL(rs);                                             \
	if (rs)                                                     \
	{                                                           \
		while (!(rs & 0x80000000))                              \
		{                                                       \
			res++;                                              \
			rs <<= 1;                                           \
		}                                                       \
	}                                                           \
	*rd = res;                                                  \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::lmo_a(uint16_t op) { LMO(A); }
void tms340x0_device::lmo_b(uint16_t op) { LMO(B); }

#define MMFM(R)                                                 \
{                                                               \
	int32_t i;                                                    \
	uint16_t l = (uint16_t) PARAM_WORD();                        \
	COUNT_CYCLES(3);                                            \
	{                                                           \
		int32_t rd = DSTREG(op);                                      \
		for (i = 15; i >= 0 ; i--)                              \
		{                                                       \
			if (l & 0x8000)                                     \
			{                                                   \
				R##REG(i) = RLONG(R##REG(rd));                 \
				R##REG(rd) += 0x20;                             \
				COUNT_CYCLES(4);                                \
			}                                                   \
			l <<= 1;                                            \
		}                                                       \
	}                                                           \
}
void tms340x0_device::mmfm_a(uint16_t op) { MMFM(A); }
void tms340x0_device::mmfm_b(uint16_t op) { MMFM(B); }

#define MMTM(R)                                                 \
{                                                               \
	uint32_t i;                                                   \
	uint16_t l = (uint16_t) PARAM_WORD();                        \
	COUNT_CYCLES(2);                                            \
	{                                                           \
		int32_t rd = DSTREG(op);                                      \
		if (m_is_34020)                                      \
		{                                                       \
			CLR_N();                                             \
			SET_N_VAL(R##REG(rd) ^ 0x80000000);                    \
		}                                                       \
		for (i = 0; i  < 16; i++)                               \
		{                                                       \
			if (l & 0x8000)                                     \
			{                                                   \
				R##REG(rd) -= 0x20;                             \
				WLONG(R##REG(rd),R##REG(i));                   \
				COUNT_CYCLES(4);                                \
			}                                                   \
			l <<= 1;                                            \
		}                                                       \
	}                                                           \
}
void tms340x0_device::mmtm_a(uint16_t op) { MMTM(A); }
void tms340x0_device::mmtm_b(uint16_t op) { MMTM(B); }

#define MODS(R)                                                 \
{                                                               \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	if (*rs != 0)                                               \
	{                                                           \
		*rd %= *rs;                                             \
		SET_NZ_VAL(*rd);                                       \
	}                                                           \
	else                                                        \
		SET_V_LOG(1);                                          \
	COUNT_CYCLES(40);                                           \
}
void tms340x0_device::mods_a(uint16_t op) { MODS(A); }
void tms340x0_device::mods_b(uint16_t op) { MODS(B); }

#define MODU(R)                                                 \
{                                                               \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_ZV();                                                        \
	if (*rs != 0)                                               \
	{                                                           \
		*rd = (uint32_t)*rd % (uint32_t)*rs;                        \
		SET_Z_VAL(*rd);                                            \
	}                                                           \
	else                                                        \
		SET_V_LOG(1);                                          \
	COUNT_CYCLES(35);                                           \
}
void tms340x0_device::modu_a(uint16_t op) { MODU(A); }
void tms340x0_device::modu_b(uint16_t op) { MODU(B); }

#define MPYS(R)                                                 \
{                                                               \
	int32_t *rd1 = &R##REG(DSTREG(op));                               \
	int32_t m1 = R##REG(SRCREG(op));                                  \
	int64_t product;                                              \
																\
	SEXTEND(m1, FW(1));                                         \
	CLR_NZ();                                                        \
	product = mul_32x32(m1, *rd1);                          \
	SET_Z_LOG(product == 0);                                   \
	SET_N_BIT(product >> 32, 31);                              \
																\
	*rd1             = (int32_t)(product >> 32);                \
	R##REG(DSTREG(op)|1) = product & 0xffffffff;                \
																\
	COUNT_CYCLES(20);                                           \
}
void tms340x0_device::mpys_a(uint16_t op) { MPYS(A); }
void tms340x0_device::mpys_b(uint16_t op) { MPYS(B); }

#define MPYU(R)                                                 \
{                                                               \
	int32_t *rd1 = &R##REG(DSTREG(op));                               \
	uint32_t m1 = R##REG(SRCREG(op));                                 \
	uint64_t product;                                             \
																\
	ZEXTEND(m1, FW(1));                                         \
	CLR_Z();                                                     \
	product = mulu_32x32(m1, *rd1);                     \
	SET_Z_LOG(product == 0);                                   \
																\
	*rd1             = (int32_t)(product >> 32);                \
	R##REG(DSTREG(op)|1) = product & 0xffffffff;                \
																\
	COUNT_CYCLES(21);                                           \
}
void tms340x0_device::mpyu_a(uint16_t op) { MPYU(A); }
void tms340x0_device::mpyu_b(uint16_t op) { MPYU(B); }

#define NEG(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t r = 0 - *rd;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(0,*rd,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::neg_a(uint16_t op) { NEG(A); }
void tms340x0_device::neg_b(uint16_t op) { NEG(B); }

#define NEGB(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = *rd + (C_FLAG() ? 1 : 0);                          \
	int32_t r = 0 - t;                                            \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(0,t,r);                                        \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::negb_a(uint16_t op) { NEGB(A); }
void tms340x0_device::negb_b(uint16_t op) { NEGB(B); }

void tms340x0_device::nop(uint16_t op)
{
	COUNT_CYCLES(1);
}

#define NOT(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd = ~(*rd);                                               \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::not_a(uint16_t op) { NOT(A); }
void tms340x0_device::not_b(uint16_t op) { NOT(B); }

#define OR(R)                                                   \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd |= R##REG(SRCREG(op));                                      \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::or_a(uint16_t op) { OR(A); }
void tms340x0_device::or_b(uint16_t op) { OR(B); }

#define ORI(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd |= PARAM_LONG();                                     \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::ori_a(uint16_t op) { ORI(A); }
void tms340x0_device::ori_b(uint16_t op) { ORI(B); }

void tms340x0_device::setc(uint16_t op)
{
	SET_C_LOG(1);
	COUNT_CYCLES(1);
}

#define SETF(F)                                                 \
{                                                               \
	uint8_t shift = F ? 6 : 0;                                    \
	m_st &= ~(0x3f << shift);                                \
	m_st |= (op & 0x3f) << shift;                        \
	COUNT_CYCLES(1+F);                                          \
}
void tms340x0_device::setf0(uint16_t op) { SETF(0); }
void tms340x0_device::setf1(uint16_t op) { SETF(1); }

#define SEXT(F,R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZ();                                                        \
	SEXTEND(*rd,FW(F));                                         \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::sext0_a(uint16_t op) { SEXT(0,A); }
void tms340x0_device::sext0_b(uint16_t op) { SEXT(0,B); }
void tms340x0_device::sext1_a(uint16_t op) { SEXT(1,A); }
void tms340x0_device::sext1_b(uint16_t op) { SEXT(1,B); }

#define RL(R,K)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t res = *rd;                                            \
	int32_t k = (K);                                              \
	CLR_CZ();                                                        \
	if (k)                                                      \
	{                                                           \
		res<<=(k-1);                                            \
		SET_C_BIT_HI(res, 31);                                 \
		res<<=1;                                                \
		res |= (((uint32_t)*rd)>>((-k)&0x1f));                    \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(res);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::rl_k_a(uint16_t op) { RL(A,PARAM_K(op)); }
void tms340x0_device::rl_k_b(uint16_t op) { RL(B,PARAM_K(op)); }
void tms340x0_device::rl_r_a(uint16_t op) { RL(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::rl_r_b(uint16_t op) { RL(B,BREG(SRCREG(op))&0x1f); }

#define SLA(R,K)                                                \
{                                                               \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t res = *rd;                                           \
		int32_t k = K;                                                \
	CLR_NCZV();                                                  \
	if (k)                                                      \
	{                                                           \
		uint32_t mask = (0xffffffff<<(31-k))&0x7fffffff;          \
		uint32_t res2 = SIGN(res) ? res^mask : res;               \
		SET_V_LOG((res2 & mask) != 0);                         \
																\
		res<<=(k-1);                                            \
		SET_C_BIT_HI(res, 31);                                 \
		res<<=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_NZ_VAL(res);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::sla_k_a(uint16_t op) { SLA(A,PARAM_K(op)); }
void tms340x0_device::sla_k_b(uint16_t op) { SLA(B,PARAM_K(op)); }
void tms340x0_device::sla_r_a(uint16_t op) { SLA(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::sla_r_b(uint16_t op) { SLA(B,BREG(SRCREG(op))&0x1f); }

#define SLL(R,K)                                                \
{                                                               \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t res = *rd;                                           \
		int32_t k = K;                                                \
	CLR_CZ();                                                        \
	if (k)                                                      \
	{                                                           \
		res<<=(k-1);                                            \
		SET_C_BIT_HI(res, 31);                                 \
		res<<=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(res);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::sll_k_a(uint16_t op) { SLL(A,PARAM_K(op)); }
void tms340x0_device::sll_k_b(uint16_t op) { SLL(B,PARAM_K(op)); }
void tms340x0_device::sll_r_a(uint16_t op) { SLL(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::sll_r_b(uint16_t op) { SLL(B,BREG(SRCREG(op))&0x1f); }

#define SRA(R,K)                                                \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t res = *rd;                                            \
	int32_t k = (-(K)) & 0x1f;                                    \
	CLR_NCZ();                                                   \
	if (k)                                                      \
	{                                                           \
		res>>=(k-1);                                            \
		SET_C_BIT_LO(res, 0);                                  \
		res>>=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_NZ_VAL(res);                                           \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::sra_k_a(uint16_t op) { SRA(A,PARAM_K(op)); }
void tms340x0_device::sra_k_b(uint16_t op) { SRA(B,PARAM_K(op)); }
void tms340x0_device::sra_r_a(uint16_t op) { SRA(A,AREG(SRCREG(op))); }
void tms340x0_device::sra_r_b(uint16_t op) { SRA(B,BREG(SRCREG(op))); }

#define SRL(R,K)                                                \
{                                                               \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	uint32_t res = *rd;                                           \
		int32_t k = (-(K)) & 0x1f;                                    \
	CLR_CZ();                                                        \
	if (k)                                                      \
	{                                                           \
		res>>=(k-1);                                            \
		SET_C_BIT_LO(res, 0);                                  \
		res>>=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(res);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::srl_k_a(uint16_t op) { SRL(A,PARAM_K(op)); }
void tms340x0_device::srl_k_b(uint16_t op) { SRL(B,PARAM_K(op)); }
void tms340x0_device::srl_r_a(uint16_t op) { SRL(A,AREG(SRCREG(op))); }
void tms340x0_device::srl_r_b(uint16_t op) { SRL(B,BREG(SRCREG(op))); }

#define SUB(R)                                                  \
{                                                               \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t r = *rd - *rs;                                        \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,*rs,r);                                    \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::sub_a(uint16_t op) { SUB(A); }
void tms340x0_device::sub_b(uint16_t op) { SUB(B); }

#define SUBB(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = R##REG(SRCREG(op));                                   \
	int32_t r = *rd - t - (C_FLAG() ? 1 : 0);                      \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::subb_a(uint16_t op) { SUBB(A); }
void tms340x0_device::subb_b(uint16_t op) { SUBB(B); }

#define SUBI_W(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t r;                                                    \
	int32_t t = ~PARAM_WORD();                                 \
	CLR_NCZV();                                                  \
	r = *rd - t;                                                \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::subi_w_a(uint16_t op) { SUBI_W(A); }
void tms340x0_device::subi_w_b(uint16_t op) { SUBI_W(B); }

#define SUBI_L(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = ~PARAM_LONG();                                 \
	int32_t r = *rd - t;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::subi_l_a(uint16_t op) { SUBI_L(A); }
void tms340x0_device::subi_l_b(uint16_t op) { SUBI_L(B); }

#define SUBK(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t t = fw_inc[PARAM_K(op)];                                  \
	int32_t r = *rd - t;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::subk_a(uint16_t op) { SUBK(A); }
void tms340x0_device::subk_b(uint16_t op) { SUBK(B); }

#define XOR(R)                                                  \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd ^= R##REG(SRCREG(op));                                      \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::xor_a(uint16_t op) { XOR(A); }
void tms340x0_device::xor_b(uint16_t op) { XOR(B); }

#define XORI(R)                                                 \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd ^= PARAM_LONG();                                     \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::xori_a(uint16_t op) { XORI(A); }
void tms340x0_device::xori_b(uint16_t op) { XORI(B); }

#define ZEXT(F,R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	ZEXTEND(*rd,FW(F));                                         \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::zext0_a(uint16_t op) { ZEXT(0,A); }
void tms340x0_device::zext0_b(uint16_t op) { ZEXT(0,B); }
void tms340x0_device::zext1_a(uint16_t op) { ZEXT(1,A); }
void tms340x0_device::zext1_b(uint16_t op) { ZEXT(1,B); }



/***************************************************************************
    MOVE INSTRUCTIONS
***************************************************************************/

#define MOVI_W(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd=PARAM_WORD();                                        \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::movi_w_a(uint16_t op) { MOVI_W(A); }
void tms340x0_device::movi_w_b(uint16_t op) { MOVI_W(B); }

#define MOVI_L(R)                                               \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd=PARAM_LONG();                                        \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movi_l_a(uint16_t op) { MOVI_L(A); }
void tms340x0_device::movi_l_b(uint16_t op) { MOVI_L(B); }

#define MOVK(R)                                                 \
{                                                               \
	int32_t k = PARAM_K(op); if (!k) k = 32;                          \
	R##REG(DSTREG(op)) = k;                                         \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::movk_a(uint16_t op) { MOVK(A); }
void tms340x0_device::movk_b(uint16_t op) { MOVK(B); }

#define MOVB_RN(R)                                              \
{                                                               \
	WBYTE(R##REG(DSTREG(op)),R##REG(SRCREG(op)));                      \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::movb_rn_a(uint16_t op) { MOVB_RN(A); }
void tms340x0_device::movb_rn_b(uint16_t op) { MOVB_RN(B); }

#define MOVB_NR(R)                                              \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = (int8_t)RBYTE(R##REG(SRCREG(op)));                     \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movb_nr_a(uint16_t op) { MOVB_NR(A); }
void tms340x0_device::movb_nr_b(uint16_t op) { MOVB_NR(B); }

#define MOVB_NN(R)                                              \
{                                                               \
	WBYTE(R##REG(DSTREG(op)),(uint32_t)(uint8_t)RBYTE(R##REG(SRCREG(op))));\
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movb_nn_a(uint16_t op) { MOVB_NN(A); }
void tms340x0_device::movb_nn_b(uint16_t op) { MOVB_NN(B); }

#define MOVB_R_NO(R)                                            \
{                                                               \
	int32_t o = PARAM_WORD();                                  \
	WBYTE(R##REG(DSTREG(op))+o,R##REG(SRCREG(op)));                        \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movb_r_no_a(uint16_t op) { MOVB_R_NO(A); }
void tms340x0_device::movb_r_no_b(uint16_t op) { MOVB_R_NO(B); }

#define MOVB_NO_R(R)                                            \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t o = PARAM_WORD();                                  \
	CLR_NZV();                                                   \
	*rd = (int8_t)RBYTE(R##REG(SRCREG(op))+o);                   \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::movb_no_r_a(uint16_t op) { MOVB_NO_R(A); }
void tms340x0_device::movb_no_r_b(uint16_t op) { MOVB_NO_R(B); }

#define MOVB_NO_NO(R)                                           \
{                                                               \
	int32_t o1 = PARAM_WORD();                                 \
	int32_t o2 = PARAM_WORD();                                 \
	WBYTE(R##REG(DSTREG(op))+o2,(uint32_t)(uint8_t)RBYTE(R##REG(SRCREG(op))+o1)); \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::movb_no_no_a(uint16_t op) { MOVB_NO_NO(A); }
void tms340x0_device::movb_no_no_b(uint16_t op) { MOVB_NO_NO(B); }

#define MOVB_RA(R)                                              \
{                                                               \
	WBYTE(PARAM_LONG(),R##REG(DSTREG(op)));                     \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::movb_ra_a(uint16_t op) { MOVB_RA(A); }
void tms340x0_device::movb_ra_b(uint16_t op) { MOVB_RA(B); }

#define MOVB_AR(R)                                              \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = (int8_t)RBYTE(PARAM_LONG());                    \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::movb_ar_a(uint16_t op) { MOVB_AR(A); }
void tms340x0_device::movb_ar_b(uint16_t op) { MOVB_AR(B); }

void tms340x0_device::movb_aa(uint16_t op)
{
	uint32_t bitaddrs=PARAM_LONG();
	WBYTE(PARAM_LONG(),(uint32_t)(uint8_t)RBYTE(bitaddrs));
	COUNT_CYCLES(6);
}

#define MOVE_RR(RS,RD)                                          \
{                                                               \
	int32_t *rd = &RD##REG(DSTREG(op));                               \
	CLR_NZV();                                                   \
	*rd = RS##REG(SRCREG(op));                                      \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::move_rr_a (uint16_t op) { MOVE_RR(A,A); }
void tms340x0_device::move_rr_b (uint16_t op) { MOVE_RR(B,B); }
void tms340x0_device::move_rr_ax(uint16_t op) { MOVE_RR(A,B); }
void tms340x0_device::move_rr_bx(uint16_t op) { MOVE_RR(B,A); }

#define MOVE_RN(F,R)                                            \
{                                                               \
	WFIELD##F(R##REG(DSTREG(op)),R##REG(SRCREG(op)));                   \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::move0_rn_a (uint16_t op) { MOVE_RN(0,A); }
void tms340x0_device::move0_rn_b (uint16_t op) { MOVE_RN(0,B); }
void tms340x0_device::move1_rn_a (uint16_t op) { MOVE_RN(1,A); }
void tms340x0_device::move1_rn_b (uint16_t op) { MOVE_RN(1,B); }

#define MOVE_R_DN(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	*rd-=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,R##REG(SRCREG(op)));                              \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::move0_r_dn_a (uint16_t op) { MOVE_R_DN(0,A); }
void tms340x0_device::move0_r_dn_b (uint16_t op) { MOVE_R_DN(0,B); }
void tms340x0_device::move1_r_dn_a (uint16_t op) { MOVE_R_DN(1,A); }
void tms340x0_device::move1_r_dn_b (uint16_t op) { MOVE_R_DN(1,B); }

#define MOVE_R_NI(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	WFIELD##F(*rd,R##REG(SRCREG(op)));                              \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::move0_r_ni_a (uint16_t op) { MOVE_R_NI(0,A); }
void tms340x0_device::move0_r_ni_b (uint16_t op) { MOVE_R_NI(0,B); }
void tms340x0_device::move1_r_ni_a (uint16_t op) { MOVE_R_NI(1,A); }
void tms340x0_device::move1_r_ni_b (uint16_t op) { MOVE_R_NI(1,B); }

#define MOVE_NR(F,R)                                            \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(R##REG(SRCREG(op)));                            \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_nr_a (uint16_t op) { MOVE_NR(0,A); }
void tms340x0_device::move0_nr_b (uint16_t op) { MOVE_NR(0,B); }
void tms340x0_device::move1_nr_a (uint16_t op) { MOVE_NR(1,A); }
void tms340x0_device::move1_nr_b (uint16_t op) { MOVE_NR(1,B); }

#define MOVE_DN_R(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	CLR_NZV();                                                   \
	*rs-=fw_inc[FW(F)];                                         \
	*rd = RFIELD##F(*rs);                                       \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(4);                                            \
}
void tms340x0_device::move0_dn_r_a (uint16_t op) { MOVE_DN_R(0,A); }
void tms340x0_device::move0_dn_r_b (uint16_t op) { MOVE_DN_R(0,B); }
void tms340x0_device::move1_dn_r_a (uint16_t op) { MOVE_DN_R(1,A); }
void tms340x0_device::move1_dn_r_b (uint16_t op) { MOVE_DN_R(1,B); }

#define MOVE_NI_R(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t data = RFIELD##F(*rs);                                \
	CLR_NZV();                                                   \
	*rs+=fw_inc[FW(F)];                                         \
	*rd = data;                                                 \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_ni_r_a (uint16_t op) { MOVE_NI_R(0,A); }
void tms340x0_device::move0_ni_r_b (uint16_t op) { MOVE_NI_R(0,B); }
void tms340x0_device::move1_ni_r_a (uint16_t op) { MOVE_NI_R(1,A); }
void tms340x0_device::move1_ni_r_b (uint16_t op) { MOVE_NI_R(1,B); }

#define MOVE_NN(F,R)                                            \
{                                                               \
	WFIELD##F(R##REG(DSTREG(op)),RFIELD##F(R##REG(SRCREG(op))));        \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_nn_a (uint16_t op) { MOVE_NN(0,A); }
void tms340x0_device::move0_nn_b (uint16_t op) { MOVE_NN(0,B); }
void tms340x0_device::move1_nn_a (uint16_t op) { MOVE_NN(1,A); }
void tms340x0_device::move1_nn_b (uint16_t op) { MOVE_NN(1,B); }

#define MOVE_DN_DN(F,R)                                         \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t data;                                                 \
	*rs-=fw_inc[FW(F)];                                         \
	data = RFIELD##F(*rs);                                      \
	*rd-=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,data);                                        \
	COUNT_CYCLES(4);                                            \
}
void tms340x0_device::move0_dn_dn_a (uint16_t op) { MOVE_DN_DN(0,A); }
void tms340x0_device::move0_dn_dn_b (uint16_t op) { MOVE_DN_DN(0,B); }
void tms340x0_device::move1_dn_dn_a (uint16_t op) { MOVE_DN_DN(1,A); }
void tms340x0_device::move1_dn_dn_b (uint16_t op) { MOVE_DN_DN(1,B); }

#define MOVE_NI_NI(F,R)                                         \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t *rs = &R##REG(SRCREG(op));                                \
	int32_t data = RFIELD##F(*rs);                                \
	*rs+=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,data);                                        \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(4);                                            \
}
void tms340x0_device::move0_ni_ni_a (uint16_t op) { MOVE_NI_NI(0,A); }
void tms340x0_device::move0_ni_ni_b (uint16_t op) { MOVE_NI_NI(0,B); }
void tms340x0_device::move1_ni_ni_a (uint16_t op) { MOVE_NI_NI(1,A); }
void tms340x0_device::move1_ni_ni_b (uint16_t op) { MOVE_NI_NI(1,B); }

#define MOVE_R_NO(F,R)                                          \
{                                                               \
	int32_t o = PARAM_WORD();                                  \
	WFIELD##F(R##REG(DSTREG(op))+o,R##REG(SRCREG(op)));                 \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_r_no_a (uint16_t op) { MOVE_R_NO(0,A); }
void tms340x0_device::move0_r_no_b (uint16_t op) { MOVE_R_NO(0,B); }
void tms340x0_device::move1_r_no_a (uint16_t op) { MOVE_R_NO(1,A); }
void tms340x0_device::move1_r_no_b (uint16_t op) { MOVE_R_NO(1,B); }

#define MOVE_NO_R(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t o = PARAM_WORD();                                  \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(R##REG(SRCREG(op))+o);                          \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_no_r_a (uint16_t op) { MOVE_NO_R(0,A); }
void tms340x0_device::move0_no_r_b (uint16_t op) { MOVE_NO_R(0,B); }
void tms340x0_device::move1_no_r_a (uint16_t op) { MOVE_NO_R(1,A); }
void tms340x0_device::move1_no_r_b (uint16_t op) { MOVE_NO_R(1,B); }

#define MOVE_NO_NI(F,R)                                         \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t o = PARAM_WORD();                                  \
	int32_t data = RFIELD##F(R##REG(SRCREG(op))+o);                   \
	WFIELD##F(*rd,data);                                        \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_no_ni_a (uint16_t op) { MOVE_NO_NI(0,A); }
void tms340x0_device::move0_no_ni_b (uint16_t op) { MOVE_NO_NI(0,B); }
void tms340x0_device::move1_no_ni_a (uint16_t op) { MOVE_NO_NI(1,A); }
void tms340x0_device::move1_no_ni_b (uint16_t op) { MOVE_NO_NI(1,B); }

#define MOVE_NO_NO(F,R)                                         \
{                                                               \
	int32_t o1 = PARAM_WORD();                                 \
	int32_t o2 = PARAM_WORD();                                 \
	int32_t data = RFIELD##F(R##REG(SRCREG(op))+o1);                  \
	WFIELD##F(R##REG(DSTREG(op))+o2,data);                          \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_no_no_a (uint16_t op) { MOVE_NO_NO(0,A); }
void tms340x0_device::move0_no_no_b (uint16_t op) { MOVE_NO_NO(0,B); }
void tms340x0_device::move1_no_no_a (uint16_t op) { MOVE_NO_NO(1,A); }
void tms340x0_device::move1_no_no_b (uint16_t op) { MOVE_NO_NO(1,B); }

#define MOVE_RA(F,R)                                            \
{                                                               \
	WFIELD##F(PARAM_LONG(),R##REG(DSTREG(op)));                  \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_ra_a (uint16_t op) { MOVE_RA(0,A); }
void tms340x0_device::move0_ra_b (uint16_t op) { MOVE_RA(0,B); }
void tms340x0_device::move1_ra_a (uint16_t op) { MOVE_RA(1,A); }
void tms340x0_device::move1_ra_b (uint16_t op) { MOVE_RA(1,B); }

#define MOVE_AR(F,R)                                            \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(PARAM_LONG());                           \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_ar_a (uint16_t op) { MOVE_AR(0,A); }
void tms340x0_device::move0_ar_b (uint16_t op) { MOVE_AR(0,B); }
void tms340x0_device::move1_ar_a (uint16_t op) { MOVE_AR(1,A); }
void tms340x0_device::move1_ar_b (uint16_t op) { MOVE_AR(1,B); }

#define MOVE_A_NI(F,R)                                          \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	WFIELD##F(*rd,RFIELD##F(PARAM_LONG()));                  \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_a_ni_a (uint16_t op) { MOVE_A_NI(0,A); }
void tms340x0_device::move0_a_ni_b (uint16_t op) { MOVE_A_NI(0,B); }
void tms340x0_device::move1_a_ni_a (uint16_t op) { MOVE_A_NI(1,A); }
void tms340x0_device::move1_a_ni_b (uint16_t op) { MOVE_A_NI(1,B); }

#define MOVE_AA(F)                                              \
{                                                               \
	uint32_t bitaddrs=PARAM_LONG();                            \
	WFIELD##F(PARAM_LONG(),RFIELD##F(bitaddrs));             \
	COUNT_CYCLES(7);                                            \
}
void tms340x0_device::move0_aa (uint16_t op) { MOVE_AA(0); }
void tms340x0_device::move1_aa (uint16_t op) { MOVE_AA(1); }



/***************************************************************************
    PROGRAM CONTROL INSTRUCTIONS
***************************************************************************/

#define CALL(R)                                                 \
{                                                               \
	PUSH(m_pc);                                                 \
	m_pc = R##REG(DSTREG(op));                                       \
	CORRECT_ODD_PC("CALL");                                     \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::call_a (uint16_t op) { CALL(A); }
void tms340x0_device::call_b (uint16_t op) { CALL(B); }

void tms340x0_device::callr(uint16_t op)
{
	PUSH(m_pc+0x10);
	m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;
	COUNT_CYCLES(3);
}

void tms340x0_device::calla(uint16_t op)
{
	PUSH(m_pc+0x20);
	m_pc = PARAM_LONG_NO_INC();
	CORRECT_ODD_PC("CALLA");
	COUNT_CYCLES(4);
}

#define DSJ(R)                                                  \
{                                                               \
	if (--R##REG(DSTREG(op)))                                       \
	{                                                           \
		m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                    \
		COUNT_CYCLES(3);                                        \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD();                                             \
		COUNT_CYCLES(2);                                        \
	}                                                           \
}
void tms340x0_device::dsj_a (uint16_t op) { DSJ(A); }
void tms340x0_device::dsj_b (uint16_t op) { DSJ(B); }

#define DSJEQ(R)                                                \
{                                                               \
	if (Z_FLAG())                                                    \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD();                                         \
			COUNT_CYCLES(2);                                    \
		}                                                       \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD();                                             \
		COUNT_CYCLES(2);                                        \
	}                                                           \
}
void tms340x0_device::dsjeq_a (uint16_t op) { DSJEQ(A); }
void tms340x0_device::dsjeq_b (uint16_t op) { DSJEQ(B); }

#define DSJNE(R)                                                \
{                                                               \
	if (!Z_FLAG())                                               \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD();                                         \
			COUNT_CYCLES(2);                                    \
		}                                                       \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD();                                             \
		COUNT_CYCLES(2);                                        \
	}                                                           \
}
void tms340x0_device::dsjne_a (uint16_t op) { DSJNE(A); }
void tms340x0_device::dsjne_b (uint16_t op) { DSJNE(B); }

#define DSJS(R)                                                 \
{                                                               \
	if (op & 0x0400)                                        \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc -= ((PARAM_K(op))<<4);                              \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(3);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (--R##REG(DSTREG(op)))                                   \
		{                                                       \
			m_pc += ((PARAM_K(op))<<4);                              \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(3);                                    \
	}                                                           \
}
void tms340x0_device::dsjs_a (uint16_t op) { DSJS(A); }
void tms340x0_device::dsjs_b (uint16_t op) { DSJS(B); }

void tms340x0_device::emu(uint16_t op)
{
	/* in RUN state, this instruction is a NOP */
	COUNT_CYCLES(6);
}

#define EXGPC(R)                                                \
{                                                               \
	int32_t *rd = &R##REG(DSTREG(op));                                \
	int32_t temppc = *rd;                                         \
	*rd = m_pc;                                                  \
	m_pc = temppc;                                               \
	CORRECT_ODD_PC("EXGPC");                                    \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::exgpc_a (uint16_t op) { EXGPC(A); }
void tms340x0_device::exgpc_b (uint16_t op) { EXGPC(B); }

#define GETPC(R)                                                \
{                                                               \
	R##REG(DSTREG(op)) = m_pc;                                       \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::getpc_a (uint16_t op) { GETPC(A); }
void tms340x0_device::getpc_b (uint16_t op) { GETPC(B); }

#define GETST(R)                                                \
{                                                               \
	R##REG(DSTREG(op)) = m_st;                               \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::getst_a (uint16_t op) { GETST(A); }
void tms340x0_device::getst_b (uint16_t op) { GETST(B); }

#define j_xx_8(TAKE)                                            \
{                                                               \
	if (DSTREG(op))                                                 \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc += (PARAM_REL8(op) << 4);                           \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(1);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc = PARAM_LONG_NO_INC();                       \
			CORRECT_ODD_PC("J_XX_8");                           \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_LONG();                                         \
			COUNT_CYCLES(4);                                    \
		}                                                       \
	}                                                           \
}

#define j_xx_0(TAKE)                                            \
{                                                               \
	if (DSTREG(op))                                             \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc += (PARAM_REL8(op) << 4);                           \
			COUNT_CYCLES(2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(1);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;                \
			COUNT_CYCLES(3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD();                                         \
			COUNT_CYCLES(2);                                    \
		}                                                       \
	}                                                           \
}

#define j_xx_x(TAKE)                                            \
{                                                               \
	if (TAKE)                                                   \
	{                                                           \
		m_pc += (PARAM_REL8(op) << 4);                               \
		COUNT_CYCLES(2);                                        \
	}                                                           \
	else                                                        \
		COUNT_CYCLES(1);                                        \
}

void tms340x0_device::j_UC_0(uint16_t op)
{
	j_xx_0(1);
}
void tms340x0_device::j_UC_8(uint16_t op)
{
	j_xx_8(1);
}
void tms340x0_device::j_UC_x(uint16_t op)
{
	j_xx_x(1);
}
void tms340x0_device::j_P_0(uint16_t op)
{
	j_xx_0(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_P_8(uint16_t op)
{
	j_xx_8(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_P_x(uint16_t op)
{
	j_xx_x(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_LS_0(uint16_t op)
{
	j_xx_0(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_LS_8(uint16_t op)
{
	j_xx_8(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_LS_x(uint16_t op)
{
	j_xx_x(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_HI_0(uint16_t op)
{
	j_xx_0(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_HI_8(uint16_t op)
{
	j_xx_8(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_HI_x(uint16_t op)
{
	j_xx_x(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_LT_0(uint16_t op)
{
	j_xx_0((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_LT_8(uint16_t op)
{
	j_xx_8((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_LT_x(uint16_t op)
{
	j_xx_x((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_GE_0(uint16_t op)
{
	j_xx_0((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_GE_8(uint16_t op)
{
	j_xx_8((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_GE_x(uint16_t op)
{
	j_xx_x((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_LE_0(uint16_t op)
{
	j_xx_0((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_LE_8(uint16_t op)
{
	j_xx_8((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_LE_x(uint16_t op)
{
	j_xx_x((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_GT_0(uint16_t op)
{
	j_xx_0((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_GT_8(uint16_t op)
{
	j_xx_8((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_GT_x(uint16_t op)
{
	j_xx_x((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_C_0(uint16_t op)
{
	j_xx_0(C_FLAG());
}
void tms340x0_device::j_C_8(uint16_t op)
{
	j_xx_8(C_FLAG());
}
void tms340x0_device::j_C_x(uint16_t op)
{
	j_xx_x(C_FLAG());
}
void tms340x0_device::j_NC_0(uint16_t op)
{
	j_xx_0(!C_FLAG());
}
void tms340x0_device::j_NC_8(uint16_t op)
{
	j_xx_8(!C_FLAG());
}
void tms340x0_device::j_NC_x(uint16_t op)
{
	j_xx_x(!C_FLAG());
}
void tms340x0_device::j_EQ_0(uint16_t op)
{
	j_xx_0(Z_FLAG());
}
void tms340x0_device::j_EQ_8(uint16_t op)
{
	j_xx_8(Z_FLAG());
}
void tms340x0_device::j_EQ_x(uint16_t op)
{
	j_xx_x(Z_FLAG());
}
void tms340x0_device::j_NE_0(uint16_t op)
{
	j_xx_0(!Z_FLAG());
}
void tms340x0_device::j_NE_8(uint16_t op)
{
	j_xx_8(!Z_FLAG());
}
void tms340x0_device::j_NE_x(uint16_t op)
{
	j_xx_x(!Z_FLAG());
}
void tms340x0_device::j_V_0(uint16_t op)
{
	j_xx_0(V_FLAG());
}
void tms340x0_device::j_V_8(uint16_t op)
{
	j_xx_8(V_FLAG());
}
void tms340x0_device::j_V_x(uint16_t op)
{
	j_xx_x(V_FLAG());
}
void tms340x0_device::j_NV_0(uint16_t op)
{
	j_xx_0(!V_FLAG());
}
void tms340x0_device::j_NV_8(uint16_t op)
{
	j_xx_8(!V_FLAG());
}
void tms340x0_device::j_NV_x(uint16_t op)
{
	j_xx_x(!V_FLAG());
}
void tms340x0_device::j_N_0(uint16_t op)
{
	j_xx_0(N_FLAG());
}
void tms340x0_device::j_N_8(uint16_t op)
{
	j_xx_8(N_FLAG());
}
void tms340x0_device::j_N_x(uint16_t op)
{
	j_xx_x(N_FLAG());
}
void tms340x0_device::j_NN_0(uint16_t op)
{
	j_xx_0(!N_FLAG());
}
void tms340x0_device::j_NN_8(uint16_t op)
{
	j_xx_8(!N_FLAG());
}
void tms340x0_device::j_NN_x(uint16_t op)
{
	j_xx_x(!N_FLAG());
}

#define JUMP(R)                                                 \
{                                                               \
	m_pc = R##REG(DSTREG(op));                                       \
	CORRECT_ODD_PC("JUMP");                                     \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::jump_a (uint16_t op) { JUMP(A); }
void tms340x0_device::jump_b (uint16_t op) { JUMP(B); }

void tms340x0_device::popst(uint16_t op)
{
	SET_ST(POP());
	COUNT_CYCLES(8);
}

void tms340x0_device::pushst(uint16_t op)
{
	PUSH(m_st);
	COUNT_CYCLES(2);
}

#define PUTST(R)                                                \
{                                                               \
	SET_ST(R##REG(DSTREG(op)));                                \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::putst_a (uint16_t op) { PUTST(A); }
void tms340x0_device::putst_b (uint16_t op) { PUTST(B); }

void tms340x0_device::reti(uint16_t op)
{
	int32_t st = POP();
	m_pc = POP();
	CORRECT_ODD_PC("RETI");
	SET_ST(st);
	COUNT_CYCLES(11);
}

void tms340x0_device::rets(uint16_t op)
{
	uint32_t offs;
	m_pc = POP();
	CORRECT_ODD_PC("RETS");
	offs = PARAM_N(op);
	if (offs)
	{
		SP()+=(offs<<4);
	}
	COUNT_CYCLES(7);
}

#define REV(R)                                                  \
{                                                               \
	R##REG(DSTREG(op)) = 0x0008;                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::rev_a (uint16_t op) { REV(A); }
void tms340x0_device::rev_b (uint16_t op) { REV(B); }

void tms340x0_device::trap(uint16_t op)
{
	uint32_t t = PARAM_N(op);
	if (t)
	{
		PUSH(m_pc);
		PUSH(m_st);
	}
	RESET_ST();
	m_pc = RLONG(0xffffffe0-(t<<5));
	CORRECT_ODD_PC("TRAP");
	COUNT_CYCLES(16);
}



/***************************************************************************
    34020 INSTRUCTIONS
***************************************************************************/

/************************************

New 34020 ops:

    0000 1100 000R dddd = ADDXYI IL,Rd
    iiii iiii iiii iiii
    iiii iiii iiii iiii

    0000 0000 1111 00SD = BLMOVE S,D

    0000 0110 0000 0000 = CEXEC S,c,ID,L
    cccc cccc S000 0000
    iiic cccc cccc cccc

    1101 1000 0ccc cccS = CEXEC S,c,ID
    iiic cccc cccc cccc

    0000 1000 1111 0010 = CLIP

    0000 0110 011R dddd = CMOVCG Rd1,Rd2,S,c,ID
    cccc cccc S00R dddd
    iiic cccc cccc cccc

    0000 0110 101R dddd = CMOVCM *Rd+,n,S,c,ID
    cccc cccc S00n nnnn
    iiic cccc cccc cccc

    0000 0110 110R dddd = CMOVCM -*Rd,n,S,c,ID
    cccc cccc S00n nnnn
    iiic cccc cccc cccc

    0000 0110 0110 0000 = CMOVCS c,ID
    cccc cccc 0000 0001
    iiic cccc cccc cccc

    0000 0110 001R ssss = CMOVGC Rs,c,ID
    cccc cccc 0000 0000
    iiic cccc cccc cccc

    0000 0110 010R ssss = CMOVGC Rs1,Rs2,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0000 0110 100n nnnn = CMOVMC *Rs+,n,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0000 1000 001n nnnn = CMOVMC -*Rs,n,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0000 0110 111R dddd = CMOVMC *Rs+,Rd,S,c,ID
    cccc cccc S00R ssss
    iiic cccc cccc cccc

    0011 01kk kkkR dddd = CMPK k,Rd

    0000 1010 100R dddd = CVDXYL Rd

    0000 1010 011R dddd = CVMXYL Rd

    1110 101s sssR dddd = CVSXYL Rs,Rd

    0000 0010 101R dddd = EXGPS Rd

    1101 1110 Z001 1010 = FLINE Z

    0000 1010 1011 1011 = FPIXEQ

    0000 1010 1101 1011 = FPIXNE

    0000 0010 110R dddd = GETPS Rd

    0000 0000 0100 0000 = IDLE

    0000 1100 0101 0111 = LINIT

    0000 0000 1000 0000 = MWAIT

    0000 1010 0011 0111 = PFILL XY

    0000 1110 0001 0111 = PIXBLT L,M,L

    0000 1000 0110 0000 = RETM

    0111 101s sssR dddd = RMO Rs,Rd

    0000 0010 100R dddd = RPIX Rd

    0000 0010 0111 0011 = SETCDP

    0000 0010 1111 1011 = SETCMP

    0000 0010 0101 0001 = SETCSP

    0111 111s sssR dddd = SWAPF *Rs,Rd,0

    0000 1110 1111 1010 = TFILL XY

    0000 1000 0000 1111 = TRAPL

    0000 1000 0101 0111 = VBLT B,L

    0000 1010 0101 0111 = VFILL L

    0000 1010 0000 0000 = VLCOL

************************************/


#define ADD_XYI(R)                              \
{                                               \
	uint32_t a = PARAM_LONG();                 \
	XY *b = &R##REG_XY(DSTREG(op));                 \
	CLR_NCZV();                                  \
	b->x += (int16_t)(a & 0xffff);                \
	b->y += ((int32_t)a >> 16);                   \
	SET_N_LOG(b->x == 0);                      \
	SET_C_BIT_LO(b->y, 15);                        \
	SET_Z_LOG(b->y == 0);                      \
	SET_V_BIT_LO(b->x, 15);                        \
	COUNT_CYCLES(1);                            \
}
void tms34020_device::addxyi_a(uint16_t op)
{
	ADD_XYI(A);
}
void tms34020_device::addxyi_b(uint16_t op)
{
	ADD_XYI(B);
}

void tms34020_device::blmove(uint16_t op)
{
	offs_t src = BREG(0);
	offs_t dst = BREG(2);
	offs_t bits = BREG(7);

	bool S = op & (1 << 1);
	bool D = op & (1 << 0);

	if ((S == false && (src & 0xf)) || (D == false && (dst & 0xf))) {
		logerror("020:BLMOVE alignment error: PC=%x: S=%d, D=%d, src=%x, dst=%x, bits=%d\n", m_pc, S, D, src, dst, bits);
	}

	// logerror("020:BLMOVE: PC=%x: S=%d, D=%d, src=%x, dst=%x, bits=%d\n", m_pc, S, D, src, dst, bits);
	while (bits >= 16 && m_icount > 0)
	{
		TMS34010_WRMEM_WORD(dst, TMS34010_RDMEM_WORD(src));
		src += 0x10;
		dst += 0x10;
		bits -= 0x10;
		m_icount -= 2;
	}
	if (bits != 0 && m_icount > 0)
	{
		(this->*s_wfield_functions[bits])(dst, (this->*s_rfield_functions[bits])(src));
		dst += bits;
		src += bits;
		bits = 0;
		m_icount -= 2;
	}

	/*
	    TODO: We do not currently emulate precisely how B0 and B2 are modified during the operation:
	    if D == 0, then B0 and B2 remain fixed during execution and are only incremented after operation completes.
	    if D == 1, then B2 is incremented during move, B0 remains fixed until operation completes.
	*/

	BREG(0) = src;
	BREG(2) = dst;
	BREG(7) = bits;

	// logerror("020:BLMOVE: PC=%x: finished: B0=%x, B2=%x, B7=%d\n", m_pc, src, dst, bits);

	/* if we're not done yet, back up the PC */
	if (bits != 0)
		m_pc -= 0x10;
}

void tms34020_device::cexec_l(uint16_t op)
{
	logerror("020:cexec_l\n");
}

void tms34020_device::cexec_s(uint16_t op)
{
	logerror("020:cexec_s\n");
}

void tms34020_device::clip(uint16_t op)
{
	XY daddr = DADDR_XY();
	XY wstart = WSTART_XY();
	XY wend = WEND_XY();
	XY dydx = DYDX_XY();
	// logerror("020:clip PC=0x%08x: WSTART=(%dx%d) WEND=(%dx%d) DADDR=(%dx%d) DYDX=(%dx%d)\n",
	//      m_pc, wstart.x, wstart.y, wend.x, wend.y, daddr.x, daddr.y, dydx.x, dydx.y);

	// Check whether array intersects with window...
	bool is_l = wstart.x < (daddr.x + dydx.x);
	bool is_r = wend.x > daddr.x;
	bool is_t = wstart.y < (daddr.y + dydx.y);
	bool is_b = wend.y > daddr.y;
	if (!(is_l || is_r || is_t || is_b))
	{
		// ...no intersection, set flags and return
		m_st |= STBIT_Z | STBIT_V;
		// TODO: manual does not specify cycles, only states that this is complex instruction
		COUNT_CYCLES(3);
		return;
	}

	CLR_V();
	CLR_Z();

	// Handle clipping if needed
	bool array_clipped = false;

	if (wstart.x > daddr.x)
	{
		DADDR_X() = wstart.x;
		array_clipped = true;
	}
	if (wend.x < (daddr.x + dydx.x - 1))
	{
		DYDX_X() = wend.x - daddr.x;
		array_clipped = true;
	}

	if (wstart.y > daddr.y)
	{
		DADDR_Y() = wstart.y;
		array_clipped = true;
	}
	if (wend.y < (daddr.y + dydx.y - 1))
	{
		DYDX_Y() = wend.y - daddr.y;
		array_clipped = true;
	}

	if (array_clipped)
		m_st |= STBIT_V;

	// TODO: manual does not specify cycles, only states that this is complex instruction
	COUNT_CYCLES(3);
}

void tms34020_device::cmovcg_a(uint16_t op)
{
	logerror("020:cmovcg_a\n");
}

void tms34020_device::cmovcg_b(uint16_t op)
{
	logerror("020:cmovcg_b\n");
}

void tms34020_device::cmovcm_f(uint16_t op)
{
	logerror("020:cmovcm_f\n");
}

void tms34020_device::cmovcm_b(uint16_t op)
{
	logerror("020:cmovcm_b\n");
}

void tms34020_device::cmovgc_a(uint16_t op)
{
	logerror("020:cmovgc_a\n");
}

void tms34020_device::cmovgc_b(uint16_t op)
{
	logerror("020:cmovgc_b\n");
}

void tms34020_device::cmovgc_a_s(uint16_t op)
{
	logerror("020:cmovgc_a_s\n");
}

void tms34020_device::cmovgc_b_s(uint16_t op)
{
	logerror("020:cmovgc_b_s\n");
}

void tms34020_device::cmovmc_f(uint16_t op)
{
	logerror("020:cmovmc_f\n");
}

void tms34020_device::cmovmc_f_va(uint16_t op)
{
	logerror("020:cmovmc_f_va\n");
}

void tms34020_device::cmovmc_f_vb(uint16_t op)
{
	logerror("020:cmovmc_f_vb\n");
}

void tms34020_device::cmovmc_b(uint16_t op)
{
	logerror("020:cmovmc_b\n");
}

#define CMPK(R)                                             \
{                                                           \
	int32_t r;                                                \
	int32_t *rd = &R##REG(DSTREG(op));                            \
	int32_t t = PARAM_K(op); if (!t) t = 32;                      \
	CLR_NCZV();                                              \
	r = *rd - t;                                            \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(1);                                        \
}
void tms34020_device::cmp_k_a(uint16_t op)
{
	CMPK(A);
}
void tms34020_device::cmp_k_b(uint16_t op)
{
	CMPK(B);
}

void tms34020_device::cvdxyl_a(uint16_t op)
{
	logerror("020:cvdxyl_a\n");
}

void tms34020_device::cvdxyl_b(uint16_t op)
{
	// Convert destination XY address to linear
	// (Y half of Rd x DPTCH) + (X half of Rd x PSIZE) + (A4 or B4) ->  Rd
	const XY rd = BREG_XY(DSTREG(op));
	const off_t dptch = DPTCH();
	const off_t offset = BREG(4);

	const off_t result = (rd.y * dptch) + (rd.x << m_pixelshift) + offset;
	// logerror("020:cvdxyl_b: PC=0x%08x: DADDR=%x, x=%x, y=%x, DPTCH=%d, PIXELSIZE=%d, OFFSET=%d, result: %x\n",
	// m_pc, rd, rd.x, rd.y, dptch, m_pixelshift, offset, result);

	DADDR() = result;

	// Handle cycles related to pitch:
	switch (population_count_32(dptch))
	{
		// power of 2: 2 clocks
		case 1:
			COUNT_CYCLES(2);
			break;
		// 2 powers of 2: 3 clocks
		case 2:
			COUNT_CYCLES(3);
			break;
		// arbitrary: 14 clocks
		default:
			COUNT_CYCLES(14);
			break;
	}
}

void tms34020_device::cvmxyl_a(uint16_t op)
{
	logerror("020:cvmxyl_a\n");
}

void tms34020_device::cvmxyl_b(uint16_t op)
{
	logerror("020:cvmxyl_b\n");
}

void tms34020_device::cvsxyl_a(uint16_t op)
{
	logerror("020:cvsxyl_a\n");
}

void tms34020_device::cvsxyl_b(uint16_t op)
{
	logerror("020:cvsxyl_b\n");
}

void tms34020_device::exgps_a(uint16_t op)
{
	logerror("020:exgps_a\n");
}

void tms34020_device::exgps_b(uint16_t op)
{
	logerror("020:exgps_b\n");
}

void tms34020_device::fline(uint16_t op)
{
	logerror("020:fline\n");
}

void tms34020_device::fpixeq(uint16_t op)
{
	logerror("020:fpixeq\n");
}

void tms34020_device::fpixne(uint16_t op)
{
	logerror("020:fpixne\n");
}

void tms34020_device::getps_a(uint16_t op)
{
	logerror("020:getps_a\n");
}

void tms34020_device::getps_b(uint16_t op)
{
	logerror("020:getps_b\n");
}

void tms34020_device::idle(uint16_t op)
{
	logerror("020:idle\n");
}

void tms34020_device::linit(uint16_t op)
{
	logerror("020:linit\n");
}

void tms34020_device::mwait(uint16_t op)
{
}

void tms34020_device::pfill_xy(uint16_t op)
{
	logerror("020:pfill_xy\n");
}

void tms34020_device::pixblt_l_m_l(uint16_t op)
{
	logerror("020:pixblt_l_m_l\n");
}

void tms34020_device::retm(uint16_t op)
{
	logerror("020:retm\n");
}

#define RMO(R)                                                  \
{                                                               \
	uint32_t res = 0;                                             \
	uint32_t rs  = R##REG(SRCREG(op));                                \
		int32_t *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	SET_Z_VAL(rs);                                             \
	if (rs)                                                     \
	{                                                           \
		while (!(rs & 0x00000001))                              \
		{                                                       \
			res++;                                              \
			rs >>= 1;                                           \
		}                                                       \
	}                                                           \
	*rd = res;                                                  \
	COUNT_CYCLES(1);                                            \
}

void tms34020_device::rmo_a(uint16_t op) { RMO(A); }
void tms34020_device::rmo_b(uint16_t op) { RMO(B); }

#define RPIX(R)                                 \
{                                               \
	uint32_t v = R##REG(DSTREG(op));                  \
	switch (m_pixelshift)                    \
	{                                           \
		case 0:                                 \
			v = (v & 1) ? 0xffffffff : 0x00000000;\
			COUNT_CYCLES(8);                    \
			break;                              \
		case 1:                                 \
			v &= 3;                             \
			v |= v << 2;                        \
			v |= v << 4;                        \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(7);                    \
			break;                              \
		case 2:                                 \
			v &= 0x0f;                          \
			v |= v << 4;                        \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(6);                    \
			break;                              \
		case 3:                                 \
			v &= 0xff;                          \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(5);                    \
			break;                              \
		case 4:                                 \
			v &= 0xffff;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(4);                    \
			break;                              \
		case 5:                                 \
			COUNT_CYCLES(2);                    \
			break;                              \
	}                                           \
	R##REG(DSTREG(op)) = v;                         \
}

void tms34020_device::rpix_a(uint16_t op)
{
	RPIX(A);
}

void tms34020_device::rpix_b(uint16_t op)
{
	RPIX(B);
}

void tms34020_device::setcdp(uint16_t op)
{
	off_t dptch = DPTCH();

	// Check whether we're dealing with an even number
	if ((dptch & 1) == 0)
	{
		switch (population_count_32(dptch))
		{
			// .. only single bit set, pitch is power of two!
			case 1:
			{
				m_convdp = 32 - count_leading_zeros_32(dptch);
				COUNT_CYCLES(4);
				return;
			}
			// .. two bits, we can decompose it to sum of two power of two numbers
			case 2:
			{
				uint8_t first_one = count_leading_zeros_32(dptch);
				uint8_t v1 = 32 - first_one;
				uint8_t v2 = 32 - count_leading_zeros_32(dptch & ~(1 << (first_one - 1)));

				m_convdp = v2 | (v1 << 8);
				COUNT_CYCLES(6);
				return;
			}
		}
	}
	// Default to arbitrary number, setting pitch to 0
	m_convdp = 0;
	COUNT_CYCLES(3);
}

void tms34020_device::setcmp(uint16_t op)
{
	logerror("020:setcmp\n");
}

void tms34020_device::setcsp(uint16_t op)
{
	logerror("020:setcsp\n");
}

void tms34020_device::swapf_a(uint16_t op)
{
	logerror("020:swapf_a\n");
}

void tms34020_device::swapf_b(uint16_t op)
{
	logerror("020:swapf_b\n");
}

void tms34020_device::tfill_xy(uint16_t op)
{
	logerror("020:tfill_xy\n");
}

void tms34020_device::trapl(uint16_t op)
{
	logerror("020:trapl\n");
}

void tms34020_device::vblt_b_l(uint16_t op)
{
	// Linear VRAM pixel block transfer
	// logerror("020:vblt_b_l: SADDR=0x%x, SPTCH=%d, DADDR=0x%x, DPTCH=%d, DYDX=(%dx%d), TEMP=%x\n", SADDR(), SPTCH(), DADDR(), DPTCH(), DYDX_X(), DYDX_Y(), TEMP());
	logerror("020:vblt_b_l\n");
}

void tms34020_device::vfill_l(uint16_t op)
{
	// Linear VRAM fast fill
	// logerror("020:vfill_l: DADDR=0x%x, DPTCH=%d, DYDX=(%dx%d)\n", DADDR(), DPTCH(), DYDX_X(), DYDX_Y());
	logerror("020:vfill_l\n");
}

void tms34020_device::vlcol(uint16_t op)
{
	// Latch COLOR1 into the VRAM color registers
	//logerror("020:vlcol: COLOR1=%x\n", COLOR1());
	logerror("020:vlcol\n");
}
