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

#define SXYTOL(T,val)   ((((INT16)(val).y * (T)->convsp) + ((INT16)(val).x << (T)->pixelshift)) + OFFSET(T))
#define DXYTOL(T,val)   ((((INT16)(val).y * (T)->convdp) + ((INT16)(val).x << (T)->pixelshift)) + OFFSET(T))
#define MXYTOL(T,val)   ((((INT16)(val).y * (T)->convmp) + ((INT16)(val).x << (T)->pixelshift)) + OFFSET(T))

#define COUNT_CYCLES(T,x)   (T)->icount -= x
#define COUNT_UNKNOWN_CYCLES(T,x) COUNT_CYCLES(T,x)

#define CORRECT_ODD_PC(T,x) do { if ((T)->pc & 0x0f) logerror("%s to PC=%08X\n", x, (T)->pc); (T)->pc &= ~0x0f; } while (0)



/***************************************************************************
    FLAG HANDLING MACROS
***************************************************************************/

#define SIGN(val)           ((val) & 0x80000000)

#define CLR_Z(T)                (T)->st &= ~STBIT_Z
#define CLR_V(T)                (T)->st &= ~STBIT_V
#define CLR_C(T)                (T)->st &= ~STBIT_C
#define CLR_N(T)                (T)->st &= ~STBIT_N
#define CLR_NZ(T)               (T)->st &= ~(STBIT_N | STBIT_Z)
#define CLR_CZ(T)               (T)->st &= ~(STBIT_C | STBIT_Z)
#define CLR_ZV(T)               (T)->st &= ~(STBIT_Z | STBIT_V)
#define CLR_NZV(T)              (T)->st &= ~(STBIT_N | STBIT_Z | STBIT_V)
#define CLR_NCZ(T)              (T)->st &= ~(STBIT_N | STBIT_C | STBIT_Z)
#define CLR_NCZV(T)             (T)->st &= ~(STBIT_N | STBIT_C | STBIT_Z | STBIT_V)

#define SET_V_BIT_LO(T,val,bit) (T)->st |= ((val) << (28 - (bit))) & STBIT_V
#define SET_V_BIT_HI(T,val,bit) (T)->st |= ((val) >> ((bit) - 28)) & STBIT_V
#define SET_V_LOG(T,val)        (T)->st |= (val) << 28
#define SET_Z_BIT_LO(T,val,bit) (T)->st |= ((val) << (29 - (bit))) & STBIT_Z
#define SET_Z_BIT_HI(T,val,bit) (T)->st |= ((val) >> ((bit) - 29)) & STBIT_Z
#define SET_Z_LOG(T,val)        (T)->st |= (val) << 29
#define SET_C_BIT_LO(T,val,bit) (T)->st |= ((val) << (30 - (bit))) & STBIT_C
#define SET_C_BIT_HI(T,val,bit) (T)->st |= ((val) >> ((bit) - 30)) & STBIT_C
#define SET_C_LOG(T,val)        (T)->st |= (val) << 30
#define SET_N_BIT(T,val,bit)    (T)->st |= ((val) << (31 - (bit))) & STBIT_N
#define SET_N_LOG(T,val)        (T)->st |= (val) << 31

#define SET_Z_VAL(T,val)        SET_Z_LOG(T, (val) == 0)
#define SET_N_VAL(T,val)        SET_N_BIT(T, val, 31)
#define SET_NZ_VAL(T,val)       SET_Z_VAL(T, val); SET_N_VAL(T, val)
#define SET_V_SUB(T,a,b,r)      SET_V_BIT_HI(T, ((a) ^ (b)) & ((a) ^ (r)), 31)
#define SET_V_ADD(T,a,b,r)      SET_V_BIT_HI(T, ~((a) ^ (b)) & ((a) ^ (r)), 31)
#define SET_C_SUB(T,a,b)        SET_C_LOG(T, (UINT32)(b) > (UINT32)(a))
#define SET_C_ADD(T,a,b)        SET_C_LOG(T, (UINT32)~(a) < (UINT32)(b))
#define SET_NZV_SUB(T,a,b,r)    SET_NZ_VAL(T,r); SET_V_SUB(T,a,b,r)
#define SET_NZCV_SUB(T,a,b,r)   SET_NZV_SUB(T,a,b,r); SET_C_SUB(T,a,b)
#define SET_NZCV_ADD(T,a,b,r)   SET_NZ_VAL(T,r); SET_V_ADD(T,a,b,r); SET_C_ADD(T,a,b)

static const UINT8 fw_inc[32] = { 32,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31 };


/***************************************************************************
    UNIMPLEMENTED INSTRUCTION
***************************************************************************/

static void unimpl(tms34010_state *tms, UINT16 op)
{
	/* kludge for Super High Impact -- this doesn't seem to cause */
	/* an illegal opcode exception */
	if (tms->direct->read_decrypted_word(TOBYTE(tms->pc - 0x10)) == 0x0007)
		return;

	/* 9 Ball Shootout calls to FFDF7468, expecting it */
	/* to execute the next instruction from FFDF7470 */
	/* but the instruction at FFDF7460 is an 0x0001 */
	if (tms->direct->read_decrypted_word(TOBYTE(tms->pc - 0x10)) == 0x0001)
		return;

	PUSH(tms, tms->pc);
	PUSH(tms, tms->st);
	RESET_ST(tms);
	tms->pc = RLONG(tms, 0xfffffc20);
	COUNT_UNKNOWN_CYCLES(tms,16);

	/* extra check to prevent bad things */
	if (tms->pc == 0 || opcode_table[tms->direct->read_decrypted_word(TOBYTE(tms->pc)) >> 4] == unimpl)
	{
		tms->device->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
		debugger_break(tms->device->machine());
	}
}



/***************************************************************************
    X/Y OPERATIONS
***************************************************************************/

#define ADD_XY(R)                               \
{                                               \
	XY  a =  R##REG_XY(tms,SRCREG(op));                 \
	XY *b = &R##REG_XY(tms,DSTREG(op));                 \
	CLR_NCZV(tms);                                  \
	b->x += a.x;                                \
	b->y += a.y;                                \
	SET_N_LOG(tms, b->x == 0);                      \
	SET_C_BIT_LO(tms, b->y, 15);                        \
	SET_Z_LOG(tms, b->y == 0);                      \
	SET_V_BIT_LO(tms, b->x, 15);                        \
	COUNT_CYCLES(tms,1);                            \
}
static void add_xy_a(tms34010_state *tms, UINT16 op) { ADD_XY(A); }
static void add_xy_b(tms34010_state *tms, UINT16 op) { ADD_XY(B); }

#define SUB_XY(R)                               \
{                                               \
	XY  a =  R##REG_XY(tms,SRCREG(op));                 \
	XY *b = &R##REG_XY(tms,DSTREG(op));                 \
	CLR_NCZV(tms);                                  \
	SET_N_LOG(tms, a.x == b->x);                        \
	SET_C_LOG(tms, a.y > b->y);                     \
	SET_Z_LOG(tms, a.y == b->y);                        \
	SET_V_LOG(tms, a.x > b->x);                     \
	b->x -= a.x;                                \
	b->y -= a.y;                                \
	COUNT_CYCLES(tms,1);                            \
}
static void sub_xy_a(tms34010_state *tms, UINT16 op) { SUB_XY(A); }
static void sub_xy_b(tms34010_state *tms, UINT16 op) { SUB_XY(B); }

#define CMP_XY(R)                               \
{                                               \
	INT16 res;                                  \
	XY a = R##REG_XY(tms,DSTREG(op));                   \
	XY b = R##REG_XY(tms,SRCREG(op));                   \
	CLR_NCZV(tms);                                  \
	res = a.x-b.x;                              \
	SET_N_LOG(tms, res == 0);                       \
	SET_V_BIT_LO(tms, res, 15);                     \
	res = a.y-b.y;                              \
	SET_Z_LOG(tms, res == 0);                       \
	SET_C_BIT_LO(tms, res, 15);                     \
	COUNT_CYCLES(tms,1);                            \
}
static void cmp_xy_a(tms34010_state *tms, UINT16 op) { CMP_XY(A); }
static void cmp_xy_b(tms34010_state *tms, UINT16 op) { CMP_XY(B); }

#define CPW(R)                                  \
{                                               \
	INT32 res = 0;                              \
	INT16 x = R##REG_X(tms,SRCREG(op));                 \
	INT16 y = R##REG_Y(tms,SRCREG(op));                 \
												\
	CLR_V(tms);                                     \
	res |= ((WSTART_X(tms) > x) ? 0x20  : 0);       \
	res |= ((x > WEND_X(tms))   ? 0x40  : 0);       \
	res |= ((WSTART_Y(tms) > y) ? 0x80  : 0);       \
	res |= ((y > WEND_Y(tms))   ? 0x100 : 0);       \
	R##REG(tms,DSTREG(op)) = res;                       \
	SET_V_LOG(tms, res != 0);                       \
	COUNT_CYCLES(tms,1);                            \
}
static void cpw_a(tms34010_state *tms, UINT16 op) { CPW(A); }
static void cpw_b(tms34010_state *tms, UINT16 op) { CPW(B); }

#define CVXYL(R)                                    \
{                                                   \
	R##REG(tms,DSTREG(op)) = DXYTOL(tms,R##REG_XY(tms,SRCREG(op)));     \
	COUNT_CYCLES(tms,3);                                \
}
static void cvxyl_a(tms34010_state *tms, UINT16 op) { CVXYL(A); }
static void cvxyl_b(tms34010_state *tms, UINT16 op) { CVXYL(B); }

#define MOVX(R)                                     \
{                                                   \
	R##REG(tms,DSTREG(op)) = (R##REG(tms,DSTREG(op)) & 0xffff0000) | (UINT16)R##REG(tms,SRCREG(op));    \
	COUNT_CYCLES(tms,1);                                                                    \
}
static void movx_a(tms34010_state *tms, UINT16 op) { MOVX(A); }
static void movx_b(tms34010_state *tms, UINT16 op) { MOVX(B); }

#define MOVY(R)                                     \
{                                                   \
	R##REG(tms,DSTREG(op)) = (R##REG(tms,SRCREG(op)) & 0xffff0000) | (UINT16)R##REG(tms,DSTREG(op));    \
	COUNT_CYCLES(tms,1);                                                                    \
}
static void movy_a(tms34010_state *tms, UINT16 op) { MOVY(A); }
static void movy_b(tms34010_state *tms, UINT16 op) { MOVY(B); }



/***************************************************************************
    PIXEL TRANSFER OPERATIONS
***************************************************************************/

#define PIXT_RI(R)                                  \
{                                                   \
	WPIXEL(tms,R##REG(tms,DSTREG(op)),R##REG(tms,SRCREG(op)));  \
	COUNT_UNKNOWN_CYCLES(tms,2);                        \
}
static void pixt_ri_a(tms34010_state *tms, UINT16 op) { PIXT_RI(A); }
static void pixt_ri_b(tms34010_state *tms, UINT16 op) { PIXT_RI(B); }

#define PIXT_RIXY(R)                                                                \
{                                                                                   \
	if (WINDOW_CHECKING(tms) != 0)                                                      \
	{                                                                               \
		CLR_V(tms);                                                                     \
		if (R##REG_X(tms,DSTREG(op)) < WSTART_X(tms) || R##REG_X(tms,DSTREG(op)) > WEND_X(tms) ||               \
			R##REG_Y(tms,DSTREG(op)) < WSTART_Y(tms) || R##REG_Y(tms,DSTREG(op)) > WEND_Y(tms))             \
		{                                                                           \
			SET_V_LOG(tms, 1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING(tms) == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(tms,DXYTOL(tms,R##REG_XY(tms,DSTREG(op))),R##REG(tms,SRCREG(op)));                               \
skip:                                                                               \
	COUNT_UNKNOWN_CYCLES(tms,4);                                                        \
}
static void pixt_rixy_a(tms34010_state *tms, UINT16 op) { PIXT_RIXY(A); }
static void pixt_rixy_b(tms34010_state *tms, UINT16 op) { PIXT_RIXY(B); }

#define PIXT_IR(R)                                  \
{                                                   \
	INT32 temp = RPIXEL(tms,R##REG(tms,SRCREG(op)));            \
	CLR_V(tms);                                         \
	R##REG(tms,DSTREG(op)) = temp;                          \
	SET_V_LOG(tms, temp != 0);                          \
	COUNT_CYCLES(tms,4);                                \
}
static void pixt_ir_a(tms34010_state *tms, UINT16 op) { PIXT_IR(A); }
static void pixt_ir_b(tms34010_state *tms, UINT16 op) { PIXT_IR(B); }

#define PIXT_II(R)                                  \
{                                                   \
	WPIXEL(tms,R##REG(tms,DSTREG(op)),RPIXEL(tms,R##REG(tms,SRCREG(op))));  \
	COUNT_UNKNOWN_CYCLES(tms,4);                        \
}
static void pixt_ii_a(tms34010_state *tms, UINT16 op) { PIXT_II(A); }
static void pixt_ii_b(tms34010_state *tms, UINT16 op) { PIXT_II(B); }

#define PIXT_IXYR(R)                                \
{                                                   \
	INT32 temp = RPIXEL(tms,SXYTOL(tms,R##REG_XY(tms,SRCREG(op)))); \
	CLR_V(tms);                                         \
	R##REG(tms,DSTREG(op)) = temp;                          \
	SET_V_LOG(tms, temp != 0);                          \
	COUNT_CYCLES(tms,6);                                \
}
static void pixt_ixyr_a(tms34010_state *tms, UINT16 op) { PIXT_IXYR(A); }
static void pixt_ixyr_b(tms34010_state *tms, UINT16 op) { PIXT_IXYR(B); }

#define PIXT_IXYIXY(R)                                                              \
{                                                                                   \
	if (WINDOW_CHECKING(tms) != 0)                                                      \
	{                                                                               \
		CLR_V(tms);                                                                     \
		if (R##REG_X(tms,DSTREG(op)) < WSTART_X(tms) || R##REG_X(tms,DSTREG(op)) > WEND_X(tms) ||               \
			R##REG_Y(tms,DSTREG(op)) < WSTART_Y(tms) || R##REG_Y(tms,DSTREG(op)) > WEND_Y(tms))             \
		{                                                                           \
			SET_V_LOG(tms, 1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING(tms) == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(tms,DXYTOL(tms,R##REG_XY(tms,DSTREG(op))),RPIXEL(tms,SXYTOL(tms,R##REG_XY(tms,SRCREG(op)))));            \
skip:                                                                               \
	COUNT_UNKNOWN_CYCLES(tms,7);                                                        \
}
static void pixt_ixyixy_a(tms34010_state *tms, UINT16 op) { PIXT_IXYIXY(A); }
static void pixt_ixyixy_b(tms34010_state *tms, UINT16 op) { PIXT_IXYIXY(B); }

#define DRAV(R)                                                                     \
{                                                                                   \
	if (WINDOW_CHECKING(tms) != 0)                                                      \
	{                                                                               \
		CLR_V(tms);                                                                     \
		if (R##REG_X(tms,DSTREG(op)) < WSTART_X(tms) || R##REG_X(tms,DSTREG(op)) > WEND_X(tms) ||               \
			R##REG_Y(tms,DSTREG(op)) < WSTART_Y(tms) || R##REG_Y(tms,DSTREG(op)) > WEND_Y(tms))             \
		{                                                                           \
			SET_V_LOG(tms, 1);                                                          \
			goto skip;                                                              \
		}                                                                           \
		if (WINDOW_CHECKING(tms) == 1) goto skip;                                       \
	}                                                                               \
	WPIXEL(tms,DXYTOL(tms,R##REG_XY(tms,DSTREG(op))),COLOR1(tms));                                      \
skip:                                                                               \
	R##REG_X(tms,DSTREG(op)) += R##REG_X(tms,SRCREG(op));                                           \
	R##REG_Y(tms,DSTREG(op)) += R##REG_Y(tms,SRCREG(op));                                           \
	COUNT_UNKNOWN_CYCLES(tms,4);                                                        \
}
static void drav_a(tms34010_state *tms, UINT16 op) { DRAV(A); }
static void drav_b(tms34010_state *tms, UINT16 op) { DRAV(B); }



/***************************************************************************
    ARITHMETIC OPERATIONS
***************************************************************************/

#define ABS(R)                                              \
{                                                           \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 r = 0 - *rd;                                      \
	CLR_NZV(tms);                                               \
	if (r > 0) *rd = r;                                     \
	SET_NZ_VAL(tms, r);                                         \
	SET_V_LOG(tms, r == (INT32)0x80000000);                     \
	COUNT_CYCLES(tms,1);                                        \
}
static void abs_a(tms34010_state *tms, UINT16 op) { ABS(A); }
static void abs_b(tms34010_state *tms, UINT16 op) { ABS(B); }

#define ADD(R)                                              \
{                                                           \
	INT32 a = R##REG(tms,SRCREG(op));                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV(tms);                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(tms,a,b,r);                                    \
	COUNT_CYCLES(tms,1);                                        \
}
static void add_a(tms34010_state *tms, UINT16 op) { ADD(A); }
static void add_b(tms34010_state *tms, UINT16 op) { ADD(B); }

#define ADDC(R)                                             \
{                                                           \
	/* I'm not sure to which side the carry is added to, should */  \
	/* verify it against the examples */                    \
	INT32 a = R##REG(tms,SRCREG(op));                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b + (C_FLAG(tms) ? 1 : 0);                        \
	CLR_NCZV(tms);                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(tms,a,b,r);                                    \
	COUNT_CYCLES(tms,1);                                        \
}
static void addc_a(tms34010_state *tms, UINT16 op) { ADDC(A); }
static void addc_b(tms34010_state *tms, UINT16 op) { ADDC(B); }

#define ADDI_W(R)                                           \
{                                                           \
	INT32 a = PARAM_WORD(tms);                              \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV(tms);                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(tms,a,b,r);                                    \
	COUNT_CYCLES(tms,2);                                        \
}
static void addi_w_a(tms34010_state *tms, UINT16 op) { ADDI_W(A); }
static void addi_w_b(tms34010_state *tms, UINT16 op) { ADDI_W(B); }

#define ADDI_L(R)                                           \
{                                                           \
	INT32 a = PARAM_LONG(tms);                              \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV(tms);                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(tms,a,b,r);                                    \
	COUNT_CYCLES(tms,3);                                        \
}
static void addi_l_a(tms34010_state *tms, UINT16 op) { ADDI_L(A); }
static void addi_l_b(tms34010_state *tms, UINT16 op) { ADDI_L(B); }

#define ADDK(R)                                             \
{                                                           \
	INT32 a = fw_inc[PARAM_K(op)];                              \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 b = *rd;                                          \
	INT32 r = a + b;                                        \
	CLR_NCZV(tms);                                              \
	*rd = r;                                                \
	SET_NZCV_ADD(tms,a,b,r);                                    \
	COUNT_CYCLES(tms,1);                                        \
}
static void addk_a(tms34010_state *tms, UINT16 op) { ADDK(A); }
static void addk_b(tms34010_state *tms, UINT16 op) { ADDK(B); }

#define AND(R)                                              \
{                                                           \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	CLR_Z(tms);                                                 \
	*rd &= R##REG(tms,SRCREG(op));                                  \
	SET_Z_VAL(tms, *rd);                                            \
	COUNT_CYCLES(tms,1);                                        \
}
static void and_a(tms34010_state *tms, UINT16 op) { AND(A); }
static void and_b(tms34010_state *tms, UINT16 op) { AND(B); }

#define ANDI(R)                                             \
{                                                           \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	CLR_Z(tms);                                                 \
	*rd &= ~PARAM_LONG(tms);                                \
	SET_Z_VAL(tms, *rd);                                            \
	COUNT_CYCLES(tms,3);                                        \
}
static void andi_a(tms34010_state *tms, UINT16 op) { ANDI(A); }
static void andi_b(tms34010_state *tms, UINT16 op) { ANDI(B); }

#define ANDN(R)                                             \
{                                                           \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	CLR_Z(tms);                                                 \
	*rd &= ~R##REG(tms,SRCREG(op));                                 \
	SET_Z_VAL(tms, *rd);                                            \
	COUNT_CYCLES(tms,1);                                        \
}
static void andn_a(tms34010_state *tms, UINT16 op) { ANDN(A); }
static void andn_b(tms34010_state *tms, UINT16 op) { ANDN(B); }

#define BTST_K(R)                                           \
{                                                           \
	int bit = 31 - PARAM_K(op);                                 \
	CLR_Z(tms);                                                 \
	if (bit <= 29)                                          \
		SET_Z_BIT_LO(tms, ~R##REG(tms,DSTREG(op)), bit);                    \
	else                                                    \
		SET_Z_BIT_HI(tms, ~R##REG(tms,DSTREG(op)), bit);                    \
	COUNT_CYCLES(tms,1);                                        \
}
static void btst_k_a(tms34010_state *tms, UINT16 op) { BTST_K(A); }
static void btst_k_b(tms34010_state *tms, UINT16 op) { BTST_K(B); }

#define BTST_R(R)                                           \
{                                                           \
	int bit = R##REG(tms,SRCREG(op)) & 0x1f;                        \
	CLR_Z(tms);                                                 \
	if (bit <= 29)                                          \
		SET_Z_BIT_LO(tms, ~R##REG(tms,DSTREG(op)), bit);                    \
	else                                                    \
		SET_Z_BIT_HI(tms, ~R##REG(tms,DSTREG(op)), bit);                    \
	COUNT_CYCLES(tms,2);                                        \
}
static void btst_r_a(tms34010_state *tms, UINT16 op) { BTST_R(A); }
static void btst_r_b(tms34010_state *tms, UINT16 op) { BTST_R(B); }

static void clrc(tms34010_state *tms, UINT16 op)
{
	CLR_C(tms);
	COUNT_CYCLES(tms,1);
}

#define CMP(R)                                              \
{                                                           \
	INT32 *rs = &R##REG(tms,SRCREG(op));                            \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 r = *rd - *rs;                                    \
	CLR_NCZV(tms);                                              \
	SET_NZCV_SUB(tms,*rd,*rs,r);                                \
	COUNT_CYCLES(tms,1);                                        \
}
static void cmp_a(tms34010_state *tms, UINT16 op) { CMP(A); }
static void cmp_b(tms34010_state *tms, UINT16 op) { CMP(B); }

#define CMPI_W(R)                                           \
{                                                           \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 t = (INT16)~PARAM_WORD(tms);                      \
	INT32 r = *rd - t;                                      \
	CLR_NCZV(tms);                                              \
	SET_NZCV_SUB(tms,*rd,t,r);                                  \
	COUNT_CYCLES(tms,2);                                        \
}
static void cmpi_w_a(tms34010_state *tms, UINT16 op) { CMPI_W(A); }
static void cmpi_w_b(tms34010_state *tms, UINT16 op) { CMPI_W(B); }

#define CMPI_L(R)                                           \
{                                                           \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 t = ~PARAM_LONG(tms);                             \
	INT32 r = *rd - t;                                      \
	CLR_NCZV(tms);                                              \
	SET_NZCV_SUB(tms,*rd,t,r);                                  \
	COUNT_CYCLES(tms,3);                                        \
}
static void cmpi_l_a(tms34010_state *tms, UINT16 op) { CMPI_L(A); }
static void cmpi_l_b(tms34010_state *tms, UINT16 op) { CMPI_L(B); }

static void dint(tms34010_state *tms, UINT16 op)
{
	tms->st &= ~STBIT_IE;
	COUNT_CYCLES(tms,3);
}

#define DIVS(R)                                             \
{                                                           \
	INT32 *rs  = &R##REG(tms,SRCREG(op));                           \
	INT32 *rd1 = &R##REG(tms,DSTREG(op));                           \
	CLR_NZV(tms);                                               \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(tms, 1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			INT32 *rd2 = &R##REG(tms,DSTREG(op)+1);                 \
			INT64 dividend = ((UINT64)*rd1 << 32) | (UINT32)*rd2; \
			INT64 quotient = dividend / *rs;                \
			INT32 remainder = dividend % *rs;               \
			UINT32 signbits = (INT32)quotient >> 31;        \
			if (EXTRACT_64HI(quotient) != signbits)         \
			{                                               \
				SET_V_LOG(tms, 1);                              \
			}                                               \
			else                                            \
			{                                               \
				*rd1 = quotient;                            \
				*rd2 = remainder;                           \
				SET_NZ_VAL(tms, *rd1);                          \
			}                                               \
		}                                                   \
		COUNT_CYCLES(tms,40);                                   \
	}                                                       \
	else                                                    \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(tms, 1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			*rd1 /= *rs;                                    \
			SET_NZ_VAL(tms, *rd1);                              \
		}                                                   \
		COUNT_CYCLES(tms,39);                                   \
	}                                                       \
}
static void divs_a(tms34010_state *tms, UINT16 op) { DIVS(A); }
static void divs_b(tms34010_state *tms, UINT16 op) { DIVS(B); }

#define DIVU(R)                                             \
{                                                           \
	INT32 *rs  = &R##REG(tms,SRCREG(op));                           \
	INT32 *rd1 = &R##REG(tms,DSTREG(op));                           \
	CLR_ZV(tms);                                                    \
	if (!(DSTREG(op) & 1))                                      \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(tms, 1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			INT32 *rd2 = &R##REG(tms,DSTREG(op)+1);                 \
			UINT64 dividend  = ((UINT64)*rd1 << 32) | (UINT32)*rd2; \
			UINT64 quotient  = dividend / (UINT32)*rs;      \
			UINT32 remainder = dividend % (UINT32)*rs;      \
			if (EXTRACT_64HI(quotient) != 0)                \
			{                                               \
				SET_V_LOG(tms, 1);                              \
			}                                               \
			else                                            \
			{                                               \
				*rd1 = quotient;                            \
				*rd2 = remainder;                           \
				SET_Z_VAL(tms, *rd1);                           \
			}                                               \
		}                                                   \
	}                                                       \
	else                                                    \
	{                                                       \
		if (!*rs)                                           \
		{                                                   \
			SET_V_LOG(tms, 1);                                  \
		}                                                   \
		else                                                \
		{                                                   \
			*rd1 = (UINT32)*rd1 / (UINT32)*rs;              \
			SET_Z_VAL(tms, *rd1);                               \
		}                                                   \
	}                                                       \
	COUNT_CYCLES(tms,37);                                       \
}
static void divu_a(tms34010_state *tms, UINT16 op) { DIVU(A); }
static void divu_b(tms34010_state *tms, UINT16 op) { DIVU(B); }

static void eint(tms34010_state *tms, UINT16 op)
{
	tms->st |= STBIT_IE;
	check_interrupt(tms);
	COUNT_CYCLES(tms,3);
}

#define EXGF(F,R)                                               \
{                                                               \
	UINT8 shift = F ? 6 : 0;                                    \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	UINT32 temp = (tms->st >> shift) & 0x3f;                    \
	tms->st &= ~(0x3f << shift);                                \
	tms->st |= (*rd & 0x3f) << shift;                           \
	*rd = temp;                                                 \
	COUNT_CYCLES(tms,1);                                            \
}
static void exgf0_a(tms34010_state *tms, UINT16 op) { EXGF(0,A); }
static void exgf0_b(tms34010_state *tms, UINT16 op) { EXGF(0,B); }
static void exgf1_a(tms34010_state *tms, UINT16 op) { EXGF(1,A); }
static void exgf1_b(tms34010_state *tms, UINT16 op) { EXGF(1,B); }

#define LMO(R)                                                  \
{                                                               \
	UINT32 res = 0;                                             \
	UINT32 rs  = R##REG(tms,SRCREG(op));                                \
		INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	SET_Z_VAL(tms, rs);                                             \
	if (rs)                                                     \
	{                                                           \
		while (!(rs & 0x80000000))                              \
		{                                                       \
			res++;                                              \
			rs <<= 1;                                           \
		}                                                       \
	}                                                           \
	*rd = res;                                                  \
	COUNT_CYCLES(tms,1);                                            \
}
static void lmo_a(tms34010_state *tms, UINT16 op) { LMO(A); }
static void lmo_b(tms34010_state *tms, UINT16 op) { LMO(B); }

#define MMFM(R)                                                 \
{                                                               \
	INT32 i;                                                    \
	UINT16 l = (UINT16) PARAM_WORD(tms);                        \
	COUNT_CYCLES(tms,3);                                            \
	{                                                           \
		INT32 rd = DSTREG(op);                                      \
		for (i = 15; i >= 0 ; i--)                              \
		{                                                       \
			if (l & 0x8000)                                     \
			{                                                   \
				R##REG(tms,i) = RLONG(tms, R##REG(tms,rd));                 \
				R##REG(tms,rd) += 0x20;                             \
				COUNT_CYCLES(tms,4);                                \
			}                                                   \
			l <<= 1;                                            \
		}                                                       \
	}                                                           \
}
static void mmfm_a(tms34010_state *tms, UINT16 op) { MMFM(A); }
static void mmfm_b(tms34010_state *tms, UINT16 op) { MMFM(B); }

#define MMTM(R)                                                 \
{                                                               \
	UINT32 i;                                                   \
	UINT16 l = (UINT16) PARAM_WORD(tms);                        \
	COUNT_CYCLES(tms,2);                                            \
	{                                                           \
		INT32 rd = DSTREG(op);                                      \
		if (tms->is_34020)                                      \
		{                                                       \
			CLR_N(tms);                                             \
			SET_N_VAL(tms, R##REG(tms,rd) ^ 0x80000000);                    \
		}                                                       \
		for (i = 0; i  < 16; i++)                               \
		{                                                       \
			if (l & 0x8000)                                     \
			{                                                   \
				R##REG(tms,rd) -= 0x20;                             \
				WLONG(tms, R##REG(tms,rd),R##REG(tms,i));                   \
				COUNT_CYCLES(tms,4);                                \
			}                                                   \
			l <<= 1;                                            \
		}                                                       \
	}                                                           \
}
static void mmtm_a(tms34010_state *tms, UINT16 op) { MMTM(A); }
static void mmtm_b(tms34010_state *tms, UINT16 op) { MMTM(B); }

#define MODS(R)                                                 \
{                                                               \
	INT32 *rs = &R##REG(tms,SRCREG(op));                                \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZV(tms);                                                   \
	if (*rs != 0)                                               \
	{                                                           \
		*rd %= *rs;                                             \
		SET_NZ_VAL(tms, *rd);                                       \
	}                                                           \
	else                                                        \
		SET_V_LOG(tms, 1);                                          \
	COUNT_CYCLES(tms,40);                                           \
}
static void mods_a(tms34010_state *tms, UINT16 op) { MODS(A); }
static void mods_b(tms34010_state *tms, UINT16 op) { MODS(B); }

#define MODU(R)                                                 \
{                                                               \
	INT32 *rs = &R##REG(tms,SRCREG(op));                                \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_ZV(tms);                                                        \
	if (*rs != 0)                                               \
	{                                                           \
		*rd = (UINT32)*rd % (UINT32)*rs;                        \
		SET_Z_VAL(tms, *rd);                                            \
	}                                                           \
	else                                                        \
		SET_V_LOG(tms, 1);                                          \
	COUNT_CYCLES(tms,35);                                           \
}
static void modu_a(tms34010_state *tms, UINT16 op) { MODU(A); }
static void modu_b(tms34010_state *tms, UINT16 op) { MODU(B); }

#define MPYS(R)                                                 \
{                                                               \
	INT32 *rd1 = &R##REG(tms,DSTREG(op));                               \
	INT32 m1 = R##REG(tms,SRCREG(op));                                  \
	INT64 product;                                              \
																\
	SEXTEND(m1, FW(tms,1));                                         \
	CLR_NZ(tms);                                                        \
	product = mul_32x32(m1, *rd1);                          \
	SET_Z_LOG(tms, product == 0);                                   \
	SET_N_BIT(tms, product >> 32, 31);                              \
																\
	*rd1             = EXTRACT_64HI(product);                       \
	R##REG(tms,DSTREG(op)|1) = EXTRACT_64LO(product);                       \
																\
	COUNT_CYCLES(tms,20);                                           \
}
static void mpys_a(tms34010_state *tms, UINT16 op) { MPYS(A); }
static void mpys_b(tms34010_state *tms, UINT16 op) { MPYS(B); }

#define MPYU(R)                                                 \
{                                                               \
	INT32 *rd1 = &R##REG(tms,DSTREG(op));                               \
	UINT32 m1 = R##REG(tms,SRCREG(op));                                 \
	UINT64 product;                                             \
																\
	ZEXTEND(m1, FW(tms,1));                                         \
	CLR_Z(tms);                                                     \
	product = mulu_32x32(m1, *rd1);                     \
	SET_Z_LOG(tms, product == 0);                                   \
																\
	*rd1             = EXTRACT_64HI(product);                       \
	R##REG(tms,DSTREG(op)|1) = EXTRACT_64LO(product);                       \
																\
	COUNT_CYCLES(tms,21);                                           \
}
static void mpyu_a(tms34010_state *tms, UINT16 op) { MPYU(A); }
static void mpyu_b(tms34010_state *tms, UINT16 op) { MPYU(B); }

#define NEG(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 r = 0 - *rd;                                          \
	CLR_NCZV(tms);                                                  \
	SET_NZCV_SUB(tms,0,*rd,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(tms,1);                                            \
}
static void neg_a(tms34010_state *tms, UINT16 op) { NEG(A); }
static void neg_b(tms34010_state *tms, UINT16 op) { NEG(B); }

#define NEGB(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 t = *rd + (C_FLAG(tms) ? 1 : 0);                          \
	INT32 r = 0 - t;                                            \
	CLR_NCZV(tms);                                                  \
	SET_NZCV_SUB(tms,0,t,r);                                        \
	*rd = r;                                                    \
	COUNT_CYCLES(tms,1);                                            \
}
static void negb_a(tms34010_state *tms, UINT16 op) { NEGB(A); }
static void negb_b(tms34010_state *tms, UINT16 op) { NEGB(B); }

static void nop(tms34010_state *tms, UINT16 op)
{
	COUNT_CYCLES(tms,1);
}

#define NOT(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	*rd = ~(*rd);                                               \
	SET_Z_VAL(tms, *rd);                                                \
	COUNT_CYCLES(tms,1);                                            \
}
static void not_a(tms34010_state *tms, UINT16 op) { NOT(A); }
static void not_b(tms34010_state *tms, UINT16 op) { NOT(B); }

#define OR(R)                                                   \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	*rd |= R##REG(tms,SRCREG(op));                                      \
	SET_Z_VAL(tms, *rd);                                                \
	COUNT_CYCLES(tms,1);                                            \
}
static void or_a(tms34010_state *tms, UINT16 op) { OR(A); }
static void or_b(tms34010_state *tms, UINT16 op) { OR(B); }

#define ORI(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	*rd |= PARAM_LONG(tms);                                     \
	SET_Z_VAL(tms, *rd);                                                \
	COUNT_CYCLES(tms,3);                                            \
}
static void ori_a(tms34010_state *tms, UINT16 op) { ORI(A); }
static void ori_b(tms34010_state *tms, UINT16 op) { ORI(B); }

static void setc(tms34010_state *tms, UINT16 op)
{
	SET_C_LOG(tms, 1);
	COUNT_CYCLES(tms,1);
}

#define SETF(F)                                                 \
{                                                               \
	UINT8 shift = F ? 6 : 0;                                    \
	tms->st &= ~(0x3f << shift);                                \
	tms->st |= (op & 0x3f) << shift;                        \
	COUNT_CYCLES(tms,1+F);                                          \
}
static void setf0(tms34010_state *tms, UINT16 op) { SETF(0); }
static void setf1(tms34010_state *tms, UINT16 op) { SETF(1); }

#define SEXT(F,R)                                               \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZ(tms);                                                        \
	SEXTEND(*rd,FW(tms,F));                                         \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,3);                                            \
}
static void sext0_a(tms34010_state *tms, UINT16 op) { SEXT(0,A); }
static void sext0_b(tms34010_state *tms, UINT16 op) { SEXT(0,B); }
static void sext1_a(tms34010_state *tms, UINT16 op) { SEXT(1,A); }
static void sext1_b(tms34010_state *tms, UINT16 op) { SEXT(1,B); }

#define RL(R,K)                                                 \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 res = *rd;                                            \
	INT32 k = (K);                                              \
	CLR_CZ(tms);                                                        \
	if (k)                                                      \
	{                                                           \
		res<<=(k-1);                                            \
		SET_C_BIT_HI(tms, res, 31);                                 \
		res<<=1;                                                \
		res |= (((UINT32)*rd)>>((-k)&0x1f));                    \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(tms, res);                                                \
	COUNT_CYCLES(tms,1);                                            \
}
static void rl_k_a(tms34010_state *tms, UINT16 op) { RL(A,PARAM_K(op)); }
static void rl_k_b(tms34010_state *tms, UINT16 op) { RL(B,PARAM_K(op)); }
static void rl_r_a(tms34010_state *tms, UINT16 op) { RL(A,AREG(tms,SRCREG(op))&0x1f); }
static void rl_r_b(tms34010_state *tms, UINT16 op) { RL(B,BREG(tms,SRCREG(op))&0x1f); }

#define SLA(R,K)                                                \
{                                                               \
		INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	UINT32 res = *rd;                                           \
		INT32 k = K;                                                \
	CLR_NCZV(tms);                                                  \
	if (k)                                                      \
	{                                                           \
		UINT32 mask = (0xffffffff<<(31-k))&0x7fffffff;          \
		UINT32 res2 = SIGN(res) ? res^mask : res;               \
		SET_V_LOG(tms, (res2 & mask) != 0);                         \
																\
		res<<=(k-1);                                            \
		SET_C_BIT_HI(tms, res, 31);                                 \
		res<<=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_NZ_VAL(tms, res);                                           \
	COUNT_CYCLES(tms,3);                                            \
}
static void sla_k_a(tms34010_state *tms, UINT16 op) { SLA(A,PARAM_K(op)); }
static void sla_k_b(tms34010_state *tms, UINT16 op) { SLA(B,PARAM_K(op)); }
static void sla_r_a(tms34010_state *tms, UINT16 op) { SLA(A,AREG(tms,SRCREG(op))&0x1f); }
static void sla_r_b(tms34010_state *tms, UINT16 op) { SLA(B,BREG(tms,SRCREG(op))&0x1f); }

#define SLL(R,K)                                                \
{                                                               \
		INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	UINT32 res = *rd;                                           \
		INT32 k = K;                                                \
	CLR_CZ(tms);                                                        \
	if (k)                                                      \
	{                                                           \
		res<<=(k-1);                                            \
		SET_C_BIT_HI(tms, res, 31);                                 \
		res<<=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(tms, res);                                                \
	COUNT_CYCLES(tms,1);                                            \
}
static void sll_k_a(tms34010_state *tms, UINT16 op) { SLL(A,PARAM_K(op)); }
static void sll_k_b(tms34010_state *tms, UINT16 op) { SLL(B,PARAM_K(op)); }
static void sll_r_a(tms34010_state *tms, UINT16 op) { SLL(A,AREG(tms,SRCREG(op))&0x1f); }
static void sll_r_b(tms34010_state *tms, UINT16 op) { SLL(B,BREG(tms,SRCREG(op))&0x1f); }

#define SRA(R,K)                                                \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 res = *rd;                                            \
	INT32 k = (-(K)) & 0x1f;                                    \
	CLR_NCZ(tms);                                                   \
	if (k)                                                      \
	{                                                           \
		res>>=(k-1);                                            \
		SET_C_BIT_LO(tms, res, 0);                                  \
		res>>=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_NZ_VAL(tms, res);                                           \
	COUNT_CYCLES(tms,1);                                            \
}
static void sra_k_a(tms34010_state *tms, UINT16 op) { SRA(A,PARAM_K(op)); }
static void sra_k_b(tms34010_state *tms, UINT16 op) { SRA(B,PARAM_K(op)); }
static void sra_r_a(tms34010_state *tms, UINT16 op) { SRA(A,AREG(tms,SRCREG(op))); }
static void sra_r_b(tms34010_state *tms, UINT16 op) { SRA(B,BREG(tms,SRCREG(op))); }

#define SRL(R,K)                                                \
{                                                               \
		INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	UINT32 res = *rd;                                           \
		INT32 k = (-(K)) & 0x1f;                                    \
	CLR_CZ(tms);                                                        \
	if (k)                                                      \
	{                                                           \
		res>>=(k-1);                                            \
		SET_C_BIT_LO(tms, res, 0);                                  \
		res>>=1;                                                \
		*rd = res;                                              \
	}                                                           \
	SET_Z_VAL(tms, res);                                                \
	COUNT_CYCLES(tms,1);                                            \
}
static void srl_k_a(tms34010_state *tms, UINT16 op) { SRL(A,PARAM_K(op)); }
static void srl_k_b(tms34010_state *tms, UINT16 op) { SRL(B,PARAM_K(op)); }
static void srl_r_a(tms34010_state *tms, UINT16 op) { SRL(A,AREG(tms,SRCREG(op))); }
static void srl_r_b(tms34010_state *tms, UINT16 op) { SRL(B,BREG(tms,SRCREG(op))); }

#define SUB(R)                                                  \
{                                                               \
	INT32 *rs = &R##REG(tms,SRCREG(op));                                \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 r = *rd - *rs;                                        \
	CLR_NCZV(tms);                                                  \
	SET_NZCV_SUB(tms,*rd,*rs,r);                                    \
	*rd = r;                                                    \
	COUNT_CYCLES(tms,1);                                            \
}
static void sub_a(tms34010_state *tms, UINT16 op) { SUB(A); }
static void sub_b(tms34010_state *tms, UINT16 op) { SUB(B); }

#define SUBB(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 t = R##REG(tms,SRCREG(op));                                   \
	INT32 r = *rd - t - (C_FLAG(tms) ? 1 : 0);                      \
	CLR_NCZV(tms);                                                  \
	SET_NZCV_SUB(tms,*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(tms,1);                                            \
}
static void subb_a(tms34010_state *tms, UINT16 op) { SUBB(A); }
static void subb_b(tms34010_state *tms, UINT16 op) { SUBB(B); }

#define SUBI_W(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 r;                                                    \
	INT32 t = ~PARAM_WORD(tms);                                 \
	CLR_NCZV(tms);                                                  \
	r = *rd - t;                                                \
	SET_NZCV_SUB(tms,*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(tms,2);                                            \
}
static void subi_w_a(tms34010_state *tms, UINT16 op) { SUBI_W(A); }
static void subi_w_b(tms34010_state *tms, UINT16 op) { SUBI_W(B); }

#define SUBI_L(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 t = ~PARAM_LONG(tms);                                 \
	INT32 r = *rd - t;                                          \
	CLR_NCZV(tms);                                                  \
	SET_NZCV_SUB(tms,*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(tms,3);                                            \
}
static void subi_l_a(tms34010_state *tms, UINT16 op) { SUBI_L(A); }
static void subi_l_b(tms34010_state *tms, UINT16 op) { SUBI_L(B); }

#define SUBK(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 t = fw_inc[PARAM_K(op)];                                  \
	INT32 r = *rd - t;                                          \
	CLR_NCZV(tms);                                                  \
	SET_NZCV_SUB(tms,*rd,t,r);                                      \
	*rd = r;                                                    \
	COUNT_CYCLES(tms,1);                                            \
}
static void subk_a(tms34010_state *tms, UINT16 op) { SUBK(A); }
static void subk_b(tms34010_state *tms, UINT16 op) { SUBK(B); }

#define XOR(R)                                                  \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	*rd ^= R##REG(tms,SRCREG(op));                                      \
	SET_Z_VAL(tms, *rd);                                                \
	COUNT_CYCLES(tms,1);                                            \
}
static void xor_a(tms34010_state *tms, UINT16 op) { XOR(A); }
static void xor_b(tms34010_state *tms, UINT16 op) { XOR(B); }

#define XORI(R)                                                 \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	*rd ^= PARAM_LONG(tms);                                     \
	SET_Z_VAL(tms, *rd);                                                \
	COUNT_CYCLES(tms,3);                                            \
}
static void xori_a(tms34010_state *tms, UINT16 op) { XORI(A); }
static void xori_b(tms34010_state *tms, UINT16 op) { XORI(B); }

#define ZEXT(F,R)                                               \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	ZEXTEND(*rd,FW(tms,F));                                         \
	SET_Z_VAL(tms, *rd);                                                \
	COUNT_CYCLES(tms,1);                                            \
}
static void zext0_a(tms34010_state *tms, UINT16 op) { ZEXT(0,A); }
static void zext0_b(tms34010_state *tms, UINT16 op) { ZEXT(0,B); }
static void zext1_a(tms34010_state *tms, UINT16 op) { ZEXT(1,A); }
static void zext1_b(tms34010_state *tms, UINT16 op) { ZEXT(1,B); }



/***************************************************************************
    MOVE INSTRUCTIONS
***************************************************************************/

#define MOVI_W(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZV(tms);                                                   \
	*rd=PARAM_WORD(tms);                                        \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,2);                                            \
}
static void movi_w_a(tms34010_state *tms, UINT16 op) { MOVI_W(A); }
static void movi_w_b(tms34010_state *tms, UINT16 op) { MOVI_W(B); }

#define MOVI_L(R)                                               \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZV(tms);                                                   \
	*rd=PARAM_LONG(tms);                                        \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,3);                                            \
}
static void movi_l_a(tms34010_state *tms, UINT16 op) { MOVI_L(A); }
static void movi_l_b(tms34010_state *tms, UINT16 op) { MOVI_L(B); }

#define MOVK(R)                                                 \
{                                                               \
	INT32 k = PARAM_K(op); if (!k) k = 32;                          \
	R##REG(tms,DSTREG(op)) = k;                                         \
	COUNT_CYCLES(tms,1);                                            \
}
static void movk_a(tms34010_state *tms, UINT16 op) { MOVK(A); }
static void movk_b(tms34010_state *tms, UINT16 op) { MOVK(B); }

#define MOVB_RN(R)                                              \
{                                                               \
	WBYTE(tms, R##REG(tms,DSTREG(op)),R##REG(tms,SRCREG(op)));                      \
	COUNT_CYCLES(tms,1);                                            \
}
static void movb_rn_a(tms34010_state *tms, UINT16 op) { MOVB_RN(A); }
static void movb_rn_b(tms34010_state *tms, UINT16 op) { MOVB_RN(B); }

#define MOVB_NR(R)                                              \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZV(tms);                                                   \
	*rd = (INT8)RBYTE(tms, R##REG(tms,SRCREG(op)));                     \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,3);                                            \
}
static void movb_nr_a(tms34010_state *tms, UINT16 op) { MOVB_NR(A); }
static void movb_nr_b(tms34010_state *tms, UINT16 op) { MOVB_NR(B); }

#define MOVB_NN(R)                                              \
{                                                               \
	WBYTE(tms, R##REG(tms,DSTREG(op)),(UINT32)(UINT8)RBYTE(tms, R##REG(tms,SRCREG(op))));\
	COUNT_CYCLES(tms,3);                                            \
}
static void movb_nn_a(tms34010_state *tms, UINT16 op) { MOVB_NN(A); }
static void movb_nn_b(tms34010_state *tms, UINT16 op) { MOVB_NN(B); }

#define MOVB_R_NO(R)                                            \
{                                                               \
	INT32 o = PARAM_WORD(tms);                                  \
	WBYTE(tms, R##REG(tms,DSTREG(op))+o,R##REG(tms,SRCREG(op)));                        \
	COUNT_CYCLES(tms,3);                                            \
}
static void movb_r_no_a(tms34010_state *tms, UINT16 op) { MOVB_R_NO(A); }
static void movb_r_no_b(tms34010_state *tms, UINT16 op) { MOVB_R_NO(B); }

#define MOVB_NO_R(R)                                            \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 o = PARAM_WORD(tms);                                  \
	CLR_NZV(tms);                                                   \
	*rd = (INT8)RBYTE(tms, R##REG(tms,SRCREG(op))+o);                   \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,5);                                            \
}
static void movb_no_r_a(tms34010_state *tms, UINT16 op) { MOVB_NO_R(A); }
static void movb_no_r_b(tms34010_state *tms, UINT16 op) { MOVB_NO_R(B); }

#define MOVB_NO_NO(R)                                           \
{                                                               \
	INT32 o1 = PARAM_WORD(tms);                                 \
	INT32 o2 = PARAM_WORD(tms);                                 \
	WBYTE(tms, R##REG(tms,DSTREG(op))+o2,(UINT32)(UINT8)RBYTE(tms, R##REG(tms,SRCREG(op))+o1)); \
	COUNT_CYCLES(tms,5);                                            \
}
static void movb_no_no_a(tms34010_state *tms, UINT16 op) { MOVB_NO_NO(A); }
static void movb_no_no_b(tms34010_state *tms, UINT16 op) { MOVB_NO_NO(B); }

#define MOVB_RA(R)                                              \
{                                                               \
	WBYTE(tms, PARAM_LONG(tms),R##REG(tms,DSTREG(op)));                     \
	COUNT_CYCLES(tms,1);                                            \
}
static void movb_ra_a(tms34010_state *tms, UINT16 op) { MOVB_RA(A); }
static void movb_ra_b(tms34010_state *tms, UINT16 op) { MOVB_RA(B); }

#define MOVB_AR(R)                                              \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZV(tms);                                                   \
	*rd = (INT8)RBYTE(tms, PARAM_LONG(tms));                    \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,5);                                            \
}
static void movb_ar_a(tms34010_state *tms, UINT16 op) { MOVB_AR(A); }
static void movb_ar_b(tms34010_state *tms, UINT16 op) { MOVB_AR(B); }

static void movb_aa(tms34010_state *tms, UINT16 op)
{
	UINT32 bitaddrs=PARAM_LONG(tms);
	WBYTE(tms, PARAM_LONG(tms),(UINT32)(UINT8)RBYTE(tms, bitaddrs));
	COUNT_CYCLES(tms,6);
}

#define MOVE_RR(RS,RD)                                          \
{                                                               \
	INT32 *rd = &RD##REG(tms,DSTREG(op));                               \
	CLR_NZV(tms);                                                   \
	*rd = RS##REG(tms,SRCREG(op));                                      \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,1);                                            \
}
static void move_rr_a (tms34010_state *tms, UINT16 op) { MOVE_RR(A,A); }
static void move_rr_b (tms34010_state *tms, UINT16 op) { MOVE_RR(B,B); }
static void move_rr_ax(tms34010_state *tms, UINT16 op) { MOVE_RR(A,B); }
static void move_rr_bx(tms34010_state *tms, UINT16 op) { MOVE_RR(B,A); }

#define MOVE_RN(F,R)                                            \
{                                                               \
	WFIELD##F(tms,R##REG(tms,DSTREG(op)),R##REG(tms,SRCREG(op)));                   \
	COUNT_CYCLES(tms,1);                                            \
}
static void move0_rn_a (tms34010_state *tms, UINT16 op) { MOVE_RN(0,A); }
static void move0_rn_b (tms34010_state *tms, UINT16 op) { MOVE_RN(0,B); }
static void move1_rn_a (tms34010_state *tms, UINT16 op) { MOVE_RN(1,A); }
static void move1_rn_b (tms34010_state *tms, UINT16 op) { MOVE_RN(1,B); }

#define MOVE_R_DN(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	*rd-=fw_inc[FW(tms,F)];                                         \
	WFIELD##F(tms,*rd,R##REG(tms,SRCREG(op)));                              \
	COUNT_CYCLES(tms,2);                                            \
}
static void move0_r_dn_a (tms34010_state *tms, UINT16 op) { MOVE_R_DN(0,A); }
static void move0_r_dn_b (tms34010_state *tms, UINT16 op) { MOVE_R_DN(0,B); }
static void move1_r_dn_a (tms34010_state *tms, UINT16 op) { MOVE_R_DN(1,A); }
static void move1_r_dn_b (tms34010_state *tms, UINT16 op) { MOVE_R_DN(1,B); }

#define MOVE_R_NI(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	WFIELD##F(tms,*rd,R##REG(tms,SRCREG(op)));                              \
	*rd+=fw_inc[FW(tms,F)];                                         \
	COUNT_CYCLES(tms,1);                                            \
}
static void move0_r_ni_a (tms34010_state *tms, UINT16 op) { MOVE_R_NI(0,A); }
static void move0_r_ni_b (tms34010_state *tms, UINT16 op) { MOVE_R_NI(0,B); }
static void move1_r_ni_a (tms34010_state *tms, UINT16 op) { MOVE_R_NI(1,A); }
static void move1_r_ni_b (tms34010_state *tms, UINT16 op) { MOVE_R_NI(1,B); }

#define MOVE_NR(F,R)                                            \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZV(tms);                                                   \
	*rd = RFIELD##F(tms,R##REG(tms,SRCREG(op)));                            \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,3);                                            \
}
static void move0_nr_a (tms34010_state *tms, UINT16 op) { MOVE_NR(0,A); }
static void move0_nr_b (tms34010_state *tms, UINT16 op) { MOVE_NR(0,B); }
static void move1_nr_a (tms34010_state *tms, UINT16 op) { MOVE_NR(1,A); }
static void move1_nr_b (tms34010_state *tms, UINT16 op) { MOVE_NR(1,B); }

#define MOVE_DN_R(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 *rs = &R##REG(tms,SRCREG(op));                                \
	CLR_NZV(tms);                                                   \
	*rs-=fw_inc[FW(tms,F)];                                         \
	*rd = RFIELD##F(tms,*rs);                                       \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,4);                                            \
}
static void move0_dn_r_a (tms34010_state *tms, UINT16 op) { MOVE_DN_R(0,A); }
static void move0_dn_r_b (tms34010_state *tms, UINT16 op) { MOVE_DN_R(0,B); }
static void move1_dn_r_a (tms34010_state *tms, UINT16 op) { MOVE_DN_R(1,A); }
static void move1_dn_r_b (tms34010_state *tms, UINT16 op) { MOVE_DN_R(1,B); }

#define MOVE_NI_R(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 *rs = &R##REG(tms,SRCREG(op));                                \
	INT32 data = RFIELD##F(tms,*rs);                                \
	CLR_NZV(tms);                                                   \
	*rs+=fw_inc[FW(tms,F)];                                         \
	*rd = data;                                                 \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,3);                                            \
}
static void move0_ni_r_a (tms34010_state *tms, UINT16 op) { MOVE_NI_R(0,A); }
static void move0_ni_r_b (tms34010_state *tms, UINT16 op) { MOVE_NI_R(0,B); }
static void move1_ni_r_a (tms34010_state *tms, UINT16 op) { MOVE_NI_R(1,A); }
static void move1_ni_r_b (tms34010_state *tms, UINT16 op) { MOVE_NI_R(1,B); }

#define MOVE_NN(F,R)                                            \
{                                                               \
	WFIELD##F(tms,R##REG(tms,DSTREG(op)),RFIELD##F(tms,R##REG(tms,SRCREG(op))));        \
	COUNT_CYCLES(tms,3);                                            \
}
static void move0_nn_a (tms34010_state *tms, UINT16 op) { MOVE_NN(0,A); }
static void move0_nn_b (tms34010_state *tms, UINT16 op) { MOVE_NN(0,B); }
static void move1_nn_a (tms34010_state *tms, UINT16 op) { MOVE_NN(1,A); }
static void move1_nn_b (tms34010_state *tms, UINT16 op) { MOVE_NN(1,B); }

#define MOVE_DN_DN(F,R)                                         \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 *rs = &R##REG(tms,SRCREG(op));                                \
	INT32 data;                                                 \
	*rs-=fw_inc[FW(tms,F)];                                         \
	data = RFIELD##F(tms,*rs);                                      \
	*rd-=fw_inc[FW(tms,F)];                                         \
	WFIELD##F(tms,*rd,data);                                        \
	COUNT_CYCLES(tms,4);                                            \
}
static void move0_dn_dn_a (tms34010_state *tms, UINT16 op) { MOVE_DN_DN(0,A); }
static void move0_dn_dn_b (tms34010_state *tms, UINT16 op) { MOVE_DN_DN(0,B); }
static void move1_dn_dn_a (tms34010_state *tms, UINT16 op) { MOVE_DN_DN(1,A); }
static void move1_dn_dn_b (tms34010_state *tms, UINT16 op) { MOVE_DN_DN(1,B); }

#define MOVE_NI_NI(F,R)                                         \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 *rs = &R##REG(tms,SRCREG(op));                                \
	INT32 data = RFIELD##F(tms,*rs);                                \
	*rs+=fw_inc[FW(tms,F)];                                         \
	WFIELD##F(tms,*rd,data);                                        \
	*rd+=fw_inc[FW(tms,F)];                                         \
	COUNT_CYCLES(tms,4);                                            \
}
static void move0_ni_ni_a (tms34010_state *tms, UINT16 op) { MOVE_NI_NI(0,A); }
static void move0_ni_ni_b (tms34010_state *tms, UINT16 op) { MOVE_NI_NI(0,B); }
static void move1_ni_ni_a (tms34010_state *tms, UINT16 op) { MOVE_NI_NI(1,A); }
static void move1_ni_ni_b (tms34010_state *tms, UINT16 op) { MOVE_NI_NI(1,B); }

#define MOVE_R_NO(F,R)                                          \
{                                                               \
	INT32 o = PARAM_WORD(tms);                                  \
	WFIELD##F(tms,R##REG(tms,DSTREG(op))+o,R##REG(tms,SRCREG(op)));                 \
	COUNT_CYCLES(tms,3);                                            \
}
static void move0_r_no_a (tms34010_state *tms, UINT16 op) { MOVE_R_NO(0,A); }
static void move0_r_no_b (tms34010_state *tms, UINT16 op) { MOVE_R_NO(0,B); }
static void move1_r_no_a (tms34010_state *tms, UINT16 op) { MOVE_R_NO(1,A); }
static void move1_r_no_b (tms34010_state *tms, UINT16 op) { MOVE_R_NO(1,B); }

#define MOVE_NO_R(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 o = PARAM_WORD(tms);                                  \
	CLR_NZV(tms);                                                   \
	*rd = RFIELD##F(tms,R##REG(tms,SRCREG(op))+o);                          \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,5);                                            \
}
static void move0_no_r_a (tms34010_state *tms, UINT16 op) { MOVE_NO_R(0,A); }
static void move0_no_r_b (tms34010_state *tms, UINT16 op) { MOVE_NO_R(0,B); }
static void move1_no_r_a (tms34010_state *tms, UINT16 op) { MOVE_NO_R(1,A); }
static void move1_no_r_b (tms34010_state *tms, UINT16 op) { MOVE_NO_R(1,B); }

#define MOVE_NO_NI(F,R)                                         \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 o = PARAM_WORD(tms);                                  \
	INT32 data = RFIELD##F(tms,R##REG(tms,SRCREG(op))+o);                   \
	WFIELD##F(tms,*rd,data);                                        \
	*rd+=fw_inc[FW(tms,F)];                                         \
	COUNT_CYCLES(tms,5);                                            \
}
static void move0_no_ni_a (tms34010_state *tms, UINT16 op) { MOVE_NO_NI(0,A); }
static void move0_no_ni_b (tms34010_state *tms, UINT16 op) { MOVE_NO_NI(0,B); }
static void move1_no_ni_a (tms34010_state *tms, UINT16 op) { MOVE_NO_NI(1,A); }
static void move1_no_ni_b (tms34010_state *tms, UINT16 op) { MOVE_NO_NI(1,B); }

#define MOVE_NO_NO(F,R)                                         \
{                                                               \
	INT32 o1 = PARAM_WORD(tms);                                 \
	INT32 o2 = PARAM_WORD(tms);                                 \
	INT32 data = RFIELD##F(tms,R##REG(tms,SRCREG(op))+o1);                  \
	WFIELD##F(tms,R##REG(tms,DSTREG(op))+o2,data);                          \
	COUNT_CYCLES(tms,5);                                            \
}
static void move0_no_no_a (tms34010_state *tms, UINT16 op) { MOVE_NO_NO(0,A); }
static void move0_no_no_b (tms34010_state *tms, UINT16 op) { MOVE_NO_NO(0,B); }
static void move1_no_no_a (tms34010_state *tms, UINT16 op) { MOVE_NO_NO(1,A); }
static void move1_no_no_b (tms34010_state *tms, UINT16 op) { MOVE_NO_NO(1,B); }

#define MOVE_RA(F,R)                                            \
{                                                               \
	WFIELD##F(tms,PARAM_LONG(tms),R##REG(tms,DSTREG(op)));                  \
	COUNT_CYCLES(tms,3);                                            \
}
static void move0_ra_a (tms34010_state *tms, UINT16 op) { MOVE_RA(0,A); }
static void move0_ra_b (tms34010_state *tms, UINT16 op) { MOVE_RA(0,B); }
static void move1_ra_a (tms34010_state *tms, UINT16 op) { MOVE_RA(1,A); }
static void move1_ra_b (tms34010_state *tms, UINT16 op) { MOVE_RA(1,B); }

#define MOVE_AR(F,R)                                            \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_NZV(tms);                                                   \
	*rd = RFIELD##F(tms,PARAM_LONG(tms));                           \
	SET_NZ_VAL(tms, *rd);                                           \
	COUNT_CYCLES(tms,5);                                            \
}
static void move0_ar_a (tms34010_state *tms, UINT16 op) { MOVE_AR(0,A); }
static void move0_ar_b (tms34010_state *tms, UINT16 op) { MOVE_AR(0,B); }
static void move1_ar_a (tms34010_state *tms, UINT16 op) { MOVE_AR(1,A); }
static void move1_ar_b (tms34010_state *tms, UINT16 op) { MOVE_AR(1,B); }

#define MOVE_A_NI(F,R)                                          \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	WFIELD##F(tms,*rd,RFIELD##F(tms,PARAM_LONG(tms)));                  \
	*rd+=fw_inc[FW(tms,F)];                                         \
	COUNT_CYCLES(tms,5);                                            \
}
static void move0_a_ni_a (tms34010_state *tms, UINT16 op) { MOVE_A_NI(0,A); }
static void move0_a_ni_b (tms34010_state *tms, UINT16 op) { MOVE_A_NI(0,B); }
static void move1_a_ni_a (tms34010_state *tms, UINT16 op) { MOVE_A_NI(1,A); }
static void move1_a_ni_b (tms34010_state *tms, UINT16 op) { MOVE_A_NI(1,B); }

#define MOVE_AA(F)                                              \
{                                                               \
	UINT32 bitaddrs=PARAM_LONG(tms);                            \
	WFIELD##F(tms,PARAM_LONG(tms),RFIELD##F(tms,bitaddrs));             \
	COUNT_CYCLES(tms,7);                                            \
}
static void move0_aa (tms34010_state *tms, UINT16 op) { MOVE_AA(0); }
static void move1_aa (tms34010_state *tms, UINT16 op) { MOVE_AA(1); }



/***************************************************************************
    PROGRAM CONTROL INSTRUCTIONS
***************************************************************************/

#define CALL(R)                                                 \
{                                                               \
	PUSH(tms, tms->pc);                                                 \
	tms->pc = R##REG(tms,DSTREG(op));                                       \
	CORRECT_ODD_PC(tms,"CALL");                                     \
	COUNT_CYCLES(tms,3);                                            \
}
static void call_a (tms34010_state *tms, UINT16 op) { CALL(A); }
static void call_b (tms34010_state *tms, UINT16 op) { CALL(B); }

static void callr(tms34010_state *tms, UINT16 op)
{
	PUSH(tms, tms->pc+0x10);
	tms->pc += (PARAM_WORD_NO_INC(tms)<<4)+0x10;
	COUNT_CYCLES(tms,3);
}

static void calla(tms34010_state *tms, UINT16 op)
{
	PUSH(tms, tms->pc+0x20);
	tms->pc = PARAM_LONG_NO_INC(tms);
	CORRECT_ODD_PC(tms,"CALLA");
	COUNT_CYCLES(tms,4);
}

#define DSJ(R)                                                  \
{                                                               \
	if (--R##REG(tms,DSTREG(op)))                                       \
	{                                                           \
		tms->pc += (PARAM_WORD_NO_INC(tms)<<4)+0x10;                    \
		COUNT_CYCLES(tms,3);                                        \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD(tms);                                             \
		COUNT_CYCLES(tms,2);                                        \
	}                                                           \
}
static void dsj_a (tms34010_state *tms, UINT16 op) { DSJ(A); }
static void dsj_b (tms34010_state *tms, UINT16 op) { DSJ(B); }

#define DSJEQ(R)                                                \
{                                                               \
	if (Z_FLAG(tms))                                                    \
	{                                                           \
		if (--R##REG(tms,DSTREG(op)))                                   \
		{                                                       \
			tms->pc += (PARAM_WORD_NO_INC(tms)<<4)+0x10;                \
			COUNT_CYCLES(tms,3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD(tms);                                         \
			COUNT_CYCLES(tms,2);                                    \
		}                                                       \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD(tms);                                             \
		COUNT_CYCLES(tms,2);                                        \
	}                                                           \
}
static void dsjeq_a (tms34010_state *tms, UINT16 op) { DSJEQ(A); }
static void dsjeq_b (tms34010_state *tms, UINT16 op) { DSJEQ(B); }

#define DSJNE(R)                                                \
{                                                               \
	if (!Z_FLAG(tms))                                               \
	{                                                           \
		if (--R##REG(tms,DSTREG(op)))                                   \
		{                                                       \
			tms->pc += (PARAM_WORD_NO_INC(tms)<<4)+0x10;                \
			COUNT_CYCLES(tms,3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD(tms);                                         \
			COUNT_CYCLES(tms,2);                                    \
		}                                                       \
	}                                                           \
	else                                                        \
	{                                                           \
		SKIP_WORD(tms);                                             \
		COUNT_CYCLES(tms,2);                                        \
	}                                                           \
}
static void dsjne_a (tms34010_state *tms, UINT16 op) { DSJNE(A); }
static void dsjne_b (tms34010_state *tms, UINT16 op) { DSJNE(B); }

#define DSJS(R)                                                 \
{                                                               \
	if (op & 0x0400)                                        \
	{                                                           \
		if (--R##REG(tms,DSTREG(op)))                                   \
		{                                                       \
			tms->pc -= ((PARAM_K(op))<<4);                              \
			COUNT_CYCLES(tms,2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(tms,3);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (--R##REG(tms,DSTREG(op)))                                   \
		{                                                       \
			tms->pc += ((PARAM_K(op))<<4);                              \
			COUNT_CYCLES(tms,2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(tms,3);                                    \
	}                                                           \
}
static void dsjs_a (tms34010_state *tms, UINT16 op) { DSJS(A); }
static void dsjs_b (tms34010_state *tms, UINT16 op) { DSJS(B); }

static void emu(tms34010_state *tms, UINT16 op)
{
	/* in RUN state, this instruction is a NOP */
	COUNT_CYCLES(tms,6);
}

#define EXGPC(R)                                                \
{                                                               \
	INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	INT32 temppc = *rd;                                         \
	*rd = tms->pc;                                                  \
	tms->pc = temppc;                                               \
	CORRECT_ODD_PC(tms,"EXGPC");                                    \
	COUNT_CYCLES(tms,2);                                            \
}
static void exgpc_a (tms34010_state *tms, UINT16 op) { EXGPC(A); }
static void exgpc_b (tms34010_state *tms, UINT16 op) { EXGPC(B); }

#define GETPC(R)                                                \
{                                                               \
	R##REG(tms,DSTREG(op)) = tms->pc;                                       \
	COUNT_CYCLES(tms,1);                                            \
}
static void getpc_a (tms34010_state *tms, UINT16 op) { GETPC(A); }
static void getpc_b (tms34010_state *tms, UINT16 op) { GETPC(B); }

#define GETST(R)                                                \
{                                                               \
	R##REG(tms,DSTREG(op)) = tms->st;                               \
	COUNT_CYCLES(tms,1);                                            \
}
static void getst_a (tms34010_state *tms, UINT16 op) { GETST(A); }
static void getst_b (tms34010_state *tms, UINT16 op) { GETST(B); }

#define j_xx_8(TAKE)                                            \
{                                                               \
	if (DSTREG(op))                                                 \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			tms->pc += (PARAM_REL8(op) << 4);                           \
			COUNT_CYCLES(tms,2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(tms,1);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			tms->pc = PARAM_LONG_NO_INC(tms);                       \
			CORRECT_ODD_PC(tms,"J_XX_8");                           \
			COUNT_CYCLES(tms,3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_LONG(tms);                                         \
			COUNT_CYCLES(tms,4);                                    \
		}                                                       \
	}                                                           \
}

#define j_xx_0(TAKE)                                            \
{                                                               \
	if (DSTREG(op))                                             \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			tms->pc += (PARAM_REL8(op) << 4);                           \
			COUNT_CYCLES(tms,2);                                    \
		}                                                       \
		else                                                    \
			COUNT_CYCLES(tms,1);                                    \
	}                                                           \
	else                                                        \
	{                                                           \
		if (TAKE)                                               \
		{                                                       \
			tms->pc += (PARAM_WORD_NO_INC(tms)<<4)+0x10;                \
			COUNT_CYCLES(tms,3);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			SKIP_WORD(tms);                                         \
			COUNT_CYCLES(tms,2);                                    \
		}                                                       \
	}                                                           \
}

#define j_xx_x(TAKE)                                            \
{                                                               \
	if (TAKE)                                                   \
	{                                                           \
		tms->pc += (PARAM_REL8(op) << 4);                               \
		COUNT_CYCLES(tms,2);                                        \
	}                                                           \
	else                                                        \
		COUNT_CYCLES(tms,1);                                        \
}

static void j_UC_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(1);
}
static void j_UC_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(1);
}
static void j_UC_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(1);
}
static void j_P_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(!N_FLAG(tms) && !Z_FLAG(tms));
}
static void j_P_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(!N_FLAG(tms) && !Z_FLAG(tms));
}
static void j_P_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(!N_FLAG(tms) && !Z_FLAG(tms));
}
static void j_LS_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(C_FLAG(tms) || Z_FLAG(tms));
}
static void j_LS_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(C_FLAG(tms) || Z_FLAG(tms));
}
static void j_LS_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(C_FLAG(tms) || Z_FLAG(tms));
}
static void j_HI_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(!C_FLAG(tms) && !Z_FLAG(tms));
}
static void j_HI_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(!C_FLAG(tms) && !Z_FLAG(tms));
}
static void j_HI_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(!C_FLAG(tms) && !Z_FLAG(tms));
}
static void j_LT_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0((N_FLAG(tms) && !V_FLAG(tms)) || (!N_FLAG(tms) && V_FLAG(tms)));
}
static void j_LT_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8((N_FLAG(tms) && !V_FLAG(tms)) || (!N_FLAG(tms) && V_FLAG(tms)));
}
static void j_LT_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x((N_FLAG(tms) && !V_FLAG(tms)) || (!N_FLAG(tms) && V_FLAG(tms)));
}
static void j_GE_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0((N_FLAG(tms) && V_FLAG(tms)) || (!N_FLAG(tms) && !V_FLAG(tms)));
}
static void j_GE_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8((N_FLAG(tms) && V_FLAG(tms)) || (!N_FLAG(tms) && !V_FLAG(tms)));
}
static void j_GE_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x((N_FLAG(tms) && V_FLAG(tms)) || (!N_FLAG(tms) && !V_FLAG(tms)));
}
static void j_LE_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0((N_FLAG(tms) && !V_FLAG(tms)) || (!N_FLAG(tms) && V_FLAG(tms)) || Z_FLAG(tms));
}
static void j_LE_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8((N_FLAG(tms) && !V_FLAG(tms)) || (!N_FLAG(tms) && V_FLAG(tms)) || Z_FLAG(tms));
}
static void j_LE_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x((N_FLAG(tms) && !V_FLAG(tms)) || (!N_FLAG(tms) && V_FLAG(tms)) || Z_FLAG(tms));
}
static void j_GT_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0((N_FLAG(tms) && V_FLAG(tms) && !Z_FLAG(tms)) || (!N_FLAG(tms) && !V_FLAG(tms) && !Z_FLAG(tms)));
}
static void j_GT_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8((N_FLAG(tms) && V_FLAG(tms) && !Z_FLAG(tms)) || (!N_FLAG(tms) && !V_FLAG(tms) && !Z_FLAG(tms)));
}
static void j_GT_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x((N_FLAG(tms) && V_FLAG(tms) && !Z_FLAG(tms)) || (!N_FLAG(tms) && !V_FLAG(tms) && !Z_FLAG(tms)));
}
static void j_C_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(C_FLAG(tms));
}
static void j_C_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(C_FLAG(tms));
}
static void j_C_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(C_FLAG(tms));
}
static void j_NC_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(!C_FLAG(tms));
}
static void j_NC_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(!C_FLAG(tms));
}
static void j_NC_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(!C_FLAG(tms));
}
static void j_EQ_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(Z_FLAG(tms));
}
static void j_EQ_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(Z_FLAG(tms));
}
static void j_EQ_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(Z_FLAG(tms));
}
static void j_NE_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(!Z_FLAG(tms));
}
static void j_NE_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(!Z_FLAG(tms));
}
static void j_NE_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(!Z_FLAG(tms));
}
static void j_V_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(V_FLAG(tms));
}
static void j_V_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(V_FLAG(tms));
}
static void j_V_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(V_FLAG(tms));
}
static void j_NV_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(!V_FLAG(tms));
}
static void j_NV_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(!V_FLAG(tms));
}
static void j_NV_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(!V_FLAG(tms));
}
static void j_N_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(N_FLAG(tms));
}
static void j_N_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(N_FLAG(tms));
}
static void j_N_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(N_FLAG(tms));
}
static void j_NN_0(tms34010_state *tms, UINT16 op)
{
	j_xx_0(!N_FLAG(tms));
}
static void j_NN_8(tms34010_state *tms, UINT16 op)
{
	j_xx_8(!N_FLAG(tms));
}
static void j_NN_x(tms34010_state *tms, UINT16 op)
{
	j_xx_x(!N_FLAG(tms));
}

#define JUMP(R)                                                 \
{                                                               \
	tms->pc = R##REG(tms,DSTREG(op));                                       \
	CORRECT_ODD_PC(tms,"JUMP");                                     \
	COUNT_CYCLES(tms,2);                                            \
}
static void jump_a (tms34010_state *tms, UINT16 op) { JUMP(A); }
static void jump_b (tms34010_state *tms, UINT16 op) { JUMP(B); }

static void popst(tms34010_state *tms, UINT16 op)
{
	SET_ST(tms, POP(tms));
	COUNT_CYCLES(tms,8);
}

static void pushst(tms34010_state *tms, UINT16 op)
{
	PUSH(tms, tms->st);
	COUNT_CYCLES(tms,2);
}

#define PUTST(R)                                                \
{                                                               \
	SET_ST(tms, R##REG(tms,DSTREG(op)));                                \
	COUNT_CYCLES(tms,3);                                            \
}
static void putst_a (tms34010_state *tms, UINT16 op) { PUTST(A); }
static void putst_b (tms34010_state *tms, UINT16 op) { PUTST(B); }

static void reti(tms34010_state *tms, UINT16 op)
{
	INT32 st = POP(tms);
	tms->pc = POP(tms);
	CORRECT_ODD_PC(tms,"RETI");
	SET_ST(tms, st);
	COUNT_CYCLES(tms,11);
}

static void rets(tms34010_state *tms, UINT16 op)
{
	UINT32 offs;
	tms->pc = POP(tms);
	CORRECT_ODD_PC(tms,"RETS");
	offs = PARAM_N(op);
	if (offs)
	{
		SP(tms)+=(offs<<4);
	}
	COUNT_CYCLES(tms,7);
}

#define REV(R)                                                  \
{                                                               \
	R##REG(tms,DSTREG(op)) = 0x0008;                                    \
	COUNT_CYCLES(tms,1);                                            \
}
static void rev_a (tms34010_state *tms, UINT16 op) { REV(A); }
static void rev_b (tms34010_state *tms, UINT16 op) { REV(B); }

static void trap(tms34010_state *tms, UINT16 op)
{
	UINT32 t = PARAM_N(op);
	if (t)
	{
		PUSH(tms, tms->pc);
		PUSH(tms, tms->st);
	}
	RESET_ST(tms);
	tms->pc = RLONG(tms, 0xffffffe0-(t<<5));
	CORRECT_ODD_PC(tms,"TRAP");
	COUNT_CYCLES(tms,16);
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
	UINT32 a = PARAM_LONG(tms);                 \
	XY *b = &R##REG_XY(tms,DSTREG(op));                 \
	CLR_NCZV(tms);                                  \
	b->x += (INT16)(a & 0xffff);                \
	b->y += ((INT32)a >> 16);                   \
	SET_N_LOG(tms, b->x == 0);                      \
	SET_C_BIT_LO(tms, b->y, 15);                        \
	SET_Z_LOG(tms, b->y == 0);                      \
	SET_V_BIT_LO(tms, b->x, 15);                        \
	COUNT_CYCLES(tms,1);                            \
}
static void addxyi_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	ADD_XYI(A);
}
static void addxyi_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	ADD_XYI(B);
}

static void blmove(tms34010_state *tms, UINT16 op)
{
	offs_t src = BREG(tms,0);
	offs_t dst = BREG(tms,2);
	offs_t bits = BREG(tms,7);

	if (!tms->is_34020) { unimpl(tms, op); return; }

	/* src and dst are aligned */
	if (!(src & 0x0f) && !(dst & 0x0f))
	{
		while (bits >= 16 && tms->icount > 0)
		{
			TMS34010_WRMEM_WORD(tms, TOBYTE(dst), TMS34010_RDMEM_WORD(tms, TOBYTE(src)));
			src += 0x10;
			dst += 0x10;
			bits -= 0x10;
			tms->icount -= 2;
		}
		if (bits != 0 && tms->icount > 0)
		{
			(*tms34010_wfield_functions[bits])(tms, dst, (*tms34010_rfield_functions[bits])(tms, src));
			dst += bits;
			src += bits;
			bits = 0;
			tms->icount -= 2;
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
	BREG(tms,0) = src;
	BREG(tms,2) = dst;
	BREG(tms,7) = bits;

	/* if we're not done yet, back up the PC */
	if (bits != 0)
		tms->pc -= 0x10;
}

static void cexec_l(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cexec_l\n");
}

static void cexec_s(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cexec_s\n");
}

static void clip(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:clip\n");
}

static void cmovcg_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovcg_a\n");
}

static void cmovcg_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovcg_b\n");
}

static void cmovcm_f(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovcm_f\n");
}

static void cmovcm_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovcm_b\n");
}

static void cmovgc_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovgc_a\n");
}

static void cmovgc_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovgc_b\n");
}

static void cmovgc_a_s(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovgc_a_s\n");
}

static void cmovgc_b_s(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovgc_b_s\n");
}

static void cmovmc_f(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovmc_f\n");
}

static void cmovmc_f_va(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovmc_f_va\n");
}

static void cmovmc_f_vb(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovmc_f_vb\n");
}

static void cmovmc_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cmovmc_b\n");
}

#define CMPK(R)                                             \
{                                                           \
	INT32 r;                                                \
	INT32 *rd = &R##REG(tms,DSTREG(op));                            \
	INT32 t = PARAM_K(op); if (!t) t = 32;                      \
	CLR_NCZV(tms);                                              \
	r = *rd - t;                                            \
	SET_NZCV_SUB(tms,*rd,t,r);                                  \
	COUNT_CYCLES(tms,1);                                        \
}
static void cmp_k_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	CMPK(A);
}
static void cmp_k_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	CMPK(B);
}

static void cvdxyl_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cvdxyl_a\n");
}

static void cvdxyl_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cvdxyl_b\n");
}

static void cvmxyl_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cvmxyl_a\n");
}

static void cvmxyl_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cvmxyl_b\n");
}

static void cvsxyl_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cvsxyl_a\n");
}

static void cvsxyl_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:cvsxyl_b\n");
}

static void exgps_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:exgps_a\n");
}

static void exgps_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:exgps_b\n");
}

static void fline(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:fline\n");
}

static void fpixeq(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:fpixeq\n");
}

static void fpixne(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:fpixne\n");
}

static void getps_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:getps_a\n");
}

static void getps_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:getps_b\n");
}

static void idle(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:idle\n");
}

static void linit(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:linit\n");
}

static void mwait(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
}

static void pfill_xy(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:pfill_xy\n");
}

static void pixblt_l_m_l(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:pixblt_l_m_l\n");
}

static void retm(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:retm\n");
}

#define RMO(R)                                                  \
{                                                               \
	UINT32 res = 0;                                             \
	UINT32 rs  = R##REG(tms,SRCREG(op));                                \
		INT32 *rd = &R##REG(tms,DSTREG(op));                                \
	CLR_Z(tms);                                                     \
	SET_Z_VAL(tms, rs);                                             \
	if (rs)                                                     \
	{                                                           \
		while (!(rs & 0x00000001))                              \
		{                                                       \
			res++;                                              \
			rs >>= 1;                                           \
		}                                                       \
	}                                                           \
	*rd = res;                                                  \
	COUNT_CYCLES(tms,1);                                            \
}

static void rmo_a(tms34010_state *tms, UINT16 op) { RMO(A); }
static void rmo_b(tms34010_state *tms, UINT16 op) { RMO(B); }

#define RPIX(R)                                 \
{                                               \
	UINT32 v = R##REG(tms,DSTREG(op));                  \
	switch (tms->pixelshift)                    \
	{                                           \
		case 0:                                 \
			v = (v & 1) ? 0xffffffff : 0x00000000;\
			COUNT_CYCLES(tms,8);                    \
			break;                              \
		case 1:                                 \
			v &= 3;                             \
			v |= v << 2;                        \
			v |= v << 4;                        \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(tms,7);                    \
			break;                              \
		case 2:                                 \
			v &= 0x0f;                          \
			v |= v << 4;                        \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(tms,6);                    \
			break;                              \
		case 3:                                 \
			v &= 0xff;                          \
			v |= v << 8;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(tms,5);                    \
			break;                              \
		case 4:                                 \
			v &= 0xffff;                        \
			v |= v << 16;                       \
			COUNT_CYCLES(tms,4);                    \
			break;                              \
		case 5:                                 \
			COUNT_CYCLES(tms,2);                    \
			break;                              \
	}                                           \
	R##REG(tms,DSTREG(op)) = v;                         \
}

static void rpix_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	RPIX(A);
}

static void rpix_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	RPIX(B);
}

static void setcdp(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:setcdp\n");
}

static void setcmp(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:setcmp\n");
}

static void setcsp(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:setcsp\n");
}

static void swapf_a(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:swapf_a\n");
}

static void swapf_b(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:swapf_b\n");
}

static void tfill_xy(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:tfill_xy\n");
}

static void trapl(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:trapl\n");
}

static void vblt_b_l(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:vblt_b_l\n");
}

static void vfill_l(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:vfill_l\n");
}

static void vlcol(tms34010_state *tms, UINT16 op)
{
	if (!tms->is_34020) { unimpl(tms, op); return; }
	logerror("020:vlcol\n");
}
