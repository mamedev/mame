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

#define ZEXTEND(val,width) if (width) (val) &= ((UINT32)0xffffffff >> (32 - (width)))
#define SEXTEND(val,width) if (width) (val) = (INT32)((val) << (32 - (width))) >> (32 - (width))

#define SXYTOL(val)   ((((INT16)(val).y * m_convsp) + ((INT16)(val).x << m_pixelshift)) + OFFSET())
#define DXYTOL(val)   ((((INT16)(val).y * m_convdp) + ((INT16)(val).x << m_pixelshift)) + OFFSET())
#define MXYTOL(val)   ((((INT16)(val).y * m_convmp) + ((INT16)(val).x << m_pixelshift)) + OFFSET())

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
#define SET_C_SUB(a,b)        SET_C_LOG((UINT32)(b) > (UINT32)(a))
#define SET_C_ADD(a,b)        SET_C_LOG((UINT32)~(a) < (UINT32)(b))
#define SET_NZV_SUB(a,b,r)    SET_NZ_VAL(r); SET_V_SUB(a,b,r)
#define SET_NZCV_SUB(a,b,r)   SET_NZV_SUB(a,b,r); SET_C_SUB(a,b)
#define SET_NZCV_ADD(a,b,r)   SET_NZ_VAL(r); SET_V_ADD(a,b,r); SET_C_ADD(a,b)

static const UINT8 fw_inc[32] = { 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };


/***************************************************************************
    UNIMPLEMENTED INSTRUCTION
***************************************************************************/

void tms340x0_device::unimpl(UINT16 op)
{
	/* kludge for Super High Impact -- this doesn't seem to cause */
	/* an illegal opcode exception */
	if (m_direct->read_word(TOBYTE(m_pc - 0x10)) == 0x0007)
		return;

	/* 9 Ball Shootout calls to FFDF7468, expecting it */
	/* to execute the next instruction from FFDF7470 */
	/* but the instruction at FFDF7460 is an 0x0001 */
	if (m_direct->read_word(TOBYTE(m_pc - 0x10)) == 0x0001)
		return;

	PUSH(m_pc);
	PUSH(m_st);
	RESET_ST();
	m_pc = RLONG(0xfffffc20);
	COUNT_UNKNOWN_CYCLES(16);

	/* extra check to prevent bad things */
	if (m_pc == 0 || s_opcode_table[m_direct->read_word(TOBYTE(m_pc)) >> 4] == &tms34010_device::unimpl)
	{
		set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		debugger_break(machine());
	}
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
void tms340x0_device::add_xy_a(UINT16 op) { ADD_XY(A); }
void tms340x0_device::add_xy_b(UINT16 op) { ADD_XY(B); }

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
void tms340x0_device::sub_xy_a(UINT16 op) { SUB_XY(A); }
void tms340x0_device::sub_xy_b(UINT16 op) { SUB_XY(B); }

#define CMP_XY(R)                               \
{                                               \
	INT16 res;                                  \
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
void tms340x0_device::cmp_xy_a(UINT16 op) { CMP_XY(A); }
void tms340x0_device::cmp_xy_b(UINT16 op) { CMP_XY(B); }

#define CPW(R)                                  \
{                                               \
	INT32 res = 0;                              \
	INT16 x = R##REG_X(SRCREG(op));                 \
	INT16 y = R##REG_Y(SRCREG(op));                 \
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
void tms340x0_device::cpw_a(UINT16 op) { CPW(A); }
void tms340x0_device::cpw_b(UINT16 op) { CPW(B); }

#define CVXYL(R)                                    \
{                                                   \
	R##REG(DSTREG(op)) = DXYTOL(R##REG_XY(SRCREG(op)));     \
	COUNT_CYCLES(3);                                \
}
void tms340x0_device::cvxyl_a(UINT16 op) { CVXYL(A); }
void tms340x0_device::cvxyl_b(UINT16 op) { CVXYL(B); }

#define MOVX(R)                                     \
{                                                   \
	R##REG(DSTREG(op)) = (R##REG(DSTREG(op)) & 0xffff0000) | (UINT16)R##REG(SRCREG(op));    \
	COUNT_CYCLES(1);                                                                    \
}
void tms340x0_device::movx_a(UINT16 op) { MOVX(A); }
void tms340x0_device::movx_b(UINT16 op) { MOVX(B); }

#define MOVY(R)                                     \
{                                                   \
	R##REG(DSTREG(op)) = (R##REG(SRCREG(op)) & 0xffff0000) | (UINT16)R##REG(DSTREG(op));    \
	COUNT_CYCLES(1);                                                                    \
}
void tms340x0_device::movy_a(UINT16 op) { MOVY(A); }
void tms340x0_device::movy_b(UINT16 op) { MOVY(B); }



/***************************************************************************
    PIXEL TRANSFER OPERATIONS
***************************************************************************/

#define PIXT_RI(R)                                  \
{                                                   \
	WPIXEL(R##REG(DSTREG(op)),R##REG(SRCREG(op)));  \
	COUNT_UNKNOWN_CYCLES(2);                        \
}
void tms340x0_device::pixt_ri_a(UINT16 op) { PIXT_RI(A); }
void tms340x0_device::pixt_ri_b(UINT16 op) { PIXT_RI(B); }

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
void tms340x0_device::pixt_rixy_a(UINT16 op) { PIXT_RIXY(A); }
void tms340x0_device::pixt_rixy_b(UINT16 op) { PIXT_RIXY(B); }

#define PIXT_IR(R)                                  \
{                                                   \
	INT32 temp = RPIXEL(R##REG(SRCREG(op)));            \
	CLR_V();                                         \
	R##REG(DSTREG(op)) = temp;                          \
	SET_V_LOG(temp != 0);                          \
	COUNT_CYCLES(4);                                \
}
void tms340x0_device::pixt_ir_a(UINT16 op) { PIXT_IR(A); }
void tms340x0_device::pixt_ir_b(UINT16 op) { PIXT_IR(B); }

#define PIXT_II(R)                                  \
{                                                   \
	WPIXEL(R##REG(DSTREG(op)),RPIXEL(R##REG(SRCREG(op))));  \
	COUNT_UNKNOWN_CYCLES(4);                        \
}
void tms340x0_device::pixt_ii_a(UINT16 op) { PIXT_II(A); }
void tms340x0_device::pixt_ii_b(UINT16 op) { PIXT_II(B); }

#define PIXT_IXYR(R)                                \
{                                                   \
	INT32 temp = RPIXEL(SXYTOL(R##REG_XY(SRCREG(op)))); \
	CLR_V();                                         \
	R##REG(DSTREG(op)) = temp;                          \
	SET_V_LOG(temp != 0);                          \
	COUNT_CYCLES(6);                                \
}
void tms340x0_device::pixt_ixyr_a(UINT16 op) { PIXT_IXYR(A); }
void tms340x0_device::pixt_ixyr_b(UINT16 op) { PIXT_IXYR(B); }

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
void tms340x0_device::pixt_ixyixy_a(UINT16 op) { PIXT_IXYIXY(A); }
void tms340x0_device::pixt_ixyixy_b(UINT16 op) { PIXT_IXYIXY(B); }

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
void tms340x0_device::drav_a(UINT16 op) { DRAV(A); }
void tms340x0_device::drav_b(UINT16 op) { DRAV(B); }



/***************************************************************************
    ARITHMETIC OPERATIONS
***************************************************************************/

#define ABS(R)                                              \
{                                                           \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 r = 0 - *rd;                                      \
	CLR_NZV();                                               \
	if (r > 0) *rd = r;                                     \
	SET_NZ_VAL(r);                                         \
	SET_V_LOG(r == (INT32)0x80000000);                     \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::abs_a(UINT16 op) { ABS(A); }
void tms340x0_device::abs_b(UINT16 op) { ABS(B); }

#define ADD(R)                                              \
{                                                           \
	INT32 a = R##REG(SRCREG(op));                               \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::add_a(UINT16 op) { ADD(A); }
void tms340x0_device::add_b(UINT16 op) { ADD(B); }

#define ADDC(R)                                             \
{                                                           \
	/* I'm not sure to which side the carry is added to, should */  \
	/* verify it against the examples */                    \
	INT32 a = R##REG(SRCREG(op));                               \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b + (C_FLAG() ? 1 : 0);                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::addc_a(UINT16 op) { ADDC(A); }
void tms340x0_device::addc_b(UINT16 op) { ADDC(B); }

#define ADDI_W(R)                                           \
{                                                           \
	INT32 a = PARAM_WORD();                              \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(2);                                        \
}
void tms340x0_device::addi_w_a(UINT16 op) { ADDI_W(A); }
void tms340x0_device::addi_w_b(UINT16 op) { ADDI_W(B); }

#define ADDI_L(R)                                           \
{                                                           \
	INT32 a = PARAM_LONG();                              \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(3);                                        \
}
void tms340x0_device::addi_l_a(UINT16 op) { ADDI_L(A); }
void tms340x0_device::addi_l_b(UINT16 op) { ADDI_L(B); }

#define ADDK(R)                                             \
{                                                           \
	INT32 a = fw_inc[PARAM_K(op)];                              \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV();                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(a,b,r);                                    \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::addk_a(UINT16 op) { ADDK(A); }
void tms340x0_device::addk_b(UINT16 op) { ADDK(B); }

#define AND(R)                                              \
{                                                           \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= R##REG(SRCREG(op));                                  \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::and_a(UINT16 op) { AND(A); }
void tms340x0_device::and_b(UINT16 op) { AND(B); }

#define ANDI(R)                                             \
{                                                           \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= ~PARAM_LONG();                                \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(3);                                        \
}
void tms340x0_device::andi_a(UINT16 op) { ANDI(A); }
void tms340x0_device::andi_b(UINT16 op) { ANDI(B); }

#define ANDN(R)                                             \
{                                                           \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	CLR_Z();                                                 \
	*rd &= ~R##REG(SRCREG(op));                                 \
	SET_Z_VAL(*rd);                                            \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::andn_a(UINT16 op) { ANDN(A); }
void tms340x0_device::andn_b(UINT16 op) { ANDN(B); }

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
void tms340x0_device::btst_k_a(UINT16 op) { BTST_K(A); }
void tms340x0_device::btst_k_b(UINT16 op) { BTST_K(B); }

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
void tms340x0_device::btst_r_a(UINT16 op) { BTST_R(A); }
void tms340x0_device::btst_r_b(UINT16 op) { BTST_R(B); }

void tms340x0_device::clrc(UINT16 op)
{
	CLR_C();
	COUNT_CYCLES(1);
}

#define CMP(R)                                              \
{                                                           \
	INT32 *rs = &R##REG(SRCREG(op));                            \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 r = *rd - *rs;                                    \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,*rs,r);                                \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::cmp_a(UINT16 op) { CMP(A); }
void tms340x0_device::cmp_b(UINT16 op) { CMP(B); }

#define CMPI_W(R)                                           \
{                                                           \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 t = (INT16)~PARAM_WORD();                      \
	INT32 r = *rd - t;                                      \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(2);                                        \
}
void tms340x0_device::cmpi_w_a(UINT16 op) { CMPI_W(A); }
void tms340x0_device::cmpi_w_b(UINT16 op) { CMPI_W(B); }

#define CMPI_L(R)                                           \
{                                                           \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 t = ~PARAM_LONG();                             \
	INT32 r = *rd - t;                                      \
	CLR_NCZV();                                              \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(3);                                        \
}
void tms340x0_device::cmpi_l_a(UINT16 op) { CMPI_L(A); }
void tms340x0_device::cmpi_l_b(UINT16 op) { CMPI_L(B); }

void tms340x0_device::dint(UINT16 op)
{
	m_st &= ~STBIT_IE;
	COUNT_CYCLES(3);
}

#define DIVS(R)                                             \
{                                                           \
	INT32 *rs  = &R##REG(SRCREG(op));                           \
	INT32 *rd1 = &R##REG(DSTREG(op));                           \
	CLR_NZV();                                               \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			INT32 *rd2 = &R##REG(DSTREG(op)+1);                 \
			INT64 dividend = ((UINT64)*rd1 << 32) | (UINT32)*rd2; \
			INT64 quotient = dividend / *rs;                \
			INT32 remainder = dividend % *rs;               \
			UINT32 signbits = (INT32)quotient >> 31;        \
			if (EXTRACT_64HI(quotient) != signbits)         \
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
void tms340x0_device::divs_a(UINT16 op) { DIVS(A); }
void tms340x0_device::divs_b(UINT16 op) { DIVS(B); }

#define DIVU(R)                                             \
{                                                           \
	INT32 *rs  = &R##REG(SRCREG(op));                           \
	INT32 *rd1 = &R##REG(DSTREG(op));                           \
	CLR_ZV();                                                    \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			INT32 *rd2 = &R##REG(DSTREG(op)+1);                 \
			UINT64 dividend  = ((UINT64)*rd1 << 32) | (UINT32)*rd2; \
			UINT64 quotient  = dividend / (UINT32)*rs;      \
			UINT32 remainder = dividend % (UINT32)*rs;      \
			if (EXTRACT_64HI(quotient) != 0)                \
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
			*rd1 = (UINT32)*rd1 / (UINT32)*rs;              \
			SET_Z_VAL(*rd1);                               \
		}                                                   \
	}                                                       \
	COUNT_CYCLES(37);                                       \
}
void tms340x0_device::divu_a(UINT16 op) { DIVU(A); }
void tms340x0_device::divu_b(UINT16 op) { DIVU(B); }

void tms340x0_device::eint(UINT16 op)
{
	m_st |= STBIT_IE;
	check_interrupt();
	COUNT_CYCLES(3);
}

#define EXGF(F,R)                                               \
{                                                               \
	UINT8 shift = F ? 6 : 0;                                    \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	UINT32 temp = (m_st >> shift) & 0x3f;                    \
	m_st &= ~(0x3f << shift);                                \
	m_st |= (*rd & 0x3f) << shift;                           \
	*rd = temp;                                                 \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::exgf0_a(UINT16 op) { EXGF(0,A); }
void tms340x0_device::exgf0_b(UINT16 op) { EXGF(0,B); }
void tms340x0_device::exgf1_a(UINT16 op) { EXGF(1,A); }
void tms340x0_device::exgf1_b(UINT16 op) { EXGF(1,B); }

#define LMO(R)                                                  \
{                                                               \
	UINT32 res = 0;                                             \
	UINT32 rs  = R##REG(SRCREG(op));                                \
		INT32 *rd = &R##REG(DSTREG(op));                                \
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
void tms340x0_device::lmo_a(UINT16 op) { LMO(A); }
void tms340x0_device::lmo_b(UINT16 op) { LMO(B); }

#define MMFM(R)                                                 \
{                                                               \
	INT32 i;                                                    \
	UINT16 l = (UINT16) PARAM_WORD();                        \
	COUNT_CYCLES(3);                                            \
	{                                                           \
		INT32 rd = DSTREG(op);                                      \
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
void tms340x0_device::mmfm_a(UINT16 op) { MMFM(A); }
void tms340x0_device::mmfm_b(UINT16 op) { MMFM(B); }

#define MMTM(R)                                                 \
{                                                               \
	UINT32 i;                                                   \
	UINT16 l = (UINT16) PARAM_WORD();                        \
	COUNT_CYCLES(2);                                            \
	{                                                           \
		INT32 rd = DSTREG(op);                                      \
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
void tms340x0_device::mmtm_a(UINT16 op) { MMTM(A); }
void tms340x0_device::mmtm_b(UINT16 op) { MMTM(B); }

#define MODS(R)                                                 \
{                                                               \
	INT32 *rs = &R##REG(SRCREG(op));                                \
	INT32 *rd = &R##REG(DSTREG(op));                                \
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
void tms340x0_device::mods_a(UINT16 op) { MODS(A); }
void tms340x0_device::mods_b(UINT16 op) { MODS(B); }

#define MODU(R)                                                 \
{                                                               \
	INT32 *rs = &R##REG(SRCREG(op));                                \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_ZV();                                                        \
	if (*rs != 0)                                               \
	{                                                           \
		*rd = (UINT32)*rd % (UINT32)*rs;                        \
		SET_Z_VAL(*rd);                                            \
	}                                                           \
	else                                                        \
		SET_V_LOG(1);                                          \
	COUNT_CYCLES(35);                                           \
}
void tms340x0_device::modu_a(UINT16 op) { MODU(A); }
void tms340x0_device::modu_b(UINT16 op) { MODU(B); }

#define MPYS(R)                                                 \
{                                                               \
	INT32 *rd1 = &R##REG(DSTREG(op));                               \
	INT32 m1 = R##REG(SRCREG(op));                                  \
	INT64 product;                                              \
																\
	SEXTEND(m1, FW(1));                                         \
	CLR_NZ();                                                        \
	product = mul_32x32(m1, *rd1);                          \
	SET_Z_LOG(product == 0);                                   \
	SET_N_BIT(product >> 32, 31);                              \
																\
	*rd1             = EXTRACT_64HI(product);                       \
	R##REG(DSTREG(op)|1) = EXTRACT_64LO(product);                       \
																\
	COUNT_CYCLES(20);                                           \
}
void tms340x0_device::mpys_a(UINT16 op) { MPYS(A); }
void tms340x0_device::mpys_b(UINT16 op) { MPYS(B); }

#define MPYU(R)                                                 \
{                                                               \
	INT32 *rd1 = &R##REG(DSTREG(op));                               \
	UINT32 m1 = R##REG(SRCREG(op));                                 \
	UINT64 product;                                             \
																\
	ZEXTEND(m1, FW(1));                                         \
	CLR_Z();                                                     \
	product = mulu_32x32(m1, *rd1);                     \
	SET_Z_LOG(product == 0);                                   \
																\
	*rd1             = EXTRACT_64HI(product);                       \
	R##REG(DSTREG(op)|1) = EXTRACT_64LO(product);                       \
																\
	COUNT_CYCLES(21);                                           \
}
void tms340x0_device::mpyu_a(UINT16 op) { MPYU(A); }
void tms340x0_device::mpyu_b(UINT16 op) { MPYU(B); }

#define NEG(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 r = 0 - *rd;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(0,*rd,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::neg_a(UINT16 op) { NEG(A); }
void tms340x0_device::neg_b(UINT16 op) { NEG(B); }

#define NEGB(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 t = *rd + (C_FLAG() ? 1 : 0);                          \
	INT32 r = 0 - t;                                            \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(0,t,r);                                        \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::negb_a(UINT16 op) { NEGB(A); }
void tms340x0_device::negb_b(UINT16 op) { NEGB(B); }

void tms340x0_device::nop(UINT16 op)
{
	COUNT_CYCLES(1);
}

#define NOT(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd = ~(*rd);                                               \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::not_a(UINT16 op) { NOT(A); }
void tms340x0_device::not_b(UINT16 op) { NOT(B); }

#define OR(R)                                                   \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd |= R##REG(SRCREG(op));                                      \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::or_a(UINT16 op) { OR(A); }
void tms340x0_device::or_b(UINT16 op) { OR(B); }

#define ORI(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd |= PARAM_LONG();                                     \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::ori_a(UINT16 op) { ORI(A); }
void tms340x0_device::ori_b(UINT16 op) { ORI(B); }

void tms340x0_device::setc(UINT16 op)
{
	SET_C_LOG(1);
	COUNT_CYCLES(1);
}

#define SETF(F)                                                 \
{                                                               \
	UINT8 shift = F ? 6 : 0;                                    \
	m_st &= ~(0x3f << shift);                                \
	m_st |= (op & 0x3f) << shift;                        \
	COUNT_CYCLES(1+F);                                          \
}
void tms340x0_device::setf0(UINT16 op) { SETF(0); }
void tms340x0_device::setf1(UINT16 op) { SETF(1); }

#define SEXT(F,R)                                               \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_NZ();                                                        \
	SEXTEND(*rd,FW(F));                                         \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::sext0_a(UINT16 op) { SEXT(0,A); }
void tms340x0_device::sext0_b(UINT16 op) { SEXT(0,B); }
void tms340x0_device::sext1_a(UINT16 op) { SEXT(1,A); }
void tms340x0_device::sext1_b(UINT16 op) { SEXT(1,B); }

#define RL(R,K)                                                 \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 res = *rd;                                            \
	INT32 k = (K);                                              \
	CLR_CZ();                                                        \
	if (k)                                                      \
	{                                                           \
		res<<=(k-1);                                            \
		SET_C_BIT_HI(res, 31);                                 \
		res<<=1;                                                \
		res |= (((UINT32)*rd)>>((-k)&0x1f));                    \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(res);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::rl_k_a(UINT16 op) { RL(A,PARAM_K(op)); }
void tms340x0_device::rl_k_b(UINT16 op) { RL(B,PARAM_K(op)); }
void tms340x0_device::rl_r_a(UINT16 op) { RL(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::rl_r_b(UINT16 op) { RL(B,BREG(SRCREG(op))&0x1f); }

#define SLA(R,K)                                                \
{                                                               \
		INT32 *rd = &R##REG(DSTREG(op));                                \
	UINT32 res = *rd;                                           \
		INT32 k = K;                                                \
	CLR_NCZV();                                                  \
	if (k)                                                      \
	{                                                           \
		UINT32 mask = (0xffffffff<<(31-k))&0x7fffffff;          \
		UINT32 res2 = SIGN(res) ? res^mask : res;               \
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
void tms340x0_device::sla_k_a(UINT16 op) { SLA(A,PARAM_K(op)); }
void tms340x0_device::sla_k_b(UINT16 op) { SLA(B,PARAM_K(op)); }
void tms340x0_device::sla_r_a(UINT16 op) { SLA(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::sla_r_b(UINT16 op) { SLA(B,BREG(SRCREG(op))&0x1f); }

#define SLL(R,K)                                                \
{                                                               \
		INT32 *rd = &R##REG(DSTREG(op));                                \
	UINT32 res = *rd;                                           \
		INT32 k = K;                                                \
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
void tms340x0_device::sll_k_a(UINT16 op) { SLL(A,PARAM_K(op)); }
void tms340x0_device::sll_k_b(UINT16 op) { SLL(B,PARAM_K(op)); }
void tms340x0_device::sll_r_a(UINT16 op) { SLL(A,AREG(SRCREG(op))&0x1f); }
void tms340x0_device::sll_r_b(UINT16 op) { SLL(B,BREG(SRCREG(op))&0x1f); }

#define SRA(R,K)                                                \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 res = *rd;                                            \
	INT32 k = (-(K)) & 0x1f;                                    \
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
void tms340x0_device::sra_k_a(UINT16 op) { SRA(A,PARAM_K(op)); }
void tms340x0_device::sra_k_b(UINT16 op) { SRA(B,PARAM_K(op)); }
void tms340x0_device::sra_r_a(UINT16 op) { SRA(A,AREG(SRCREG(op))); }
void tms340x0_device::sra_r_b(UINT16 op) { SRA(B,BREG(SRCREG(op))); }

#define SRL(R,K)                                                \
{                                                               \
		INT32 *rd = &R##REG(DSTREG(op));                                \
	UINT32 res = *rd;                                           \
		INT32 k = (-(K)) & 0x1f;                                    \
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
void tms340x0_device::srl_k_a(UINT16 op) { SRL(A,PARAM_K(op)); }
void tms340x0_device::srl_k_b(UINT16 op) { SRL(B,PARAM_K(op)); }
void tms340x0_device::srl_r_a(UINT16 op) { SRL(A,AREG(SRCREG(op))); }
void tms340x0_device::srl_r_b(UINT16 op) { SRL(B,BREG(SRCREG(op))); }

#define SUB(R)                                                  \
{                                                               \
	INT32 *rs = &R##REG(SRCREG(op));                                \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 r = *rd - *rs;                                        \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,*rs,r);                                    \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::sub_a(UINT16 op) { SUB(A); }
void tms340x0_device::sub_b(UINT16 op) { SUB(B); }

#define SUBB(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 t = R##REG(SRCREG(op));                                   \
	INT32 r = *rd - t - (C_FLAG() ? 1 : 0);                      \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::subb_a(UINT16 op) { SUBB(A); }
void tms340x0_device::subb_b(UINT16 op) { SUBB(B); }

#define SUBI_W(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 r;                                                    \
	INT32 t = ~PARAM_WORD();                                 \
	CLR_NCZV();                                                  \
	r = *rd - t;                                                \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::subi_w_a(UINT16 op) { SUBI_W(A); }
void tms340x0_device::subi_w_b(UINT16 op) { SUBI_W(B); }

#define SUBI_L(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 t = ~PARAM_LONG();                                 \
	INT32 r = *rd - t;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::subi_l_a(UINT16 op) { SUBI_L(A); }
void tms340x0_device::subi_l_b(UINT16 op) { SUBI_L(B); }

#define SUBK(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 t = fw_inc[PARAM_K(op)];                                  \
	INT32 r = *rd - t;                                          \
	CLR_NCZV();                                                  \
	SET_NZCV_SUB(*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::subk_a(UINT16 op) { SUBK(A); }
void tms340x0_device::subk_b(UINT16 op) { SUBK(B); }

#define XOR(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd ^= R##REG(SRCREG(op));                                      \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::xor_a(UINT16 op) { XOR(A); }
void tms340x0_device::xor_b(UINT16 op) { XOR(B); }

#define XORI(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	*rd ^= PARAM_LONG();                                     \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::xori_a(UINT16 op) { XORI(A); }
void tms340x0_device::xori_b(UINT16 op) { XORI(B); }

#define ZEXT(F,R)                                               \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_Z();                                                     \
	ZEXTEND(*rd,FW(F));                                         \
	SET_Z_VAL(*rd);                                                \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::zext0_a(UINT16 op) { ZEXT(0,A); }
void tms340x0_device::zext0_b(UINT16 op) { ZEXT(0,B); }
void tms340x0_device::zext1_a(UINT16 op) { ZEXT(1,A); }
void tms340x0_device::zext1_b(UINT16 op) { ZEXT(1,B); }



/***************************************************************************
    MOVE INSTRUCTIONS
***************************************************************************/

#define MOVI_W(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd=PARAM_WORD();                                        \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::movi_w_a(UINT16 op) { MOVI_W(A); }
void tms340x0_device::movi_w_b(UINT16 op) { MOVI_W(B); }

#define MOVI_L(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd=PARAM_LONG();                                        \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movi_l_a(UINT16 op) { MOVI_L(A); }
void tms340x0_device::movi_l_b(UINT16 op) { MOVI_L(B); }

#define MOVK(R)                                                 \
{                                                               \
	INT32 k = PARAM_K(op); if (!k) k = 32;                          \
	R##REG(DSTREG(op)) = k;                                         \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::movk_a(UINT16 op) { MOVK(A); }
void tms340x0_device::movk_b(UINT16 op) { MOVK(B); }

#define MOVB_RN(R)                                              \
{                                                               \
	WBYTE(R##REG(DSTREG(op)),R##REG(SRCREG(op)));                      \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::movb_rn_a(UINT16 op) { MOVB_RN(A); }
void tms340x0_device::movb_rn_b(UINT16 op) { MOVB_RN(B); }

#define MOVB_NR(R)                                              \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = (INT8)RBYTE(R##REG(SRCREG(op)));                     \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movb_nr_a(UINT16 op) { MOVB_NR(A); }
void tms340x0_device::movb_nr_b(UINT16 op) { MOVB_NR(B); }

#define MOVB_NN(R)                                              \
{                                                               \
	WBYTE(R##REG(DSTREG(op)),(UINT32)(UINT8)RBYTE(R##REG(SRCREG(op))));\
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movb_nn_a(UINT16 op) { MOVB_NN(A); }
void tms340x0_device::movb_nn_b(UINT16 op) { MOVB_NN(B); }

#define MOVB_R_NO(R)                                            \
{                                                               \
	INT32 o = PARAM_WORD();                                  \
	WBYTE(R##REG(DSTREG(op))+o,R##REG(SRCREG(op)));                        \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::movb_r_no_a(UINT16 op) { MOVB_R_NO(A); }
void tms340x0_device::movb_r_no_b(UINT16 op) { MOVB_R_NO(B); }

#define MOVB_NO_R(R)                                            \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 o = PARAM_WORD();                                  \
	CLR_NZV();                                                   \
	*rd = (INT8)RBYTE(R##REG(SRCREG(op))+o);                   \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::movb_no_r_a(UINT16 op) { MOVB_NO_R(A); }
void tms340x0_device::movb_no_r_b(UINT16 op) { MOVB_NO_R(B); }

#define MOVB_NO_NO(R)                                           \
{                                                               \
	INT32 o1 = PARAM_WORD();                                 \
	INT32 o2 = PARAM_WORD();                                 \
	WBYTE(R##REG(DSTREG(op))+o2,(UINT32)(UINT8)RBYTE(R##REG(SRCREG(op))+o1)); \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::movb_no_no_a(UINT16 op) { MOVB_NO_NO(A); }
void tms340x0_device::movb_no_no_b(UINT16 op) { MOVB_NO_NO(B); }

#define MOVB_RA(R)                                              \
{                                                               \
	WBYTE(PARAM_LONG(),R##REG(DSTREG(op)));                     \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::movb_ra_a(UINT16 op) { MOVB_RA(A); }
void tms340x0_device::movb_ra_b(UINT16 op) { MOVB_RA(B); }

#define MOVB_AR(R)                                              \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = (INT8)RBYTE(PARAM_LONG());                    \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::movb_ar_a(UINT16 op) { MOVB_AR(A); }
void tms340x0_device::movb_ar_b(UINT16 op) { MOVB_AR(B); }

void tms340x0_device::movb_aa(UINT16 op)
{
	UINT32 bitaddrs=PARAM_LONG();
	WBYTE(PARAM_LONG(),(UINT32)(UINT8)RBYTE(bitaddrs));
	COUNT_CYCLES(6);
}

#define MOVE_RR(RS,RD)                                          \
{                                                               \
	INT32 *rd = &RD##REG(DSTREG(op));                               \
	CLR_NZV();                                                   \
	*rd = RS##REG(SRCREG(op));                                      \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::move_rr_a (UINT16 op) { MOVE_RR(A,A); }
void tms340x0_device::move_rr_b (UINT16 op) { MOVE_RR(B,B); }
void tms340x0_device::move_rr_ax(UINT16 op) { MOVE_RR(A,B); }
void tms340x0_device::move_rr_bx(UINT16 op) { MOVE_RR(B,A); }

#define MOVE_RN(F,R)                                            \
{                                                               \
	WFIELD##F(R##REG(DSTREG(op)),R##REG(SRCREG(op)));                   \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::move0_rn_a (UINT16 op) { MOVE_RN(0,A); }
void tms340x0_device::move0_rn_b (UINT16 op) { MOVE_RN(0,B); }
void tms340x0_device::move1_rn_a (UINT16 op) { MOVE_RN(1,A); }
void tms340x0_device::move1_rn_b (UINT16 op) { MOVE_RN(1,B); }

#define MOVE_R_DN(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	*rd-=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,R##REG(SRCREG(op)));                              \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::move0_r_dn_a (UINT16 op) { MOVE_R_DN(0,A); }
void tms340x0_device::move0_r_dn_b (UINT16 op) { MOVE_R_DN(0,B); }
void tms340x0_device::move1_r_dn_a (UINT16 op) { MOVE_R_DN(1,A); }
void tms340x0_device::move1_r_dn_b (UINT16 op) { MOVE_R_DN(1,B); }

#define MOVE_R_NI(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	WFIELD##F(*rd,R##REG(SRCREG(op)));                              \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::move0_r_ni_a (UINT16 op) { MOVE_R_NI(0,A); }
void tms340x0_device::move0_r_ni_b (UINT16 op) { MOVE_R_NI(0,B); }
void tms340x0_device::move1_r_ni_a (UINT16 op) { MOVE_R_NI(1,A); }
void tms340x0_device::move1_r_ni_b (UINT16 op) { MOVE_R_NI(1,B); }

#define MOVE_NR(F,R)                                            \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(R##REG(SRCREG(op)));                            \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_nr_a (UINT16 op) { MOVE_NR(0,A); }
void tms340x0_device::move0_nr_b (UINT16 op) { MOVE_NR(0,B); }
void tms340x0_device::move1_nr_a (UINT16 op) { MOVE_NR(1,A); }
void tms340x0_device::move1_nr_b (UINT16 op) { MOVE_NR(1,B); }

#define MOVE_DN_R(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 *rs = &R##REG(SRCREG(op));                                \
	CLR_NZV();                                                   \
	*rs-=fw_inc[FW(F)];                                         \
	*rd = RFIELD##F(*rs);                                       \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(4);                                            \
}
void tms340x0_device::move0_dn_r_a (UINT16 op) { MOVE_DN_R(0,A); }
void tms340x0_device::move0_dn_r_b (UINT16 op) { MOVE_DN_R(0,B); }
void tms340x0_device::move1_dn_r_a (UINT16 op) { MOVE_DN_R(1,A); }
void tms340x0_device::move1_dn_r_b (UINT16 op) { MOVE_DN_R(1,B); }

#define MOVE_NI_R(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 *rs = &R##REG(SRCREG(op));                                \
	INT32 data = RFIELD##F(*rs);                                \
	CLR_NZV();                                                   \
	*rs+=fw_inc[FW(F)];                                         \
	*rd = data;                                                 \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_ni_r_a (UINT16 op) { MOVE_NI_R(0,A); }
void tms340x0_device::move0_ni_r_b (UINT16 op) { MOVE_NI_R(0,B); }
void tms340x0_device::move1_ni_r_a (UINT16 op) { MOVE_NI_R(1,A); }
void tms340x0_device::move1_ni_r_b (UINT16 op) { MOVE_NI_R(1,B); }

#define MOVE_NN(F,R)                                            \
{                                                               \
	WFIELD##F(R##REG(DSTREG(op)),RFIELD##F(R##REG(SRCREG(op))));        \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_nn_a (UINT16 op) { MOVE_NN(0,A); }
void tms340x0_device::move0_nn_b (UINT16 op) { MOVE_NN(0,B); }
void tms340x0_device::move1_nn_a (UINT16 op) { MOVE_NN(1,A); }
void tms340x0_device::move1_nn_b (UINT16 op) { MOVE_NN(1,B); }

#define MOVE_DN_DN(F,R)                                         \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 *rs = &R##REG(SRCREG(op));                                \
	INT32 data;                                                 \
	*rs-=fw_inc[FW(F)];                                         \
	data = RFIELD##F(*rs);                                      \
	*rd-=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,data);                                        \
	COUNT_CYCLES(4);                                            \
}
void tms340x0_device::move0_dn_dn_a (UINT16 op) { MOVE_DN_DN(0,A); }
void tms340x0_device::move0_dn_dn_b (UINT16 op) { MOVE_DN_DN(0,B); }
void tms340x0_device::move1_dn_dn_a (UINT16 op) { MOVE_DN_DN(1,A); }
void tms340x0_device::move1_dn_dn_b (UINT16 op) { MOVE_DN_DN(1,B); }

#define MOVE_NI_NI(F,R)                                         \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 *rs = &R##REG(SRCREG(op));                                \
	INT32 data = RFIELD##F(*rs);                                \
	*rs+=fw_inc[FW(F)];                                         \
	WFIELD##F(*rd,data);                                        \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(4);                                            \
}
void tms340x0_device::move0_ni_ni_a (UINT16 op) { MOVE_NI_NI(0,A); }
void tms340x0_device::move0_ni_ni_b (UINT16 op) { MOVE_NI_NI(0,B); }
void tms340x0_device::move1_ni_ni_a (UINT16 op) { MOVE_NI_NI(1,A); }
void tms340x0_device::move1_ni_ni_b (UINT16 op) { MOVE_NI_NI(1,B); }

#define MOVE_R_NO(F,R)                                          \
{                                                               \
	INT32 o = PARAM_WORD();                                  \
	WFIELD##F(R##REG(DSTREG(op))+o,R##REG(SRCREG(op)));                 \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_r_no_a (UINT16 op) { MOVE_R_NO(0,A); }
void tms340x0_device::move0_r_no_b (UINT16 op) { MOVE_R_NO(0,B); }
void tms340x0_device::move1_r_no_a (UINT16 op) { MOVE_R_NO(1,A); }
void tms340x0_device::move1_r_no_b (UINT16 op) { MOVE_R_NO(1,B); }

#define MOVE_NO_R(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 o = PARAM_WORD();                                  \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(R##REG(SRCREG(op))+o);                          \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_no_r_a (UINT16 op) { MOVE_NO_R(0,A); }
void tms340x0_device::move0_no_r_b (UINT16 op) { MOVE_NO_R(0,B); }
void tms340x0_device::move1_no_r_a (UINT16 op) { MOVE_NO_R(1,A); }
void tms340x0_device::move1_no_r_b (UINT16 op) { MOVE_NO_R(1,B); }

#define MOVE_NO_NI(F,R)                                         \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 o = PARAM_WORD();                                  \
	INT32 data = RFIELD##F(R##REG(SRCREG(op))+o);                   \
	WFIELD##F(*rd,data);                                        \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_no_ni_a (UINT16 op) { MOVE_NO_NI(0,A); }
void tms340x0_device::move0_no_ni_b (UINT16 op) { MOVE_NO_NI(0,B); }
void tms340x0_device::move1_no_ni_a (UINT16 op) { MOVE_NO_NI(1,A); }
void tms340x0_device::move1_no_ni_b (UINT16 op) { MOVE_NO_NI(1,B); }

#define MOVE_NO_NO(F,R)                                         \
{                                                               \
	INT32 o1 = PARAM_WORD();                                 \
	INT32 o2 = PARAM_WORD();                                 \
	INT32 data = RFIELD##F(R##REG(SRCREG(op))+o1);                  \
	WFIELD##F(R##REG(DSTREG(op))+o2,data);                          \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_no_no_a (UINT16 op) { MOVE_NO_NO(0,A); }
void tms340x0_device::move0_no_no_b (UINT16 op) { MOVE_NO_NO(0,B); }
void tms340x0_device::move1_no_no_a (UINT16 op) { MOVE_NO_NO(1,A); }
void tms340x0_device::move1_no_no_b (UINT16 op) { MOVE_NO_NO(1,B); }

#define MOVE_RA(F,R)                                            \
{                                                               \
	WFIELD##F(PARAM_LONG(),R##REG(DSTREG(op)));                  \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::move0_ra_a (UINT16 op) { MOVE_RA(0,A); }
void tms340x0_device::move0_ra_b (UINT16 op) { MOVE_RA(0,B); }
void tms340x0_device::move1_ra_a (UINT16 op) { MOVE_RA(1,A); }
void tms340x0_device::move1_ra_b (UINT16 op) { MOVE_RA(1,B); }

#define MOVE_AR(F,R)                                            \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	CLR_NZV();                                                   \
	*rd = RFIELD##F(PARAM_LONG());                           \
	SET_NZ_VAL(*rd);                                           \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_ar_a (UINT16 op) { MOVE_AR(0,A); }
void tms340x0_device::move0_ar_b (UINT16 op) { MOVE_AR(0,B); }
void tms340x0_device::move1_ar_a (UINT16 op) { MOVE_AR(1,A); }
void tms340x0_device::move1_ar_b (UINT16 op) { MOVE_AR(1,B); }

#define MOVE_A_NI(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	WFIELD##F(*rd,RFIELD##F(PARAM_LONG()));                  \
	*rd+=fw_inc[FW(F)];                                         \
	COUNT_CYCLES(5);                                            \
}
void tms340x0_device::move0_a_ni_a (UINT16 op) { MOVE_A_NI(0,A); }
void tms340x0_device::move0_a_ni_b (UINT16 op) { MOVE_A_NI(0,B); }
void tms340x0_device::move1_a_ni_a (UINT16 op) { MOVE_A_NI(1,A); }
void tms340x0_device::move1_a_ni_b (UINT16 op) { MOVE_A_NI(1,B); }

#define MOVE_AA(F)                                              \
{                                                               \
	UINT32 bitaddrs=PARAM_LONG();                            \
	WFIELD##F(PARAM_LONG(),RFIELD##F(bitaddrs));             \
	COUNT_CYCLES(7);                                            \
}
void tms340x0_device::move0_aa (UINT16 op) { MOVE_AA(0); }
void tms340x0_device::move1_aa (UINT16 op) { MOVE_AA(1); }



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
void tms340x0_device::call_a (UINT16 op) { CALL(A); }
void tms340x0_device::call_b (UINT16 op) { CALL(B); }

void tms340x0_device::callr(UINT16 op)
{
	PUSH(m_pc+0x10);
	m_pc += (PARAM_WORD_NO_INC()<<4)+0x10;
	COUNT_CYCLES(3);
}

void tms340x0_device::calla(UINT16 op)
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
void tms340x0_device::dsj_a (UINT16 op) { DSJ(A); }
void tms340x0_device::dsj_b (UINT16 op) { DSJ(B); }

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
void tms340x0_device::dsjeq_a (UINT16 op) { DSJEQ(A); }
void tms340x0_device::dsjeq_b (UINT16 op) { DSJEQ(B); }

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
void tms340x0_device::dsjne_a (UINT16 op) { DSJNE(A); }
void tms340x0_device::dsjne_b (UINT16 op) { DSJNE(B); }

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
void tms340x0_device::dsjs_a (UINT16 op) { DSJS(A); }
void tms340x0_device::dsjs_b (UINT16 op) { DSJS(B); }

void tms340x0_device::emu(UINT16 op)
{
	/* in RUN state, this instruction is a NOP */
	COUNT_CYCLES(6);
}

#define EXGPC(R)                                                \
{                                                               \
	INT32 *rd = &R##REG(DSTREG(op));                                \
	INT32 temppc = *rd;                                         \
	*rd = m_pc;                                                  \
	m_pc = temppc;                                               \
	CORRECT_ODD_PC("EXGPC");                                    \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::exgpc_a (UINT16 op) { EXGPC(A); }
void tms340x0_device::exgpc_b (UINT16 op) { EXGPC(B); }

#define GETPC(R)                                                \
{                                                               \
	R##REG(DSTREG(op)) = m_pc;                                       \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::getpc_a (UINT16 op) { GETPC(A); }
void tms340x0_device::getpc_b (UINT16 op) { GETPC(B); }

#define GETST(R)                                                \
{                                                               \
	R##REG(DSTREG(op)) = m_st;                               \
	COUNT_CYCLES(1);                                            \
}
void tms340x0_device::getst_a (UINT16 op) { GETST(A); }
void tms340x0_device::getst_b (UINT16 op) { GETST(B); }

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

void tms340x0_device::j_UC_0(UINT16 op)
{
	j_xx_0(1);
}
void tms340x0_device::j_UC_8(UINT16 op)
{
	j_xx_8(1);
}
void tms340x0_device::j_UC_x(UINT16 op)
{
	j_xx_x(1);
}
void tms340x0_device::j_P_0(UINT16 op)
{
	j_xx_0(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_P_8(UINT16 op)
{
	j_xx_8(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_P_x(UINT16 op)
{
	j_xx_x(!N_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_LS_0(UINT16 op)
{
	j_xx_0(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_LS_8(UINT16 op)
{
	j_xx_8(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_LS_x(UINT16 op)
{
	j_xx_x(C_FLAG() || Z_FLAG());
}
void tms340x0_device::j_HI_0(UINT16 op)
{
	j_xx_0(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_HI_8(UINT16 op)
{
	j_xx_8(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_HI_x(UINT16 op)
{
	j_xx_x(!C_FLAG() && !Z_FLAG());
}
void tms340x0_device::j_LT_0(UINT16 op)
{
	j_xx_0((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_LT_8(UINT16 op)
{
	j_xx_8((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_LT_x(UINT16 op)
{
	j_xx_x((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()));
}
void tms340x0_device::j_GE_0(UINT16 op)
{
	j_xx_0((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_GE_8(UINT16 op)
{
	j_xx_8((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_GE_x(UINT16 op)
{
	j_xx_x((N_FLAG() && V_FLAG()) || (!N_FLAG() && !V_FLAG()));
}
void tms340x0_device::j_LE_0(UINT16 op)
{
	j_xx_0((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_LE_8(UINT16 op)
{
	j_xx_8((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_LE_x(UINT16 op)
{
	j_xx_x((N_FLAG() && !V_FLAG()) || (!N_FLAG() && V_FLAG()) || Z_FLAG());
}
void tms340x0_device::j_GT_0(UINT16 op)
{
	j_xx_0((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_GT_8(UINT16 op)
{
	j_xx_8((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_GT_x(UINT16 op)
{
	j_xx_x((N_FLAG() && V_FLAG() && !Z_FLAG()) || (!N_FLAG() && !V_FLAG() && !Z_FLAG()));
}
void tms340x0_device::j_C_0(UINT16 op)
{
	j_xx_0(C_FLAG());
}
void tms340x0_device::j_C_8(UINT16 op)
{
	j_xx_8(C_FLAG());
}
void tms340x0_device::j_C_x(UINT16 op)
{
	j_xx_x(C_FLAG());
}
void tms340x0_device::j_NC_0(UINT16 op)
{
	j_xx_0(!C_FLAG());
}
void tms340x0_device::j_NC_8(UINT16 op)
{
	j_xx_8(!C_FLAG());
}
void tms340x0_device::j_NC_x(UINT16 op)
{
	j_xx_x(!C_FLAG());
}
void tms340x0_device::j_EQ_0(UINT16 op)
{
	j_xx_0(Z_FLAG());
}
void tms340x0_device::j_EQ_8(UINT16 op)
{
	j_xx_8(Z_FLAG());
}
void tms340x0_device::j_EQ_x(UINT16 op)
{
	j_xx_x(Z_FLAG());
}
void tms340x0_device::j_NE_0(UINT16 op)
{
	j_xx_0(!Z_FLAG());
}
void tms340x0_device::j_NE_8(UINT16 op)
{
	j_xx_8(!Z_FLAG());
}
void tms340x0_device::j_NE_x(UINT16 op)
{
	j_xx_x(!Z_FLAG());
}
void tms340x0_device::j_V_0(UINT16 op)
{
	j_xx_0(V_FLAG());
}
void tms340x0_device::j_V_8(UINT16 op)
{
	j_xx_8(V_FLAG());
}
void tms340x0_device::j_V_x(UINT16 op)
{
	j_xx_x(V_FLAG());
}
void tms340x0_device::j_NV_0(UINT16 op)
{
	j_xx_0(!V_FLAG());
}
void tms340x0_device::j_NV_8(UINT16 op)
{
	j_xx_8(!V_FLAG());
}
void tms340x0_device::j_NV_x(UINT16 op)
{
	j_xx_x(!V_FLAG());
}
void tms340x0_device::j_N_0(UINT16 op)
{
	j_xx_0(N_FLAG());
}
void tms340x0_device::j_N_8(UINT16 op)
{
	j_xx_8(N_FLAG());
}
void tms340x0_device::j_N_x(UINT16 op)
{
	j_xx_x(N_FLAG());
}
void tms340x0_device::j_NN_0(UINT16 op)
{
	j_xx_0(!N_FLAG());
}
void tms340x0_device::j_NN_8(UINT16 op)
{
	j_xx_8(!N_FLAG());
}
void tms340x0_device::j_NN_x(UINT16 op)
{
	j_xx_x(!N_FLAG());
}

#define JUMP(R)                                                 \
{                                                               \
	m_pc = R##REG(DSTREG(op));                                       \
	CORRECT_ODD_PC("JUMP");                                     \
	COUNT_CYCLES(2);                                            \
}
void tms340x0_device::jump_a (UINT16 op) { JUMP(A); }
void tms340x0_device::jump_b (UINT16 op) { JUMP(B); }

void tms340x0_device::popst(UINT16 op)
{
	SET_ST(POP());
	COUNT_CYCLES(8);
}

void tms340x0_device::pushst(UINT16 op)
{
	PUSH(m_st);
	COUNT_CYCLES(2);
}

#define PUTST(R)                                                \
{                                                               \
	SET_ST(R##REG(DSTREG(op)));                                \
	COUNT_CYCLES(3);                                            \
}
void tms340x0_device::putst_a (UINT16 op) { PUTST(A); }
void tms340x0_device::putst_b (UINT16 op) { PUTST(B); }

void tms340x0_device::reti(UINT16 op)
{
	INT32 st = POP();
	m_pc = POP();
	CORRECT_ODD_PC("RETI");
	SET_ST(st);
	COUNT_CYCLES(11);
}

void tms340x0_device::rets(UINT16 op)
{
	UINT32 offs;
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
void tms340x0_device::rev_a (UINT16 op) { REV(A); }
void tms340x0_device::rev_b (UINT16 op) { REV(B); }

void tms340x0_device::trap(UINT16 op)
{
	UINT32 t = PARAM_N(op);
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
	UINT32 a = PARAM_LONG();                 \
	XY *b = &R##REG_XY(DSTREG(op));                 \
	CLR_NCZV();                                  \
	b->x += (INT16)(a & 0xffff);                \
	b->y += ((INT32)a >> 16);                   \
	SET_N_LOG(b->x == 0);                      \
	SET_C_BIT_LO(b->y, 15);                        \
	SET_Z_LOG(b->y == 0);                      \
	SET_V_BIT_LO(b->x, 15);                        \
	COUNT_CYCLES(1);                            \
}
void tms340x0_device::addxyi_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	ADD_XYI(A);
}
void tms340x0_device::addxyi_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	ADD_XYI(B);
}

void tms340x0_device::blmove(UINT16 op)
{
	offs_t src = BREG(0);
	offs_t dst = BREG(2);
	offs_t bits = BREG(7);

	if (!m_is_34020) { unimpl(op); return; }

	/* src and dst are aligned */
	if (!(src & 0x0f) && !(dst & 0x0f))
	{
		while (bits >= 16 && m_icount > 0)
		{
			TMS34010_WRMEM_WORD(TOBYTE(dst), TMS34010_RDMEM_WORD(TOBYTE(src)));
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
	}

	/* src is aligned, dst is not */
	else if (!(src & 0x0f))
	{
		logerror("020:BLMOVE with aligned src and unaligned dst\n");
	}

	/* dst is aligned, src is not */
	else if (!(dst & 0x0f))
	{
		logerror("020:BLMOVE with unaligned src and aligned dst\n");
	}

	/* neither are aligned */
	else
	{
		logerror("020:BLMOVE with completely unaligned src and dst\n");
	}

	/* update the final results */
	BREG(0) = src;
	BREG(2) = dst;
	BREG(7) = bits;

	/* if we're not done yet, back up the PC */
	if (bits != 0)
		m_pc -= 0x10;
}

void tms340x0_device::cexec_l(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cexec_l\n");
}

void tms340x0_device::cexec_s(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cexec_s\n");
}

void tms340x0_device::clip(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:clip\n");
}

void tms340x0_device::cmovcg_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovcg_a\n");
}

void tms340x0_device::cmovcg_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovcg_b\n");
}

void tms340x0_device::cmovcm_f(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovcm_f\n");
}

void tms340x0_device::cmovcm_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovcm_b\n");
}

void tms340x0_device::cmovgc_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovgc_a\n");
}

void tms340x0_device::cmovgc_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovgc_b\n");
}

void tms340x0_device::cmovgc_a_s(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovgc_a_s\n");
}

void tms340x0_device::cmovgc_b_s(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovgc_b_s\n");
}

void tms340x0_device::cmovmc_f(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovmc_f\n");
}

void tms340x0_device::cmovmc_f_va(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovmc_f_va\n");
}

void tms340x0_device::cmovmc_f_vb(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovmc_f_vb\n");
}

void tms340x0_device::cmovmc_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cmovmc_b\n");
}

#define CMPK(R)                                             \
{                                                           \
	INT32 r;                                                \
	INT32 *rd = &R##REG(DSTREG(op));                            \
	INT32 t = PARAM_K(op); if (!t) t = 32;                      \
	CLR_NCZV();                                              \
	r = *rd - t;                                            \
	SET_NZCV_SUB(*rd,t,r);                                  \
	COUNT_CYCLES(1);                                        \
}
void tms340x0_device::cmp_k_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	CMPK(A);
}
void tms340x0_device::cmp_k_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	CMPK(B);
}

void tms340x0_device::cvdxyl_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cvdxyl_a\n");
}

void tms340x0_device::cvdxyl_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cvdxyl_b\n");
}

void tms340x0_device::cvmxyl_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cvmxyl_a\n");
}

void tms340x0_device::cvmxyl_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cvmxyl_b\n");
}

void tms340x0_device::cvsxyl_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cvsxyl_a\n");
}

void tms340x0_device::cvsxyl_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:cvsxyl_b\n");
}

void tms340x0_device::exgps_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:exgps_a\n");
}

void tms340x0_device::exgps_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:exgps_b\n");
}

void tms340x0_device::fline(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:fline\n");
}

void tms340x0_device::fpixeq(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:fpixeq\n");
}

void tms340x0_device::fpixne(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:fpixne\n");
}

void tms340x0_device::getps_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:getps_a\n");
}

void tms340x0_device::getps_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:getps_b\n");
}

void tms340x0_device::idle(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:idle\n");
}

void tms340x0_device::linit(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:linit\n");
}

void tms340x0_device::mwait(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
}

void tms340x0_device::pfill_xy(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:pfill_xy\n");
}

void tms340x0_device::pixblt_l_m_l(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:pixblt_l_m_l\n");
}

void tms340x0_device::retm(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:retm\n");
}

#define RMO(R)                                                  \
{                                                               \
	UINT32 res = 0;                                             \
	UINT32 rs  = R##REG(SRCREG(op));                                \
		INT32 *rd = &R##REG(DSTREG(op));                                \
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

void tms340x0_device::rmo_a(UINT16 op) { RMO(A); }
void tms340x0_device::rmo_b(UINT16 op) { RMO(B); }

#define RPIX(R)                                 \
{                                               \
	UINT32 v = R##REG(DSTREG(op));                  \
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

void tms340x0_device::rpix_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	RPIX(A);
}

void tms340x0_device::rpix_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	RPIX(B);
}

void tms340x0_device::setcdp(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:setcdp\n");
}

void tms340x0_device::setcmp(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:setcmp\n");
}

void tms340x0_device::setcsp(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:setcsp\n");
}

void tms340x0_device::swapf_a(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:swapf_a\n");
}

void tms340x0_device::swapf_b(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:swapf_b\n");
}

void tms340x0_device::tfill_xy(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:tfill_xy\n");
}

void tms340x0_device::trapl(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:trapl\n");
}

void tms340x0_device::vblt_b_l(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:vblt_b_l\n");
}

void tms340x0_device::vfill_l(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:vfill_l\n");
}

void tms340x0_device::vlcol(UINT16 op)
{
	if (!m_is_34020) { unimpl(op); return; }
	logerror("020:vlcol\n");
}
