/*****************************************************************************
 *
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 emulator V0.2
 *   Copyright (c) 2001 Juergen Buchmueller, all rights reserved.
 *
 *   7810ops.c  - opcode functions
 *
 *****************************************************************************/

static void illegal(void)
{
	logerror("uPD7810 #%d: illegal opcode %02x at PC:%04x\n", cpu_getactivecpu(), OP, PC);
}

static void illegal2(void)
{
	logerror("uPD7810 #%d: illegal opcode %02x %02x at PC:%04x\n", cpu_getactivecpu(), OP, OP2, PC);
}

/* prefix 48 */

/* 48 01: 0100 1000 0000 0001 */
static void SLRC_A(void)
{
	PSW = (PSW & ~CY) | (A & CY);
	A >>= 1;
	SKIP_CY;
}

/* 48 02: 0100 1000 0000 0010 */
static void SLRC_B(void)
{
	PSW = (PSW & ~CY) | (B & CY);
	B >>= 1;
	SKIP_CY;
}

/* 48 03: 0100 1000 0000 0011 */
static void SLRC_C(void)
{
	PSW = (PSW & ~CY) | (C & CY);
	C >>= 1;
	SKIP_CY;
}


/* 48 05: 0100 1000 0000 0101 */
static void SLLC_A(void)
{
	PSW = (PSW & ~CY) | ((A >> 7) & CY);
	A <<= 1;
	SKIP_CY;
}

/* 48 06: 0100 1000 0000 0110 */
static void SLLC_B(void)
{
	PSW = (PSW & ~CY) | ((B >> 7) & CY);
	B <<= 1;
	SKIP_CY;
}

/* 48 07: 0100 1000 0000 0111 */
static void SLLC_C(void)
{
	PSW = (PSW & ~CY) | ((C >> 7) & CY);
	C <<= 1;
	SKIP_CY;
}

/* 48 08: 0100 1000 0000 1000 */
static void SK_NV(void)
{
	/* 48 skip never */
}

/* 48 0a: 0100 1000 0000 1010 */
static void SK_CY(void)
{
	if (CY == (PSW & CY))
		PSW |= SK;
}

/* 48 0b: 0100 1000 0000 1011 */
static void SK_HC(void)
{
	if (HC == (PSW & HC))
		PSW |= SK;
}

/* 48 0c: 0100 1000 0000 1100 */
static void SK_Z(void)
{
	if (Z == (PSW & Z))
		PSW |= SK;
}

/* 48 18: 0100 1000 0001 1000 */
static void SKN_NV(void)
{
	/* skip not never -> skip always ;-) */
	PSW |= SK;
}

/* 48 1a: 0100 1000 0001 1010 */
static void SKN_CY(void)
{
	if (0 == (PSW & CY))
		PSW |= SK;
}

/* 48 1b: 0100 1000 0001 1011 */
static void SKN_HC(void)
{
	if (0 == (PSW & HC))
		PSW |= SK;
}

/* 48 1c: 0100 1000 0001 1100 */
static void SKN_Z(void)
{
	if (0 == (PSW & Z))
		PSW |= SK;
}

/* 48 21: 0100 1000 0010 0001 */
static void SLR_A(void)
{
	PSW = (PSW & ~CY) | (A & CY);
	A >>= 1;
}

/* 48 22: 0100 1000 0010 0010 */
static void SLR_B(void)
{
	PSW = (PSW & ~CY) | (B & CY);
	B >>= 1;
}

/* 48 23: 0100 1000 0010 0011 */
static void SLR_C(void)
{
	PSW = (PSW & ~CY) | (C & CY);
	C >>= 1;
}

/* 48 25: 0100 1000 0010 0101 */
static void SLL_A(void)
{
	PSW = (PSW & ~CY) | ((A >> 7) & CY);
	A <<= 1;
}

/* 48 26: 0100 1000 0010 0110 */
static void SLL_B(void)
{
	PSW = (PSW & ~CY) | ((B >> 7) & CY);
	B <<= 1;
}

/* 48 27: 0100 1000 0010 0111 */
static void SLL_C(void)
{
	PSW = (PSW & ~CY) | ((C >> 7) & CY);
	C <<= 1;
}

/* 48 28: 0100 1000 0010 1000 */
static void JEA(void)
{
	PC = EA;
	change_pc( PCD );
}

/* 48 29: 0100 1000 0010 1001 */
static void CALB(void)
{
	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PC = BC;
	change_pc( PCD );
}

/* 48 2a: 0100 1000 0010 1010 */
static void CLC(void)
{
	PSW &= ~CY;
}

/* 48 2b: 0100 1000 0010 1011 */
static void STC(void)
{
	PSW |= CY;
}

/* 48 2d: 0100 1000 0010 1101 */
static void MUL_A(void)
{
	EA = A * A;
}

/* 48 2e: 0100 1000 0010 1110 */
static void MUL_B(void)
{
	EA = A * B;
}

/* 48 2f: 0100 1000 0010 1111 */
static void MUL_C(void)
{
	EA = A * C;
}

/* 48 31: 0100 1000 0011 0001 */
static void RLR_A(void)
{
	UINT8 carry=(PSW&CY)<<7;
	PSW = (PSW & ~CY) | (A & CY);
	A = (A >> 1) | carry;
}

/* 48 32: 0100 1000 0011 0010 */
static void RLR_B(void)
{
	UINT8 carry=(PSW&CY)<<7;
	PSW = (PSW & ~CY) | (B & CY);
	B = (B >> 1) | carry;
}

/* 48 33: 0100 1000 0011 0011 */
static void RLR_C(void)
{
	UINT8 carry=(PSW&CY)<<7;
	PSW = (PSW & ~CY) | (C & CY);
	C = (C >> 1) | carry;
}

/* 48 35: 0100 1000 0011 0101 */
static void RLL_A(void)
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((A >> 7) & CY);
	A = (A << 1) | carry;
}

/* 48 36: 0100 1000 0011 0110 */
static void RLL_B(void)
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((B >> 7) & CY);
	B = (B << 1) | carry;
}

/* 48 37: 0100 1000 0011 0111 */
static void RLL_C(void)
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((C >> 7) & CY);
	C = (C << 1) | carry;
}

/* 48 38: 0100 1000 0011 1000 */
static void RLD(void)
{
	UINT8 m = RM( HL ), tmp;
	tmp = (m << 4) | (A & 0x0f);
	A = (A & 0xf0) | (m >> 4);
	WM( HL, tmp );
}

/* 48 39: 0100 1000 0011 1001 */
static void RRD(void)
{
	UINT8 m = RM( HL ), tmp;
	tmp = (A << 4) | (m >> 4);
	A = (A & 0xf0) | (m & 0x0f);
	WM( HL, tmp );
}

/* 48 3a: 0100 1000 0011 1010 */
static void NEGA(void)
{
	A = ~A + 1;
}

/* 48 3b: 0100 1000 0011 1011 */
static void HALT(void)
{
	int cycles = (upd7810_icount / 4) * 4;
	upd7810_icount -= cycles;
	upd7810_timers(cycles);
	PC -= 1;		/* continue executing HALT */
}

/* 48 3d: 0100 1000 0011 1101 */
static void DIV_A(void)
{
	if (A)
	{
		UINT8 remainder;
		remainder = EA % A;
		EA /= A;
		A = remainder;
	}
	else
		EA = 0xffff;	/* guess */
}

/* 48 3e: 0100 1000 0011 1110 */
static void DIV_B(void)
{
	if (B)
	{
		UINT8 remainder;
		remainder = EA % B;
		EA /= B;
		B = remainder;
	}
	else
		EA = 0xffff;	/* guess */
}

/* 48 3f: 0100 1000 0011 1111 */
static void DIV_C(void)
{
	if (C)
	{
		UINT8 remainder;
		remainder = EA % C;
		EA /= C;
		C = remainder;
	}
	else
		EA = 0xffff;	/* guess */
}

/* 48 40: 0100 1000 0100 0000 */
static void SKIT_NMI(void)
{
	if (IRR & INTNMI)
		PSW |= SK;
	IRR &= ~INTNMI;
}

/* 48 41: 0100 1000 0100 0001 */
static void SKIT_FT0(void)
{
	if (IRR & INTFT0)
		PSW |= SK;
	IRR &= ~INTFT0;
}

/* 48 42: 0100 1000 0100 0010 */
static void SKIT_FT1(void)
{
	if (IRR & INTFT1)
		PSW |= SK;
	IRR &= ~INTFT1;
}

/* 48 43: 0100 1000 0100 0011 */
static void SKIT_F1(void)
{
	if (IRR & INTF1)
		PSW |= SK;
	IRR &= ~INTF1;
}

/* 48 44: 0100 1000 0100 0100 */
static void SKIT_F2(void)
{
	if (IRR & INTF2)
		PSW |= SK;
	IRR &= ~INTF2;
}

/* 48 45: 0100 1000 0100 0101 */
static void SKIT_FE0(void)
{
	if (IRR & INTFE0)
		PSW |= SK;
	IRR &= ~INTFE0;
}

/* 48 46: 0100 1000 0100 0110 */
static void SKIT_FE1(void)
{
	if (IRR & INTFE1)
		PSW |= SK;
	IRR &= ~INTFE1;
}

/* 48 47: 0100 1000 0100 0111 */
static void SKIT_FEIN(void)
{
	if (IRR & INTFEIN)
		PSW |= SK;
	IRR &= ~INTFEIN;
}

/* 48 48: 0100 1000 0100 1000 */
static void SKIT_FAD(void)
{
	if (IRR & INTFAD)
		PSW |= SK;
	IRR &= ~INTFAD;
}

/* 48 49: 0100 1000 0100 1001 */
static void SKIT_FSR(void)
{
	if (IRR & INTFSR)
		PSW |= SK;
	IRR &= ~INTFSR;
}

/* 48 4a: 0100 1000 0100 1010 */
static void SKIT_FST(void)
{
	if (IRR & INTFST)
		PSW |= SK;
	IRR &= ~INTFST;
}

/* 48 4b: 0100 1000 0100 1011 */
static void SKIT_ER(void)
{
	if (IRR & INTER)
		PSW |= SK;
	IRR &= ~INTER;
}

/* 48 4c: 0100 1000 0100 1100 */
static void SKIT_OV(void)
{
	if (IRR & INTOV)
		PSW |= SK;
	IRR &= ~INTOV;
}

/* 48 50: 0100 1000 0101 0000 */
static void SKIT_AN4(void)
{
	if (ITF & INTAN4)
		PSW |= SK;
	ITF &= ~INTAN4;
}

/* 48 51: 0100 1000 0101 0001 */
static void SKIT_AN5(void)
{
	if (ITF & INTAN5)
		PSW |= SK;
	ITF &= ~INTAN5;
}

/* 48 52: 0100 1000 0101 0010 */
static void SKIT_AN6(void)
{
	if (ITF & INTAN6)
		PSW |= SK;
	ITF &= ~INTAN6;
}

/* 48 53: 0100 1000 0101 0011 */
static void SKIT_AN7(void)
{
	if (ITF & INTAN7)
		PSW |= SK;
	ITF &= ~INTAN7;
}

/* 48 54: 0100 1000 0101 0100 */
static void SKIT_SB(void)
{
	if (ITF & INTSB)
		PSW |= SK;
	ITF &= ~INTSB;
}

/* 48 60: 0100 1000 0110 0000 */
static void SKNIT_NMI(void)
{
	if (0 == (IRR & INTNMI))
		PSW |= SK;
	IRR &= ~INTNMI;
}

/* 48 61: 0100 1000 0110 0001 */
static void SKNIT_FT0(void)
{
	if (0 == (IRR & INTFT0))
		PSW |= SK;
	IRR &= ~INTFT0;
}

/* 48 62: 0100 1000 0110 0010 */
static void SKNIT_FT1(void)
{
	if (0 == (IRR & INTFT1))
		PSW |= SK;
	IRR &= ~INTFT1;
}

/* 48 63: 0100 1000 0110 0011 */
static void SKNIT_F1(void)
{
	if (0 == (IRR & INTF1))
		PSW |= SK;
	IRR &= ~INTF1;
}

/* 48 64: 0100 1000 0110 0100 */
static void SKNIT_F2(void)
{
	if (0 == (IRR & INTF2))
		PSW |= SK;
	IRR &= ~INTF2;
}

/* 48 65: 0100 1000 0110 0101 */
static void SKNIT_FE0(void)
{
	if (0 == (IRR & INTFE0))
		PSW |= SK;
	IRR &= ~INTFE0;
}

/* 48 66: 0100 1000 0110 0110 */
static void SKNIT_FE1(void)
{
	if (0 == (IRR & INTFE1))
		PSW |= SK;
	IRR &= ~INTFE1;
}

/* 48 67: 0100 1000 0110 0111 */
static void SKNIT_FEIN(void)
{
	if (0 == (IRR & INTFEIN))
		PSW |= SK;
	IRR &= ~INTFEIN;
}

/* 48 68: 0100 1000 0110 1000 */
static void SKNIT_FAD(void)
{
	if (0 == (IRR & INTFAD))
		PSW |= SK;
	IRR &= ~INTFAD;
}

/* 48 69: 0100 1000 0110 1001 */
static void SKNIT_FSR(void)
{
	if (0 == (IRR & INTFSR))
		PSW |= SK;
	IRR &= ~INTFSR;
}

/* 48 6a: 0100 1000 0110 1010 */
static void SKNIT_FST(void)
{
	if (0 == (IRR & INTFST))
		PSW |= SK;
	IRR &= ~INTFST;
}

/* 48 6b: 0100 1000 0110 1011 */
static void SKNIT_ER(void)
{
	if (0 == (IRR & INTER))
		PSW |= SK;
	IRR &= ~INTER;
}

/* 48 6c: 0100 1000 0110 1100 */
static void SKNIT_OV(void)
{
	if (0 == (IRR & INTOV))
		PSW |= SK;
	IRR &= ~INTOV;
}

/* 48 70: 0100 1000 0111 0000 */
static void SKNIT_AN4(void)
{
	if (0 == (ITF & INTAN4))
		PSW |= SK;
	ITF &= ~INTAN4;
}

/* 48 71: 0100 1000 0111 0001 */
static void SKNIT_AN5(void)
{
	if (0 == (ITF & INTAN5))
		PSW |= SK;
	ITF &= ~INTAN5;
}

/* 48 72: 0100 1000 0111 0010 */
static void SKNIT_AN6(void)
{
	if (0 == (ITF & INTAN6))
		PSW |= SK;
	ITF &= ~INTAN6;
}

/* 48 73: 0100 1000 0111 0011 */
static void SKNIT_AN7(void)
{
	if (0 == (ITF & INTAN7))
		PSW |= SK;
	ITF &= ~INTAN7;
}

/* 48 74: 0100 1000 0111 0100 */
static void SKNIT_SB(void)
{
	if (0 == (ITF & INTSB))
		PSW |= SK;
	ITF &= ~INTSB;
}

/* 48 82: 0100 1000 1000 0010 */
static void LDEAX_D(void)
{
	EAL = RM( DE );
	EAH = RM( DE + 1 );
}

/* 48 83: 0100 1000 1000 0011 */
static void LDEAX_H(void)
{
	EAL = RM( HL );
	EAH = RM( HL + 1 );
}

/* 48 84: 0100 1000 1000 0100 */
static void LDEAX_Dp(void)
{
	EAL = RM( DE );
	EAH = RM( DE + 1 );
	DE += 2;
}

/* 48 85: 0100 1000 1000 0101 */
static void LDEAX_Hp(void)
{
	EAL = RM( HL );
	EAH = RM( HL + 1 );
	HL += 2;
}

/* 48 8b: 0100 1000 1000 1011 xxxx xxxx */
static void LDEAX_D_xx(void)
{
	UINT16 ea;
	RDOPARG( ea );
	ea += DE;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8c: 0100 1000 1000 1100 */
static void LDEAX_H_A(void)
{
	UINT16 ea = HL + A;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8d: 0100 1000 1000 1101 */
static void LDEAX_H_B(void)
{
	UINT16 ea = HL + B;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8e: 0100 1000 1000 1110 */
static void LDEAX_H_EA(void)
{
	UINT16 ea = HL + EA;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8f: 0100 1000 1000 1111 xxxx xxxx */
static void LDEAX_H_xx(void)
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 92: 0100 1000 1000 0010 */
static void STEAX_D(void)
{
	WM( DE, EAL );
	WM( DE + 1, EAH );
}

/* 48 93: 0100 1000 1000 0011 */
static void STEAX_H(void)
{
	WM( HL, EAL );
	WM( HL + 1, EAH );
}

/* 48 94: 0100 1000 1000 0100 */
static void STEAX_Dp(void)
{
	WM( DE, EAL );
	WM( DE + 1, EAH );
	DE += 2;
}

/* 48 95: 0100 1000 1000 0101 */
static void STEAX_Hp(void)
{
	WM( HL, EAL );
	WM( HL + 1, EAH );
	HL += 2;
}

/* 48 9b: 0100 1000 1000 1011 xxxx xxxx */
static void STEAX_D_xx(void)
{
	UINT16 ea;
	RDOPARG( ea );
	ea += DE;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9c: 0100 1000 1000 1100 */
static void STEAX_H_A(void)
{
	UINT16 ea = HL + A;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9d: 0100 1000 1000 1101 */
static void STEAX_H_B(void)
{
	UINT16 ea = HL + B;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9e: 0100 1000 1000 1110 */
static void STEAX_H_EA(void)
{
	UINT16 ea = HL + EA;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9f: 0100 1000 1000 1111 xxxx xxxx */
static void STEAX_H_xx(void)
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 a0: 0100 1000 1010 0000 */
static void DSLR_EA(void)
{
	PSW = (PSW & ~CY) | (EA & CY);
	EA >>= 1;
}

/* 48 a4: 0100 1000 1010 0100 */
static void DSLL_EA(void)
{
	PSW = (PSW & ~CY) | ((EA >> 15) & CY);
	EA <<= 1;
}

/* 48 a8: 0100 1000 1010 1000 */
static void TABLE(void)
{
	UINT16 ea = PC + A + 1;
	C = RM( ea );
	B = RM( ea + 1 );
}

/* 48 b0: 0100 1000 1011 0000 */
static void DRLR_EA(void)
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | (EA & CY);
	EA = (EA >> 1) | (carry << 15);
}

/* 48 b4: 0100 1000 1011 0100 */
static void DRLL_EA(void)
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((EA >> 15) & CY);
	EA = (EA << 1) | carry;
}

/* 48 bb: 0100 1000 1011 1011 */
static void STOP(void)
{
	int cycles = (upd7810_icount / 4) * 4;
	upd7810_icount -= cycles;
	upd7810_timers(cycles);
	PC -= 1;
}

/* 48 c0: 0100 1000 1100 0000 */
static void DMOV_EA_ECNT(void)
{
	EA = ECNT;
}

/* 48 c1: 0100 1000 1100 0001 */
static void DMOV_EA_ECPT(void)
{
	EA = ECPT;
}

/* 48 d2: 0100 1000 1101 0010 */
static void DMOV_ETM0_EA(void)
{
	ETM0 = EA;
}

/* 48 d3: 0100 1000 1101 0011 */
static void DMOV_ETM1_EA(void)
{
	ETM1 = EA;
}

/* prefix 4C */
/* 4c c0: 0100 1100 1100 0000 */
static void MOV_A_PA(void)
{
	A = RP( UPD7810_PORTA );
}

/* 4c c1: 0100 1100 1100 0001 */
static void MOV_A_PB(void)
{
	A = RP( UPD7810_PORTB );
}

/* 4c c2: 0100 1100 1100 0010 */
static void MOV_A_PC(void)
{
	A = RP( UPD7810_PORTC );
}

/* 4c c3: 0100 1100 1100 0011 */
static void MOV_A_PD(void)
{
	A = RP( UPD7810_PORTD );
}

/* 4c c5: 0100 1100 1100 0101 */
static void MOV_A_PF(void)
{
	A = RP( UPD7810_PORTF );
}

/* 4c c6: 0100 1100 1100 0110 */
static void MOV_A_MKH(void)
{
	A = MKH;
}

/* 4c c7: 0100 1100 1100 0111 */
static void MOV_A_MKL(void)
{
	A = MKL;
}

/* 4c c8: 0100 1100 1100 1000 */
static void MOV_A_ANM(void)
{
	A = ANM;
}

/* 4c c9: 0100 1100 1100 1001 */
static void MOV_A_SMH(void)
{
	A = SMH;
}

/* 4c cb: 0100 1100 1100 1011 */
static void MOV_A_EOM(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	A = eom;
}

/* 4c cd: 0100 1100 1100 1101 */
static void MOV_A_TMM(void)
{
	A = TMM;
}

/* 4c ce: 0100 1100 1110 0000 (7807 only) */
static void MOV_A_PT(void)
{
	A = RP( UPD7807_PORTT );
}

/* 4c d9: 0100 1100 1101 1001 */
static void MOV_A_RXB(void)
{
	A = RXB;
}

/* 4c e0: 0100 1100 1110 0000 */
static void MOV_A_CR0(void)
{
	A = CR0;
}

/* 4c e1: 0100 1100 1110 0001 */
static void MOV_A_CR1(void)
{
	A = CR1;
}

/* 4c e2: 0100 1100 1110 0010 */
static void MOV_A_CR2(void)
{
	A = CR2;
}

/* 4c e3: 0100 1100 1110 0011 */
static void MOV_A_CR3(void)
{
	A = CR3;
}

/* prefix 4D */
/* 4d c0: 0100 1101 1100 0000 */
static void MOV_PA_A(void)
{
	WP( UPD7810_PORTA, A );
}

/* 4d c1: 0100 1101 1100 0001 */
static void MOV_PB_A(void)
{
	WP( UPD7810_PORTB, A );
}

/* 4d c2: 0100 1101 1100 0010 */
static void MOV_PC_A(void)
{
	WP( UPD7810_PORTC, A );
}

/* 4d c3: 0100 1101 1100 0011 */
static void MOV_PD_A(void)
{
	WP( UPD7810_PORTD, A );
}

/* 4d c5: 0100 1101 1100 0101 */
static void MOV_PF_A(void)
{
	WP( UPD7810_PORTF, A );
}

/* 4d c6: 0100 1101 1100 0110 */
static void MOV_MKH_A(void)
{
	MKH = A;
}

/* 4d c7: 0100 1101 1100 0111 */
static void MOV_MKL_A(void)
{
	MKL = A;
}

/* 4d c8: 0100 1101 1100 1000 */
static void MOV_ANM_A(void)
{
	ANM = A;
}

/* 4d c9: 0100 1101 1100 1001 */
static void MOV_SMH_A(void)
{
	SMH = A;
}

/* 4d ca: 0100 1101 1100 1010 */
static void MOV_SML_A(void)
{
	SML = A;
}

/* 4d cb: 0100 1101 1100 1011 */
static void MOV_EOM_A(void)
{
	EOM = A;
	upd7810_write_EOM();
}

/* 4d cc: 0100 1101 1100 1100 */
static void MOV_ETMM_A(void)
{
	ETMM = A;
}

/* 4d cd: 0100 1101 1100 1101 */
static void MOV_TMM_A(void)
{
	TMM = A;
}

/* 4d d0: 0100 1101 1101 0000 */
static void MOV_MM_A(void)
{
	MM = A;
}

/* 4d d1: 0100 1101 1101 0001 */
static void MOV_MCC_A(void)
{
	MCC = A;
}

/* 4d d2: 0100 1101 1101 0010 */
static void MOV_MA_A(void)
{
	MA = A;
}

/* 4d d3: 0100 1101 1101 0011 */
static void MOV_MB_A(void)
{
	MB = A;
}

/* 4d d4: 0100 1101 1101 0100 */
static void MOV_MC_A(void)
{
	MC = A;
}

/* 4d d7: 0100 1101 1101 0111 */
static void MOV_MF_A(void)
{
	MF = A;
}

/* 4d d8: 0100 1101 1101 1000 */
static void MOV_TXB_A(void)
{
	TXB = A;
	upd7810_write_TXB();
}

/* 4d da: 0100 1101 1101 1010 */
static void MOV_TM0_A(void)
{
	TM0 = A;
}

/* 4d db: 0100 1101 1101 1011 */
static void MOV_TM1_A(void)
{
	TM1 = A;
}

/* 4d e8: 0100 1101 1110 1000 */
static void MOV_ZCM_A(void)
{
	ZCM = A;
}

/* prefix 60 */
/* 60 08: 0110 0000 0000 1000 */
static void ANA_V_A(void)
{
	V &= A;
	SET_Z(V);
}

/* 60 09: 0110 0000 0000 1001 */
static void ANA_A_A(void)
{
	A &= A;
	SET_Z(A);
}

/* 60 0a: 0110 0000 0000 1010 */
static void ANA_B_A(void)
{
	B &= A;
	SET_Z(B);
}

/* 60 0b: 0110 0000 0000 1011 */
static void ANA_C_A(void)
{
	C &= A;
	SET_Z(C);
}

/* 60 0c: 0110 0000 0000 1100 */
static void ANA_D_A(void)
{
	D &= A;
	SET_Z(D);
}

/* 60 0d: 0110 0000 0000 1101 */
static void ANA_E_A(void)
{
	E &= A;
	SET_Z(E);
}

/* 60 0e: 0110 0000 0000 1110 */
static void ANA_H_A(void)
{
	H &= A;
	SET_Z(H);
}

/* 60 0f: 0110 0000 0000 1111 */
static void ANA_L_A(void)
{
	L &= A;
	SET_Z(L);
}

/* 60 10: 0110 0000 0001 0000 */
static void XRA_V_A(void)
{
	V ^= A;
	SET_Z(V);
}

/* 60 11: 0110 0000 0001 0001 */
static void XRA_A_A(void)
{
	A ^= A;
	SET_Z(A);
}

/* 60 12: 0110 0000 0001 0010 */
static void XRA_B_A(void)
{
	B ^= A;
	SET_Z(B);
}

/* 60 13: 0110 0000 0001 0011 */
static void XRA_C_A(void)
{
	C ^= A;
	SET_Z(C);
}

/* 60 14: 0110 0000 0001 0100 */
static void XRA_D_A(void)
{
	D ^= A;
	SET_Z(D);
}

/* 60 15: 0110 0000 0001 0101 */
static void XRA_E_A(void)
{
	E ^= A;
	SET_Z(E);
}

/* 60 16: 0110 0000 0001 0110 */
static void XRA_H_A(void)
{
	H ^= A;
	SET_Z(H);
}

/* 60 17: 0110 0000 0001 0111 */
static void XRA_L_A(void)
{
	L ^= A;
	SET_Z(L);
}

/* 60 18: 0110 0000 0001 1000 */
static void ORA_V_A(void)
{
	V |= A;
	SET_Z(V);
}

/* 60 19: 0110 0000 0001 1001 */
static void ORA_A_A(void)
{
	A |= A;
	SET_Z(A);
}

/* 60 1a: 0110 0000 0001 1010 */
static void ORA_B_A(void)
{
	B |= A;
	SET_Z(B);
}

/* 60 1b: 0110 0000 0001 1011 */
static void ORA_C_A(void)
{
	C |= A;
	SET_Z(C);
}

/* 60 1c: 0110 0000 0001 1100 */
static void ORA_D_A(void)
{
	D |= A;
	SET_Z(D);
}

/* 60 1d: 0110 0000 0001 1101 */
static void ORA_E_A(void)
{
	E |= A;
	SET_Z(E);
}

/* 60 1e: 0110 0000 0001 1110 */
static void ORA_H_A(void)
{
	H |= A;
	SET_Z(H);
}

/* 60 1f: 0110 0000 0001 1111 */
static void ORA_L_A(void)
{
	L |= A;
	SET_Z(L);
}

/* 60 20: 0110 0000 0010 0000 */
static void ADDNC_V_A(void)
{
	UINT8 tmp = V + A;
	ZHC_ADD( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 60 21: 0110 0000 0010 0001 */
static void ADDNC_A_A(void)
{
	UINT8 tmp = A + A;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 22: 0110 0000 0010 0010 */
static void ADDNC_B_A(void)
{
	UINT8 tmp = B + A;
	ZHC_ADD( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 60 23: 0110 0000 0010 0011 */
static void ADDNC_C_A(void)
{
	UINT8 tmp = C + A;
	ZHC_ADD( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 60 24: 0110 0000 0010 0100 */
static void ADDNC_D_A(void)
{
	UINT8 tmp = D + A;
	ZHC_ADD( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 60 25: 0110 0000 0010 0101 */
static void ADDNC_E_A(void)
{
	UINT8 tmp = E + A;
	ZHC_ADD( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 60 26: 0110 0000 0010 0110 */
static void ADDNC_H_A(void)
{
	UINT8 tmp = H + A;
	ZHC_ADD( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 60 27: 0110 0000 0010 0111 */
static void ADDNC_L_A(void)
{
	UINT8 tmp = L + A;
	ZHC_ADD( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 60 28: 0110 0000 0010 1000 */
static void GTA_V_A(void)
{
	UINT16 tmp = V - A - 1;
	ZHC_SUB( tmp, V, 0 );
	SKIP_NC;
}

/* 60 29: 0110 0000 0010 1001 */
static void GTA_A_A(void)
{
	UINT16 tmp = A - A - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 2a: 0110 0000 0010 1010 */
static void GTA_B_A(void)
{
	UINT16 tmp = B - A - 1;
	ZHC_SUB( tmp, B, 0 );
	SKIP_NC;
}

/* 60 2b: 0110 0000 0010 1011 */
static void GTA_C_A(void)
{
	UINT16 tmp = C - A - 1;
	ZHC_SUB( tmp, C, 0 );
	SKIP_NC;
}

/* 60 2c: 0110 0000 0010 1100 */
static void GTA_D_A(void)
{
	UINT16 tmp = D - A - 1;
	ZHC_SUB( tmp, D, 0 );
	SKIP_NC;
}

/* 60 2d: 0110 0000 0010 1101 */
static void GTA_E_A(void)
{
	UINT16 tmp = E - A - 1;
	ZHC_SUB( tmp, E, 0 );
	SKIP_NC;
}

/* 60 2e: 0110 0000 0010 1110 */
static void GTA_H_A(void)
{
	UINT16 tmp = H - A - 1;
	ZHC_SUB( tmp, H, 0 );
	SKIP_NC;
}

/* 60 2f: 0110 0000 0010 1111 */
static void GTA_L_A(void)
{
	UINT16 tmp = L - A - 1;
	ZHC_SUB( tmp, L, 0 );
	SKIP_NC;
}

/* 60 30: 0110 0000 0011 0000 */
static void SUBNB_V_A(void)
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 60 31: 0110 0000 0011 0001 */
static void SUBNB_A_A(void)
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 32: 0110 0000 0011 0010 */
static void SUBNB_B_A(void)
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 60 33: 0110 0000 0011 0011 */
static void SUBNB_C_A(void)
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 60 34: 0110 0000 0011 0100 */
static void SUBNB_D_A(void)
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 60 35: 0110 0000 0011 0101 */
static void SUBNB_E_A(void)
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 60 36: 0110 0000 0011 0110 */
static void SUBNB_H_A(void)
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 60 37: 0110 0000 0011 0111 */
static void SUBNB_L_A(void)
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 60 38: 0110 0000 0011 1000 */
static void LTA_V_A(void)
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	SKIP_CY;
}

/* 60 39: 0110 0000 0011 1001 */
static void LTA_A_A(void)
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 3a: 0110 0000 0011 1010 */
static void LTA_B_A(void)
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	SKIP_CY;
}

/* 60 3b: 0110 0000 0011 1011 */
static void LTA_C_A(void)
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	SKIP_CY;
}

/* 60 3c: 0110 0000 0011 1100 */
static void LTA_D_A(void)
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	SKIP_CY;
}

/* 60 3d: 0110 0000 0011 1101 */
static void LTA_E_A(void)
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	SKIP_CY;
}

/* 60 3e: 0110 0000 0011 1110 */
static void LTA_H_A(void)
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	SKIP_CY;
}

/* 60 3f: 0110 0000 0011 1111 */
static void LTA_L_A(void)
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	SKIP_CY;
}

/* 60 40: 0110 0000 0100 0000 */
static void ADD_V_A(void)
{
	UINT8 tmp = V + A;
	ZHC_ADD( tmp, V, 0 );
	V = tmp;
}

/* 60 41: 0110 0000 0100 0001 */
static void ADD_A_A(void)
{
	UINT8 tmp = A + A;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 42: 0110 0000 0100 0010 */
static void ADD_B_A(void)
{
	UINT8 tmp = B + A;
	ZHC_ADD( tmp, B, 0 );
	B = tmp;
}

/* 60 43: 0110 0000 0100 0011 */
static void ADD_C_A(void)
{
	UINT8 tmp = C + A;
	ZHC_ADD( tmp, C, 0 );
	C = tmp;
}

/* 60 44: 0110 0000 0100 0100 */
static void ADD_D_A(void)
{
	UINT8 tmp = D + A;
	ZHC_ADD( tmp, D, 0 );
	D = tmp;
}

/* 60 45: 0110 0000 0100 0101 */
static void ADD_E_A(void)
{
	UINT8 tmp = E + A;
	ZHC_ADD( tmp, E, 0 );
	E = tmp;
}

/* 60 46: 0110 0000 0100 0110 */
static void ADD_H_A(void)
{
	UINT8 tmp = H + A;
	ZHC_ADD( tmp, H, 0 );
	H = tmp;
}

/* 60 47: 0110 0000 0100 0111 */
static void ADD_L_A(void)
{
	UINT8 tmp = L + A;
	ZHC_ADD( tmp, L, 0 );
	L = tmp;
}

/* 60 50: 0110 0000 0101 0000 */
static void ADC_V_A(void)
{
	UINT8 tmp = V + A + (PSW & CY);
	ZHC_ADD( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 60 51: 0110 0000 0101 0001 */
static void ADC_A_A(void)
{
	UINT8 tmp = A + A + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 52: 0110 0000 0101 0010 */
static void ADC_B_A(void)
{
	UINT8 tmp = B + A + (PSW & CY);
	ZHC_ADD( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 60 53: 0110 0000 0101 0011 */
static void ADC_C_A(void)
{
	UINT8 tmp = C + A + (PSW & CY);
	ZHC_ADD( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 60 54: 0110 0000 0101 0100 */
static void ADC_D_A(void)
{
	UINT8 tmp = D + A + (PSW & CY);
	ZHC_ADD( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 60 55: 0110 0000 0101 0101 */
static void ADC_E_A(void)
{
	UINT8 tmp = E + A + (PSW & CY);
	ZHC_ADD( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 60 56: 0110 0000 0101 0110 */
static void ADC_H_A(void)
{
	UINT8 tmp = H + A + (PSW & CY);
	ZHC_ADD( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 60 57: 0110 0000 0101 0111 */
static void ADC_L_A(void)
{
	UINT8 tmp = L + A + (PSW & CY);
	ZHC_ADD( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 60 60: 0110 0000 0110 0000 */
static void SUB_V_A(void)
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
}

/* 60 61: 0110 0000 0110 0001 */
static void SUB_A_A(void)
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 62: 0110 0000 0110 0010 */
static void SUB_B_A(void)
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
}

/* 60 63: 0110 0000 0110 0011 */
static void SUB_C_A(void)
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
}

/* 60 64: 0110 0000 0110 0100 */
static void SUB_D_A(void)
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
}

/* 60 65: 0110 0000 0110 0101 */
static void SUB_E_A(void)
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
}

/* 60 66: 0110 0000 0110 0110 */
static void SUB_H_A(void)
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
}

/* 60 67: 0110 0000 0110 0111 */
static void SUB_L_A(void)
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
}

/* 60 68: 0110 0000 0110 1000 */
static void NEA_V_A(void)
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	SKIP_NZ;
}

/* 60 69: 0110 0000 0110 1001 */
static void NEA_A_A(void)
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 6a: 0110 0000 0110 1010 */
static void NEA_B_A(void)
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	SKIP_NZ;
}

/* 60 6b: 0110 0000 0110 1011 */
static void NEA_C_A(void)
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	SKIP_NZ;
}

/* 60 6c: 0110 0000 0110 1100 */
static void NEA_D_A(void)
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	SKIP_NZ;
}

/* 60 6d: 0110 0000 0110 1101 */
static void NEA_E_A(void)
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	SKIP_NZ;
}

/* 60 6e: 0110 0000 0110 1110 */
static void NEA_H_A(void)
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	SKIP_NZ;
}

/* 60 6f: 0110 0000 0110 1111 */
static void NEA_L_A(void)
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	SKIP_NZ;
}

/* 60 70: 0110 0000 0111 0000 */
static void SBB_V_A(void)
{
	UINT8 tmp = V - A - (PSW & CY);
	ZHC_SUB( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 60 71: 0110 0000 0111 0001 */
static void SBB_A_A(void)
{
	UINT8 tmp = A - A - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 72: 0110 0000 0111 0010 */
static void SBB_B_A(void)
{
	UINT8 tmp = B - A - (PSW & CY);
	ZHC_SUB( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 60 73: 0110 0000 0111 0011 */
static void SBB_C_A(void)
{
	UINT8 tmp = C - A - (PSW & CY);
	ZHC_SUB( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 60 74: 0110 0000 0111 0100 */
static void SBB_D_A(void)
{
	UINT8 tmp = D - A - (PSW & CY);
	ZHC_SUB( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 60 75: 0110 0000 0111 0101 */
static void SBB_E_A(void)
{
	UINT8 tmp = E - A - (PSW & CY);
	ZHC_SUB( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 60 76: 0110 0000 0111 0110 */
static void SBB_H_A(void)
{
	UINT8 tmp = H - A - (PSW & CY);
	ZHC_SUB( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 60 77: 0110 0000 0111 0111 */
static void SBB_L_A(void)
{
	UINT8 tmp = L - A - (PSW & CY);
	ZHC_SUB( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 60 78: 0110 0000 0111 1000 */
static void EQA_V_A(void)
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	SKIP_Z;
}

/* 60 79: 0110 0000 0111 1001 */
static void EQA_A_A(void)
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 7a: 0110 0000 0111 1010 */
static void EQA_B_A(void)
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	SKIP_Z;
}

/* 60 7b: 0110 0000 0111 1011 */
static void EQA_C_A(void)
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	SKIP_Z;
}

/* 60 7c: 0110 0000 0111 1100 */
static void EQA_D_A(void)
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	SKIP_Z;
}

/* 60 7d: 0110 0000 0111 1101 */
static void EQA_E_A(void)
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	SKIP_Z;
}

/* 60 7e: 0110 0000 0111 1110 */
static void EQA_H_A(void)
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	SKIP_Z;
}

/* 60 7f: 0110 0000 0111 1111 */
static void EQA_L_A(void)
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	SKIP_Z;
}

/* 60 88: 0110 0000 1000 1000 */
static void ANA_A_V(void)
{
	A &= V;
	SET_Z(A);
}

/* 60 89: 0110 0000 1000 1001 */
/* ANA_A_A already defined */

/* 60 8a: 0110 0000 1000 1010 */
static void ANA_A_B(void)
{
	A &= B;
	SET_Z(A);
}

/* 60 8b: 0110 0000 1000 1011 */
static void ANA_A_C(void)
{
	A &= C;
	SET_Z(A);
}

/* 60 8c: 0110 0000 1000 1100 */
static void ANA_A_D(void)
{
	A &= D;
	SET_Z(A);
}

/* 60 8d: 0110 0000 1000 1101 */
static void ANA_A_E(void)
{
	A &= E;
	SET_Z(A);
}

/* 60 8e: 0110 0000 1000 1110 */
static void ANA_A_H(void)
{
	A &= H;
	SET_Z(A);
}

/* 60 8f: 0110 0000 1000 1111 */
static void ANA_A_L(void)
{
	A &= L;
	SET_Z(A);
}

/* 60 90: 0110 0000 1001 0000 */
static void XRA_A_V(void)
{
	A ^= V;
	SET_Z(A);
}

/* 60 91: 0110 0000 1001 0001 */
/* XRA_A_A already defined */

/* 60 92: 0110 0000 1001 0010 */
static void XRA_A_B(void)
{
	A ^= B;
	SET_Z(A);
}

/* 60 93: 0110 0000 1001 0011 */
static void XRA_A_C(void)
{
	A ^= C;
	SET_Z(A);
}

/* 60 94: 0110 0000 1001 0100 */
static void XRA_A_D(void)
{
	A ^= D;
	SET_Z(A);
}

/* 60 95: 0110 0000 1001 0101 */
static void XRA_A_E(void)
{
	A ^= E;
	SET_Z(A);
}

/* 60 96: 0110 0000 1001 0110 */
static void XRA_A_H(void)
{
	A ^= H;
	SET_Z(A);
}

/* 60 97: 0110 0000 1001 0111 */
static void XRA_A_L(void)
{
	A ^= L;
	SET_Z(A);
}

/* 60 98: 0110 0000 1001 1000 */
static void ORA_A_V(void)
{
	A |= V;
	SET_Z(A);
}

/* 60 99: 0110 0000 1001 1001 */
/* ORA_A_A already defined */

/* 60 9a: 0110 0000 1001 1010 */
static void ORA_A_B(void)
{
	A |= B;
	SET_Z(A);
}

/* 60 9b: 0110 0000 1001 1011 */
static void ORA_A_C(void)
{
	A |= C;
	SET_Z(A);
}

/* 60 9c: 0110 0000 1001 1100 */
static void ORA_A_D(void)
{
	A |= D;
	SET_Z(A);
}

/* 60 9d: 0110 0000 1001 1101 */
static void ORA_A_E(void)
{
	A |= E;
	SET_Z(A);
}

/* 60 9e: 0110 0000 1001 1110 */
static void ORA_A_H(void)
{
	A |= H;
	SET_Z(A);
}

/* 60 9f: 0110 0000 1001 1111 */
static void ORA_A_L(void)
{
	A |= L;
	SET_Z(A);
}

/* 60 a0: 0110 0000 1010 0000 */
static void ADDNC_A_V(void)
{
	UINT8 tmp = A + V;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a1: 0110 0000 1010 0001 */
/* ADDNC_A_A already defined */

/* 60 a2: 0110 0000 1010 0010 */
static void ADDNC_A_B(void)
{
	UINT8 tmp = A + B;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a3: 0110 0000 1010 0011 */
static void ADDNC_A_C(void)
{
	UINT8 tmp = A + C;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a4: 0110 0000 1010 0100 */
static void ADDNC_A_D(void)
{
	UINT8 tmp = A + D;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a5: 0110 0000 1010 0101 */
static void ADDNC_A_E(void)
{
	UINT8 tmp = A + E;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a6: 0110 0000 1010 0110 */
static void ADDNC_A_H(void)
{
	UINT8 tmp = A + H;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a7: 0110 0000 1010 0111 */
static void ADDNC_A_L(void)
{
	UINT8 tmp = A + L;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a8: 0110 0000 1010 1000 */
static void GTA_A_V(void)
{
	UINT16 tmp = A - V - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 a9: 0110 0000 1010 1001 */
/* GTA_A_A already defined */

/* 60 aa: 0110 0000 1010 1010 */
static void GTA_A_B(void)
{
	UINT16 tmp = A - B - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ab: 0110 0000 1010 1011 */
static void GTA_A_C(void)
{
	UINT16 tmp = A - C - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ac: 0110 0000 1010 1100 */
static void GTA_A_D(void)
{
	UINT16 tmp = A - D - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ad: 0110 0000 1010 1101 */
static void GTA_A_E(void)
{
	UINT16 tmp = A - E - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ae: 0110 0000 1010 1110 */
static void GTA_A_H(void)
{
	UINT16 tmp = A - H - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 af: 0110 0000 1010 1111 */
static void GTA_A_L(void)
{
	UINT16 tmp = A - L - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 b0: 0110 0000 1011 0000 */
static void SUBNB_A_V(void)
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b1: 0110 0000 1011 0001 */
/* SUBNB_A_A already defined */

/* 60 b2: 0110 0000 1011 0010 */
static void SUBNB_A_B(void)
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b3: 0110 0000 1011 0011 */
static void SUBNB_A_C(void)
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b4: 0110 0000 1011 0100 */
static void SUBNB_A_D(void)
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b5: 0110 0000 1011 0101 */
static void SUBNB_A_E(void)
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b6: 0110 0000 1011 0110 */
static void SUBNB_A_H(void)
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b7: 0110 0000 1011 0111 */
static void SUBNB_A_L(void)
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b8: 0110 0000 1011 1000 */
static void LTA_A_V(void)
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 b9: 0110 0000 1011 1001 */
/* LTA_A_A already defined */

/* 60 ba: 0110 0000 1011 1010 */
static void LTA_A_B(void)
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bb: 0110 0000 1011 1011 */
static void LTA_A_C(void)
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bc: 0110 0000 1011 1100 */
static void LTA_A_D(void)
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bd: 0110 0000 1011 1101 */
static void LTA_A_E(void)
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 be: 0110 0000 1011 1110 */
static void LTA_A_H(void)
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bf: 0110 0000 1011 1111 */
static void LTA_A_L(void)
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 c0: 0110 0000 1100 0000 */
static void ADD_A_V(void)
{
	UINT8 tmp = A + V;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c1: 0110 0000 1100 0001 */
/* ADD_A_A already defined */

/* 60 c2: 0110 0000 1100 0010 */
static void ADD_A_B(void)
{
	UINT8 tmp = A + B;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c3: 0110 0000 1100 0011 */
static void ADD_A_C(void)
{
	UINT8 tmp = A + C;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c4: 0110 0000 1100 0100 */
static void ADD_A_D(void)
{
	UINT8 tmp = A + D;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c5: 0110 0000 1100 0101 */
static void ADD_A_E(void)
{
	UINT8 tmp = A + E;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c6: 0110 0000 1100 0110 */
static void ADD_A_H(void)
{
	UINT8 tmp = A + H;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c7: 0110 0000 1100 0111 */
static void ADD_A_L(void)
{
	UINT8 tmp = A + L;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c8: 0110 0000 1100 1000 */
static void ONA_A_V(void)
{
	if (A & V)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 c9: 0110 0000 1100 1001 */
static void ONA_A_A(void)
{
	if (A & A)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 ca: 0110 0000 1100 1010 */
static void ONA_A_B(void)
{
	if (A & B)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cb: 0110 0000 1100 1011 */
static void ONA_A_C(void)
{
	if (A & C)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cc: 0110 0000 1100 1100 */
static void ONA_A_D(void)
{
	if (A & D)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cd: 0110 0000 1100 1101 */
static void ONA_A_E(void)
{
	if (A & E)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 ce: 0110 0000 1100 1110 */
static void ONA_A_H(void)
{
	if (A & H)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cf: 0110 0000 1100 1111 */
static void ONA_A_L(void)
{
	if (A & L)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 d0: 0110 0000 1101 0000 */
static void ADC_A_V(void)
{
	UINT8 tmp = A + V + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d1: 0110 0000 1101 0001 */
/* ADC_A_A already defined */

/* 60 d2: 0110 0000 1101 0010 */
static void ADC_A_B(void)
{
	UINT8 tmp = A + B + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d3: 0110 0000 1101 0011 */
static void ADC_A_C(void)
{
	UINT8 tmp = A + C + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d4: 0110 0000 1101 0100 */
static void ADC_A_D(void)
{
	UINT8 tmp = A + D + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d5: 0110 0000 1101 0101 */
static void ADC_A_E(void)
{
	UINT8 tmp = A + E + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d6: 0110 0000 1101 0110 */
static void ADC_A_H(void)
{
	UINT8 tmp = A + H + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d7: 0110 0000 1101 0111 */
static void ADC_A_L(void)
{
	UINT8 tmp = A + L + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d8: 0110 0000 1101 1000 */
static void OFFA_A_V(void)
{
	if ( A & V )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 d9: 0110 0000 1101 1001 */
static void OFFA_A_A(void)
{
	if ( A & A )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 da: 0110 0000 1101 1010 */
static void OFFA_A_B(void)
{
	if ( A & B )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 db: 0110 0000 1101 1011 */
static void OFFA_A_C(void)
{
	if ( A & C )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 dc: 0110 0000 1101 1100 */
static void OFFA_A_D(void)
{
	if ( A & D )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 dd: 0110 0000 1101 1101 */
static void OFFA_A_E(void)
{
	if ( A & E )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 de: 0110 0000 1101 1110 */
static void OFFA_A_H(void)
{
	if ( A & H )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 df: 0110 0000 1101 1111 */
static void OFFA_A_L(void)
{
	if ( A & L )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 e0: 0110 0000 1110 0000 */
static void SUB_A_V(void)
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e1: 0110 0000 1110 0001 */
/* SUB_A_A already defined */

/* 60 e2: 0110 0000 1110 0010 */
static void SUB_A_B(void)
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e3: 0110 0000 1110 0011 */
static void SUB_A_C(void)
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e4: 0110 0000 1110 0100 */
static void SUB_A_D(void)
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e5: 0110 0000 1110 0101 */
static void SUB_A_E(void)
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e6: 0110 0000 1110 0110 */
static void SUB_A_H(void)
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e7: 0110 0000 1110 0111 */
static void SUB_A_L(void)
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e8: 0110 0000 1110 1000 */
static void NEA_A_V(void)
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 e9: 0110 0000 1110 1001 */
/* NEA_A_A already defined */

/* 60 ea: 0110 0000 1110 1010 */
static void NEA_A_B(void)
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 eb: 0110 0000 1110 1011 */
static void NEA_A_C(void)
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ec: 0110 0000 1110 1100 */
static void NEA_A_D(void)
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ed: 0110 0000 1110 1101 */
static void NEA_A_E(void)
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ee: 0110 0000 1110 1110 */
static void NEA_A_H(void)
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ef: 0110 0000 1110 1111 */
static void NEA_A_L(void)
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 f0: 0110 0000 1111 0000 */
static void SBB_A_V(void)
{
	UINT8 tmp = A - V - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f1: 0110 0000 1111 0001 */
/* SBB_A_A already defined */

/* 60 f2: 0110 0000 1111 0010 */
static void SBB_A_B(void)
{
	UINT8 tmp = A - B - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f3: 0110 0000 1111 0011 */
static void SBB_A_C(void)
{
	UINT8 tmp = A - C - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f4: 0110 0000 1111 0100 */
static void SBB_A_D(void)
{
	UINT8 tmp = A - D - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f5: 0110 0000 1111 0101 */
static void SBB_A_E(void)
{
	UINT8 tmp = A - E - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f6: 0110 0000 1111 0110 */
static void SBB_A_H(void)
{
	UINT8 tmp = A - H - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f7: 0110 0000 1111 0111 */
static void SBB_A_L(void)
{
	UINT8 tmp = A - L - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f8: 0110 0000 1111 1000 */
static void EQA_A_V(void)
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 f9: 0110 0000 1111 1001 */
/* EQA_A_A already defined */

/* 60 fa: 0110 0000 1111 1010 */
static void EQA_A_B(void)
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fb: 0110 0000 1111 1011 */
static void EQA_A_C(void)
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fc: 0110 0000 1111 1100 */
static void EQA_A_D(void)
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fd: 0110 0000 1111 1101 */
static void EQA_A_E(void)
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fe: 0110 0000 1111 1110 */
static void EQA_A_H(void)
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 ff: 0110 0000 1111 1111 */
static void EQA_A_L(void)
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* prefix 64 */
/* 64 00: 0110 0100 0000 0000 xxxx xxxx */
static void MVI_PA_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTA, imm );
}

/* 64 01: 0110 0100 0000 0001 xxxx xxxx */
static void MVI_PB_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTB, imm );
}

/* 64 02: 0110 0100 0000 0010 xxxx xxxx */
static void MVI_PC_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTC, imm );
}

/* 64 03: 0110 0100 0000 0011 xxxx xxxx */
static void MVI_PD_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTD, imm );
}

/* 64 05: 0110 0100 0000 0101 xxxx xxxx */
static void MVI_PF_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTF, imm );
}

/* 64 06: 0110 0100 0000 0110 xxxx xxxx */
static void MVI_MKH_xx(void)
{
	RDOPARG( MKH );
}

/* 64 07: 0110 0100 0000 0111 xxxx xxxx */
static void MVI_MKL_xx(void)
{
	RDOPARG( MKL );
}

/* 64 08: 0110 0100 0000 1000 xxxx xxxx */
static void ANI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA), imm;
	RDOPARG( imm );
	pa &= imm;
	WP( UPD7810_PORTA, pa );
	SET_Z(pa);
}

/* 64 09: 0110 0100 0000 1001 xxxx xxxx */
static void ANI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB), imm;
	RDOPARG( imm );
	pb &= imm;
	WP( UPD7810_PORTB, pb );
	SET_Z(pb);
}

/* 64 0a: 0110 0100 0000 1010 xxxx xxxx */
static void ANI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC), imm;
	RDOPARG( imm );
	pc &= imm;
	WP( UPD7810_PORTC, pc );
	SET_Z(pc);
}

/* 64 0b: 0110 0100 0000 1011 xxxx xxxx */
static void ANI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	RDOPARG( imm );
	pd &= imm;
	WP( UPD7810_PORTD, pd );
	SET_Z(pd);
}

/* 64 0d: 0110 0100 0000 1101 xxxx xxxx */
static void ANI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	RDOPARG( imm );
	pf &= imm;
	WP( UPD7810_PORTF, pf );
	SET_Z(pf);
}

/* 64 0e: 0110 0100 0000 1110 xxxx xxxx */
static void ANI_MKH_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	MKH &= imm;
	SET_Z(MKH);
}

/* 64 0f: 0110 0100 0000 1111 xxxx xxxx */
static void ANI_MKL_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	MKL &= imm;
	SET_Z(MKL);
}

/* 64 10: 0110 0100 0001 0000 xxxx xxxx */
static void XRI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;
	RDOPARG( imm );
	pa ^= imm;
	WP( UPD7810_PORTA, pa );
	SET_Z(pa);
}

/* 64 11: 0110 0100 0001 0001 xxxx xxxx */
static void XRI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;
	RDOPARG( imm );
	pb ^= imm;
	WP( UPD7810_PORTB, pb );
	SET_Z(pb);
}

/* 64 12: 0110 0100 0001 0010 xxxx xxxx */
static void XRI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;
	RDOPARG( imm );
	pc ^= imm;
	WP( UPD7810_PORTC, pc );
	SET_Z(pc);
}

/* 64 13: 0110 0100 0001 0011 xxxx xxxx */
static void XRI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	RDOPARG( imm );
	pd ^= imm;
	WP( UPD7810_PORTD, pd );
	SET_Z(pd);
}

/* 64 15: 0110 0100 0001 0101 xxxx xxxx */
static void XRI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	RDOPARG( imm );
	pf ^= imm;
	WP( UPD7810_PORTF, pf );
	SET_Z(pf);
}

/* 64 16: 0110 0100 0001 0110 xxxx xxxx */
static void XRI_MKH_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	MKH ^= imm;
	SET_Z(MKH);
}

/* 64 17: 0110 0100 0001 0111 xxxx xxxx */
static void XRI_MKL_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	MKL ^= imm;
	SET_Z(MKL);
}

/* 64 18: 0110 0100 0001 1000 xxxx xxxx */
static void ORI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;
	RDOPARG( imm );
	pa |= imm;
	WP( UPD7810_PORTA, pa );
	SET_Z(pa);
}

/* 64 19: 0110 0100 0001 1001 xxxx xxxx */
static void ORI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;
	RDOPARG( imm );
	pb |= imm;
	WP( UPD7810_PORTB, pb );
	SET_Z(pb);
}

/* 64 1a: 0110 0100 0001 1010 xxxx xxxx */
static void ORI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;
	RDOPARG( imm );
	pc |= imm;
	WP( UPD7810_PORTC, pc );
	SET_Z(pc);
}

/* 64 1b: 0110 0100 0001 1011 xxxx xxxx */
static void ORI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	RDOPARG( imm );
	pd |= imm;
	WP( UPD7810_PORTD, pd );
	SET_Z(pd);
}

/* 64 1d: 0110 0100 0001 1101 xxxx xxxx */
static void ORI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	RDOPARG( imm );
	pf |= imm;
	WP( UPD7810_PORTF, pf );
	SET_Z(pf);
}

/* 64 1e: 0110 0100 0001 1110 xxxx xxxx */
static void ORI_MKH_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	MKH |= imm;
	SET_Z(MKH);
}

/* 64 1f: 0110 0100 0001 1111 xxxx xxxx */
static void ORI_MKL_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	MKL |= imm;
	SET_Z(MKL);
}

/* 64 20: 0110 0100 0010 0000 xxxx xxxx */
static void ADINC_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA );
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = pa + imm;

	ZHC_ADD( tmp, pa, 0 );
	WP( UPD7810_PORTA , tmp );
	SKIP_NC;
}

/* 64 21: 0110 0100 0010 0001 xxxx xxxx */
static void ADINC_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB );
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = pb + imm;

	ZHC_ADD( tmp, pb, 0 );
	WP( UPD7810_PORTB, tmp );
	SKIP_NC;
}

/* 64 22: 0110 0100 0010 0010 xxxx xxxx */
static void ADINC_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC );
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = pc + imm;

	ZHC_ADD( tmp, pc, 0 );
	WP( UPD7810_PORTC, tmp );
	SKIP_NC;
}

/* 64 23: 0110 0100 0010 0011 xxxx xxxx */
static void ADINC_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD );
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = pd + imm;

	ZHC_ADD( tmp, pd, 0 );
	WP( UPD7810_PORTD, tmp );
	SKIP_NC;
}

/* 64 25: 0110 0100 0010 0101 xxxx xxxx */
static void ADINC_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF );
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = pf + imm;

	ZHC_ADD( tmp, pf, 0 );
	WP( UPD7810_PORTF, tmp );
	SKIP_NC;
}

/* 64 26: 0110 0100 0010 0110 xxxx xxxx */
static void ADINC_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH + imm;

	ZHC_ADD( tmp, MKH, 0 );
	MKH = tmp;
	SKIP_NC;
}

/* 64 27: 0110 0100 0010 0111 xxxx xxxx */
static void ADINC_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL + imm;

	ZHC_ADD( tmp, MKL, 0 );
	MKL = tmp;
	SKIP_NC;
}

/* 64 28: 0110 0100 0010 1000 xxxx xxxx */
static void GTI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pa - imm - 1;
	ZHC_SUB( tmp, pa, 0 );

	SKIP_NC;
}

/* 64 29: 0110 0100 0010 1001 xxxx xxxx */
static void GTI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pb - imm - 1;
	ZHC_SUB( tmp, pb, 0 );

	SKIP_NC;
}

/* 64 2a: 0110 0100 0010 1010 xxxx xxxx */
static void GTI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pc - imm - 1;
	ZHC_SUB( tmp, pc, 0 );

	SKIP_NC;
}

/* 64 2b: 0110 0100 0010 1011 xxxx xxxx */
static void GTI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pd - imm - 1;
	ZHC_SUB( tmp, pd, 0 );

	SKIP_NC;
}

/* 64 2d: 0110 0100 0010 1101 xxxx xxxx */
static void GTI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pf - imm - 1;
	ZHC_SUB( tmp, pf, 0 );

	SKIP_NC;
}

/* 64 2e: 0110 0100 0010 1110 xxxx xxxx */
static void GTI_MKH_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = MKH - imm - 1;
	ZHC_SUB( tmp, MKH, 0 );

	SKIP_NC;
}

/* 64 2f: 0110 0100 0010 1111 xxxx xxxx */
static void GTI_MKL_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = MKL - imm - 1;
	ZHC_SUB( tmp, MKL, 0 );

	SKIP_NC;
}

/* 64 30: 0110 0100 0011 0000 xxxx xxxx */
static void SUINB_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
	SKIP_NC;
}

/* 64 31: 0110 0100 0011 0001 xxxx xxxx */
static void SUINB_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
	SKIP_NC;
}

/* 64 32: 0110 0100 0011 0010 xxxx xxxx */
static void SUINB_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
	SKIP_NC;
}

/* 64 33: 0110 0100 0011 0011 xxxx xxxx */
static void SUINB_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
	SKIP_NC;
}

/* 64 35: 0110 0100 0011 0101 xxxx xxxx */
static void SUINB_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
	SKIP_NC;
}

/* 64 36: 0110 0100 0011 0110 xxxx xxxx */
static void SUINB_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	MKH = tmp;
	SKIP_NC;
}

/* 64 37: 0110 0100 0011 0111 xxxx xxxx */
static void SUINB_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	MKL = tmp;
	SKIP_NC;
}

/* 64 38: 0110 0100 0011 1000 xxxx xxxx */
static void LTI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;
	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	SKIP_CY;
}

/* 64 39: 0110 0100 0011 1001 xxxx xxxx */
static void LTI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;
	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	SKIP_CY;
}

/* 64 3a: 0110 0100 0011 1010 xxxx xxxx */
static void LTI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;
	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	SKIP_CY;
}

/* 64 3b: 0110 0100 0011 1011 xxxx xxxx */
static void LTI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;
	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	SKIP_CY;
}

/* 64 3d: 0110 0100 0011 1101 xxxx xxxx */
static void LTI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;
	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	SKIP_CY;
}

/* 64 3e: 0110 0100 0011 1110 xxxx xxxx */
static void LTI_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	SKIP_CY;
}

/* 64 3f: 0110 0100 0011 1111 xxxx xxxx */
static void LTI_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	SKIP_CY;
}

/* 64 40: 0110 0100 0100 0000 xxxx xxxx */
static void ADI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa + imm;
	ZHC_ADD( tmp, pa, 0 );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 41: 0110 0100 0100 0001 xxxx xxxx */
static void ADI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb + imm;
	ZHC_ADD( tmp, pb, 0 );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 42: 0110 0100 0100 0010 xxxx xxxx */
static void ADI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc + imm;
	ZHC_ADD( tmp, pc, 0 );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 43: 0110 0100 0100 0011 xxxx xxxx */
static void ADI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd + imm;
	ZHC_ADD( tmp, pd, 0 );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 45: 0110 0100 0100 0101 xxxx xxxx */
static void ADI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf + imm;
	ZHC_ADD( tmp, pf, 0 );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 46: 0110 0100 0100 0110 xxxx xxxx */
static void ADI_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH + imm;
	ZHC_ADD( tmp, MKH, 0 );
	MKH = tmp;
}

/* 64 47: 0110 0100 0100 0111 xxxx xxxx */
static void ADI_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL + imm;
	ZHC_ADD( tmp, MKL, 0 );
	MKL = tmp;
}

/* 64 48: 0110 0100 0100 1000 xxxx xxxx */
static void ONI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;

	RDOPARG( imm );
	if (pa & imm)
		PSW |= SK;
}

/* 64 49: 0110 0100 0100 1001 xxxx xxxx */
static void ONI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;

	RDOPARG( imm );
	if (pb & imm)
		PSW |= SK;
}

/* 64 4a: 0110 0100 0100 1010 xxxx xxxx */
static void ONI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;

	RDOPARG( imm );
	if (pc & imm)
		PSW |= SK;
}

/* 64 4b: 0110 0100 0100 1011 xxxx xxxx */
static void ONI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;

	RDOPARG( imm );
	if (pd & imm)
		PSW |= SK;
}

/* 64 4d: 0110 0100 0100 1101 xxxx xxxx */
static void ONI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;

	RDOPARG( imm );
	if (pf & imm)
		PSW |= SK;
}

/* 64 4e: 0110 0100 0100 1110 xxxx xxxx */
static void ONI_MKH_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (MKH & imm)
		PSW |= SK;
}

/* 64 4f: 0110 0100 0100 1111 xxxx xxxx */
static void ONI_MKL_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (MKL & imm)
		PSW |= SK;
}

/* 64 50: 0110 0100 0101 0000 xxxx xxxx */
static void ACI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa + imm + (PSW & CY);
	ZHC_ADD( tmp, pa, (PSW & CY) );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 51: 0110 0100 0101 0001 xxxx xxxx */
static void ACI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb + imm + (PSW & CY);
	ZHC_ADD( tmp, pb, (PSW & CY) );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 52: 0110 0100 0101 0010 xxxx xxxx */
static void ACI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc + imm + (PSW & CY);
	ZHC_ADD( tmp, pc, (PSW & CY) );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 53: 0110 0100 0101 0011 xxxx xxxx */
static void ACI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd + imm + (PSW & CY);
	ZHC_ADD( tmp, pd, (PSW & CY) );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 55: 0110 0100 0101 0101 xxxx xxxx */
static void ACI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf + imm + (PSW & CY);
	ZHC_ADD( tmp, pf, (PSW & CY) );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 56: 0110 0100 0101 0110 xxxx xxxx */
static void ACI_MKH_xx(void)
{
	UINT8 imm, tmp;

	RDOPARG( imm );
	tmp = MKH + imm + (PSW & CY);
	ZHC_ADD( tmp, MKH, (PSW & CY) );
	MKH = tmp;
}

/* 64 57: 0110 0100 0101 0111 xxxx xxxx */
static void ACI_MKL_xx(void)
{
	UINT8 imm, tmp;

	RDOPARG( imm );
	tmp = MKL + imm + (PSW & CY);
	ZHC_ADD( tmp, MKL, (PSW & CY) );
	MKL = tmp;
}

/* 64 58: 0110 0100 0101 1000 xxxx xxxx */
static void OFFI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;

	RDOPARG( imm );
	if (0 == (pa & imm))
		PSW |= SK;
}

/* 64 59: 0110 0100 0101 1001 xxxx xxxx */
static void OFFI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;

	RDOPARG( imm );
	if (0 == (pb & imm))
		PSW |= SK;
}

/* 64 5a: 0110 0100 0101 1010 xxxx xxxx */
static void OFFI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;

	RDOPARG( imm );
	if (0 == (pc & imm))
		PSW |= SK;
}

/* 64 5b: 0110 0100 0101 1011 xxxx xxxx */
static void OFFI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;

	RDOPARG( imm );
	if (0 == (pd & imm))
		PSW |= SK;
}

/* 64 5d: 0110 0100 0101 1101 xxxx xxxx */
static void OFFI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;

	RDOPARG( imm );
	if (0 == (pf & imm))
		PSW |= SK;
}

/* 64 5e: 0110 0100 0101 1110 xxxx xxxx */
static void OFFI_MKH_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (MKH & imm))
		PSW |= SK;
}

/* 64 5f: 0110 0100 0101 1111 xxxx xxxx */
static void OFFI_MKL_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (MKL & imm))
		PSW |= SK;
}

/* 64 60: 0110 0100 0110 0000 xxxx xxxx */
static void SUI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 61: 0110 0100 0110 0001 xxxx xxxx */
static void SUI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 62: 0110 0100 0110 0010 xxxx xxxx */
static void SUI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 63: 0110 0100 0110 0011 xxxx xxxx */
static void SUI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 65: 0110 0100 0110 0101 xxxx xxxx */
static void SUI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 66: 0110 0100 0110 0110 xxxx xxxx */
static void SUI_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	MKH = tmp;
}

/* 64 67: 0110 0100 0110 0111 xxxx xxxx */
static void SUI_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	MKL = tmp;
}

/* 64 68: 0110 0100 0110 1000 xxxx xxxx */
static void NEI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	SKIP_NZ;
}

/* 64 69: 0110 0100 0110 1001 xxxx xxxx */
static void NEI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	SKIP_NZ;
}

/* 64 6a: 0110 0100 0110 1010 xxxx xxxx */
static void NEI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	SKIP_NZ;
}

/* 64 6b: 0110 0100 0110 1011 xxxx xxxx */
static void NEI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	SKIP_NZ;
}

/* 64 6d: 0110 0100 0110 1101 xxxx xxxx */
static void NEI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	SKIP_NZ;
}

/* 64 6e: 0110 0100 0110 1110 xxxx xxxx */
static void NEI_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	SKIP_NZ;
}

/* 64 6f: 0110 0100 0110 1111 xxxx xxxx */
static void NEI_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	SKIP_NZ;
}

/* 64 70: 0110 0100 0111 0000 xxxx xxxx */
static void SBI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm - (PSW & CY);
	ZHC_SUB( tmp, pa, (PSW & CY) );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 71: 0110 0100 0111 0001 xxxx xxxx */
static void SBI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm - (PSW & CY);
	ZHC_SUB( tmp, pb, (PSW & CY) );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 72: 0110 0100 0111 0010 xxxx xxxx */
static void SBI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm - (PSW & CY);
	ZHC_SUB( tmp, pc, (PSW & CY) );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 73: 0110 0100 0111 0011 xxxx xxxx */
static void SBI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm - (PSW & CY);
	ZHC_SUB( tmp, pd, (PSW & CY) );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 75: 0110 0100 0111 0101 xxxx xxxx */
static void SBI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm - (PSW & CY);
	ZHC_SUB( tmp, pf, (PSW & CY) );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 76: 0110 0100 0111 0110 xxxx xxxx */
static void SBI_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm - (PSW & CY);
	ZHC_SUB( tmp, MKH, (PSW & CY) );
	MKH = tmp;
}

/* 64 77: 0110 0100 0111 0111 xxxx xxxx */
static void SBI_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm - (PSW & CY);
	ZHC_SUB( tmp, MKL, (PSW & CY) );
	MKL = tmp;
}

/* 64 78: 0110 0100 0111 1000 xxxx xxxx */
static void EQI_PA_xx(void)
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	SKIP_Z;
}

/* 64 79: 0110 0100 0111 1001 xxxx xxxx */
static void EQI_PB_xx(void)
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	SKIP_Z;
}

/* 64 7a: 0110 0100 0111 1010 xxxx xxxx */
static void EQI_PC_xx(void)
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	SKIP_Z;
}

/* 64 7b: 0110 0100 0111 1011 xxxx xxxx */
static void EQI_PD_xx(void)
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	SKIP_Z;
}

/* 64 7d: 0110 0100 0111 1101 xxxx xxxx */
static void EQI_PF_xx(void)
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	SKIP_Z;
}

/* 64 7e: 0110 0100 0111 1110 xxxx xxxx */
static void EQI_MKH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	SKIP_Z;
}

/* 64 7f: 0110 0100 0111 1111 xxxx xxxx */
static void EQI_MKL_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	SKIP_Z;
}

/* 64 80: 0110 0100 1000 0000 xxxx xxxx */
static void MVI_ANM_xx(void)
{
	RDOPARG( ANM );
}

/* 64 81: 0110 0100 1000 0001 xxxx xxxx */
static void MVI_SMH_xx(void)
{
	RDOPARG( SMH );
}

/* 64 83: 0110 0100 1000 0011 xxxx xxxx */
static void MVI_EOM_xx(void)
{
	RDOPARG( EOM );
	upd7810_write_EOM();
}

/* 64 85: 0110 0100 1000 0101 xxxx xxxx */
static void MVI_TMM_xx(void)
{
	RDOPARG( TMM );
}

/* 64 88: 0110 0100 1000 1000 xxxx xxxx */
static void ANI_ANM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	ANM &= imm;
	SET_Z(ANM);
}

/* 64 89: 0110 0100 1000 1001 xxxx xxxx */
static void ANI_SMH_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	SMH &= imm;
	SET_Z(SMH);
}

/* 64 8b: 0110 0100 1000 1011 xxxx xxxx */
static void ANI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;

	RDOPARG( imm );
	/* only bits #1 and #5 can be read */
	EOM = eom & imm;
	SET_Z(EOM);
	upd7810_write_EOM();
}

/* 64 8d: 0110 0100 1000 1101 xxxx xxxx */
static void ANI_TMM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	TMM &= imm;
	SET_Z(TMM);
}

/* 64 90: 0110 0100 1001 0000 xxxx xxxx */
static void XRI_ANM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	ANM ^= imm;
	SET_Z(ANM);
}

/* 64 91: 0110 0100 1001 0001 xxxx xxxx */
static void XRI_SMH_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	SMH ^= imm;
	SET_Z(SMH);
}

/* 64 93: 0110 0100 1001 0011 xxxx xxxx */
static void XRI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;

	RDOPARG( imm );
	/* only bits #1 and #5 can be read */
	EOM = eom ^ imm;
	SET_Z(EOM);
	upd7810_write_EOM();
}

/* 64 95: 0110 0100 1001 0101 xxxx xxxx */
static void XRI_TMM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	TMM ^= imm;
	SET_Z(TMM);
}

/* 64 98: 0110 0100 1001 1000 xxxx xxxx */
static void ORI_ANM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	ANM |= imm;
	SET_Z(ANM);
}

/* 64 99: 0110 0100 1001 1001 xxxx xxxx */
static void ORI_SMH_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	SMH |= imm;
	SET_Z(SMH);
}

/* 64 9b: 0110 0100 1001 1011 xxxx xxxx */
static void ORI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;

	RDOPARG( imm );
	/* only bits #1 and #5 can be read */
	EOM = eom | imm;
	SET_Z(EOM);
	upd7810_write_EOM();
}

/* 64 9d: 0110 0100 1001 1101 xxxx xxxx */
static void ORI_TMM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	TMM |= imm;
	SET_Z(TMM);
}

/* 64 a0: 0110 0100 1010 0000 xxxx xxxx */
static void ADINC_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM + imm;

	ZHC_ADD( tmp, ANM, 0 );
	ANM = tmp;
	SKIP_NC;
}

/* 64 a1: 0110 0100 1010 0001 xxxx xxxx */
static void ADINC_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH + imm;

	ZHC_ADD( tmp, SMH, 0 );
	SMH = tmp;
	SKIP_NC;
}

/* 64 a3: 0110 0100 1010 0011 xxxx xxxx */
static void ADINC_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	/* only bits #1 and #5 can be read */
	tmp = eom + imm;

	ZHC_ADD( tmp, eom, 0 );
	EOM = tmp;
	SKIP_NC;
	upd7810_write_EOM();
}

/* 64 a5: 0110 0100 1010 0101 xxxx xxxx */
static void ADINC_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM + imm;

	ZHC_ADD( tmp, TMM, 0 );
	TMM = tmp;
	SKIP_NC;
}

/* 64 a8: 0110 0100 1010 1000 xxxx xxxx */
static void GTI_ANM_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = ANM - imm - 1;
	ZHC_SUB( tmp, ANM, 0 );

	SKIP_NC;
}

/* 64 a9: 0110 0100 1010 1001 xxxx xxxx */
static void GTI_SMH_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = SMH - imm - 1;
	ZHC_SUB( tmp, SMH, 0 );

	SKIP_NC;
}

/* 64 ab: 0110 0100 1010 1011 xxxx xxxx */
static void GTI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = eom - imm - 1;
	ZHC_SUB( tmp, eom, 0 );

	SKIP_NC;
}

/* 64 ad: 0110 0100 1010 1101 xxxx xxxx */
static void GTI_TMM_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = TMM - imm - 1;
	ZHC_SUB( tmp, TMM, 0 );

	SKIP_NC;
}

/* 64 b0: 0110 0100 1011 0000 xxxx xxxx */
static void SUINB_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	ANM = tmp;
	SKIP_NC;
}

/* 64 b1: 0110 0100 1011 0001 xxxx xxxx */
static void SUINB_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SMH = tmp;
	SKIP_NC;
}

/* 64 b3: 0110 0100 1011 0011 xxxx xxxx */
static void SUINB_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom - imm;
	ZHC_SUB( tmp, eom, 0 );
	EOM = tmp;
	SKIP_NC;
	upd7810_write_EOM();
}

/* 64 b5: 0110 0100 1011 0101 xxxx xxxx */
static void SUINB_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	TMM = tmp;
	SKIP_NC;
}

/* 64 b8: 0110 0100 1011 1000 xxxx xxxx */
static void LTI_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	SKIP_CY;
}

/* 64 b9: 0110 0100 1011 1001 xxxx xxxx */
static void LTI_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SKIP_CY;
}

/* 64 bb: 0110 0100 1011 1011 xxxx xxxx */
static void LTI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom - imm;
	ZHC_SUB( tmp, eom, 0 );
	SKIP_CY;
}

/* 64 bd: 0110 0100 1011 1101 xxxx xxxx */
static void LTI_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	SKIP_CY;
}

/* 64 c0: 0110 0100 1100 0000 xxxx xxxx */
static void ADI_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM + imm;

	ZHC_ADD( tmp, ANM, 0 );
	ANM = tmp;
}

/* 64 c1: 0110 0100 1100 0001 xxxx xxxx */
static void ADI_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH + imm;

	ZHC_ADD( tmp, SMH, 0 );
	SMH = tmp;
}

/* 64 c3: 0110 0100 1100 0011 xxxx xxxx */
static void ADI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom + imm;

	ZHC_ADD( tmp, eom, 0 );
	EOM = tmp;
	upd7810_write_EOM();
}

/* 64 c5: 0110 0100 1100 0101 xxxx xxxx */
static void ADI_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM + imm;

	ZHC_ADD( tmp, TMM, 0 );
	TMM = tmp;
}

/* 64 c8: 0110 0100 1100 1000 xxxx xxxx */
static void ONI_ANM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (ANM & imm)
		PSW |= SK;
}

/* 64 c9: 0110 0100 1100 1001 xxxx xxxx */
static void ONI_SMH_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (SMH & imm)
		PSW |= SK;
}

/* 64 cb: 0110 0100 1100 1011 xxxx xxxx */
static void ONI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;

	RDOPARG( imm );
	if (eom & imm)
		PSW |= SK;
}

/* 64 cd: 0110 0100 1100 1101 xxxx xxxx */
static void ONI_TMM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (TMM & imm)
		PSW |= SK;
}

/* 64 d0: 0110 0100 1101 0000 xxxx xxxx */
static void ACI_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM + imm + (PSW & CY);

	ZHC_ADD( tmp, ANM, (PSW & CY) );
	ANM = tmp;
}

/* 64 d1: 0110 0100 1101 0001 xxxx xxxx */
static void ACI_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH + imm + (PSW & CY);

	ZHC_ADD( tmp, SMH, (PSW & CY) );
	SMH = tmp;
}

/* 64 d3: 0110 0100 1101 0011 xxxx xxxx */
static void ACI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom + imm + (PSW & CY);

	ZHC_ADD( tmp, eom, (PSW & CY) );
	EOM = tmp;
	upd7810_write_EOM();
}

/* 64 d5: 0110 0100 1101 0101 xxxx xxxx */
static void ACI_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM + imm + (PSW & CY);

	ZHC_ADD( tmp, TMM, (PSW & CY) );
	TMM = tmp;
}

/* 64 d8: 0110 0100 1101 1000 xxxx xxxx */
static void OFFI_ANM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (ANM & imm))
		PSW |= SK;
}

/* 64 d9: 0110 0100 1101 1001 xxxx xxxx */
static void OFFI_SMH_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (SMH & imm))
		PSW |= SK;
}

/* 64 db: 0110 0100 1101 1011 xxxx xxxx */
static void OFFI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (eom & imm))
		PSW |= SK;
}

/* 64 dd: 0110 0100 1101 1101 xxxx xxxx */
static void OFFI_TMM_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (TMM & imm))
		PSW |= SK;
}

/* 64 e0: 0110 0100 1110 0000 xxxx xxxx */
static void SUI_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	ANM = tmp;
}

/* 64 e1: 0110 0100 1110 0001 xxxx xxxx */
static void SUI_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SMH = tmp;
}

/* 64 e3: 0110 0100 1110 0011 xxxx xxxx */
static void SUI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom - imm;
	ZHC_SUB( tmp, eom, 0 );
	EOM = tmp;
	upd7810_write_EOM();
}

/* 64 e5: 0110 0100 1110 0101 xxxx xxxx */
static void SUI_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	TMM = tmp;
}

/* 64 e8: 0110 0100 1110 1000 xxxx xxxx */
static void NEI_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	SKIP_NZ;
}

/* 64 e9: 0110 0100 1110 1001 xxxx xxxx */
static void NEI_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SKIP_NZ;
}

/* 64 eb: 0110 0100 1110 1011 xxxx xxxx */
static void NEI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom - imm;
	ZHC_SUB( tmp, eom, 0 );
	SKIP_NZ;
}

/* 64 ed: 0110 0100 1110 1101 xxxx xxxx */
static void NEI_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	SKIP_NZ;
}

/* 64 f0: 0110 0100 1111 0000 xxxx xxxx */
static void SBI_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm - (PSW & CY);
	ZHC_SUB( tmp, ANM, (PSW & CY) );
	ANM = tmp;
}

/* 64 f1: 0110 0100 1111 0001 xxxx xxxx */
static void SBI_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm - (PSW & CY);
	ZHC_SUB( tmp, SMH, (PSW & CY) );
	SMH = tmp;
}

/* 64 f3: 0110 0100 1111 0011 xxxx xxxx */
static void SBI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom - imm - (PSW & CY);
	ZHC_SUB( tmp, eom, (PSW & CY) );
	EOM = tmp;
	upd7810_write_EOM();
}

/* 64 f5: 0110 0100 1111 0101 xxxx xxxx */
static void SBI_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm - (PSW & CY);
	ZHC_SUB( tmp, TMM, (PSW & CY) );
	TMM = tmp;
}

/* 64 f8: 0110 0100 1111 1000 xxxx xxxx */
static void EQI_ANM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	SKIP_Z;
}

/* 64 f9: 0110 0100 1111 1001 xxxx xxxx */
static void EQI_SMH_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SKIP_Z;
}

/* 64 fb: 0110 0100 1111 1011 xxxx xxxx */
static void EQI_EOM_xx(void)
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = eom - imm;
	ZHC_SUB( tmp, eom, 0 );
	SKIP_Z;
}

/* 64 fd: 0110 0100 1111 1101 xxxx xxxx */
static void EQI_TMM_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	SKIP_Z;
}

/* prefix 70 */
/* 70 0e: 0111 0000 0000 1110 llll llll hhhh hhhh */
static void SSPD_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, SPL );
	WM( ea.d + 1, SPH );
}

/* 70 0f: 0111 0000 0000 1111 llll llll hhhh hhhh */
static void LSPD_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	SPL = RM( ea.d );
	SPH = RM( ea.d + 1 );
}

/* 70 1e: 0111 0000 0001 1110 llll llll hhhh hhhh */
static void SBCD_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, C );
	WM( ea.d + 1, B );
}

/* 70 1f: 0111 0000 0001 1111 llll llll hhhh hhhh */
static void LBCD_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	C = RM( ea.d );
	B = RM( ea.d + 1 );
}

/* 70 2e: 0111 0000 0010 1110 llll llll hhhh hhhh */
static void SDED_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, E );
	WM( ea.d + 1, D );
}

/* 70 2f: 0111 0000 0010 1111 llll llll hhhh hhhh */
static void LDED_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	E = RM( ea.d );
	D = RM( ea.d + 1 );
}

/* 70 3e: 0111 0000 0011 1110 llll llll hhhh hhhh */
static void SHLD_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, L );
	WM( ea.d + 1, H );
}

/* 70 3f: 0111 0000 0011 1111 llll llll hhhh hhhh */
static void LHLD_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	L = RM( ea.d );
	H = RM( ea.d + 1 );
}

/* 70 41: 0111 0000 0100 0001 */
static void EADD_EA_A(void)
{
	UINT16 tmp;
	tmp = EA + A;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 70 42: 0111 0000 0100 0010 */
static void EADD_EA_B(void)
{
	UINT16 tmp;
	tmp = EA + B;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 70 43: 0111 0000 0100 0011 */
static void EADD_EA_C(void)
{
	UINT16 tmp;
	tmp = EA + C;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 70 61: 0111 0000 0110 0001 */
static void ESUB_EA_A(void)
{
	UINT16 tmp;
	tmp = EA - A;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 70 62: 0111 0000 0110 0010 */
static void ESUB_EA_B(void)
{
	UINT16 tmp;
	tmp = EA - B;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 70 63: 0111 0000 0110 0011 */
static void ESUB_EA_C(void)
{
	UINT16 tmp;
	tmp = EA - C;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 70 68: 0111 0000 0110 1000 llll llll hhhh hhhh */
static void MOV_V_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	V = RM( ea.d );
}

/* 70 69: 0111 0000 0110 1001 llll llll hhhh hhhh */
static void MOV_A_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	A = RM( ea.d );
}

/* 70 6a: 0111 0000 0110 1010 llll llll hhhh hhhh */
static void MOV_B_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	B = RM( ea.d );
}

/* 70 6b: 0111 0000 0110 1011 llll llll hhhh hhhh */
static void MOV_C_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	C = RM( ea.d );
}

/* 70 6c: 0111 0000 0110 1100 llll llll hhhh hhhh */
static void MOV_D_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	D = RM( ea.d );
}

/* 70 6d: 0111 0000 0110 1101 llll llll hhhh hhhh */
static void MOV_E_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	E = RM( ea.d );
}

/* 70 6e: 0111 0000 0110 1110 llll llll hhhh hhhh */
static void MOV_H_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	H = RM( ea.d );
}

/* 70 6f: 0111 0000 0110 1111 llll llll hhhh hhhh */
static void MOV_L_w(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	L = RM( ea.d );
}

/* 70 78: 0111 0000 0111 1000 llll llll hhhh hhhh */
static void MOV_w_V(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, V );
}

/* 70 79: 0111 0000 0111 1001 llll llll hhhh hhhh */
static void MOV_w_A(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, A );
}

/* 70 7a: 0111 0000 0111 1010 llll llll hhhh hhhh */
static void MOV_w_B(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, B );
}

/* 70 7b: 0111 0000 0111 1011 llll llll hhhh hhhh */
static void MOV_w_C(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, C );
}

/* 70 7c: 0111 0000 0111 1100 llll llll hhhh hhhh */
static void MOV_w_D(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, D );
}

/* 70 7d: 0111 0000 0111 1101 llll llll hhhh hhhh */
static void MOV_w_E(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, E );
}

/* 70 7e: 0111 0000 0111 1110 llll llll hhhh hhhh */
static void MOV_w_H(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, H );
}

/* 70 7f: 0111 0000 0111 1111 llll llll hhhh hhhh */
static void MOV_w_L(void)
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, L );
}

/* 70 89: 0111 0000 1000 1001 */
static void ANAX_B(void)
{
	A &= RM( BC );
	SET_Z(A);
}

/* 70 8a: 0111 0000 1000 1010 */
static void ANAX_D(void)
{
	A &= RM( DE );
	SET_Z(A);
}

/* 70 8b: 0111 0000 1000 1011 */
static void ANAX_H(void)
{
	A &= RM( HL );
	SET_Z(A);
}

/* 70 8c: 0111 0000 1000 1100 */
static void ANAX_Dp(void)
{
	A &= RM( DE );
	DE++;
	SET_Z(A);
}

/* 70 8d: 0111 0000 1000 1101 */
static void ANAX_Hp(void)
{
	A &= RM( HL );
	HL++;
	SET_Z(A);
}

/* 70 8e: 0111 0000 1000 1110 */
static void ANAX_Dm(void)
{
	A &= RM( DE );
	DE--;
	SET_Z(A);
}

/* 70 8f: 0111 0000 1000 1111 */
static void ANAX_Hm(void)
{
	A &= RM( HL );
	HL--;
	SET_Z(A);
}

/* 70 91: 0111 0000 1001 0001 */
static void XRAX_B(void)
{
	A ^= RM( BC );
	SET_Z(A);
}

/* 70 92: 0111 0000 1001 0010 */
static void XRAX_D(void)
{
	A ^= RM( DE );
	SET_Z(A);
}

/* 70 93: 0111 0000 1001 0011 */
static void XRAX_H(void)
{
	A ^= RM( HL );
	SET_Z(A);
}

/* 70 94: 0111 0000 1001 0100 */
static void XRAX_Dp(void)
{
	A ^= RM( DE );
	DE++;
	SET_Z(A);
}

/* 70 95: 0111 0000 1001 0101 */
static void XRAX_Hp(void)
{
	A ^= RM( HL );
	HL++;
	SET_Z(A);
}

/* 70 96: 0111 0000 1001 0110 */
static void XRAX_Dm(void)
{
	A ^= RM( DE );
	DE--;
	SET_Z(A);
}

/* 70 97: 0111 0000 1001 0111 */
static void XRAX_Hm(void)
{
	A ^= RM( HL );
	HL--;
	SET_Z(A);
}

/* 70 99: 0111 0000 1001 1001 */
static void ORAX_B(void)
{
	A |= RM( BC );
	SET_Z(A);
}

/* 70 9a: 0111 0000 1001 1010 */
static void ORAX_D(void)
{
	A |= RM( DE );
	SET_Z(A);
}

/* 70 9b: 0111 0000 1001 1011 */
static void ORAX_H(void)
{
	A |= RM( HL );
	SET_Z(A);
}

/* 70 9c: 0111 0000 1001 0100 */
static void ORAX_Dp(void)
{
	A |= RM( DE );
	DE++;
	SET_Z(A);
}

/* 70 9d: 0111 0000 1001 1101 */
static void ORAX_Hp(void)
{
	A |= RM( HL );
	HL++;
	SET_Z(A);
}

/* 70 9e: 0111 0000 1001 1110 */
static void ORAX_Dm(void)
{
	A |= RM( DE );
	DE--;
	SET_Z(A);
}

/* 70 9f: 0111 0000 1001 1111 */
static void ORAX_Hm(void)
{
	A |= RM( HL );
	HL--;
	SET_Z(A);
}

/* 70 a1: 0111 0000 1010 0001 */
static void ADDNCX_B(void)
{
	UINT8 tmp = A + RM( BC );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a2: 0111 0000 1010 0010 */
static void ADDNCX_D(void)
{
	UINT8 tmp = A + RM( DE );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a3: 0111 0000 1010 0011 */
static void ADDNCX_H(void)
{
	UINT8 tmp = A + RM( HL );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a4: 0111 0000 1010 0100 */
static void ADDNCX_Dp(void)
{
	UINT8 tmp = A + RM( DE );
	DE++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a5: 0111 0000 1010 0101 */
static void ADDNCX_Hp(void)
{
	UINT8 tmp = A + RM( HL );
	HL++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a6: 0111 0000 1010 0110 */
static void ADDNCX_Dm(void)
{
	UINT8 tmp = A + RM( DE );
	DE--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a7: 0111 0000 1010 0111 */
static void ADDNCX_Hm(void)
{
	UINT8 tmp = A + RM( HL );
	HL--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a9: 0111 0000 1010 1001 */
static void GTAX_B(void)
{
	UINT16 tmp = A - RM( BC ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 aa: 0111 0000 1010 1010 */
static void GTAX_D(void)
{
	UINT16 tmp = A - RM( DE ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ab: 0111 0000 1010 1011 */
static void GTAX_H(void)
{
	UINT16 tmp = A - RM( HL ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ac: 0111 0000 1010 1100 */
static void GTAX_Dp(void)
{
	UINT16 tmp = A - RM( DE ) - 1;
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ad: 0111 0000 1010 1101 */
static void GTAX_Hp(void)
{
	UINT16 tmp = A - RM( HL ) - 1;
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ae: 0111 0000 1010 1110 */
static void GTAX_Dm(void)
{
	UINT16 tmp = A - RM( DE ) - 1;
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 af: 0111 0000 1010 1111 */
static void GTAX_Hm(void)
{
	UINT16 tmp = A - RM( HL ) - 1;
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 b1: 0111 0000 1011 0001 */
static void SUBNBX_B(void)
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b2: 0111 0000 1011 0010 */
static void SUBNBX_D(void)
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b3: 0111 0000 1011 0011 */
static void SUBNBX_H(void)
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b4: 0111 0000 1011 0100 */
static void SUBNBX_Dp(void)
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b5: 0111 0000 1011 0101 */
static void SUBNBX_Hp(void)
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b6: 0111 0000 1011 0110 */
static void SUBNBX_Dm(void)
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b7: 0111 0000 1011 0111 */
static void SUBNBX_Hm(void)
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b9: 0111 0000 1011 1001 */
static void LTAX_B(void)
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 ba: 0111 0000 1011 1010 */
static void LTAX_D(void)
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bb: 0111 0000 1011 1011 */
static void LTAX_H(void)
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bc: 0111 0000 1011 1100 */
static void LTAX_Dp(void)
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bd: 0111 0000 1011 1101 */
static void LTAX_Hp(void)
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 be: 0111 0000 1011 1110 */
static void LTAX_Dm(void)
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bf: 0111 0000 1011 1111 */
static void LTAX_Hm(void)
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 c1: 0111 0000 1100 0001 */
static void ADDX_B(void)
{
	UINT8 tmp = A + RM( BC );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c2: 0111 0000 1100 0010 */
static void ADDX_D(void)
{
	UINT8 tmp = A + RM( DE );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c3: 0111 0000 1100 0011 */
static void ADDX_H(void)
{
	UINT8 tmp = A + RM( HL );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c4: 0111 0000 1100 0100 */
static void ADDX_Dp(void)
{
	UINT8 tmp = A + RM( DE );
	DE++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c5: 0111 0000 1100 0101 */
static void ADDX_Hp(void)
{
	UINT8 tmp = A + RM( HL );
	HL++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c6: 0111 0000 1100 0110 */
static void ADDX_Dm(void)
{
	UINT8 tmp = A + RM( DE );
	DE--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c7: 0111 0000 1100 0111 */
static void ADDX_Hm(void)
{
	UINT8 tmp = A + RM( HL );
	HL--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c9: 0111 0000 1100 1001 */
static void ONAX_B(void)
{
	if (A & RM( BC ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 70 ca: 0111 0000 1100 1010 */
static void ONAX_D(void)
{
	if (A & RM( DE ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 70 cb: 0111 0000 1100 1011 */
static void ONAX_H(void)
{
	if (A & RM( HL ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 70 cc: 0111 0000 1100 1100 */
static void ONAX_Dp(void)
{
	if (A & RM( DE ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	DE++;
}

/* 70 cd: 0111 0000 1100 1101 */
static void ONAX_Hp(void)
{
	if (A & RM( HL ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	HL++;
}

/* 70 ce: 0111 0000 1100 1110 */
static void ONAX_Dm(void)
{
	if (A & RM( DE ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	DE--;
}

/* 70 cf: 0111 0000 1100 1111 */
static void ONAX_Hm(void)
{
	if (A & RM( HL ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	HL--;
}

/* 70 d1: 0111 0000 1101 0001 */
static void ADCX_B(void)
{
	UINT8 tmp = A + RM( BC ) + (PSW & CY);
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d2: 0111 0000 1101 0010 */
static void ADCX_D(void)
{
	UINT8 tmp = A + RM( DE ) + (PSW & CY);
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d3: 0111 0000 1101 0011 */
static void ADCX_H(void)
{
	UINT8 tmp = A + RM( HL ) + (PSW & CY);
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d4: 0111 0000 1101 0100 */
static void ADCX_Dp(void)
{
	UINT8 tmp = A + RM( DE ) + (PSW & CY);
	DE++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d5: 0111 0000 1101 0101 */
static void ADCX_Hp(void)
{
	UINT8 tmp = A + RM( HL ) + (PSW & CY);
	HL++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d6: 0111 0000 1101 0110 */
static void ADCX_Dm(void)
{
	UINT8 tmp = A + RM( DE ) + (PSW & CY);
	DE--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d7: 0111 0000 1101 0111 */
static void ADCX_Hm(void)
{
	UINT8 tmp = A + RM( HL ) + (PSW & CY);
	HL--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d9: 0111 0000 1101 1001 */
static void OFFAX_B(void)
{
	if ( A & RM( BC ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 70 da: 0111 0000 1101 1010 */
static void OFFAX_D(void)
{
	if ( A & RM( DE ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 70 db: 0111 0000 1101 1011 */
static void OFFAX_H(void)
{
	if ( A & RM( HL ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 70 dc: 0111 0000 1101 1100 */
static void OFFAX_Dp(void)
{
	if ( A & RM( DE ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	DE++;
}

/* 70 dd: 0111 0000 1101 1101 */
static void OFFAX_Hp(void)
{
	if ( A & RM( HL ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	HL++;
}

/* 70 de: 0111 0000 1101 1110 */
static void OFFAX_Dm(void)
{
	if ( A & RM( DE ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	DE--;
}

/* 70 df: 0111 0000 1101 1111 */
static void OFFAX_Hm(void)
{
	if ( A & RM( HL ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	HL--;
}

/* 70 e1: 0111 0000 1110 0001 */
static void SUBX_B(void)
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 70 e2: 0111 0000 1110 0010 */
static void SUBX_D(void)
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 70 e3: 0111 0000 1110 0011 */
static void SUBX_H(void)
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 70 e4: 0111 0000 1110 0100 */
static void SUBX_Dp(void)
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	DE++;
}

/* 70 e5: 0111 0000 1110 0101 */
static void SUBX_Hp(void)
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	HL++;
}

/* 70 e6: 0111 0000 1110 0110 */
static void SUBX_Dm(void)
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	DE--;
}

/* 70 e7: 0111 0000 1110 0111 */
static void SUBX_Hm(void)
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	HL--;
}

/* 70 e9: 0111 0000 1110 1001 */
static void NEAX_B(void)
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ea: 0111 0000 1110 1010 */
static void NEAX_D(void)
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 eb: 0111 0000 1110 1011 */
static void NEAX_H(void)
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ec: 0111 0000 1110 1100 */
static void NEAX_Dp(void)
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ed: 0111 0000 1110 1101 */
static void NEAX_Hp(void)
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ee: 0111 0000 1110 1110 */
static void NEAX_Dm(void)
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ef: 0111 0000 1110 1111 */
static void NEAX_Hm(void)
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 f1: 0111 0000 1111 0001 */
static void SBBX_B(void)
{
	UINT8 tmp = A - RM( BC ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f2: 0111 0000 1111 0010 */
static void SBBX_D(void)
{
	UINT8 tmp = A - RM( DE ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f3: 0111 0000 1111 0011 */
static void SBBX_H(void)
{
	UINT8 tmp = A - RM( HL ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f4: 0111 0000 1111 0100 */
static void SBBX_Dp(void)
{
	UINT8 tmp = A - RM( DE ) - (PSW & CY);
	DE++;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f5: 0111 0000 1111 0101 */
static void SBBX_Hp(void)
{
	UINT8 tmp = A - RM( HL ) - (PSW & CY);
	HL++;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f6: 0111 0000 1111 0110 */
static void SBBX_Dm(void)
{
	UINT8 tmp = A - RM( DE ) - (PSW & CY);
	DE--;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f7: 0111 0000 1111 0111 */
static void SBBX_Hm(void)
{
	UINT8 tmp = A - RM( HL ) - (PSW & CY);
	HL--;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f9: 0111 0000 1111 1001 */
static void EQAX_B(void)
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fa: 0111 0000 1111 1010 */
static void EQAX_D(void)
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fb: 0111 0000 1111 1011 */
static void EQAX_H(void)
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fc: 0111 0000 1111 1100 */
static void EQAX_Dp(void)
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fd: 0111 0000 1111 1101 */
static void EQAX_Hp(void)
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fe: 0111 0000 1111 1110 */
static void EQAX_Dm(void)
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 ff: 0111 0000 1111 1111 */
static void EQAX_Hm(void)
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* prefix 74 */
/* 74 08: 0111 0100 0000 1000 xxxx xxxx */
static void ANI_V_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	V &= imm;
	SET_Z(V);
}

/* 74 09: 0111 0100 0000 1001 xxxx xxxx */
static void ANI_A_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	A &= imm;
	SET_Z(A);
}

/* 74 0a: 0111 0100 0000 1010 xxxx xxxx */
static void ANI_B_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	B &= imm;
	SET_Z(B);
}

/* 74 0b: 0111 0100 0000 1011 xxxx xxxx */
static void ANI_C_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	C &= imm;
	SET_Z(C);
}

/* 74 0c: 0111 0100 0000 1100 xxxx xxxx */
static void ANI_D_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	D &= imm;
	SET_Z(D);
}

/* 74 0d: 0111 0100 0000 1101 xxxx xxxx */
static void ANI_E_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	E &= imm;
	SET_Z(E);
}

/* 74 0e: 0111 0100 0000 1110 xxxx xxxx */
static void ANI_H_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	H &= imm;
	SET_Z(H);
}

/* 74 0f: 0111 0100 0000 1111 xxxx xxxx */
static void ANI_L_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	L &= imm;
	SET_Z(L);
}

/* 74 10: 0111 0100 0001 0000 xxxx xxxx */
static void XRI_V_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	V ^= imm;
	SET_Z(V);
}

/* 74 11: 0111 0100 0001 0001 xxxx xxxx */
static void XRI_A_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	A ^= imm;
	SET_Z(A);
}

/* 74 12: 0111 0100 0001 0010 xxxx xxxx */
static void XRI_B_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	B ^= imm;
	SET_Z(B);
}

/* 74 13: 0111 0100 0001 0011 xxxx xxxx */
static void XRI_C_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	C ^= imm;
	SET_Z(C);
}

/* 74 14: 0111 0100 0001 0100 xxxx xxxx */
static void XRI_D_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	D ^= imm;
	SET_Z(D);
}

/* 74 15: 0111 0100 0001 0101 xxxx xxxx */
static void XRI_E_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	E ^= imm;
	SET_Z(E);
}

/* 74 16: 0111 0100 0001 0110 xxxx xxxx */
static void XRI_H_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	H ^= imm;
	SET_Z(H);
}

/* 74 17: 0111 0100 0001 0111 xxxx xxxx */
static void XRI_L_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	L ^= imm;
	SET_Z(L);
}

/* 74 18: 0111 0100 0001 1000 xxxx xxxx */
static void ORI_V_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	V |= imm;
	SET_Z(V);
}

/* 74 19: 0111 0100 0001 1001 xxxx xxxx */
static void ORI_A_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	A |= imm;
	SET_Z(A);
}

/* 74 1a: 0111 0100 0001 1010 xxxx xxxx */
static void ORI_B_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	B |= imm;
	SET_Z(B);
}

/* 74 1b: 0111 0100 0001 1011 xxxx xxxx */
static void ORI_C_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	C |= imm;
	SET_Z(C);
}

/* 74 1c: 0111 0100 0001 1100 xxxx xxxx */
static void ORI_D_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	D |= imm;
	SET_Z(D);
}

/* 74 1d: 0111 0100 0001 1101 xxxx xxxx */
static void ORI_E_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	E |= imm;
	SET_Z(E);
}

/* 74 1e: 0111 0100 0001 1110 xxxx xxxx */
static void ORI_H_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	H |= imm;
	SET_Z(H);
}

/* 74 1f: 0111 0100 0001 1111 xxxx xxxx */
static void ORI_L_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	L |= imm;
	SET_Z(L);
}

/* 74 20: 0111 0100 0010 0000 xxxx xxxx */
static void ADINC_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V + imm;

	ZHC_ADD( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 74 21: 0111 0100 0010 0001 xxxx xxxx */
static void ADINC_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A + imm;

	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 22: 0111 0100 0010 0010 xxxx xxxx */
static void ADINC_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B + imm;

	ZHC_ADD( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 74 23: 0111 0100 0010 0011 xxxx xxxx */
static void ADINC_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C + imm;

	ZHC_ADD( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 74 24: 0111 0100 0010 0100 xxxx xxxx */
static void ADINC_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D + imm;

	ZHC_ADD( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 74 25: 0111 0100 0010 0101 xxxx xxxx */
static void ADINC_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E + imm;

	ZHC_ADD( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 74 26: 0111 0100 0010 0110 xxxx xxxx */
static void ADINC_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H + imm;

	ZHC_ADD( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 74 27: 0111 0100 0010 0111 xxxx xxxx */
static void ADINC_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L + imm;

	ZHC_ADD( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 74 28: 0111 0100 0010 1000 xxxx xxxx */
static void GTI_V_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = V - imm - 1;
	ZHC_SUB( tmp, V, 0 );

	SKIP_NC;
}

/* 74 29: 0111 0100 0010 1001 xxxx xxxx */
static void GTI_A_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = A - imm - 1;
	ZHC_SUB( tmp, A, 0 );

	SKIP_NC;
}

/* 74 2a: 0111 0100 0010 1010 xxxx xxxx */
static void GTI_B_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = B - imm - 1;
	ZHC_SUB( tmp, B, 0 );

	SKIP_NC;
}

/* 74 2b: 0111 0100 0010 1011 xxxx xxxx */
static void GTI_C_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = C - imm - 1;
	ZHC_SUB( tmp, C, 0 );

	SKIP_NC;
}

/* 74 2c: 0111 0100 0010 1100 xxxx xxxx */
static void GTI_D_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = D - imm - 1;
	ZHC_SUB( tmp, D, 0 );

	SKIP_NC;
}

/* 74 2d: 0111 0100 0010 1101 xxxx xxxx */
static void GTI_E_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = E - imm - 1;
	ZHC_SUB( tmp, E, 0 );

	SKIP_NC;
}

/* 74 2e: 0111 0100 0010 1110 xxxx xxxx */
static void GTI_H_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = H - imm - 1;
	ZHC_SUB( tmp, H, 0 );

	SKIP_NC;
}

/* 74 2f: 0111 0100 0010 1111 xxxx xxxx */
static void GTI_L_xx(void)
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = L - imm - 1;
	ZHC_SUB( tmp, L, 0 );

	SKIP_NC;
}

/* 74 30: 0111 0100 0011 0000 xxxx xxxx */
static void SUINB_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 74 31: 0111 0100 0011 0001 xxxx xxxx */
static void SUINB_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 32: 0111 0100 0011 0010 xxxx xxxx */
static void SUINB_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 74 33: 0111 0100 0011 0011 xxxx xxxx */
static void SUINB_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 74 34: 0111 0100 0011 0100 xxxx xxxx */
static void SUINB_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 74 35: 0111 0100 0011 0101 xxxx xxxx */
static void SUINB_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 74 36: 0111 0100 0011 0110 xxxx xxxx */
static void SUINB_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 74 37: 0111 0100 0011 0111 xxxx xxxx */
static void SUINB_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 74 38: 0111 0100 0011 1000 xxxx xxxx */
static void LTI_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	SKIP_CY;
}

/* 74 39: 0111 0100 0011 1001 xxxx xxxx */
static void LTI_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 74 3a: 0111 0100 0011 1010 xxxx xxxx */
static void LTI_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	SKIP_CY;
}

/* 74 3b: 0111 0100 0011 1011 xxxx xxxx */
static void LTI_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	SKIP_CY;
}

/* 74 3c: 0111 0100 0011 1100 xxxx xxxx */
static void LTI_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	SKIP_CY;
}

/* 74 3d: 0111 0100 0011 1101 xxxx xxxx */
static void LTI_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	SKIP_CY;
}

/* 74 3e: 0111 0100 0011 1110 xxxx xxxx */
static void LTI_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	SKIP_CY;
}

/* 74 3f: 0111 0100 0011 1111 xxxx xxxx */
static void LTI_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	SKIP_CY;
}

/* 74 40: 0111 0100 0100 0000 xxxx xxxx */
static void ADI_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V + imm;

	ZHC_ADD( tmp, V, 0 );
	V = tmp;
}

/* 74 41: 0111 0100 0100 0001 xxxx xxxx */
static void ADI_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A + imm;

	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 74 42: 0111 0100 0100 0010 xxxx xxxx */
static void ADI_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B + imm;

	ZHC_ADD( tmp, B, 0 );
	B = tmp;
}

/* 74 43: 0111 0100 0100 0011 xxxx xxxx */
static void ADI_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C + imm;

	ZHC_ADD( tmp, C, 0 );
	C = tmp;
}

/* 74 44: 0111 0100 0100 0100 xxxx xxxx */
static void ADI_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D + imm;

	ZHC_ADD( tmp, D, 0 );
	D = tmp;
}

/* 74 45: 0111 0100 0100 0101 xxxx xxxx */
static void ADI_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E + imm;

	ZHC_ADD( tmp, E, 0 );
	E = tmp;
}

/* 74 46: 0111 0100 0100 0110 xxxx xxxx */
static void ADI_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H + imm;

	ZHC_ADD( tmp, H, 0 );
	H = tmp;
}

/* 74 47: 0111 0100 0100 0111 xxxx xxxx */
static void ADI_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L + imm;

	ZHC_ADD( tmp, L, 0 );
	L = tmp;
}

/* 74 48: 0111 0100 0100 1000 xxxx xxxx */
static void ONI_V_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (V & imm)
		PSW |= SK;
}

/* 74 49: 0111 0100 0100 1001 xxxx xxxx */
static void ONI_A_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (A & imm)
		PSW |= SK;
}

/* 74 4a: 0111 0100 0100 1010 xxxx xxxx */
static void ONI_B_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (B & imm)
		PSW |= SK;
}

/* 74 4b: 0111 0100 0100 1011 xxxx xxxx */
static void ONI_C_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (C & imm)
		PSW |= SK;
}

/* 74 4c: 0111 0100 0100 1100 xxxx xxxx */
static void ONI_D_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (D & imm)
		PSW |= SK;
}

/* 74 4d: 0111 0100 0100 1101 xxxx xxxx */
static void ONI_E_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (E & imm)
		PSW |= SK;
}

/* 74 4e: 0111 0100 0100 1110 xxxx xxxx */
static void ONI_H_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (H & imm)
		PSW |= SK;
}

/* 74 4f: 0111 0100 0100 1111 xxxx xxxx */
static void ONI_L_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (L & imm)
		PSW |= SK;
}

/* 74 50: 0111 0100 0101 0000 xxxx xxxx */
static void ACI_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V + imm + (PSW & CY);
	ZHC_SUB( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 74 51: 0111 0100 0101 0001 xxxx xxxx */
static void ACI_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A + imm + (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 52: 0111 0100 0101 0010 xxxx xxxx */
static void ACI_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B + imm + (PSW & CY);
	ZHC_SUB( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 74 53: 0111 0100 0101 0011 xxxx xxxx */
static void ACI_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C + imm + (PSW & CY);
	ZHC_SUB( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 74 54: 0111 0100 0101 0100 xxxx xxxx */
static void ACI_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D + imm + (PSW & CY);
	ZHC_SUB( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 74 55: 0111 0100 0101 0101 xxxx xxxx */
static void ACI_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E + imm + (PSW & CY);
	ZHC_SUB( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 74 56: 0111 0100 0101 0110 xxxx xxxx */
static void ACI_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H + imm + (PSW & CY);
	ZHC_SUB( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 74 57: 0111 0100 0101 0111 xxxx xxxx */
static void ACI_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L + imm + (PSW & CY);
	ZHC_SUB( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 74 58: 0111 0100 0101 1000 xxxx xxxx */
static void OFFI_V_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (V & imm))
		PSW |= SK;
}

/* 74 59: 0111 0100 0101 1001 xxxx xxxx */
static void OFFI_A_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (A & imm))
		PSW |= SK;
}

/* 74 5a: 0111 0100 0101 1010 xxxx xxxx */
static void OFFI_B_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (B & imm))
		PSW |= SK;
}

/* 74 5b: 0111 0100 0101 1011 xxxx xxxx */
static void OFFI_C_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (C & imm))
		PSW |= SK;
}

/* 74 5c: 0111 0100 0101 1100 xxxx xxxx */
static void OFFI_D_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (D & imm))
		PSW |= SK;
}

/* 74 5d: 0111 0100 0101 1101 xxxx xxxx */
static void OFFI_E_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (E & imm))
		PSW |= SK;
}

/* 74 5e: 0111 0100 0101 1110 xxxx xxxx */
static void OFFI_H_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (H & imm))
		PSW |= SK;
}

/* 74 5f: 0111 0100 0101 1111 xxxx xxxx */
static void OFFI_L_xx(void)
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (L & imm))
		PSW |= SK;
}

/* 74 60: 0111 0100 0110 0000 xxxx xxxx */
static void SUI_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
}

/* 74 61: 0111 0100 0110 0001 xxxx xxxx */
static void SUI_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 74 62: 0111 0100 0110 0010 xxxx xxxx */
static void SUI_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
}

/* 74 63: 0111 0100 0110 0011 xxxx xxxx */
static void SUI_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
}

/* 74 64: 0111 0100 0110 0100 xxxx xxxx */
static void SUI_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
}

/* 74 65: 0111 0100 0110 0101 xxxx xxxx */
static void SUI_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
}

/* 74 66: 0111 0100 0110 0110 xxxx xxxx */
static void SUI_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
}

/* 74 67: 0111 0100 0110 0111 xxxx xxxx */
static void SUI_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
}

/* 74 68: 0111 0100 0110 1000 xxxx xxxx */
static void NEI_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	SKIP_NZ;
}

/* 74 69: 0111 0100 0110 1001 xxxx xxxx */
static void NEI_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 74 6a: 0111 0100 0110 1010 xxxx xxxx */
static void NEI_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	SKIP_NZ;
}

/* 74 6b: 0111 0100 0110 1011 xxxx xxxx */
static void NEI_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	SKIP_NZ;
}

/* 74 6c: 0111 0100 0110 1100 xxxx xxxx */
static void NEI_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	SKIP_NZ;
}

/* 74 6d: 0111 0100 0110 1101 xxxx xxxx */
static void NEI_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	SKIP_NZ;
}

/* 74 6e: 0111 0100 0110 1110 xxxx xxxx */
static void NEI_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	SKIP_NZ;
}

/* 74 6f: 0111 0100 0110 1111 xxxx xxxx */
static void NEI_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	SKIP_NZ;
}

/* 74 70: 0111 0100 0111 0000 xxxx xxxx */
static void SBI_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm - (PSW & CY);
	ZHC_SUB( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 74 71: 0111 0100 0111 0001 xxxx xxxx */
static void SBI_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 72: 0111 0100 0111 0010 xxxx xxxx */
static void SBI_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm - (PSW & CY);
	ZHC_SUB( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 74 73: 0111 0100 0111 0011 xxxx xxxx */
static void SBI_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm - (PSW & CY);
	ZHC_SUB( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 74 74: 0111 0100 0111 0100 xxxx xxxx */
static void SBI_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm - (PSW & CY);
	ZHC_SUB( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 74 75: 0111 0100 0111 0101 xxxx xxxx */
static void SBI_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm - (PSW & CY);
	ZHC_SUB( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 74 76: 0111 0100 0111 0110 xxxx xxxx */
static void SBI_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm - (PSW & CY);
	ZHC_SUB( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 74 77: 0111 0100 0111 0111 xxxx xxxx */
static void SBI_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm - (PSW & CY);
	ZHC_SUB( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 74 78: 0111 0100 0111 1000 xxxx xxxx */
static void EQI_V_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	SKIP_Z;
}

/* 74 79: 0111 0100 0111 1001 xxxx xxxx */
static void EQI_A_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 74 7a: 0111 0100 0111 1010 xxxx xxxx */
static void EQI_B_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	SKIP_Z;
}

/* 74 7b: 0111 0100 0111 1011 xxxx xxxx */
static void EQI_C_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	SKIP_Z;
}

/* 74 7c: 0111 0100 0111 1100 xxxx xxxx */
static void EQI_D_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	SKIP_Z;
}

/* 74 7d: 0111 0100 0111 1101 xxxx xxxx */
static void EQI_E_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	SKIP_Z;
}

/* 74 7e: 0111 0100 0111 1110 xxxx xxxx */
static void EQI_H_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	SKIP_Z;
}

/* 74 7f: 0111 0100 0111 1111 xxxx xxxx */
static void EQI_L_xx(void)
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	SKIP_Z;
}

/* 74 88: 0111 0100 1000 1000 oooo oooo */
static void ANAW_wa(void)
{
	PAIR ea = upd7810.va;
	RDOPARG( ea.b.l );

	A &= RM( ea.d );
	SET_Z(A);
}

/* 74 8d: 0111 0100 1000 1101 */
static void DAN_EA_BC(void)
{
	EA &= BC;
	SET_Z(EA);
}

/* 74 8e: 0111 0100 1000 1110 */
static void DAN_EA_DE(void)
{
	EA &= DE;
	SET_Z(EA);
}

/* 74 8f: 0111 0100 1000 1111 */
static void DAN_EA_HL(void)
{
	EA &= HL;
	SET_Z(EA);
}

/* 74 90: 0111 0100 1001 0000 oooo oooo */
static void XRAW_wa(void)
{
	PAIR ea = upd7810.va;
	RDOPARG( ea.b.l );

	A ^= RM( ea.d );
	SET_Z(A);
}

/* 74 95: 0111 0100 1001 0101 */
static void DXR_EA_BC(void)
{
	EA ^= BC;
	SET_Z(EA);
}

/* 74 96: 0111 0100 1001 0110 */
static void DXR_EA_DE(void)
{
	EA ^= DE;
	SET_Z(EA);
}

/* 74 97: 0111 0100 1001 0111 */
static void DXR_EA_HL(void)
{
	EA ^= HL;
	SET_Z(EA);
}

/* 74 98: 0111 0100 1001 1000 oooo oooo */
static void ORAW_wa(void)
{
	PAIR ea = upd7810.va;
	RDOPARG( ea.b.l );

	A |= RM( ea.d );
	SET_Z(A);
}

/* 74 9d: 0111 0100 1001 1101 */
static void DOR_EA_BC(void)
{
	EA |= BC;
	SET_Z(EA);
}

/* 74 9e: 0111 0100 1001 1110 */
static void DOR_EA_DE(void)
{
	EA |= DE;
	SET_Z(EA);
}

/* 74 9f: 0111 0100 1001 1111 */
static void DOR_EA_HL(void)
{
	EA |= HL;
	SET_Z(EA);
}

/* 74 a0: 0111 0100 1010 0000 oooo oooo */
static void ADDNCW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );

	tmp = A + RM( ea.d );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 a5: 0111 0100 1010 0101 */
static void DADDNC_EA_BC(void)
{
	UINT16 tmp = EA + BC;

	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 a6: 0111 0100 1010 0110 */
static void DADDNC_EA_DE(void)
{
	UINT16 tmp = EA + DE;

	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 a7: 0111 0100 1010 0111 */
static void DADDNC_EA_HL(void)
{
	UINT16 tmp = EA + HL;

	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 a8: 0111 0100 1010 1000 oooo oooo */
static void GTAW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT16 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 74 ad: 0111 0100 1010 1101 */
static void DGT_EA_BC(void)
{
	UINT32 tmp = EA - BC - 1;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NC;
}

/* 74 ae: 0111 0100 1010 1110 */
static void DGT_EA_DE(void)
{
	UINT32 tmp = EA - DE - 1;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NC;
}

/* 74 af: 0111 0100 1010 1111 */
static void DGT_EA_HL(void)
{
	UINT32 tmp = EA - HL - 1;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NC;
}

/* 74 b0: 0111 0100 1011 0000 oooo oooo */
static void SUBNBW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 b5: 0111 0100 1011 0101 */
static void DSUBNB_EA_BC(void)
{
	UINT16 tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 b6: 0111 0100 1011 0110 */
static void DSUBNB_EA_DE(void)
{
	UINT16 tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 b7: 0111 0100 1011 0111 */
static void DSUBNB_EA_HL(void)
{
	UINT16 tmp;

	tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 b8: 0111 0100 1011 1000 oooo oooo */
static void LTAW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 74 bd: 0111 0100 1011 1101 */
static void DLT_EA_BC(void)
{
	UINT16 tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_CY;
}

/* 74 be: 0111 0100 1011 1110 */
static void DLT_EA_DE(void)
{
	UINT16 tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_CY;
}

/* 74 bf: 0111 0100 1011 1111 */
static void DLT_EA_HL(void)
{
	UINT16 tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_CY;
}

/* 74 c0: 0111 0100 1100 0000 oooo oooo */
static void ADDW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;
	RDOPARG( ea.b.l );
	tmp = A + RM( ea.d );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 74 c5: 0111 0100 1100 0101 */
static void DADD_EA_BC(void)
{
	UINT16 tmp = EA + BC;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 74 c6: 0111 0100 1100 0110 */
static void DADD_EA_DE(void)
{
	UINT16 tmp = EA + DE;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 74 c7: 0111 0100 1100 0111 */
static void DADD_EA_HL(void)
{
	UINT16 tmp = EA + HL;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 74 c8: 0111 0100 1100 1000 oooo oooo */
static void ONAW_wa(void)
{
	PAIR ea = upd7810.va;
	RDOPARG( ea.b.l );

	if (A & RM( ea.d ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 cd: 0111 0100 1100 1101 */
static void DON_EA_BC(void)
{
	if (EA & BC)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 ce: 0111 0100 1100 1110 */
static void DON_EA_DE(void)
{
	if (EA & DE)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 cf: 0111 0100 1100 1111 */
static void DON_EA_HL(void)
{
	if (EA & HL)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 d0: 0111 0100 1101 0000 oooo oooo */
static void ADCW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A + RM( ea.d ) + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 d5: 0111 0100 1101 0101 */
static void DADC_EA_BC(void)
{
	UINT16 tmp = EA + BC + (PSW & CY);
	ZHC_ADD( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 d6: 0111 0100 1101 0110 */
static void DADC_EA_DE(void)
{
	UINT16 tmp = EA + DE + (PSW & CY);
	ZHC_ADD( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 d7: 0111 0100 1101 0111 */
static void DADC_EA_HL(void)
{
	UINT16 tmp = EA + HL + (PSW & CY);
	ZHC_ADD( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 d8: 0111 0100 1101 1000 oooo oooo */
static void OFFAW_wa(void)
{
	PAIR ea = upd7810.va;
	RDOPARG( ea.b.l );

	if ( A & RM( ea.d ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 dd: 0111 0100 1101 1101 */
static void DOFF_EA_BC(void)
{
	if ( EA & BC )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 de: 0111 0100 1101 1110 */
static void DOFF_EA_DE(void)
{
	if ( EA & DE )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 df: 0111 0100 1101 1111 */
static void DOFF_EA_HL(void)
{
	if ( EA & HL )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 e0: 0111 0100 1110 0000 oooo oooo */
static void SUBW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 74 e5: 0111 0100 1110 0101 */
static void DSUB_EA_BC(void)
{
	UINT16 tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 74 e6: 0111 0100 1110 0110 */
static void DSUB_EA_DE(void)
{
	UINT16 tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 74 e7: 0111 0100 1110 0111 */
static void DSUB_EA_HL(void)
{
	UINT16 tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 74 e8: 0111 0100 1110 1000 oooo oooo */
static void NEAW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 74 ed: 0111 0100 1110 1101 */
static void DNE_EA_BC(void)
{
	UINT16 tmp;

	tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NZ;
}

/* 74 ee: 0111 0100 1110 1110 */
static void DNE_EA_DE(void)
{
	UINT16 tmp;

	tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NZ;
}

/* 74 ef: 0111 0100 1110 1111 */
static void DNE_EA_HL(void)
{
	UINT16 tmp;

	tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NZ;
}

/* 74 f0: 0111 0100 1111 0000 oooo oooo */
static void SBBW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 f5: 0111 0100 1111 0101 */
static void DSBB_EA_BC(void)
{
	UINT16 tmp = EA - BC - (PSW & CY);
	ZHC_SUB( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 f6: 0111 0100 1111 0110 */
static void DSBB_EA_DE(void)
{
	UINT16 tmp = EA - DE - (PSW & CY);
	ZHC_SUB( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 f7: 0111 0100 1111 0111 */
static void DSBB_EA_HL(void)
{
	UINT16 tmp = EA - HL - (PSW & CY);
	ZHC_SUB( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 f8: 0111 0100 1111 1000 oooo oooo */
static void EQAW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 74 fd: 0111 0100 1111 1101 */
static void DEQ_EA_BC(void)
{
	UINT16 tmp;

	tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_Z;
}

/* 74 fe: 0111 0100 1111 1110 */
static void DEQ_EA_DE(void)
{
	UINT16 tmp;

	tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_Z;
}

/* 74 ff: 0111 0100 1111 1111 */
static void DEQ_EA_HL(void)
{
	UINT16 tmp;

	tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_Z;
}

/************************************************
 * main opcodes
 ************************************************/

/* 00: 0000 0000 */
static void NOP(void)
{
}

/* 01: 0000 0001 oooo oooo */
static void LDAW_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	A = RM( ea.d );
}

/* 02: 0000 0010 */
static void INX_SP(void)
{
	SP++;
}

/* 03: 0000 0011 */
static void DCX_SP(void)
{
	SP--;
}

/* 04: 0000 0100 llll llll hhhh hhhh */
static void LXI_S_w(void)
{
	RDOPARG( SPL );
	RDOPARG( SPH );
}

/* 05: 0000 0101 oooo oooo xxxx xxxx */
static void ANIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 m, imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );
	m = RM( ea.d );
	m &= imm;
	WM( ea.d, m );
	SET_Z(m);
}

/* 07: 0000 0111 xxxx xxxx */
/* ANI_A_xx already defined (long form) */

/* 08: 0000 1000 */
static void MOV_A_EAH(void)
{
	A = EAH;
}

/* 09: 0000 1001 */
static void MOV_A_EAL(void)
{
	A = EAL;
}

/* 0a: 0000 1010 */
static void MOV_A_B(void)
{
	A = B;
}

/* 0b: 0000 1011 */
static void MOV_A_C(void)
{
	A = C;
}

/* 0c: 0000 1100 */
static void MOV_A_D(void)
{
	A = D;
}

/* 0d: 0000 1101 */
static void MOV_A_E(void)
{
	A = E;
}

/* 0e: 0000 1110 */
static void MOV_A_H(void)
{
	A = H;
}

/* 0f: 0000 1111 */
static void MOV_A_L(void)
{
	A = L;
}

/* 10: 0001 0000 */
static void EXA(void)
{
	UINT16 tmp;
	tmp = EA; EA = EA2; EA2 = tmp;
	tmp = VA; VA = VA2; VA2 = tmp;
}

/* 11: 0001 0001 */
static void EXX(void)
{
	UINT16 tmp;
	tmp = BC; BC = BC2; BC2 = tmp;
	tmp = DE; DE = DE2; DE2 = tmp;
	tmp = HL; HL = HL2; HL2 = tmp;
}

/* 48 AD (7807 only) */
static void EXR(void)
{
	UINT16 tmp;
	tmp = BC; BC = BC2; BC2 = tmp;
	tmp = DE; DE = DE2; DE2 = tmp;
	tmp = HL; HL = HL2; HL2 = tmp;
	tmp = EA; EA = EA2; EA2 = tmp;
	tmp = VA; VA = VA2; VA2 = tmp;
}

/* 12: 0001 0010 */
static void INX_BC(void)
{
	BC++;
}

/* 13: 0001 0011 */
static void DCX_BC(void)
{
	BC--;
}

/* 14: 0001 0100 llll llll hhhh hhhh */
static void LXI_B_w(void)
{
	RDOPARG( C );
	RDOPARG( B );
}

/* 15: 0001 0101 oooo oooo xxxx xxxx */
static void ORIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 m, imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );
	m = RM( ea.d );
	m |= imm;
	WM( ea.d, m );
	SET_Z(m);
}

/* 16: 0001 0110 xxxx xxxx */
/* XRI_A_xx already defined (long form) */

/* 17: 0001 0111 xxxx xxxx */
/* ORI_A_xx already defined (long form) */

/* 18: 0001 1000 */
static void MOV_EAH_A(void)
{
	EAH = A;
}

/* 19: 0001 1001 */
static void MOV_EAL_A(void)
{
	EAL = A;
}

/* 1a: 0001 1010 */
static void MOV_B_A(void)
{
	B = A;
}

/* 1b: 0001 1011 */
static void MOV_C_A(void)
{
	C = A;
}

/* 1c: 0001 1100 */
static void MOV_D_A(void)
{
	D = A;
}

/* 1d: 0001 1101 */
static void MOV_E_A(void)
{
	E = A;
}

/* 1e: 0001 1110 */
static void MOV_H_A(void)
{
	H = A;
}

/* 1f: 0001 1111 */
static void MOV_L_A(void)
{
	L = A;
}

/* 20: 0010 0000 oooo oooo */
static void INRW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp, m;

	RDOPARG( ea.b.l );
	m = RM( ea.d );
	tmp = m + 1;
	ZHC_ADD( tmp, m, 0 );
	WM( ea.d, tmp );
	SKIP_CY;
}

/* 21: 0010 0001 */
static void JB(void)
{
	PC = BC;
	change_pc( PCD );
}

/* 22: 0010 0010 */
static void INX_DE(void)
{
	DE++;
}

/* 23: 0010 0011 */
static void DCX_DE(void)
{
	DE--;
}

/* 24: 0010 0100 llll llll hhhh hhhh */
static void LXI_D_w(void)
{
	RDOPARG( E );
	RDOPARG( D );
}

/* 25: 0010 0101 oooo oooo xxxx xxxx */
static void GTIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 m, imm;
	UINT16 tmp;

	RDOPARG( ea.b.l );
	RDOPARG( imm );
	m = RM( ea.d );
	tmp = m - imm - 1;
	ZHC_SUB( tmp, m, 0 );
	SKIP_NC;
}

/* 26: 0010 0110 xxxx xxxx */
/* ADINC_A_xx already defined (long form) */

/* 27: 0010 0111 xxxx xxxx */
/* GTI_A_xx already defined (long form) */

/* 29: 0010 1001 */
static void LDAX_B(void)
{
	A = RM( BC );
}

/* 2a: 0010 1010 */
static void LDAX_D(void)
{
	A = RM( DE );
}

/* 2b: 0010 1011 */
static void LDAX_H(void)
{
	A = RM( HL );
}

/* 2c: 0010 1100 */
static void LDAX_Dp(void)
{
	A = RM( DE );
	DE++;
}

/* 2d: 0010 1101 dddd dddd */
static void LDAX_Hp(void)
{
	A = RM( HL );
	HL++;
}

/* 2e: 0010 1110 dddd dddd */
static void LDAX_Dm(void)
{
	A = RM( DE );
	DE--;
}

/* 2f: 0010 1111 dddd dddd */
static void LDAX_Hm(void)
{
	A = RM( HL );
	HL--;
}

/* 30: 0011 0000 oooo oooo */
static void DCRW_wa(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp, m;

	RDOPARG( ea.b.l );
	m = RM( ea.d );
	tmp = m - 1;
	ZHC_SUB( tmp, m, 0 );
	WM( ea.d, tmp );
	SKIP_CY;
}

/* 31: 0011 0001 */
static void BLOCK(void)
{
	WM( DE, RM( HL ) );
	DE++;
	HL++;
	C--;
	if (C == 0xff)
		PSW |= CY;
	else
	{
		PSW &= ~CY;
		PC--;
	}
}

/* 32: 0011 0010 */
static void INX_HL(void)
{
	HL++;
}

/* 33: 0011 0011 */
static void DCX_HL(void)
{
	HL--;
}

/* 34: 0011 0100 llll llll hhhh hhhh */
static void LXI_H_w(void)
{
	if (PSW & L0) { /* overlay active? */
		PC+=2;
		return;
	}
	RDOPARG( L );
	RDOPARG( H );
	PSW |= L0;
}

/* 35: 0011 0101 oooo oooo xxxx xxxx */
static void LTIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp, m, imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );
	m = RM( ea.d );
	tmp = m - imm;
	ZHC_SUB( tmp, m, 0 );
	SKIP_CY;
}

/* 36: 0011 0110 xxxx xxxx */
/* SUINB_A_xx already defined (long form) */

/* 37: 0011 0111 xxxx xxxx */
/* LTI_A_xx already defined (long form) */

/* 39: 0011 1001 */
static void STAX_B(void)
{
	WM( BC, A );
}

/* 3a: 0011 1010 */
static void STAX_D(void)
{
	WM( DE, A );
}

/* 3b: 0011 1011 */
static void STAX_H(void)
{
	WM( HL, A );
}

/* 3c: 0011 1100 */
static void STAX_Dp(void)
{
	WM( DE, A );
	DE++;
}

/* 3d: 0011 1101 */
static void STAX_Hp(void)
{
	WM( HL, A );
	HL++;
}

/* 3e: 0011 1110 */
static void STAX_Dm(void)
{
	WM( DE, A );
	DE--;
}

/* 3f: 0011 1111 */
static void STAX_Hm(void)
{
	WM( HL, A );
	HL--;
}

/* 40: 0100 0000 llll llll hhhh hhhh */
static void CALL_w(void)
{
	PAIR w;
	w.d = 0;

	RDOPARG( w.b.l );
	RDOPARG( w.b.h );

	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PC = w.w.l;
	change_pc( PCD );
}

/* 41: 0100 0001 */
static void INR_A(void)
{
	UINT8 tmp = A + 1;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_CY;
}

/* 42: 0100 0010 */
static void INR_B(void)
{
	UINT8 tmp = B + 1;
	ZHC_ADD( tmp, B, 0 );
	B = tmp;
	SKIP_CY;
}

/* 43: 0100 0011 */
static void INR_C(void)
{
	UINT8 tmp = C + 1;
	ZHC_ADD( tmp, C, 0 );
	C = tmp;
	SKIP_CY;
}

/* 44: 0100 0100 llll llll hhhh hhhh */
static void LXI_EA_s(void)
{
	RDOPARG( EAL );
	RDOPARG( EAH );
}

/* 45: 0100 0101 oooo oooo xxxx xxxx */
static void ONIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );

	if (RM( ea.d ) & imm)
		PSW |= SK;
}

/* 46: 0100 0110 xxxx xxxx */
/* ADI_A_xx already defined (long form) */

/* 47: 0100 0111 xxxx xxxx */
/* ONI_A_xx already defined (long form) */

/* 48: prefix */
static void PRE_48(void)
{
	RDOP(OP2);
	upd7810_icount -= op48[OP2].cycles;
	upd7810_timers(op48[OP2].cycles);
	(*op48[OP2].opfunc)();
}

/* 49: 0100 1001 xxxx xxxx */
static void MVIX_BC_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WM( BC, imm );
}

/* 4a: 0100 1010 xxxx xxxx */
static void MVIX_DE_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WM( DE, imm );
}

/* 4b: 0100 1011 xxxx xxxx */
static void MVIX_HL_xx(void)
{
	UINT8 imm;
	RDOPARG( imm );
	WM( HL, imm );
}

/* 4c: prefix */
static void PRE_4C(void)
{
	RDOP(OP2);
	upd7810_icount -= op4C[OP2].cycles;
	upd7810_timers(op4C[OP2].cycles);
	(*op4C[OP2].opfunc)();
}

/* 4d: prefix */
static void PRE_4D(void)
{
	RDOP(OP2);
	upd7810_icount -= op4D[OP2].cycles;
	upd7810_timers(op4D[OP2].cycles);
	(*op4D[OP2].opfunc)();
}

/* 4e: 0100 111d dddd dddd */
static void JRE(void)
{
	UINT8 offs;
	RDOPARG( offs );
	if (OP & 0x01)
		PC -= 256 - offs;
	else
		PC += offs;
	change_pc( PCD );
}

/* 50: 0101 0000 */
static void EXH(void)
{
	UINT16 tmp;
	tmp = HL; HL = HL2; HL2 = tmp;
}

/* 51: 0101 0001 */
static void DCR_A(void)
{
	UINT8 tmp = A - 1;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_CY;
}

/* 52: 0101 0010 */
static void DCR_B(void)
{
	UINT8 tmp = B - 1;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
	SKIP_CY;
}

/* 53: 0101 0011 */
static void DCR_C(void)
{
	UINT8 tmp = C - 1;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
	SKIP_CY;
}

/* 54: 0101 0100 llll llll hhhh hhhh */
static void JMP_w(void)
{
	PAIR w;
	w.d = 0;

	RDOPARG( w.b.l );
	RDOPARG( w.b.h );

	PCD = w.d;
	change_pc( PCD );
}

/* 55: 0101 0101 oooo oooo xxxx xxxx */
static void OFFIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );

	if (0 == (RM( ea.d ) & imm))
		PSW |= SK;
}

/* 56: 0101 0110 xxxx xxxx */
/* ACI_A_xx already defined (long form) */

/* 57: 0101 0111 xxxx xxxx */
/* OFFI_A_xx already defined (long form) */

/* 58: 0101 1000 oooo oooo (7810 only) */
static void BIT_0_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x01)
		PSW |= SK;
}

/* 59: 0101 1001 oooo oooo (7810 only) */
static void BIT_1_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x02)
		PSW |= SK;
}

/* 5a: 0101 1010 oooo oooo (7810 only) */
static void BIT_2_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x04)
		PSW |= SK;
}

/* 5b: 0101 1011 oooo oooo (7810 only) */
static void BIT_3_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x08)
		PSW |= SK;
}

/* 5c: 0101 1100 oooo oooo (7810 only) */
static void BIT_4_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x10)
		PSW |= SK;
}

/* 5d: 0101 1101 oooo oooo (7810 only) */
static void BIT_5_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x20)
		PSW |= SK;
}

/* 5e: 0101 1110 oooo oooo (7810 only) */
static void BIT_6_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x40)
		PSW |= SK;
}

/* 5f: 0101 1111 oooo oooo (7810 only) */
static void BIT_7_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x80)
		PSW |= SK;
}

/* 5d: 0101 1111 bbbb bbbb (7807 only) */
static void SKN_bit(void)
{
	UINT8 imm;
	int val;

	RDOPARG( imm );

	switch( imm & 0x1f )
	{
		case 0x10:	/* PA */
			val = RP( UPD7810_PORTA );
			break;
		case 0x11:	/* PB */
			val = RP( UPD7810_PORTB );
			break;
		case 0x12:	/* PC */
			val = RP( UPD7810_PORTC );
			break;
		case 0x13:	/* PD */
			val = RP( UPD7810_PORTD );
			break;
		case 0x15:	/* PF */
			val = RP( UPD7810_PORTF );
			break;
		case 0x16:	/* MKH */
			val = MKH;
			break;
		case 0x17:	/* MKL */
			val = MKL;
			break;
		case 0x19:	/* SMH */
			val = SMH;
			break;
		case 0x1b:	/* EOM */
			val = EOM;
			break;
		case 0x1d:	/* TMM */
			val = TMM;
			break;
		case 0x1e:	/* PT */
			val = RP( UPD7807_PORTT );
			break;
		default:
			logerror("uPD7810 #%d: illegal opcode %02x %02x at PC:%04x\n", cpu_getactivecpu(), OP, imm, PC);
			val = 0;
			break;
	}

	if (~val & (1 << (imm >> 5)))
		PSW |= SK;
}

/* 58: 0101 1000 bbbb bbbb (7807 only) */
static void SETB(void)
{
	UINT8 imm;
	int bit;

	RDOPARG( imm );
	bit = imm >> 5;

	switch( imm & 0x1f )
	{
		case 0x10:	/* PA */
			WP( UPD7810_PORTA, RP( UPD7810_PORTA ) | (1 << bit));
			break;
		case 0x11:	/* PB */
			WP( UPD7810_PORTB, RP( UPD7810_PORTB ) | (1 << bit));
			break;
		case 0x12:	/* PC */
			WP( UPD7810_PORTC, RP( UPD7810_PORTC ) | (1 << bit));
			break;
		case 0x13:	/* PD */
			WP( UPD7810_PORTD, RP( UPD7810_PORTD ) | (1 << bit));
			break;
		case 0x15:	/* PF */
			WP( UPD7810_PORTF, RP( UPD7810_PORTF ) | (1 << bit));
			break;
		case 0x16:	/* MKH */
			MKH |= (1 << bit);
			break;
		case 0x17:	/* MKL */
			MKL |= (1 << bit);
			break;
		case 0x19:	/* SMH */
			SMH |= (1 << bit);
			break;
		case 0x1b:	/* EOM */
			EOM |= (1 << bit);
			break;
		case 0x1d:	/* TMM */
			TMM |= (1 << bit);
			break;
//      case 0x1e:  /* PT */
//          PT is input only
//          break;
		default:
			logerror("uPD7810 #%d: illegal opcode %02x %02x at PC:%04x\n", cpu_getactivecpu(), OP, imm, PC);
			break;
	}
}

/* 5b: 0101 1011 bbbb bbbb (7807 only) */
static void CLR(void)
{
	UINT8 imm;
	int bit;

	RDOPARG( imm );
	bit = imm >> 5;

	switch( imm & 0x1f )
	{
		case 0x10:	/* PA */
			WP( UPD7810_PORTA, RP( UPD7810_PORTA ) & ~(1 << bit));
			break;
		case 0x11:	/* PB */
			WP( UPD7810_PORTB, RP( UPD7810_PORTB ) & ~(1 << bit));
			break;
		case 0x12:	/* PC */
			WP( UPD7810_PORTC, RP( UPD7810_PORTC ) & ~(1 << bit));
			break;
		case 0x13:	/* PD */
			WP( UPD7810_PORTD, RP( UPD7810_PORTD ) & ~(1 << bit));
			break;
		case 0x15:	/* PF */
			WP( UPD7810_PORTF, RP( UPD7810_PORTF ) & ~(1 << bit));
			break;
		case 0x16:	/* MKH */
			MKH &= ~(1 << bit);
			break;
		case 0x17:	/* MKL */
			MKL &= ~(1 << bit);
			break;
		case 0x19:	/* SMH */
			SMH &= ~(1 << bit);
			break;
		case 0x1b:	/* EOM */
			EOM &= ~(1 << bit);
			break;
		case 0x1d:	/* TMM */
			TMM &= ~(1 << bit);
			break;
//      case 0x1e:  /* PT */
//          PT is input only
//          break;
		default:
			logerror("uPD7810 #%d: illegal opcode %02x %02x at PC:%04x\n", cpu_getactivecpu(), OP, imm, PC);
			break;
	}
}

/* 5d: 0101 1111 bbbb bbbb (7807 only) */
static void SK_bit(void)
{
	UINT8 imm;
	int val;

	RDOPARG( imm );

	switch( imm & 0x1f )
	{
		case 0x10:	/* PA */
			val = RP( UPD7810_PORTA );
			break;
		case 0x11:	/* PB */
			val = RP( UPD7810_PORTB );
			break;
		case 0x12:	/* PC */
			val = RP( UPD7810_PORTC );
			break;
		case 0x13:	/* PD */
			val = RP( UPD7810_PORTD );
			break;
		case 0x15:	/* PF */
			val = RP( UPD7810_PORTF );
			break;
		case 0x16:	/* MKH */
			val = MKH;
			break;
		case 0x17:	/* MKL */
			val = MKL;
			break;
		case 0x19:	/* SMH */
			val = SMH;
			break;
		case 0x1b:	/* EOM */
			val = EOM;
			break;
		case 0x1d:	/* TMM */
			val = TMM;
			break;
		case 0x1e:	/* PT */
			val = RP( UPD7807_PORTT );
			break;
		default:
			logerror("uPD7810 #%d: illegal opcode %02x %02x at PC:%04x\n", cpu_getactivecpu(), OP, imm, PC);
			val = 0;
			break;
	}

	if (val & (1 << (imm >> 5)))
		PSW |= SK;
}

/* 60:*/
static void PRE_60(void)
{
	RDOP(OP2);
	upd7810_icount -= op60[OP2].cycles;
	upd7810_timers(op60[OP2].cycles);
	(*op60[OP2].opfunc)();
}

/* 61: 0110 0001 */
static void DAA(void)
{
	UINT8 l = A & 0x0f, h = A >> 4, tmp, adj = 0x00;
	if (0 == (PSW & HC))
	{
		if (l < 10)
		{
		    if (!(h < 10 && 0 == (PSW & CY)))
			adj = 0x60;
		}
		else
		{
			if (h < 9 && 0 == (PSW & CY))
				adj = 0x06;
			else
				adj = 0x66;
		}
	}
	else
	if (l < 3)
	{
		if (h < 10 && 0 == (PSW & CY))
			adj = 0x06;
		else
			adj = 0x66;
	}
	tmp = A + adj;
	ZHC_ADD( tmp, A, PSW & CY );
	A = tmp;
}

/* 62: 0110 0010 */
static void RETI(void)
{
	PCL = RM( SPD );
	SP++;
	PCH = RM( SPD );
	SP++;
	PSW = RM( SPD );
	SP++;
	change_pc( PCD );
}

/* 63: 0110 0011 oooo oooo */
static void STAW_wa(void)
{
	PAIR ea = upd7810.va;

	RDOPARG( ea.b.l );

	WM( ea.d, A );
}

/* 64: prefix */
static void PRE_64(void)
{
	RDOP(OP2);
	upd7810_icount -= op64[OP2].cycles;
	upd7810_timers(op64[OP2].cycles);
	(*op64[OP2].opfunc)();
}

/* 65: 0110 0101 oooo oooo xxxx xxxx */
static void NEIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp, m, imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );
	m = RM( ea.d );
	tmp = m - imm;
	ZHC_SUB( tmp, m, 0 );
	SKIP_NZ;
}

/* 66: 0110 0110 xxxx xxxx */
/* SUI_A_xx already defined (long form) */

/* 67: 0110 0111 xxxx xxxx */
/* NEI_A_xx already defined (long form) */

/* 68: 0110 1000 xxxx xxxx */
static void MVI_V_xx(void)
{
	RDOPARG( V );
}

/* 69: 0110 1001 xxxx xxxx */
static void MVI_A_xx(void)
{
	if (PSW & L1) {	/* overlay active? */
		PC++;
		return; 	/* NOP */
	}
	RDOPARG( A );
	PSW |= L1;
}

/* 6a: 0110 1010 xxxx xxxx */
static void MVI_B_xx(void)
{
	RDOPARG( B );
}

/* 6b: 0110 1011 xxxx xxxx */
static void MVI_C_xx(void)
{
	RDOPARG( C );
}

/* 6c: 0110 1100 xxxx xxxx */
static void MVI_D_xx(void)
{
	RDOPARG( D );
}

/* 6d: 0110 1101 xxxx xxxx */
static void MVI_E_xx(void)
{
	RDOPARG( E );
}

/* 6e: 0110 1110 xxxx xxxx */
static void MVI_H_xx(void)
{
	RDOPARG( H );
}

/* 6f: 0110 1111 xxxx xxxx */
static void MVI_L_xx(void)
{
	if (PSW & L0) {	/* overlay active? */
		PC++;
		return; 	/* NOP */
	}
	RDOPARG( L );
	PSW |= L0;
}

/* 70: prefix */
static void PRE_70(void)
{
	RDOP(OP2);
	upd7810_icount -= op70[OP2].cycles;
	upd7810_timers(op70[OP2].cycles);
	(*op70[OP2].opfunc)();
}

/* 71: 0111 0001 oooo oooo xxxx xxxx */
static void MVIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );

	WM( ea.d, imm );
}

/* 72: 0111 0010 */
static void SOFTI(void)
{
	SP--;
	WM( SPD, PSW );
	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PC = 0x0060;
	change_pc( PCD );
}

/* 74: prefix */
static void PRE_74(void)
{
	RDOP(OP2);
	upd7810_icount -= op74[OP2].cycles;
	upd7810_timers(op74[OP2].cycles);
	(*op74[OP2].opfunc)();
}

/* 75: 0111 0101 oooo oooo xxxx xxxx */
static void EQIW_wa_xx(void)
{
	PAIR ea = upd7810.va;
	UINT8 tmp, m, imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );
	m = RM( ea.d );
	tmp = m - imm;
	ZHC_SUB( tmp, m, 0 );
	SKIP_Z;
}

/* 76: 0111 0110 xxxx xxxx */
/* SBI_A_xx already defined (long form) */

/* 77: 0111 0111 xxxx xxxx */
/* EQI_A_xx already defined (long form) */

/* 78: 0111 1ddd dddd dddd */
static void CALF(void)
{
	PAIR w;
	w.d = 0;

	RDOPARG( w.b.l );
	w.b.h = 0x08 + (OP & 0x07);

	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PCD = w.d;
	change_pc( PCD );
}

/* 80: 100t tttt */
static void CALT(void)
{
	PAIR w;
	w.d = 0;

	switch (upd7810.config.type) {
	case TYPE_7810_GAMEMASTER:
	    logerror ("!!!!!!!%.4x calt %.2x game master table position not known\n",PPC, OP);
	    break;
	default:
	w.w.l = 0x80 + 2 * (OP & 0x1f);
	}

	if (upd7810.config.type!=TYPE_7810_GAMEMASTER) {
	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	    PCL=RM(w.w.l);
	    PCH=RM(w.w.l+1);

	change_pc( PCD );
	    logerror ("!!!!!!!%.4x calt %.2x %.4x; game master table position not known\n",PPC, OP, PCD);
	}
}

/* a0: 1010 0000 */
static void POP_VA(void)
{
	A = RM( SPD );
	SP++;
	V = RM( SPD );
	SP++;
}

/* a1: 1010 0001 */
static void POP_BC(void)
{
	C = RM( SPD );
	SP++;
	B = RM( SPD );
	SP++;
}

/* a2: 1010 0010 */
static void POP_DE(void)
{
	E = RM( SPD );
	SP++;
	D = RM( SPD );
	SP++;
}

/* a3: 1010 0011 */
static void POP_HL(void)
{
	L = RM( SPD );
	SP++;
	H = RM( SPD );
	SP++;
}

/* a4: 1010 0100 */
static void POP_EA(void)
{
	EAL = RM( SPD );
	SP++;
	EAH = RM( SPD );
	SP++;
}

/* a5: 1010 0101 */
static void DMOV_EA_BC(void)
{
	EA = BC;
}

/* a6: 1010 0110 */
static void DMOV_EA_DE(void)
{
	EA = DE;
}

/* a7: 1010 0111 */
static void DMOV_EA_HL(void)
{
	EA = HL;
}

/* a8: 1010 1000 */
static void INX_EA(void)
{
	EA++;
}

/* a9: 1010 1001 */
static void DCX_EA(void)
{
	EA--;
}

/* aa: 1010 1010 */
static void EI(void)
{
	IFF = 1;
}

/* ab: 1010 1011 dddd dddd */
static void LDAX_D_xx(void)
{
	UINT16 ea;
	RDOPARG( ea );
	ea += DE;
	A = RM( ea );
}

/* ac: 1010 1100 */
static void LDAX_H_A(void)
{
	UINT16 ea;
	ea = HL + A;
	A = RM( ea );
}

/* ad: 1010 1101 */
static void LDAX_H_B(void)
{
	UINT16 ea;
	ea = HL + B;
	A = RM( ea );
}

/* ae: 1010 1110 */
static void LDAX_H_EA(void)
{
	UINT16 ea;
	ea = HL + EA;
	A = RM( ea );
}

/* af: 1010 1111 dddd dddd */
static void LDAX_H_xx(void)
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	A = RM( ea );
}

/* b0: 1011 0000 */
static void PUSH_VA(void)
{
	SP--;
	WM( SPD, V );
	SP--;
	WM( SPD, A );
}

/* b1: 1011 0001 */
static void PUSH_BC(void)
{
	SP--;
	WM( SPD, B );
	SP--;
	WM( SPD, C );
}

/* b2: 1011 0010 */
static void PUSH_DE(void)
{
	SP--;
	WM( SPD, D );
	SP--;
	WM( SPD, E );
}

/* b3: 1011 0011 */
static void PUSH_HL(void)
{
	SP--;
	WM( SPD, H );
	SP--;
	WM( SPD, L );
}

/* b4: 1011 0100 */
static void PUSH_EA(void)
{
	SP--;
	WM( SPD, EAH );
	SP--;
	WM( SPD, EAL );
}

/* b5: 1011 0101 */
static void DMOV_BC_EA(void)
{
	BC = EA;
}

/* b6: 1011 0110 */
static void DMOV_DE_EA(void)
{
	DE = EA;
}

/* b7: 1011 0111 */
static void DMOV_HL_EA(void)
{
	HL = EA;
}

/* b8: 1011 1000 */
static void RET(void)
{
	PCL = RM( SPD );
	SP++;
	PCH = RM( SPD );
	SP++;
	change_pc( PCD );
}

/* b9: 1011 1001 */
static void RETS(void)
{
	PCL = RM( SPD );
	SP++;
	PCH = RM( SPD );
	SP++;
	PSW|=SK;	/* skip one instruction */
	change_pc( PCD );
}

/* ba: 1011 1010 */
static void DI(void)
{
	IFF = 0;
}

/* bb: 1011 1011 dddd dddd */
static void STAX_D_xx(void)
{
	UINT16 ea;
	RDOPARG(ea);
	ea += DE;
	WM( ea, A );
}

/* bc: 1011 1100 */
static void STAX_H_A(void)
{
	UINT16 ea = A;
	ea += HL;
	WM( ea, A );
}

/* bd: 1011 1101 */
static void STAX_H_B(void)
{
	UINT16 ea = B;
	ea += HL;
	WM( ea, A );
}

/* be: 1011 1110 */
static void STAX_H_EA(void)
{
	UINT16 ea = EA;
	ea += HL;
	WM( ea, A );
}

/* bf: 1011 1111 dddd dddd */
static void STAX_H_xx(void)
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	WM( ea, A );
}

/* c0: 11dd dddd */
static void JR(void)
{
	INT8 offs = (INT8)(OP << 2) >> 2;
	PC += offs;
	change_pc(PCD);
}

