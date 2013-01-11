	/**************************************************************************\
	*                  Microchip PIC16C62X Emulator                            *
	*                                                                          *
	*                          Based On                                        *
	*                  Microchip PIC16C5X Emulator                             *
	*                    Copyright Tony La Porta                               *
	*                 Originally written for the MAME project.                 *
	*                                                                          *
	*                                                                          *
	*      Addressing architecture is based on the Harvard addressing scheme.  *
	*                                                                          *
	*                                                                          *
	*  **** Change Log ****                                                    *
	*  SZ (22-Oct-2009)                                                        *
	*   - Improvements and tests                                               *
	*  SZ (2-Oct-2009)                                                         *
	*   - Internal ram and registers                                           *
	*  SZ (12-Sep-2009)                                                        *
	*   - Started working on it.                                               *
	*                                                                          *
	*                                                                          *
	*  **** TODO ****                                                          *
	*   - Finish checking opcodes/instructions                                 *
	*   - Internal devices                                                     *
	*   - Interrupts                                                           *
	*   - Everything !                                                         *
	*                                                                          *
	*  **** DONE ****                                                          *
	*   - I/O ports                                                            *
	*   - Savestates                                                           *
	*   - Internal memory                                                      *
	*   - New opcodes                                                          *
	*   - Opcode disassembly                                                   *
	*                                                                          *
	*  **** Notes (from PIC16C5X): ****                                        *
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
#include "pic16c62x.h"




struct pic16c62x_state
{
	/******************** CPU Internal Registers *******************/
	UINT16  PC;
	UINT16  PREVPC;     /* previous program counter */
	UINT8   W;
	UINT8   PCLATH;     /* 0a,8a */
	UINT8   OPTION;     /* 81 */
	UINT16  CONFIG;
	UINT8   ALU;
	UINT16  WDT;
	UINT8   TRISA;      /* 85 */
	UINT8   TRISB;      /* 86 */
	UINT16  STACK[8];
	UINT16  prescaler;  /* Note: this is really an 8-bit register */
	PAIR    opcode;
	UINT8   *internalram;

	int     icount;
	int     reset_vector;
	int     picmodel;
	int     delay_timer;
	UINT16  temp_config;
	UINT8   old_T0;
	INT8    old_data;
	UINT8   picRAMmask;
	int     inst_cycles;


	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;
	address_space *io;
};

INLINE pic16c62x_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == PIC16C620 ||
			device->type() == PIC16C620A ||
//         device->type() == PIC16CR620A ||
			device->type() == PIC16C621 ||
			device->type() == PIC16C621A ||
			device->type() == PIC16C622 ||
			device->type() == PIC16C622A);
	return (pic16c62x_state *)downcast<legacy_cpu_device *>(device)->token();
}


/* opcode table entry */
struct pic16c62x_opcode
{
	UINT8   cycles;
	void    (*function)(pic16c62x_state *);
};
/* instruction list entry */
struct pic16c62x_instruction
{
	char    *format;
	void    (*function)(pic16c62x_state *);
	UINT8   cycles;
};


INLINE void update_internalram_ptr(pic16c62x_state *cpustate)
{
	cpustate->internalram = (UINT8 *)cpustate->data->get_write_ptr(0x00);
}

#define PIC16C62x_RDOP(A)         (cpustate->direct->read_decrypted_word((A)<<1))
#define PIC16C62x_RAM_RDMEM(A)    ((UINT8)cpustate->data->read_byte(A))
#define PIC16C62x_RAM_WRMEM(A,V)  (cpustate->data->write_byte(A,V))
#define PIC16C62x_In(Port)        ((UINT8)cpustate->io->read_byte((Port)))
#define PIC16C62x_Out(Port,Value) (cpustate->io->write_byte((Port),Value))
/************  Read the state of the T0 Clock input signal  ************/
#define PIC16C62x_T0_In           (cpustate->io->read_byte(PIC16C62x_T0) >> 4)

#define M_RDRAM(A)      (((A) == 0) ? cpustate->internalram[0] : PIC16C62x_RAM_RDMEM(A))
#define M_WRTRAM(A,V)   do { if ((A) == 0) cpustate->internalram[0] = (V); else PIC16C62x_RAM_WRMEM(A,V); } while (0)
#define M_RDOP(A)       PIC16C62x_RDOP(A)
#define P_IN(A)         PIC16C62x_In(A)
#define P_OUT(A,V)      PIC16C62x_Out(A,V)
#define S_T0_IN         PIC16C62x_T0_In
#define ADDR_MASK       0x1fff



#define TMR0    internalram[1]
#define PCL     internalram[2]
#define STATUS  internalram[3]
#define FSR     internalram[4]
#define PORTA   internalram[5]
#define PORTB   internalram[6]
#define INDF    M_RDRAM(cpustate->FSR)

#define  RISING_EDGE_T0  (( (int)(T0_in - cpustate->old_T0) > 0) ? 1 : 0)
#define FALLING_EDGE_T0  (( (int)(T0_in - cpustate->old_T0) < 0) ? 1 : 0)


/********  The following is the Status Flag register definition.  *********/
			/* | 7 | 6 | 5 |  4 |  3 | 2 |  1 | 0 | */
			/* |IRP|RP1|RP0| TO | PD | Z | DC | C | */
#define IRP_FLAG    0x80    /* IRP  Register Bank Select bit (used for indirect addressing) */
#define RP1_FLAG    0x40    /* RP1  Register Bank Select bits (used for direct addressing) */
#define RP0_FLAG    0x20    /* RP0  Register Bank Select bits (used for direct addressing) */
#define TO_FLAG     0x10    /* TO   Time Out flag (WatchDog) */
#define PD_FLAG     0x08    /* PD   Power Down flag */
#define Z_FLAG      0x04    /* Z    Zero Flag */
#define DC_FLAG     0x02    /* DC   Digit Carry/Borrow flag (Nibble) */
#define C_FLAG      0x01    /* C    Carry/Borrow Flag (Byte) */

#define IRP     (cpustate->STATUS & IRP_FLAG)
#define RP1     (cpustate->STATUS & RP1_FLAG)
#define RP0     (cpustate->STATUS & RP0_FLAG)
#define TO      (cpustate->STATUS & TO_FLAG)
#define PD      (cpustate->STATUS & PD_FLAG)
#define ZERO    (cpustate->STATUS & Z_FLAG)
#define DC      (cpustate->STATUS & DC_FLAG)
#define CARRY   (cpustate->STATUS & C_FLAG)

#define ADDR    ((cpustate->opcode.b.l & 0x7f) | (RP0 << 2))

/********  The following is the Option Flag register definition.  *********/
			/* |   7  |   6    |   5  |   4  |  3  | 2 | 1 | 0 | */
			/* | RBPU | INTEDG | TOCS | TOSE | PSA |    PS     | */
#define RBPU_FLAG   0x80    /* RBPU     Pull-up Enable */
#define INTEDG_FLAG 0x40    /* INTEDG   Interrupt Edge Select */
#define T0CS_FLAG   0x20    /* TOCS     Timer 0 clock source select */
#define T0SE_FLAG   0x10    /* TOSE     Timer 0 clock source edge select */
#define PSA_FLAG    0x08    /* PSA      Prescaler Assignment bit */
#define PS_REG      0x07    /* PS       Prescaler Rate select */

#define T0CS    (cpustate->OPTION & T0CS_FLAG)
#define T0SE    (cpustate->OPTION & T0SE_FLAG)
#define PSA     (cpustate->OPTION & PSA_FLAG)
#define PS      (cpustate->OPTION & PS_REG)

/********  The following is the Config Flag register definition.  *********/
	/* | 13 | 12 | 11 | 10 | 9 | 8 | 7 |   6   | 5 | 4 |   3   |   2  | 1 | 0 | */
	/* |           CP              |   | BODEN |  CP   | PWRTE | WDTE |  FOSC | */
	/* CP       Code Protect (ROM read protect) */
#define BODEN_FLAG  0x40    /* BODEN    Brown-out Reset Enable */
#define PWRTE_FLAG  0x08    /* PWRTE    Power-up Timer Enable */
#define WDTE_FLAG   0x04    /* WDTE     WatchDog Timer enable */
#define FOSC_FLAG   0x03    /* FOSC     Oscillator source select */

#define WDTE    (cpustate->CONFIG & WDTE_FLAG)
#define FOSC    (cpustate->CONFIG & FOSC_FLAG)


/************************************************************************
 *  Shortcuts
 ************************************************************************/

#define CLR(flagreg, flag) ( flagreg &= (UINT8)(~flag) )
#define SET(flagreg, flag) ( flagreg |=  flag )


/* Easy bit position selectors */
#define POS  ((cpustate->opcode.w.l >> 7) & 7)
static const unsigned int bit_clr[8] = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
static const unsigned int bit_set[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };



INLINE void CALCULATE_Z_FLAG(pic16c62x_state *cpustate)
{
	if (cpustate->ALU == 0) SET(cpustate->STATUS, Z_FLAG);
	else CLR(cpustate->STATUS, Z_FLAG);
}

INLINE void CALCULATE_ADD_CARRY(pic16c62x_state *cpustate)
{
	if ((UINT8)(cpustate->old_data) > (UINT8)(cpustate->ALU)) {
		SET(cpustate->STATUS, C_FLAG);
	}
	else {
		CLR(cpustate->STATUS, C_FLAG);
	}
}

INLINE void CALCULATE_ADD_DIGITCARRY(pic16c62x_state *cpustate)
{
	if (((UINT8)(cpustate->old_data) & 0x0f) > ((UINT8)(cpustate->ALU) & 0x0f)) {
		SET(cpustate->STATUS, DC_FLAG);
	}
	else {
		CLR(cpustate->STATUS, DC_FLAG);
	}
}

INLINE void CALCULATE_SUB_CARRY(pic16c62x_state *cpustate)
{
	if ((UINT8)(cpustate->old_data) < (UINT8)(cpustate->ALU)) {
		CLR(cpustate->STATUS, C_FLAG);
	}
	else {
		SET(cpustate->STATUS, C_FLAG);
	}
}

INLINE void CALCULATE_SUB_DIGITCARRY(pic16c62x_state *cpustate)
{
	if (((UINT8)(cpustate->old_data) & 0x0f) < ((UINT8)(cpustate->ALU) & 0x0f)) {
		CLR(cpustate->STATUS, DC_FLAG);
	}
	else {
		SET(cpustate->STATUS, DC_FLAG);
	}
}



INLINE UINT16 POP_STACK(pic16c62x_state *cpustate)
{
	UINT16 data = cpustate->STACK[7];
	cpustate->STACK[7] = cpustate->STACK[6];
	cpustate->STACK[6] = cpustate->STACK[5];
	cpustate->STACK[5] = cpustate->STACK[4];
	cpustate->STACK[4] = cpustate->STACK[3];
	cpustate->STACK[3] = cpustate->STACK[2];
	cpustate->STACK[2] = cpustate->STACK[1];
	cpustate->STACK[1] = cpustate->STACK[0];
	return (data & ADDR_MASK);
}
INLINE void PUSH_STACK(pic16c62x_state *cpustate, UINT16 data)
{
	cpustate->STACK[0] = cpustate->STACK[1];
	cpustate->STACK[1] = cpustate->STACK[2];
	cpustate->STACK[2] = cpustate->STACK[3];
	cpustate->STACK[3] = cpustate->STACK[4];
	cpustate->STACK[4] = cpustate->STACK[5];
	cpustate->STACK[5] = cpustate->STACK[6];
	cpustate->STACK[6] = cpustate->STACK[7];
	cpustate->STACK[7] = (data & ADDR_MASK);
}



INLINE UINT8 GET_REGFILE(pic16c62x_state *cpustate, offs_t addr)    /* Read from internal memory */
{
	UINT8 data;

	if (addr == 0) {                        /* Indirect addressing  */
		addr = (cpustate->FSR & cpustate->picRAMmask);
	}

	switch(addr)
	{
		case 0x00:  /* Not an actual register, so return 0 */
		case 0x80:
					data = 0;
					break;
		case 0x02:
		case 0x03:
		case 0x0b:
		case 0x82:
		case 0x83:
		case 0x8b:
					data = M_RDRAM(addr & 0x7f);
					break;
		case 0x84:
		case 0x04:  data = (cpustate->FSR | (UINT8)(~cpustate->picRAMmask));
					break;
		case 0x05:  data = P_IN(0);
					data &= cpustate->TRISA;
					data |= ((UINT8)(~cpustate->TRISA) & cpustate->PORTA);
					data &= 0x1f;       /* 5-bit port (only lower 5 bits used) */
					break;
		case 0x06:  data = P_IN(1);
					data &= cpustate->TRISB;
					data |= ((UINT8)(~cpustate->TRISB) & cpustate->PORTB);
					break;
		case 0x8a:
		case 0x0a:  data = cpustate->PCLATH;
					break;
		case 0x81:  data = cpustate->OPTION;
					break;
		case 0x85:  data = cpustate->TRISA;
					break;
		case 0x86:  data = cpustate->TRISB;
					break;
		default:    data = M_RDRAM(addr);
					break;
	}
	return data;
}

INLINE void STORE_REGFILE(pic16c62x_state *cpustate, offs_t addr, UINT8 data)   /* Write to internal memory */
{
	if (addr == 0) {                        /* Indirect addressing  */
		addr = (cpustate->FSR & cpustate->picRAMmask);
	}

	switch(addr)
	{
		case 0x80:
		case 0x00:  /* Not an actual register, nothing to save */
					break;
		case 0x01:  cpustate->delay_timer = 2;      /* Timer starts after next two instructions */
					if (PSA == 0) cpustate->prescaler = 0;  /* Must clear the Prescaler */
					cpustate->TMR0 = data;
					break;
		case 0x82:
		case 0x02:  cpustate->PCL = data;
					cpustate->PC = (cpustate->PCLATH << 8) | data;
					break;
		case 0x83:
		case 0x03:  cpustate->STATUS &= (UINT8)(~(IRP_FLAG|RP1_FLAG|RP0_FLAG)); cpustate->STATUS |= (data & (IRP_FLAG|RP1_FLAG|RP0_FLAG));
					break;
		case 0x84:
		case 0x04:  cpustate->FSR = (data | (UINT8)(~cpustate->picRAMmask));
					break;
		case 0x05:  data &= 0x1f;       /* 5-bit port (only lower 5 bits used) */
					P_OUT(0,data & (UINT8)(~cpustate->TRISA)); cpustate->PORTA = data;
					break;
		case 0x06:  P_OUT(1,data & (UINT8)(~cpustate->TRISB)); cpustate->PORTB = data;
					break;
		case 0x8a:
		case 0x0a:
					cpustate->PCLATH = data & 0x1f;
					M_WRTRAM(0x0a, cpustate->PCLATH);
					break;
		case 0x8b:
		case 0x0b:  M_WRTRAM(0x0b, data);
					break;
		case 0x81:  cpustate->OPTION = data;
					M_WRTRAM(0x81, data);
					break;
		case 0x85:  if   (cpustate->TRISA != data)
					{
						cpustate->TRISA = data | 0xf0;
						P_OUT(2,cpustate->TRISA);
						P_OUT(0,cpustate->PORTA & (UINT8)(~cpustate->TRISA) & 0x0f);
						M_WRTRAM(addr, data);
					}
					break;
		case 0x86:  if   (cpustate->TRISB != data)
					{
						cpustate->TRISB = data;
						P_OUT(3,cpustate->TRISB);
						P_OUT(1,cpustate->PORTB & (UINT8)(~cpustate->TRISB));
						M_WRTRAM(addr, data);
					}
					break;
		default:    M_WRTRAM(addr, data);
					break;
	}
}


INLINE void STORE_RESULT(pic16c62x_state *cpustate, offs_t addr, UINT8 data)
{
	if (cpustate->opcode.b.l & 0x80)
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


static void illegal(pic16c62x_state *cpustate)
{
	logerror("PIC16C62x:  PC=%03x,  Illegal opcode = %04x\n", (cpustate->PC-1), cpustate->opcode.w.l);
}


static void addwf(pic16c62x_state *cpustate)
{
	cpustate->old_data = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU = cpustate->old_data + cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
	CALCULATE_ADD_CARRY(cpustate);
	CALCULATE_ADD_DIGITCARRY(cpustate);
}

static void addlw(pic16c62x_state *cpustate)
{
	cpustate->ALU = (cpustate->opcode.b.l & 0xff) + cpustate->W;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
	CALCULATE_ADD_CARRY(cpustate);
	CALCULATE_ADD_DIGITCARRY(cpustate);
}

static void andwf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) & cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void andlw(pic16c62x_state *cpustate)
{
	cpustate->ALU = cpustate->opcode.b.l & cpustate->W;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
}

static void bcf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU &= bit_clr[POS];
	STORE_REGFILE(cpustate, ADDR, cpustate->ALU);
}

static void bsf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU |= bit_set[POS];
	STORE_REGFILE(cpustate, ADDR, cpustate->ALU);
}

static void btfss(pic16c62x_state *cpustate)
{
	if ((GET_REGFILE(cpustate, ADDR) & bit_set[POS]) == bit_set[POS])
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;     /* Add NOP cycles */
	}
}

static void btfsc(pic16c62x_state *cpustate)
{
	if ((GET_REGFILE(cpustate, ADDR) & bit_set[POS]) == 0)
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;     /* Add NOP cycles */
	}
}

static void call(pic16c62x_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->PC);
	cpustate->PC = ((cpustate->PCLATH & 0x18) << 8) | (cpustate->opcode.w.l & 0x7ff);
	cpustate->PC &= ADDR_MASK;
	cpustate->PCL = cpustate->PC & 0xff;
}

static void clrw(pic16c62x_state *cpustate)
{
	cpustate->W = 0;
	SET(cpustate->STATUS, Z_FLAG);
}

static void clrf(pic16c62x_state *cpustate)
{
	STORE_REGFILE(cpustate, ADDR, 0);
	SET(cpustate->STATUS, Z_FLAG);
}

static void clrwdt(pic16c62x_state *cpustate)
{
	cpustate->WDT = 0;
	if (PSA) cpustate->prescaler = 0;
	SET(cpustate->STATUS, TO_FLAG);
	SET(cpustate->STATUS, PD_FLAG);
}

static void comf(pic16c62x_state *cpustate)
{
	cpustate->ALU = (UINT8)(~(GET_REGFILE(cpustate, ADDR)));
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void decf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) - 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void decfsz(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) - 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	if (cpustate->ALU == 0)
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;     /* Add NOP cycles */
	}
}

static void goto_op(pic16c62x_state *cpustate)
{
	cpustate->PC = ((cpustate->PCLATH & 0x18) << 8) | (cpustate->opcode.w.l & 0x7ff);
	cpustate->PC &= ADDR_MASK;
	cpustate->PCL = cpustate->PC & 0xff;
}

static void incf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) + 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void incfsz(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) + 1;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	if (cpustate->ALU == 0)
	{
		cpustate->PC++ ;
		cpustate->PCL = cpustate->PC & 0xff;
		cpustate->inst_cycles += 1;     /* Add NOP cycles */
	}
}

static void iorlw(pic16c62x_state *cpustate)
{
	cpustate->ALU = cpustate->opcode.b.l | cpustate->W;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
}

static void iorwf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) | cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void movf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}

static void movlw(pic16c62x_state *cpustate)
{
	cpustate->W = cpustate->opcode.b.l;
}

static void movwf(pic16c62x_state *cpustate)
{
	STORE_REGFILE(cpustate, ADDR, cpustate->W);
}

static void nop(pic16c62x_state *cpustate)
{
	/* Do nothing */
}

static void option(pic16c62x_state *cpustate)
{
	cpustate->OPTION = cpustate->W;
}

static void retlw(pic16c62x_state *cpustate)
{
	cpustate->W = cpustate->opcode.b.l;
	cpustate->PC = POP_STACK(cpustate);
	cpustate->PCL = cpustate->PC & 0xff;
}

static void returns(pic16c62x_state *cpustate)
{
	cpustate->PC = POP_STACK(cpustate);
	cpustate->PCL = cpustate->PC & 0xff;
}

static void retfie(pic16c62x_state *cpustate)
{
	cpustate->PC = POP_STACK(cpustate);
	cpustate->PCL = cpustate->PC & 0xff;
	//INTCON(7)=1;
}

static void rlf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU <<= 1;
	if (cpustate->STATUS & C_FLAG) cpustate->ALU |= 1;
	if (GET_REGFILE(cpustate, ADDR) & 0x80) SET(cpustate->STATUS, C_FLAG);
	else CLR(cpustate->STATUS, C_FLAG);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
}

static void rrf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU >>= 1;
	if (cpustate->STATUS & C_FLAG) cpustate->ALU |= 0x80;
	if (GET_REGFILE(cpustate, ADDR) & 1) SET(cpustate->STATUS, C_FLAG);
	else CLR(cpustate->STATUS, C_FLAG);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
}

static void sleepic(pic16c62x_state *cpustate)
{
	if (WDTE) cpustate->WDT = 0;
	if (PSA) cpustate->prescaler = 0;
	SET(cpustate->STATUS, TO_FLAG);
	CLR(cpustate->STATUS, PD_FLAG);
}

static void subwf(pic16c62x_state *cpustate)
{
	cpustate->old_data = GET_REGFILE(cpustate, ADDR);
	cpustate->ALU = cpustate->old_data - cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
	CALCULATE_SUB_CARRY(cpustate);
	CALCULATE_SUB_DIGITCARRY(cpustate);
}

static void sublw(pic16c62x_state *cpustate)
{
	cpustate->ALU = (cpustate->opcode.b.l & 0xff) - cpustate->W;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
	CALCULATE_SUB_CARRY(cpustate);
	CALCULATE_SUB_DIGITCARRY(cpustate);
}

static void swapf(pic16c62x_state *cpustate)
{
	cpustate->ALU  = ((GET_REGFILE(cpustate, ADDR) << 4) & 0xf0);
	cpustate->ALU |= ((GET_REGFILE(cpustate, ADDR) >> 4) & 0x0f);
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
}

static void tris(pic16c62x_state *cpustate)
{
	switch(cpustate->opcode.b.l & 0x7)
	{
		case 05:    STORE_REGFILE(cpustate, 0x85, cpustate->W); break;
		case 06:    STORE_REGFILE(cpustate, 0x86, cpustate->W); break;
		default:    illegal(cpustate); break;
	}
}

static void xorlw(pic16c62x_state *cpustate)
{
	cpustate->ALU = cpustate->W ^ cpustate->opcode.b.l;
	cpustate->W = cpustate->ALU;
	CALCULATE_Z_FLAG(cpustate);
}

static void xorwf(pic16c62x_state *cpustate)
{
	cpustate->ALU = GET_REGFILE(cpustate, ADDR) ^ cpustate->W;
	STORE_RESULT(cpustate, ADDR, cpustate->ALU);
	CALCULATE_Z_FLAG(cpustate);
}


/***********************************************************************
 *  Instruction Table (Format, Instruction, Cycles)
 ***********************************************************************/

static const pic16c62x_instruction instructiontable[]=
{
	{(char *)"000111dfffffff", addwf, 1},
	{(char *)"000101dfffffff", andwf, 1},
	{(char *)"0000011fffffff", clrf, 1},
	{(char *)"00000100000011", clrw, 1},
	{(char *)"001001dfffffff", comf, 1},
	{(char *)"000011dfffffff", decf, 1},
	{(char *)"001011dfffffff", decfsz, 1},
	{(char *)"001010dfffffff", incf, 1},
	{(char *)"001111dfffffff", incfsz, 1},
	{(char *)"000100dfffffff", iorwf, 1},
	{(char *)"001000dfffffff", movf, 1},
	{(char *)"0000001fffffff", movwf, 1},
	{(char *)"0000000xx00000", nop, 1},
	{(char *)"001101dfffffff", rlf, 1},
	{(char *)"001100dfffffff", rrf, 1},
	{(char *)"000010dfffffff", subwf, 1},
	{(char *)"001110dfffffff", swapf, 1},
	{(char *)"000110dfffffff", xorwf, 1},
	{(char *)"0100bbbfffffff", bcf, 1},
	{(char *)"0101bbbfffffff", bsf, 1},
	{(char *)"0110bbbfffffff", btfsc, 1},
	{(char *)"0111bbbfffffff", btfss, 1},
	{(char *)"11111xkkkkkkkk", addlw, 1},
	{(char *)"111001kkkkkkkk", andlw, 1},
	{(char *)"100aaaaaaaaaaa", call, 2},
	{(char *)"101aaaaaaaaaaa", goto_op, 2},
	{(char *)"111000kkkkkkkk", iorlw, 1},
	{(char *)"1100xxkkkkkkkk", movlw, 1},
	{(char *)"00000000001001", retfie, 2},
	{(char *)"1101xxkkkkkkkk", retlw, 2},
	{(char *)"00000000001000", returns, 2},
	{(char *)"00000001100011", sleepic, 1},
	{(char *)"11110xkkkkkkkk", sublw, 1},
	{(char *)"111010kkkkkkkk", xorlw, 1},
	{(char *)"00000001100100", clrwdt, 1},
	{(char *)"00000001100010", option, 1},      // deprecated
	{(char *)"00000001100fff", tris, 1},        // deprecated
	{NULL, NULL, 0}
};

/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

static pic16c62x_opcode opcode_table[16384];

/***********************************************************************
 *  Opcode Table build function
 ***********************************************************************/

static void build_opcode_table(void)
{
int instr,mask,bits;
int a;

	// defaults
	for ( a = 0; a < 16384; a++)
	{
		opcode_table[a].cycles = 0;
		opcode_table[a].function = illegal;
	}
	// build table
	for( instr = 0; instructiontable[instr].cycles != 0; instr++)
	{
		bits=0;
		mask=0;
		for ( a = 0; a < 14; a++)
		{
			switch (instructiontable[instr].format[a])
			{
				case '0':
					bits = bits << 1;
					mask = (mask << 1) | 1;
					break;
				case '1':
					bits = (bits << 1) | 1;
					mask = (mask << 1) | 1;
					break;
				default:
					bits = bits << 1;
					mask = mask << 1;
					break;
			}
		}
		for ( a = 0; a < 16384; a++)
		{
			if (((a & mask) == bits) && (opcode_table[a].cycles == 0))
			{
				opcode_table[a].cycles = instructiontable[instr].cycles;
				opcode_table[a].function = instructiontable[instr].function;
			}
		}
	}
}

/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

static CPU_INIT( pic16c62x )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = &device->space(AS_DATA);
	cpustate->io = &device->space(AS_IO);

	cpustate->CONFIG = 0x3fff;

	/* ensure the internal ram pointers are set before get_info is called */
	update_internalram_ptr(cpustate);

	build_opcode_table();

	device->save_item(NAME(cpustate->W));
	device->save_item(NAME(cpustate->ALU));
	device->save_item(NAME(cpustate->OPTION));
	device->save_item(NAME(cpustate->PCLATH));
	device->save_item(NAME(cpustate->TMR0));
	device->save_item(NAME(cpustate->PCL));
	device->save_item(NAME(cpustate->STATUS));
	device->save_item(NAME(cpustate->FSR));
	device->save_item(NAME(cpustate->PORTA));
	device->save_item(NAME(cpustate->PORTB));
	device->save_item(NAME(cpustate->TRISA));
	device->save_item(NAME(cpustate->TRISB));
	device->save_item(NAME(cpustate->old_T0));
	device->save_item(NAME(cpustate->old_data));
	device->save_item(NAME(cpustate->picRAMmask));
	device->save_item(NAME(cpustate->WDT));
	device->save_item(NAME(cpustate->prescaler));
	device->save_item(NAME(cpustate->STACK[0]));
	device->save_item(NAME(cpustate->STACK[1]));
	device->save_item(NAME(cpustate->STACK[2]));
	device->save_item(NAME(cpustate->STACK[3]));
	device->save_item(NAME(cpustate->STACK[4]));
	device->save_item(NAME(cpustate->STACK[5]));
	device->save_item(NAME(cpustate->STACK[6]));
	device->save_item(NAME(cpustate->STACK[7]));
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

static void pic16c62x_reset_regs(pic16c62x_state *cpustate)
{
	cpustate->PC     = cpustate->reset_vector;
	cpustate->TRISA  = 0x1f;
	cpustate->TRISB  = 0xff;
	cpustate->OPTION = 0xff;
	cpustate->STATUS = 0x18;
	cpustate->PCL    = 0;
	cpustate->FSR   |= (UINT8)(~cpustate->picRAMmask);
	cpustate->PORTA  = 0;
	cpustate->prescaler = 0;
	cpustate->delay_timer = 0;
	cpustate->old_T0 = 0;
	cpustate->inst_cycles = 0;
	PIC16C62x_RAM_WRMEM(0x85,cpustate->TRISA);
	PIC16C62x_RAM_WRMEM(0x86,cpustate->TRISB);
	PIC16C62x_RAM_WRMEM(0x81,cpustate->OPTION);
}

static void pic16c62x_soft_reset(pic16c62x_state *cpustate)
{
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG | Z_FLAG | DC_FLAG | C_FLAG));
	pic16c62x_reset_regs(cpustate);
}

void pic16c62x_set_config(device_t *cpu, int data)
{
	pic16c62x_state *cpustate = get_safe_token(cpu);

	logerror("Writing %04x to the PIC16C62x configuration bits\n",data);
	cpustate->CONFIG = (data & 0x3fff);
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/

static CPU_EXIT( pic16c62x )
{
	/* nothing to do */
}


/****************************************************************************
 *  WatchDog
 ****************************************************************************/

static void pic16c62x_update_watchdog(pic16c62x_state *cpustate, int counts)
{
	/* TODO: needs updating */
	/* WatchDog is set up to count 18,000 (0x464f hex) ticks to provide */
	/* the timeout period of 0.018ms based on a 4MHz input clock. */
	/* Note: the 4MHz clock should be divided by the PIC16C5x_CLOCK_DIVIDER */
	/* which effectively makes the PIC run at 1MHz internally. */

	/* If the current instruction is CLRWDT or SLEEP, don't update the WDT */

	if ((cpustate->opcode.w.l != 0x64) && (cpustate->opcode.w.l != 0x63))
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
				if (cpustate->prescaler >= (1 << PS)) { /* Prescale values from 1 to 128 */
					cpustate->prescaler = 0;
					CLR(cpustate->STATUS, TO_FLAG);
					pic16c62x_soft_reset(cpustate);
				}
			}
			else {
				CLR(cpustate->STATUS, TO_FLAG);
				pic16c62x_soft_reset(cpustate);
			}
		}
	}
}


/****************************************************************************
 *  Update Timer
 ****************************************************************************/

static void pic16c62x_update_timer(pic16c62x_state *cpustate, int counts)
{
	if (PSA == 0) {
		cpustate->prescaler += counts;
		if (cpustate->prescaler >= (2 << PS)) { /* Prescale values from 2 to 256 */
			cpustate->TMR0 += (cpustate->prescaler / (2 << PS));
			cpustate->prescaler %= (2 << PS);   /* Overflow prescaler */
		}
	}
	else {
		cpustate->TMR0 += counts;
	}
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

static CPU_EXECUTE( pic16c62x )
{
	pic16c62x_state *cpustate = get_safe_token(device);
	UINT8 T0_in;

	update_internalram_ptr(cpustate);

	do
	{
		if (PD == 0)                        /* Sleep Mode */
		{
			cpustate->inst_cycles = 1;
			debugger_instruction_hook(device, cpustate->PC);
			if (WDTE) {
				pic16c62x_update_watchdog(cpustate, 1);
			}
		}
		else
		{
			cpustate->PREVPC = cpustate->PC;

			debugger_instruction_hook(device, cpustate->PC);

			cpustate->opcode.d = M_RDOP(cpustate->PC);
			cpustate->PC++;
			cpustate->PCL++;

			cpustate->inst_cycles = opcode_table[cpustate->opcode.w.l & 16383].cycles;
			(*opcode_table[cpustate->opcode.w.l & 16383].function)(cpustate);

			if (T0CS) {                     /* Count mode */
				T0_in = S_T0_IN;
				if (T0_in) T0_in = 1;
				if (T0SE) {                 /* Count falling edge T0 input */
					if (FALLING_EDGE_T0) {
						pic16c62x_update_timer(cpustate, 1);
					}
				}
				else {                      /* Count rising edge T0 input */
					if (RISING_EDGE_T0) {
						pic16c62x_update_timer(cpustate, 1);
					}
				}
				cpustate->old_T0 = T0_in;
			}
			else {                          /* Timer mode */
				if (cpustate->delay_timer) {
					cpustate->delay_timer--;
				}
				else {
					pic16c62x_update_timer(cpustate, cpustate->inst_cycles);
				}
			}
			if (WDTE) {
				pic16c62x_update_watchdog(cpustate, cpustate->inst_cycles);
			}
		}

		cpustate->icount -= cpustate->inst_cycles;

	} while (cpustate->icount > 0);
}



/**************************************************************************
 *  Generic set_info
 **************************************************************************/

static CPU_SET_INFO( pic16c62x )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PIC16C62x_PC:       cpustate->PC = info->i; cpustate->PCL = info->i & 0xff ;break;
		/* This is actually not a stack pointer, but the stack contents */
		/* Stack is a 8 level First In Last Out stack */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PIC16C62x_STK7:     cpustate->STACK[7] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK6:     cpustate->STACK[6] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK5:     cpustate->STACK[5] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK4:     cpustate->STACK[4] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK3:     cpustate->STACK[3] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK2:     cpustate->STACK[2] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK1:     cpustate->STACK[1] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK0:     cpustate->STACK[0] = info->i;                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_W:        cpustate->W      = info->i;                     break;
		case CPUINFO_INT_REGISTER + PIC16C62x_ALU:      cpustate->ALU    = info->i;                     break;
		case CPUINFO_INT_REGISTER + PIC16C62x_OPT:      cpustate->OPTION = info->i & (RBPU_FLAG | INTEDG_FLAG | T0CS_FLAG | T0SE_FLAG | PSA_FLAG | PS_REG); break;
		case CPUINFO_INT_REGISTER + PIC16C62x_TMR0:     cpustate->TMR0   = info->i;                     break;
		case CPUINFO_INT_REGISTER + PIC16C62x_WDT:      cpustate->WDT    = info->i;                     break;
		case CPUINFO_INT_REGISTER + PIC16C62x_PSCL:     cpustate->prescaler = info->i;                  break;
		case CPUINFO_INT_REGISTER + PIC16C62x_PRTA:     cpustate->PORTA  = info->i & 0x1f;              break;
		case CPUINFO_INT_REGISTER + PIC16C62x_PRTB:     cpustate->PORTB  = info->i;                     break;
		case CPUINFO_INT_REGISTER + PIC16C62x_FSR:      cpustate->FSR    = ((info->i & cpustate->picRAMmask) | (UINT8)(~cpustate->picRAMmask)); break;
	}
}



/**************************************************************************
 *  Generic get_info
 **************************************************************************/

static CPU_GET_INFO( pic16c62x )
{
	pic16c62x_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:                  info->i = sizeof(pic16c62x_state);  break;
		case CPUINFO_INT_INPUT_LINES:                   info->i = 1;                        break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:            info->i = 0;                        break;
		case CPUINFO_INT_ENDIANNESS:                    info->i = ENDIANNESS_LITTLE;        break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:              info->i = 1;                        break;
		case CPUINFO_INT_CLOCK_DIVIDER:                 info->i = 4;                        break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:         info->i = 2;                        break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:         info->i = 2;                        break;
		case CPUINFO_INT_MIN_CYCLES:                    info->i = 1;                        break;
		case CPUINFO_INT_MAX_CYCLES:                    info->i = 2;                        break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:            info->i = 16;                       break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:            info->i = 12;                       break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM:            info->i = -1;                       break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:           info->i = 8;                        break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:           info->i = 8;                        break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:           info->i = 0;                        break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:             info->i = 8;                        break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:             info->i = 5;                        break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:             info->i = 0;                        break;

		case CPUINFO_INT_PREVIOUSPC:                    info->i = cpustate->PREVPC;                     break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + PIC16C62x_PC:       info->i = cpustate->PC;                         break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + PIC16C62x_STK7:     info->i = cpustate->STACK[7];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK6:     info->i = cpustate->STACK[6];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK5:     info->i = cpustate->STACK[5];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK4:     info->i = cpustate->STACK[4];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK3:     info->i = cpustate->STACK[3];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK2:     info->i = cpustate->STACK[2];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK1:     info->i = cpustate->STACK[1];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STK0:     info->i = cpustate->STACK[0];                   break;
		case CPUINFO_INT_REGISTER + PIC16C62x_W:            info->i = cpustate->W;                          break;
		case CPUINFO_INT_REGISTER + PIC16C62x_ALU:      info->i = cpustate->ALU;                        break;
		case CPUINFO_INT_REGISTER + PIC16C62x_STR:      info->i = cpustate->STATUS;                     break;
		case CPUINFO_INT_REGISTER + PIC16C62x_OPT:      info->i = cpustate->OPTION;                     break;
		case CPUINFO_INT_REGISTER + PIC16C62x_TMR0:     info->i = cpustate->TMR0;                       break;
		case CPUINFO_INT_REGISTER + PIC16C62x_WDT:      info->i = cpustate->WDT;                        break;
		case CPUINFO_INT_REGISTER + PIC16C62x_PSCL:     info->i = cpustate->prescaler;                  break;
		case CPUINFO_INT_REGISTER + PIC16C62x_PRTA:     info->i = cpustate->PORTA;                      break;
		case CPUINFO_INT_REGISTER + PIC16C62x_PRTB:     info->i = cpustate->PORTB;                      break;
		case CPUINFO_INT_REGISTER + PIC16C62x_FSR:      info->i = ((cpustate->FSR & cpustate->picRAMmask) | (UINT8)(~cpustate->picRAMmask));    break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:                      info->setinfo = CPU_SET_INFO_NAME(pic16c62x);   break;
		case CPUINFO_FCT_INIT:                          info->init = CPU_INIT_NAME(pic16c62x);          break;
		case CPUINFO_FCT_RESET:                         /* set per-CPU */                               break;
		case CPUINFO_FCT_EXIT:                          info->exit = CPU_EXIT_NAME(pic16c62x);          break;
		case CPUINFO_FCT_EXECUTE:                       info->execute = CPU_EXECUTE_NAME(pic16c62x);        break;
		case CPUINFO_FCT_BURN:                          info->burn = NULL;                              break;
		case CPUINFO_FCT_DISASSEMBLE:                   info->disassemble = CPU_DISASSEMBLE_NAME(pic16c62x);    break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:           info->icount = &cpustate->icount;               break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PIC16C62x");                   break;
		case CPUINFO_STR_FAMILY:                        strcpy(info->s, "Microchip");                   break;
		case CPUINFO_STR_VERSION:                       strcpy(info->s, "1.0");                     break;
		case CPUINFO_STR_SOURCE_FILE:                   strcpy(info->s, __FILE__);                      break;
		case CPUINFO_STR_CREDITS:                       strcpy(info->s, "Copyright Tony La Porta");     break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%01x%c%c%c%c%c %c%c%c%03x",
				(cpustate->STATUS & 0xe0) >> 5,         /* Register bank */
				cpustate->STATUS & 0x10 ? '.':'O',      /* WDT Overflow */
				cpustate->STATUS & 0x08 ? 'P':'D',      /* Power/Down */
				cpustate->STATUS & 0x04 ? 'Z':'.',      /* Zero */
				cpustate->STATUS & 0x02 ? 'c':'b',      /* Nibble Carry/Borrow */
				cpustate->STATUS & 0x01 ? 'C':'B',      /* Carry/Borrow */

				cpustate->OPTION & 0x20 ? 'C':'T',      /* Counter/Timer */
				cpustate->OPTION & 0x10 ? 'N':'P',      /* Negative/Positive */
				cpustate->OPTION & 0x08 ? 'W':'T',      /* WatchDog/Timer */
				cpustate->OPTION & 0x08 ? (1<<(cpustate->OPTION&7)) : (2<<(cpustate->OPTION&7)) );
			break;

		case CPUINFO_STR_REGISTER + PIC16C62x_PC:       sprintf(info->s, "PC:%03X",   cpustate->PC);                break;
		case CPUINFO_STR_REGISTER + PIC16C62x_W:            sprintf(info->s, "W:%02X",    cpustate->W);                 break;
		case CPUINFO_STR_REGISTER + PIC16C62x_ALU:      sprintf(info->s, "ALU:%02X",  cpustate->ALU);               break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STR:      sprintf(info->s, "STR:%02X",  cpustate->STATUS);            break;
		case CPUINFO_STR_REGISTER + PIC16C62x_TMR0:     sprintf(info->s, "TMR:%02X",  cpustate->TMR0);              break;
		case CPUINFO_STR_REGISTER + PIC16C62x_WDT:      sprintf(info->s, "WDT:%04X",  cpustate->WDT);               break;
		case CPUINFO_STR_REGISTER + PIC16C62x_OPT:      sprintf(info->s, "OPT:%02X",  cpustate->OPTION);            break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK0:     sprintf(info->s, "STK0:%03X", cpustate->STACK[0]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK1:     sprintf(info->s, "STK1:%03X", cpustate->STACK[1]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK2:     sprintf(info->s, "STK2:%03X", cpustate->STACK[2]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK3:     sprintf(info->s, "STK3:%03X", cpustate->STACK[3]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK4:     sprintf(info->s, "STK4:%03X", cpustate->STACK[4]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK5:     sprintf(info->s, "STK5:%03X", cpustate->STACK[5]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK6:     sprintf(info->s, "STK6:%03X", cpustate->STACK[6]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_STK7:     sprintf(info->s, "STK7:%03X", cpustate->STACK[7]);          break;
		case CPUINFO_STR_REGISTER + PIC16C62x_PRTA:     sprintf(info->s, "PRTA:%01X", ((cpustate->PORTA) & 0x1f));  break;
		case CPUINFO_STR_REGISTER + PIC16C62x_PRTB:     sprintf(info->s, "PRTB:%02X", cpustate->PORTB);             break;
		case CPUINFO_STR_REGISTER + PIC16C62x_TRSA:     sprintf(info->s, "TRSA:%01X", ((cpustate->TRISA) & 0x1f));  break;
		case CPUINFO_STR_REGISTER + PIC16C62x_TRSB:     sprintf(info->s, "TRSB:%02X", cpustate->TRISB);             break;
		case CPUINFO_STR_REGISTER + PIC16C62x_FSR:      sprintf(info->s, "FSR:%02X",  ((cpustate->FSR) & cpustate->picRAMmask) | (UINT8)(~cpustate->picRAMmask));   break;
		case CPUINFO_STR_REGISTER + PIC16C62x_PSCL:     sprintf(info->s, "PSCL:%c%02X", ((cpustate->OPTION & 0x08) ? 'W':'T'), cpustate->prescaler);    break;
	}
}


/****************************************************************************
 *  PIC16C620
 ****************************************************************************/
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c620_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c620_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x06) AM_RAM
	AM_RANGE(0x0a, 0x0c) AM_RAM
	AM_RANGE(0x1f, 0x6f) AM_RAM
	AM_RANGE(0x80, 0x86) AM_RAM
	AM_RANGE(0x8a, 0x8e) AM_RAM
	AM_RANGE(0x9f, 0x9f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C620 Reset
 ****************************************************************************/

static CPU_RESET( pic16c620 )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C620;
	cpustate->picRAMmask = 0xff;
	cpustate->reset_vector = 0x0;
	pic16c62x_reset_regs(cpustate);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c620 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:            info->i = 9;                            break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:           info->i = 8;                            break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(pic16c620);                    break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:  info->internal_map16 = ADDRESS_MAP_NAME(pic16c620_rom); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:     info->internal_map8 = ADDRESS_MAP_NAME(pic16c620_ram);  break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PIC16C620");           break;

		default:                                        CPU_GET_INFO_CALL(pic16c62x);           break;
	}
}


/****************************************************************************
 *  PIC16C621
 ****************************************************************************/
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c621_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c621_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x06) AM_RAM
	AM_RANGE(0x0a, 0x0c) AM_RAM
	AM_RANGE(0x1f, 0x6f) AM_RAM
	AM_RANGE(0x80, 0x86) AM_RAM
	AM_RANGE(0x8a, 0x8e) AM_RAM
	AM_RANGE(0x9f, 0x9f) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C621 Reset
 ****************************************************************************/

static CPU_RESET( pic16c621 )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C621;
	cpustate->picRAMmask = 0xff;
	cpustate->reset_vector = 0x0;
	pic16c62x_reset_regs(cpustate);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c621 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:            info->i = 10;                           break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:           info->i = 8;                            break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(pic16c621);                    break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:  info->internal_map16 = ADDRESS_MAP_NAME(pic16c621_rom); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:     info->internal_map8 = ADDRESS_MAP_NAME(pic16c621_ram);  break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PIC16C621");           break;

		default:                                        CPU_GET_INFO_CALL(pic16c62x);           break;
	}
}


/****************************************************************************
 *  PIC16C622
 ****************************************************************************/
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c622_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c622_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x06) AM_RAM
	AM_RANGE(0x0a, 0x0c) AM_RAM
	AM_RANGE(0x1f, 0x7f) AM_RAM
	AM_RANGE(0x80, 0x86) AM_RAM
	AM_RANGE(0x8a, 0x8e) AM_RAM
	AM_RANGE(0x9f, 0xbf) AM_RAM
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C622 Reset
 ****************************************************************************/

static CPU_RESET( pic16c622 )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C622;
	cpustate->picRAMmask = 0xff;
	cpustate->reset_vector = 0x0;
	pic16c62x_reset_regs(cpustate);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c622 )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:            info->i = 11;                           break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:           info->i = 8;                            break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(pic16c622);                    break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:  info->internal_map16 = ADDRESS_MAP_NAME(pic16c622_rom); break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:     info->internal_map8 = ADDRESS_MAP_NAME(pic16c622_ram);  break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PIC16C622");           break;

		default:                                        CPU_GET_INFO_CALL(pic16c62x);           break;
	}
}


/****************************************************************************
 *  PIC16C620A
 ****************************************************************************/
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c620a_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x1ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c620a_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x06) AM_RAM
	AM_RANGE(0x0a, 0x0c) AM_RAM
	AM_RANGE(0x1f, 0x6f) AM_RAM
	AM_RANGE(0x70, 0x7f) AM_RAM AM_SHARE(0)
	AM_RANGE(0x80, 0x86) AM_RAM
	AM_RANGE(0x8a, 0x8e) AM_RAM
	AM_RANGE(0x9f, 0x9f) AM_RAM
	AM_RANGE(0xf0, 0xff) AM_RAM AM_SHARE(0)
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C620A Reset
 ****************************************************************************/

static CPU_RESET( pic16c620a )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C620A;
	cpustate->picRAMmask = 0xff;
	cpustate->reset_vector = 0x0;
	pic16c62x_reset_regs(cpustate);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c620a )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:            info->i = 9;                            break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:           info->i = 8;                            break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(pic16c620a);                   break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:  info->internal_map16 = ADDRESS_MAP_NAME(pic16c620a_rom);    break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:     info->internal_map8 = ADDRESS_MAP_NAME(pic16c620a_ram); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PIC16C620A");          break;

		default:                                        CPU_GET_INFO_CALL(pic16c62x);           break;
	}
}


/****************************************************************************
 *  PIC16C621A
 ****************************************************************************/
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c621a_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x3ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c621a_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x06) AM_RAM
	AM_RANGE(0x0a, 0x0c) AM_RAM
	AM_RANGE(0x1f, 0x6f) AM_RAM
	AM_RANGE(0x70, 0x7f) AM_RAM AM_SHARE(0)
	AM_RANGE(0x80, 0x86) AM_RAM
	AM_RANGE(0x8a, 0x8e) AM_RAM
	AM_RANGE(0x9f, 0x9f) AM_RAM
	AM_RANGE(0xf0, 0xff) AM_RAM AM_SHARE(0)
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C621A Reset
 ****************************************************************************/

static CPU_RESET( pic16c621a )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C621A;
	cpustate->picRAMmask = 0xff;
	cpustate->reset_vector = 0x0;
	pic16c62x_reset_regs(cpustate);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c621a )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:            info->i = 10;                           break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:           info->i = 8;                            break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(pic16c621a);                   break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:  info->internal_map16 = ADDRESS_MAP_NAME(pic16c621a_rom);    break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:     info->internal_map8 = ADDRESS_MAP_NAME(pic16c621a_ram); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PIC16C621A");          break;

		default:                                        CPU_GET_INFO_CALL(pic16c62x);           break;
	}
}


/****************************************************************************
 *  PIC16C622A
 ****************************************************************************/
/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( pic16c622a_rom, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0x000, 0x7ff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( pic16c622a_ram, AS_DATA, 8, legacy_cpu_device )
	AM_RANGE(0x00, 0x06) AM_RAM
	AM_RANGE(0x0a, 0x0c) AM_RAM
	AM_RANGE(0x1f, 0x6f) AM_RAM
	AM_RANGE(0x70, 0x7f) AM_RAM AM_SHARE(0)
	AM_RANGE(0x80, 0x86) AM_RAM
	AM_RANGE(0x8a, 0x8e) AM_RAM
	AM_RANGE(0x9f, 0xbf) AM_RAM
	AM_RANGE(0xf0, 0xff) AM_RAM AM_SHARE(0)
ADDRESS_MAP_END


/****************************************************************************
 *  PIC16C622A Reset
 ****************************************************************************/

static CPU_RESET( pic16c622a )
{
	pic16c62x_state *cpustate = get_safe_token(device);

	update_internalram_ptr(cpustate);

	cpustate->picmodel = 0x16C622A;
	cpustate->picRAMmask = 0xff;
	cpustate->reset_vector = 0x0;
	pic16c62x_reset_regs(cpustate);
	SET(cpustate->STATUS, (TO_FLAG | PD_FLAG));
}


/**************************************************************************
 *  CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( pic16c622a )
{
	switch (state)
	{
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM:            info->i = 11;                           break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:           info->i = 8;                            break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:                         info->reset = CPU_RESET_NAME(pic16c622a);                   break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM:  info->internal_map16 = ADDRESS_MAP_NAME(pic16c622a_rom);    break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:     info->internal_map8 = ADDRESS_MAP_NAME(pic16c622a_ram); break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:                          strcpy(info->s, "PIC16C622A");          break;

		default:                                        CPU_GET_INFO_CALL(pic16c62x);           break;
	}
}


DEFINE_LEGACY_CPU_DEVICE(PIC16C620, pic16c620);
DEFINE_LEGACY_CPU_DEVICE(PIC16C620A, pic16c620a);
//DEFINE_LEGACY_CPU_DEVICE(PIC16CR620A, pic16cr620a);
DEFINE_LEGACY_CPU_DEVICE(PIC16C621, pic16c621);
DEFINE_LEGACY_CPU_DEVICE(PIC16C621A, pic16c621a);
DEFINE_LEGACY_CPU_DEVICE(PIC16C622, pic16c622);
DEFINE_LEGACY_CPU_DEVICE(PIC16C622A, pic16c622a);
