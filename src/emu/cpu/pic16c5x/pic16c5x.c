 /**************************************************************************\
 *                      Microchip PIC16C5x Emulator                         *
 *                                                                          *
 *                    Copyright (C) 2003+ Tony La Porta                     *
 *                 Originally written for the MAME project.                 *
 *                                                                          *
 *                                                                          *
 *      Addressing architecture is based on the Harvard addressing scheme.  *
 *                                                                          *
 *                                                                          *
 *  **** Change Log ****                                                    *
 *  TLP (06-Apr-2003)                                                       *
 *   - First Public release.                                                *
 *  BO  (07-Apr-2003) Ver 1.01                                              *
 *   - Renamed 'sleep' function to 'sleepic' to avoid C conflicts.          *
 *  TLP (09-Apr-2003) Ver 1.10                                              *
 *   - Fixed modification of file register $03 (Status).                    *
 *   - Corrected support for 7FFh (12-bit) size ROMs.                       *
 *   - The 'call' and 'goto' instructions weren't correctly handling the    *
 *     STATUS page info correctly.                                          *
 *   - The FSR register was incorrectly oring the data with 0xe0 when read. *
 *   - Prescaler masking information was set to 3 instead of 7.             *
 *   - Prescaler assign bit was set to 4 instead of 8.                      *
 *   - Timer source and edge select flags/masks were wrong.                 *
 *   - Corrected the memory bank selection in GET/SET_REGFILE and also the  *
 *     indirect register addressing.                                        *
 *  BMP (18-May-2003) Ver 1.11                                              *
 *   - pic16c5x_get_reg functions were missing 'returns'.                   *
 *  TLP (27-May-2003) Ver 1.12                                              *
 *   - Fixed the WatchDog timer count.                                      *
 *   - The Prescaler rate was incorrectly being zeroed, instead of the      *
 *     actual Prescaler counter in the CLRWDT and SLEEP instructions.       *
 *   - Added masking to the FSR register. Upper unused bits are always 1.   *
 *                                                                          *
 *                                                                          *
 *  **** Notes: ****                                                        *
 *  PIC WatchDog Timer has a seperate internal clock. For the moment, we're *
 *     basing the count on a 4MHz input clock, since 4MHz is the typical    *
 *     input frequency (but by no means always).                            *
 *  A single scaler is available for the Counter/Timer or WatchDog Timer.   *
 *     When connected to the Counter/Timer, it functions as a Prescaler,    *
 *     hence prescale overflows, tick the Counter/Timer.                    *
 *     When connected to the WatchDog Timer, it functions as a Postscaler   *
 *     hence WatchDog Timer overflows, tick the Postscaler. This scenario   *
 *     means that the WatchDog timeout occurs when the Postscaler has       *
 *     reached the scaler rate value, not when the WatchDog reaches zero.   *
 *  CLRWDT should prevent the WatchDog Timer from timing out and generating *
 *     a device reset, but how is not known. The manual also mentions that  *
 *     the WatchDog Timer can only be disabled during ROM programming, and  *
 *     no other means seem to exist???                                      *
 *                                                                          *
 \**************************************************************************/



#include "debugger.h"
#include "pic16c5x.h"


#define CLK 1	/* 1 cycle equals 4 Q-clock ticks */


#ifndef INLINE
#define INLINE static inline
#endif


#define M_RDRAM(A)		(((A) < 8) ? R.internalram[A] : PIC16C5x_RAM_RDMEM(A))
#define M_WRTRAM(A,V)	do { if ((A) < 8) R.internalram[A] = (V); else PIC16C5x_RAM_WRMEM(A,V); } while (0)
#define M_RDOP(A)		PIC16C5x_RDOP(A)
#define M_RDOP_ARG(A)	PIC16C5x_RDOP_ARG(A)
#define P_IN(A)			PIC16C5x_In(A)
#define P_OUT(A,V)		PIC16C5x_Out(A,V)
#define S_T0_IN			PIC16C5x_T0_In
#define ADDR_MASK		0x7ff




typedef struct
{
	/******************** CPU Internal Registers *******************/
	UINT16	PC;
	UINT16	PREVPC;		/* previous program counter */
	UINT8	W;
	UINT8	OPTION;
	UINT16	CONFIG;
	UINT8	ALU;
	UINT16	WDT;
	UINT8	TRISA;
	UINT8	TRISB;
	UINT8	TRISC;
	UINT16	STACK[2];
	UINT16	prescaler;	/* Note: this is really an 8-bit register */
	PAIR	opcode;
	UINT8	internalram[8];
} pic16C5x_Regs;

static pic16C5x_Regs R;
static UINT16 temp_config;
static UINT8  old_T0;
static INT8   old_data;
static UINT8  picRAMmask;
static int    inst_cycles;
static int    delay_timer;
static int    picmodel;
static int    pic16C5x_reset_vector;
static int    pic16C5x_icount;
typedef void (*opcode_fn) (void);
static unsigned cycles_000_other[16];


#define TMR0	internalram[1]
#define PCL		internalram[2]
#define STATUS	internalram[3]
#define FSR		internalram[4]
#define PORTA	internalram[5]
#define PORTB	internalram[6]
#define PORTC	internalram[7]
#define INDF	M_RDRAM(R.FSR)

#define ADDR	(R.opcode.b.l & 0x1f)

#define POSITIVE_EDGE_T0  (( (int)(T0_in-old_T0) > 0) ? 1 : 0)
#define NEGATIVE_EDGE_T0  (( (int)(old_T0-T0_in) > 0) ? 1 : 0)


/********  The following is the Status Flag register definition.  *********/
			/* | 7 | 6 | 5 |  4 |  3 | 2 | 1  | 0 | */
			/* |    PA     | TO | PD | Z | DC | C | */
#define PA_REG		0xe0	/* PA   Program Page Preselect - bit 8 is unused here */
#define TO_FLAG		0x10	/* TO   Time Out flag (WatchDog) */
#define PD_FLAG		0x08	/* PD   Power Down flag */
#define Z_FLAG		0x04	/* Z    Zero Flag */
#define DC_FLAG		0x02	/* DC   Digit Carry/Borrow flag (Nibble) */
#define C_FLAG		0x01	/* C    Carry/Borrow Flag (Byte) */

#define PA		(R.STATUS & PA_REG)
#define TO		(R.STATUS & TO_FLAG)
#define PD		(R.STATUS & PD_FLAG)
#define ZERO	(R.STATUS & Z_FLAG)
#define DC		(R.STATUS & DC_FLAG)
#define CARRY	(R.STATUS & C_FLAG)


/********  The following is the Option Flag register definition.  *********/
			/* | 7 | 6 |   5  |   4  |  3  | 2 | 1 | 0 | */
			/* | 0 | 0 | TOCS | TOSE | PSA |    PS     | */
#define T0CS_FLAG	0x20	/* TOCS     Timer 0 clock source select */
#define T0SE_FLAG	0x10	/* TOSE     Timer 0 clock source edge select */
#define PSA_FLAG	0x08	/* PSA      Prescaler Assignment bit */
#define PS_REG		0x07	/* PS       Prescaler Rate select */

#define T0CS	(R.OPTION & T0CS_FLAG)
#define T0SE	(R.OPTION & T0SE_FLAG)
#define PSA		(R.OPTION & PSA_FLAG)
#define PS		(R.OPTION & PS_REG)


/********  The following is the Config Flag register definition.  *********/
	/* | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 |   2  | 1 | 0 | */
	/* |              CP                     | WDTE |  FOSC | */
							/* CP       Code Protect (ROM read protect) */
#define WDTE_FLAG	0x04	/* WDTE     WatchDog Timer enable */
#define FOSC_FLAG	0x03	/* FOSC     Oscillator source select */

#define WDTE	(R.CONFIG & WDTE_FLAG)
#define FOSC	(R.CONFIG & FOSC_FLAG)


/************************************************************************
 *  Shortcuts
 ************************************************************************/

/* Easy bit position selectors */
#define POS	 ((R.opcode.b.l >> 5) & 7)
static unsigned int bit_clr[8] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
static unsigned int bit_set[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };


INLINE void CLR(UINT16 flag) { R.STATUS &= ~flag; }
INLINE void SET(UINT16 flag) { R.STATUS |=  flag; }



INLINE void CALCULATE_Z_FLAG(void)
{
	if (R.ALU == 0) SET(Z_FLAG);
	else CLR(Z_FLAG);
}

INLINE void CALCULATE_ADD_CARRY(void)
{
	if ((UINT8)(old_data) > (UINT8)(R.ALU)) {
		SET(C_FLAG);
	}
	else {
		CLR(C_FLAG);
	}
}

INLINE void CALCULATE_ADD_DIGITCARRY(void)
{
	if (((UINT8)(old_data) & 0x0f) > ((UINT8)(R.ALU) & 0x0f)) {
		SET(DC_FLAG);
	}
	else {
		CLR(DC_FLAG);
	}
}

INLINE void CALCULATE_SUB_CARRY(void)
{
	if ((UINT8)(old_data) < (UINT8)(R.ALU)) {
		CLR(C_FLAG);
	}
	else {
		SET(C_FLAG);
	}
}

INLINE void CALCULATE_SUB_DIGITCARRY(void)
{
	if (((UINT8)(old_data) & 0x0f) < ((UINT8)(R.ALU) & 0x0f)) {
		CLR(DC_FLAG);
	}
	else {
		SET(DC_FLAG);
	}
}



INLINE UINT16 POP_STACK(void)
{
	UINT16 data = R.STACK[1];
	R.STACK[1] = R.STACK[0];
	return (data & ADDR_MASK);
}
INLINE void PUSH_STACK(UINT16 data)
{
	R.STACK[0] = R.STACK[1];
	R.STACK[1] = (data & ADDR_MASK);
}


INLINE UINT8 GET_REGFILE(offs_t addr)	/* Read from internal memory */
{
	UINT8 data;

	if ((picmodel == 0x16C57) || (picmodel == 0x16C58))
	{
		addr |= (R.FSR & 0x60);		/* FSR used for banking */
	}
	if ((addr & 0x10) == 0) addr &= 0x0f;

	switch(addr)
	{
		case 00:	addr = (R.FSR & picRAMmask);
					if (addr == 0) { data = 0; break; }
					if ((addr & 0x10) == 0) addr &= 0x0f;
					data = M_RDRAM(addr);			/* Indirect address */
					break;
		case 04:	data = (R.FSR | (~picRAMmask)); break;
		case 05:	data = P_IN(0);
					data &= R.TRISA;
					data |= (~R.TRISA & R.PORTA);
					data &= 0xf;		/* 4-bit port (only lower 4 bits used) */
					break;
		case 06:	data = P_IN(1);
					data &= R.TRISB;
					data |= (~R.TRISB & R.PORTB);
					break;
		case 07:	if ((picmodel == 0x16C55) || (picmodel == 0x16C57)) {
						data = P_IN(2);
						data &= R.TRISC;
						data |= (~R.TRISC & R.PORTC);
					}
					else {		/* PIC16C54, PIC16C56, PIC16C58 */
						data = M_RDRAM(addr);
					}
					break;
		default:	data = M_RDRAM(addr); break;
	}

	return data;
}

INLINE void STORE_REGFILE(offs_t addr, UINT8 data)	/* Write to internal memory */
{
	if ((picmodel == 0x16C57) || (picmodel == 0x16C58))
	{
		addr |= (R.FSR & 0x60);
	}
	if ((addr & 0x10) == 0) addr &= 0x0f;

	switch(addr)
	{
		case 00:	addr = (R.FSR & picRAMmask);
					if (addr == 0) break;
					if ((addr & 0x10) == 0) addr &= 0x0f;
					M_WRTRAM(addr, data); break;	/* Indirect address */
		case 01:	delay_timer = 2;		/* Timer starts after next two instructions */
					if (PSA == 0) R.prescaler = 0;	/* Must clear the Prescaler */
					R.TMR0 = data; break;
		case 02:	R.PCL = data;
					R.PC = ((R.STATUS & PA_REG) << 4) | data; break;
		case 03:	R.STATUS &= ~PA_REG; R.STATUS |= (data & PA_REG); break;
		case 04:	R.FSR = (data | (~picRAMmask)); break;
		case 05:	data &= 0xf;		/* 4-bit port (only lower 4 bits used) */
					P_OUT(0,data & (~R.TRISA)); R.PORTA = data; break;
		case 06:	P_OUT(1,data & (~R.TRISB)); R.PORTB = data; break;
		case 07:	if ((picmodel == 0x16C55) || (picmodel == 0x16C57)) {
						P_OUT(2,data & (~R.TRISC));
						R.PORTC = data;
					}
					else {		/* PIC16C54, PIC16C56, PIC16C58 */
						M_WRTRAM(addr, data);
					}
					break;
		default:	M_WRTRAM(addr, data); break;
	}
}


INLINE void STORE_RESULT(offs_t addr, UINT8 data)
{
	if (R.opcode.b.l & 0x20)
	{
		STORE_REGFILE(addr, data);
	}
	else
	{
		R.W = data;
	}
}


/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */


static void illegal(void)
{
	logerror("PIC16C5x:  PC=%03x,  Illegal opcode = %04x\n", (R.PC-1), R.opcode.w.l);
}


static void addwf(void)
{
	old_data = GET_REGFILE(ADDR);
	R.ALU = old_data + R.W;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
	CALCULATE_ADD_CARRY();
	CALCULATE_ADD_DIGITCARRY();
}

static void andwf(void)
{
	R.ALU = GET_REGFILE(ADDR) & R.W;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}

static void andlw(void)
{
	R.ALU = R.opcode.b.l & R.W;
	R.W = R.ALU;
	CALCULATE_Z_FLAG();
}

static void bcf(void)
{
	R.ALU = GET_REGFILE(ADDR);
	R.ALU &= bit_clr[POS];
	STORE_REGFILE(ADDR, R.ALU);
}

static void bsf(void)
{
	R.ALU = GET_REGFILE(ADDR);
	R.ALU |= bit_set[POS];
	STORE_REGFILE(ADDR, R.ALU);
}

static void btfss(void)
{
	if ((GET_REGFILE(ADDR) & bit_set[POS]) == bit_set[POS])
	{
		R.PC++ ;
		R.PCL = R.PC & 0xff;
		inst_cycles += cycles_000_other[0];		/* Add NOP cycles */
	}
}

static void btfsc(void)
{
	if ((GET_REGFILE(ADDR) & bit_set[POS]) == 0)
	{
		R.PC++ ;
		R.PCL = R.PC & 0xff;
		inst_cycles += cycles_000_other[0];		/* Add NOP cycles */
	}
}

static void call(void)
{
	PUSH_STACK(R.PC);
	R.PC = ((R.STATUS & PA_REG) << 4) | R.opcode.b.l;
	R.PC &= 0x6ff;
	R.PCL = R.PC & 0xff;
}

static void clrw(void)
{
	R.W = 0;
	SET(Z_FLAG);
}

static void clrf(void)
{
	STORE_REGFILE(ADDR, 0);
	SET(Z_FLAG);
}

static void clrwdt(void)
{
	R.WDT = 0;
	if (PSA) R.prescaler = 0;
	SET(TO_FLAG);
	SET(PD_FLAG);
}

static void comf(void)
{
	R.ALU = ~(GET_REGFILE(ADDR));
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}

static void decf(void)
{
	R.ALU = GET_REGFILE(ADDR) - 1;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}

static void decfsz(void)
{
	R.ALU = GET_REGFILE(ADDR) - 1;
	STORE_RESULT(ADDR, R.ALU);
	if (R.ALU == 0)
	{
		R.PC++ ;
		R.PCL = R.PC & 0xff;
		inst_cycles += cycles_000_other[0];		/* Add NOP cycles */
	}
}

static void goto_op(void)
{
	R.PC = ((R.STATUS & PA_REG) << 4) | (R.opcode.w.l & 0x1ff);
	R.PC &= ADDR_MASK;
	R.PCL = R.PC & 0xff;
}

static void incf(void)
{
	R.ALU = GET_REGFILE(ADDR) + 1;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}

static void incfsz(void)
{
	R.ALU = GET_REGFILE(ADDR) + 1;
	STORE_RESULT(ADDR, R.ALU);
	if (R.ALU == 0)
	{
		R.PC++ ;
		R.PCL = R.PC & 0xff;
		inst_cycles += cycles_000_other[0];		/* Add NOP cycles */
	}
}

static void iorlw(void)
{
	R.ALU = R.opcode.b.l | R.W;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}

static void iorwf(void)
{
	R.ALU = GET_REGFILE(ADDR) | R.W;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}

static void movf(void)
{
	R.ALU = GET_REGFILE(ADDR);
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}

static void movlw(void)
{
	R.W = R.opcode.b.l;
}

static void movwf(void)
{
	STORE_REGFILE(ADDR, R.W);
}

static void nop(void)
{
	/* Do nothing */
}

static void option(void)
{
	R.OPTION = R.W & 0x3f;
}

static void retlw(void)
{
	R.W = R.opcode.b.l;
	R.PC = POP_STACK();
	R.PCL = R.PC & 0xff;
}

static void rlf(void)
{
	R.ALU = GET_REGFILE(ADDR);
	R.ALU <<= 1;
	if (R.STATUS & C_FLAG) R.ALU |= 1;
	if (GET_REGFILE(ADDR) & 0x80) SET(C_FLAG);
	else CLR(C_FLAG);
	STORE_RESULT(ADDR, R.ALU);
}

static void rrf(void)
{
	R.ALU = GET_REGFILE(ADDR);
	R.ALU >>= 1;
	if (R.STATUS & C_FLAG) R.ALU |= 0x80;
	if (GET_REGFILE(ADDR) & 1) SET(C_FLAG);
	else CLR(C_FLAG);
	STORE_RESULT(ADDR, R.ALU);
}

static void sleepic(void)
{
	if (WDTE) R.WDT = 0;
	if (PSA) R.prescaler = 0;
	SET(TO_FLAG);
	CLR(PD_FLAG);
}

static void subwf(void)
{
	old_data = GET_REGFILE(ADDR);
	R.ALU = old_data - R.W;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
	CALCULATE_SUB_CARRY();
	CALCULATE_SUB_DIGITCARRY();
}

static void swapf(void)
{
	R.ALU  = ((GET_REGFILE(ADDR) << 4) & 0xf0);
	R.ALU |= ((GET_REGFILE(ADDR) >> 4) & 0x0f);
	STORE_RESULT(ADDR, R.ALU);
}

static void tris(void)
{
	switch(R.opcode.b.l & 0x7)
	{
		case 05:	if (R.TRISA == R.W) break;
					else R.TRISA = R.W; P_OUT(0,R.PORTA & (~R.TRISA) & 0xf); break;
		case 06:	if (R.TRISB == R.W) break;
					else R.TRISB = R.W; P_OUT(1,R.PORTB & (~R.TRISB)); break;
		case 07:	if (R.TRISC == R.W) break;
					else R.TRISC = R.W; P_OUT(2,R.PORTC & (~R.TRISC)); break;
		default:	illegal(); break;
	}
}

static void xorlw(void)
{
	R.ALU = R.W ^ R.opcode.b.l;
	R.W = R.ALU;
	CALCULATE_Z_FLAG();
}

static void xorwf(void)
{
	R.ALU = GET_REGFILE(ADDR) ^ R.W;
	STORE_RESULT(ADDR, R.ALU);
	CALCULATE_Z_FLAG();
}



/***********************************************************************
 *  Cycle Timings
 ***********************************************************************/

static unsigned cycles_main[256]=
{
/*00*/	1*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*10*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*20*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*30*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*40*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*50*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*60*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*70*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*80*/	2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*90*/	2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*A0*/	2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*B0*/	2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*C0*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*D0*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*E0*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*F0*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK
};

static opcode_fn opcode_main[256]=
{
/*00*/  nop,	illegal,movwf,	movwf,	clrw,	illegal,clrf,	clrf,
/*08*/  subwf,	subwf,	subwf,	subwf,	decf,	decf,	decf,	decf,
/*10*/  iorwf,	iorwf,	iorwf,	iorwf,	andwf,	andwf,	andwf,	andwf,
/*18*/  xorwf,	xorwf,	xorwf,	xorwf,	addwf,	addwf,	addwf,	addwf,
/*20*/  movf,	movf,	movf,	movf,	comf,	comf,	comf,	comf,
/*28*/  incf,	incf,	incf,	incf,	decfsz,	decfsz,	decfsz,	decfsz,
/*30*/  rrf,	rrf,	rrf,	rrf,	rlf,	rlf,	rlf,	rlf,
/*38*/  swapf,	swapf,	swapf,	swapf,	incfsz,	incfsz,	incfsz,	incfsz,
/*40*/  bcf,	bcf,	bcf,	bcf,	bcf,	bcf,	bcf,	bcf,
/*48*/  bcf,	bcf,	bcf,	bcf,	bcf,	bcf,	bcf,	bcf,
/*50*/  bsf,	bsf,	bsf,	bsf,	bsf,	bsf,	bsf,	bsf,
/*58*/  bsf,	bsf,	bsf,	bsf,	bsf,	bsf,	bsf,	bsf,
/*60*/  btfsc,	btfsc,	btfsc,	btfsc,	btfsc,	btfsc,	btfsc,	btfsc,
/*68*/  btfsc,	btfsc,	btfsc,	btfsc,	btfsc,	btfsc,	btfsc,	btfsc,
/*70*/  btfss,	btfss,	btfss,	btfss,	btfss,	btfss,	btfss,	btfss,
/*78*/  btfss,	btfss,	btfss,	btfss,	btfss,	btfss,	btfss,	btfss,
/*80*/  retlw,	retlw,	retlw,	retlw,	retlw,	retlw,	retlw,	retlw,
/*88*/  retlw,	retlw,	retlw,	retlw,	retlw,	retlw,	retlw,	retlw,
/*90*/  call,	call,	call,	call,	call,	call,	call,	call,
/*98*/  call,	call,	call,	call,	call,	call,	call,	call,
/*A0*/  goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,
/*A8*/  goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,
/*B0*/  goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,
/*B8*/  goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,goto_op,
/*C0*/  movlw,	movlw,	movlw,	movlw,	movlw,	movlw,	movlw,	movlw,
/*C8*/  movlw,	movlw,	movlw,	movlw,	movlw,	movlw,	movlw,	movlw,
/*D0*/  iorlw,	iorlw,	iorlw,	iorlw,	iorlw,	iorlw,	iorlw,	iorlw,
/*D8*/  iorlw,	iorlw,	iorlw,	iorlw,	iorlw,	iorlw,	iorlw,	iorlw,
/*E0*/  andlw,	andlw,	andlw,	andlw,	andlw,	andlw,	andlw,	andlw,
/*E8*/  andlw,	andlw,	andlw,	andlw,	andlw,	andlw,	andlw,	andlw,
/*F0*/  xorlw,	xorlw,	xorlw,	xorlw,	xorlw,	xorlw,	xorlw,	xorlw,
/*F8*/  xorlw,	xorlw,	xorlw,	xorlw,	xorlw,	xorlw,	xorlw,	xorlw
};


static unsigned cycles_000_other[16]=
{
/*00*/	1*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK
};

static opcode_fn opcode_000_other[16]=
{
/*00*/  nop,	illegal,option,	sleepic,clrwdt,	tris,	tris,	tris,
/*08*/  illegal,illegal,illegal,illegal,illegal,illegal,illegal,illegal
};



/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

static void pic16C5x_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	state_save_register_item("pic16C5x", index, old_data);
	state_save_register_item("pic16C5x", index, R.W);
	state_save_register_item("pic16C5x", index, R.ALU);
	state_save_register_item("pic16C5x", index, R.OPTION);
	state_save_register_item("pic16C5x", index, R.TMR0);
	state_save_register_item("pic16C5x", index, R.PCL);
	state_save_register_item("pic16C5x", index, R.STATUS);
	state_save_register_item("pic16C5x", index, R.FSR);
	state_save_register_item("pic16C5x", index, R.PORTA);
	state_save_register_item("pic16C5x", index, R.PORTB);
	state_save_register_item("pic16C5x", index, R.PORTC);
	state_save_register_item("pic16C5x", index, R.TRISA);
	state_save_register_item("pic16C5x", index, R.TRISB);
	state_save_register_item("pic16C5x", index, R.TRISC);
	state_save_register_item("pic16C5x", index, old_T0);
	state_save_register_item("pic16C5x", index, picRAMmask);
	state_save_register_item("pic16C5x", index, R.WDT);
	state_save_register_item("pic16C5x", index, R.prescaler);
	state_save_register_item("pic16C5x", index, R.STACK[0]);
	state_save_register_item("pic16C5x", index, R.STACK[1]);
	state_save_register_item("pic16C5x", index, R.PC);
	state_save_register_item("pic16C5x", index, R.PREVPC);
	state_save_register_item("pic16C5x", index, R.CONFIG);
	state_save_register_item("pic16C5x", index, R.opcode.d);
	state_save_register_item("pic16C5x", index, delay_timer);
	state_save_register_item("pic16C5x", index, picmodel);
	state_save_register_item("pic16C5x", index, pic16C5x_reset_vector);
}


/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

static void pic16C5x_reset_regs(void)
{
	R.PC     = pic16C5x_reset_vector;
	R.CONFIG = temp_config;
	R.TRISA  = 0xff;
	R.TRISB  = 0xff;
	R.TRISC  = 0xff;
	R.OPTION = 0x3f;
	R.PCL    = 0xff;
	R.FSR   |= (~picRAMmask);
	R.PORTA &= 0x0f;
	R.prescaler = 0;
	delay_timer = 0;
	old_T0      = 0;
	inst_cycles = 0;
}

static void pic16C5x_soft_reset(void)
{
	R.STATUS &= 0x1f;
	pic16C5x_reset_regs();
}

void pic16c5x_config(int data)
{
	logerror("Writing %04x to the PIC16C5x config register\n",data);
	temp_config = (data & 0xfff);
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/

static void pic16C5x_exit (void)
{
	/* nothing to do */
}


/****************************************************************************
 *  WatchDog
 ****************************************************************************/

static void pic16C5x_update_watchdog(int counts)
{
	/* WatchDog is set up to count 18,000 (0x464f hex) ticks to provide */
	/* the timeout period of 0.018ms based on a 4MHz input clock. */
	/* Note: the 4MHz clock should be divided by the PIC16C5x_CLOCK_DIVIDER */
	/* which effectively makes the PIC run at 1MHz internally. */

	/* If the current instruction is CLRWDT or SLEEP, don't update the WDT */

	if ((R.opcode.w.l != 3) && (R.opcode.w.l != 4))
	{
		UINT16 old_WDT = R.WDT;

		R.WDT -= counts;

		if (R.WDT > 0x464f) {
			R.WDT = 0x464f - (0xffff - R.WDT);
		}

		if (((old_WDT != 0) && (old_WDT < R.WDT)) || (R.WDT == 0))
		{
			if (PSA) {
				R.prescaler++;
				if (R.prescaler >= (1 << PS)) {	/* Prescale values from 1 to 128 */
					R.prescaler = 0;
					CLR(TO_FLAG);
					pic16C5x_soft_reset();
				}
			}
			else {
				CLR(TO_FLAG);
				pic16C5x_soft_reset();
			}
		}
	}
}


/****************************************************************************
 *  Update Timer
 ****************************************************************************/

static void pic16C5x_update_timer(int counts)
{
	if (PSA == 0) {
		R.prescaler += counts;
		if (R.prescaler >= (2 << PS)) {	/* Prescale values from 2 to 256 */
			R.TMR0 += (R.prescaler / (2 << PS));
			R.prescaler %= (2 << PS);	/* Overflow prescaler */
		}
	}
	else {
		R.TMR0 += counts;
	}
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

static int pic16C5x_execute(int cycles)
{
	int T0_in;
	pic16C5x_icount = cycles;

	do
	{
		if (PD == 0)						/* Sleep Mode */
		{
			inst_cycles = (1*CLK);
			CALL_MAME_DEBUG;
			if (WDTE) {
				pic16C5x_update_watchdog(1*CLK);
			}
		}
		else
		{
			R.PREVPC = R.PC;

			CALL_MAME_DEBUG;

			R.opcode.d = M_RDOP(R.PC);
			R.PC++;
			R.PCL++;

			if ((R.opcode.w.l & 0xff0) != 0x000)	{	/* Do all opcodes except the 00? ones */
				inst_cycles = cycles_main[((R.opcode.w.l >> 4) & 0xff)];
				(*(opcode_main[((R.opcode.w.l >> 4) & 0xff)]))();
			}
			else {	/* Opcode 0x00? has many opcodes in its minor nibble */
				inst_cycles = cycles_000_other[(R.opcode.b.l & 0x1f)];
				(*(opcode_000_other[(R.opcode.b.l & 0x1f)]))();
			}

			if (T0CS) {						/* Count mode */
				T0_in = S_T0_IN;
				if (T0SE) {					/* Count rising edge */
					if (POSITIVE_EDGE_T0) {
						pic16C5x_update_timer(1);
					}
				}
				else {						/* Count falling edge */
					if (NEGATIVE_EDGE_T0) {
						pic16C5x_update_timer(1);
					}
				}
				old_T0 = T0_in;
			}
			else {							/* Timer mode */
				if (delay_timer) {
					delay_timer--;
				}
				else {
					pic16C5x_update_timer((inst_cycles/CLK));
				}
			}
			if (WDTE) {
				pic16C5x_update_watchdog((inst_cycles/CLK));
			}
		}

		pic16C5x_icount -= inst_cycles;

	} while (pic16C5x_icount>0);

	return cycles - pic16C5x_icount;
}


/****************************************************************************
 *  Get all registers in given buffer
 ****************************************************************************/

static void pic16C5x_get_context (void *dst)
{
	if( dst )
		*(pic16C5x_Regs*)dst = R;
}


/****************************************************************************
 *  Set all registers to given values
 ****************************************************************************/

static void pic16C5x_set_context (void *src)
{
	if (src)
		R = *(pic16C5x_Regs*)src;
}


/**************************************************************************
 *  Generic set_info
 **************************************************************************/

static void pic16C5x_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PIC16C5x_PC:		R.PC = info->i; R.PCL = info->i & 0xff ;break;
		/* This is actually not a stack pointer, but the stack contents */
		/* Stack is a 2 level First In Last Out stack */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PIC16C5x_STK1:		R.STACK[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STK0:		R.STACK[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_W:		    R.W      = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_ALU:		R.ALU    = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STR:		R.STATUS = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_OPT:		R.OPTION = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_TMR0:		R.TMR0   = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_WDT:		R.WDT    = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PSCL:		R.prescaler = info->i;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTA:		R.PORTA  = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTB:		R.PORTB  = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTC:		R.PORTC  = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_FSR:		R.FSR    = (info->i & picRAMmask);		break;
	}
}



/**************************************************************************
 *  Generic get_info
 **************************************************************************/

static void pic16C5x_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(R);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = PIC16C5x_CLOCK_DIVIDER;		break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1*CLK;						break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 10*CLK;						break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 9;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 5;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 5;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = R.PREVPC;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PIC16C5x_PC:		info->i = R.PC;							break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PIC16C5x_STK1:		info->i = R.STACK[1];					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STK0:		info->i = R.STACK[0];					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_W:   		info->i = R.W;							break;
		case CPUINFO_INT_REGISTER + PIC16C5x_ALU: 		info->i = R.ALU;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STR: 		info->i = R.STATUS;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_OPT: 		info->i = R.OPTION;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_TMR0:		info->i = R.TMR0;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_WDT: 		info->i = R.WDT;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PSCL:		info->i = R.prescaler;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTA:		info->i = R.PORTA;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTB:		info->i = R.PORTB;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTC:		info->i = R.PORTC;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_FSR: 		info->i = (R.FSR & picRAMmask);			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = pic16C5x_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = pic16C5x_get_context; break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = pic16C5x_set_context; break;
		case CPUINFO_PTR_INIT:							info->init = pic16C5x_init;				break;
		case CPUINFO_PTR_RESET:							/* set per-CPU */						break;
		case CPUINFO_PTR_EXIT:							info->exit = pic16C5x_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = pic16C5x_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = pic16C5x_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &pic16C5x_icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C5x");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Microchip");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.12");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C)2003+ by Tony La Porta"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%01x%c%c%c%c%c %c%c%c%03x",
				(R.STATUS & 0xe0) >> 5,
				R.STATUS & 0x10 ? '.':'O',		/* WDT Overflow */
				R.STATUS & 0x08 ? 'P':'D',		/* Power/Down */
				R.STATUS & 0x04 ? 'Z':'.',		/* Zero */
				R.STATUS & 0x02 ? 'c':'b',		/* Nibble Carry/Borrow */
				R.STATUS & 0x01 ? 'C':'B',		/* Carry/Borrow */

				R.OPTION & 0x20 ? 'C':'T',		/* Counter/Timer */
				R.OPTION & 0x10 ? 'N':'P',		/* Negative/Positive */
				R.OPTION & 0x08 ? 'W':'T',		/* WatchDog/Timer */
				R.OPTION & 0x08 ? (1<<(R.OPTION&7)) : (2<<(R.OPTION&7)) );
			break;

		case CPUINFO_STR_REGISTER + PIC16C5x_PC:   		sprintf(info->s, "PC:%03X",   R.PC); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_W:    		sprintf(info->s, "W:%02X",    R.W); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_ALU:  		sprintf(info->s, "ALU:%02X",  R.ALU); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_STR:  		sprintf(info->s, "STR:%02X",  R.STATUS); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TMR0: 		sprintf(info->s, "TMR:%02X",  R.TMR0); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_WDT:  		sprintf(info->s, "WDT:%04X",  R.WDT); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_OPT:  		sprintf(info->s, "OPT:%02X",  R.OPTION); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_STK0: 		sprintf(info->s, "STK0:%03X", R.STACK[0]); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_STK1: 		sprintf(info->s, "STK1:%03X", R.STACK[1]); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PRTA: 		sprintf(info->s, "PRTA:%01X", ((R.PORTA) & 0xf)); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PRTB: 		sprintf(info->s, "PRTB:%02X", R.PORTB); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PRTC: 		sprintf(info->s, "PRTC:%02X", R.PORTC); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TRSA: 		sprintf(info->s, "TRSA:%01X", ((R.TRISA) & 0xf)); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TRSB: 		sprintf(info->s, "TRSB:%02X", R.TRISB); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TRSC: 		sprintf(info->s, "TRSC:%02X", R.TRISC); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_FSR:  		sprintf(info->s, "FSR:%02X",  ((R.FSR) & picRAMmask)); break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PSCL: 		sprintf(info->s, "PSCL:%c%02X", ((R.OPTION & 0x08) ? 'W':'T'), R.prescaler); break;
	}
}



#if (HAS_PIC16C54)
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c54_rom, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c54_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x1f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C54 Reset
 ****************************************************************************/

void pic16C54_reset(void)
{
	picmodel = 0x16C54;
	picRAMmask = 0x1f;
	pic16C5x_reset_vector = 0x1ff;
	pic16C5x_reset_regs();
	R.STATUS = 0x00;
	SET(TO_FLAG);
	SET(PD_FLAG);
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

void pic16C54_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = pic16C54_reset;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map = construct_map_pic16c54_rom; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:		info->internal_map = construct_map_pic16c54_ram; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C54");			break;

		default:										pic16C5x_get_info(state, info);			break;
	}
}
#endif


#if (HAS_PIC16C55)
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c55_rom, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c55_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x1f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C55 Reset
 ****************************************************************************/

void pic16C55_reset(void)
{
	picmodel = 0x16C55;
	picRAMmask = 0x1f;
	pic16C5x_reset_vector = 0x1ff;
	pic16C5x_reset_regs();
	R.STATUS = 0x00;
	SET(TO_FLAG);
	SET(PD_FLAG);
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

void pic16C55_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = pic16C55_reset;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map = construct_map_pic16c55_rom; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:		info->internal_map = construct_map_pic16c55_ram; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C55");			break;

		default:										pic16C5x_get_info(state, info);			break;
	}
}
#endif


#if (HAS_PIC16C56)
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c56_rom, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c56_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x1f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C56 Reset
 ****************************************************************************/

void pic16C56_reset(void)
{
	picmodel = 0x16C56;
	picRAMmask = 0x1f;
	pic16C5x_reset_vector = 0x3ff;
	pic16C5x_reset_regs();
	R.STATUS = 0x00;
	SET(TO_FLAG);
	SET(PD_FLAG);
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

void pic16C56_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = pic16C56_reset;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map = construct_map_pic16c56_rom; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:		info->internal_map = construct_map_pic16c56_ram; break;

		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 10;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C56");			break;

		default:										pic16C5x_get_info(state, info);			break;
	}
}
#endif


#if (HAS_PIC16C57)
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c57_rom, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c57_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x1f) AM_RAM
	AM_RANGE(0x30, 0x3f) AM_RAM
	AM_RANGE(0x50, 0x5f) AM_RAM
	AM_RANGE(0x70, 0x7f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C57 Reset
 ****************************************************************************/

void pic16C57_reset(void)
{
	picmodel = 0x16C57;
	picRAMmask = 0x7f;
	pic16C5x_reset_vector = 0x7ff;
	pic16C5x_reset_regs();
	R.STATUS = 0x00;
	SET(TO_FLAG);
	SET(PD_FLAG);
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

void pic16C57_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = pic16C57_reset;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map = construct_map_pic16c57_rom; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:		info->internal_map = construct_map_pic16c57_ram; break;

		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 7;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C57");			break;

		default:										pic16C5x_get_info(state, info);			break;
	}
}
#endif


#if (HAS_PIC16C58)
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c58_rom, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c58_ram, ADDRESS_SPACE_DATA, 8 )
	AM_RANGE(0x00, 0x1f) AM_RAM
	AM_RANGE(0x30, 0x3f) AM_RAM
	AM_RANGE(0x50, 0x5f) AM_RAM
	AM_RANGE(0x70, 0x7f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C58 Reset
 ****************************************************************************/

void pic16C58_reset(void)
{
	picmodel = 0x16C58;
	picRAMmask = 0x7f;
	pic16C5x_reset_vector = 0x7ff;
	pic16C5x_reset_regs();
	R.STATUS = 0x00;
	SET(TO_FLAG);
	SET(PD_FLAG);
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

void pic16C58_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = pic16C58_reset;			break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM:	info->internal_map = construct_map_pic16c58_rom; break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:		info->internal_map = construct_map_pic16c58_ram; break;

		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 11;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 7;					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C58");			break;

		default:										pic16C5x_get_info(state, info);			break;
	}
}
#endif
