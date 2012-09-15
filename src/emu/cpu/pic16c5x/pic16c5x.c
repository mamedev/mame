 /**************************************************************************\
 *                      Microchip PIC16C5x Emulator                         *
 *                                                                          *
 *                    Copyright Tony La Porta                               *
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
 *  TLP (27-Aug-2009) Ver 1.13                                              *
 *   - Indirect addressing was not taking into account special purpose      *
 *     memory mapped locations.                                             *
 *   - 'iorlw' instruction was saving the result to memory instead of       *
 *     the W register.                                                      *
 *   - 'tris' instruction no longer modifies Port-C on PIC models that      *
 *     do not have Port-C implemented.                                      *
 *  TLP (07-Sep-2009) Ver 1.14                                              *
 *   - Edge sense control for the T0 count input was incorrectly reversed   *
 *                                                                          *
 *                                                                          *
 *  **** Notes: ****                                                        *
 *  PIC WatchDog Timer has a separate internal clock. For the moment, we're *
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

#include "emu.h"
#include "debugger.h"
#include "pic16c5x.h"




struct pic16c5x_state
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
	UINT8	*internalram;

	int		icount;
	int		reset_vector;
	int		picmodel;
	int		delay_timer;
	UINT16	temp_config;
	UINT8	old_T0;
	INT8	old_data;
	UINT8	picRAMmask;
	int		inst_cycles;


	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;
	address_space *io;
};

INLINE pic16c5x_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PIC16C54 ||
		   device->type() == PIC16C55 ||
		   device->type() == PIC16C56 ||
		   device->type() == PIC16C57 ||
		   device->type() == PIC16C58);
	return (pic16c5x_state *)downcast<legacy_cpu_device *>(device)->token();
}


/* opcode table entry */
struct pic16c5x_opcode
{
	UINT8	cycles;
	void	(*function)(pic16c5x_state *);
};


INLINE void update_internalram_ptr(pic16c5x_state *cpustate)
{
	cpustate->internalram = (UINT8 *)cpustate->data->get_write_ptr(0x00);
}




#define PIC16C5x_RDOP(A)         (cpustate->direct->read_decrypted_word((A)<<1))
#define PIC16C5x_RAM_RDMEM(A)    ((UINT8)cpustate->data->read_byte(A))
#define PIC16C5x_RAM_WRMEM(A,V)  (cpustate->data->write_byte(A,V))
#define PIC16C5x_In(Port)        ((UINT8)cpustate->io->read_byte((Port)))
#define PIC16C5x_Out(Port,Value) (cpustate->io->write_byte((Port),Value))
/************  Read the state of the T0 Clock input signal  ************/
#define PIC16C5x_T0_In           (cpustate->io->read_byte(PIC16C5x_T0))

#define M_RDRAM(A)		(((A) < 8) ? cpustate->internalram[A] : PIC16C5x_RAM_RDMEM(A))
#define M_WRTRAM(A,V)	do { if ((A) < 8) cpustate->internalram[A] = (V); else PIC16C5x_RAM_WRMEM(A,V); } while (0)
#define M_RDOP(A)		PIC16C5x_RDOP(A)
#define P_IN(A)			PIC16C5x_In(A)
#define P_OUT(A,V)		PIC16C5x_Out(A,V)
#define S_T0_IN			PIC16C5x_T0_In
#define ADDR_MASK		0x7ff



#define TMR0	internalram[1]
#define PCL		internalram[2]
#define STATUS	internalram[3]
#define FSR		internalram[4]
#define PORTA	internalram[5]
#define PORTB	internalram[6]
#define PORTC	internalram[7]
#define INDF	M_RDRAM(cpustate->FSR)

#define ADDR	(cpustate->opcode.b.l & 0x1f)

#define  RISING_EDGE_T0  (( (int)(T0_in - cpustate->old_T0) > 0) ? 1 : 0)
#define FALLING_EDGE_T0  (( (int)(T0_in - cpustate->old_T0) < 0) ? 1 : 0)


/********  The following is the Status Flag register definition.  *********/
			/* | 7 | 6 | 5 |  4 |  3 | 2 |  1 | 0 | */
			/* |    PA     | TO | PD | Z | DC | C | */
#define PA_REG		0xe0	/* PA   Program Page Preselect - bit 8 is unused here */
#define TO_FLAG		0x10	/* TO   Time Out flag (WatchDog) */
#define PD_FLAG		0x08	/* PD   Power Down flag */
#define Z_FLAG		0x04	/* Z    Zero Flag */
#define DC_FLAG		0x02	/* DC   Digit Carry/Borrow flag (Nibble) */
#define C_FLAG		0x01	/* C    Carry/Borrow Flag (Byte) */

#define PA		(cpustate->STATUS & PA_REG)
#define TO		(cpustate->STATUS & TO_FLAG)
#define PD		(cpustate->STATUS & PD_FLAG)
#define ZERO	(cpustate->STATUS & Z_FLAG)
#define DC		(cpustate->STATUS & DC_FLAG)
#define CARRY	(cpustate->STATUS & C_FLAG)


/********  The following is the Option Flag register definition.  *********/
			/* | 7 | 6 |   5  |   4  |  3  | 2 | 1 | 0 | */
			/* | 0 | 0 | TOCS | TOSE | PSA |    PS     | */
#define T0CS_FLAG	0x20	/* TOCS     Timer 0 clock source select */
#define T0SE_FLAG	0x10	/* TOSE     Timer 0 clock source edge select */
#define PSA_FLAG	0x08	/* PSA      Prescaler Assignment bit */
#define PS_REG		0x07	/* PS       Prescaler Rate select */

#define T0CS	(cpustate->OPTION & T0CS_FLAG)
#define T0SE	(cpustate->OPTION & T0SE_FLAG)
#define PSA		(cpustate->OPTION & PSA_FLAG)
#define PS		(cpustate->OPTION & PS_REG)


/********  The following is the Config Flag register definition.  *********/
	/* | 11 | 10 | 9 | 8 | 7 | 6 | 5 | 4 | 3 |   2  | 1 | 0 | */
	/* |              CP                     | WDTE |  FOSC | */
							/* CP       Code Protect (ROM read protect) */
#define WDTE_FLAG	0x04	/* WDTE     WatchDog Timer enable */
#define FOSC_FLAG	0x03	/* FOSC     Oscillator source select */

#define WDTE	(cpustate->CONFIG & WDTE_FLAG)
#define FOSC	(cpustate->CONFIG & FOSC_FLAG)


/************************************************************************
 *  Shortcuts
 ************************************************************************/

#define CLR(flagreg, flag) ( flagreg &= (UINT8)(~flag) )
#define SET(flagreg, flag) ( flagreg |=  flag )


/* Easy bit position selectors */
#define POS	 ((cpustate->opcode.b.l >> 5) & 7)
static const unsigned int bit_clr[8] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
static const unsigned int bit_set[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };



INLINE void CALCULATE_Z_FLAG(pic16c5x_state *cpustate)
{
	if (cpustate->ALU == 0) SET(cpustate->STATUS, Z_FLAG);
	else CLR(cpustate->STATUS, Z_FLAG);
}

INLINE void CALCULATE_ADD_CARRY(pic16c5x_state *cpustate)
{
	if ((UINT8)(cpustate->old_data) > (UINT8)(cpustate->ALU)) {
		SET(cpustate->STATUS, C_FLAG);
	}
	else {
		CLR(cpustate->STATUS, C_FLAG);
	}
}

INLINE void CALCULATE_ADD_DIGITCARRY(pic16c5x_state *cpustate)
{
	if (((UINT8)(cpustate->old_data) & 0x0f) > ((UINT8)(cpustate->ALU) & 0x0f)) {
		SET(cpustate->STATUS, DC_FLAG);
	}
	else {
		CLR(cpustate->STATUS, DC_FLAG);
	}
}

INLINE void CALCULATE_SUB_CARRY(pic16c5x_state *cpustate)
{
	if ((UINT8)(cpustate->old_data) < (UINT8)(cpustate->ALU)) {
		CLR(cpustate->STATUS, C_FLAG);
	}
	else {
		SET(cpustate->STATUS, C_FLAG);
	}
}

INLINE void CALCULATE_SUB_DIGITCARRY(pic16c5x_state *cpustate)
{
	if (((UINT8)(cpustate->old_data) & 0x0f) < ((UINT8)(cpustate->ALU) & 0x0f)) {
		CLR(cpustate->STATUS, DC_FLAG);
	}
	else {
		SET(cpustate->STATUS, DC_FLAG);
	}
}



INLINE UINT16 POP_STACK(pic16c5x_state *cpustate)
{
	UINT16 data = cpustate->STACK[1];
	cpustate->STACK[1] = cpustate->STACK[0];
	return (data & ADDR_MASK);
}
INLINE void PUSH_STACK(pic16c5x_state *cpustate, UINT16 data)
{
	cpustate->STACK[0] = cpustate->STACK[1];
	cpustate->STACK[1] = (data & ADDR_MASK);
}



INLINE UINT8 GET_REGFILE(pic16c5x_state *cpustate, offs_t addr)	/* Read from internal memory */
{
	UINT8 data;

	if (addr == 0) {						/* Indirect addressing  */
		addr = (cpustate->FSR & cpustate->picRAMmask);
	}

	if ((cpustate->picmodel == 0x16C57) || (cpustate->picmodel == 0x16C58)) {
		addr |= (cpustate->FSR & 0x60);		/* FSR bits 6-5 are used for banking in direct mode */
	}

	if ((addr & 0x10) == 0) addr &= 0x0f;

	switch(addr)
	{
		case 00:	/* Not an actual register, so return 0 */
					data = 0;
					break;
		case 04:	data = (cpustate->FSR | (UINT8)(~cpustate->picRAMmask));
					break;
		case 05:	data = P_IN(0);
					data &= cpustate->TRISA;
					data |= ((UINT8)(~cpustate->TRISA) & cpustate->PORTA);
					data &= 0x0f;		/* 4-bit port (only lower 4 bits used) */
					break;
		case 06:	data = P_IN(1);
					data &= cpustate->TRISB;
					data |= ((UINT8)(~cpustate->TRISB) & cpustate->PORTB);
					break;
		case 07:	if ((cpustate->picmodel == 0x16C55) || (cpustate->picmodel == 0x16C57)) {
						data = P_IN(2);
						data &= cpustate->TRISC;
						data |= ((UINT8)(~cpustate->TRISC) & cpustate->PORTC);
					}
					else {				/* PIC16C54, PIC16C56, PIC16C58 */
						data = M_RDRAM(addr);
					}
					break;
		default:	data = M_RDRAM(addr);
					break;
	}
	return data;
}

INLINE void STORE_REGFILE(pic16c5x_state *cpustate, offs_t addr, UINT8 data)	/* Write to internal memory */
{
	if (addr == 0) {						/* Indirect addressing  */
		addr = (cpustate->FSR & cpustate->picRAMmask);
	}

	if ((cpustate->picmodel == 0x16C57) || (cpustate->picmodel == 0x16C58)) {
		addr |= (cpustate->FSR & 0x60);		/* FSR bits 6-5 are used for banking in direct mode */
	}

	if ((addr & 0x10) == 0) addr &= 0x0f;

	switch(addr)
	{
		case 00:	/* Not an actual register, nothing to save */
					break;
		case 01:	cpustate->delay_timer = 2;		/* Timer starts after next two instructions */
					if (PSA == 0) cpustate->prescaler = 0;	/* Must clear the Prescaler */
					cpustate->TMR0 = data;
					break;
		case 02:	cpustate->PCL = data;
					cpustate->PC = ((cpustate->STATUS & PA_REG) << 4) | data;
					break;
		case 03:	cpustate->STATUS &= (UINT8)(~PA_REG); cpustate->STATUS |= (data & PA_REG);
					break;
		case 04:	cpustate->FSR = (data | (UINT8)(~cpustate->picRAMmask));
					break;
		case 05:	data &= 0x0f;		/* 4-bit port (only lower 4 bits used) */
					P_OUT(0,data & (UINT8)(~cpustate->TRISA)); cpustate->PORTA = data;
					break;
		case 06:	P_OUT(1,data & (UINT8)(~cpustate->TRISB)); cpustate->PORTB = data;
					break;
		case 07:	if ((cpustate->picmodel == 0x16C55) || (cpustate->picmodel == 0x16C57)) {
						P_OUT(2,data & (UINT8)(~cpustate->TRISC));
						cpustate->PORTC = data;
					}
					else {		/* PIC16C54, PIC16C56, PIC16C58 */
						M_WRTRAM(addr, data);
					}
					break;
		default:	M_WRTRAM(addr, data);
					break;
	}
}


INLINE void STORE_RESULT(pic16c5x_state *cpustate, offs_t addr, UINT8 data)
{
	if (cpustate->opcode.b.l & 0x20)
	{
		STORE_REGFILE(cpustate, addr, data);
	}
	else
	{
		cpustate->W = data;
	}
}


/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */


static void illegal(pic16c5x_state *cpustate)
{
	logerror("PIC16C5x:  PC=%03x,  Illegal opcode = %04x\n", (cpustate->PC-1), cpustate->opcode.w.l);
}


static void addwf(pic16c5x_state *cpustate)
{
	cpustate->old_data = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU = cpustate->old_data + cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
	CALCULATE_ADD_CARRY(cpustate);
	CALCULATE_ADD_DIGITCARRY(cpustate);
}

static void andwf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) & cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void andlw(pic16c5x_state *cpustate)
{
	cpustate->ALU = cpustate->opcode.b.l & cpustate->W;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
}

static void bcf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU &= bit_clr[POS];
	STORE_REGFILE(cpustate, ADDR, cpustate->ALU);
}

static void bsf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU |= bit_set[POS];
	STORE_REGFILE(cpustate, ADDR, cpustate->ALU);
}

static void btfss(pic16c5x_state *cpustate)
{
	if ((GET_REGFILE(cpustate, ADDR) & bit_set[POS]) == bit_set[POS])
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;		/* Add NOP cycles */
	}
}

static void btfsc(pic16c5x_state *cpustate)
{
	if ((GET_REGFILE(cpustate, ADDR) & bit_set[POS]) == 0)
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;		/* Add NOP cycles */
	}
}

static void call(pic16c5x_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->PC);
	cpustate->PC = ((cpustate->STATUS & PA_REG) << 4) | cpustate->opcode.b.l;
	cpustate->PC &= 0x6ff;
	cpustate->PCL = cpustate->PC & 0xff;
}

static void clrw(pic16c5x_state *cpustate)
{
	cpustate->W = 0;
	SET(cpustate->STATUS, Z_FLAG);
}

static void clrf(pic16c5x_state *cpustate)
{
	STORE_REGFILE(cpustate, ADDR, 0);
	SET(cpustate->STATUS, Z_FLAG);
}

static void clrwdt(pic16c5x_state *cpustate)
{
	cpustate->WDT = 0;
	if (PSA) cpustate->prescaler = 0;
	SET(cpustate->STATUS, TO_FLAG);
	SET(cpustate->STATUS, PD_FLAG);
}

static void comf(pic16c5x_state *cpustate)
{
	cpustate->ALU = (UINT8)(~(GET_REGFILE(cpustate, ADDR)));
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void decf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) - 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void decfsz(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) - 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	if (cpustate->ALU == 0)
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;		/* Add NOP cycles */
	}
}

static void goto_op(pic16c5x_state *cpustate)
{
	cpustate->PC = ((cpustate->STATUS & PA_REG) << 4) | (cpustate->opcode.w.l & 0x1ff);
	cpustate->PC &= ADDR_MASK;
	cpustate->PCL = cpustate->PC & 0xff;
}

static void incf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) + 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void incfsz(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) + 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	if (cpustate->ALU == 0)
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;		/* Add NOP cycles */
	}
}

static void iorlw(pic16c5x_state *cpustate)
{
	cpustate->ALU = cpustate->opcode.b.l | cpustate->W;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
}

static void iorwf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) | cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void movf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void movlw(pic16c5x_state *cpustate)
{
	cpustate->W = cpustate->opcode.b.l;
}

static void movwf(pic16c5x_state *cpustate)
{
	STORE_REGFILE(cpustate, ADDR, cpustate->W);
}

static void nop(pic16c5x_state *cpustate)
{
	/* Do nothing */
}

static void option(pic16c5x_state *cpustate)
{
	cpustate->OPTION = cpustate->W & (T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG);
}

static void retlw(pic16c5x_state *cpustate)
{
	cpustate->W = cpustate->opcode.b.l;
	cpustate->PC = POP_STACK(cpustate);
	cpustate->PCL = cpustate->PC & 0xff;
}

static void rlf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU <<= 1;
	if (cpustate->STATUS & C_FLAG) cpustate->ALU |= 1;
	if (GET_REGFILE(cpustate, ADDR) & 0x80) SET(cpustate->STATUS, C_FLAG);
	else CLR(cpustate->STATUS, C_FLAG);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
}

static void rrf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU >>= 1;
	if (cpustate->STATUS & C_FLAG) cpustate->ALU |= 0x80;
	if (GET_REGFILE(cpustate, ADDR) & 1) SET(cpustate->STATUS, C_FLAG);
	else CLR(cpustate->STATUS, C_FLAG);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
}

static void sleepic(pic16c5x_state *cpustate)
{
	if (WDTE) cpustate->WDT = 0;
	if (PSA) cpustate->prescaler = 0;
	SET(cpustate->STATUS, TO_FLAG);
	CLR(cpustate->STATUS, PD_FLAG);
}

static void subwf(pic16c5x_state *cpustate)
{
	cpustate->old_data = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU = cpustate->old_data - cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
	CALCULATE_SUB_CARRY(cpustate);
	CALCULATE_SUB_DIGITCARRY(cpustate);
}

static void swapf(pic16c5x_state *cpustate)
{
	cpustate->ALU  = ((GET_REGFILE(cpustate, ADDR) << 4) & 0xf0);
	cpustate->ALU |= ((GET_REGFILE(cpustate, ADDR) >> 4) & 0x0f);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
}

static void tris(pic16c5x_state *cpustate)
{
	switch(cpustate->opcode.b.l & 0x7)
	{
		case 05:	if   (cpustate->TRISA == cpustate->W) break;
					else { cpustate->TRISA = cpustate->W | 0xf0; P_OUT(0,cpustate->PORTA & (UINT8)(~cpustate->TRISA) & 0x0f); break; }
		case 06:	if   (cpustate->TRISB == cpustate->W) break;
					else { cpustate->TRISB = cpustate->W; P_OUT(1,cpustate->PORTB & (UINT8)(~cpustate->TRISB)); break; }
		case 07:	if ((cpustate->picmodel == 0x16C55) || (cpustate->picmodel == 0x16C57)) {
						if   (cpustate->TRISC == cpustate->W) break;
						else { cpustate->TRISC = cpustate->W; P_OUT(2,cpustate->PORTC & (UINT8)(~cpustate->TRISC)); break; }
					}
					else {
						illegal(cpustate); break;
					}
		default:	illegal(cpustate); break;
	}
}

static void xorlw(pic16c5x_state *cpustate)
{
	cpustate->ALU = cpustate->W ^ cpustate->opcode.b.l;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
}

static void xorwf(pic16c5x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) ^ cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}




/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

static const pic16c5x_opcode opcode_main[256]=
{
/*00*/  {1, nop		},{0, illegal	},{1, movwf		},{1, movwf		},{1, clrw		},{0, illegal	},{1, clrf		},{1, clrf		},
/*08*/  {1, subwf	},{1, subwf		},{1, subwf		},{1, subwf		},{1, decf		},{1, decf		},{1, decf		},{1, decf		},
/*10*/  {1, iorwf	},{1, iorwf		},{1, iorwf		},{1, iorwf		},{1, andwf		},{1, andwf		},{1, andwf		},{1, andwf		},
/*18*/  {1, xorwf	},{1, xorwf		},{1, xorwf		},{1, xorwf		},{1, addwf		},{1, addwf		},{1, addwf		},{1, addwf		},
/*20*/  {1, movf	},{1, movf		},{1, movf		},{1, movf		},{1, comf		},{1, comf		},{1, comf		},{1, comf		},
/*28*/  {1, incf	},{1, incf		},{1, incf		},{1, incf		},{1, decfsz	},{1, decfsz	},{1, decfsz	},{1, decfsz	},
/*30*/  {1, rrf		},{1, rrf		},{1, rrf		},{1, rrf		},{1, rlf		},{1, rlf		},{1, rlf		},{1, rlf		},
/*38*/  {1, swapf	},{1, swapf		},{1, swapf		},{1, swapf		},{1, incfsz	},{1, incfsz	},{1, incfsz	},{1, incfsz	},
/*40*/  {1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},
/*48*/  {1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},{1, bcf		},
/*50*/  {1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},
/*58*/  {1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},{1, bsf		},
/*60*/  {1, btfsc	},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},
/*68*/  {1, btfsc	},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},{1, btfsc		},
/*70*/  {1, btfss	},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},
/*78*/  {1, btfss	},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},{1, btfss		},
/*80*/  {2, retlw	},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},
/*88*/  {2, retlw	},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},{2, retlw		},
/*90*/  {2, call	},{2, call		},{2, call		},{2, call		},{2, call		},{2, call		},{2, call		},{2, call		},
/*98*/  {2, call	},{2, call		},{2, call		},{2, call		},{2, call		},{2, call		},{2, call		},{2, call		},
/*A0*/  {2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},
/*A8*/  {2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},
/*B0*/  {2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},
/*B8*/  {2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},{2, goto_op	},
/*C0*/  {1, movlw	},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},
/*C8*/  {1, movlw	},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},{1, movlw		},
/*D0*/  {1, iorlw	},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},
/*D8*/  {1, iorlw	},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},{1, iorlw		},
/*E0*/  {1, andlw	},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},
/*E8*/  {1, andlw	},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},{1, andlw		},
/*F0*/  {1, xorlw	},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		},
/*F8*/  {1, xorlw	},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		},{1, xorlw		}
};


static const pic16c5x_opcode opcode_00x[16]=
{
/*00*/  {1, nop		},{0, illegal	},{1, option	},{1, sleepic	},{1, clrwdt	},{1, tris		},{1, tris		},{1, tris		},
/*08*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	}
};



/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

static CPU_INIT( pic16c5x )
{
	pic16c5x_state *cpustate = get_safe_token(device);

	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	/* ensure the internal ram pointers are set before get_info is called */
	update_internalram_ptr(cpustate);

	device->save_item(NAME(cpustate->W));
	device->save_item(NAME(cpustate->ALU));
	device->save_item(NAME(cpustate->OPTION));
	device->save_item(NAME(cpustate->TMR0));
	device->save_item(NAME(cpustate->PCL));
	device->save_item(NAME(cpustate->STATUS));
	device->save_item(NAME(cpustate->FSR));
	device->save_item(NAME(cpustate->PORTA));
	device->save_item(NAME(cpustate->PORTB));
	device->save_item(NAME(cpustate->PORTC));
	device->save_item(NAME(cpustate->TRISA));
	device->save_item(NAME(cpustate->TRISB));
	device->save_item(NAME(cpustate->TRISC));
	device->save_item(NAME(cpustate->old_T0));
	device->save_item(NAME(cpustate->old_data));
	device->save_item(NAME(cpustate->picRAMmask));
	device->save_item(NAME(cpustate->WDT));
	device->save_item(NAME(cpustate->prescaler));
	device->save_item(NAME(cpustate->STACK[0]));
	device->save_item(NAME(cpustate->STACK[1]));
	device->save_item(NAME(cpustate->PC));
	device->save_item(NAME(cpustate->PREVPC));
	device->save_item(NAME(cpustate->CONFIG));
	device->save_item(NAME(cpustate->opcode.d));
	device->save_item(NAME(cpustate->delay_timer));
	device->save_item(NAME(cpustate->picmodel));
	device->save_item(NAME(cpustate->reset_vector));

	device->save_item(NAME(cpustate->icount));
	device->save_item(NAME(cpustate->temp_config));
	device->save_item(NAME(cpustate->inst_cycles));
}


/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

static void pic16c5x_reset_regs(pic16c5x_state *cpustate)
{
	cpustate->PC     = cpustate->reset_vector;
	cpustate->CONFIG = cpustate->temp_config;
	cpustate->TRISA  = 0xff;
	cpustate->TRISB  = 0xff;
	cpustate->TRISC  = 0xff;
	cpustate->OPTION = (T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG);
	cpustate->PCL    = 0xff;
	cpustate->FSR   |= (UINT8)(~cpustate->picRAMmask);
	cpustate->PORTA &= 0x0f;
	cpustate->prescaler = 0;
	cpustate->delay_timer = 0;
	cpustate->old_T0 = 0;
	cpustate->inst_cycles = 0;
}

static void pic16c5x_soft_reset(pic16c5x_state *cpustate)
{
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG));
	pic16c5x_reset_regs(cpustate);
}

void pic16c5x_set_config(device_t *cpu, int data)
{
	pic16c5x_state *cpustate = get_safe_token(cpu);

	logerror("Writing %04x to the PIC16C5x config register\n",data);
	cpustate->temp_config = (data & 0xfff);
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/

static CPU_EXIT( pic16c5x )
{
	/* nothing to do */
}


/****************************************************************************
 *  WatchDog
 ****************************************************************************/

static void pic16c5x_update_watchdog(pic16c5x_state *cpustate, int counts)
{
	/* WatchDog is set up to count 18,000 (0x464f hex) ticks to provide */
	/* the timeout period of 0.018ms based on a 4MHz input clock. */
	/* Note: the 4MHz clock should be divided by the PIC16C5x_CLOCK_DIVIDER */
	/* which effectively makes the PIC run at 1MHz internally. */

	/* If the current instruction is CLRWDT or SLEEP, don't update the WDT */

	if ((cpustate->opcode.w.l != 3) && (cpustate->opcode.w.l != 4))
	{
		UINT16 old_WDT = cpustate->WDT;

		cpustate->WDT -= counts;

		if (cpustate->WDT > 0x464f) {
			cpustate->WDT = 0x464f - (0xffff - cpustate->WDT);
		}

		if (((old_WDT != 0) && (old_WDT < cpustate->WDT)) || (cpustate->WDT == 0))
		{
			if (PSA) {
				cpustate->prescaler++;
				if (cpustate->prescaler >= (1 << PS)) {	/* Prescale values from 1 to 128 */
					cpustate->prescaler = 0;
					CLR(cpustate->STATUS, TO_FLAG);
					pic16c5x_soft_reset(cpustate);
				}
			}
			else {
				CLR(cpustate->STATUS, TO_FLAG);
				pic16c5x_soft_reset(cpustate);
			}
		}
	}
}


/****************************************************************************
 *  Update Timer
 ****************************************************************************/

static void pic16c5x_update_timer(pic16c5x_state *cpustate, int counts)
{
	if (PSA == 0) {
		cpustate->prescaler += counts;
		if (cpustate->prescaler >= (2 << PS)) {	/* Prescale values from 2 to 256 */
			cpustate->TMR0 += (cpustate->prescaler / (2 << PS));
			cpustate->prescaler %= (2 << PS);	/* Overflow prescaler */
		}
	}
	else {
		cpustate->TMR0 += counts;
	}
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

static CPU_EXECUTE( pic16c5x )
{
	pic16c5x_state *cpustate = get_safe_token(device);
	UINT8 T0_in;

	update_internalram_ptr(cpustate);

	do
	{
		if (PD == 0)						/* Sleep Mode */
		{
			cpustate->inst_cycles = 1;
			debugger_instruction_hook(device, cpustate->PC);
			if (WDTE) {
				pic16c5x_update_watchdog(cpustate, 1);
			}
		}
		else
		{
			cpustate->PREVPC = cpustate->PC;

			debugger_instruction_hook(device, cpustate->PC);

			cpustate->opcode.d = M_RDOP(cpustate->PC);
			cpustate->PC++;
			cpustate->PCL++;

			if ((cpustate->opcode.w.l & 0xff0) != 0x000)	{	/* Do all opcodes except the 00? ones */
				cpustate->inst_cycles = opcode_main[((cpustate->opcode.w.l >> 4) & 0xff)].cycles;
				(*opcode_main[((cpustate->opcode.w.l >> 4) & 0xff)].function)(cpustate);
			}
			else {	/* Opcode 0x00? has many opcodes in its minor nibble */
				cpustate->inst_cycles = opcode_00x[(cpustate->opcode.b.l & 0x1f)].cycles;
				(*opcode_00x[(cpustate->opcode.b.l & 0x1f)].function)(cpustate);
			}

			if (T0CS) {						/* Count mode */
				T0_in = S_T0_IN;
				if (T0_in) T0_in = 1;
				if (T0SE) {					/* Count falling edge T0 input */
					if (FALLING_EDGE_T0) {
						pic16c5x_update_timer(cpustate, 1);
					}
				}
				else {						/* Count rising edge T0 input */
					if (RISING_EDGE_T0) {
						pic16c5x_update_timer(cpustate, 1);
					}
				}
				cpustate->old_T0 = T0_in;
			}
			else {							/* Timer mode */
				if (cpustate->delay_timer) {
					cpustate->delay_timer--;
				}
				else {
					pic16c5x_update_timer(cpustate, cpustate->inst_cycles);
				}
			}
			if (WDTE) {
				pic16c5x_update_watchdog(cpustate, cpustate->inst_cycles);
			}
		}

		cpustate->icount -= cpustate->inst_cycles;

	} while (cpustate->icount > 0);
}



/**************************************************************************
 *  Generic set_info
 **************************************************************************/

static CPU_SET_INFO( pic16c5x )
{
	pic16c5x_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PIC16C5x_PC:		cpustate->PC = info->i; cpustate->PCL = info->i & 0xff ;break;
		/* This is actually not a stack pointer, but the stack contents */
		/* Stack is a 2 level First In Last Out stack */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PIC16C5x_STK1:		cpustate->STACK[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STK0:		cpustate->STACK[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_W:			cpustate->W      = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_ALU:		cpustate->ALU    = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STR:		cpustate->STATUS = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_OPT:		cpustate->OPTION = info->i & (T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG);	break;
		case CPUINFO_INT_REGISTER + PIC16C5x_TMR0:		cpustate->TMR0   = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_WDT:		cpustate->WDT    = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PSCL:		cpustate->prescaler = info->i;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTA:		cpustate->PORTA  = info->i & 0x0f;				break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTB:		cpustate->PORTB  = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTC:		cpustate->PORTC  = info->i;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_FSR:		cpustate->FSR    = ((info->i & cpustate->picRAMmask) | (UINT8)(~cpustate->picRAMmask));	break;
	}
}



/**************************************************************************
 *  Generic get_info
 **************************************************************************/

static CPU_GET_INFO( pic16c5x )
{
	pic16c5x_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(pic16c5x_state);	break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;						break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;						break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;		break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;						break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;						break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;						break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;						break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;						break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;						break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:			info->i = 16;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 9;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:			info->i = -1;						break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:			info->i = 8;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 5;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:			info->i = 0;						break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:				info->i = 8;						break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:				info->i = 5;						break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:				info->i = 0;						break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->PREVPC;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PIC16C5x_PC:		info->i = cpustate->PC;							break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PIC16C5x_STK1:		info->i = cpustate->STACK[1];					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STK0:		info->i = cpustate->STACK[0];					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_W:			info->i = cpustate->W;							break;
		case CPUINFO_INT_REGISTER + PIC16C5x_ALU:		info->i = cpustate->ALU;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_STR:		info->i = cpustate->STATUS;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_OPT:		info->i = cpustate->OPTION;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_TMR0:		info->i = cpustate->TMR0;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_WDT:		info->i = cpustate->WDT;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PSCL:		info->i = cpustate->prescaler;					break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTA:		info->i = cpustate->PORTA;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTB:		info->i = cpustate->PORTB;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_PRTC:		info->i = cpustate->PORTC;						break;
		case CPUINFO_INT_REGISTER + PIC16C5x_FSR:		info->i = ((cpustate->FSR & cpustate->picRAMmask) | (UINT8)(~cpustate->picRAMmask));	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(pic16c5x);	break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(pic16c5x);			break;
		case CPUINFO_FCT_RESET:							/* set per-CPU */								break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(pic16c5x);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(pic16c5x);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(pic16c5x);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C5x");					break;
		case CPUINFO_STR_FAMILY:						strcpy(info->s, "Microchip");					break;
		case CPUINFO_STR_VERSION:						strcpy(info->s, "1.14");						break;
		case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
		case CPUINFO_STR_CREDITS:						strcpy(info->s, "Copyright Tony La Porta");		break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%01x%c%c%c%c%c %c%c%c%03x",
				(cpustate->STATUS & 0xe0) >> 5,
				cpustate->STATUS & 0x10 ? '.':'O',		/* WDT Overflow */
				cpustate->STATUS & 0x08 ? 'P':'D',		/* Power/Down */
				cpustate->STATUS & 0x04 ? 'Z':'.',		/* Zero */
				cpustate->STATUS & 0x02 ? 'c':'b',		/* Nibble Carry/Borrow */
				cpustate->STATUS & 0x01 ? 'C':'B',		/* Carry/Borrow */

				cpustate->OPTION & 0x20 ? 'C':'T',		/* Counter/Timer */
				cpustate->OPTION & 0x10 ? 'N':'P',		/* Negative/Positive */
				cpustate->OPTION & 0x08 ? 'W':'T',		/* WatchDog/Timer */
				cpustate->OPTION & 0x08 ? (1<<(cpustate->OPTION&7)) : (2<<(cpustate->OPTION&7)) );
			break;

		case CPUINFO_STR_REGISTER + PIC16C5x_PC:		sprintf(info->s, "PC:%03X",   cpustate->PC);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_W:			sprintf(info->s, "W:%02X",    cpustate->W);					break;
		case CPUINFO_STR_REGISTER + PIC16C5x_ALU:		sprintf(info->s, "ALU:%02X",  cpustate->ALU);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_STR:		sprintf(info->s, "STR:%02X",  cpustate->STATUS);			break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TMR0:		sprintf(info->s, "TMR:%02X",  cpustate->TMR0);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_WDT:		sprintf(info->s, "WDT:%04X",  cpustate->WDT);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_OPT:		sprintf(info->s, "OPT:%02X",  cpustate->OPTION);			break;
		case CPUINFO_STR_REGISTER + PIC16C5x_STK0:		sprintf(info->s, "STK0:%03X", cpustate->STACK[0]);			break;
		case CPUINFO_STR_REGISTER + PIC16C5x_STK1:		sprintf(info->s, "STK1:%03X", cpustate->STACK[1]);			break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PRTA:		sprintf(info->s, "PRTA:%01X", ((cpustate->PORTA) & 0x0f));	break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PRTB:		sprintf(info->s, "PRTB:%02X", cpustate->PORTB);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PRTC:		sprintf(info->s, "PRTC:%02X", cpustate->PORTC);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TRSA:		sprintf(info->s, "TRSA:%01X", ((cpustate->TRISA) & 0x0f));	break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TRSB:		sprintf(info->s, "TRSB:%02X", cpustate->TRISB);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_TRSC:		sprintf(info->s, "TRSC:%02X", cpustate->TRISC);				break;
		case CPUINFO_STR_REGISTER + PIC16C5x_FSR:		sprintf(info->s, "FSR:%02X",  ((cpustate->FSR) & cpustate->picRAMmask) | (UINT8)(~cpustate->picRAMmask));	break;
		case CPUINFO_STR_REGISTER + PIC16C5x_PSCL:		sprintf(info->s, "PSCL:%c%02X", ((cpustate->OPTION & 0x08) ? 'W':'T'), cpustate->prescaler);	break;
	}
}



/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c54_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c54_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x07) AM_RAM
	AM_RANGE(0x08, 0x0f) AM_RAM
	AM_RANGE(0x10, 0x1f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C54 Reset
 ****************************************************************************/

static CPU_RESET( pic16c54 )
{
	pic16c5x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C54;
	cpustate->picRAMmask = 0x1f;
	cpustate->reset_vector = 0x1ff;
	pic16c5x_reset_regs(cpustate);
	CLR(cpustate->STATUS, PA_REG);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c54 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 9;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 5;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(pic16c54);					break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map16 = ADDRESS_MAP_NAME(pic16c54_rom);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(pic16c54_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C54");			break;

		default:										CPU_GET_INFO_CALL(pic16c5x);			break;
	}
}


/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c55_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c55_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x07) AM_RAM
	AM_RANGE(0x08, 0x0f) AM_RAM
	AM_RANGE(0x10, 0x1f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C55 Reset
 ****************************************************************************/

static CPU_RESET( pic16c55 )
{
	pic16c5x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C55;
	cpustate->picRAMmask = 0x1f;
	cpustate->reset_vector = 0x1ff;
	pic16c5x_reset_regs(cpustate);
	CLR(cpustate->STATUS, PA_REG);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c55 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 9;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 5;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(pic16c55);					break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map16 = ADDRESS_MAP_NAME(pic16c55_rom);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(pic16c55_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C55");			break;

		default:										CPU_GET_INFO_CALL(pic16c5x);			break;
	}
}


/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c56_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c56_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x07) AM_RAM
	AM_RANGE(0x08, 0x0f) AM_RAM
	AM_RANGE(0x10, 0x1f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C56 Reset
 ****************************************************************************/

static CPU_RESET( pic16c56 )
{
	pic16c5x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C56;
	cpustate->picRAMmask = 0x1f;
	cpustate->reset_vector = 0x3ff;
	pic16c5x_reset_regs(cpustate);
	CLR(cpustate->STATUS, PA_REG);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c56 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:			info->i = 10;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:			info->i = 5;							break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(pic16c56);					break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map16 = ADDRESS_MAP_NAME(pic16c56_rom);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(pic16c56_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C56");			break;

		default:										CPU_GET_INFO_CALL(pic16c5x);			break;
	}
}


/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c57_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c57_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x07) AM_RAM AM_MIRROR(0x60)
	AM_RANGE(0x08, 0x0f) AM_RAM AM_MIRROR(0x60)
	AM_RANGE(0x10, 0x1f) AM_RAM
	AM_RANGE(0x30, 0x3f) AM_RAM
	AM_RANGE(0x50, 0x5f) AM_RAM
	AM_RANGE(0x70, 0x7f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C57 Reset
 ****************************************************************************/

static CPU_RESET( pic16c57 )
{
	pic16c5x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C57;
	cpustate->picRAMmask = 0x7f;
	cpustate->reset_vector = 0x7ff;
	pic16c5x_reset_regs(cpustate);
	CLR(cpustate->STATUS, PA_REG);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c57 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 11;									break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 7;									break;
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(pic16c57);					break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map16 = ADDRESS_MAP_NAME(pic16c57_rom);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(pic16c57_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C57");			break;

		default:										CPU_GET_INFO_CALL(pic16c5x);			break;
	}
}


/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c58_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c58_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x07) AM_RAM AM_MIRROR(0x60)
	AM_RANGE(0x08, 0x0f) AM_RAM AM_MIRROR(0x60)
	AM_RANGE(0x10, 0x1f) AM_RAM
	AM_RANGE(0x30, 0x3f) AM_RAM
	AM_RANGE(0x50, 0x5f) AM_RAM
	AM_RANGE(0x70, 0x7f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C58 Reset
 ****************************************************************************/

static CPU_RESET( pic16c58 )
{
	pic16c5x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C58;
	cpustate->picRAMmask = 0x7f;
	cpustate->reset_vector = 0x7ff;
	pic16c5x_reset_regs(cpustate);
	CLR(cpustate->STATUS, PA_REG);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c58 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 11;								break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 7;								break;
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(pic16c58);					break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:	info->internal_map16 = ADDRESS_MAP_NAME(pic16c58_rom);	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map8 = ADDRESS_MAP_NAME(pic16c58_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "PIC16C58");			break;

		default:										CPU_GET_INFO_CALL(pic16c5x);			break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(PIC16C54, pic16c54);
DEFINE_LEGACY_CPU_DEVICE(PIC16C55, pic16c55);
DEFINE_LEGACY_CPU_DEVICE(PIC16C56, pic16c56);
DEFINE_LEGACY_CPU_DEVICE(PIC16C57, pic16c57);
DEFINE_LEGACY_CPU_DEVICE(PIC16C58, pic16c58);
