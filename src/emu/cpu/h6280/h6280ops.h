/*****************************************************************************

    h6280ops.h - Addressing modes and opcode macros for the Hu6820 cpu

    Copyright Bryan McPhail, mish@tendril.co.uk

    This source code is based (with permission!) on the 6502 emulator by
    Juergen Buchmueller.  It is released as part of the Mame emulator project.
    Let me know if you intend to use this code in any other project.

******************************************************************************/

/* 6280 flags */
#define _fC 0x01
#define _fZ 0x02
#define _fI 0x04
#define _fD 0x08
#define _fB 0x10
#define _fT 0x20
#define _fV 0x40
#define _fN 0x80

/* some shortcuts for improved readability */
#define A   m_a
#define X   m_x
#define Y   m_y
#define P   m_p
#define S   m_sp.b.l

#define TRANSLATED(addr)    ((m_mmr[((addr) >> 13) & 7] << 13) | ((addr) & 0x1fff))
#define H6280_CYCLES(cyc)                               \
	{                                                   \
		m_icount -= ((cyc) * m_clocks_per_cycle);       \
		m_timer_value -= ((cyc) * m_clocks_per_cycle);  \
	}

#if LAZY_FLAGS

#define NZ  m_NZ
#define SET_NZ(n)               \
	P &= ~_fT;                  \
	NZ = ((n & _fN) << 8) | n

#else

#define SET_NZ(n)               \
	P = (P & ~(_fN|_fT|_fZ)) |  \
		(n & _fN) |             \
		((n == 0) ? _fZ : 0)

#endif

#define EAL m_ea.b.l
#define EAH m_ea.b.h
#define EAW m_ea.w.l
#define EAD m_ea.d

#define ZPL m_zp.b.l
#define ZPH m_zp.b.h
#define ZPW m_zp.w.l
#define ZPD m_zp.d

#define PCL m_pc.b.l
#define PCH m_pc.b.h
#define PCW m_pc.w.l
#define PCD m_pc.d

#define CLEAR_T  \
	P &= ~_fT;

#define DO_INTERRUPT(vector)                                    \
{                                                               \
	H6280_CYCLES(7);    /* 7 cycles for an int */               \
	push(PCH);                                                  \
	push(PCL);                                                  \
	COMPOSE_P(0, _fB);                                          \
	push(P);                                                    \
	P = (P & ~_fD) | _fI;   /* knock out D and set I flag */    \
	PCL = program_read8(vector);                                \
	PCH = program_read8(vector + 1);                            \
}

#define CHECK_AND_TAKE_IRQ_LINES                \
	if ( m_nmi_state != CLEAR_LINE ) {          \
		m_nmi_state = CLEAR_LINE;               \
		DO_INTERRUPT(H6280_NMI_VEC);            \
	}                                           \
	else if( !(P & _fI) )                       \
	{                                           \
		if ( m_irq_state[2] != CLEAR_LINE &&    \
				!(m_irq_mask & 0x4) )               \
		{                                       \
			DO_INTERRUPT(H6280_TIMER_VEC);      \
		} else                                  \
		if ( m_irq_state[0] != CLEAR_LINE &&    \
				!(m_irq_mask & 0x2) )               \
		{                                       \
			DO_INTERRUPT(H6280_IRQ1_VEC);       \
			standard_irq_callback(0);           \
		} else                                  \
		if ( m_irq_state[1] != CLEAR_LINE &&    \
				!(m_irq_mask & 0x1) )               \
		{                                       \
			DO_INTERRUPT(H6280_IRQ2_VEC);       \
			standard_irq_callback(1);           \
		}                                       \
	}

#define CHECK_IRQ_LINES                         \
	if (!m_irq_pending)                         \
		m_irq_pending = 2;

/***************************************************************
 *  CHECK_VDC_VCE_PENALTY
 * The CPU inserts 1 clock delay when accessing the VDC or VCE
 * area.
 ***************************************************************/
#define CHECK_VDC_VCE_PENALTY(addr)                             \
	if ( ( TRANSLATED(addr) & 0x1FF800 ) == 0x1FE000 ) {        \
		H6280_CYCLES(1);                                        \
	}

/***************************************************************
 *  BRA  branch relative
 ***************************************************************/
#define BRA(cond)                                               \
	CLEAR_T;                                                    \
	if (cond)                                                   \
	{                                                           \
		H6280_CYCLES(4);                                        \
		tmp = read_opcode_arg();                                \
		PCW++;                                                  \
		EAW = PCW + (signed char)tmp;                           \
		PCD = EAD;                                              \
	}                                                           \
	else                                                        \
	{                                                           \
		PCW++;                                                  \
		H6280_CYCLES(2);                                        \
	}

/***************************************************************
 *
 * Helper macros to build the effective address
 *
 ***************************************************************/

/***************************************************************
 *  EA = zero page address
 ***************************************************************/
#define EA_ZPG                                                  \
	ZPL = read_opcode_arg();                                    \
	PCW++;                                                      \
	EAD = ZPD

/***************************************************************
 *  EA = zero page address - T flag
 ***************************************************************/
#define EA_TFLG                                                 \
	ZPL = X;                                                    \
	EAD = ZPD

/***************************************************************
 *  EA = zero page address + X
 ***************************************************************/
#define EA_ZPX                                                  \
	ZPL = read_opcode_arg() + X;                                \
	PCW++;                                                      \
	EAD = ZPD

/***************************************************************
 *  EA = zero page address + Y
 ***************************************************************/
#define EA_ZPY                                                  \
	ZPL = read_opcode_arg() + Y;                                \
	PCW++;                                                      \
	EAD = ZPD

/***************************************************************
 *  EA = absolute address
 ***************************************************************/
#define EA_ABS                                                  \
	EAL = read_opcode_arg();                                    \
	PCW++;                                                      \
	EAH = read_opcode_arg();                                    \
	PCW++

/***************************************************************
 *  EA = absolute address + X
 ***************************************************************/
#define EA_ABX                                                  \
	EA_ABS;                                                     \
	EAW += X

/***************************************************************
 *  EA = absolute address + Y
 ***************************************************************/
#define EA_ABY                                                  \
	EA_ABS;                                                     \
	EAW += Y

/***************************************************************
 *  EA = zero page indirect (65c02 pre indexed w/o X)
 ***************************************************************/
#define EA_ZPI                                                  \
	ZPL = read_opcode_arg();                                    \
	PCW++;                                                      \
	EAD = program_read16z(ZPD);

/***************************************************************
 *  EA = zero page + X indirect (pre indexed)
 ***************************************************************/
#define EA_IDX                                                  \
	ZPL = read_opcode_arg() + X;                                \
	PCW++;                                                      \
	EAD = program_read16z(ZPD);

/***************************************************************
 *  EA = zero page indirect + Y (post indexed)
 ***************************************************************/
#define EA_IDY                                                  \
	ZPL = read_opcode_arg();                                    \
	PCW++;                                                      \
	EAD = program_read16z(ZPD);                                 \
	EAW += Y

/***************************************************************
 *  EA = indirect (only used by JMP)
 ***************************************************************/
#define EA_IND                                                  \
	EA_ABS;                                                     \
	tmp = program_read8(EAD);                                   \
	EAD++;                                                      \
	EAH = program_read8(EAD);                                   \
	EAL = tmp

/***************************************************************
 *  EA = indirect plus x (only used by JMP)
 ***************************************************************/
#define EA_IAX                                                  \
	EA_ABS;                                                     \
	EAD+=X;                                                     \
	tmp = program_read8(EAD);                                   \
	EAD++;                                                      \
	EAH = program_read8(EAD);                                   \
	EAL = tmp

/* read a value into tmp */
#define RD_IMM  tmp = read_opcode_arg(); PCW++
#define RD_IMM2 tmp2 = read_opcode_arg(); PCW++
#define RD_ACC  tmp = A
#define RD_ZPG  EA_ZPG; tmp = program_read8z(EAD)
#define RD_ZPX  EA_ZPX; tmp = program_read8z(EAD)
#define RD_ZPY  EA_ZPY; tmp = program_read8z(EAD)
#define RD_ABS  EA_ABS; tmp = program_read8(EAD)
#define RD_ABX  EA_ABX; tmp = program_read8(EAD)
#define RD_ABY  EA_ABY; tmp = program_read8(EAD)
#define RD_ZPI  EA_ZPI; tmp = program_read8(EAD)
#define RD_IDX  EA_IDX; tmp = program_read8(EAD)
#define RD_IDY  EA_IDY; tmp = program_read8(EAD)
#define RD_TFL  EA_TFLG; tflagtemp = program_read8z(EAD)

/* write a value from tmp */
#define WR_ZPG  EA_ZPG; program_write8z(EAD, tmp)
#define WR_ZPX  EA_ZPX; program_write8z(EAD, tmp)
#define WR_ZPY  EA_ZPY; program_write8z(EAD, tmp)
#define WR_ABS  EA_ABS; program_write8(EAD, tmp)
#define WR_ABX  EA_ABX; program_write8(EAD, tmp)
#define WR_ABY  EA_ABY; program_write8(EAD, tmp)
#define WR_ZPI  EA_ZPI; program_write8(EAD, tmp)
#define WR_IDX  EA_IDX; program_write8(EAD, tmp)
#define WR_IDY  EA_IDY; program_write8(EAD, tmp)

/* write back a value from tmp to the last EA */
#define WB_ACC  A = (UINT8)tmp;
#define WB_EA   program_write8(EAD, tmp)
#define WB_EAZ  program_write8z(EAD, tmp)
#define WB_TFL  program_write8z(EAD, tflagtemp)

/***************************************************************
 *
 * Macros to emulate the 6280 opcodes
 *
 ***************************************************************/

/***************************************************************
 * compose the real flag register by
 * including N and Z and set any
 * SET and clear any CLR bits also
 ***************************************************************/
#if LAZY_FLAGS

#define COMPOSE_P(SET,CLR)                                      \
	P = (P & ~(_fN | _fZ | CLR)) |                              \
		(NZ >> 8) |                                             \
		((NZ & 0xff) ? 0 : _fZ) |                               \
		SET

#else

#define COMPOSE_P(SET,CLR)                                      \
	P = (P & ~CLR) | SET

#endif

/* 6280 ********************************************************
 *  ADC Add with carry
 ***************************************************************/
#define TADC                                                    \
	{                                                           \
		int tflagtemp;                                          \
		CLEAR_T;                                                \
		RD_TFL;                                                 \
		if (P & _fD)                                            \
		{                                                       \
		int c = (P & _fC);                                      \
		int lo = (tflagtemp & 0x0f) + (tmp & 0x0f) + c;         \
		int hi = (tflagtemp & 0xf0) + (tmp & 0xf0);             \
			P &= ~_fC;                                          \
			if (lo > 0x09)                                      \
			{                                                   \
				hi += 0x10;                                     \
				lo += 0x06;                                     \
			}                                                   \
			if (hi > 0x90)                                      \
				hi += 0x60;                                     \
			if (hi & 0xff00)                                    \
				P |= _fC;                                       \
			tflagtemp = (lo & 0x0f) + (hi & 0xf0);              \
			H6280_CYCLES(1);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
		int c = (P & _fC);                                      \
		int sum = tflagtemp + tmp + c;                          \
			P &= ~(_fV | _fC);                                  \
			if (~(tflagtemp^tmp) & (tflagtemp^sum) & _fN)       \
				P |= _fV;                                       \
			if (sum & 0xff00)                                   \
				P |= _fC;                                       \
			tflagtemp = (UINT8) sum;                            \
		}                                                       \
		SET_NZ(tflagtemp);                                      \
		WB_TFL;                                                 \
		H6280_CYCLES(3);                                        \
	}


#define ADC                                                     \
	if(P & _fT)                                                 \
		TADC                                                    \
	else {                                                      \
		if (P & _fD)                                            \
		{                                                       \
			int c = (P & _fC);                                  \
			int lo = (A & 0x0f) + (tmp & 0x0f) + c;             \
			int hi = (A & 0xf0) + (tmp & 0xf0);                 \
			P &= ~_fC;                                          \
			if (lo > 0x09)                                      \
			{                                                   \
				hi += 0x10;                                     \
				lo += 0x06;                                     \
			}                                                   \
			if (hi > 0x90)                                      \
				hi += 0x60;                                     \
			if (hi & 0xff00)                                    \
				P |= _fC;                                       \
			A = (lo & 0x0f) + (hi & 0xf0);                      \
			H6280_CYCLES(1);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			int c = (P & _fC);                                  \
			int sum = A + tmp + c;                              \
			P &= ~(_fV | _fC);                                  \
			if (~(A^tmp) & (A^sum) & _fN)                       \
				P |= _fV;                                       \
			if (sum & 0xff00)                                   \
				P |= _fC;                                       \
			A = (UINT8) sum;                                    \
		}                                                       \
		SET_NZ(A);                                              \
	}

/* 6280 ********************************************************
 *  AND Logical and
 ***************************************************************/
#define TAND                                                    \
	{                                                           \
		int tflagtemp;                                          \
		CLEAR_T;                                                \
		RD_TFL;                                                 \
		tflagtemp = (UINT8)(tflagtemp & tmp);                   \
		WB_TFL;                                                 \
		SET_NZ(tflagtemp);                                      \
		H6280_CYCLES(3);                                        \
	}

#define AND                                                     \
	if(P & _fT)                                                 \
		TAND                                                    \
	else {                                                      \
		A = (UINT8)(A & tmp);                                   \
		SET_NZ(A);                                              \
	}

/* 6280 ********************************************************
 *  ASL Arithmetic shift left
 ***************************************************************/
#define ASL                                                     \
	CLEAR_T;                                                    \
	P = (P & ~_fC) | ((tmp >> 7) & _fC);                        \
	tmp = (UINT8)(tmp << 1);                                    \
	SET_NZ(tmp)

/* 6280 ********************************************************
 *  BBR Branch if bit is reset
 ***************************************************************/
#define BBR(bit)                                                \
	BRA(!(tmp & (1<<bit)))

/* 6280 ********************************************************
 *  BBS Branch if bit is set
 ***************************************************************/
#define BBS(bit)                                                \
	BRA(tmp & (1<<bit))

/* 6280 ********************************************************
 *  BCC Branch if carry clear
 ***************************************************************/
#define BCC BRA(!(P & _fC))

/* 6280 ********************************************************
 *  BCS Branch if carry set
 ***************************************************************/
#define BCS BRA(P & _fC)

/* 6280 ********************************************************
 *  BEQ Branch if equal
 ***************************************************************/
#if LAZY_FLAGS
#define BEQ BRA(!(NZ & 0xff))
#else
#define BEQ BRA(P & _fZ)
#endif

/* 6280 ********************************************************
 *  BIT Bit test
 ***************************************************************/
#define HBIT                                                    \
	P = (P & ~(_fN|_fV|_fT|_fZ))                                \
		| ((tmp&0x80) ? _fN:0)                                  \
		| ((tmp&0x40) ? _fV:0)                                  \
		| ((tmp&A)  ? 0:_fZ)

/* 6280 ********************************************************
 *  BMI Branch if minus
 ***************************************************************/
#if LAZY_FLAGS
#define BMI BRA(NZ & 0x8000)
#else
#define BMI BRA(P & _fN)
#endif

/* 6280 ********************************************************
 *  BNE Branch if not equal
 ***************************************************************/
#if LAZY_FLAGS
#define BNE BRA(NZ & 0xff)
#else
#define BNE BRA(!(P & _fZ))
#endif

/* 6280 ********************************************************
 *  BPL Branch if plus
 ***************************************************************/
#if LAZY_FLAGS
#define BPL BRA(!(NZ & 0x8000))
#else
#define BPL BRA(!(P & _fN))
#endif

/* 6280 ********************************************************
 *  BRK Break
 *  increment PC, push PC hi, PC lo, flags (with B bit set),
 *  set I flag, reset D flag and jump via IRQ vector
 ***************************************************************/
#define BRK                                                     \
	logerror("BRK %04x\n",PCW);                                 \
	CLEAR_T;                                                    \
	PCW++;                                                      \
	push(PCH);                                                  \
	push(PCL);                                                  \
	push(P);                                                    \
	P = (P & ~_fD) | _fI;                                       \
	PCL = program_read8(H6280_IRQ2_VEC);                        \
	PCH = program_read8(H6280_IRQ2_VEC+1);
/* 6280 ********************************************************
 *  BSR Branch to subroutine
 ***************************************************************/
#define BSR                                                     \
	push(PCH);                                                  \
	push(PCL);                                                  \
	H6280_CYCLES(4); /* 4 cycles here, 4 in BRA */              \
	BRA(1)

/* 6280 ********************************************************
 *  BVC Branch if overflow clear
 ***************************************************************/
#define BVC BRA(!(P & _fV))

/* 6280 ********************************************************
 *  BVS Branch if overflow set
 ***************************************************************/
#define BVS BRA(P & _fV)

/* 6280 ********************************************************
 *  CLA Clear accumulator
 ***************************************************************/
#define CLA                                                     \
	CLEAR_T;                                                    \
	A = 0

/* 6280 ********************************************************
 *  CLC Clear carry flag
 ***************************************************************/
#define CLC                                                     \
	CLEAR_T;                                                    \
	P &= ~_fC

/* 6280 ********************************************************
 *  CLD Clear decimal flag
 ***************************************************************/
#define CLD                                                     \
	CLEAR_T;                                                    \
	P &= ~_fD

/* 6280 ********************************************************
 *  CLI Clear interrupt flag
 ***************************************************************/
#define CLI                                                     \
	CLEAR_T;                                                    \
	if( P & _fI )                                               \
	{                                                           \
		P &= ~_fI;                                              \
		CHECK_IRQ_LINES;                                        \
	}


/* 6280 ********************************************************
 *  CLV Clear overflow flag
 ***************************************************************/
#define CLV                                                     \
	CLEAR_T;                                                    \
	P &= ~_fV

/* 6280 ********************************************************
 *  CLX Clear index X
 ***************************************************************/
#define CLX                                                     \
	CLEAR_T;                                                    \
	X = 0

/* 6280 ********************************************************
 *  CLY Clear index Y
 ***************************************************************/
#define CLY                                                     \
	CLEAR_T;                                                    \
	Y = 0

/* 6280 ********************************************************
 *  CMP Compare accumulator
 ***************************************************************/
#define CMP                                                     \
	CLEAR_T;                                                    \
	P &= ~_fC;                                                  \
	if (A >= tmp)                                               \
		P |= _fC;                                               \
	SET_NZ((UINT8)(A - tmp))

/* 6280 ********************************************************
 *  CPX Compare index X
 ***************************************************************/
#define CPX                                                     \
	CLEAR_T;                                                    \
	P &= ~_fC;                                                  \
	if (X >= tmp)                                               \
		P |= _fC;                                               \
	SET_NZ((UINT8)(X - tmp))

/* 6280 ********************************************************
 *  CPY Compare index Y
 ***************************************************************/
#define CPY                                                     \
	CLEAR_T;                                                    \
	P &= ~_fC;                                                  \
	if (Y >= tmp)                                               \
		P |= _fC;                                               \
	SET_NZ((UINT8)(Y - tmp))

/* 6280 ********************************************************
 *  DEA Decrement accumulator
 ***************************************************************/
#define DEA                                                     \
	CLEAR_T;                                                    \
	A = (UINT8)(A - 1);                                         \
	SET_NZ(A)

/* 6280 ********************************************************
 *  DEC Decrement memory
 ***************************************************************/
#define DEC                                                     \
	CLEAR_T;                                                    \
	tmp = (UINT8)(tmp-1);                                       \
	SET_NZ(tmp)

/* 6280 ********************************************************
 *  DEX Decrement index X
 ***************************************************************/
#define DEX                                                     \
	CLEAR_T;                                                    \
	X = (UINT8)(X - 1);                                         \
	SET_NZ(X)

/* 6280 ********************************************************
 *  DEY Decrement index Y
 ***************************************************************/
#define DEY                                                     \
	CLEAR_T;                                                    \
	Y = (UINT8)(Y - 1);                                         \
	SET_NZ(Y)

/* 6280 ********************************************************
 *  EOR Logical exclusive or
 ***************************************************************/
#define TEOR                                                    \
	{                                                           \
		int tflagtemp;                                          \
		CLEAR_T;                                                \
		RD_TFL;                                                 \
		tflagtemp = (UINT8)(tflagtemp ^ tmp);                   \
		WB_TFL;                                                 \
		SET_NZ(tflagtemp);                                      \
		H6280_CYCLES(3);                                        \
	}

#define EOR                                                     \
	if(P & _fT)                                                 \
		TEOR                                                    \
	else {                                                      \
		A = (UINT8)(A ^ tmp);                                   \
		SET_NZ(A);                                              \
	}

/* 6280 ********************************************************
 *  INA Increment accumulator
 ***************************************************************/
#define INA                                                     \
	CLEAR_T;                                                    \
	A = (UINT8)(A + 1);                                         \
	SET_NZ(A)

/* 6280 ********************************************************
 *  INC Increment memory
 ***************************************************************/
#define INC                                                     \
	CLEAR_T;                                                    \
	tmp = (UINT8)(tmp+1);                                       \
	SET_NZ(tmp)

/* 6280 ********************************************************
 *  INX Increment index X
 ***************************************************************/
#define INX                                                     \
	CLEAR_T;                                                    \
	X = (UINT8)(X + 1);                                         \
	SET_NZ(X)

/* 6280 ********************************************************
 *  INY Increment index Y
 ***************************************************************/
#define INY                                                     \
	CLEAR_T;                                                    \
	Y = (UINT8)(Y + 1);                                         \
	SET_NZ(Y)

/* 6280 ********************************************************
 *  JMP Jump to address
 *  set PC to the effective address
 ***************************************************************/
#define JMP                                                     \
	CLEAR_T;                                                    \
	PCD = EAD;
/* 6280 ********************************************************
 *  JSR Jump to subroutine
 *  decrement PC (sic!) push PC hi, push PC lo and set
 *  PC to the effective address
 ***************************************************************/
#define JSR                                                     \
	CLEAR_T;                                                    \
	PCW--;                                                      \
	push(PCH);                                                  \
	push(PCL);                                                  \
	PCD = EAD;
/* 6280 ********************************************************
 *  LDA Load accumulator
 ***************************************************************/
#define LDA                                                     \
	CLEAR_T;                                                    \
	A = (UINT8)tmp;                                             \
	SET_NZ(A)

/* 6280 ********************************************************
 *  LDX Load index X
 ***************************************************************/
#define LDX                                                     \
	CLEAR_T;                                                    \
	X = (UINT8)tmp;                                             \
	SET_NZ(X)

/* 6280 ********************************************************
 *  LDY Load index Y
 ***************************************************************/
#define LDY                                                     \
	CLEAR_T;                                                    \
	Y = (UINT8)tmp;                                             \
	SET_NZ(Y)

/* 6280 ********************************************************
 *  LSR Logic shift right
 *  0 -> [7][6][5][4][3][2][1][0] -> C
 ***************************************************************/
#define LSR                                                     \
	CLEAR_T;                                                    \
	P = (P & ~_fC) | (tmp & _fC);                               \
	tmp = (UINT8)tmp >> 1;                                      \
	SET_NZ(tmp)

/* 6280 ********************************************************
 *  NOP No operation
 ***************************************************************/
#define NOP CLEAR_T;

/* 6280 ********************************************************
 *  ORA Logical inclusive or
 ***************************************************************/

#define TORA                                                    \
	{                                                           \
		int tflagtemp;                                          \
		CLEAR_T;                                                \
		RD_TFL;                                                 \
		tflagtemp = (UINT8)(tflagtemp | tmp);                   \
		WB_TFL;                                                 \
		SET_NZ(tflagtemp);                                      \
		H6280_CYCLES(3);                                        \
	}

#define ORA                                                     \
	if(P & _fT)                                                 \
		TORA                                                    \
	else {                                                      \
		A = (UINT8)(A | tmp);                                   \
		SET_NZ(A);                                              \
	}

/* 6280 ********************************************************
 *  PHA Push accumulator
 ***************************************************************/
#define PHA                                                     \
	CLEAR_T;                                                    \
	push(A)

/* 6280 ********************************************************
 *  PHP Push processor status (flags)
 ***************************************************************/
#define PHP                                                     \
	CLEAR_T;                                                    \
	COMPOSE_P(0,0);                                             \
	push(P)

/* 6280 ********************************************************
 *  PHX Push index X
 ***************************************************************/
#define PHX                                                     \
	CLEAR_T;                                                    \
	push(X)

/* 6280 ********************************************************
 *  PHY Push index Y
 ***************************************************************/
#define PHY                                                     \
	CLEAR_T;                                                    \
	push(Y)

/* 6280 ********************************************************
 *  PLA Pull accumulator
 ***************************************************************/
#define PLA                                                     \
	CLEAR_T;                                                    \
	pull(A);                                                    \
	SET_NZ(A)

/* 6280 ********************************************************
 *  PLP Pull processor status (flags)
 ***************************************************************/
#if LAZY_FLAGS

#define PLP                                                     \
	pull(P);                                                    \
	P |= _fB;                                                   \
	NZ = ((P & _fN) << 8) |                                     \
			((P & _fZ) ^ _fZ);                                  \
	CHECK_IRQ_LINES

#else

#define PLP                                                     \
	pull(P);                                                    \
	P |= _fB;                                                   \
	CHECK_IRQ_LINES
#endif

/* 6280 ********************************************************
 *  PLX Pull index X
 ***************************************************************/
#define PLX                                                     \
	CLEAR_T;                                                    \
	pull(X);                                                    \
	SET_NZ(X)

/* 6280 ********************************************************
 *  PLY Pull index Y
 ***************************************************************/
#define PLY                                                     \
	CLEAR_T;                                                    \
	pull(Y);                                                    \
	SET_NZ(Y)

/* 6280 ********************************************************
 *  RMB Reset memory bit
 ***************************************************************/
#define RMB(bit)                                                \
	CLEAR_T;                                                    \
	tmp &= ~(1<<bit)

/* 6280 ********************************************************
 *  ROL Rotate left
 *  new C <- [7][6][5][4][3][2][1][0] <- C
 ***************************************************************/
#define ROL                                                     \
	CLEAR_T;                                                    \
	tmp = (tmp << 1) | (P & _fC);                               \
	P = (P & ~_fC) | ((tmp >> 8) & _fC);                        \
	tmp = (UINT8)tmp;                                           \
	SET_NZ(tmp)

/* 6280 ********************************************************
 *  ROR Rotate right
 *  C -> [7][6][5][4][3][2][1][0] -> new C
 ***************************************************************/
#define ROR                                                     \
	CLEAR_T;                                                    \
	tmp |= (P & _fC) << 8;                                      \
	P = (P & ~_fC) | (tmp & _fC);                               \
	tmp = (UINT8)(tmp >> 1);                                    \
	SET_NZ(tmp)

/* 6280 ********************************************************
 *  RTI Return from interrupt
 *  pull flags, pull PC lo, pull PC hi and increment PC
 ***************************************************************/
#if LAZY_FLAGS

#define RTI                                                     \
	pull(P);                                                    \
	P |= _fB;                                                   \
	NZ = ((P & _fN) << 8) |                                     \
			((P & _fZ) ^ _fZ);                                  \
	pull(PCL);                                                  \
	pull(PCH);                                                  \
	CHECK_IRQ_LINES
#else

#define RTI                                                     \
	pull(P);                                                    \
	P |= _fB;                                                   \
	pull(PCL);                                                  \
	pull(PCH);                                                  \
	CHECK_IRQ_LINES
#endif

/* 6280 ********************************************************
 *  RTS Return from subroutine
 *  pull PC lo, PC hi and increment PC
 ***************************************************************/
#define RTS                                                     \
	CLEAR_T;                                                    \
	pull(PCL);                                                  \
	pull(PCH);                                                  \
	PCW++;

/* 6280 ********************************************************
 *  SAX Swap accumulator and index X
 ***************************************************************/
#define SAX                                                     \
	CLEAR_T;                                                    \
	tmp = X;                                                    \
	X = A;                                                      \
	A = tmp

/* 6280 ********************************************************
 *  SAY Swap accumulator and index Y
 ***************************************************************/
#define SAY                                                     \
	CLEAR_T;                                                    \
	tmp = Y;                                                    \
	Y = A;                                                      \
	A = tmp

/* 6280 ********************************************************
 *  SBC Subtract with carry
 ***************************************************************/
#define TSBC                                                    \
	{                                                           \
		int tflagtemp;                                          \
		CLEAR_T;                                                \
		RD_TFL;                                                 \
		if (P & _fD)                                            \
		{                                                       \
			int c = (P & _fC) ^ _fC;                            \
			int sum = tflagtemp - tmp -c;                       \
			int lo = (tflagtemp & 0x0f) - (tmp & 0x0f) - c;     \
			int hi = (tflagtemp & 0xf0) - (tmp & 0xf0);         \
			P &= ~_fC;                                          \
			if (lo & 0xf0)                                      \
				lo -= 6;                                        \
			if (lo & 0x80)                                      \
				hi -= 0x10;                                     \
			if (hi & 0x0f00)                                    \
				hi -= 0x60;                                     \
			if ((sum & 0xff00) == 0)                            \
				P |= _fC;                                       \
			tflagtemp = (lo & 0x0f) + (hi & 0xf0);              \
			H6280_CYCLES(1);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			int c = (P & _fC) ^ _fC;                            \
			int sum = tflagtemp - tmp - c;                      \
			P &= ~(_fV | _fC);                                  \
			if ((tflagtemp^tmp) & (tflagtemp^sum) & _fN)        \
				P |= _fV;                                       \
			if ((sum & 0xff00) == 0)                            \
				P |= _fC;                                       \
			tflagtemp = (UINT8) sum;                            \
		}                                                       \
		SET_NZ(tflagtemp);                                      \
		WB_TFL;                                                 \
		H6280_CYCLES(3);                                        \
	}

#define SBC                                                     \
	if(P & _fT)                                                 \
		TSBC                                                    \
	else {                                                      \
		if (P & _fD)                                            \
		{                                                       \
			int c = (P & _fC) ^ _fC;                            \
			int sum = A - tmp - c;                              \
			int lo = (A & 0x0f) - (tmp & 0x0f) - c;             \
			int hi = (A & 0xf0) - (tmp & 0xf0);                 \
			P &= ~_fC;                                          \
			if (lo & 0xf0)                                      \
				lo -= 6;                                        \
			if (lo & 0x80)                                      \
				hi -= 0x10;                                     \
			if (hi & 0x0f00)                                    \
				hi -= 0x60;                                     \
			if ((sum & 0xff00) == 0)                            \
				P |= _fC;                                       \
			A = (lo & 0x0f) + (hi & 0xf0);                      \
			H6280_CYCLES(1);                                    \
		}                                                       \
		else                                                    \
		{                                                       \
			int c = (P & _fC) ^ _fC;                            \
			int sum = A - tmp - c;                              \
			P &= ~(_fV | _fC);                                  \
			if ((A^tmp) & (A^sum) & _fN)                        \
				P |= _fV;                                       \
			if ((sum & 0xff00) == 0)                            \
				P |= _fC;                                       \
			A = (UINT8) sum;                                    \
		}                                                       \
		SET_NZ(A);                                              \
	}

/* 6280 ********************************************************
 *  SEC Set carry flag
 ***************************************************************/
#if defined(SEC)
#undef SEC
#endif
#define SEC                                                     \
	CLEAR_T;                                                    \
	P |= _fC

/* 6280 ********************************************************
 *  SED Set decimal flag
 ***************************************************************/
#define SED                                                     \
	CLEAR_T;                                                    \
	P |= _fD

/* 6280 ********************************************************
 *  SEI Set interrupt flag
 ***************************************************************/
#define SEI                                                     \
	CLEAR_T;                                                    \
	P |= _fI

/* 6280 ********************************************************
 *  SET Set t flag
 ***************************************************************/
#define SET                                                     \
	P |= _fT;

/* 6280 ********************************************************
 *  SMB Set memory bit
 ***************************************************************/
#define SMB(bit)                                                \
	CLEAR_T;                                                    \
	tmp |= (1<<bit)

/* 6280 ********************************************************
 *  ST0 Store at hardware address 0
 ***************************************************************/
#define ST0                                                     \
	CLEAR_T;                                                    \
	m_io->write_byte(0x0000,tmp)

/* 6280 ********************************************************
 *  ST1 Store at hardware address 2
 ***************************************************************/
#define ST1                                                     \
	CLEAR_T;                                                    \
	m_io->write_byte(0x0002,tmp)

/* 6280 ********************************************************
 *  ST2 Store at hardware address 3
 ***************************************************************/
#define ST2                                                     \
	CLEAR_T;                                                    \
	m_io->write_byte(0x0003,tmp)

/* 6280 ********************************************************
 *  STA Store accumulator
 ***************************************************************/
#define STA                                                     \
	CLEAR_T;                                                    \
	tmp = A

/* 6280 ********************************************************
 *  STX Store index X
 ***************************************************************/
#define STX                                                     \
	CLEAR_T;                                                    \
	tmp = X

/* 6280 ********************************************************
 *  STY Store index Y
 ***************************************************************/
#define STY                                                     \
	CLEAR_T;                                                    \
	tmp = Y

/* 6280 ********************************************************
 * STZ  Store zero
 ***************************************************************/
#define STZ                                                     \
	CLEAR_T;                                                    \
	tmp = 0

/* H6280 *******************************************************
 *  SXY Swap index X and index Y
 ***************************************************************/
#define SXY                                                    \
	CLEAR_T;                                                    \
	tmp = X;                                                   \
	X = Y;                                                     \
	Y = tmp

/* H6280 *******************************************************
 *  TAI Transfer Alternate Increment
 ***************************************************************/
#define TAI                                                     \
	CLEAR_T;                                                    \
	from = program_read16(PCW);                                 \
	to = program_read16(PCW + 2);                               \
	length = program_read16(PCW + 4);                           \
	PCW += 6;                                                   \
	alternate = 0;                                              \
	if (!length) length = 0x10000;                              \
	H6280_CYCLES( ((6 * length) + 17) );                        \
	while ((length--) != 0)                                     \
	{                                                           \
		program_write8(to, program_read8(from + alternate));    \
		to++;                                                   \
		alternate ^= 1;                                         \
	}

/* H6280 *******************************************************
 *  TAM Transfer accumulator to memory mapper register(s)
 ***************************************************************/
#define TAM                                                     \
	CLEAR_T;                                                    \
	if (tmp&0x01) m_mmr[0] = A;                                 \
	if (tmp&0x02) m_mmr[1] = A;                                 \
	if (tmp&0x04) m_mmr[2] = A;                                 \
	if (tmp&0x08) m_mmr[3] = A;                                 \
	if (tmp&0x10) m_mmr[4] = A;                                 \
	if (tmp&0x20) m_mmr[5] = A;                                 \
	if (tmp&0x40) m_mmr[6] = A;                                 \
	if (tmp&0x80) m_mmr[7] = A

/* 6280 ********************************************************
 *  TAX Transfer accumulator to index X
 ***************************************************************/
#define TAX                                                     \
	CLEAR_T;                                                    \
	X = A;                                                      \
	SET_NZ(X)

/* 6280 ********************************************************
 *  TAY Transfer accumulator to index Y
 ***************************************************************/
#define TAY                                                     \
	CLEAR_T;                                                    \
	Y = A;                                                      \
	SET_NZ(Y)

/* 6280 ********************************************************
 *  TDD Transfer Decrement Decrement
 ***************************************************************/
#define TDD                                                     \
	CLEAR_T;                                                    \
	from = program_read16(PCW);                                 \
	to = program_read16(PCW + 2);                               \
	length = program_read16(PCW + 4);                           \
	PCW+=6;                                                     \
	if (!length) length = 0x10000;                              \
	H6280_CYCLES( ((6 * length) + 17) );                        \
	while ((length--) != 0) {                                   \
		program_write8(to, program_read8(from));                \
		to--;                                                   \
		from--;                                                 \
	}

/* 6280 ********************************************************
 *  TIA Transfer Increment Alternate
 ***************************************************************/
#define TIA                                                     \
	CLEAR_T;                                                    \
	from = program_read16(PCW);                                 \
	to  = program_read16(PCW + 2);                              \
	length = program_read16(PCW + 4);                           \
	PCW+=6;                                                     \
	alternate=0;                                                \
	if (!length) length = 0x10000;                              \
	H6280_CYCLES( ((6 * length) + 17) );                        \
	while ((length--) != 0) {                                   \
		program_write8(to + alternate, program_read8(from));    \
		from++;                                                 \
		alternate ^= 1;                                         \
	}

/* 6280 ********************************************************
 *  TII Transfer Increment Increment
 ***************************************************************/
#define TII                                                     \
	CLEAR_T;                                                    \
	from = program_read16(PCW);                                 \
	to = program_read16(PCW + 2);                               \
	length = program_read16(PCW + 4);                           \
	PCW += 6;                                                   \
	if (!length) length = 0x10000;                              \
	H6280_CYCLES( ((6 * length) + 17) );                        \
	while ((length--) != 0) {                                   \
		program_write8(to, program_read8(from));                \
		to++;                                                   \
		from++;                                                 \
	}

/* 6280 ********************************************************
 *  TIN Transfer block, source increments every loop
 ***************************************************************/
#define TIN                                                     \
	CLEAR_T;                                                    \
	from = program_read16(PCW);                                 \
	to = program_read16(PCW + 2);                               \
	length = program_read16(PCW + 4);                           \
	PCW+=6;                                                     \
	if (!length) length = 0x10000;                              \
	H6280_CYCLES( ((6 * length) + 17) );                        \
	while ((length--) != 0) {                                   \
		program_write8(to, program_read8(from));                \
		from++;                                                 \
	}

/* 6280 ********************************************************
 *  TMA Transfer memory mapper register(s) to accumulator
 *  the highest bit set in tmp is the one that counts
 ***************************************************************/
#define TMA                                                     \
	CLEAR_T;                                                    \
	if (tmp&0x01) A = m_mmr[0];                                 \
	if (tmp&0x02) A = m_mmr[1];                                 \
	if (tmp&0x04) A = m_mmr[2];                                 \
	if (tmp&0x08) A = m_mmr[3];                                 \
	if (tmp&0x10) A = m_mmr[4];                                 \
	if (tmp&0x20) A = m_mmr[5];                                 \
	if (tmp&0x40) A = m_mmr[6];                                 \
	if (tmp&0x80) A = m_mmr[7]

/* 6280 ********************************************************
 * TRB  Test and reset bits
 ***************************************************************/
#define TRB                                                     \
	CLEAR_T;                                                    \
	P = (P & ~(_fN|_fV|_fT|_fZ))                                \
		| ((tmp&0x80) ? _fN:0)                                  \
		| ((tmp&0x40) ? _fV:0)                                  \
		| ((tmp&~A)  ? 0:_fZ);                                  \
	tmp &= ~A

/* 6280 ********************************************************
 * TSB  Test and set bits
 ***************************************************************/
#define TSB                                                     \
	CLEAR_T;                                                    \
	P = (P & ~(_fN|_fV|_fT|_fZ))                                \
		| ((tmp&0x80) ? _fN:0)                                  \
		| ((tmp&0x40) ? _fV:0)                                  \
		| ((tmp|A)  ? 0:_fZ);                                   \
	tmp |= A

/* 6280 ********************************************************
 *  TSX Transfer stack LSB to index X
 ***************************************************************/
#define TSX                                                     \
	CLEAR_T;                                                    \
	X = S;                                                      \
	SET_NZ(X)

/* 6280 ********************************************************
 *  TST
 ***************************************************************/
#define TST                                                     \
	P = (P & ~(_fN|_fV|_fT|_fZ))                                \
		| ((tmp&0x80) ? _fN:0)                                  \
		| ((tmp&0x40) ? _fV:0)                                  \
		| ((tmp&tmp2)  ? 0:_fZ)

/* 6280 ********************************************************
 *  TXA Transfer index X to accumulator
 ***************************************************************/
#define TXA                                                     \
	CLEAR_T;                                                    \
	A = X;                                                      \
	SET_NZ(A)

/* 6280 ********************************************************
 *  TXS Transfer index X to stack LSB
 *  no flags changed (sic!)
 ***************************************************************/
#define TXS                                                     \
	CLEAR_T;                                                    \
	S = X

/* 6280 ********************************************************
 *  TYA Transfer index Y to accumulator
 ***************************************************************/
#define TYA                                                     \
	CLEAR_T;                                                    \
	A = Y;                                                      \
	SET_NZ(A)

/* 6280 ********************************************************
 * CSH Set CPU in high speed mode
 ***************************************************************/
#define CSH                                                     \
	m_clocks_per_cycle = 1;

/* 6280 ********************************************************
 * CSL Set CPU in low speed mode
 ***************************************************************/
#define CSL                                                     \
	m_clocks_per_cycle = 4;
