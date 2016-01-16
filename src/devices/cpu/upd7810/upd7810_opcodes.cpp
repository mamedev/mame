// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   Portable uPD7810/11, 7810H/11H, 78C10/C11/C14 emulator V0.2
 *
 *   7810ops.inc  - opcode functions
 *
 *****************************************************************************/

#include "emu.h"
#include "upd7810.h"
#include "upd7810_macros.h"

void upd7810_device::illegal()
{
	logerror("uPD7810 '%s': illegal opcode %02x at PC:%04x\n", tag().c_str(), OP, PC);
}

void upd7810_device::illegal2()
{
	logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", tag().c_str(), OP, OP2, PC);
}

/* prefix 48 */

/* 48 01: 0100 1000 0000 0001 */
void upd7810_device::SLRC_A()
{
	PSW = (PSW & ~CY) | (A & CY);
	A >>= 1;
	SKIP_CY;
}

/* 48 02: 0100 1000 0000 0010 */
void upd7810_device::SLRC_B()
{
	PSW = (PSW & ~CY) | (B & CY);
	B >>= 1;
	SKIP_CY;
}

/* 48 03: 0100 1000 0000 0011 */
void upd7810_device::SLRC_C()
{
	PSW = (PSW & ~CY) | (C & CY);
	C >>= 1;
	SKIP_CY;
}


/* 48 05: 0100 1000 0000 0101 */
void upd7810_device::SLLC_A()
{
	PSW = (PSW & ~CY) | ((A >> 7) & CY);
	A <<= 1;
	SKIP_CY;
}

/* 48 06: 0100 1000 0000 0110 */
void upd7810_device::SLLC_B()
{
	PSW = (PSW & ~CY) | ((B >> 7) & CY);
	B <<= 1;
	SKIP_CY;
}

/* 48 07: 0100 1000 0000 0111 */
void upd7810_device::SLLC_C()
{
	PSW = (PSW & ~CY) | ((C >> 7) & CY);
	C <<= 1;
	SKIP_CY;
}

/* 48 08: 0100 1000 0000 1000 */
void upd7810_device::SK_NV()
{
	/* 48 skip never */
}

/* 48 0a: 0100 1000 0000 1010 */
void upd7810_device::SK_CY()
{
	if (CY == (PSW & CY))
		PSW |= SK;
}

/* 48 0b: 0100 1000 0000 1011 */
void upd7810_device::SK_HC()
{
	if (HC == (PSW & HC))
		PSW |= SK;
}

/* 48 0c: 0100 1000 0000 1100 */
void upd7810_device::SK_Z()
{
	if (Z == (PSW & Z))
		PSW |= SK;
}

/* 48 18: 0100 1000 0001 1000 */
void upd7810_device::SKN_NV()
{
	/* skip not never -> skip always ;-) */
	PSW |= SK;
}

/* 48 1a: 0100 1000 0001 1010 */
void upd7810_device::SKN_CY()
{
	if (0 == (PSW & CY))
		PSW |= SK;
}

/* 48 1b: 0100 1000 0001 1011 */
void upd7810_device::SKN_HC()
{
	if (0 == (PSW & HC))
		PSW |= SK;
}

/* 48 1c: 0100 1000 0001 1100 */
void upd7810_device::SKN_Z()
{
	if (0 == (PSW & Z))
		PSW |= SK;
}

/* 48 21: 0100 1000 0010 0001 */
void upd7810_device::SLR_A()
{
	PSW = (PSW & ~CY) | (A & CY);
	A >>= 1;
}

/* 48 22: 0100 1000 0010 0010 */
void upd7810_device::SLR_B()
{
	PSW = (PSW & ~CY) | (B & CY);
	B >>= 1;
}

/* 48 23: 0100 1000 0010 0011 */
void upd7810_device::SLR_C()
{
	PSW = (PSW & ~CY) | (C & CY);
	C >>= 1;
}

/* 48 25: 0100 1000 0010 0101 */
void upd7810_device::SLL_A()
{
	PSW = (PSW & ~CY) | ((A >> 7) & CY);
	A <<= 1;
}

/* 48 26: 0100 1000 0010 0110 */
void upd7810_device::SLL_B()
{
	PSW = (PSW & ~CY) | ((B >> 7) & CY);
	B <<= 1;
}

/* 48 27: 0100 1000 0010 0111 */
void upd7810_device::SLL_C()
{
	PSW = (PSW & ~CY) | ((C >> 7) & CY);
	C <<= 1;
}

/* 48 28: 0100 1000 0010 1000 */
void upd7810_device::JEA()
{
	PC = EA;
}

/* 48 29: 0100 1000 0010 1001 */
void upd7810_device::CALB()
{
	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PC = BC;
}

/* 48 2a: 0100 1000 0010 1010 */
void upd7810_device::CLC()
{
	PSW &= ~CY;
}

/* 48 2b: 0100 1000 0010 1011 */
void upd7810_device::STC()
{
	PSW |= CY;
}

/* 48 2d: 0100 1000 0010 1101 */
void upd7810_device::MUL_A()
{
	EA = A * A;
}

/* 48 2e: 0100 1000 0010 1110 */
void upd7810_device::MUL_B()
{
	EA = A * B;
}

/* 48 2f: 0100 1000 0010 1111 */
void upd7810_device::MUL_C()
{
	EA = A * C;
}

/* 48 31: 0100 1000 0011 0001 */
void upd7810_device::RLR_A()
{
	UINT8 carry=(PSW&CY)<<7;
	PSW = (PSW & ~CY) | (A & CY);
	A = (A >> 1) | carry;
}

/* 48 32: 0100 1000 0011 0010 */
void upd7810_device::RLR_B()
{
	UINT8 carry=(PSW&CY)<<7;
	PSW = (PSW & ~CY) | (B & CY);
	B = (B >> 1) | carry;
}

/* 48 33: 0100 1000 0011 0011 */
void upd7810_device::RLR_C()
{
	UINT8 carry=(PSW&CY)<<7;
	PSW = (PSW & ~CY) | (C & CY);
	C = (C >> 1) | carry;
}

/* 48 35: 0100 1000 0011 0101 */
void upd7810_device::RLL_A()
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((A >> 7) & CY);
	A = (A << 1) | carry;
}

/* 48 36: 0100 1000 0011 0110 */
void upd7810_device::RLL_B()
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((B >> 7) & CY);
	B = (B << 1) | carry;
}

/* 48 37: 0100 1000 0011 0111 */
void upd7810_device::RLL_C()
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((C >> 7) & CY);
	C = (C << 1) | carry;
}

/* 48 38: 0100 1000 0011 1000 */
void upd7810_device::RLD()
{
	UINT8 m = RM( HL ), tmp;
	tmp = (m << 4) | (A & 0x0f);
	A = (A & 0xf0) | (m >> 4);
	WM( HL, tmp );
}

/* 48 39: 0100 1000 0011 1001 */
void upd7810_device::RRD()
{
	UINT8 m = RM( HL ), tmp;
	tmp = (A << 4) | (m >> 4);
	A = (A & 0xf0) | (m & 0x0f);
	WM( HL, tmp );
}

/* 48 3a: 0100 1000 0011 1010 */
void upd7810_device::NEGA()
{
	A = ~A + 1;
}

/* 48 3b: 0100 1000 0011 1011 */
void upd7810_device::HALT()
{
	int cycles = (m_icount / 4) * 4;
	m_icount -= cycles;
	handle_timers(cycles);
	PC -= 1;        /* continue executing HALT */
}

/* 48 3d: 0100 1000 0011 1101 */
void upd7810_device::DIV_A()
{
	if (A)
	{
		UINT8 remainder;
		remainder = EA % A;
		EA /= A;
		A = remainder;
	}
	else
		EA = 0xffff;    /* guess */
}

/* 48 3e: 0100 1000 0011 1110 */
void upd7810_device::DIV_B()
{
	if (B)
	{
		UINT8 remainder;
		remainder = EA % B;
		EA /= B;
		B = remainder;
	}
	else
		EA = 0xffff;    /* guess */
}

/* 48 3f: 0100 1000 0011 1111 */
void upd7810_device::DIV_C()
{
	if (C)
	{
		UINT8 remainder;
		remainder = EA % C;
		EA /= C;
		C = remainder;
	}
	else
		EA = 0xffff;    /* guess */
}

/* 48 40: 0100 1000 0100 0000 */
void upd7810_device::SKIT_NMI()
{
	if (IRR & INTNMI)
		PSW |= SK;
	IRR &= ~INTNMI;
}

/* 48 41: 0100 1000 0100 0001 */
void upd7810_device::SKIT_FT0()
{
	if (IRR & INTFT0)
		PSW |= SK;
	IRR &= ~INTFT0;
}

/* 48 42: 0100 1000 0100 0010 */
void upd7810_device::SKIT_FT1()
{
	if (IRR & INTFT1)
		PSW |= SK;
	IRR &= ~INTFT1;
}

/* 48 43: 0100 1000 0100 0011 */
void upd7810_device::SKIT_F1()
{
	if (IRR & INTF1)
		PSW |= SK;
	IRR &= ~INTF1;
}

/* 48 44: 0100 1000 0100 0100 */
void upd7810_device::SKIT_F2()
{
	if (IRR & INTF2)
		PSW |= SK;
	IRR &= ~INTF2;
}

/* 48 45: 0100 1000 0100 0101 */
void upd7810_device::SKIT_FE0()
{
	if (IRR & INTFE0)
		PSW |= SK;
	IRR &= ~INTFE0;
}

/* 48 46: 0100 1000 0100 0110 */
void upd7810_device::SKIT_FE1()
{
	if (IRR & INTFE1)
		PSW |= SK;
	IRR &= ~INTFE1;
}

/* 48 47: 0100 1000 0100 0111 */
void upd7810_device::SKIT_FEIN()
{
	if (IRR & INTFEIN)
		PSW |= SK;
	IRR &= ~INTFEIN;
}

/* 48 48: 0100 1000 0100 1000 */
void upd7810_device::SKIT_FAD()
{
	if (IRR & INTFAD)
		PSW |= SK;
	IRR &= ~INTFAD;
}

/* 48 49: 0100 1000 0100 1001 */
void upd7810_device::SKIT_FSR()
{
	if (IRR & INTFSR)
		PSW |= SK;
	IRR &= ~INTFSR;
}

/* 48 4a: 0100 1000 0100 1010 */
void upd7810_device::SKIT_FST()
{
	if (IRR & INTFST)
		PSW |= SK;
	IRR &= ~INTFST;
}

/* 48 4b: 0100 1000 0100 1011 */
void upd7810_device::SKIT_ER()
{
	if (IRR & INTER)
		PSW |= SK;
	IRR &= ~INTER;
}

/* 48 4c: 0100 1000 0100 1100 */
void upd7810_device::SKIT_OV()
{
	if (IRR & INTOV)
		PSW |= SK;
	IRR &= ~INTOV;
}

/* 48 50: 0100 1000 0101 0000 */
void upd7810_device::SKIT_AN4()
{
	if (ITF & INTAN4)
		PSW |= SK;
	ITF &= ~INTAN4;
}

/* 48 51: 0100 1000 0101 0001 */
void upd7810_device::SKIT_AN5()
{
	if (ITF & INTAN5)
		PSW |= SK;
	ITF &= ~INTAN5;
}

/* 48 52: 0100 1000 0101 0010 */
void upd7810_device::SKIT_AN6()
{
	if (ITF & INTAN6)
		PSW |= SK;
	ITF &= ~INTAN6;
}

/* 48 53: 0100 1000 0101 0011 */
void upd7810_device::SKIT_AN7()
{
	if (ITF & INTAN7)
		PSW |= SK;
	ITF &= ~INTAN7;
}

/* 48 54: 0100 1000 0101 0100 */
void upd7810_device::SKIT_SB()
{
	if (ITF & INTSB)
		PSW |= SK;
	ITF &= ~INTSB;
}

/* 48 60: 0100 1000 0110 0000 */
void upd7810_device::SKNIT_NMI()
{
	if (0 == (IRR & INTNMI))
		PSW |= SK;
	IRR &= ~INTNMI;
}

/* 48 61: 0100 1000 0110 0001 */
void upd7810_device::SKNIT_FT0()
{
	if (0 == (IRR & INTFT0))
		PSW |= SK;
	IRR &= ~INTFT0;
}

/* 48 62: 0100 1000 0110 0010 */
void upd7810_device::SKNIT_FT1()
{
	if (0 == (IRR & INTFT1))
		PSW |= SK;
	IRR &= ~INTFT1;
}

/* 48 63: 0100 1000 0110 0011 */
void upd7810_device::SKNIT_F1()
{
	if (0 == (IRR & INTF1))
		PSW |= SK;
	IRR &= ~INTF1;
}

/* 48 64: 0100 1000 0110 0100 */
void upd7810_device::SKNIT_F2()
{
	if (0 == (IRR & INTF2))
		PSW |= SK;
	IRR &= ~INTF2;
}

/* 48 65: 0100 1000 0110 0101 */
void upd7810_device::SKNIT_FE0()
{
	if (0 == (IRR & INTFE0))
		PSW |= SK;
	IRR &= ~INTFE0;
}

/* 48 66: 0100 1000 0110 0110 */
void upd7810_device::SKNIT_FE1()
{
	if (0 == (IRR & INTFE1))
		PSW |= SK;
	IRR &= ~INTFE1;
}

/* 48 67: 0100 1000 0110 0111 */
void upd7810_device::SKNIT_FEIN()
{
	if (0 == (IRR & INTFEIN))
		PSW |= SK;
	IRR &= ~INTFEIN;
}

/* 48 68: 0100 1000 0110 1000 */
void upd7810_device::SKNIT_FAD()
{
	if (0 == (IRR & INTFAD))
		PSW |= SK;
	IRR &= ~INTFAD;
}

/* 48 69: 0100 1000 0110 1001 */
void upd7810_device::SKNIT_FSR()
{
	if (0 == (IRR & INTFSR))
		PSW |= SK;
	IRR &= ~INTFSR;
}

/* 48 6a: 0100 1000 0110 1010 */
void upd7810_device::SKNIT_FST()
{
	if (0 == (IRR & INTFST))
		PSW |= SK;
	IRR &= ~INTFST;
}

/* 48 6b: 0100 1000 0110 1011 */
void upd7810_device::SKNIT_ER()
{
	if (0 == (IRR & INTER))
		PSW |= SK;
	IRR &= ~INTER;
}

/* 48 6c: 0100 1000 0110 1100 */
void upd7810_device::SKNIT_OV()
{
	if (0 == (IRR & INTOV))
		PSW |= SK;
	IRR &= ~INTOV;
}

/* 48 70: 0100 1000 0111 0000 */
void upd7810_device::SKNIT_AN4()
{
	if (0 == (ITF & INTAN4))
		PSW |= SK;
	ITF &= ~INTAN4;
}

/* 48 71: 0100 1000 0111 0001 */
void upd7810_device::SKNIT_AN5()
{
	if (0 == (ITF & INTAN5))
		PSW |= SK;
	ITF &= ~INTAN5;
}

/* 48 72: 0100 1000 0111 0010 */
void upd7810_device::SKNIT_AN6()
{
	if (0 == (ITF & INTAN6))
		PSW |= SK;
	ITF &= ~INTAN6;
}

/* 48 73: 0100 1000 0111 0011 */
void upd7810_device::SKNIT_AN7()
{
	if (0 == (ITF & INTAN7))
		PSW |= SK;
	ITF &= ~INTAN7;
}

/* 48 74: 0100 1000 0111 0100 */
void upd7810_device::SKNIT_SB()
{
	if (0 == (ITF & INTSB))
		PSW |= SK;
	ITF &= ~INTSB;
}

/* 48 82: 0100 1000 1000 0010 */
void upd7810_device::LDEAX_D()
{
	EAL = RM( DE );
	EAH = RM( DE + 1 );
}

/* 48 83: 0100 1000 1000 0011 */
void upd7810_device::LDEAX_H()
{
	EAL = RM( HL );
	EAH = RM( HL + 1 );
}

/* 48 84: 0100 1000 1000 0100 */
void upd7810_device::LDEAX_Dp()
{
	EAL = RM( DE );
	EAH = RM( DE + 1 );
	DE += 2;
}

/* 48 85: 0100 1000 1000 0101 */
void upd7810_device::LDEAX_Hp()
{
	EAL = RM( HL );
	EAH = RM( HL + 1 );
	HL += 2;
}

/* 48 8b: 0100 1000 1000 1011 xxxx xxxx */
void upd7810_device::LDEAX_D_xx()
{
	UINT16 ea;
	RDOPARG( ea );
	ea += DE;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8c: 0100 1000 1000 1100 */
void upd7810_device::LDEAX_H_A()
{
	UINT16 ea = HL + A;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8d: 0100 1000 1000 1101 */
void upd7810_device::LDEAX_H_B()
{
	UINT16 ea = HL + B;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8e: 0100 1000 1000 1110 */
void upd7810_device::LDEAX_H_EA()
{
	UINT16 ea = HL + EA;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 8f: 0100 1000 1000 1111 xxxx xxxx */
void upd7810_device::LDEAX_H_xx()
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	EAL = RM( ea );
	EAH = RM( ea + 1 );
}

/* 48 92: 0100 1000 1000 0010 */
void upd7810_device::STEAX_D()
{
	WM( DE, EAL );
	WM( DE + 1, EAH );
}

/* 48 93: 0100 1000 1000 0011 */
void upd7810_device::STEAX_H()
{
	WM( HL, EAL );
	WM( HL + 1, EAH );
}

/* 48 94: 0100 1000 1000 0100 */
void upd7810_device::STEAX_Dp()
{
	WM( DE, EAL );
	WM( DE + 1, EAH );
	DE += 2;
}

/* 48 95: 0100 1000 1000 0101 */
void upd7810_device::STEAX_Hp()
{
	WM( HL, EAL );
	WM( HL + 1, EAH );
	HL += 2;
}

/* 48 9b: 0100 1000 1000 1011 xxxx xxxx */
void upd7810_device::STEAX_D_xx()
{
	UINT16 ea;
	RDOPARG( ea );
	ea += DE;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9c: 0100 1000 1000 1100 */
void upd7810_device::STEAX_H_A()
{
	UINT16 ea = HL + A;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9d: 0100 1000 1000 1101 */
void upd7810_device::STEAX_H_B()
{
	UINT16 ea = HL + B;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9e: 0100 1000 1000 1110 */
void upd7810_device::STEAX_H_EA()
{
	UINT16 ea = HL + EA;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 9f: 0100 1000 1000 1111 xxxx xxxx */
void upd7810_device::STEAX_H_xx()
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	WM( ea, EAL );
	WM( ea + 1, EAH );
}

/* 48 a0: 0100 1000 1010 0000 */
void upd7810_device::DSLR_EA()
{
	PSW = (PSW & ~CY) | (EA & CY);
	EA >>= 1;
}

/* 48 a4: 0100 1000 1010 0100 */
void upd7810_device::DSLL_EA()
{
	PSW = (PSW & ~CY) | ((EA >> 15) & CY);
	EA <<= 1;
}

/* 48 a8: 0100 1000 1010 1000 */
void upd7810_device::TABLE()
{
	UINT16 ea = PC + A + 1;
	C = RM( ea );
	B = RM( ea + 1 );
}

/* 48 b0: 0100 1000 1011 0000 */
void upd7810_device::DRLR_EA()
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | (EA & CY);
	EA = (EA >> 1) | (carry << 15);
}

/* 48 b4: 0100 1000 1011 0100 */
void upd7810_device::DRLL_EA()
{
	UINT8 carry=PSW&CY;
	PSW = (PSW & ~CY) | ((EA >> 15) & CY);
	EA = (EA << 1) | carry;
}

/* 48 bb: 0100 1000 1011 1011 */
void upd7810_device::STOP()
{
	int cycles = (m_icount / 4) * 4;
	m_icount -= cycles;
	handle_timers(cycles);
	PC -= 1;
}

/* 48 c0: 0100 1000 1100 0000 */
void upd7810_device::DMOV_EA_ECNT()
{
	EA = ECNT;
}

/* 48 c1: 0100 1000 1100 0001 */
void upd7810_device::DMOV_EA_ECPT()
{
	EA = ECPT;
}

/* 48 d2: 0100 1000 1101 0010 */
void upd7810_device::DMOV_ETM0_EA()
{
	ETM0 = EA;
}

/* 48 d3: 0100 1000 1101 0011 */
void upd7810_device::DMOV_ETM1_EA()
{
	ETM1 = EA;
}

/* prefix 4C */
/* 4c c0: 0100 1100 1100 0000 */
void upd7810_device::MOV_A_PA()
{
	A = RP( UPD7810_PORTA );
}

/* 4c c1: 0100 1100 1100 0001 */
void upd7810_device::MOV_A_PB()
{
	A = RP( UPD7810_PORTB );
}

/* 4c c2: 0100 1100 1100 0010 */
void upd7810_device::MOV_A_PC()
{
	A = RP( UPD7810_PORTC );
}

/* 4c c3: 0100 1100 1100 0011 */
void upd7810_device::MOV_A_PD()
{
	A = RP( UPD7810_PORTD );
}

/* 4c c5: 0100 1100 1100 0101 */
void upd7810_device::MOV_A_PF()
{
	A = RP( UPD7810_PORTF );
}

/* 4c c6: 0100 1100 1100 0110 */
void upd7810_device::MOV_A_MKH()
{
	A = MKH;
}

/* 4c c7: 0100 1100 1100 0111 */
void upd7810_device::MOV_A_MKL()
{
	A = MKL;
}

/* 4c c8: 0100 1100 1100 1000 */
void upd7810_device::MOV_A_ANM()
{
	A = ANM;
}

/* 4c c9: 0100 1100 1100 1001 */
void upd7810_device::MOV_A_SMH()
{
	A = SMH;
}

/* 4c cb: 0100 1100 1100 1011 */
void upd7810_device::MOV_A_EOM()
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	A = eom;
}

/* 4c cd: 0100 1100 1100 1101 */
void upd7810_device::MOV_A_TMM()
{
	A = TMM;
}

/* 4c ce: 0100 1100 1110 0000 (7807 only) */
void upd7810_device::MOV_A_PT()
{
	A = RP( UPD7807_PORTT );
}

/* 4c d9: 0100 1100 1101 1001 */
void upd7810_device::MOV_A_RXB()
{
	A = RXB;
}

/* 4c e0: 0100 1100 1110 0000 */
void upd7810_device::MOV_A_CR0()
{
	A = CR0;
}

/* 4c e1: 0100 1100 1110 0001 */
void upd7810_device::MOV_A_CR1()
{
	A = CR1;
}

/* 4c e2: 0100 1100 1110 0010 */
void upd7810_device::MOV_A_CR2()
{
	A = CR2;
}

/* 4c e3: 0100 1100 1110 0011 */
void upd7810_device::MOV_A_CR3()
{
	A = CR3;
}

/* prefix 4D */
/* 4d c0: 0100 1101 1100 0000 */
void upd7810_device::MOV_PA_A()
{
	WP( UPD7810_PORTA, A );
}

/* 4d c1: 0100 1101 1100 0001 */
void upd7810_device::MOV_PB_A()
{
	WP( UPD7810_PORTB, A );
}

/* 4d c2: 0100 1101 1100 0010 */
void upd7810_device::MOV_PC_A()
{
	WP( UPD7810_PORTC, A );
}

/* 4d c3: 0100 1101 1100 0011 */
void upd7810_device::MOV_PD_A()
{
	WP( UPD7810_PORTD, A );
}

/* 4d c5: 0100 1101 1100 0101 */
void upd7810_device::MOV_PF_A()
{
	WP( UPD7810_PORTF, A );
}

/* 4d c6: 0100 1101 1100 0110 */
void upd7810_device::MOV_MKH_A()
{
	MKH = A;
}

/* 4d c7: 0100 1101 1100 0111 */
void upd7810_device::MOV_MKL_A()
{
	MKL = A;
}

/* 4d c8: 0100 1101 1100 1000 */
void upd7810_device::MOV_ANM_A()
{
	ANM = A;
}

/* 4d c9: 0100 1101 1100 1001 */
void upd7810_device::MOV_SMH_A()
{
	SMH = A;
}

/* 4d ca: 0100 1101 1100 1010 */
void upd7810_device::MOV_SML_A()
{
	SML = A;
}

/* 4d cb: 0100 1101 1100 1011 */
void upd7810_device::MOV_EOM_A()
{
	EOM = A;
	upd7810_write_EOM();
}

/* 4d cc: 0100 1101 1100 1100 */
void upd7810_device::MOV_ETMM_A()
{
	ETMM = A;
}

/* 4d cd: 0100 1101 1100 1101 */
void upd7810_device::MOV_TMM_A()
{
	TMM = A;
}

/* 4d d0: 0100 1101 1101 0000 */
void upd7810_device::MOV_MM_A()
{
	MM = A;
}

/* 4d d1: 0100 1101 1101 0001 */
void upd7810_device::MOV_MCC_A()
{
	MCC = A;
}

/* 4d d2: 0100 1101 1101 0010 */
void upd7810_device::MOV_MA_A()
{
	MA = A;
}

/* 4d d3: 0100 1101 1101 0011 */
void upd7810_device::MOV_MB_A()
{
	MB = A;
}

/* 4d d4: 0100 1101 1101 0100 */
void upd7810_device::MOV_MC_A()
{
	MC = A;
}

/* 4d d7: 0100 1101 1101 0111 */
void upd7810_device::MOV_MF_A()
{
	MF = A;
}

/* 4d d8: 0100 1101 1101 1000 */
void upd7810_device::MOV_TXB_A()
{
	TXB = A;
	upd7810_write_TXB();
}

/* 4d da: 0100 1101 1101 1010 */
void upd7810_device::MOV_TM0_A()
{
	TM0 = A;
}

/* 4d db: 0100 1101 1101 1011 */
void upd7810_device::MOV_TM1_A()
{
	TM1 = A;
}

/* 4d e8: 0100 1101 1110 1000 */
void upd7810_device::MOV_ZCM_A()
{
	ZCM = A;
}

/* prefix 60 */
/* 60 08: 0110 0000 0000 1000 */
void upd7810_device::ANA_V_A()
{
	V &= A;
	SET_Z(V);
}

/* 60 09: 0110 0000 0000 1001 */
void upd7810_device::ANA_A_A()
{
	A &= A;
	SET_Z(A);
}

/* 60 0a: 0110 0000 0000 1010 */
void upd7810_device::ANA_B_A()
{
	B &= A;
	SET_Z(B);
}

/* 60 0b: 0110 0000 0000 1011 */
void upd7810_device::ANA_C_A()
{
	C &= A;
	SET_Z(C);
}

/* 60 0c: 0110 0000 0000 1100 */
void upd7810_device::ANA_D_A()
{
	D &= A;
	SET_Z(D);
}

/* 60 0d: 0110 0000 0000 1101 */
void upd7810_device::ANA_E_A()
{
	E &= A;
	SET_Z(E);
}

/* 60 0e: 0110 0000 0000 1110 */
void upd7810_device::ANA_H_A()
{
	H &= A;
	SET_Z(H);
}

/* 60 0f: 0110 0000 0000 1111 */
void upd7810_device::ANA_L_A()
{
	L &= A;
	SET_Z(L);
}

/* 60 10: 0110 0000 0001 0000 */
void upd7810_device::XRA_V_A()
{
	V ^= A;
	SET_Z(V);
}

/* 60 11: 0110 0000 0001 0001 */
void upd7810_device::XRA_A_A()
{
	A ^= A;
	SET_Z(A);
}

/* 60 12: 0110 0000 0001 0010 */
void upd7810_device::XRA_B_A()
{
	B ^= A;
	SET_Z(B);
}

/* 60 13: 0110 0000 0001 0011 */
void upd7810_device::XRA_C_A()
{
	C ^= A;
	SET_Z(C);
}

/* 60 14: 0110 0000 0001 0100 */
void upd7810_device::XRA_D_A()
{
	D ^= A;
	SET_Z(D);
}

/* 60 15: 0110 0000 0001 0101 */
void upd7810_device::XRA_E_A()
{
	E ^= A;
	SET_Z(E);
}

/* 60 16: 0110 0000 0001 0110 */
void upd7810_device::XRA_H_A()
{
	H ^= A;
	SET_Z(H);
}

/* 60 17: 0110 0000 0001 0111 */
void upd7810_device::XRA_L_A()
{
	L ^= A;
	SET_Z(L);
}

/* 60 18: 0110 0000 0001 1000 */
void upd7810_device::ORA_V_A()
{
	V |= A;
	SET_Z(V);
}

/* 60 19: 0110 0000 0001 1001 */
void upd7810_device::ORA_A_A()
{
	A |= A;
	SET_Z(A);
}

/* 60 1a: 0110 0000 0001 1010 */
void upd7810_device::ORA_B_A()
{
	B |= A;
	SET_Z(B);
}

/* 60 1b: 0110 0000 0001 1011 */
void upd7810_device::ORA_C_A()
{
	C |= A;
	SET_Z(C);
}

/* 60 1c: 0110 0000 0001 1100 */
void upd7810_device::ORA_D_A()
{
	D |= A;
	SET_Z(D);
}

/* 60 1d: 0110 0000 0001 1101 */
void upd7810_device::ORA_E_A()
{
	E |= A;
	SET_Z(E);
}

/* 60 1e: 0110 0000 0001 1110 */
void upd7810_device::ORA_H_A()
{
	H |= A;
	SET_Z(H);
}

/* 60 1f: 0110 0000 0001 1111 */
void upd7810_device::ORA_L_A()
{
	L |= A;
	SET_Z(L);
}

/* 60 20: 0110 0000 0010 0000 */
void upd7810_device::ADDNC_V_A()
{
	UINT8 tmp = V + A;
	ZHC_ADD( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 60 21: 0110 0000 0010 0001 */
void upd7810_device::ADDNC_A_A()
{
	UINT8 tmp = A + A;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 22: 0110 0000 0010 0010 */
void upd7810_device::ADDNC_B_A()
{
	UINT8 tmp = B + A;
	ZHC_ADD( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 60 23: 0110 0000 0010 0011 */
void upd7810_device::ADDNC_C_A()
{
	UINT8 tmp = C + A;
	ZHC_ADD( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 60 24: 0110 0000 0010 0100 */
void upd7810_device::ADDNC_D_A()
{
	UINT8 tmp = D + A;
	ZHC_ADD( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 60 25: 0110 0000 0010 0101 */
void upd7810_device::ADDNC_E_A()
{
	UINT8 tmp = E + A;
	ZHC_ADD( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 60 26: 0110 0000 0010 0110 */
void upd7810_device::ADDNC_H_A()
{
	UINT8 tmp = H + A;
	ZHC_ADD( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 60 27: 0110 0000 0010 0111 */
void upd7810_device::ADDNC_L_A()
{
	UINT8 tmp = L + A;
	ZHC_ADD( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 60 28: 0110 0000 0010 1000 */
void upd7810_device::GTA_V_A()
{
	UINT16 tmp = V - A - 1;
	ZHC_SUB( tmp, V, 0 );
	SKIP_NC;
}

/* 60 29: 0110 0000 0010 1001 */
void upd7810_device::GTA_A_A()
{
	UINT16 tmp = A - A - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 2a: 0110 0000 0010 1010 */
void upd7810_device::GTA_B_A()
{
	UINT16 tmp = B - A - 1;
	ZHC_SUB( tmp, B, 0 );
	SKIP_NC;
}

/* 60 2b: 0110 0000 0010 1011 */
void upd7810_device::GTA_C_A()
{
	UINT16 tmp = C - A - 1;
	ZHC_SUB( tmp, C, 0 );
	SKIP_NC;
}

/* 60 2c: 0110 0000 0010 1100 */
void upd7810_device::GTA_D_A()
{
	UINT16 tmp = D - A - 1;
	ZHC_SUB( tmp, D, 0 );
	SKIP_NC;
}

/* 60 2d: 0110 0000 0010 1101 */
void upd7810_device::GTA_E_A()
{
	UINT16 tmp = E - A - 1;
	ZHC_SUB( tmp, E, 0 );
	SKIP_NC;
}

/* 60 2e: 0110 0000 0010 1110 */
void upd7810_device::GTA_H_A()
{
	UINT16 tmp = H - A - 1;
	ZHC_SUB( tmp, H, 0 );
	SKIP_NC;
}

/* 60 2f: 0110 0000 0010 1111 */
void upd7810_device::GTA_L_A()
{
	UINT16 tmp = L - A - 1;
	ZHC_SUB( tmp, L, 0 );
	SKIP_NC;
}

/* 60 30: 0110 0000 0011 0000 */
void upd7810_device::SUBNB_V_A()
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 60 31: 0110 0000 0011 0001 */
void upd7810_device::SUBNB_A_A()
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 32: 0110 0000 0011 0010 */
void upd7810_device::SUBNB_B_A()
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 60 33: 0110 0000 0011 0011 */
void upd7810_device::SUBNB_C_A()
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 60 34: 0110 0000 0011 0100 */
void upd7810_device::SUBNB_D_A()
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 60 35: 0110 0000 0011 0101 */
void upd7810_device::SUBNB_E_A()
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 60 36: 0110 0000 0011 0110 */
void upd7810_device::SUBNB_H_A()
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 60 37: 0110 0000 0011 0111 */
void upd7810_device::SUBNB_L_A()
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 60 38: 0110 0000 0011 1000 */
void upd7810_device::LTA_V_A()
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	SKIP_CY;
}

/* 60 39: 0110 0000 0011 1001 */
void upd7810_device::LTA_A_A()
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 3a: 0110 0000 0011 1010 */
void upd7810_device::LTA_B_A()
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	SKIP_CY;
}

/* 60 3b: 0110 0000 0011 1011 */
void upd7810_device::LTA_C_A()
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	SKIP_CY;
}

/* 60 3c: 0110 0000 0011 1100 */
void upd7810_device::LTA_D_A()
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	SKIP_CY;
}

/* 60 3d: 0110 0000 0011 1101 */
void upd7810_device::LTA_E_A()
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	SKIP_CY;
}

/* 60 3e: 0110 0000 0011 1110 */
void upd7810_device::LTA_H_A()
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	SKIP_CY;
}

/* 60 3f: 0110 0000 0011 1111 */
void upd7810_device::LTA_L_A()
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	SKIP_CY;
}

/* 60 40: 0110 0000 0100 0000 */
void upd7810_device::ADD_V_A()
{
	UINT8 tmp = V + A;
	ZHC_ADD( tmp, V, 0 );
	V = tmp;
}

/* 60 41: 0110 0000 0100 0001 */
void upd7810_device::ADD_A_A()
{
	UINT8 tmp = A + A;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 42: 0110 0000 0100 0010 */
void upd7810_device::ADD_B_A()
{
	UINT8 tmp = B + A;
	ZHC_ADD( tmp, B, 0 );
	B = tmp;
}

/* 60 43: 0110 0000 0100 0011 */
void upd7810_device::ADD_C_A()
{
	UINT8 tmp = C + A;
	ZHC_ADD( tmp, C, 0 );
	C = tmp;
}

/* 60 44: 0110 0000 0100 0100 */
void upd7810_device::ADD_D_A()
{
	UINT8 tmp = D + A;
	ZHC_ADD( tmp, D, 0 );
	D = tmp;
}

/* 60 45: 0110 0000 0100 0101 */
void upd7810_device::ADD_E_A()
{
	UINT8 tmp = E + A;
	ZHC_ADD( tmp, E, 0 );
	E = tmp;
}

/* 60 46: 0110 0000 0100 0110 */
void upd7810_device::ADD_H_A()
{
	UINT8 tmp = H + A;
	ZHC_ADD( tmp, H, 0 );
	H = tmp;
}

/* 60 47: 0110 0000 0100 0111 */
void upd7810_device::ADD_L_A()
{
	UINT8 tmp = L + A;
	ZHC_ADD( tmp, L, 0 );
	L = tmp;
}

/* 60 50: 0110 0000 0101 0000 */
void upd7810_device::ADC_V_A()
{
	UINT8 tmp = V + A + (PSW & CY);
	ZHC_ADD( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 60 51: 0110 0000 0101 0001 */
void upd7810_device::ADC_A_A()
{
	UINT8 tmp = A + A + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 52: 0110 0000 0101 0010 */
void upd7810_device::ADC_B_A()
{
	UINT8 tmp = B + A + (PSW & CY);
	ZHC_ADD( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 60 53: 0110 0000 0101 0011 */
void upd7810_device::ADC_C_A()
{
	UINT8 tmp = C + A + (PSW & CY);
	ZHC_ADD( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 60 54: 0110 0000 0101 0100 */
void upd7810_device::ADC_D_A()
{
	UINT8 tmp = D + A + (PSW & CY);
	ZHC_ADD( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 60 55: 0110 0000 0101 0101 */
void upd7810_device::ADC_E_A()
{
	UINT8 tmp = E + A + (PSW & CY);
	ZHC_ADD( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 60 56: 0110 0000 0101 0110 */
void upd7810_device::ADC_H_A()
{
	UINT8 tmp = H + A + (PSW & CY);
	ZHC_ADD( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 60 57: 0110 0000 0101 0111 */
void upd7810_device::ADC_L_A()
{
	UINT8 tmp = L + A + (PSW & CY);
	ZHC_ADD( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 60 60: 0110 0000 0110 0000 */
void upd7810_device::SUB_V_A()
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
}

/* 60 61: 0110 0000 0110 0001 */
void upd7810_device::SUB_A_A()
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 62: 0110 0000 0110 0010 */
void upd7810_device::SUB_B_A()
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
}

/* 60 63: 0110 0000 0110 0011 */
void upd7810_device::SUB_C_A()
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
}

/* 60 64: 0110 0000 0110 0100 */
void upd7810_device::SUB_D_A()
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
}

/* 60 65: 0110 0000 0110 0101 */
void upd7810_device::SUB_E_A()
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
}

/* 60 66: 0110 0000 0110 0110 */
void upd7810_device::SUB_H_A()
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
}

/* 60 67: 0110 0000 0110 0111 */
void upd7810_device::SUB_L_A()
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
}

/* 60 68: 0110 0000 0110 1000 */
void upd7810_device::NEA_V_A()
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	SKIP_NZ;
}

/* 60 69: 0110 0000 0110 1001 */
void upd7810_device::NEA_A_A()
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 6a: 0110 0000 0110 1010 */
void upd7810_device::NEA_B_A()
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	SKIP_NZ;
}

/* 60 6b: 0110 0000 0110 1011 */
void upd7810_device::NEA_C_A()
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	SKIP_NZ;
}

/* 60 6c: 0110 0000 0110 1100 */
void upd7810_device::NEA_D_A()
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	SKIP_NZ;
}

/* 60 6d: 0110 0000 0110 1101 */
void upd7810_device::NEA_E_A()
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	SKIP_NZ;
}

/* 60 6e: 0110 0000 0110 1110 */
void upd7810_device::NEA_H_A()
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	SKIP_NZ;
}

/* 60 6f: 0110 0000 0110 1111 */
void upd7810_device::NEA_L_A()
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	SKIP_NZ;
}

/* 60 70: 0110 0000 0111 0000 */
void upd7810_device::SBB_V_A()
{
	UINT8 tmp = V - A - (PSW & CY);
	ZHC_SUB( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 60 71: 0110 0000 0111 0001 */
void upd7810_device::SBB_A_A()
{
	UINT8 tmp = A - A - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 72: 0110 0000 0111 0010 */
void upd7810_device::SBB_B_A()
{
	UINT8 tmp = B - A - (PSW & CY);
	ZHC_SUB( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 60 73: 0110 0000 0111 0011 */
void upd7810_device::SBB_C_A()
{
	UINT8 tmp = C - A - (PSW & CY);
	ZHC_SUB( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 60 74: 0110 0000 0111 0100 */
void upd7810_device::SBB_D_A()
{
	UINT8 tmp = D - A - (PSW & CY);
	ZHC_SUB( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 60 75: 0110 0000 0111 0101 */
void upd7810_device::SBB_E_A()
{
	UINT8 tmp = E - A - (PSW & CY);
	ZHC_SUB( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 60 76: 0110 0000 0111 0110 */
void upd7810_device::SBB_H_A()
{
	UINT8 tmp = H - A - (PSW & CY);
	ZHC_SUB( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 60 77: 0110 0000 0111 0111 */
void upd7810_device::SBB_L_A()
{
	UINT8 tmp = L - A - (PSW & CY);
	ZHC_SUB( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 60 78: 0110 0000 0111 1000 */
void upd7810_device::EQA_V_A()
{
	UINT8 tmp = V - A;
	ZHC_SUB( tmp, V, 0 );
	SKIP_Z;
}

/* 60 79: 0110 0000 0111 1001 */
void upd7810_device::EQA_A_A()
{
	UINT8 tmp = A - A;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 7a: 0110 0000 0111 1010 */
void upd7810_device::EQA_B_A()
{
	UINT8 tmp = B - A;
	ZHC_SUB( tmp, B, 0 );
	SKIP_Z;
}

/* 60 7b: 0110 0000 0111 1011 */
void upd7810_device::EQA_C_A()
{
	UINT8 tmp = C - A;
	ZHC_SUB( tmp, C, 0 );
	SKIP_Z;
}

/* 60 7c: 0110 0000 0111 1100 */
void upd7810_device::EQA_D_A()
{
	UINT8 tmp = D - A;
	ZHC_SUB( tmp, D, 0 );
	SKIP_Z;
}

/* 60 7d: 0110 0000 0111 1101 */
void upd7810_device::EQA_E_A()
{
	UINT8 tmp = E - A;
	ZHC_SUB( tmp, E, 0 );
	SKIP_Z;
}

/* 60 7e: 0110 0000 0111 1110 */
void upd7810_device::EQA_H_A()
{
	UINT8 tmp = H - A;
	ZHC_SUB( tmp, H, 0 );
	SKIP_Z;
}

/* 60 7f: 0110 0000 0111 1111 */
void upd7810_device::EQA_L_A()
{
	UINT8 tmp = L - A;
	ZHC_SUB( tmp, L, 0 );
	SKIP_Z;
}

/* 60 88: 0110 0000 1000 1000 */
void upd7810_device::ANA_A_V()
{
	A &= V;
	SET_Z(A);
}

/* 60 89: 0110 0000 1000 1001 */
/* ANA_A_A already defined */

/* 60 8a: 0110 0000 1000 1010 */
void upd7810_device::ANA_A_B()
{
	A &= B;
	SET_Z(A);
}

/* 60 8b: 0110 0000 1000 1011 */
void upd7810_device::ANA_A_C()
{
	A &= C;
	SET_Z(A);
}

/* 60 8c: 0110 0000 1000 1100 */
void upd7810_device::ANA_A_D()
{
	A &= D;
	SET_Z(A);
}

/* 60 8d: 0110 0000 1000 1101 */
void upd7810_device::ANA_A_E()
{
	A &= E;
	SET_Z(A);
}

/* 60 8e: 0110 0000 1000 1110 */
void upd7810_device::ANA_A_H()
{
	A &= H;
	SET_Z(A);
}

/* 60 8f: 0110 0000 1000 1111 */
void upd7810_device::ANA_A_L()
{
	A &= L;
	SET_Z(A);
}

/* 60 90: 0110 0000 1001 0000 */
void upd7810_device::XRA_A_V()
{
	A ^= V;
	SET_Z(A);
}

/* 60 91: 0110 0000 1001 0001 */
/* XRA_A_A already defined */

/* 60 92: 0110 0000 1001 0010 */
void upd7810_device::XRA_A_B()
{
	A ^= B;
	SET_Z(A);
}

/* 60 93: 0110 0000 1001 0011 */
void upd7810_device::XRA_A_C()
{
	A ^= C;
	SET_Z(A);
}

/* 60 94: 0110 0000 1001 0100 */
void upd7810_device::XRA_A_D()
{
	A ^= D;
	SET_Z(A);
}

/* 60 95: 0110 0000 1001 0101 */
void upd7810_device::XRA_A_E()
{
	A ^= E;
	SET_Z(A);
}

/* 60 96: 0110 0000 1001 0110 */
void upd7810_device::XRA_A_H()
{
	A ^= H;
	SET_Z(A);
}

/* 60 97: 0110 0000 1001 0111 */
void upd7810_device::XRA_A_L()
{
	A ^= L;
	SET_Z(A);
}

/* 60 98: 0110 0000 1001 1000 */
void upd7810_device::ORA_A_V()
{
	A |= V;
	SET_Z(A);
}

/* 60 99: 0110 0000 1001 1001 */
/* ORA_A_A already defined */

/* 60 9a: 0110 0000 1001 1010 */
void upd7810_device::ORA_A_B()
{
	A |= B;
	SET_Z(A);
}

/* 60 9b: 0110 0000 1001 1011 */
void upd7810_device::ORA_A_C()
{
	A |= C;
	SET_Z(A);
}

/* 60 9c: 0110 0000 1001 1100 */
void upd7810_device::ORA_A_D()
{
	A |= D;
	SET_Z(A);
}

/* 60 9d: 0110 0000 1001 1101 */
void upd7810_device::ORA_A_E()
{
	A |= E;
	SET_Z(A);
}

/* 60 9e: 0110 0000 1001 1110 */
void upd7810_device::ORA_A_H()
{
	A |= H;
	SET_Z(A);
}

/* 60 9f: 0110 0000 1001 1111 */
void upd7810_device::ORA_A_L()
{
	A |= L;
	SET_Z(A);
}

/* 60 a0: 0110 0000 1010 0000 */
void upd7810_device::ADDNC_A_V()
{
	UINT8 tmp = A + V;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a1: 0110 0000 1010 0001 */
/* ADDNC_A_A already defined */

/* 60 a2: 0110 0000 1010 0010 */
void upd7810_device::ADDNC_A_B()
{
	UINT8 tmp = A + B;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a3: 0110 0000 1010 0011 */
void upd7810_device::ADDNC_A_C()
{
	UINT8 tmp = A + C;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a4: 0110 0000 1010 0100 */
void upd7810_device::ADDNC_A_D()
{
	UINT8 tmp = A + D;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a5: 0110 0000 1010 0101 */
void upd7810_device::ADDNC_A_E()
{
	UINT8 tmp = A + E;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a6: 0110 0000 1010 0110 */
void upd7810_device::ADDNC_A_H()
{
	UINT8 tmp = A + H;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a7: 0110 0000 1010 0111 */
void upd7810_device::ADDNC_A_L()
{
	UINT8 tmp = A + L;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 a8: 0110 0000 1010 1000 */
void upd7810_device::GTA_A_V()
{
	UINT16 tmp = A - V - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 a9: 0110 0000 1010 1001 */
/* GTA_A_A already defined */

/* 60 aa: 0110 0000 1010 1010 */
void upd7810_device::GTA_A_B()
{
	UINT16 tmp = A - B - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ab: 0110 0000 1010 1011 */
void upd7810_device::GTA_A_C()
{
	UINT16 tmp = A - C - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ac: 0110 0000 1010 1100 */
void upd7810_device::GTA_A_D()
{
	UINT16 tmp = A - D - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ad: 0110 0000 1010 1101 */
void upd7810_device::GTA_A_E()
{
	UINT16 tmp = A - E - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 ae: 0110 0000 1010 1110 */
void upd7810_device::GTA_A_H()
{
	UINT16 tmp = A - H - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 af: 0110 0000 1010 1111 */
void upd7810_device::GTA_A_L()
{
	UINT16 tmp = A - L - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 60 b0: 0110 0000 1011 0000 */
void upd7810_device::SUBNB_A_V()
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b1: 0110 0000 1011 0001 */
/* SUBNB_A_A already defined */

/* 60 b2: 0110 0000 1011 0010 */
void upd7810_device::SUBNB_A_B()
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b3: 0110 0000 1011 0011 */
void upd7810_device::SUBNB_A_C()
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b4: 0110 0000 1011 0100 */
void upd7810_device::SUBNB_A_D()
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b5: 0110 0000 1011 0101 */
void upd7810_device::SUBNB_A_E()
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b6: 0110 0000 1011 0110 */
void upd7810_device::SUBNB_A_H()
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b7: 0110 0000 1011 0111 */
void upd7810_device::SUBNB_A_L()
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 60 b8: 0110 0000 1011 1000 */
void upd7810_device::LTA_A_V()
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 b9: 0110 0000 1011 1001 */
/* LTA_A_A already defined */

/* 60 ba: 0110 0000 1011 1010 */
void upd7810_device::LTA_A_B()
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bb: 0110 0000 1011 1011 */
void upd7810_device::LTA_A_C()
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bc: 0110 0000 1011 1100 */
void upd7810_device::LTA_A_D()
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bd: 0110 0000 1011 1101 */
void upd7810_device::LTA_A_E()
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 be: 0110 0000 1011 1110 */
void upd7810_device::LTA_A_H()
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 bf: 0110 0000 1011 1111 */
void upd7810_device::LTA_A_L()
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 60 c0: 0110 0000 1100 0000 */
void upd7810_device::ADD_A_V()
{
	UINT8 tmp = A + V;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c1: 0110 0000 1100 0001 */
/* ADD_A_A already defined */

/* 60 c2: 0110 0000 1100 0010 */
void upd7810_device::ADD_A_B()
{
	UINT8 tmp = A + B;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c3: 0110 0000 1100 0011 */
void upd7810_device::ADD_A_C()
{
	UINT8 tmp = A + C;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c4: 0110 0000 1100 0100 */
void upd7810_device::ADD_A_D()
{
	UINT8 tmp = A + D;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c5: 0110 0000 1100 0101 */
void upd7810_device::ADD_A_E()
{
	UINT8 tmp = A + E;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c6: 0110 0000 1100 0110 */
void upd7810_device::ADD_A_H()
{
	UINT8 tmp = A + H;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c7: 0110 0000 1100 0111 */
void upd7810_device::ADD_A_L()
{
	UINT8 tmp = A + L;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 60 c8: 0110 0000 1100 1000 */
void upd7810_device::ONA_A_V()
{
	if (A & V)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 c9: 0110 0000 1100 1001 */
void upd7810_device::ONA_A_A()
{
	if (A & A)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 ca: 0110 0000 1100 1010 */
void upd7810_device::ONA_A_B()
{
	if (A & B)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cb: 0110 0000 1100 1011 */
void upd7810_device::ONA_A_C()
{
	if (A & C)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cc: 0110 0000 1100 1100 */
void upd7810_device::ONA_A_D()
{
	if (A & D)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cd: 0110 0000 1100 1101 */
void upd7810_device::ONA_A_E()
{
	if (A & E)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 ce: 0110 0000 1100 1110 */
void upd7810_device::ONA_A_H()
{
	if (A & H)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 cf: 0110 0000 1100 1111 */
void upd7810_device::ONA_A_L()
{
	if (A & L)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 60 d0: 0110 0000 1101 0000 */
void upd7810_device::ADC_A_V()
{
	UINT8 tmp = A + V + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d1: 0110 0000 1101 0001 */
/* ADC_A_A already defined */

/* 60 d2: 0110 0000 1101 0010 */
void upd7810_device::ADC_A_B()
{
	UINT8 tmp = A + B + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d3: 0110 0000 1101 0011 */
void upd7810_device::ADC_A_C()
{
	UINT8 tmp = A + C + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d4: 0110 0000 1101 0100 */
void upd7810_device::ADC_A_D()
{
	UINT8 tmp = A + D + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d5: 0110 0000 1101 0101 */
void upd7810_device::ADC_A_E()
{
	UINT8 tmp = A + E + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d6: 0110 0000 1101 0110 */
void upd7810_device::ADC_A_H()
{
	UINT8 tmp = A + H + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d7: 0110 0000 1101 0111 */
void upd7810_device::ADC_A_L()
{
	UINT8 tmp = A + L + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 d8: 0110 0000 1101 1000 */
void upd7810_device::OFFA_A_V()
{
	if ( A & V )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 d9: 0110 0000 1101 1001 */
void upd7810_device::OFFA_A_A()
{
	if ( A & A )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 da: 0110 0000 1101 1010 */
void upd7810_device::OFFA_A_B()
{
	if ( A & B )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 db: 0110 0000 1101 1011 */
void upd7810_device::OFFA_A_C()
{
	if ( A & C )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 dc: 0110 0000 1101 1100 */
void upd7810_device::OFFA_A_D()
{
	if ( A & D )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 dd: 0110 0000 1101 1101 */
void upd7810_device::OFFA_A_E()
{
	if ( A & E )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 de: 0110 0000 1101 1110 */
void upd7810_device::OFFA_A_H()
{
	if ( A & H )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 df: 0110 0000 1101 1111 */
void upd7810_device::OFFA_A_L()
{
	if ( A & L )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 60 e0: 0110 0000 1110 0000 */
void upd7810_device::SUB_A_V()
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e1: 0110 0000 1110 0001 */
/* SUB_A_A already defined */

/* 60 e2: 0110 0000 1110 0010 */
void upd7810_device::SUB_A_B()
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e3: 0110 0000 1110 0011 */
void upd7810_device::SUB_A_C()
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e4: 0110 0000 1110 0100 */
void upd7810_device::SUB_A_D()
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e5: 0110 0000 1110 0101 */
void upd7810_device::SUB_A_E()
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e6: 0110 0000 1110 0110 */
void upd7810_device::SUB_A_H()
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e7: 0110 0000 1110 0111 */
void upd7810_device::SUB_A_L()
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 60 e8: 0110 0000 1110 1000 */
void upd7810_device::NEA_A_V()
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 e9: 0110 0000 1110 1001 */
/* NEA_A_A already defined */

/* 60 ea: 0110 0000 1110 1010 */
void upd7810_device::NEA_A_B()
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 eb: 0110 0000 1110 1011 */
void upd7810_device::NEA_A_C()
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ec: 0110 0000 1110 1100 */
void upd7810_device::NEA_A_D()
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ed: 0110 0000 1110 1101 */
void upd7810_device::NEA_A_E()
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ee: 0110 0000 1110 1110 */
void upd7810_device::NEA_A_H()
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 ef: 0110 0000 1110 1111 */
void upd7810_device::NEA_A_L()
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 60 f0: 0110 0000 1111 0000 */
void upd7810_device::SBB_A_V()
{
	UINT8 tmp = A - V - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f1: 0110 0000 1111 0001 */
/* SBB_A_A already defined */

/* 60 f2: 0110 0000 1111 0010 */
void upd7810_device::SBB_A_B()
{
	UINT8 tmp = A - B - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f3: 0110 0000 1111 0011 */
void upd7810_device::SBB_A_C()
{
	UINT8 tmp = A - C - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f4: 0110 0000 1111 0100 */
void upd7810_device::SBB_A_D()
{
	UINT8 tmp = A - D - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f5: 0110 0000 1111 0101 */
void upd7810_device::SBB_A_E()
{
	UINT8 tmp = A - E - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f6: 0110 0000 1111 0110 */
void upd7810_device::SBB_A_H()
{
	UINT8 tmp = A - H - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f7: 0110 0000 1111 0111 */
void upd7810_device::SBB_A_L()
{
	UINT8 tmp = A - L - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 60 f8: 0110 0000 1111 1000 */
void upd7810_device::EQA_A_V()
{
	UINT8 tmp = A - V;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 f9: 0110 0000 1111 1001 */
/* EQA_A_A already defined */

/* 60 fa: 0110 0000 1111 1010 */
void upd7810_device::EQA_A_B()
{
	UINT8 tmp = A - B;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fb: 0110 0000 1111 1011 */
void upd7810_device::EQA_A_C()
{
	UINT8 tmp = A - C;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fc: 0110 0000 1111 1100 */
void upd7810_device::EQA_A_D()
{
	UINT8 tmp = A - D;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fd: 0110 0000 1111 1101 */
void upd7810_device::EQA_A_E()
{
	UINT8 tmp = A - E;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 fe: 0110 0000 1111 1110 */
void upd7810_device::EQA_A_H()
{
	UINT8 tmp = A - H;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 60 ff: 0110 0000 1111 1111 */
void upd7810_device::EQA_A_L()
{
	UINT8 tmp = A - L;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* prefix 64 */
/* 64 00: 0110 0100 0000 0000 xxxx xxxx */
void upd7810_device::MVI_PA_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTA, imm );
}

/* 64 01: 0110 0100 0000 0001 xxxx xxxx */
void upd7810_device::MVI_PB_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTB, imm );
}

/* 64 02: 0110 0100 0000 0010 xxxx xxxx */
void upd7810_device::MVI_PC_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTC, imm );
}

/* 64 03: 0110 0100 0000 0011 xxxx xxxx */
void upd7810_device::MVI_PD_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTD, imm );
}

/* 64 05: 0110 0100 0000 0101 xxxx xxxx */
void upd7810_device::MVI_PF_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WP( UPD7810_PORTF, imm );
}

/* 64 06: 0110 0100 0000 0110 xxxx xxxx */
void upd7810_device::MVI_MKH_xx()
{
	RDOPARG( MKH );
}

/* 64 07: 0110 0100 0000 0111 xxxx xxxx */
void upd7810_device::MVI_MKL_xx()
{
	RDOPARG( MKL );
}

/* 64 08: 0110 0100 0000 1000 xxxx xxxx */
void upd7810_device::ANI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA), imm;
	RDOPARG( imm );
	pa &= imm;
	WP( UPD7810_PORTA, pa );
	SET_Z(pa);
}

/* 64 09: 0110 0100 0000 1001 xxxx xxxx */
void upd7810_device::ANI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB), imm;
	RDOPARG( imm );
	pb &= imm;
	WP( UPD7810_PORTB, pb );
	SET_Z(pb);
}

/* 64 0a: 0110 0100 0000 1010 xxxx xxxx */
void upd7810_device::ANI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC), imm;
	RDOPARG( imm );
	pc &= imm;
	WP( UPD7810_PORTC, pc );
	SET_Z(pc);
}

/* 64 0b: 0110 0100 0000 1011 xxxx xxxx */
void upd7810_device::ANI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	RDOPARG( imm );
	pd &= imm;
	WP( UPD7810_PORTD, pd );
	SET_Z(pd);
}

/* 64 0d: 0110 0100 0000 1101 xxxx xxxx */
void upd7810_device::ANI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	RDOPARG( imm );
	pf &= imm;
	WP( UPD7810_PORTF, pf );
	SET_Z(pf);
}

/* 64 0e: 0110 0100 0000 1110 xxxx xxxx */
void upd7810_device::ANI_MKH_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	MKH &= imm;
	SET_Z(MKH);
}

/* 64 0f: 0110 0100 0000 1111 xxxx xxxx */
void upd7810_device::ANI_MKL_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	MKL &= imm;
	SET_Z(MKL);
}

/* 64 10: 0110 0100 0001 0000 xxxx xxxx */
void upd7810_device::XRI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;
	RDOPARG( imm );
	pa ^= imm;
	WP( UPD7810_PORTA, pa );
	SET_Z(pa);
}

/* 64 11: 0110 0100 0001 0001 xxxx xxxx */
void upd7810_device::XRI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;
	RDOPARG( imm );
	pb ^= imm;
	WP( UPD7810_PORTB, pb );
	SET_Z(pb);
}

/* 64 12: 0110 0100 0001 0010 xxxx xxxx */
void upd7810_device::XRI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;
	RDOPARG( imm );
	pc ^= imm;
	WP( UPD7810_PORTC, pc );
	SET_Z(pc);
}

/* 64 13: 0110 0100 0001 0011 xxxx xxxx */
void upd7810_device::XRI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	RDOPARG( imm );
	pd ^= imm;
	WP( UPD7810_PORTD, pd );
	SET_Z(pd);
}

/* 64 15: 0110 0100 0001 0101 xxxx xxxx */
void upd7810_device::XRI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	RDOPARG( imm );
	pf ^= imm;
	WP( UPD7810_PORTF, pf );
	SET_Z(pf);
}

/* 64 16: 0110 0100 0001 0110 xxxx xxxx */
void upd7810_device::XRI_MKH_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	MKH ^= imm;
	SET_Z(MKH);
}

/* 64 17: 0110 0100 0001 0111 xxxx xxxx */
void upd7810_device::XRI_MKL_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	MKL ^= imm;
	SET_Z(MKL);
}

/* 64 18: 0110 0100 0001 1000 xxxx xxxx */
void upd7810_device::ORI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;
	RDOPARG( imm );
	pa |= imm;
	WP( UPD7810_PORTA, pa );
	SET_Z(pa);
}

/* 64 19: 0110 0100 0001 1001 xxxx xxxx */
void upd7810_device::ORI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;
	RDOPARG( imm );
	pb |= imm;
	WP( UPD7810_PORTB, pb );
	SET_Z(pb);
}

/* 64 1a: 0110 0100 0001 1010 xxxx xxxx */
void upd7810_device::ORI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;
	RDOPARG( imm );
	pc |= imm;
	WP( UPD7810_PORTC, pc );
	SET_Z(pc);
}

/* 64 1b: 0110 0100 0001 1011 xxxx xxxx */
void upd7810_device::ORI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	RDOPARG( imm );
	pd |= imm;
	WP( UPD7810_PORTD, pd );
	SET_Z(pd);
}

/* 64 1d: 0110 0100 0001 1101 xxxx xxxx */
void upd7810_device::ORI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	RDOPARG( imm );
	pf |= imm;
	WP( UPD7810_PORTF, pf );
	SET_Z(pf);
}

/* 64 1e: 0110 0100 0001 1110 xxxx xxxx */
void upd7810_device::ORI_MKH_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	MKH |= imm;
	SET_Z(MKH);
}

/* 64 1f: 0110 0100 0001 1111 xxxx xxxx */
void upd7810_device::ORI_MKL_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	MKL |= imm;
	SET_Z(MKL);
}

/* 64 20: 0110 0100 0010 0000 xxxx xxxx */
void upd7810_device::ADINC_PA_xx()
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
void upd7810_device::ADINC_PB_xx()
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
void upd7810_device::ADINC_PC_xx()
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
void upd7810_device::ADINC_PD_xx()
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
void upd7810_device::ADINC_PF_xx()
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
void upd7810_device::ADINC_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH + imm;

	ZHC_ADD( tmp, MKH, 0 );
	MKH = tmp;
	SKIP_NC;
}

/* 64 27: 0110 0100 0010 0111 xxxx xxxx */
void upd7810_device::ADINC_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL + imm;

	ZHC_ADD( tmp, MKL, 0 );
	MKL = tmp;
	SKIP_NC;
}

/* 64 28: 0110 0100 0010 1000 xxxx xxxx */
void upd7810_device::GTI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pa - imm - 1;
	ZHC_SUB( tmp, pa, 0 );

	SKIP_NC;
}

/* 64 29: 0110 0100 0010 1001 xxxx xxxx */
void upd7810_device::GTI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pb - imm - 1;
	ZHC_SUB( tmp, pb, 0 );

	SKIP_NC;
}

/* 64 2a: 0110 0100 0010 1010 xxxx xxxx */
void upd7810_device::GTI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pc - imm - 1;
	ZHC_SUB( tmp, pc, 0 );

	SKIP_NC;
}

/* 64 2b: 0110 0100 0010 1011 xxxx xxxx */
void upd7810_device::GTI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pd - imm - 1;
	ZHC_SUB( tmp, pd, 0 );

	SKIP_NC;
}

/* 64 2d: 0110 0100 0010 1101 xxxx xxxx */
void upd7810_device::GTI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = pf - imm - 1;
	ZHC_SUB( tmp, pf, 0 );

	SKIP_NC;
}

/* 64 2e: 0110 0100 0010 1110 xxxx xxxx */
void upd7810_device::GTI_MKH_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = MKH - imm - 1;
	ZHC_SUB( tmp, MKH, 0 );

	SKIP_NC;
}

/* 64 2f: 0110 0100 0010 1111 xxxx xxxx */
void upd7810_device::GTI_MKL_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = MKL - imm - 1;
	ZHC_SUB( tmp, MKL, 0 );

	SKIP_NC;
}

/* 64 30: 0110 0100 0011 0000 xxxx xxxx */
void upd7810_device::SUINB_PA_xx()
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
void upd7810_device::SUINB_PB_xx()
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
void upd7810_device::SUINB_PC_xx()
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
void upd7810_device::SUINB_PD_xx()
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
void upd7810_device::SUINB_PF_xx()
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
void upd7810_device::SUINB_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	MKH = tmp;
	SKIP_NC;
}

/* 64 37: 0110 0100 0011 0111 xxxx xxxx */
void upd7810_device::SUINB_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	MKL = tmp;
	SKIP_NC;
}

/* 64 38: 0110 0100 0011 1000 xxxx xxxx */
void upd7810_device::LTI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;
	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	SKIP_CY;
}

/* 64 39: 0110 0100 0011 1001 xxxx xxxx */
void upd7810_device::LTI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;
	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	SKIP_CY;
}

/* 64 3a: 0110 0100 0011 1010 xxxx xxxx */
void upd7810_device::LTI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;
	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	SKIP_CY;
}

/* 64 3b: 0110 0100 0011 1011 xxxx xxxx */
void upd7810_device::LTI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;
	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	SKIP_CY;
}

/* 64 3d: 0110 0100 0011 1101 xxxx xxxx */
void upd7810_device::LTI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;
	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	SKIP_CY;
}

/* 64 3e: 0110 0100 0011 1110 xxxx xxxx */
void upd7810_device::LTI_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	SKIP_CY;
}

/* 64 3f: 0110 0100 0011 1111 xxxx xxxx */
void upd7810_device::LTI_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	SKIP_CY;
}

/* 64 40: 0110 0100 0100 0000 xxxx xxxx */
void upd7810_device::ADI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa + imm;
	ZHC_ADD( tmp, pa, 0 );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 41: 0110 0100 0100 0001 xxxx xxxx */
void upd7810_device::ADI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb + imm;
	ZHC_ADD( tmp, pb, 0 );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 42: 0110 0100 0100 0010 xxxx xxxx */
void upd7810_device::ADI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc + imm;
	ZHC_ADD( tmp, pc, 0 );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 43: 0110 0100 0100 0011 xxxx xxxx */
void upd7810_device::ADI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd + imm;
	ZHC_ADD( tmp, pd, 0 );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 45: 0110 0100 0100 0101 xxxx xxxx */
void upd7810_device::ADI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf + imm;
	ZHC_ADD( tmp, pf, 0 );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 46: 0110 0100 0100 0110 xxxx xxxx */
void upd7810_device::ADI_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH + imm;
	ZHC_ADD( tmp, MKH, 0 );
	MKH = tmp;
}

/* 64 47: 0110 0100 0100 0111 xxxx xxxx */
void upd7810_device::ADI_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL + imm;
	ZHC_ADD( tmp, MKL, 0 );
	MKL = tmp;
}

/* 64 48: 0110 0100 0100 1000 xxxx xxxx */
void upd7810_device::ONI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;

	RDOPARG( imm );
	if (pa & imm)
		PSW |= SK;
}

/* 64 49: 0110 0100 0100 1001 xxxx xxxx */
void upd7810_device::ONI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;

	RDOPARG( imm );
	if (pb & imm)
		PSW |= SK;
}

/* 64 4a: 0110 0100 0100 1010 xxxx xxxx */
void upd7810_device::ONI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;

	RDOPARG( imm );
	if (pc & imm)
		PSW |= SK;
}

/* 64 4b: 0110 0100 0100 1011 xxxx xxxx */
void upd7810_device::ONI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;

	RDOPARG( imm );
	if (pd & imm)
		PSW |= SK;
}

/* 64 4d: 0110 0100 0100 1101 xxxx xxxx */
void upd7810_device::ONI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;

	RDOPARG( imm );
	if (pf & imm)
		PSW |= SK;
}

/* 64 4e: 0110 0100 0100 1110 xxxx xxxx */
void upd7810_device::ONI_MKH_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (MKH & imm)
		PSW |= SK;
}

/* 64 4f: 0110 0100 0100 1111 xxxx xxxx */
void upd7810_device::ONI_MKL_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (MKL & imm)
		PSW |= SK;
}

/* 64 50: 0110 0100 0101 0000 xxxx xxxx */
void upd7810_device::ACI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa + imm + (PSW & CY);
	ZHC_ADD( tmp, pa, (PSW & CY) );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 51: 0110 0100 0101 0001 xxxx xxxx */
void upd7810_device::ACI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb + imm + (PSW & CY);
	ZHC_ADD( tmp, pb, (PSW & CY) );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 52: 0110 0100 0101 0010 xxxx xxxx */
void upd7810_device::ACI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc + imm + (PSW & CY);
	ZHC_ADD( tmp, pc, (PSW & CY) );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 53: 0110 0100 0101 0011 xxxx xxxx */
void upd7810_device::ACI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd + imm + (PSW & CY);
	ZHC_ADD( tmp, pd, (PSW & CY) );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 55: 0110 0100 0101 0101 xxxx xxxx */
void upd7810_device::ACI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf + imm + (PSW & CY);
	ZHC_ADD( tmp, pf, (PSW & CY) );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 56: 0110 0100 0101 0110 xxxx xxxx */
void upd7810_device::ACI_MKH_xx()
{
	UINT8 imm, tmp;

	RDOPARG( imm );
	tmp = MKH + imm + (PSW & CY);
	ZHC_ADD( tmp, MKH, (PSW & CY) );
	MKH = tmp;
}

/* 64 57: 0110 0100 0101 0111 xxxx xxxx */
void upd7810_device::ACI_MKL_xx()
{
	UINT8 imm, tmp;

	RDOPARG( imm );
	tmp = MKL + imm + (PSW & CY);
	ZHC_ADD( tmp, MKL, (PSW & CY) );
	MKL = tmp;
}

/* 64 58: 0110 0100 0101 1000 xxxx xxxx */
void upd7810_device::OFFI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), imm;

	RDOPARG( imm );
	if (0 == (pa & imm))
		PSW |= SK;
}

/* 64 59: 0110 0100 0101 1001 xxxx xxxx */
void upd7810_device::OFFI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), imm;

	RDOPARG( imm );
	if (0 == (pb & imm))
		PSW |= SK;
}

/* 64 5a: 0110 0100 0101 1010 xxxx xxxx */
void upd7810_device::OFFI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), imm;

	RDOPARG( imm );
	if (0 == (pc & imm))
		PSW |= SK;
}

/* 64 5b: 0110 0100 0101 1011 xxxx xxxx */
void upd7810_device::OFFI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), imm;

	RDOPARG( imm );
	if (0 == (pd & imm))
		PSW |= SK;
}

/* 64 5d: 0110 0100 0101 1101 xxxx xxxx */
void upd7810_device::OFFI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), imm;

	RDOPARG( imm );
	if (0 == (pf & imm))
		PSW |= SK;
}

/* 64 5e: 0110 0100 0101 1110 xxxx xxxx */
void upd7810_device::OFFI_MKH_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (MKH & imm))
		PSW |= SK;
}

/* 64 5f: 0110 0100 0101 1111 xxxx xxxx */
void upd7810_device::OFFI_MKL_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (MKL & imm))
		PSW |= SK;
}

/* 64 60: 0110 0100 0110 0000 xxxx xxxx */
void upd7810_device::SUI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 61: 0110 0100 0110 0001 xxxx xxxx */
void upd7810_device::SUI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 62: 0110 0100 0110 0010 xxxx xxxx */
void upd7810_device::SUI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 63: 0110 0100 0110 0011 xxxx xxxx */
void upd7810_device::SUI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 65: 0110 0100 0110 0101 xxxx xxxx */
void upd7810_device::SUI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 66: 0110 0100 0110 0110 xxxx xxxx */
void upd7810_device::SUI_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	MKH = tmp;
}

/* 64 67: 0110 0100 0110 0111 xxxx xxxx */
void upd7810_device::SUI_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	MKL = tmp;
}

/* 64 68: 0110 0100 0110 1000 xxxx xxxx */
void upd7810_device::NEI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	SKIP_NZ;
}

/* 64 69: 0110 0100 0110 1001 xxxx xxxx */
void upd7810_device::NEI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	SKIP_NZ;
}

/* 64 6a: 0110 0100 0110 1010 xxxx xxxx */
void upd7810_device::NEI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	SKIP_NZ;
}

/* 64 6b: 0110 0100 0110 1011 xxxx xxxx */
void upd7810_device::NEI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	SKIP_NZ;
}

/* 64 6d: 0110 0100 0110 1101 xxxx xxxx */
void upd7810_device::NEI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	SKIP_NZ;
}

/* 64 6e: 0110 0100 0110 1110 xxxx xxxx */
void upd7810_device::NEI_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	SKIP_NZ;
}

/* 64 6f: 0110 0100 0110 1111 xxxx xxxx */
void upd7810_device::NEI_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	SKIP_NZ;
}

/* 64 70: 0110 0100 0111 0000 xxxx xxxx */
void upd7810_device::SBI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm - (PSW & CY);
	ZHC_SUB( tmp, pa, (PSW & CY) );
	pa = tmp;
	WP( UPD7810_PORTA, pa );
}

/* 64 71: 0110 0100 0111 0001 xxxx xxxx */
void upd7810_device::SBI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm - (PSW & CY);
	ZHC_SUB( tmp, pb, (PSW & CY) );
	pb = tmp;
	WP( UPD7810_PORTB, pb );
}

/* 64 72: 0110 0100 0111 0010 xxxx xxxx */
void upd7810_device::SBI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm - (PSW & CY);
	ZHC_SUB( tmp, pc, (PSW & CY) );
	pc = tmp;
	WP( UPD7810_PORTC, pc );
}

/* 64 73: 0110 0100 0111 0011 xxxx xxxx */
void upd7810_device::SBI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm - (PSW & CY);
	ZHC_SUB( tmp, pd, (PSW & CY) );
	pd = tmp;
	WP( UPD7810_PORTD, pd );
}

/* 64 75: 0110 0100 0111 0101 xxxx xxxx */
void upd7810_device::SBI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm - (PSW & CY);
	ZHC_SUB( tmp, pf, (PSW & CY) );
	pf = tmp;
	WP( UPD7810_PORTF, pf );
}

/* 64 76: 0110 0100 0111 0110 xxxx xxxx */
void upd7810_device::SBI_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm - (PSW & CY);
	ZHC_SUB( tmp, MKH, (PSW & CY) );
	MKH = tmp;
}

/* 64 77: 0110 0100 0111 0111 xxxx xxxx */
void upd7810_device::SBI_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm - (PSW & CY);
	ZHC_SUB( tmp, MKL, (PSW & CY) );
	MKL = tmp;
}

/* 64 78: 0110 0100 0111 1000 xxxx xxxx */
void upd7810_device::EQI_PA_xx()
{
	UINT8 pa = RP( UPD7810_PORTA ), tmp, imm;

	RDOPARG( imm );
	tmp = pa - imm;
	ZHC_SUB( tmp, pa, 0 );
	SKIP_Z;
}

/* 64 79: 0110 0100 0111 1001 xxxx xxxx */
void upd7810_device::EQI_PB_xx()
{
	UINT8 pb = RP( UPD7810_PORTB ), tmp, imm;

	RDOPARG( imm );
	tmp = pb - imm;
	ZHC_SUB( tmp, pb, 0 );
	SKIP_Z;
}

/* 64 7a: 0110 0100 0111 1010 xxxx xxxx */
void upd7810_device::EQI_PC_xx()
{
	UINT8 pc = RP( UPD7810_PORTC ), tmp, imm;

	RDOPARG( imm );
	tmp = pc - imm;
	ZHC_SUB( tmp, pc, 0 );
	SKIP_Z;
}

/* 64 7b: 0110 0100 0111 1011 xxxx xxxx */
void upd7810_device::EQI_PD_xx()
{
	UINT8 pd = RP( UPD7810_PORTD ), tmp, imm;

	RDOPARG( imm );
	tmp = pd - imm;
	ZHC_SUB( tmp, pd, 0 );
	SKIP_Z;
}

/* 64 7d: 0110 0100 0111 1101 xxxx xxxx */
void upd7810_device::EQI_PF_xx()
{
	UINT8 pf = RP( UPD7810_PORTF ), tmp, imm;

	RDOPARG( imm );
	tmp = pf - imm;
	ZHC_SUB( tmp, pf, 0 );
	SKIP_Z;
}

/* 64 7e: 0110 0100 0111 1110 xxxx xxxx */
void upd7810_device::EQI_MKH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKH - imm;
	ZHC_SUB( tmp, MKH, 0 );
	SKIP_Z;
}

/* 64 7f: 0110 0100 0111 1111 xxxx xxxx */
void upd7810_device::EQI_MKL_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = MKL - imm;
	ZHC_SUB( tmp, MKL, 0 );
	SKIP_Z;
}

/* 64 80: 0110 0100 1000 0000 xxxx xxxx */
void upd7810_device::MVI_ANM_xx()
{
	RDOPARG( ANM );
}

/* 64 81: 0110 0100 1000 0001 xxxx xxxx */
void upd7810_device::MVI_SMH_xx()
{
	RDOPARG( SMH );
}

/* 64 83: 0110 0100 1000 0011 xxxx xxxx */
void upd7810_device::MVI_EOM_xx()
{
	RDOPARG( EOM );
	upd7810_write_EOM();
}

/* 64 85: 0110 0100 1000 0101 xxxx xxxx */
void upd7810_device::MVI_TMM_xx()
{
	RDOPARG( TMM );
}

/* 64 88: 0110 0100 1000 1000 xxxx xxxx */
void upd7810_device::ANI_ANM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	ANM &= imm;
	SET_Z(ANM);
}

/* 64 89: 0110 0100 1000 1001 xxxx xxxx */
void upd7810_device::ANI_SMH_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	SMH &= imm;
	SET_Z(SMH);
}

/* 64 8b: 0110 0100 1000 1011 xxxx xxxx */
void upd7810_device::ANI_EOM_xx()
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
void upd7810_device::ANI_TMM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	TMM &= imm;
	SET_Z(TMM);
}

/* 64 90: 0110 0100 1001 0000 xxxx xxxx */
void upd7810_device::XRI_ANM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	ANM ^= imm;
	SET_Z(ANM);
}

/* 64 91: 0110 0100 1001 0001 xxxx xxxx */
void upd7810_device::XRI_SMH_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	SMH ^= imm;
	SET_Z(SMH);
}

/* 64 93: 0110 0100 1001 0011 xxxx xxxx */
void upd7810_device::XRI_EOM_xx()
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
void upd7810_device::XRI_TMM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	TMM ^= imm;
	SET_Z(TMM);
}

/* 64 98: 0110 0100 1001 1000 xxxx xxxx */
void upd7810_device::ORI_ANM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	ANM |= imm;
	SET_Z(ANM);
}

/* 64 99: 0110 0100 1001 1001 xxxx xxxx */
void upd7810_device::ORI_SMH_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	SMH |= imm;
	SET_Z(SMH);
}

/* 64 9b: 0110 0100 1001 1011 xxxx xxxx */
void upd7810_device::ORI_EOM_xx()
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
void upd7810_device::ORI_TMM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	TMM |= imm;
	SET_Z(TMM);
}

/* 64 a0: 0110 0100 1010 0000 xxxx xxxx */
void upd7810_device::ADINC_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM + imm;

	ZHC_ADD( tmp, ANM, 0 );
	ANM = tmp;
	SKIP_NC;
}

/* 64 a1: 0110 0100 1010 0001 xxxx xxxx */
void upd7810_device::ADINC_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH + imm;

	ZHC_ADD( tmp, SMH, 0 );
	SMH = tmp;
	SKIP_NC;
}

/* 64 a3: 0110 0100 1010 0011 xxxx xxxx */
void upd7810_device::ADINC_EOM_xx()
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
void upd7810_device::ADINC_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM + imm;

	ZHC_ADD( tmp, TMM, 0 );
	TMM = tmp;
	SKIP_NC;
}

/* 64 a8: 0110 0100 1010 1000 xxxx xxxx */
void upd7810_device::GTI_ANM_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = ANM - imm - 1;
	ZHC_SUB( tmp, ANM, 0 );

	SKIP_NC;
}

/* 64 a9: 0110 0100 1010 1001 xxxx xxxx */
void upd7810_device::GTI_SMH_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = SMH - imm - 1;
	ZHC_SUB( tmp, SMH, 0 );

	SKIP_NC;
}

/* 64 ab: 0110 0100 1010 1011 xxxx xxxx */
void upd7810_device::GTI_EOM_xx()
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
void upd7810_device::GTI_TMM_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = TMM - imm - 1;
	ZHC_SUB( tmp, TMM, 0 );

	SKIP_NC;
}

/* 64 b0: 0110 0100 1011 0000 xxxx xxxx */
void upd7810_device::SUINB_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	ANM = tmp;
	SKIP_NC;
}

/* 64 b1: 0110 0100 1011 0001 xxxx xxxx */
void upd7810_device::SUINB_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SMH = tmp;
	SKIP_NC;
}

/* 64 b3: 0110 0100 1011 0011 xxxx xxxx */
void upd7810_device::SUINB_EOM_xx()
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
void upd7810_device::SUINB_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	TMM = tmp;
	SKIP_NC;
}

/* 64 b8: 0110 0100 1011 1000 xxxx xxxx */
void upd7810_device::LTI_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	SKIP_CY;
}

/* 64 b9: 0110 0100 1011 1001 xxxx xxxx */
void upd7810_device::LTI_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SKIP_CY;
}

/* 64 bb: 0110 0100 1011 1011 xxxx xxxx */
void upd7810_device::LTI_EOM_xx()
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
void upd7810_device::LTI_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	SKIP_CY;
}

/* 64 c0: 0110 0100 1100 0000 xxxx xxxx */
void upd7810_device::ADI_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM + imm;

	ZHC_ADD( tmp, ANM, 0 );
	ANM = tmp;
}

/* 64 c1: 0110 0100 1100 0001 xxxx xxxx */
void upd7810_device::ADI_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH + imm;

	ZHC_ADD( tmp, SMH, 0 );
	SMH = tmp;
}

/* 64 c3: 0110 0100 1100 0011 xxxx xxxx */
void upd7810_device::ADI_EOM_xx()
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
void upd7810_device::ADI_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM + imm;

	ZHC_ADD( tmp, TMM, 0 );
	TMM = tmp;
}

/* 64 c8: 0110 0100 1100 1000 xxxx xxxx */
void upd7810_device::ONI_ANM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (ANM & imm)
		PSW |= SK;
}

/* 64 c9: 0110 0100 1100 1001 xxxx xxxx */
void upd7810_device::ONI_SMH_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (SMH & imm)
		PSW |= SK;
}

/* 64 cb: 0110 0100 1100 1011 xxxx xxxx */
void upd7810_device::ONI_EOM_xx()
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;

	RDOPARG( imm );
	if (eom & imm)
		PSW |= SK;
}

/* 64 cd: 0110 0100 1100 1101 xxxx xxxx */
void upd7810_device::ONI_TMM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (TMM & imm)
		PSW |= SK;
}

/* 64 d0: 0110 0100 1101 0000 xxxx xxxx */
void upd7810_device::ACI_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM + imm + (PSW & CY);

	ZHC_ADD( tmp, ANM, (PSW & CY) );
	ANM = tmp;
}

/* 64 d1: 0110 0100 1101 0001 xxxx xxxx */
void upd7810_device::ACI_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH + imm + (PSW & CY);

	ZHC_ADD( tmp, SMH, (PSW & CY) );
	SMH = tmp;
}

/* 64 d3: 0110 0100 1101 0011 xxxx xxxx */
void upd7810_device::ACI_EOM_xx()
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
void upd7810_device::ACI_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM + imm + (PSW & CY);

	ZHC_ADD( tmp, TMM, (PSW & CY) );
	TMM = tmp;
}

/* 64 d8: 0110 0100 1101 1000 xxxx xxxx */
void upd7810_device::OFFI_ANM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (ANM & imm))
		PSW |= SK;
}

/* 64 d9: 0110 0100 1101 1001 xxxx xxxx */
void upd7810_device::OFFI_SMH_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (SMH & imm))
		PSW |= SK;
}

/* 64 db: 0110 0100 1101 1011 xxxx xxxx */
void upd7810_device::OFFI_EOM_xx()
{
	/* only bits #1 and #5 can be read */
	UINT8 eom = EOM & 0x22;
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (eom & imm))
		PSW |= SK;
}

/* 64 dd: 0110 0100 1101 1101 xxxx xxxx */
void upd7810_device::OFFI_TMM_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (TMM & imm))
		PSW |= SK;
}

/* 64 e0: 0110 0100 1110 0000 xxxx xxxx */
void upd7810_device::SUI_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	ANM = tmp;
}

/* 64 e1: 0110 0100 1110 0001 xxxx xxxx */
void upd7810_device::SUI_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SMH = tmp;
}

/* 64 e3: 0110 0100 1110 0011 xxxx xxxx */
void upd7810_device::SUI_EOM_xx()
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
void upd7810_device::SUI_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	TMM = tmp;
}

/* 64 e8: 0110 0100 1110 1000 xxxx xxxx */
void upd7810_device::NEI_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	SKIP_NZ;
}

/* 64 e9: 0110 0100 1110 1001 xxxx xxxx */
void upd7810_device::NEI_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SKIP_NZ;
}

/* 64 eb: 0110 0100 1110 1011 xxxx xxxx */
void upd7810_device::NEI_EOM_xx()
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
void upd7810_device::NEI_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	SKIP_NZ;
}

/* 64 f0: 0110 0100 1111 0000 xxxx xxxx */
void upd7810_device::SBI_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm - (PSW & CY);
	ZHC_SUB( tmp, ANM, (PSW & CY) );
	ANM = tmp;
}

/* 64 f1: 0110 0100 1111 0001 xxxx xxxx */
void upd7810_device::SBI_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm - (PSW & CY);
	ZHC_SUB( tmp, SMH, (PSW & CY) );
	SMH = tmp;
}

/* 64 f3: 0110 0100 1111 0011 xxxx xxxx */
void upd7810_device::SBI_EOM_xx()
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
void upd7810_device::SBI_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm - (PSW & CY);
	ZHC_SUB( tmp, TMM, (PSW & CY) );
	TMM = tmp;
}

/* 64 f8: 0110 0100 1111 1000 xxxx xxxx */
void upd7810_device::EQI_ANM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = ANM - imm;
	ZHC_SUB( tmp, ANM, 0 );
	SKIP_Z;
}

/* 64 f9: 0110 0100 1111 1001 xxxx xxxx */
void upd7810_device::EQI_SMH_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = SMH - imm;
	ZHC_SUB( tmp, SMH, 0 );
	SKIP_Z;
}

/* 64 fb: 0110 0100 1111 1011 xxxx xxxx */
void upd7810_device::EQI_EOM_xx()
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
void upd7810_device::EQI_TMM_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = TMM - imm;
	ZHC_SUB( tmp, TMM, 0 );
	SKIP_Z;
}

/* prefix 70 */
/* 70 0e: 0111 0000 0000 1110 llll llll hhhh hhhh */
void upd7810_device::SSPD_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, SPL );
	WM( ea.d + 1, SPH );
}

/* 70 0f: 0111 0000 0000 1111 llll llll hhhh hhhh */
void upd7810_device::LSPD_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	SPL = RM( ea.d );
	SPH = RM( ea.d + 1 );
}

/* 70 1e: 0111 0000 0001 1110 llll llll hhhh hhhh */
void upd7810_device::SBCD_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, C );
	WM( ea.d + 1, B );
}

/* 70 1f: 0111 0000 0001 1111 llll llll hhhh hhhh */
void upd7810_device::LBCD_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	C = RM( ea.d );
	B = RM( ea.d + 1 );
}

/* 70 2e: 0111 0000 0010 1110 llll llll hhhh hhhh */
void upd7810_device::SDED_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, E );
	WM( ea.d + 1, D );
}

/* 70 2f: 0111 0000 0010 1111 llll llll hhhh hhhh */
void upd7810_device::LDED_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	E = RM( ea.d );
	D = RM( ea.d + 1 );
}

/* 70 3e: 0111 0000 0011 1110 llll llll hhhh hhhh */
void upd7810_device::SHLD_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, L );
	WM( ea.d + 1, H );
}

/* 70 3f: 0111 0000 0011 1111 llll llll hhhh hhhh */
void upd7810_device::LHLD_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	L = RM( ea.d );
	H = RM( ea.d + 1 );
}

/* 70 41: 0111 0000 0100 0001 */
void upd7810_device::EADD_EA_A()
{
	UINT16 tmp;
	tmp = EA + A;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 70 42: 0111 0000 0100 0010 */
void upd7810_device::EADD_EA_B()
{
	UINT16 tmp;
	tmp = EA + B;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 70 43: 0111 0000 0100 0011 */
void upd7810_device::EADD_EA_C()
{
	UINT16 tmp;
	tmp = EA + C;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 70 61: 0111 0000 0110 0001 */
void upd7810_device::ESUB_EA_A()
{
	UINT16 tmp;
	tmp = EA - A;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 70 62: 0111 0000 0110 0010 */
void upd7810_device::ESUB_EA_B()
{
	UINT16 tmp;
	tmp = EA - B;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 70 63: 0111 0000 0110 0011 */
void upd7810_device::ESUB_EA_C()
{
	UINT16 tmp;
	tmp = EA - C;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 70 68: 0111 0000 0110 1000 llll llll hhhh hhhh */
void upd7810_device::MOV_V_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	V = RM( ea.d );
}

/* 70 69: 0111 0000 0110 1001 llll llll hhhh hhhh */
void upd7810_device::MOV_A_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	A = RM( ea.d );
}

/* 70 6a: 0111 0000 0110 1010 llll llll hhhh hhhh */
void upd7810_device::MOV_B_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	B = RM( ea.d );
}

/* 70 6b: 0111 0000 0110 1011 llll llll hhhh hhhh */
void upd7810_device::MOV_C_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	C = RM( ea.d );
}

/* 70 6c: 0111 0000 0110 1100 llll llll hhhh hhhh */
void upd7810_device::MOV_D_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	D = RM( ea.d );
}

/* 70 6d: 0111 0000 0110 1101 llll llll hhhh hhhh */
void upd7810_device::MOV_E_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	E = RM( ea.d );
}

/* 70 6e: 0111 0000 0110 1110 llll llll hhhh hhhh */
void upd7810_device::MOV_H_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	H = RM( ea.d );
}

/* 70 6f: 0111 0000 0110 1111 llll llll hhhh hhhh */
void upd7810_device::MOV_L_w()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	L = RM( ea.d );
}

/* 70 78: 0111 0000 0111 1000 llll llll hhhh hhhh */
void upd7810_device::MOV_w_V()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, V );
}

/* 70 79: 0111 0000 0111 1001 llll llll hhhh hhhh */
void upd7810_device::MOV_w_A()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, A );
}

/* 70 7a: 0111 0000 0111 1010 llll llll hhhh hhhh */
void upd7810_device::MOV_w_B()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, B );
}

/* 70 7b: 0111 0000 0111 1011 llll llll hhhh hhhh */
void upd7810_device::MOV_w_C()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, C );
}

/* 70 7c: 0111 0000 0111 1100 llll llll hhhh hhhh */
void upd7810_device::MOV_w_D()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, D );
}

/* 70 7d: 0111 0000 0111 1101 llll llll hhhh hhhh */
void upd7810_device::MOV_w_E()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, E );
}

/* 70 7e: 0111 0000 0111 1110 llll llll hhhh hhhh */
void upd7810_device::MOV_w_H()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, H );
}

/* 70 7f: 0111 0000 0111 1111 llll llll hhhh hhhh */
void upd7810_device::MOV_w_L()
{
	PAIR ea;
	ea.d = 0;

	RDOPARG( ea.b.l );
	RDOPARG( ea.b.h );
	WM( ea.d, L );
}

/* 70 89: 0111 0000 1000 1001 */
void upd7810_device::ANAX_B()
{
	A &= RM( BC );
	SET_Z(A);
}

/* 70 8a: 0111 0000 1000 1010 */
void upd7810_device::ANAX_D()
{
	A &= RM( DE );
	SET_Z(A);
}

/* 70 8b: 0111 0000 1000 1011 */
void upd7810_device::ANAX_H()
{
	A &= RM( HL );
	SET_Z(A);
}

/* 70 8c: 0111 0000 1000 1100 */
void upd7810_device::ANAX_Dp()
{
	A &= RM( DE );
	DE++;
	SET_Z(A);
}

/* 70 8d: 0111 0000 1000 1101 */
void upd7810_device::ANAX_Hp()
{
	A &= RM( HL );
	HL++;
	SET_Z(A);
}

/* 70 8e: 0111 0000 1000 1110 */
void upd7810_device::ANAX_Dm()
{
	A &= RM( DE );
	DE--;
	SET_Z(A);
}

/* 70 8f: 0111 0000 1000 1111 */
void upd7810_device::ANAX_Hm()
{
	A &= RM( HL );
	HL--;
	SET_Z(A);
}

/* 70 91: 0111 0000 1001 0001 */
void upd7810_device::XRAX_B()
{
	A ^= RM( BC );
	SET_Z(A);
}

/* 70 92: 0111 0000 1001 0010 */
void upd7810_device::XRAX_D()
{
	A ^= RM( DE );
	SET_Z(A);
}

/* 70 93: 0111 0000 1001 0011 */
void upd7810_device::XRAX_H()
{
	A ^= RM( HL );
	SET_Z(A);
}

/* 70 94: 0111 0000 1001 0100 */
void upd7810_device::XRAX_Dp()
{
	A ^= RM( DE );
	DE++;
	SET_Z(A);
}

/* 70 95: 0111 0000 1001 0101 */
void upd7810_device::XRAX_Hp()
{
	A ^= RM( HL );
	HL++;
	SET_Z(A);
}

/* 70 96: 0111 0000 1001 0110 */
void upd7810_device::XRAX_Dm()
{
	A ^= RM( DE );
	DE--;
	SET_Z(A);
}

/* 70 97: 0111 0000 1001 0111 */
void upd7810_device::XRAX_Hm()
{
	A ^= RM( HL );
	HL--;
	SET_Z(A);
}

/* 70 99: 0111 0000 1001 1001 */
void upd7810_device::ORAX_B()
{
	A |= RM( BC );
	SET_Z(A);
}

/* 70 9a: 0111 0000 1001 1010 */
void upd7810_device::ORAX_D()
{
	A |= RM( DE );
	SET_Z(A);
}

/* 70 9b: 0111 0000 1001 1011 */
void upd7810_device::ORAX_H()
{
	A |= RM( HL );
	SET_Z(A);
}

/* 70 9c: 0111 0000 1001 0100 */
void upd7810_device::ORAX_Dp()
{
	A |= RM( DE );
	DE++;
	SET_Z(A);
}

/* 70 9d: 0111 0000 1001 1101 */
void upd7810_device::ORAX_Hp()
{
	A |= RM( HL );
	HL++;
	SET_Z(A);
}

/* 70 9e: 0111 0000 1001 1110 */
void upd7810_device::ORAX_Dm()
{
	A |= RM( DE );
	DE--;
	SET_Z(A);
}

/* 70 9f: 0111 0000 1001 1111 */
void upd7810_device::ORAX_Hm()
{
	A |= RM( HL );
	HL--;
	SET_Z(A);
}

/* 70 a1: 0111 0000 1010 0001 */
void upd7810_device::ADDNCX_B()
{
	UINT8 tmp = A + RM( BC );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a2: 0111 0000 1010 0010 */
void upd7810_device::ADDNCX_D()
{
	UINT8 tmp = A + RM( DE );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a3: 0111 0000 1010 0011 */
void upd7810_device::ADDNCX_H()
{
	UINT8 tmp = A + RM( HL );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a4: 0111 0000 1010 0100 */
void upd7810_device::ADDNCX_Dp()
{
	UINT8 tmp = A + RM( DE );
	DE++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a5: 0111 0000 1010 0101 */
void upd7810_device::ADDNCX_Hp()
{
	UINT8 tmp = A + RM( HL );
	HL++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a6: 0111 0000 1010 0110 */
void upd7810_device::ADDNCX_Dm()
{
	UINT8 tmp = A + RM( DE );
	DE--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a7: 0111 0000 1010 0111 */
void upd7810_device::ADDNCX_Hm()
{
	UINT8 tmp = A + RM( HL );
	HL--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 a9: 0111 0000 1010 1001 */
void upd7810_device::GTAX_B()
{
	UINT16 tmp = A - RM( BC ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 aa: 0111 0000 1010 1010 */
void upd7810_device::GTAX_D()
{
	UINT16 tmp = A - RM( DE ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ab: 0111 0000 1010 1011 */
void upd7810_device::GTAX_H()
{
	UINT16 tmp = A - RM( HL ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ac: 0111 0000 1010 1100 */
void upd7810_device::GTAX_Dp()
{
	UINT16 tmp = A - RM( DE ) - 1;
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ad: 0111 0000 1010 1101 */
void upd7810_device::GTAX_Hp()
{
	UINT16 tmp = A - RM( HL ) - 1;
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 ae: 0111 0000 1010 1110 */
void upd7810_device::GTAX_Dm()
{
	UINT16 tmp = A - RM( DE ) - 1;
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 af: 0111 0000 1010 1111 */
void upd7810_device::GTAX_Hm()
{
	UINT16 tmp = A - RM( HL ) - 1;
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 70 b1: 0111 0000 1011 0001 */
void upd7810_device::SUBNBX_B()
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b2: 0111 0000 1011 0010 */
void upd7810_device::SUBNBX_D()
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b3: 0111 0000 1011 0011 */
void upd7810_device::SUBNBX_H()
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b4: 0111 0000 1011 0100 */
void upd7810_device::SUBNBX_Dp()
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b5: 0111 0000 1011 0101 */
void upd7810_device::SUBNBX_Hp()
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b6: 0111 0000 1011 0110 */
void upd7810_device::SUBNBX_Dm()
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b7: 0111 0000 1011 0111 */
void upd7810_device::SUBNBX_Hm()
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 70 b9: 0111 0000 1011 1001 */
void upd7810_device::LTAX_B()
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 ba: 0111 0000 1011 1010 */
void upd7810_device::LTAX_D()
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bb: 0111 0000 1011 1011 */
void upd7810_device::LTAX_H()
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bc: 0111 0000 1011 1100 */
void upd7810_device::LTAX_Dp()
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bd: 0111 0000 1011 1101 */
void upd7810_device::LTAX_Hp()
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 be: 0111 0000 1011 1110 */
void upd7810_device::LTAX_Dm()
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 bf: 0111 0000 1011 1111 */
void upd7810_device::LTAX_Hm()
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 70 c1: 0111 0000 1100 0001 */
void upd7810_device::ADDX_B()
{
	UINT8 tmp = A + RM( BC );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c2: 0111 0000 1100 0010 */
void upd7810_device::ADDX_D()
{
	UINT8 tmp = A + RM( DE );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c3: 0111 0000 1100 0011 */
void upd7810_device::ADDX_H()
{
	UINT8 tmp = A + RM( HL );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c4: 0111 0000 1100 0100 */
void upd7810_device::ADDX_Dp()
{
	UINT8 tmp = A + RM( DE );
	DE++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c5: 0111 0000 1100 0101 */
void upd7810_device::ADDX_Hp()
{
	UINT8 tmp = A + RM( HL );
	HL++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c6: 0111 0000 1100 0110 */
void upd7810_device::ADDX_Dm()
{
	UINT8 tmp = A + RM( DE );
	DE--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c7: 0111 0000 1100 0111 */
void upd7810_device::ADDX_Hm()
{
	UINT8 tmp = A + RM( HL );
	HL--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 c9: 0111 0000 1100 1001 */
void upd7810_device::ONAX_B()
{
	if (A & RM( BC ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 70 ca: 0111 0000 1100 1010 */
void upd7810_device::ONAX_D()
{
	if (A & RM( DE ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 70 cb: 0111 0000 1100 1011 */
void upd7810_device::ONAX_H()
{
	if (A & RM( HL ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 70 cc: 0111 0000 1100 1100 */
void upd7810_device::ONAX_Dp()
{
	if (A & RM( DE ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	DE++;
}

/* 70 cd: 0111 0000 1100 1101 */
void upd7810_device::ONAX_Hp()
{
	if (A & RM( HL ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	HL++;
}

/* 70 ce: 0111 0000 1100 1110 */
void upd7810_device::ONAX_Dm()
{
	if (A & RM( DE ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	DE--;
}

/* 70 cf: 0111 0000 1100 1111 */
void upd7810_device::ONAX_Hm()
{
	if (A & RM( HL ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
	HL--;
}

/* 70 d1: 0111 0000 1101 0001 */
void upd7810_device::ADCX_B()
{
	UINT8 tmp = A + RM( BC ) + (PSW & CY);
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d2: 0111 0000 1101 0010 */
void upd7810_device::ADCX_D()
{
	UINT8 tmp = A + RM( DE ) + (PSW & CY);
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d3: 0111 0000 1101 0011 */
void upd7810_device::ADCX_H()
{
	UINT8 tmp = A + RM( HL ) + (PSW & CY);
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d4: 0111 0000 1101 0100 */
void upd7810_device::ADCX_Dp()
{
	UINT8 tmp = A + RM( DE ) + (PSW & CY);
	DE++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d5: 0111 0000 1101 0101 */
void upd7810_device::ADCX_Hp()
{
	UINT8 tmp = A + RM( HL ) + (PSW & CY);
	HL++;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d6: 0111 0000 1101 0110 */
void upd7810_device::ADCX_Dm()
{
	UINT8 tmp = A + RM( DE ) + (PSW & CY);
	DE--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d7: 0111 0000 1101 0111 */
void upd7810_device::ADCX_Hm()
{
	UINT8 tmp = A + RM( HL ) + (PSW & CY);
	HL--;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 70 d9: 0111 0000 1101 1001 */
void upd7810_device::OFFAX_B()
{
	if ( A & RM( BC ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 70 da: 0111 0000 1101 1010 */
void upd7810_device::OFFAX_D()
{
	if ( A & RM( DE ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 70 db: 0111 0000 1101 1011 */
void upd7810_device::OFFAX_H()
{
	if ( A & RM( HL ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 70 dc: 0111 0000 1101 1100 */
void upd7810_device::OFFAX_Dp()
{
	if ( A & RM( DE ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	DE++;
}

/* 70 dd: 0111 0000 1101 1101 */
void upd7810_device::OFFAX_Hp()
{
	if ( A & RM( HL ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	HL++;
}

/* 70 de: 0111 0000 1101 1110 */
void upd7810_device::OFFAX_Dm()
{
	if ( A & RM( DE ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	DE--;
}

/* 70 df: 0111 0000 1101 1111 */
void upd7810_device::OFFAX_Hm()
{
	if ( A & RM( HL ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
	HL--;
}

/* 70 e1: 0111 0000 1110 0001 */
void upd7810_device::SUBX_B()
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 70 e2: 0111 0000 1110 0010 */
void upd7810_device::SUBX_D()
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 70 e3: 0111 0000 1110 0011 */
void upd7810_device::SUBX_H()
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 70 e4: 0111 0000 1110 0100 */
void upd7810_device::SUBX_Dp()
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	DE++;
}

/* 70 e5: 0111 0000 1110 0101 */
void upd7810_device::SUBX_Hp()
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	HL++;
}

/* 70 e6: 0111 0000 1110 0110 */
void upd7810_device::SUBX_Dm()
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	DE--;
}

/* 70 e7: 0111 0000 1110 0111 */
void upd7810_device::SUBX_Hm()
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	HL--;
}

/* 70 e9: 0111 0000 1110 1001 */
void upd7810_device::NEAX_B()
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ea: 0111 0000 1110 1010 */
void upd7810_device::NEAX_D()
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 eb: 0111 0000 1110 1011 */
void upd7810_device::NEAX_H()
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ec: 0111 0000 1110 1100 */
void upd7810_device::NEAX_Dp()
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ed: 0111 0000 1110 1101 */
void upd7810_device::NEAX_Hp()
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ee: 0111 0000 1110 1110 */
void upd7810_device::NEAX_Dm()
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 ef: 0111 0000 1110 1111 */
void upd7810_device::NEAX_Hm()
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 70 f1: 0111 0000 1111 0001 */
void upd7810_device::SBBX_B()
{
	UINT8 tmp = A - RM( BC ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f2: 0111 0000 1111 0010 */
void upd7810_device::SBBX_D()
{
	UINT8 tmp = A - RM( DE ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f3: 0111 0000 1111 0011 */
void upd7810_device::SBBX_H()
{
	UINT8 tmp = A - RM( HL ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f4: 0111 0000 1111 0100 */
void upd7810_device::SBBX_Dp()
{
	UINT8 tmp = A - RM( DE ) - (PSW & CY);
	DE++;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f5: 0111 0000 1111 0101 */
void upd7810_device::SBBX_Hp()
{
	UINT8 tmp = A - RM( HL ) - (PSW & CY);
	HL++;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f6: 0111 0000 1111 0110 */
void upd7810_device::SBBX_Dm()
{
	UINT8 tmp = A - RM( DE ) - (PSW & CY);
	DE--;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f7: 0111 0000 1111 0111 */
void upd7810_device::SBBX_Hm()
{
	UINT8 tmp = A - RM( HL ) - (PSW & CY);
	HL--;
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 70 f9: 0111 0000 1111 1001 */
void upd7810_device::EQAX_B()
{
	UINT8 tmp = A - RM( BC );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fa: 0111 0000 1111 1010 */
void upd7810_device::EQAX_D()
{
	UINT8 tmp = A - RM( DE );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fb: 0111 0000 1111 1011 */
void upd7810_device::EQAX_H()
{
	UINT8 tmp = A - RM( HL );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fc: 0111 0000 1111 1100 */
void upd7810_device::EQAX_Dp()
{
	UINT8 tmp = A - RM( DE );
	DE++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fd: 0111 0000 1111 1101 */
void upd7810_device::EQAX_Hp()
{
	UINT8 tmp = A - RM( HL );
	HL++;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 fe: 0111 0000 1111 1110 */
void upd7810_device::EQAX_Dm()
{
	UINT8 tmp = A - RM( DE );
	DE--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 70 ff: 0111 0000 1111 1111 */
void upd7810_device::EQAX_Hm()
{
	UINT8 tmp = A - RM( HL );
	HL--;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* prefix 74 */
/* 74 08: 0111 0100 0000 1000 xxxx xxxx */
void upd7810_device::ANI_V_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	V &= imm;
	SET_Z(V);
}

/* 74 09: 0111 0100 0000 1001 xxxx xxxx */
void upd7810_device::ANI_A_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	A &= imm;
	SET_Z(A);
}

/* 74 0a: 0111 0100 0000 1010 xxxx xxxx */
void upd7810_device::ANI_B_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	B &= imm;
	SET_Z(B);
}

/* 74 0b: 0111 0100 0000 1011 xxxx xxxx */
void upd7810_device::ANI_C_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	C &= imm;
	SET_Z(C);
}

/* 74 0c: 0111 0100 0000 1100 xxxx xxxx */
void upd7810_device::ANI_D_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	D &= imm;
	SET_Z(D);
}

/* 74 0d: 0111 0100 0000 1101 xxxx xxxx */
void upd7810_device::ANI_E_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	E &= imm;
	SET_Z(E);
}

/* 74 0e: 0111 0100 0000 1110 xxxx xxxx */
void upd7810_device::ANI_H_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	H &= imm;
	SET_Z(H);
}

/* 74 0f: 0111 0100 0000 1111 xxxx xxxx */
void upd7810_device::ANI_L_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	L &= imm;
	SET_Z(L);
}

/* 74 10: 0111 0100 0001 0000 xxxx xxxx */
void upd7810_device::XRI_V_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	V ^= imm;
	SET_Z(V);
}

/* 74 11: 0111 0100 0001 0001 xxxx xxxx */
void upd7810_device::XRI_A_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	A ^= imm;
	SET_Z(A);
}

/* 74 12: 0111 0100 0001 0010 xxxx xxxx */
void upd7810_device::XRI_B_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	B ^= imm;
	SET_Z(B);
}

/* 74 13: 0111 0100 0001 0011 xxxx xxxx */
void upd7810_device::XRI_C_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	C ^= imm;
	SET_Z(C);
}

/* 74 14: 0111 0100 0001 0100 xxxx xxxx */
void upd7810_device::XRI_D_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	D ^= imm;
	SET_Z(D);
}

/* 74 15: 0111 0100 0001 0101 xxxx xxxx */
void upd7810_device::XRI_E_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	E ^= imm;
	SET_Z(E);
}

/* 74 16: 0111 0100 0001 0110 xxxx xxxx */
void upd7810_device::XRI_H_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	H ^= imm;
	SET_Z(H);
}

/* 74 17: 0111 0100 0001 0111 xxxx xxxx */
void upd7810_device::XRI_L_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	L ^= imm;
	SET_Z(L);
}

/* 74 18: 0111 0100 0001 1000 xxxx xxxx */
void upd7810_device::ORI_V_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	V |= imm;
	SET_Z(V);
}

/* 74 19: 0111 0100 0001 1001 xxxx xxxx */
void upd7810_device::ORI_A_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	A |= imm;
	SET_Z(A);
}

/* 74 1a: 0111 0100 0001 1010 xxxx xxxx */
void upd7810_device::ORI_B_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	B |= imm;
	SET_Z(B);
}

/* 74 1b: 0111 0100 0001 1011 xxxx xxxx */
void upd7810_device::ORI_C_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	C |= imm;
	SET_Z(C);
}

/* 74 1c: 0111 0100 0001 1100 xxxx xxxx */
void upd7810_device::ORI_D_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	D |= imm;
	SET_Z(D);
}

/* 74 1d: 0111 0100 0001 1101 xxxx xxxx */
void upd7810_device::ORI_E_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	E |= imm;
	SET_Z(E);
}

/* 74 1e: 0111 0100 0001 1110 xxxx xxxx */
void upd7810_device::ORI_H_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	H |= imm;
	SET_Z(H);
}

/* 74 1f: 0111 0100 0001 1111 xxxx xxxx */
void upd7810_device::ORI_L_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	L |= imm;
	SET_Z(L);
}

/* 74 20: 0111 0100 0010 0000 xxxx xxxx */
void upd7810_device::ADINC_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V + imm;

	ZHC_ADD( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 74 21: 0111 0100 0010 0001 xxxx xxxx */
void upd7810_device::ADINC_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A + imm;

	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 22: 0111 0100 0010 0010 xxxx xxxx */
void upd7810_device::ADINC_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B + imm;

	ZHC_ADD( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 74 23: 0111 0100 0010 0011 xxxx xxxx */
void upd7810_device::ADINC_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C + imm;

	ZHC_ADD( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 74 24: 0111 0100 0010 0100 xxxx xxxx */
void upd7810_device::ADINC_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D + imm;

	ZHC_ADD( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 74 25: 0111 0100 0010 0101 xxxx xxxx */
void upd7810_device::ADINC_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E + imm;

	ZHC_ADD( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 74 26: 0111 0100 0010 0110 xxxx xxxx */
void upd7810_device::ADINC_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H + imm;

	ZHC_ADD( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 74 27: 0111 0100 0010 0111 xxxx xxxx */
void upd7810_device::ADINC_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L + imm;

	ZHC_ADD( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 74 28: 0111 0100 0010 1000 xxxx xxxx */
void upd7810_device::GTI_V_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = V - imm - 1;
	ZHC_SUB( tmp, V, 0 );

	SKIP_NC;
}

/* 74 29: 0111 0100 0010 1001 xxxx xxxx */
void upd7810_device::GTI_A_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = A - imm - 1;
	ZHC_SUB( tmp, A, 0 );

	SKIP_NC;
}

/* 74 2a: 0111 0100 0010 1010 xxxx xxxx */
void upd7810_device::GTI_B_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = B - imm - 1;
	ZHC_SUB( tmp, B, 0 );

	SKIP_NC;
}

/* 74 2b: 0111 0100 0010 1011 xxxx xxxx */
void upd7810_device::GTI_C_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = C - imm - 1;
	ZHC_SUB( tmp, C, 0 );

	SKIP_NC;
}

/* 74 2c: 0111 0100 0010 1100 xxxx xxxx */
void upd7810_device::GTI_D_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = D - imm - 1;
	ZHC_SUB( tmp, D, 0 );

	SKIP_NC;
}

/* 74 2d: 0111 0100 0010 1101 xxxx xxxx */
void upd7810_device::GTI_E_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = E - imm - 1;
	ZHC_SUB( tmp, E, 0 );

	SKIP_NC;
}

/* 74 2e: 0111 0100 0010 1110 xxxx xxxx */
void upd7810_device::GTI_H_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = H - imm - 1;
	ZHC_SUB( tmp, H, 0 );

	SKIP_NC;
}

/* 74 2f: 0111 0100 0010 1111 xxxx xxxx */
void upd7810_device::GTI_L_xx()
{
	UINT8 imm;
	UINT16 tmp;

	RDOPARG( imm );
	tmp = L - imm - 1;
	ZHC_SUB( tmp, L, 0 );

	SKIP_NC;
}

/* 74 30: 0111 0100 0011 0000 xxxx xxxx */
void upd7810_device::SUINB_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
	SKIP_NC;
}

/* 74 31: 0111 0100 0011 0001 xxxx xxxx */
void upd7810_device::SUINB_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 32: 0111 0100 0011 0010 xxxx xxxx */
void upd7810_device::SUINB_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
	SKIP_NC;
}

/* 74 33: 0111 0100 0011 0011 xxxx xxxx */
void upd7810_device::SUINB_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
	SKIP_NC;
}

/* 74 34: 0111 0100 0011 0100 xxxx xxxx */
void upd7810_device::SUINB_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
	SKIP_NC;
}

/* 74 35: 0111 0100 0011 0101 xxxx xxxx */
void upd7810_device::SUINB_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
	SKIP_NC;
}

/* 74 36: 0111 0100 0011 0110 xxxx xxxx */
void upd7810_device::SUINB_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
	SKIP_NC;
}

/* 74 37: 0111 0100 0011 0111 xxxx xxxx */
void upd7810_device::SUINB_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
	SKIP_NC;
}

/* 74 38: 0111 0100 0011 1000 xxxx xxxx */
void upd7810_device::LTI_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	SKIP_CY;
}

/* 74 39: 0111 0100 0011 1001 xxxx xxxx */
void upd7810_device::LTI_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 74 3a: 0111 0100 0011 1010 xxxx xxxx */
void upd7810_device::LTI_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	SKIP_CY;
}

/* 74 3b: 0111 0100 0011 1011 xxxx xxxx */
void upd7810_device::LTI_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	SKIP_CY;
}

/* 74 3c: 0111 0100 0011 1100 xxxx xxxx */
void upd7810_device::LTI_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	SKIP_CY;
}

/* 74 3d: 0111 0100 0011 1101 xxxx xxxx */
void upd7810_device::LTI_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	SKIP_CY;
}

/* 74 3e: 0111 0100 0011 1110 xxxx xxxx */
void upd7810_device::LTI_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	SKIP_CY;
}

/* 74 3f: 0111 0100 0011 1111 xxxx xxxx */
void upd7810_device::LTI_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	SKIP_CY;
}

/* 74 40: 0111 0100 0100 0000 xxxx xxxx */
void upd7810_device::ADI_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V + imm;

	ZHC_ADD( tmp, V, 0 );
	V = tmp;
}

/* 74 41: 0111 0100 0100 0001 xxxx xxxx */
void upd7810_device::ADI_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A + imm;

	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 74 42: 0111 0100 0100 0010 xxxx xxxx */
void upd7810_device::ADI_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B + imm;

	ZHC_ADD( tmp, B, 0 );
	B = tmp;
}

/* 74 43: 0111 0100 0100 0011 xxxx xxxx */
void upd7810_device::ADI_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C + imm;

	ZHC_ADD( tmp, C, 0 );
	C = tmp;
}

/* 74 44: 0111 0100 0100 0100 xxxx xxxx */
void upd7810_device::ADI_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D + imm;

	ZHC_ADD( tmp, D, 0 );
	D = tmp;
}

/* 74 45: 0111 0100 0100 0101 xxxx xxxx */
void upd7810_device::ADI_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E + imm;

	ZHC_ADD( tmp, E, 0 );
	E = tmp;
}

/* 74 46: 0111 0100 0100 0110 xxxx xxxx */
void upd7810_device::ADI_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H + imm;

	ZHC_ADD( tmp, H, 0 );
	H = tmp;
}

/* 74 47: 0111 0100 0100 0111 xxxx xxxx */
void upd7810_device::ADI_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L + imm;

	ZHC_ADD( tmp, L, 0 );
	L = tmp;
}

/* 74 48: 0111 0100 0100 1000 xxxx xxxx */
void upd7810_device::ONI_V_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (V & imm)
		PSW |= SK;
}

/* 74 49: 0111 0100 0100 1001 xxxx xxxx */
void upd7810_device::ONI_A_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (A & imm)
		PSW |= SK;
}

/* 74 4a: 0111 0100 0100 1010 xxxx xxxx */
void upd7810_device::ONI_B_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (B & imm)
		PSW |= SK;
}

/* 74 4b: 0111 0100 0100 1011 xxxx xxxx */
void upd7810_device::ONI_C_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (C & imm)
		PSW |= SK;
}

/* 74 4c: 0111 0100 0100 1100 xxxx xxxx */
void upd7810_device::ONI_D_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (D & imm)
		PSW |= SK;
}

/* 74 4d: 0111 0100 0100 1101 xxxx xxxx */
void upd7810_device::ONI_E_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (E & imm)
		PSW |= SK;
}

/* 74 4e: 0111 0100 0100 1110 xxxx xxxx */
void upd7810_device::ONI_H_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (H & imm)
		PSW |= SK;
}

/* 74 4f: 0111 0100 0100 1111 xxxx xxxx */
void upd7810_device::ONI_L_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (L & imm)
		PSW |= SK;
}

/* 74 50: 0111 0100 0101 0000 xxxx xxxx */
void upd7810_device::ACI_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V + imm + (PSW & CY);
	ZHC_ADD( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 74 51: 0111 0100 0101 0001 xxxx xxxx */
void upd7810_device::ACI_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A + imm + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 52: 0111 0100 0101 0010 xxxx xxxx */
void upd7810_device::ACI_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B + imm + (PSW & CY);
	ZHC_ADD( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 74 53: 0111 0100 0101 0011 xxxx xxxx */
void upd7810_device::ACI_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C + imm + (PSW & CY);
	ZHC_ADD( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 74 54: 0111 0100 0101 0100 xxxx xxxx */
void upd7810_device::ACI_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D + imm + (PSW & CY);
	ZHC_ADD( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 74 55: 0111 0100 0101 0101 xxxx xxxx */
void upd7810_device::ACI_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E + imm + (PSW & CY);
	ZHC_ADD( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 74 56: 0111 0100 0101 0110 xxxx xxxx */
void upd7810_device::ACI_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H + imm + (PSW & CY);
	ZHC_ADD( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 74 57: 0111 0100 0101 0111 xxxx xxxx */
void upd7810_device::ACI_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L + imm + (PSW & CY);
	ZHC_ADD( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 74 58: 0111 0100 0101 1000 xxxx xxxx */
void upd7810_device::OFFI_V_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (V & imm))
		PSW |= SK;
}

/* 74 59: 0111 0100 0101 1001 xxxx xxxx */
void upd7810_device::OFFI_A_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (A & imm))
		PSW |= SK;
}

/* 74 5a: 0111 0100 0101 1010 xxxx xxxx */
void upd7810_device::OFFI_B_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (B & imm))
		PSW |= SK;
}

/* 74 5b: 0111 0100 0101 1011 xxxx xxxx */
void upd7810_device::OFFI_C_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (C & imm))
		PSW |= SK;
}

/* 74 5c: 0111 0100 0101 1100 xxxx xxxx */
void upd7810_device::OFFI_D_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (D & imm))
		PSW |= SK;
}

/* 74 5d: 0111 0100 0101 1101 xxxx xxxx */
void upd7810_device::OFFI_E_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (E & imm))
		PSW |= SK;
}

/* 74 5e: 0111 0100 0101 1110 xxxx xxxx */
void upd7810_device::OFFI_H_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (H & imm))
		PSW |= SK;
}

/* 74 5f: 0111 0100 0101 1111 xxxx xxxx */
void upd7810_device::OFFI_L_xx()
{
	UINT8 imm;

	RDOPARG( imm );
	if (0 == (L & imm))
		PSW |= SK;
}

/* 74 60: 0111 0100 0110 0000 xxxx xxxx */
void upd7810_device::SUI_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	V = tmp;
}

/* 74 61: 0111 0100 0110 0001 xxxx xxxx */
void upd7810_device::SUI_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 74 62: 0111 0100 0110 0010 xxxx xxxx */
void upd7810_device::SUI_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
}

/* 74 63: 0111 0100 0110 0011 xxxx xxxx */
void upd7810_device::SUI_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
}

/* 74 64: 0111 0100 0110 0100 xxxx xxxx */
void upd7810_device::SUI_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	D = tmp;
}

/* 74 65: 0111 0100 0110 0101 xxxx xxxx */
void upd7810_device::SUI_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	E = tmp;
}

/* 74 66: 0111 0100 0110 0110 xxxx xxxx */
void upd7810_device::SUI_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	H = tmp;
}

/* 74 67: 0111 0100 0110 0111 xxxx xxxx */
void upd7810_device::SUI_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	L = tmp;
}

/* 74 68: 0111 0100 0110 1000 xxxx xxxx */
void upd7810_device::NEI_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	SKIP_NZ;
}

/* 74 69: 0111 0100 0110 1001 xxxx xxxx */
void upd7810_device::NEI_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 74 6a: 0111 0100 0110 1010 xxxx xxxx */
void upd7810_device::NEI_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	SKIP_NZ;
}

/* 74 6b: 0111 0100 0110 1011 xxxx xxxx */
void upd7810_device::NEI_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	SKIP_NZ;
}

/* 74 6c: 0111 0100 0110 1100 xxxx xxxx */
void upd7810_device::NEI_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	SKIP_NZ;
}

/* 74 6d: 0111 0100 0110 1101 xxxx xxxx */
void upd7810_device::NEI_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	SKIP_NZ;
}

/* 74 6e: 0111 0100 0110 1110 xxxx xxxx */
void upd7810_device::NEI_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	SKIP_NZ;
}

/* 74 6f: 0111 0100 0110 1111 xxxx xxxx */
void upd7810_device::NEI_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	SKIP_NZ;
}

/* 74 70: 0111 0100 0111 0000 xxxx xxxx */
void upd7810_device::SBI_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm - (PSW & CY);
	ZHC_SUB( tmp, V, (PSW & CY) );
	V = tmp;
}

/* 74 71: 0111 0100 0111 0001 xxxx xxxx */
void upd7810_device::SBI_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 72: 0111 0100 0111 0010 xxxx xxxx */
void upd7810_device::SBI_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm - (PSW & CY);
	ZHC_SUB( tmp, B, (PSW & CY) );
	B = tmp;
}

/* 74 73: 0111 0100 0111 0011 xxxx xxxx */
void upd7810_device::SBI_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm - (PSW & CY);
	ZHC_SUB( tmp, C, (PSW & CY) );
	C = tmp;
}

/* 74 74: 0111 0100 0111 0100 xxxx xxxx */
void upd7810_device::SBI_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm - (PSW & CY);
	ZHC_SUB( tmp, D, (PSW & CY) );
	D = tmp;
}

/* 74 75: 0111 0100 0111 0101 xxxx xxxx */
void upd7810_device::SBI_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm - (PSW & CY);
	ZHC_SUB( tmp, E, (PSW & CY) );
	E = tmp;
}

/* 74 76: 0111 0100 0111 0110 xxxx xxxx */
void upd7810_device::SBI_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm - (PSW & CY);
	ZHC_SUB( tmp, H, (PSW & CY) );
	H = tmp;
}

/* 74 77: 0111 0100 0111 0111 xxxx xxxx */
void upd7810_device::SBI_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm - (PSW & CY);
	ZHC_SUB( tmp, L, (PSW & CY) );
	L = tmp;
}

/* 74 78: 0111 0100 0111 1000 xxxx xxxx */
void upd7810_device::EQI_V_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = V - imm;
	ZHC_SUB( tmp, V, 0 );
	SKIP_Z;
}

/* 74 79: 0111 0100 0111 1001 xxxx xxxx */
void upd7810_device::EQI_A_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = A - imm;
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 74 7a: 0111 0100 0111 1010 xxxx xxxx */
void upd7810_device::EQI_B_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = B - imm;
	ZHC_SUB( tmp, B, 0 );
	SKIP_Z;
}

/* 74 7b: 0111 0100 0111 1011 xxxx xxxx */
void upd7810_device::EQI_C_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = C - imm;
	ZHC_SUB( tmp, C, 0 );
	SKIP_Z;
}

/* 74 7c: 0111 0100 0111 1100 xxxx xxxx */
void upd7810_device::EQI_D_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = D - imm;
	ZHC_SUB( tmp, D, 0 );
	SKIP_Z;
}

/* 74 7d: 0111 0100 0111 1101 xxxx xxxx */
void upd7810_device::EQI_E_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = E - imm;
	ZHC_SUB( tmp, E, 0 );
	SKIP_Z;
}

/* 74 7e: 0111 0100 0111 1110 xxxx xxxx */
void upd7810_device::EQI_H_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = H - imm;
	ZHC_SUB( tmp, H, 0 );
	SKIP_Z;
}

/* 74 7f: 0111 0100 0111 1111 xxxx xxxx */
void upd7810_device::EQI_L_xx()
{
	UINT8 tmp, imm;

	RDOPARG( imm );
	tmp = L - imm;
	ZHC_SUB( tmp, L, 0 );
	SKIP_Z;
}

/* 74 88: 0111 0100 1000 1000 oooo oooo */
void upd7810_device::ANAW_wa()
{
	PAIR ea = m_va;
	RDOPARG( ea.b.l );

	A &= RM( ea.d );
	SET_Z(A);
}

/* 74 8d: 0111 0100 1000 1101 */
void upd7810_device::DAN_EA_BC()
{
	EA &= BC;
	SET_Z(EA);
}

/* 74 8e: 0111 0100 1000 1110 */
void upd7810_device::DAN_EA_DE()
{
	EA &= DE;
	SET_Z(EA);
}

/* 74 8f: 0111 0100 1000 1111 */
void upd7810_device::DAN_EA_HL()
{
	EA &= HL;
	SET_Z(EA);
}

/* 74 90: 0111 0100 1001 0000 oooo oooo */
void upd7810_device::XRAW_wa()
{
	PAIR ea = m_va;
	RDOPARG( ea.b.l );

	A ^= RM( ea.d );
	SET_Z(A);
}

/* 74 95: 0111 0100 1001 0101 */
void upd7810_device::DXR_EA_BC()
{
	EA ^= BC;
	SET_Z(EA);
}

/* 74 96: 0111 0100 1001 0110 */
void upd7810_device::DXR_EA_DE()
{
	EA ^= DE;
	SET_Z(EA);
}

/* 74 97: 0111 0100 1001 0111 */
void upd7810_device::DXR_EA_HL()
{
	EA ^= HL;
	SET_Z(EA);
}

/* 74 98: 0111 0100 1001 1000 oooo oooo */
void upd7810_device::ORAW_wa()
{
	PAIR ea = m_va;
	RDOPARG( ea.b.l );

	A |= RM( ea.d );
	SET_Z(A);
}

/* 74 9d: 0111 0100 1001 1101 */
void upd7810_device::DOR_EA_BC()
{
	EA |= BC;
	SET_Z(EA);
}

/* 74 9e: 0111 0100 1001 1110 */
void upd7810_device::DOR_EA_DE()
{
	EA |= DE;
	SET_Z(EA);
}

/* 74 9f: 0111 0100 1001 1111 */
void upd7810_device::DOR_EA_HL()
{
	EA |= HL;
	SET_Z(EA);
}

/* 74 a0: 0111 0100 1010 0000 oooo oooo */
void upd7810_device::ADDNCW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );

	tmp = A + RM( ea.d );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 a5: 0111 0100 1010 0101 */
void upd7810_device::DADDNC_EA_BC()
{
	UINT16 tmp = EA + BC;

	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 a6: 0111 0100 1010 0110 */
void upd7810_device::DADDNC_EA_DE()
{
	UINT16 tmp = EA + DE;

	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 a7: 0111 0100 1010 0111 */
void upd7810_device::DADDNC_EA_HL()
{
	UINT16 tmp = EA + HL;

	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 a8: 0111 0100 1010 1000 oooo oooo */
void upd7810_device::GTAW_wa()
{
	PAIR ea = m_va;
	UINT16 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d ) - 1;
	ZHC_SUB( tmp, A, 0 );
	SKIP_NC;
}

/* 74 ad: 0111 0100 1010 1101 */
void upd7810_device::DGT_EA_BC()
{
	UINT32 tmp = EA - BC - 1;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NC;
}

/* 74 ae: 0111 0100 1010 1110 */
void upd7810_device::DGT_EA_DE()
{
	UINT32 tmp = EA - DE - 1;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NC;
}

/* 74 af: 0111 0100 1010 1111 */
void upd7810_device::DGT_EA_HL()
{
	UINT32 tmp = EA - HL - 1;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NC;
}

/* 74 b0: 0111 0100 1011 0000 oooo oooo */
void upd7810_device::SUBNBW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_NC;
}

/* 74 b5: 0111 0100 1011 0101 */
void upd7810_device::DSUBNB_EA_BC()
{
	UINT16 tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 b6: 0111 0100 1011 0110 */
void upd7810_device::DSUBNB_EA_DE()
{
	UINT16 tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 b7: 0111 0100 1011 0111 */
void upd7810_device::DSUBNB_EA_HL()
{
	UINT16 tmp;

	tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
	SKIP_NC;
}

/* 74 b8: 0111 0100 1011 1000 oooo oooo */
void upd7810_device::LTAW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	SKIP_CY;
}

/* 74 bd: 0111 0100 1011 1101 */
void upd7810_device::DLT_EA_BC()
{
	UINT16 tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_CY;
}

/* 74 be: 0111 0100 1011 1110 */
void upd7810_device::DLT_EA_DE()
{
	UINT16 tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_CY;
}

/* 74 bf: 0111 0100 1011 1111 */
void upd7810_device::DLT_EA_HL()
{
	UINT16 tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_CY;
}

/* 74 c0: 0111 0100 1100 0000 oooo oooo */
void upd7810_device::ADDW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;
	RDOPARG( ea.b.l );
	tmp = A + RM( ea.d );
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
}

/* 74 c5: 0111 0100 1100 0101 */
void upd7810_device::DADD_EA_BC()
{
	UINT16 tmp = EA + BC;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 74 c6: 0111 0100 1100 0110 */
void upd7810_device::DADD_EA_DE()
{
	UINT16 tmp = EA + DE;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 74 c7: 0111 0100 1100 0111 */
void upd7810_device::DADD_EA_HL()
{
	UINT16 tmp = EA + HL;
	ZHC_ADD( tmp, EA, 0 );
	EA = tmp;
}

/* 74 c8: 0111 0100 1100 1000 oooo oooo */
void upd7810_device::ONAW_wa()
{
	PAIR ea = m_va;
	RDOPARG( ea.b.l );

	if (A & RM( ea.d ))
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 cd: 0111 0100 1100 1101 */
void upd7810_device::DON_EA_BC()
{
	if (EA & BC)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 ce: 0111 0100 1100 1110 */
void upd7810_device::DON_EA_DE()
{
	if (EA & DE)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 cf: 0111 0100 1100 1111 */
void upd7810_device::DON_EA_HL()
{
	if (EA & HL)
		PSW = (PSW & ~Z) | SK;
	else
		PSW |= Z;
}

/* 74 d0: 0111 0100 1101 0000 oooo oooo */
void upd7810_device::ADCW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A + RM( ea.d ) + (PSW & CY);
	ZHC_ADD( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 d5: 0111 0100 1101 0101 */
void upd7810_device::DADC_EA_BC()
{
	UINT16 tmp = EA + BC + (PSW & CY);
	ZHC_ADD( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 d6: 0111 0100 1101 0110 */
void upd7810_device::DADC_EA_DE()
{
	UINT16 tmp = EA + DE + (PSW & CY);
	ZHC_ADD( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 d7: 0111 0100 1101 0111 */
void upd7810_device::DADC_EA_HL()
{
	UINT16 tmp = EA + HL + (PSW & CY);
	ZHC_ADD( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 d8: 0111 0100 1101 1000 oooo oooo */
void upd7810_device::OFFAW_wa()
{
	PAIR ea = m_va;
	RDOPARG( ea.b.l );

	if ( A & RM( ea.d ) )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 dd: 0111 0100 1101 1101 */
void upd7810_device::DOFF_EA_BC()
{
	if ( EA & BC )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 de: 0111 0100 1101 1110 */
void upd7810_device::DOFF_EA_DE()
{
	if ( EA & DE )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 df: 0111 0100 1101 1111 */
void upd7810_device::DOFF_EA_HL()
{
	if ( EA & HL )
		PSW &= ~Z;
	else
		PSW = PSW | Z | SK;
}

/* 74 e0: 0111 0100 1110 0000 oooo oooo */
void upd7810_device::SUBW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
}

/* 74 e5: 0111 0100 1110 0101 */
void upd7810_device::DSUB_EA_BC()
{
	UINT16 tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 74 e6: 0111 0100 1110 0110 */
void upd7810_device::DSUB_EA_DE()
{
	UINT16 tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 74 e7: 0111 0100 1110 0111 */
void upd7810_device::DSUB_EA_HL()
{
	UINT16 tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	EA = tmp;
}

/* 74 e8: 0111 0100 1110 1000 oooo oooo */
void upd7810_device::NEAW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	SKIP_NZ;
}

/* 74 ed: 0111 0100 1110 1101 */
void upd7810_device::DNE_EA_BC()
{
	UINT16 tmp;

	tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NZ;
}

/* 74 ee: 0111 0100 1110 1110 */
void upd7810_device::DNE_EA_DE()
{
	UINT16 tmp;

	tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NZ;
}

/* 74 ef: 0111 0100 1110 1111 */
void upd7810_device::DNE_EA_HL()
{
	UINT16 tmp;

	tmp = EA - HL;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_NZ;
}

/* 74 f0: 0111 0100 1111 0000 oooo oooo */
void upd7810_device::SBBW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d ) - (PSW & CY);
	ZHC_SUB( tmp, A, (PSW & CY) );
	A = tmp;
}

/* 74 f5: 0111 0100 1111 0101 */
void upd7810_device::DSBB_EA_BC()
{
	UINT16 tmp = EA - BC - (PSW & CY);
	ZHC_SUB( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 f6: 0111 0100 1111 0110 */
void upd7810_device::DSBB_EA_DE()
{
	UINT16 tmp = EA - DE - (PSW & CY);
	ZHC_SUB( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 f7: 0111 0100 1111 0111 */
void upd7810_device::DSBB_EA_HL()
{
	UINT16 tmp = EA - HL - (PSW & CY);
	ZHC_SUB( tmp, EA, (PSW & CY) );
	EA = tmp;
}

/* 74 f8: 0111 0100 1111 1000 oooo oooo */
void upd7810_device::EQAW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp;

	RDOPARG( ea.b.l );
	tmp = A - RM( ea.d );
	ZHC_SUB( tmp, A, 0 );
	SKIP_Z;
}

/* 74 fd: 0111 0100 1111 1101 */
void upd7810_device::DEQ_EA_BC()
{
	UINT16 tmp;

	tmp = EA - BC;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_Z;
}

/* 74 fe: 0111 0100 1111 1110 */
void upd7810_device::DEQ_EA_DE()
{
	UINT16 tmp;

	tmp = EA - DE;
	ZHC_SUB( tmp, EA, 0 );
	SKIP_Z;
}

/* 74 ff: 0111 0100 1111 1111 */
void upd7810_device::DEQ_EA_HL()
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
void upd7810_device::NOP()
{
}

/* 01: 0000 0001 oooo oooo */
void upd7810_device::LDAW_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	A = RM( ea.d );
}

/* 02: 0000 0010 */
void upd7810_device::INX_SP()
{
	SP++;
}

/* 03: 0000 0011 */
void upd7810_device::DCX_SP()
{
	SP--;
}

/* 04: 0000 0100 llll llll hhhh hhhh */
void upd7810_device::LXI_S_w()
{
	RDOPARG( SPL );
	RDOPARG( SPH );
}

/* 05: 0000 0101 oooo oooo xxxx xxxx */
void upd7810_device::ANIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::MOV_A_EAH()
{
	A = EAH;
}

/* 09: 0000 1001 */
void upd7810_device::MOV_A_EAL()
{
	A = EAL;
}

/* 0a: 0000 1010 */
void upd7810_device::MOV_A_B()
{
	A = B;
}

/* 0b: 0000 1011 */
void upd7810_device::MOV_A_C()
{
	A = C;
}

/* 0c: 0000 1100 */
void upd7810_device::MOV_A_D()
{
	A = D;
}

/* 0d: 0000 1101 */
void upd7810_device::MOV_A_E()
{
	A = E;
}

/* 0e: 0000 1110 */
void upd7810_device::MOV_A_H()
{
	A = H;
}

/* 0f: 0000 1111 */
void upd7810_device::MOV_A_L()
{
	A = L;
}

/* 10: 0001 0000 */
void upd7810_device::EXA()
{
	UINT16 tmp;
	tmp = EA; EA = EA2; EA2 = tmp;
	tmp = VA; VA = VA2; VA2 = tmp;
}

/* 11: 0001 0001 */
void upd7810_device::EXX()
{
	UINT16 tmp;
	tmp = BC; BC = BC2; BC2 = tmp;
	tmp = DE; DE = DE2; DE2 = tmp;
	tmp = HL; HL = HL2; HL2 = tmp;
}

/* 48 AD (7807 only) */
void upd7810_device::EXR()
{
	UINT16 tmp;
	tmp = BC; BC = BC2; BC2 = tmp;
	tmp = DE; DE = DE2; DE2 = tmp;
	tmp = HL; HL = HL2; HL2 = tmp;
	tmp = EA; EA = EA2; EA2 = tmp;
	tmp = VA; VA = VA2; VA2 = tmp;
}

/* 12: 0001 0010 */
void upd7810_device::INX_BC()
{
	BC++;
}

/* 13: 0001 0011 */
void upd7810_device::DCX_BC()
{
	BC--;
}

/* 14: 0001 0100 llll llll hhhh hhhh */
void upd7810_device::LXI_B_w()
{
	RDOPARG( C );
	RDOPARG( B );
}

/* 15: 0001 0101 oooo oooo xxxx xxxx */
void upd7810_device::ORIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::MOV_EAH_A()
{
	EAH = A;
}

/* 19: 0001 1001 */
void upd7810_device::MOV_EAL_A()
{
	EAL = A;
}

/* 1a: 0001 1010 */
void upd7810_device::MOV_B_A()
{
	B = A;
}

/* 1b: 0001 1011 */
void upd7810_device::MOV_C_A()
{
	C = A;
}

/* 1c: 0001 1100 */
void upd7810_device::MOV_D_A()
{
	D = A;
}

/* 1d: 0001 1101 */
void upd7810_device::MOV_E_A()
{
	E = A;
}

/* 1e: 0001 1110 */
void upd7810_device::MOV_H_A()
{
	H = A;
}

/* 1f: 0001 1111 */
void upd7810_device::MOV_L_A()
{
	L = A;
}

/* 20: 0010 0000 oooo oooo */
void upd7810_device::INRW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp, m;

	RDOPARG( ea.b.l );
	m = RM( ea.d );
	tmp = m + 1;
	ZHC_ADD( tmp, m, 0 );
	WM( ea.d, tmp );
	SKIP_CY;
}

/* 21: 0010 0001 */
void upd7810_device::JB()
{
	PC = BC;
}

/* 22: 0010 0010 */
void upd7810_device::INX_DE()
{
	DE++;
}

/* 23: 0010 0011 */
void upd7810_device::DCX_DE()
{
	DE--;
}

/* 24: 0010 0100 llll llll hhhh hhhh */
void upd7810_device::LXI_D_w()
{
	RDOPARG( E );
	RDOPARG( D );
}

/* 25: 0010 0101 oooo oooo xxxx xxxx */
void upd7810_device::GTIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::LDAX_B()
{
	A = RM( BC );
}

/* 2a: 0010 1010 */
void upd7810_device::LDAX_D()
{
	A = RM( DE );
}

/* 2b: 0010 1011 */
void upd7810_device::LDAX_H()
{
	A = RM( HL );
}

/* 2c: 0010 1100 */
void upd7810_device::LDAX_Dp()
{
	A = RM( DE );
	DE++;
}

/* 2d: 0010 1101 dddd dddd */
void upd7810_device::LDAX_Hp()
{
	A = RM( HL );
	HL++;
}

/* 2e: 0010 1110 dddd dddd */
void upd7810_device::LDAX_Dm()
{
	A = RM( DE );
	DE--;
}

/* 2f: 0010 1111 dddd dddd */
void upd7810_device::LDAX_Hm()
{
	A = RM( HL );
	HL--;
}

/* 30: 0011 0000 oooo oooo */
void upd7810_device::DCRW_wa()
{
	PAIR ea = m_va;
	UINT8 tmp, m;

	RDOPARG( ea.b.l );
	m = RM( ea.d );
	tmp = m - 1;
	ZHC_SUB( tmp, m, 0 );
	WM( ea.d, tmp );
	SKIP_CY;
}

/* 31: 0011 0001 */
void upd7810_device::BLOCK()
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
void upd7810_device::INX_HL()
{
	HL++;
}

/* 33: 0011 0011 */
void upd7810_device::DCX_HL()
{
	HL--;
}

/* 34: 0011 0100 llll llll hhhh hhhh */
void upd7810_device::LXI_H_w()
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
void upd7810_device::LTIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::STAX_B()
{
	WM( BC, A );
}

/* 3a: 0011 1010 */
void upd7810_device::STAX_D()
{
	WM( DE, A );
}

/* 3b: 0011 1011 */
void upd7810_device::STAX_H()
{
	WM( HL, A );
}

/* 3c: 0011 1100 */
void upd7810_device::STAX_Dp()
{
	WM( DE, A );
	DE++;
}

/* 3d: 0011 1101 */
void upd7810_device::STAX_Hp()
{
	WM( HL, A );
	HL++;
}

/* 3e: 0011 1110 */
void upd7810_device::STAX_Dm()
{
	WM( DE, A );
	DE--;
}

/* 3f: 0011 1111 */
void upd7810_device::STAX_Hm()
{
	WM( HL, A );
	HL--;
}

/* 40: 0100 0000 llll llll hhhh hhhh */
void upd7810_device::CALL_w()
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
}

/* 41: 0100 0001 */
void upd7810_device::INR_A()
{
	UINT8 tmp = A + 1;
	ZHC_ADD( tmp, A, 0 );
	A = tmp;
	SKIP_CY;
}

/* 42: 0100 0010 */
void upd7810_device::INR_B()
{
	UINT8 tmp = B + 1;
	ZHC_ADD( tmp, B, 0 );
	B = tmp;
	SKIP_CY;
}

/* 43: 0100 0011 */
void upd7810_device::INR_C()
{
	UINT8 tmp = C + 1;
	ZHC_ADD( tmp, C, 0 );
	C = tmp;
	SKIP_CY;
}

/* 44: 0100 0100 llll llll hhhh hhhh */
void upd7810_device::LXI_EA_s()
{
	RDOPARG( EAL );
	RDOPARG( EAH );
}

/* 45: 0100 0101 oooo oooo xxxx xxxx */
void upd7810_device::ONIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::PRE_48()
{
	RDOP(OP2);
	m_icount -= m_op48[OP2].cycles;
	handle_timers(m_op48[OP2].cycles);
	(this->*m_op48[OP2].opfunc)();
}

/* 49: 0100 1001 xxxx xxxx */
void upd7810_device::MVIX_BC_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WM( BC, imm );
}

/* 4a: 0100 1010 xxxx xxxx */
void upd7810_device::MVIX_DE_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WM( DE, imm );
}

/* 4b: 0100 1011 xxxx xxxx */
void upd7810_device::MVIX_HL_xx()
{
	UINT8 imm;
	RDOPARG( imm );
	WM( HL, imm );
}

/* 4c: prefix */
void upd7810_device::PRE_4C()
{
	RDOP(OP2);
	m_icount -= m_op4C[OP2].cycles;
	handle_timers(m_op4C[OP2].cycles);
	(this->*m_op4C[OP2].opfunc)();
}

/* 4d: prefix */
void upd7810_device::PRE_4D()
{
	RDOP(OP2);
	m_icount -= m_op4D[OP2].cycles;
	handle_timers(m_op4D[OP2].cycles);
	(this->*m_op4D[OP2].opfunc)();
}

/* 4e: 0100 111d dddd dddd */
void upd7810_device::JRE()
{
	UINT8 offs;
	RDOPARG( offs );
	if (OP & 0x01)
		PC -= 256 - offs;
	else
		PC += offs;
}

/* 50: 0101 0000 */
void upd7810_device::EXH()
{
	UINT16 tmp;
	tmp = HL; HL = HL2; HL2 = tmp;
}

/* 51: 0101 0001 */
void upd7810_device::DCR_A()
{
	UINT8 tmp = A - 1;
	ZHC_SUB( tmp, A, 0 );
	A = tmp;
	SKIP_CY;
}

/* 52: 0101 0010 */
void upd7810_device::DCR_B()
{
	UINT8 tmp = B - 1;
	ZHC_SUB( tmp, B, 0 );
	B = tmp;
	SKIP_CY;
}

/* 53: 0101 0011 */
void upd7810_device::DCR_C()
{
	UINT8 tmp = C - 1;
	ZHC_SUB( tmp, C, 0 );
	C = tmp;
	SKIP_CY;
}

/* 54: 0101 0100 llll llll hhhh hhhh */
void upd7810_device::JMP_w()
{
	PAIR w;
	w.d = 0;

	RDOPARG( w.b.l );
	RDOPARG( w.b.h );

	PCD = w.d;
}

/* 55: 0101 0101 oooo oooo xxxx xxxx */
void upd7810_device::OFFIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::BIT_0_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x01)
		PSW |= SK;
}

/* 59: 0101 1001 oooo oooo (7810 only) */
void upd7810_device::BIT_1_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x02)
		PSW |= SK;
}

/* 5a: 0101 1010 oooo oooo (7810 only) */
void upd7810_device::BIT_2_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x04)
		PSW |= SK;
}

/* 5b: 0101 1011 oooo oooo (7810 only) */
void upd7810_device::BIT_3_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x08)
		PSW |= SK;
}

/* 5c: 0101 1100 oooo oooo (7810 only) */
void upd7810_device::BIT_4_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x10)
		PSW |= SK;
}

/* 5d: 0101 1101 oooo oooo (7810 only) */
void upd7810_device::BIT_5_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x20)
		PSW |= SK;
}

/* 5e: 0101 1110 oooo oooo (7810 only) */
void upd7810_device::BIT_6_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x40)
		PSW |= SK;
}

/* 5f: 0101 1111 oooo oooo (7810 only) */
void upd7810_device::BIT_7_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	if (RM( ea.d ) & 0x80)
		PSW |= SK;
}

/* 5d: 0101 1111 bbbb bbbb (7807 only) */
void upd7810_device::SKN_bit()
{
	UINT8 imm;
	int val;

	RDOPARG( imm );

	switch( imm & 0x1f )
	{
		case 0x10:  /* PA */
			val = RP( UPD7810_PORTA );
			break;
		case 0x11:  /* PB */
			val = RP( UPD7810_PORTB );
			break;
		case 0x12:  /* PC */
			val = RP( UPD7810_PORTC );
			break;
		case 0x13:  /* PD */
			val = RP( UPD7810_PORTD );
			break;
		case 0x15:  /* PF */
			val = RP( UPD7810_PORTF );
			break;
		case 0x16:  /* MKH */
			val = MKH;
			break;
		case 0x17:  /* MKL */
			val = MKL;
			break;
		case 0x19:  /* SMH */
			val = SMH;
			break;
		case 0x1b:  /* EOM */
			val = EOM;
			break;
		case 0x1d:  /* TMM */
			val = TMM;
			break;
		case 0x1e:  /* PT */
			val = RP( UPD7807_PORTT );
			break;
		default:
			logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", tag().c_str(), OP, imm, PC);
			val = 0;
			break;
	}

	if (~val & (1 << (imm >> 5)))
		PSW |= SK;
}

/* 58: 0101 1000 bbbb bbbb (7807 only) */
void upd7810_device::SETB()
{
	UINT8 imm;
	int bit;

	RDOPARG( imm );
	bit = imm >> 5;

	switch( imm & 0x1f )
	{
		case 0x10:  /* PA */
			WP( UPD7810_PORTA, RP( UPD7810_PORTA ) | (1 << bit));
			break;
		case 0x11:  /* PB */
			WP( UPD7810_PORTB, RP( UPD7810_PORTB ) | (1 << bit));
			break;
		case 0x12:  /* PC */
			WP( UPD7810_PORTC, RP( UPD7810_PORTC ) | (1 << bit));
			break;
		case 0x13:  /* PD */
			WP( UPD7810_PORTD, RP( UPD7810_PORTD ) | (1 << bit));
			break;
		case 0x15:  /* PF */
			WP( UPD7810_PORTF, RP( UPD7810_PORTF ) | (1 << bit));
			break;
		case 0x16:  /* MKH */
			MKH |= (1 << bit);
			break;
		case 0x17:  /* MKL */
			MKL |= (1 << bit);
			break;
		case 0x19:  /* SMH */
			SMH |= (1 << bit);
			break;
		case 0x1b:  /* EOM */
			EOM |= (1 << bit);
			break;
		case 0x1d:  /* TMM */
			TMM |= (1 << bit);
			break;
//      case 0x1e:  /* PT */
//          PT is input only
//          break;
		default:
			logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", tag().c_str(), OP, imm, PC);
			break;
	}
}

/* 5b: 0101 1011 bbbb bbbb (7807 only) */
void upd7810_device::CLR()
{
	UINT8 imm;
	int bit;

	RDOPARG( imm );
	bit = imm >> 5;

	switch( imm & 0x1f )
	{
		case 0x10:  /* PA */
			WP( UPD7810_PORTA, RP( UPD7810_PORTA ) & ~(1 << bit));
			break;
		case 0x11:  /* PB */
			WP( UPD7810_PORTB, RP( UPD7810_PORTB ) & ~(1 << bit));
			break;
		case 0x12:  /* PC */
			WP( UPD7810_PORTC, RP( UPD7810_PORTC ) & ~(1 << bit));
			break;
		case 0x13:  /* PD */
			WP( UPD7810_PORTD, RP( UPD7810_PORTD ) & ~(1 << bit));
			break;
		case 0x15:  /* PF */
			WP( UPD7810_PORTF, RP( UPD7810_PORTF ) & ~(1 << bit));
			break;
		case 0x16:  /* MKH */
			MKH &= ~(1 << bit);
			break;
		case 0x17:  /* MKL */
			MKL &= ~(1 << bit);
			break;
		case 0x19:  /* SMH */
			SMH &= ~(1 << bit);
			break;
		case 0x1b:  /* EOM */
			EOM &= ~(1 << bit);
			break;
		case 0x1d:  /* TMM */
			TMM &= ~(1 << bit);
			break;
//      case 0x1e:  /* PT */
//          PT is input only
//          break;
		default:
			logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", tag().c_str(), OP, imm, PC);
			break;
	}
}

/* 5d: 0101 1111 bbbb bbbb (7807 only) */
void upd7810_device::SK_bit()
{
	UINT8 imm;
	int val;

	RDOPARG( imm );

	switch( imm & 0x1f )
	{
		case 0x10:  /* PA */
			val = RP( UPD7810_PORTA );
			break;
		case 0x11:  /* PB */
			val = RP( UPD7810_PORTB );
			break;
		case 0x12:  /* PC */
			val = RP( UPD7810_PORTC );
			break;
		case 0x13:  /* PD */
			val = RP( UPD7810_PORTD );
			break;
		case 0x15:  /* PF */
			val = RP( UPD7810_PORTF );
			break;
		case 0x16:  /* MKH */
			val = MKH;
			break;
		case 0x17:  /* MKL */
			val = MKL;
			break;
		case 0x19:  /* SMH */
			val = SMH;
			break;
		case 0x1b:  /* EOM */
			val = EOM;
			break;
		case 0x1d:  /* TMM */
			val = TMM;
			break;
		case 0x1e:  /* PT */
			val = RP( UPD7807_PORTT );
			break;
		default:
			logerror("uPD7810 '%s': illegal opcode %02x %02x at PC:%04x\n", tag().c_str(), OP, imm, PC);
			val = 0;
			break;
	}

	if (val & (1 << (imm >> 5)))
		PSW |= SK;
}

/* 60:*/
void upd7810_device::PRE_60()
{
	RDOP(OP2);
	m_icount -= m_op60[OP2].cycles;
	handle_timers(m_op60[OP2].cycles);
	(this->*m_op60[OP2].opfunc)();
}

/* 61: 0110 0001 */
void upd7810_device::DAA()
{
	UINT8 l = A & 0x0f, h = A >> 4, tmp, adj = 0x00, old_cy = PSW & CY;

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
	PSW |= old_cy;
	A = tmp;
}

/* 62: 0110 0010 */
void upd7810_device::RETI()
{
	PCL = RM( SPD );
	SP++;
	PCH = RM( SPD );
	SP++;
	PSW = RM( SPD );
	SP++;
}

/* 63: 0110 0011 oooo oooo */
void upd7810_device::STAW_wa()
{
	PAIR ea = m_va;

	RDOPARG( ea.b.l );

	WM( ea.d, A );
}

/* 64: prefix */
void upd7810_device::PRE_64()
{
	RDOP(OP2);
	m_icount -= m_op64[OP2].cycles;
	handle_timers(m_op64[OP2].cycles);
	(this->*m_op64[OP2].opfunc)();
}

/* 65: 0110 0101 oooo oooo xxxx xxxx */
void upd7810_device::NEIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::MVI_V_xx()
{
	RDOPARG( V );
}

/* 69: 0110 1001 xxxx xxxx */
void upd7810_device::MVI_A_xx()
{
	if (PSW & L1) { /* overlay active? */
		PC++;
		return;     /* NOP */
	}
	RDOPARG( A );
	PSW |= L1;
}

/* 6a: 0110 1010 xxxx xxxx */
void upd7810_device::MVI_B_xx()
{
	RDOPARG( B );
}

/* 6b: 0110 1011 xxxx xxxx */
void upd7810_device::MVI_C_xx()
{
	RDOPARG( C );
}

/* 6c: 0110 1100 xxxx xxxx */
void upd7810_device::MVI_D_xx()
{
	RDOPARG( D );
}

/* 6d: 0110 1101 xxxx xxxx */
void upd7810_device::MVI_E_xx()
{
	RDOPARG( E );
}

/* 6e: 0110 1110 xxxx xxxx */
void upd7810_device::MVI_H_xx()
{
	RDOPARG( H );
}

/* 6f: 0110 1111 xxxx xxxx */
void upd7810_device::MVI_L_xx()
{
	if (PSW & L0) { /* overlay active? */
		PC++;
		return;     /* NOP */
	}
	RDOPARG( L );
	PSW |= L0;
}

/* 70: prefix */
void upd7810_device::PRE_70()
{
	RDOP(OP2);
	m_icount -= m_op70[OP2].cycles;
	handle_timers(m_op70[OP2].cycles);
	(this->*m_op70[OP2].opfunc)();
}

/* 71: 0111 0001 oooo oooo xxxx xxxx */
void upd7810_device::MVIW_wa_xx()
{
	PAIR ea = m_va;
	UINT8 imm;

	RDOPARG( ea.b.l );
	RDOPARG( imm );

	WM( ea.d, imm );
}

/* 72: 0111 0010 */
void upd7810_device::SOFTI()
{
	SP--;
	WM( SPD, PSW );
	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PC = 0x0060;
}

/* 74: prefix */
void upd7810_device::PRE_74()
{
	RDOP(OP2);
	m_icount -= m_op74[OP2].cycles;
	handle_timers(m_op74[OP2].cycles);
	(this->*m_op74[OP2].opfunc)();
}

/* 75: 0111 0101 oooo oooo xxxx xxxx */
void upd7810_device::EQIW_wa_xx()
{
	PAIR ea = m_va;
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
void upd7810_device::CALF()
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
}

/* 80: 100t tttt */
void upd7810_device::CALT()
{
	PAIR w;
	w.d = 0;

	w.w.l = 0x80 + 2 * (OP & 0x1f);

	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PCL=RM(w.w.l);
	PCH=RM(w.w.l+1);
}

/* a0: 1010 0000 */
void upd7810_device::POP_VA()
{
	A = RM( SPD );
	SP++;
	V = RM( SPD );
	SP++;
}

/* a1: 1010 0001 */
void upd7810_device::POP_BC()
{
	C = RM( SPD );
	SP++;
	B = RM( SPD );
	SP++;
}

/* a2: 1010 0010 */
void upd7810_device::POP_DE()
{
	E = RM( SPD );
	SP++;
	D = RM( SPD );
	SP++;
}

/* a3: 1010 0011 */
void upd7810_device::POP_HL()
{
	L = RM( SPD );
	SP++;
	H = RM( SPD );
	SP++;
}

/* a4: 1010 0100 */
void upd7810_device::POP_EA()
{
	EAL = RM( SPD );
	SP++;
	EAH = RM( SPD );
	SP++;
}

/* a5: 1010 0101 */
void upd7810_device::DMOV_EA_BC()
{
	EA = BC;
}

/* a6: 1010 0110 */
void upd7810_device::DMOV_EA_DE()
{
	EA = DE;
}

/* a7: 1010 0111 */
void upd7810_device::DMOV_EA_HL()
{
	EA = HL;
}

/* a8: 1010 1000 */
void upd7810_device::INX_EA()
{
	EA++;
}

/* a9: 1010 1001 */
void upd7810_device::DCX_EA()
{
	EA--;
}

/* aa: 1010 1010 */
void upd7810_device::EI()
{
	IFF = 1;
}

/* ab: 1010 1011 dddd dddd */
void upd7810_device::LDAX_D_xx()
{
	UINT16 ea;
	RDOPARG( ea );
	ea += DE;
	A = RM( ea );
}

/* ac: 1010 1100 */
void upd7810_device::LDAX_H_A()
{
	UINT16 ea;
	ea = HL + A;
	A = RM( ea );
}

/* ad: 1010 1101 */
void upd7810_device::LDAX_H_B()
{
	UINT16 ea;
	ea = HL + B;
	A = RM( ea );
}

/* ae: 1010 1110 */
void upd7810_device::LDAX_H_EA()
{
	UINT16 ea;
	ea = HL + EA;
	A = RM( ea );
}

/* af: 1010 1111 dddd dddd */
void upd7810_device::LDAX_H_xx()
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	A = RM( ea );
}

/* b0: 1011 0000 */
void upd7810_device::PUSH_VA()
{
	SP--;
	WM( SPD, V );
	SP--;
	WM( SPD, A );
}

/* b1: 1011 0001 */
void upd7810_device::PUSH_BC()
{
	SP--;
	WM( SPD, B );
	SP--;
	WM( SPD, C );
}

/* b2: 1011 0010 */
void upd7810_device::PUSH_DE()
{
	SP--;
	WM( SPD, D );
	SP--;
	WM( SPD, E );
}

/* b3: 1011 0011 */
void upd7810_device::PUSH_HL()
{
	SP--;
	WM( SPD, H );
	SP--;
	WM( SPD, L );
}

/* b4: 1011 0100 */
void upd7810_device::PUSH_EA()
{
	SP--;
	WM( SPD, EAH );
	SP--;
	WM( SPD, EAL );
}

/* b5: 1011 0101 */
void upd7810_device::DMOV_BC_EA()
{
	BC = EA;
}

/* b6: 1011 0110 */
void upd7810_device::DMOV_DE_EA()
{
	DE = EA;
}

/* b7: 1011 0111 */
void upd7810_device::DMOV_HL_EA()
{
	HL = EA;
}

/* b8: 1011 1000 */
void upd7810_device::RET()
{
	PCL = RM( SPD );
	SP++;
	PCH = RM( SPD );
	SP++;
}

/* b9: 1011 1001 */
void upd7810_device::RETS()
{
	PCL = RM( SPD );
	SP++;
	PCH = RM( SPD );
	SP++;
	PSW|=SK;    /* skip one instruction */
}

/* ba: 1011 1010 */
void upd7810_device::DI()
{
	IFF = 0;
}

/* bb: 1011 1011 dddd dddd */
void upd7810_device::STAX_D_xx()
{
	UINT16 ea;
	RDOPARG(ea);
	ea += DE;
	WM( ea, A );
}

/* bc: 1011 1100 */
void upd7810_device::STAX_H_A()
{
	UINT16 ea = A;
	ea += HL;
	WM( ea, A );
}

/* bd: 1011 1101 */
void upd7810_device::STAX_H_B()
{
	UINT16 ea = B;
	ea += HL;
	WM( ea, A );
}

/* be: 1011 1110 */
void upd7810_device::STAX_H_EA()
{
	UINT16 ea = EA;
	ea += HL;
	WM( ea, A );
}

/* bf: 1011 1111 dddd dddd */
void upd7810_device::STAX_H_xx()
{
	UINT16 ea;
	RDOPARG( ea );
	ea += HL;
	WM( ea, A );
}

/* c0: 11dd dddd */
void upd7810_device::JR()
{
	INT8 offs = (INT8)(OP << 2) >> 2;
	PC += offs;
}

/*********************/
/*                   */
/* 7801 instructions */
/*                   */
/*********************/

void upd7810_device::CALT_7801()
{
	PAIR w;
	w.d = 0;

	w.w.l = 0x80 + 2 * (OP & 0x3f);

	SP--;
	WM( SPD, PCH );
	SP--;
	WM( SPD, PCL );

	PCL=RM(w.w.l);
	PCH=RM(w.w.l+1);
}

/* DCR(W) and INR(W) instructions do not modify the CY register on at least 78c05 and 78c06 */
void upd7810_device::DCR_A_7801()
{
	UINT32 old_CY = PSW & CY;
	DCR_A();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::DCR_B_7801()
{
	UINT32 old_CY = PSW & CY;
	DCR_B();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::DCR_C_7801()
{
	UINT32 old_CY = PSW & CY;
	DCR_C();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::DCRW_wa_7801()
{
	UINT32 old_CY = PSW & CY;
	DCRW_wa();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::INR_A_7801()
{
	UINT32 old_CY = PSW & CY;
	INR_A();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::INR_B_7801()
{
	UINT32 old_CY = PSW & CY;
	INR_B();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::INR_C_7801()
{
	UINT32 old_CY = PSW & CY;
	INR_C();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::INRW_wa_7801()
{
	UINT32 old_CY = PSW & CY;
	INRW_wa();
	PSW = ( PSW & ~CY ) | old_CY;
}

void upd7810_device::IN()
{
	logerror("unimplemented instruction: IN\n");
}

void upd7810_device::OUT()
{
	logerror("unimplemented instruction: OUT\n");
}

void upd7810_device::MOV_A_S()
{
	logerror("unimplemented instruction: MOV_A_S\n");
}

void upd7810_device::MOV_S_A()
{
	logerror("unimplemented instruction: MOV_A_S\n");
}

void upd7810_device::PEN()
{
	logerror("unimplemented instruction: PEN\n");
}

void upd7810_device::PER()
{
	logerror("unimplemented instruction: PER\n");
}

void upd7810_device::PEX()
{
	logerror("unimplemented instruction: PEX\n");
}

void upd7810_device::SIO()
{
	logerror("unimplemented instruction: SIO\n");
}

void upd7810_device::SKIT_F0()
{
	if (IRR & INTF0)
		PSW |= SK;
	IRR &= ~INTF0;
}

void upd7810_device::SKNIT_F0()
{
	logerror("unimplemented instruction: SKNIT_F0\n");
}

void upd7810_device::STM()
{
	m_ovc0 = ( ( TMM & 0x04 ) ? 16 * 8 : 8 ) * TM0;
}

void upd7810_device::STM_7801()
{
	/* Set the timer flip/fliop */
	TO = 1;
	m_to_func(TO);

	/* Reload the timer */
	m_ovc0 = 16 * ( TM0 + ( ( TM1 & 0x0f ) << 8 ) );
}

void upd7810_device::MOV_MC_A_7801()
{
	/* On the 7801 the mode C bits function as follows: */
	/*       Cn=1   Cn=0         */
	/* PC0  Input   Output       */
	/* PC1  Input   Output       */
	/* PC2  Input   -SCS Input   */
	/* PC3  Output  SAK Output   */
	/* PC4  Output  To Output    */
	/* PC5  Output  IO/-M Output */
	/* PC6  Output  HLDA Output  */
	/* PC7  Input   HOLD Input   */
	MC = 0x84 | ( ( A & 0x02 ) ? 0x02 : 0x00 ) | ( ( A & 0x01 ) ? 0x01 : 0x00 );
}
