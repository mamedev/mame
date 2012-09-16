 /**************************************************************************\
 *                Texas Instruments TMS320x25 DSP Emulator                  *
 *                                                                          *
 *                 Copyright Tony La Porta                                  *
 *                      Written for the MAME project.                       *
 *                                                                          *
 *                                                                          *
 *  Three versions of the chip are available, and they are:                 *
 *  TMS320C25   Internal ROM one time programmed at TI                      *
 *  TMS320E25   Internal ROM programmable as a normal EPROM                 *
 *  TMS320P25   Internal ROM programmable once as a normal EPROM only       *
 *  These devices can also be used as a MicroController with external ROM   *
 *                                                                          *
 *                                                                          *
 *      Notes : The term 'DMA' within this document, is in reference        *
 *                  to Direct Memory Addressing, and NOT the usual term     *
 *                  of Direct Memory Access.                                *
 *              This is a word based microcontroller, with addressing       *
 *                  architecture based on the Harvard addressing scheme.    *
 *                                                                          *
 *                                                                          *
 *                                                                          *
 *  **** Change Log ****                                                    *
 *                                                                          *
 *  TLP (2x-May-2001)                                                       *
 *   - Work began on this emulator                                          *
 *  TLP (12-Jul-2001)                                                       *
 *   - First private release                                                *
 *  TLP (xx-Dec-2001) Ver 0.11                                              *
 *   - Various undocumented fixes                                           *
 *  TLP (13-Jul-2002) Ver 0.12                                              *
 *   - Corrected IRQ2 vector pointer                                        *
 *   - Fixed the signedness in many equation based instructions             *
 *   - Adjusted the level sensing for the Signal inputs                     *
 *   - Added the ability to view the CPU in the debugger when it's halted   *
 *  TLP (16-Nov-2002)                                                       *
 *   - First public release after nearly 1.5 years!                         *
 *   - Adjusted more signedness instructions (ADDH, SUBC, SUBH, etc)        *
 *  TLP (21-Dec-2002)                                                       *
 *   - Added memory banking for the CNFD, CNFP and CONF instructions        *
 *   - Corrected IRQ masking checks                                         *
 *  TLP (25-Dec-2002) Ver 1.10                                              *
 *   - Added internal timer                                                 *
 *                                                                          *
 \**************************************************************************/

/*****************************************************************************
 To fix, or currently lacking from this emulator are:

 Fix the levels for S_IN and S_OUT - use assert/release line

 #  Support for the built in Timer/Counter Page 91
    When idling, Counter must still be activly counting down. When counter
    reaches 0 it should issue a TINT (if it's not masked), then come out of
    IDLE mode.
    If TINT is masked, the Timer still needs to count down.

 #  Support for the built in Serial Port
 #  Support for the Global memory register
 #  Support for the switch for RAM block 0 banking between RAM and ROM space
 #  Correct the multi-cycle instruction cycle counts
 #  Add support to set ROM & RAM as Internal/External in order to correctly
    compute cycle timings
 #  Check (read) Hold signal level during execution loop ?
 #  Fix bugs
 #  Fix more bugs :-)
 #  Add/fix other things I forgot
*****************************************************************************/

/*
     TMS32025 CONF Mode Decoding Table
|=======================================|
| Status bit |           Blocks         |
|     CNF    |   B0    |   B1    |  B2  |
|------------+---------+---------+------|
|     0  0   |  data   |  data   | data |
|     1  1   | program |  data   | data |
|=======================================|


     TMS32026 CONF Mode Decoding Table
|==================================================|
| Status bits |               Blocks               |
| CNF1 | CNF0 |   B0    |   B1    |  B2  |   B3    |
|------+------+---------+---------+------+---------|
|  0   |  0   |  data   |  data   | data |  data   |
|  0   |  1   | program |  data   | data |  data   |
|  1   |  0   | program | program | data |  data   |
|  1   |  1   | program | program | data | program |
|==================================================|



Table 3-2.  TMS32025/26 Memory Blocks
|=========================================================|
|             Configured As Data Memory                   |
|-------+-------TMS320C25--------+-------TMS320C26--------|
|       |         | Hexadecimal  |         | Hexadecimal  |
| Block |  Pages  |   Address    |  Pages  |   Address    |
|-------+---------+--------------+---------+--------------|
|   B2  |    0    | 0060h-007Fh  |    0    | 0060h-007Fh  |
|   B0  |   4-5   | 0200h-02FFh  |   4-7   | 0200h-03FFh  |
|   B1  |   6-7   | 0300h-03FFh  |   8-11  | 0400h-05FFh  |
|   B3  |   B3 does not exist    |  12-15  | 0600h-07FFh  |
|=========================================================|
|             Configured As Program Memory                |
|-------+-------TMS320C25--------+-------TMS320C26--------|
|       |         | Hexadecimal  |         | Hexadecimal  |
| Block |  Pages  |   Address    |  Pages  |   Address    |
|-------+---------+--------------+---------+--------------|
|   B2  | B2 is not configurable | B2 is not configurable |
|   B0  | 510-511 | FF00h-FFFFh  | 500-503 | FA00h-FBFFh  |
|   B1  | B1 is not configurable | 504-507 | FC00h-FDFFh  |
|   B3  | B3 does not exist      | 508-511 | FE00h-FFFFh  |
|=========================================================|
*/


#include "emu.h"
#include "debugger.h"
#include "tms32025.h"


#define CLK 4	/* 1 cycle equals 4 clock ticks */		/* PE/DI */



#define SET_PC(x)	do { cpustate->PC = (x); } while (0)

#define P_IN(A)			(cpustate->io->read_word((A)<<1))
#define P_OUT(A,V)		(cpustate->io->write_word(((A)<<1),(V)))
#define S_IN(A)			(cpustate->io->read_word((A)<<1))
#define S_OUT(A,V)		(cpustate->io->write_word(((A)<<1),(V)))

#define M_RDOP(A)		((cpustate->pgmmap[(A) >> 7]) ? (cpustate->pgmmap[(A) >> 7][(A) & 0x7f]) : cpustate->direct->read_decrypted_word((A)<<1))
#define M_RDOP_ARG(A)	((cpustate->pgmmap[(A) >> 7]) ? (cpustate->pgmmap[(A) >> 7][(A) & 0x7f]) : cpustate->direct->read_decrypted_word((A)<<1))



struct tms32025_state
{
	/******************** CPU Internal Registers *******************/
	UINT16	PREVPC;		/* previous program counter */
	UINT16	PC;
	UINT16	PFC;
	UINT16	STR0, STR1;
	UINT8	IFR;
	UINT8	RPTC;
	PAIR	ACC;
	PAIR	Preg;
	UINT16	Treg;
	UINT16	AR[8];
	UINT16	STACK[8];
	PAIR	ALU;
	UINT16	*intRAM;
	UINT8	timerover;

	/********************** Status data ****************************/
	PAIR	opcode;
	int		idle;
	int		hold;
	int		external_mem_access;	/** required for hold mode. Implement it ! */
	int		init_load_addr;			/* 0=No, 1=Yes, 2=Once for repeat mode */
	int		tms32025_irq_cycles;
	int		tms32025_dec_cycles;
	device_irq_acknowledge_callback irq_callback;

	PAIR	oldacc;
	UINT32	memaccess;
	int		icount;
	int		mHackIgnoreARP;			 /* special handling for lst, lst1 instructions */
	int		waiting_for_serial_frame;

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;
	address_space *io;

	UINT16 *pgmmap[0x200];
	UINT16 *datamap[0x200];
};

INLINE tms32025_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS32025 ||
		   device->type() == TMS32026);
	return (tms32025_state *)downcast<legacy_cpu_device *>(device)->token();
}

/* opcode table entry */
struct tms32025_opcode
{
	UINT8	cycles;
	void	(*function)(tms32025_state *);
};
/* opcode table entry (Opcode CE has sub-opcodes) */
struct tms32025_opcode_CE
{
	UINT8	cycles;
	void	(*function)(tms32025_state *);
};
/* opcode table entry (Opcode Dx has sub-opcodes) */
struct tms32025_opcode_Dx
{
	UINT8	cycles;
	void	(*function)(tms32025_state *);
};



/************************** Memory mapped registers ****************/
#define DRR 	cpustate->intRAM[0]
#define DXR 	cpustate->intRAM[1]
#define TIM 	cpustate->intRAM[2]
#define PRD 	cpustate->intRAM[3]
#define IMR 	cpustate->intRAM[4]
#define GREG	cpustate->intRAM[5]



/****************************************************************************
 *******  The following is the Status (Flag) register 0 definition.  ********
| 15 | 14 | 13 | 12 |  11 | 10 |   9  | 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
| <----ARP---> | OV | OVM |  1 | INTM | <--------------DP---------------> | */

#define ARP_REG		0xe000	/* ARP  (Auxiliary Register Pointer) */
#define OV_FLAG		0x1000	/* OV   (Overflow flag) 1 indicates an overflow */
#define OVM_FLAG	0x0800	/* OVM  (Overflow Mode bit) 1 forces ACC overflow to greatest positive or negative saturation value */
#define INTM_FLAG	0x0200	/* INTM (Interrupt Mask flag) 0 enables maskable interrupts */
#define DP_REG		0x01ff	/* DP   (Data bank memory Pointer) */


/***********************************************************************************
 *** The following is the Status (Flag) register 1 definition for TMS32025. ********
| 15 | 14 | 13 |  12  | 11 |  10 | 9 | 8 | 7 |  6 |  5  |  4 |  3 |  2  | 1 | 0  |
| <----ARB---> | CNF0 | TC | SXM | C | 1 | 1 | HM | FSM | XF | FO | TXM | <-PM-> | */

/*** The following is the Status (Flag) register 1 definition for TMS32026. ***********
| 15 | 14 | 13 |  12  | 11 |  10 | 9 | 8 |   7  |  6 |  5  |  4 |  3 |  2  | 1 | 0  |
| <----ARB---> | CNF0 | TC | SXM | C | 1 | CNF1 | HM | FSM | XF | FO | TXM | <-PM-> | */

#define ARB_REG		0xe000	/* ARB  (Auxiliary Register pointer Backup) */
#define CNF0_REG	0x1000	/* CNF0 (Onchip RAM CoNFiguration) 0 means B0=data memory, 1means B0=program memory */
#define CNF1_REG	0x0080	/* CNF1 (Onchip RAM CoNFiguration) 0 means B0=data memory, 1means B0=program memory */
#define TC_FLAG		0x0800	/* TC   (Test Control flag) */
#define SXM_FLAG	0x0400	/* SXM  (Sign eXtension Mode) */
#define C_FLAG		0x0200	/* C    (Carry flag) */
#define HM_FLAG		0x0040	/* HM   (Processor Hold Mode) */
#define FSM_FLAG	0x0020	/* FSM  (Frame Synchronization Mode - for serial port) */
#define XF_FLAG		0x0010	/* XF   (XF output pin status) */
#define FO_FLAG		0x0008	/* FO   (Serial port Format In/Out mode) */
#define TXM_FLAG	0x0004	/* TXM  (Transmit Mode - for serial port) */
#define PM_REG		0x0003	/* PM   (Product shift Mode) */


#define OV		( cpustate->STR0 & OV_FLAG)			/* OV   (Overflow flag) */
#define OVM		( cpustate->STR0 & OVM_FLAG)		/* OVM  (Overflow Mode bit) 1 indicates an overflow */
#define INTM	( cpustate->STR0 & INTM_FLAG)		/* INTM (Interrupt enable flag) 0 enables maskable interrupts */
#define ARP		((cpustate->STR0 & ARP_REG) >> 13)	/* ARP  (Auxiliary Register Pointer) */
#define DP		((cpustate->STR0 & DP_REG) << 7)	/* DP   (Data memory Pointer bit) */
#define ARB		( cpustate->STR1 & ARB_REG)			/* ARB  (Backup Auxiliary Register pointer) */
#define CNF0	( cpustate->STR1 & CNF0_REG)		/* CNF0 (Onchip Ram Config register) */
#define TC		( cpustate->STR1 & TC_FLAG)			/* TC   (Test Control Flag) */
#define SXM		( cpustate->STR1 & SXM_FLAG)		/* SXM  (Sign Extension Mode) */
#define CARRY	( cpustate->STR1 & C_FLAG)			/* C    (Carry Flag for accumulator) */
#define HM		( cpustate->STR1 & HM_FLAG)			/* HM   (Processor Hold Mode) */
#define FSM		( cpustate->STR1 & FSM_FLAG)		/* FSM  (Frame Synchronization Mode - for serial port) */
#define XF		( cpustate->STR1 & FSM_FLAG)		/* XF   (XF output pin status) */
#define FO		( cpustate->STR1 & FO_FLAG)			/* FO   (Serial port Format In/Out mode) */
#define TXM		( cpustate->STR1 & TXM_FLAG)		/* TXM  (Transmit Mode - for serial port) */
#define PM		( cpustate->STR1 & PM_REG)			/* PM   (P register shift Mode. See SHIFT_Preg_TO_ALU below )*/

#define DMA		(DP | (cpustate->opcode.b.l & 0x7f))	/* address used in direct memory access operations */
#define DMApg0	(cpustate->opcode.b.l & 0x7f)			/* address used in direct memory access operations for sst instruction */
#define IND		cpustate->AR[ARP]						/* address used in indirect memory access operations */

INLINE void CLR0(tms32025_state *cpustate, UINT16 flag) { cpustate->STR0 &= ~flag; cpustate->STR0 |= 0x0400; }
INLINE void SET0(tms32025_state *cpustate, UINT16 flag) { cpustate->STR0 |=  flag; cpustate->STR0 |= 0x0400; }
INLINE void CLR1(tms32025_state *cpustate, UINT16 flag) { cpustate->STR1 &= ~flag; cpustate->STR1 |= 0x0180; }
INLINE void SET1(tms32025_state *cpustate, UINT16 flag) { cpustate->STR1 |=  flag; cpustate->STR1 |= 0x0180; }

INLINE void MODIFY_DP(tms32025_state *cpustate, int data)
{
	cpustate->STR0 &= ~DP_REG;
	cpustate->STR0 |= (data & DP_REG);
	cpustate->STR0 |= 0x0400;
}
INLINE void MODIFY_PM(tms32025_state *cpustate, int data)
{
	cpustate->STR1 &= ~PM_REG;
	cpustate->STR1 |= (data & PM_REG);
	cpustate->STR1 |= 0x0180;
}
INLINE void MODIFY_ARP(tms32025_state *cpustate, int data)
{
	cpustate->STR1 &= ~ARB_REG;
	cpustate->STR1 |= (cpustate->STR0 & ARP_REG);
	cpustate->STR1 |= 0x0180;
	cpustate->STR0 &= ~ARP_REG;
	cpustate->STR0 |= ((data << 13) & ARP_REG);
	cpustate->STR0 |= 0x0400;
}

INLINE UINT16 M_RDROM(tms32025_state *cpustate, offs_t addr)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = cpustate->pgmmap[addr >> 7];
	if (ram) return ram[addr & 0x7f];
	return cpustate->program->read_word(addr << 1);
}

INLINE void M_WRTROM(tms32025_state *cpustate, offs_t addr, UINT16 data)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = cpustate->pgmmap[addr >> 7];
	if (ram) { ram[addr & 0x7f] = data; }
	else cpustate->program->write_word(addr << 1, data);
}

INLINE UINT16 M_RDRAM(tms32025_state *cpustate, offs_t addr)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = cpustate->datamap[addr >> 7];
	if (ram) return ram[addr & 0x7f];
	return cpustate->data->read_word(addr << 1);
}

INLINE void M_WRTRAM(tms32025_state *cpustate, offs_t addr, UINT16 data)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = cpustate->datamap[addr >> 7];
	if (ram) {
		ram[addr & 0x7f] = data;
		if(addr == 1 && ram == cpustate->intRAM && TXM) {
			if(FSM)
				cpustate->waiting_for_serial_frame = 1;
			else
				cpustate->IFR |= 0x20;
		}
	}
	else cpustate->data->write_word(addr << 1, data);
}


static UINT16 reverse_carry_add(UINT16 arg0, UINT16 arg1 )
{
	UINT16 result = 0;
	int carry = 0;
	int count;
	for( count=0; count<16; count++ )
	{
		int sum = (arg0>>15)+(arg1>>15)+carry;
		result = (result<<1)|(sum&1);
		carry = sum>>1;
		arg0<<=1;
		arg1<<=1;
	}
	return result;
}

INLINE void MODIFY_AR_ARP(tms32025_state *cpustate)
{ /* modify address register referenced by ARP */
	switch (cpustate->opcode.b.l & 0x70)		/* Cases ordered by predicted useage */
	{
		case 0x00: /* 000   nop      */
			break;

		case 0x10: /* 001   *-       */
			cpustate->AR[ARP] -- ;
			break;

		case 0x20: /* 010   *+       */
			cpustate->AR[ARP] ++ ;
			break;

		case 0x30: /* 011   reserved */
			break;

		case 0x40: /* 100   *BR0-    */
			cpustate->AR[ARP] = reverse_carry_add(cpustate->AR[ARP],-cpustate->AR[0]);
			break;

		case 0x50: /* 101   *0-      */
			cpustate->AR[ARP] -= cpustate->AR[0];
			break;

		case 0x60: /* 110   *0+      */
			cpustate->AR[ARP] += cpustate->AR[0];
			break;

		case 0x70: /* 111   *BR0+    */
			cpustate->AR[ARP] += reverse_carry_add(cpustate->AR[ARP],cpustate->AR[0]);
			break;
	}

	if( !cpustate->mHackIgnoreARP )
	{
		if (cpustate->opcode.b.l & 8)
		{ /* bit 3 determines if new value is loaded into ARP */
			MODIFY_ARP(cpustate, (cpustate->opcode.b.l & 7) );
		}
	}
}

INLINE void CALCULATE_ADD_CARRY(tms32025_state *cpustate)
{
	if ( (UINT32)(cpustate->oldacc.d) > (UINT32)(cpustate->ACC.d) ) {
		SET1(cpustate, C_FLAG);
	}
	else {
		CLR1(cpustate, C_FLAG);
	}
}

INLINE void CALCULATE_SUB_CARRY(tms32025_state *cpustate)
{
	if ( (UINT32)(cpustate->oldacc.d) < (UINT32)(cpustate->ACC.d) ) {
		CLR1(cpustate, C_FLAG);
	}
	else {
		SET1(cpustate, C_FLAG);
	}
}

INLINE void CALCULATE_ADD_OVERFLOW(tms32025_state *cpustate, INT32 addval)
{
	if ((INT32)((cpustate->ACC.d ^ addval) & (cpustate->oldacc.d ^ cpustate->ACC.d)) < 0)
	{
		SET0(cpustate, OV_FLAG);
		if (OVM)
		{
			cpustate->ACC.d = ((INT32)cpustate->oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
		}
	}
}
INLINE void CALCULATE_SUB_OVERFLOW(tms32025_state *cpustate, INT32 subval)
{
	if ((INT32)((cpustate->oldacc.d ^ subval) & (cpustate->oldacc.d ^ cpustate->ACC.d)) < 0)
	{
		SET0(cpustate, OV_FLAG);
		if (OVM)
		{
			cpustate->ACC.d = ((INT32)cpustate->oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
		}
	}
}

INLINE UINT16 POP_STACK(tms32025_state *cpustate)
{
	UINT16 data = cpustate->STACK[7];
	cpustate->STACK[7] = cpustate->STACK[6];
	cpustate->STACK[6] = cpustate->STACK[5];
	cpustate->STACK[5] = cpustate->STACK[4];
	cpustate->STACK[4] = cpustate->STACK[3];
	cpustate->STACK[3] = cpustate->STACK[2];
	cpustate->STACK[2] = cpustate->STACK[1];
	cpustate->STACK[1] = cpustate->STACK[0];
	return data;
}
INLINE void PUSH_STACK(tms32025_state *cpustate, UINT16 data)
{
	cpustate->STACK[0] = cpustate->STACK[1];
	cpustate->STACK[1] = cpustate->STACK[2];
	cpustate->STACK[2] = cpustate->STACK[3];
	cpustate->STACK[3] = cpustate->STACK[4];
	cpustate->STACK[4] = cpustate->STACK[5];
	cpustate->STACK[5] = cpustate->STACK[6];
	cpustate->STACK[6] = cpustate->STACK[7];
	cpustate->STACK[7] = data;
}

INLINE void SHIFT_Preg_TO_ALU(tms32025_state *cpustate)
{
	switch(PM)		/* PM (in STR1) is the shift mode for Preg */
	{
		case 0:		cpustate->ALU.d = cpustate->Preg.d; break;
		case 1:		cpustate->ALU.d = (cpustate->Preg.d << 1); break;
		case 2:		cpustate->ALU.d = (cpustate->Preg.d << 4); break;
		case 3:		cpustate->ALU.d = (cpustate->Preg.d >> 6); if (cpustate->Preg.d & 0x80000000) cpustate->ALU.d |= 0xfc000000; break;
		default:	break;
	}
}

INLINE void GETDATA(tms32025_state *cpustate, int shift,int signext)
{
	if (cpustate->opcode.b.l & 0x80)
	{ /* indirect memory access */
		cpustate->memaccess = IND;
	}
	else
	{ /* direct memory address */
		cpustate->memaccess = DMA;
	}

	if (cpustate->memaccess >= 0x800)
	{
		cpustate->external_mem_access = 1;	/* Pause if hold pin is active */
	}
	else
	{
		cpustate->external_mem_access = 0;
	}

	cpustate->ALU.d = (UINT16)M_RDRAM(cpustate, cpustate->memaccess);
	if (signext) cpustate->ALU.d = (INT16)cpustate->ALU.d;
	cpustate->ALU.d <<= shift;

	/* next ARP */
	if (cpustate->opcode.b.l & 0x80) MODIFY_AR_ARP(cpustate);
}

INLINE void PUTDATA(tms32025_state *cpustate, UINT16 data)
{
	if (cpustate->opcode.b.l & 0x80) {
		if (cpustate->memaccess >= 0x800) cpustate->external_mem_access = 1;	/* Pause if hold pin is active */
		else cpustate->external_mem_access = 0;

		M_WRTRAM(cpustate, IND, data);
		MODIFY_AR_ARP(cpustate);
	}
	else {
		if (cpustate->memaccess >= 0x800) cpustate->external_mem_access = 1;	/* Pause if hold pin is active */
		else cpustate->external_mem_access = 0;

		M_WRTRAM(cpustate, DMA, data);
	}
}
INLINE void PUTDATA_SST(tms32025_state *cpustate, UINT16 data)
{
	if (cpustate->opcode.b.l & 0x80) cpustate->memaccess = IND;
	else cpustate->memaccess = DMApg0;

	if (cpustate->memaccess >= 0x800) cpustate->external_mem_access = 1;		/* Pause if hold pin is active */
	else cpustate->external_mem_access = 0;

	if (cpustate->opcode.b.l & 0x80) {
		cpustate->opcode.b.l &= 0xf7;					/* Stop ARP changes */
		MODIFY_AR_ARP(cpustate);
	}
	M_WRTRAM(cpustate, cpustate->memaccess, data);
}



/****************************************************************************
 *  Emulate the Instructions
 ****************************************************************************/

/* The following functions are here to fill the void for the */
/* opcode call functions. These functions are never actually called. */
static void opcodes_CE(tms32025_state *cpustate) { }
static void opcodes_Dx(tms32025_state *cpustate) { }

static void illegal(tms32025_state *cpustate)
{
	logerror("TMS32025:  PC = %04x,  Illegal opcode = %04x\n", (cpustate->PC-1), cpustate->opcode.w.l);
}

static void abst(tms32025_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) < 0 ) {
		cpustate->ACC.d = -cpustate->ACC.d;
		if (cpustate->ACC.d == 0x80000000) {
			SET0(cpustate, OV_FLAG);
			if (OVM) cpustate->ACC.d-- ;
		}
	}
	CLR1(cpustate, C_FLAG);
}
static void add(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, (cpustate->opcode.b.h & 0xf), SXM);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void addc(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	if (CARRY) cpustate->ACC.d++;
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	if (cpustate->ACC.d == cpustate->oldacc.d) {}	/* edge case, carry remains same */
	else CALCULATE_ADD_CARRY(cpustate);
}
static void addh(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.w.h += cpustate->ALU.w.l;
	if ( (UINT16)(cpustate->oldacc.w.h) > (UINT16)(cpustate->ACC.w.h) ) {
		SET1(cpustate, C_FLAG);	/* Carry flag is not cleared, if no carry occurred */
	}
	if ((INT16)((cpustate->ACC.w.h ^ cpustate->ALU.w.l) & (cpustate->oldacc.w.h ^ cpustate->ACC.w.h)) < 0) {
		SET0(cpustate, OV_FLAG);
		if (OVM) cpustate->ACC.w.h = ((INT16)cpustate->oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
static void addk(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	cpustate->ALU.d = (UINT8)cpustate->opcode.b.l;
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void adds(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void addt(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, (cpustate->Treg & 0xf), SXM);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void adlk(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	if (SXM) cpustate->ALU.d =  (INT16)M_RDOP_ARG(cpustate->PC);
	else     cpustate->ALU.d = (UINT16)M_RDOP_ARG(cpustate->PC);
	cpustate->PC++;
	cpustate->ALU.d <<= (cpustate->opcode.b.h & 0xf);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void adrk(tms32025_state *cpustate)
{
	cpustate->AR[ARP] += cpustate->opcode.b.l;
}
static void and_(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.d &= cpustate->ALU.d;
}
static void andk(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	cpustate->ALU.d = (UINT16)M_RDOP_ARG(cpustate->PC);
	cpustate->PC++;
	cpustate->ALU.d <<= (cpustate->opcode.b.h & 0xf);
	cpustate->ACC.d &= cpustate->ALU.d;
}
static void apac(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void br(tms32025_state *cpustate)
{
	SET_PC(M_RDOP_ARG(cpustate->PC));
	MODIFY_AR_ARP(cpustate);
}
static void bacc(tms32025_state *cpustate)
{
	SET_PC(cpustate->ACC.w.l);
}
static void banz(tms32025_state *cpustate)
{
	if (cpustate->AR[ARP]) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bbnz(tms32025_state *cpustate)
{
	if (TC) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bbz(tms32025_state *cpustate)
{
	if (TC == 0) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bc(tms32025_state *cpustate)
{
	if (CARRY) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bgez(tms32025_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) >= 0 ) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bgz(tms32025_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) > 0 ) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bioz(tms32025_state *cpustate)
{
	if (S_IN(TMS32025_BIO) != CLEAR_LINE) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bit(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	if (cpustate->ALU.d & (0x8000 >> (cpustate->opcode.b.h & 0xf))) SET1(cpustate, TC_FLAG);
	else CLR1(cpustate, TC_FLAG);
}
static void bitt(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	if (cpustate->ALU.d & (0x8000 >> (cpustate->Treg & 0xf))) SET1(cpustate, TC_FLAG);
	else CLR1(cpustate, TC_FLAG);
}
static void blez(tms32025_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) <= 0 ) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void blkd(tms32025_state *cpustate)
{										/** Fix cycle timing **/
	if (cpustate->init_load_addr) {
		cpustate->PFC = M_RDOP_ARG(cpustate->PC);
		cpustate->PC++;
	}
	cpustate->ALU.d = M_RDRAM(cpustate, cpustate->PFC);
	PUTDATA(cpustate, cpustate->ALU.d);
	cpustate->PFC++;
	cpustate->tms32025_dec_cycles += (1*CLK);
}
static void blkp(tms32025_state *cpustate)
{										/** Fix cycle timing **/
	if (cpustate->init_load_addr) {
		cpustate->PFC = M_RDOP_ARG(cpustate->PC);
		cpustate->PC++;
	}
	cpustate->ALU.d = M_RDROM(cpustate, cpustate->PFC);
	PUTDATA(cpustate, cpustate->ALU.d);
	cpustate->PFC++;
	cpustate->tms32025_dec_cycles += (2*CLK);
}
static void blz(tms32025_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) <  0 ) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bnc(tms32025_state *cpustate)
{
	if (CARRY == 0) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bnv(tms32025_state *cpustate)
{
	if (OV == 0) SET_PC(M_RDOP_ARG(cpustate->PC));
	else {
		cpustate->PC++ ;
		CLR0(cpustate, OV_FLAG);
	}
	MODIFY_AR_ARP(cpustate);
}
static void bnz(tms32025_state *cpustate)
{
	if (cpustate->ACC.d != 0) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bv(tms32025_state *cpustate)
{
	if (OV) {
		SET_PC(M_RDOP_ARG(cpustate->PC));
		CLR0(cpustate, OV_FLAG);
	}
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void bz(tms32025_state *cpustate)
{
	if (cpustate->ACC.d == 0) SET_PC(M_RDOP_ARG(cpustate->PC));
	else cpustate->PC++ ;
	MODIFY_AR_ARP(cpustate);
}
static void cala(tms32025_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->PC);
	SET_PC(cpustate->ACC.w.l);
}
static void call(tms32025_state *cpustate)
{
	cpustate->PC++ ;
	PUSH_STACK(cpustate, cpustate->PC);
	SET_PC(M_RDOP_ARG((cpustate->PC - 1)));
	MODIFY_AR_ARP(cpustate);
}
static void cmpl(tms32025_state *cpustate)
{
	cpustate->ACC.d = (~cpustate->ACC.d);
}
static void cmpr(tms32025_state *cpustate)
{
	switch (cpustate->opcode.b.l & 3)
	{
		case 00:	if ( (UINT16)(cpustate->AR[ARP]) == (UINT16)(cpustate->AR[0]) ) SET1(cpustate, TC_FLAG);
					else CLR1(cpustate, TC_FLAG);
					break;
		case 01:	if ( (UINT16)(cpustate->AR[ARP]) <  (UINT16)(cpustate->AR[0]) ) SET1(cpustate, TC_FLAG);
					else CLR1(cpustate, TC_FLAG);
					break;
		case 02:	if ( (UINT16)(cpustate->AR[ARP])  > (UINT16)(cpustate->AR[0]) ) SET1(cpustate, TC_FLAG);
					else CLR1(cpustate, TC_FLAG);
					break;
		case 03:	if ( (UINT16)(cpustate->AR[ARP]) != (UINT16)(cpustate->AR[0]) ) SET1(cpustate, TC_FLAG);
					else CLR1(cpustate, TC_FLAG);
					break;
	}
}
static void cnfd(tms32025_state *cpustate)	/** next two fetches need to use previous CNF value ! **/
{
	CLR1(cpustate, CNF0_REG);
	cpustate->datamap[4] = &cpustate->intRAM[0x200];			/* B0 */
	cpustate->datamap[5] = &cpustate->intRAM[0x280];			/* B0 */
	cpustate->pgmmap[510] = NULL;
	cpustate->pgmmap[511] = NULL;
}
static void cnfp(tms32025_state *cpustate)	/** next two fetches need to use previous CNF value ! **/
{
	SET1(cpustate, CNF0_REG);
	cpustate->datamap[4] = NULL;						/* B0 */
	cpustate->datamap[5] = NULL;						/* B0 */
	cpustate->pgmmap[510] = &cpustate->intRAM[0x200];
	cpustate->pgmmap[511] = &cpustate->intRAM[0x280];
}
static void conf(tms32025_state *cpustate)	/** Need to reconfigure the memory blocks */
{
	switch (cpustate->opcode.b.l & 3)
	{
		case 00:	CLR1(cpustate, CNF1_REG); CLR1(cpustate, CNF0_REG);
					cpustate->datamap[4] = &cpustate->intRAM[0x200];	/* B0 */
					cpustate->datamap[5] = &cpustate->intRAM[0x280];	/* B0 */
					cpustate->datamap[6] = &cpustate->intRAM[0x300];	/* B0 */
					cpustate->datamap[7] = &cpustate->intRAM[0x380];	/* B0 */
					cpustate->datamap[8] = &cpustate->intRAM[0x400];	/* B1 */
					cpustate->datamap[9] = &cpustate->intRAM[0x480];	/* B1 */
					cpustate->datamap[10] = &cpustate->intRAM[0x500];	/* B1 */
					cpustate->datamap[11] = &cpustate->intRAM[0x580];	/* B1 */
					cpustate->datamap[12] = &cpustate->intRAM[0x600];	/* B3 */
					cpustate->datamap[13] = &cpustate->intRAM[0x680];	/* B3 */
					cpustate->datamap[14] = &cpustate->intRAM[0x700];	/* B3 */
					cpustate->datamap[15] = &cpustate->intRAM[0x780];	/* B3 */
					cpustate->pgmmap[500] = NULL;
					cpustate->pgmmap[501] = NULL;
					cpustate->pgmmap[502] = NULL;
					cpustate->pgmmap[503] = NULL;
					cpustate->pgmmap[504] = NULL;
					cpustate->pgmmap[505] = NULL;
					cpustate->pgmmap[506] = NULL;
					cpustate->pgmmap[507] = NULL;
					cpustate->pgmmap[508] = NULL;
					cpustate->pgmmap[509] = NULL;
					cpustate->pgmmap[510] = NULL;
					cpustate->pgmmap[511] = NULL;
					break;

		case 01:	CLR1(cpustate, CNF1_REG); SET1(cpustate, CNF0_REG);
					cpustate->datamap[4] = NULL;
					cpustate->datamap[5] = NULL;
					cpustate->datamap[6] = NULL;
					cpustate->datamap[7] = NULL;
					cpustate->datamap[8] = &cpustate->intRAM[0x400];	/* B1 */
					cpustate->datamap[9] = &cpustate->intRAM[0x480];	/* B1 */
					cpustate->datamap[10] = &cpustate->intRAM[0x500];	/* B1 */
					cpustate->datamap[11] = &cpustate->intRAM[0x580];	/* B1 */
					cpustate->datamap[12] = &cpustate->intRAM[0x600];	/* B3 */
					cpustate->datamap[13] = &cpustate->intRAM[0x680];	/* B3 */
					cpustate->datamap[14] = &cpustate->intRAM[0x700];	/* B3 */
					cpustate->datamap[15] = &cpustate->intRAM[0x780];	/* B3 */
					cpustate->pgmmap[500] = &cpustate->intRAM[0x200];	/* B0 */
					cpustate->pgmmap[501] = &cpustate->intRAM[0x280];	/* B0 */
					cpustate->pgmmap[502] = &cpustate->intRAM[0x300];	/* B0 */
					cpustate->pgmmap[503] = &cpustate->intRAM[0x380];	/* B0 */
					cpustate->pgmmap[504] = NULL;
					cpustate->pgmmap[505] = NULL;
					cpustate->pgmmap[506] = NULL;
					cpustate->pgmmap[507] = NULL;
					cpustate->pgmmap[508] = NULL;
					cpustate->pgmmap[509] = NULL;
					cpustate->pgmmap[510] = NULL;
					cpustate->pgmmap[511] = NULL;
					break;

		case 02:	SET1(cpustate, CNF1_REG); CLR1(cpustate, CNF0_REG);
					cpustate->datamap[4] = NULL;
					cpustate->datamap[5] = NULL;
					cpustate->datamap[6] = NULL;
					cpustate->datamap[7] = NULL;
					cpustate->datamap[8] = NULL;
					cpustate->datamap[9] = NULL;
					cpustate->datamap[10] = NULL;
					cpustate->datamap[11] = NULL;
					cpustate->datamap[12] = &cpustate->intRAM[0x600];	/* B3 */
					cpustate->datamap[13] = &cpustate->intRAM[0x680];	/* B3 */
					cpustate->datamap[14] = &cpustate->intRAM[0x700];	/* B3 */
					cpustate->datamap[15] = &cpustate->intRAM[0x780];	/* B3 */
					cpustate->pgmmap[500] = &cpustate->intRAM[0x200];	/* B0 */
					cpustate->pgmmap[501] = &cpustate->intRAM[0x280];	/* B0 */
					cpustate->pgmmap[502] = &cpustate->intRAM[0x300];	/* B0 */
					cpustate->pgmmap[503] = &cpustate->intRAM[0x380];	/* B0 */
					cpustate->pgmmap[504] = &cpustate->intRAM[0x400];	/* B1 */
					cpustate->pgmmap[505] = &cpustate->intRAM[0x480];	/* B1 */
					cpustate->pgmmap[506] = &cpustate->intRAM[0x500];	/* B1 */
					cpustate->pgmmap[507] = &cpustate->intRAM[0x580];	/* B1 */
					cpustate->pgmmap[508] = NULL;
					cpustate->pgmmap[509] = NULL;
					cpustate->pgmmap[510] = NULL;
					cpustate->pgmmap[511] = NULL;
					break;

		case 03:	SET1(cpustate, CNF1_REG); SET1(cpustate, CNF0_REG);
					cpustate->datamap[4] = NULL;
					cpustate->datamap[5] = NULL;
					cpustate->datamap[6] = NULL;
					cpustate->datamap[7] = NULL;
					cpustate->datamap[8] = NULL;
					cpustate->datamap[9] = NULL;
					cpustate->datamap[10] = NULL;
					cpustate->datamap[11] = NULL;
					cpustate->datamap[12] = NULL;
					cpustate->datamap[13] = NULL;
					cpustate->datamap[14] = NULL;
					cpustate->datamap[15] = NULL;
					cpustate->pgmmap[500] = &cpustate->intRAM[0x200];	/* B0 */
					cpustate->pgmmap[501] = &cpustate->intRAM[0x280];	/* B0 */
					cpustate->pgmmap[502] = &cpustate->intRAM[0x300];	/* B0 */
					cpustate->pgmmap[503] = &cpustate->intRAM[0x380];	/* B0 */
					cpustate->pgmmap[504] = &cpustate->intRAM[0x400];	/* B1 */
					cpustate->pgmmap[505] = &cpustate->intRAM[0x480];	/* B1 */
					cpustate->pgmmap[506] = &cpustate->intRAM[0x500];	/* B1 */
					cpustate->pgmmap[507] = &cpustate->intRAM[0x580];	/* B1 */
					cpustate->pgmmap[508] = &cpustate->intRAM[0x600];	/* B3 */
					cpustate->pgmmap[509] = &cpustate->intRAM[0x680];	/* B3 */
					cpustate->pgmmap[510] = &cpustate->intRAM[0x700];	/* B3 */
					cpustate->pgmmap[511] = &cpustate->intRAM[0x780];	/* B3 */
					break;
	}
}
static void dint(tms32025_state *cpustate)
{
	SET0(cpustate, INTM_FLAG);
}
static void dmov(tms32025_state *cpustate)	/** Careful with how memory is configured !! */
{
	GETDATA(cpustate, 0, 0);
	M_WRTRAM(cpustate, (cpustate->memaccess + 1), cpustate->ALU.w.l);
}
static void eint(tms32025_state *cpustate)
{
	CLR0(cpustate, INTM_FLAG);
}
static void fort(tms32025_state *cpustate)
{
	if (cpustate->opcode.b.l & 1) SET1(cpustate, FO_FLAG);
	else CLR1(cpustate, FO_FLAG);
}
static void idle(tms32025_state *cpustate)
{
	CLR0(cpustate, INTM_FLAG);
	cpustate->idle = 1;
}
static void in(tms32025_state *cpustate)
{
	cpustate->ALU.w.l = P_IN( (cpustate->opcode.b.h & 0xf) );
	PUTDATA(cpustate, cpustate->ALU.w.l);
}
static void lac(tms32025_state *cpustate)
{
	GETDATA(cpustate, (cpustate->opcode.b.h & 0xf), SXM);
	cpustate->ACC.d = cpustate->ALU.d;
}
static void lack(tms32025_state *cpustate)		/* ZAC is a subset of this instruction */
{
	cpustate->ACC.d = (UINT8)cpustate->opcode.b.l;
}
static void lact(tms32025_state *cpustate)
{
	GETDATA(cpustate, (cpustate->Treg & 0xf), SXM);
	cpustate->ACC.d = cpustate->ALU.d;
}
static void lalk(tms32025_state *cpustate)
{
	if (SXM) cpustate->ALU.d =  (INT16)M_RDOP_ARG(cpustate->PC);
	else     cpustate->ALU.d = (UINT16)M_RDOP_ARG(cpustate->PC);
	cpustate->PC++;
	cpustate->ALU.d <<= (cpustate->opcode.b.h & 0xf);
	cpustate->ACC.d = cpustate->ALU.d;
}
static void lar_ar0(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[0] = cpustate->ALU.w.l; }
static void lar_ar1(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[1] = cpustate->ALU.w.l; }
static void lar_ar2(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[2] = cpustate->ALU.w.l; }
static void lar_ar3(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[3] = cpustate->ALU.w.l; }
static void lar_ar4(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[4] = cpustate->ALU.w.l; }
static void lar_ar5(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[5] = cpustate->ALU.w.l; }
static void lar_ar6(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[6] = cpustate->ALU.w.l; }
static void lar_ar7(tms32025_state *cpustate)	{ GETDATA(cpustate, 0, 0); cpustate->AR[7] = cpustate->ALU.w.l; }
static void lark_ar0(tms32025_state *cpustate)	{ cpustate->AR[0] = cpustate->opcode.b.l; }
static void lark_ar1(tms32025_state *cpustate)	{ cpustate->AR[1] = cpustate->opcode.b.l; }
static void lark_ar2(tms32025_state *cpustate)	{ cpustate->AR[2] = cpustate->opcode.b.l; }
static void lark_ar3(tms32025_state *cpustate)	{ cpustate->AR[3] = cpustate->opcode.b.l; }
static void lark_ar4(tms32025_state *cpustate)	{ cpustate->AR[4] = cpustate->opcode.b.l; }
static void lark_ar5(tms32025_state *cpustate)	{ cpustate->AR[5] = cpustate->opcode.b.l; }
static void lark_ar6(tms32025_state *cpustate)	{ cpustate->AR[6] = cpustate->opcode.b.l; }
static void lark_ar7(tms32025_state *cpustate)	{ cpustate->AR[7] = cpustate->opcode.b.l; }
static void ldp(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	MODIFY_DP(cpustate, cpustate->ALU.d & 0x1ff);
}
static void ldpk(tms32025_state *cpustate)
{
	MODIFY_DP(cpustate, cpustate->opcode.w.l & 0x1ff);
}
static void lph(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->Preg.w.h = cpustate->ALU.w.l;
}
static void lrlk(tms32025_state *cpustate)
{
	cpustate->ALU.d = (UINT16)M_RDOP_ARG(cpustate->PC);
	cpustate->PC++;
	cpustate->AR[cpustate->opcode.b.h & 7] = cpustate->ALU.w.l;
}
static void lst(tms32025_state *cpustate)
{
	cpustate->mHackIgnoreARP = 1;
	GETDATA(cpustate, 0, 0);
	cpustate->mHackIgnoreARP = 0;

	cpustate->ALU.w.l &= (~INTM_FLAG);
	cpustate->STR0 &= INTM_FLAG;
	cpustate->STR0 |= cpustate->ALU.w.l;		/* Must not affect INTM */
	cpustate->STR0 |= 0x0400;
}
static void lst1(tms32025_state *cpustate)
{
	cpustate->mHackIgnoreARP = 1;
	GETDATA(cpustate, 0, 0);
	cpustate->mHackIgnoreARP = 0;

	cpustate->STR1 = cpustate->ALU.w.l;
	cpustate->STR1 |= 0x0180;
	cpustate->STR0 &= (~ARP_REG);		/* ARB also gets copied to ARP */
	cpustate->STR0 |= (cpustate->STR1 & ARB_REG);
}
static void lt(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
}
static void lta(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void ltd(tms32025_state *cpustate)	/** Careful with how memory is configured !! */
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
	M_WRTRAM(cpustate, (cpustate->memaccess+1), cpustate->ALU.w.l);
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
}
static void ltp(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d = cpustate->ALU.d;
}
static void lts(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
}
static void mac(tms32025_state *cpustate)			/** RAM blocks B0,B1,B2 may be important ! */
{								/** Fix cycle timing **/
	cpustate->oldacc.d = cpustate->ACC.d;
	if (cpustate->init_load_addr) {
		cpustate->PFC = M_RDOP_ARG(cpustate->PC);
		cpustate->PC++;
	}
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
	cpustate->Preg.d = ( (INT16)cpustate->ALU.w.l * (INT16)M_RDROM(cpustate, cpustate->PFC) );
	cpustate->PFC++;
	cpustate->tms32025_dec_cycles += (2*CLK);
}
static void macd(tms32025_state *cpustate)			/** RAM blocks B0,B1,B2 may be important ! */
{													/** Fix cycle timing **/
	cpustate->oldacc.d = cpustate->ACC.d;
	if (cpustate->init_load_addr) {
		cpustate->PFC = M_RDOP_ARG(cpustate->PC);
		cpustate->PC++;
	}
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
	GETDATA(cpustate, 0, 0);
	if ( (cpustate->opcode.b.l & 0x80) || cpustate->init_load_addr ) {	/* No writing during repitition, or DMA mode */
		M_WRTRAM(cpustate, (cpustate->memaccess+1), cpustate->ALU.w.l);
	}
	cpustate->Treg = cpustate->ALU.w.l;
	cpustate->Preg.d = ( (INT16)cpustate->ALU.w.l * (INT16)M_RDROM(cpustate, cpustate->PFC) );
	cpustate->PFC++;
	cpustate->tms32025_dec_cycles += (2*CLK);
}
static void mar(tms32025_state *cpustate)		/* LARP and NOP are a subset of this instruction */
{
	if (cpustate->opcode.b.l & 0x80) MODIFY_AR_ARP(cpustate);
}
static void mpy(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->Preg.d = (INT16)(cpustate->ALU.w.l) * (INT16)(cpustate->Treg);
}
static void mpya(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
	GETDATA(cpustate, 0, 0);
	cpustate->Preg.d = (INT16)(cpustate->ALU.w.l) * (INT16)(cpustate->Treg);
}
static void mpyk(tms32025_state *cpustate)
{
	cpustate->Preg.d = (INT16)cpustate->Treg * ((INT16)(cpustate->opcode.w.l << 3) >> 3);

}
static void mpys(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
	GETDATA(cpustate, 0, 0);
	cpustate->Preg.d = (INT16)(cpustate->ALU.w.l) * (INT16)(cpustate->Treg);
}
static void mpyu(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->Preg.d = (UINT16)(cpustate->ALU.w.l) * (UINT16)(cpustate->Treg);
}
static void neg(tms32025_state *cpustate)
{
	if (cpustate->ACC.d == 0x80000000) {
		SET0(cpustate, OV_FLAG);
		if (OVM) cpustate->ACC.d = 0x7fffffff;
	}
	else cpustate->ACC.d = -cpustate->ACC.d;
	if (cpustate->ACC.d) CLR0(cpustate, C_FLAG);
	else SET0(cpustate, C_FLAG);
}
/*
static void nop(tms32025_state *cpustate) { }   // NOP is a subset of the MAR instruction
*/
static void norm(tms32025_state *cpustate)
{
	if (cpustate->ACC.d !=0 && (INT32)(cpustate->ACC.d ^ (cpustate->ACC.d << 1)) >= 0)
	{
		CLR1(cpustate, TC_FLAG);
		cpustate->ACC.d <<= 1;
		MODIFY_AR_ARP(cpustate);
	}
	else SET1(cpustate, TC_FLAG);
}
static void or_(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.w.l |= cpustate->ALU.w.l;
}
static void ork(tms32025_state *cpustate)
{
	cpustate->ALU.d = (UINT16)M_RDOP_ARG(cpustate->PC);
	cpustate->PC++;
	cpustate->ALU.d <<= (cpustate->opcode.b.h & 0xf);
	cpustate->ACC.d |=  (cpustate->ALU.d);
}
static void out(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	P_OUT( (cpustate->opcode.b.h & 0xf), cpustate->ALU.w.l );
}
static void pac(tms32025_state *cpustate)
{
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d = cpustate->ALU.d;
}
static void pop(tms32025_state *cpustate)
{
	cpustate->ACC.d = (UINT16)POP_STACK(cpustate);
}
static void popd(tms32025_state *cpustate)
{
	cpustate->ALU.d = (UINT16)POP_STACK(cpustate);
	PUTDATA(cpustate, cpustate->ALU.w.l);
}
static void pshd(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	PUSH_STACK(cpustate, cpustate->ALU.w.l);
}
static void push(tms32025_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->ACC.w.l);
}
static void rc(tms32025_state *cpustate)
{
	CLR1(cpustate, C_FLAG);
}
static void ret(tms32025_state *cpustate)
{
	SET_PC(POP_STACK(cpustate));
}
static void rfsm(tms32025_state *cpustate)				/** serial port mode */
{
	CLR1(cpustate, FSM_FLAG);
}
static void rhm(tms32025_state *cpustate)
{
	CLR1(cpustate, HM_FLAG);
}
static void rol(tms32025_state *cpustate)
{
	cpustate->ALU.d = cpustate->ACC.d;
	cpustate->ACC.d <<= 1;
	if (CARRY) cpustate->ACC.d |= 1;
	if (cpustate->ALU.d & 0x80000000) SET1(cpustate, C_FLAG);
	else CLR1(cpustate, C_FLAG);
}
static void ror(tms32025_state *cpustate)
{
	cpustate->ALU.d = cpustate->ACC.d;
	cpustate->ACC.d >>= 1;
	if (CARRY) cpustate->ACC.d |= 0x80000000;
	if (cpustate->ALU.d & 1) SET1(cpustate, C_FLAG);
	else CLR1(cpustate, C_FLAG);
}
static void rovm(tms32025_state *cpustate)
{
	CLR0(cpustate, OVM_FLAG);
}
static void rpt(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->RPTC = cpustate->ALU.b.l;
	cpustate->init_load_addr = 2;		/* Initiate repeat mode */
}
static void rptk(tms32025_state *cpustate)
{
	cpustate->RPTC = cpustate->opcode.b.l;
	cpustate->init_load_addr = 2;		/* Initiate repeat mode */
}
static void rsxm(tms32025_state *cpustate)
{
	CLR1(cpustate, SXM_FLAG);
}
static void rtc(tms32025_state *cpustate)
{
	CLR1(cpustate, TC_FLAG);
}
static void rtxm(tms32025_state *cpustate)	/** Serial port stuff */
{
	CLR1(cpustate, TXM_FLAG);
}
static void rxf(tms32025_state *cpustate)
{
	CLR1(cpustate, XF_FLAG);
	S_OUT(TMS32025_XF,CLEAR_LINE);
}
static void sach(tms32025_state *cpustate)
{
	cpustate->ALU.d = (cpustate->ACC.d << (cpustate->opcode.b.h & 7));
	PUTDATA(cpustate, cpustate->ALU.w.h);
}
static void sacl(tms32025_state *cpustate)
{
	cpustate->ALU.d = (cpustate->ACC.d << (cpustate->opcode.b.h & 7));
	PUTDATA(cpustate, cpustate->ALU.w.l);
}
static void sar_ar0(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[0]); }
static void sar_ar1(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[1]); }
static void sar_ar2(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[2]); }
static void sar_ar3(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[3]); }
static void sar_ar4(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[4]); }
static void sar_ar5(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[5]); }
static void sar_ar6(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[6]); }
static void sar_ar7(tms32025_state *cpustate)	{ PUTDATA(cpustate, cpustate->AR[7]); }

static void sblk(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	if (SXM) cpustate->ALU.d =  (INT16)M_RDOP_ARG(cpustate->PC);
	else     cpustate->ALU.d = (UINT16)M_RDOP_ARG(cpustate->PC);
	cpustate->PC++;
	cpustate->ALU.d <<= (cpustate->opcode.b.h & 0xf);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
}
static void sbrk_ar(tms32025_state *cpustate)
{
	cpustate->AR[ARP] -= cpustate->opcode.b.l;
}
static void sc(tms32025_state *cpustate)
{
	SET1(cpustate, C_FLAG);
}
static void sfl(tms32025_state *cpustate)
{
	cpustate->ALU.d = cpustate->ACC.d;
	cpustate->ACC.d <<= 1;
	if (cpustate->ALU.d & 0x80000000) SET1(cpustate, C_FLAG);
	else CLR1(cpustate, C_FLAG);
}
static void sfr(tms32025_state *cpustate)
{
	cpustate->ALU.d = cpustate->ACC.d;
	cpustate->ACC.d >>= 1;
	if (SXM) {
		if (cpustate->ALU.d & 0x80000000) cpustate->ACC.d |= 0x80000000;
	}
	if (cpustate->ALU.d & 1) SET1(cpustate, C_FLAG);
	else CLR1(cpustate, C_FLAG);
}
static void sfsm(tms32025_state *cpustate)	/** Serial port mode */
{
	SET1(cpustate, FSM_FLAG);
}
static void shm(tms32025_state *cpustate)
{
	SET1(cpustate, HM_FLAG);
}
static void sovm(tms32025_state *cpustate)
{
	SET0(cpustate, OVM_FLAG);
}
static void spac(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
}
static void sph(tms32025_state *cpustate)
{
	SHIFT_Preg_TO_ALU(cpustate);
	PUTDATA(cpustate, cpustate->ALU.w.h);
}
static void spl(tms32025_state *cpustate)
{
	SHIFT_Preg_TO_ALU(cpustate);
	PUTDATA(cpustate, cpustate->ALU.w.l);
}
static void spm(tms32025_state *cpustate)
{
	MODIFY_PM(cpustate, (cpustate->opcode.b.l & 3) );
}
static void sqra(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_ADD_CARRY(cpustate);
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
	cpustate->Preg.d = ((INT16)cpustate->ALU.w.l * (INT16)cpustate->ALU.w.l);
}
static void sqrs(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	SHIFT_Preg_TO_ALU(cpustate);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
	GETDATA(cpustate, 0, 0);
	cpustate->Treg = cpustate->ALU.w.l;
	cpustate->Preg.d = ((INT16)cpustate->ALU.w.l * (INT16)cpustate->ALU.w.l);
}
static void sst(tms32025_state *cpustate)
{
	PUTDATA_SST(cpustate, cpustate->STR0);
}
static void sst1(tms32025_state *cpustate)
{
	PUTDATA_SST(cpustate, cpustate->STR1);
}
static void ssxm(tms32025_state *cpustate)
{
	SET1(cpustate, SXM_FLAG);
}
static void stc(tms32025_state *cpustate)
{
	SET1(cpustate, TC_FLAG);
}
static void stxm(tms32025_state *cpustate)		/** Serial port stuff */
{
	SET1(cpustate, TXM_FLAG);
}
static void sub(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, (cpustate->opcode.b.h & 0xf), SXM);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
}
static void subb(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	if (CARRY == 0) cpustate->ACC.d--;
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	if (cpustate->ACC.d == cpustate->oldacc.d) {}	/* edge case, carry remains same */
	else CALCULATE_SUB_CARRY(cpustate);
}
static void subc(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 15, SXM);
	cpustate->ACC.d -= cpustate->ALU.d;		/* Temporary switch to ACC. Actual calculation is done as (ACC)-[mem] -> ALU, will be preserved later on. */
	if ((INT32)((cpustate->oldacc.d ^ cpustate->ALU.d) & (cpustate->oldacc.d ^ cpustate->ACC.d)) < 0) {
		SET0(cpustate, OV_FLAG);			/* Not affected by OVM */
	}
	CALCULATE_SUB_CARRY(cpustate);
	if( cpustate->oldacc.d >= cpustate->ALU.d ) {
		cpustate->ALU.d = cpustate->ACC.d;
		cpustate->ACC.d = cpustate->ACC.d << 1 | 1;
	}
	else {
		cpustate->ALU.d = cpustate->ACC.d;
		cpustate->ACC.d = cpustate->oldacc.d << 1;
	}
}
static void subh(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.w.h -= cpustate->ALU.w.l;
	if ( (UINT16)(cpustate->oldacc.w.h) < (UINT16)(cpustate->ACC.w.h) ) {
		CLR1(cpustate, C_FLAG);	/* Carry flag is not affected, if no borrow occurred */
	}
	if ((INT16)((cpustate->oldacc.w.h ^ cpustate->ALU.w.l) & (cpustate->oldacc.w.h ^ cpustate->ACC.w.h)) < 0) {
		SET0(cpustate, OV_FLAG);
		if (OVM) cpustate->ACC.w.h = ((INT16)cpustate->oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
static void subk(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	cpustate->ALU.d = (UINT8)cpustate->opcode.b.l;
	cpustate->ACC.d -= cpustate->ALU.b.l;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
}
static void subs(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.d -= cpustate->ALU.w.l;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
}
static void subt(tms32025_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	GETDATA(cpustate, (cpustate->Treg & 0xf), SXM);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
	CALCULATE_SUB_CARRY(cpustate);
}
static void sxf(tms32025_state *cpustate)
{
	SET1(cpustate, XF_FLAG);
	S_OUT(TMS32025_XF,ASSERT_LINE);
}
static void tblr(tms32025_state *cpustate)
{
	if (cpustate->init_load_addr) {
		cpustate->PFC = cpustate->ACC.w.l;
	}
	cpustate->ALU.w.l = M_RDROM(cpustate, cpustate->PFC);
	if ( (CNF0) && ( (UINT16)(cpustate->PFC) >= 0xff00 ) ) {}	/** TMS32025 only */
	else cpustate->tms32025_dec_cycles += (1*CLK);
	PUTDATA(cpustate, cpustate->ALU.w.l);
	cpustate->PFC++;
}
static void tblw(tms32025_state *cpustate)
{
	if (cpustate->init_load_addr) {
		cpustate->PFC = cpustate->ACC.w.l;
	}
	cpustate->tms32025_dec_cycles += (1*CLK);
	GETDATA(cpustate, 0, 0);
	if (cpustate->external_mem_access) cpustate->tms32025_dec_cycles += (1*CLK);
	M_WRTROM(cpustate, cpustate->PFC, cpustate->ALU.w.l);
	cpustate->PFC++;
}
static void trap(tms32025_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->PC);
	SET_PC(0x001E);		/* Trap vector */
}
static void xor_(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.w.l ^= cpustate->ALU.w.l;
}
static void xork(tms32025_state *cpustate)
{
	cpustate->ALU.d = M_RDOP_ARG(cpustate->PC);
	cpustate->PC++;
	cpustate->ALU.d <<= (cpustate->opcode.b.h & 0xf);
	cpustate->ACC.d ^= cpustate->ALU.d;
}
static void zalh(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.w.h = cpustate->ALU.w.l;
	cpustate->ACC.w.l = 0x0000;
}
static void zalr(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.w.h = cpustate->ALU.w.l;
	cpustate->ACC.w.l = 0x8000;
}
static void zals(tms32025_state *cpustate)
{
	GETDATA(cpustate, 0, 0);
	cpustate->ACC.w.l = cpustate->ALU.w.l;
	cpustate->ACC.w.h = 0x0000;
}


/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

static const tms32025_opcode opcode_main[256]=
{
/*00*/ {1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},
/*08*/ {1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},{1*CLK, add		},
/*10*/ {1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},
/*18*/ {1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},{1*CLK, sub		},
/*20*/ {1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},
/*28*/ {1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},{1*CLK, lac		},
/*30*/ {1*CLK, lar_ar0	},{1*CLK, lar_ar1	},{1*CLK, lar_ar2	},{1*CLK, lar_ar3	},{1*CLK, lar_ar4	},{1*CLK, lar_ar5	},{1*CLK, lar_ar6	},{1*CLK, lar_ar7	},
/*38*/ {1*CLK, mpy		},{1*CLK, sqra		},{1*CLK, mpya		},{1*CLK, mpys		},{1*CLK, lt		},{1*CLK, lta		},{1*CLK, ltp		},{1*CLK, ltd		},
/*40*/ {1*CLK, zalh		},{1*CLK, zals		},{1*CLK, lact		},{1*CLK, addc		},{1*CLK, subh		},{1*CLK, subs		},{1*CLK, subt		},{1*CLK, subc		},
/*48*/ {1*CLK, addh		},{1*CLK, adds		},{1*CLK, addt		},{1*CLK, rpt		},{1*CLK, xor_		},{1*CLK, or_		},{1*CLK, and_		},{1*CLK, subb		},
/*50*/ {1*CLK, lst		},{1*CLK, lst1		},{1*CLK, ldp		},{1*CLK, lph		},{1*CLK, pshd		},{1*CLK, mar		},{1*CLK, dmov		},{1*CLK, bitt		},
/*58*/ {3*CLK, tblr		},{2*CLK, tblw		},{1*CLK, sqrs		},{1*CLK, lts		},{2*CLK, macd		},{2*CLK, mac		},{2*CLK, bc		},{2*CLK, bnc		},
/*60*/ {1*CLK, sacl		},{1*CLK, sacl		},{1*CLK, sacl		},{1*CLK, sacl		},{1*CLK, sacl		},{1*CLK, sacl		},{1*CLK, sacl		},{1*CLK, sacl		},
/*68*/ {1*CLK, sach		},{1*CLK, sach		},{1*CLK, sach		},{1*CLK, sach		},{1*CLK, sach		},{1*CLK, sach		},{1*CLK, sach		},{1*CLK, sach		},
/*70*/ {1*CLK, sar_ar0	},{1*CLK, sar_ar1	},{1*CLK, sar_ar2	},{1*CLK, sar_ar3	},{1*CLK, sar_ar4	},{1*CLK, sar_ar5	},{1*CLK, sar_ar6	},{1*CLK, sar_ar7	},
/*78*/ {1*CLK, sst		},{1*CLK, sst1		},{1*CLK, popd		},{1*CLK, zalr		},{1*CLK, spl		},{1*CLK, sph		},{1*CLK, adrk		},{1*CLK, sbrk_ar	},
/*80*/ {2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},
/*88*/ {2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},{2*CLK, in		},
/*90*/ {1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},
/*98*/ {1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},{1*CLK, bit		},
/*A0*/ {1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},
/*A8*/ {1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},
/*B0*/ {1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},
/*B8*/ {1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},{1*CLK, mpyk		},
/*C0*/ {1*CLK, lark_ar0	},{1*CLK, lark_ar1	},{1*CLK, lark_ar2	},{1*CLK, lark_ar3	},{1*CLK, lark_ar4	},{1*CLK, lark_ar5	},{1*CLK, lark_ar6	},{1*CLK, lark_ar7	},
/*C8*/ {1*CLK, ldpk		},{1*CLK, ldpk		},{1*CLK, lack		},{1*CLK, rptk		},{1*CLK, addk		},{1*CLK, subk		},{1*CLK, opcodes_CE},{1*CLK, mpyu		},
/*D0*/ {1*CLK,opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{0*CLK, opcodes_Dx},
/*D8*/ {1*CLK,opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},{1*CLK, opcodes_Dx},
/*E0*/ {2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},
/*E8*/ {2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},{2*CLK, out		},
/*F0*/ {2*CLK, bv		},{2*CLK, bgz		},{2*CLK, blez		},{2*CLK, blz		},{2*CLK, bgez		},{2*CLK, bnz		},{2*CLK, bz		},{2*CLK, bnv		},
/*F8*/ {2*CLK, bbz		},{2*CLK, bbnz		},{2*CLK, bioz		},{2*CLK, banz		},{2*CLK, blkp		},{2*CLK, blkd		},{2*CLK, call		},{2*CLK, br		}
};

static const tms32025_opcode_CE opcode_CE_subset[256]=	/* Instructions living under the CExx opcode */
{
/*00*/ {1*CLK, eint		},{1*CLK, dint		},{1*CLK, rovm		},{1*CLK, sovm		},{1*CLK, cnfd		},{1*CLK, cnfp		},{1*CLK, rsxm		},{1*CLK, ssxm		},
/*08*/ {1*CLK, spm		},{1*CLK, spm		},{1*CLK, spm		},{1*CLK, spm		},{1*CLK, rxf		},{1*CLK, sxf		},{1*CLK, fort		},{1*CLK, fort		},
/*10*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, pac		},{1*CLK, apac		},{1*CLK, spac		},{0*CLK, illegal	},
/*18*/ {1*CLK, sfl		},{1*CLK, sfr		},{0*CLK, illegal	},{1*CLK, abst		},{1*CLK, push		},{1*CLK, pop		},{2*CLK, trap		},{3*CLK, idle		},
/*20*/ {1*CLK, rtxm		},{1*CLK, stxm		},{0*CLK, illegal	},{1*CLK, neg		},{2*CLK, cala		},{2*CLK, bacc		},{2*CLK, ret		},{1*CLK, cmpl		},
/*28*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*30*/ {1*CLK, rc		},{1*CLK, sc		},{1*CLK, rtc		},{1*CLK, stc		},{1*CLK, rol		},{1*CLK, ror		},{1*CLK, rfsm		},{1*CLK, sfsm		},
/*38*/ {1*CLK, rhm		},{1*CLK, shm		},{0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, conf		},{1*CLK, conf		},{1*CLK, conf		},{1*CLK, conf		},
/*40*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*48*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*50*/ {1*CLK, cmpr		},{1*CLK, cmpr		},{1*CLK, cmpr		},{1*CLK, cmpr		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*58*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*60*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*68*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*70*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*78*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*80*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*88*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*90*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*98*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*A0*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*A8*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*B0*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*B8*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*C0*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*C8*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*D0*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*D8*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*E0*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*E8*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*F0*/ {0*CLK, illegal	},{0*CLK, illegal	},{1*CLK, norm		},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},
/*F8*/ {0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	},{0*CLK, illegal	}
};

static const tms32025_opcode_Dx opcode_Dx_subset[8]=	/* Instructions living under the Dxxx opcode */
{
/*00*/ {2*CLK, lrlk		},{2*CLK, lalk		},{2*CLK, adlk		},{2*CLK, sblk		},{2*CLK, andk		},{2*CLK, ork		},{2*CLK, xork		},{0*CLK, illegal	}
};



/****************************************************************************
 *  Initialise the CPU emulation
 ****************************************************************************/
static CPU_INIT( tms32025 )
{
	tms32025_state *cpustate = get_safe_token(device);

	cpustate->intRAM = auto_alloc_array(device->machine(), UINT16, 0x800);
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = device->space(AS_DATA);
	cpustate->io = device->space(AS_IO);

	device->save_item(NAME(cpustate->PC));
	device->save_item(NAME(cpustate->STR0));
	device->save_item(NAME(cpustate->STR1));
	device->save_item(NAME(cpustate->PFC));
	device->save_item(NAME(cpustate->IFR));
	device->save_item(NAME(cpustate->RPTC));
	device->save_item(NAME(cpustate->ACC.d));
	device->save_item(NAME(cpustate->ALU.d));
	device->save_item(NAME(cpustate->Preg.d));
	device->save_item(NAME(cpustate->Treg));
	device->save_item(NAME(cpustate->AR[0]));
	device->save_item(NAME(cpustate->AR[1]));
	device->save_item(NAME(cpustate->AR[2]));
	device->save_item(NAME(cpustate->AR[3]));
	device->save_item(NAME(cpustate->AR[4]));
	device->save_item(NAME(cpustate->AR[5]));
	device->save_item(NAME(cpustate->AR[6]));
	device->save_item(NAME(cpustate->AR[7]));
	device->save_item(NAME(cpustate->STACK[0]));
	device->save_item(NAME(cpustate->STACK[1]));
	device->save_item(NAME(cpustate->STACK[2]));
	device->save_item(NAME(cpustate->STACK[3]));
	device->save_item(NAME(cpustate->STACK[4]));
	device->save_item(NAME(cpustate->STACK[5]));
	device->save_item(NAME(cpustate->STACK[6]));
	device->save_item(NAME(cpustate->STACK[7]));

	device->save_item(NAME(cpustate->oldacc));
	device->save_item(NAME(cpustate->memaccess));
	device->save_item(NAME(cpustate->icount));
	device->save_item(NAME(cpustate->mHackIgnoreARP));

	device->save_item(NAME(cpustate->idle));
	device->save_item(NAME(cpustate->hold));
	device->save_item(NAME(cpustate->external_mem_access));
	device->save_item(NAME(cpustate->init_load_addr));
	device->save_item(NAME(cpustate->PREVPC));

//  device->save_pointer(NAME(cpustate->intRAM), 0x800*2);
}

/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/
static CPU_RESET( tms32025 )
{
	tms32025_state *cpustate = get_safe_token(device);

	SET_PC(0);					/* Starting address on a reset */
	cpustate->STR0 |= 0x0600;	/* INTM and unused bit set to 1 */
	cpustate->STR0 &= 0xefff;	/* OV cleared to 0. Remaining bits undefined */
	cpustate->STR1 |= 0x07f0;	/* SXM, C, HM, FSM, XF and unused bits set to 1 */
	cpustate->STR1 &= 0xeff0;	/* CNF, FO, TXM, PM bits cleared to 0. Remaining bits undefined */
	cpustate->RPTC = 0;			/* Reset repeat counter to 0 */
	cpustate->IFR = 0;			/* IRQ pending flags */

	S_OUT(TMS32025_XF,ASSERT_LINE);	/* XF flag is high. Must set the pin */

	/* Set the internal memory mapped registers */
	GREG = 0;
	TIM  = 0xffff;
	PRD  = 0xffff;
	IMR  = 0xffc0;

	cpustate->idle = 0;
	cpustate->hold = 0;
	cpustate->tms32025_dec_cycles = 0;
	cpustate->init_load_addr = 1;

	/* Reset the Data/Program address banks */
	memset(cpustate->pgmmap, 0, sizeof(cpustate->pgmmap));
	memset(cpustate->datamap, 0, sizeof(cpustate->datamap));

	cpustate->datamap[0] = &cpustate->intRAM[0x000];			/* B2 */
	cpustate->datamap[4] = &cpustate->intRAM[0x200];			/* B0 */
	cpustate->datamap[5] = &cpustate->intRAM[0x280];			/* B0 */
	cpustate->datamap[6] = &cpustate->intRAM[0x300];			/* B1 */
	cpustate->datamap[7] = &cpustate->intRAM[0x380];			/* B1 */
}

static CPU_RESET( tms32026 )
{
	tms32025_state *cpustate = get_safe_token(device);

	CPU_RESET_CALL(tms32025);

	/* Reset the Data/Program address banks */
	memset(cpustate->pgmmap, 0, sizeof(cpustate->pgmmap));
	memset(cpustate->datamap, 0, sizeof(cpustate->datamap));

	cpustate->datamap[0] = &cpustate->intRAM[0x000];			/* B2 */
	cpustate->datamap[4] = &cpustate->intRAM[0x200];			/* B0 */
	cpustate->datamap[5] = &cpustate->intRAM[0x280];			/* B0 */
	cpustate->datamap[6] = &cpustate->intRAM[0x300];			/* B0 */
	cpustate->datamap[7] = &cpustate->intRAM[0x380];			/* B0 */
	cpustate->datamap[8] = &cpustate->intRAM[0x400];			/* B1 */
	cpustate->datamap[9] = &cpustate->intRAM[0x480];			/* B1 */
	cpustate->datamap[10] = &cpustate->intRAM[0x500];			/* B1 */
	cpustate->datamap[11] = &cpustate->intRAM[0x580];			/* B1 */
	cpustate->datamap[12] = &cpustate->intRAM[0x600];			/* B3 */
	cpustate->datamap[13] = &cpustate->intRAM[0x680];			/* B3 */
	cpustate->datamap[14] = &cpustate->intRAM[0x700];			/* B3 */
	cpustate->datamap[15] = &cpustate->intRAM[0x780];			/* B3 */
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/
static CPU_EXIT( tms32025 )
{
}


/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/
INLINE int process_IRQs(tms32025_state *cpustate)
{
	/********** Interrupt Flag Register (IFR) **********
        |  5  |  4  |  3  |  2  |  1  |  0  |
        | XINT| RINT| TINT| INT2| INT1| INT0|
    */

	cpustate->tms32025_irq_cycles = 0;

	/* Dont service Interrupts if masked, or prev instruction was EINT ! */

	if ( (INTM == 0) && (cpustate->opcode.w.l != 0xce00) && (cpustate->IFR & IMR) )
	{
		cpustate->tms32025_irq_cycles = (3*CLK);	/* 3 clock cycles used due to PUSH and DINT operation ? */
		PUSH_STACK(cpustate, cpustate->PC);

		if ((cpustate->IFR & 0x01) && (IMR & 0x01)) {		/* IRQ line 0 */
			//logerror("TMS32025:  Active INT0\n");
			SET_PC(0x0002);
			(*cpustate->irq_callback)(cpustate->device, 0);
			cpustate->idle = 0;
			cpustate->IFR &= (~0x01);
			SET0(cpustate, INTM_FLAG);
			return cpustate->tms32025_irq_cycles;
		}
		if ((cpustate->IFR & 0x02) && (IMR & 0x02)) {		/* IRQ line 1 */
			//logerror("TMS32025:  Active INT1\n");
			SET_PC(0x0004);
			(*cpustate->irq_callback)(cpustate->device, 1);
			cpustate->idle = 0;
			cpustate->IFR &= (~0x02);
			SET0(cpustate, INTM_FLAG);
			return cpustate->tms32025_irq_cycles;
		}
		if ((cpustate->IFR & 0x04) && (IMR & 0x04)) {		/* IRQ line 2 */
			//logerror("TMS32025:  Active INT2\n");
			SET_PC(0x0006);
			(*cpustate->irq_callback)(cpustate->device, 2);
			cpustate->idle = 0;
			cpustate->IFR &= (~0x04);
			SET0(cpustate, INTM_FLAG);
			return cpustate->tms32025_irq_cycles;
		}
		if ((cpustate->IFR & 0x08) && (IMR & 0x08)) {		/* Timer IRQ (internal) */
//          logerror("TMS32025:  Active TINT (Timer)\n");
			SET_PC(0x0018);
			cpustate->idle = 0;
			cpustate->IFR &= (~0x08);
			SET0(cpustate, INTM_FLAG);
			return cpustate->tms32025_irq_cycles;
		}
		if ((cpustate->IFR & 0x10) && (IMR & 0x10)) {		/* Serial port receive IRQ (internal) */
//          logerror("TMS32025:  Active RINT (Serial receive)\n");
			DRR = S_IN(TMS32025_DR);
			SET_PC(0x001A);
			cpustate->idle = 0;
			cpustate->IFR &= (~0x10);
			SET0(cpustate, INTM_FLAG);
			return cpustate->tms32025_irq_cycles;
		}
		if ((cpustate->IFR & 0x20) && (IMR & 0x20)) {		/* Serial port transmit IRQ (internal) */
//          logerror("TMS32025:  Active XINT (Serial transmit)\n");
			S_OUT(TMS32025_DX,DXR);
			SET_PC(0x001C);
			cpustate->idle = 0;
			cpustate->IFR &= (~0x20);
			SET0(cpustate, INTM_FLAG);
			return cpustate->tms32025_irq_cycles;
		}
	}
	return cpustate->tms32025_irq_cycles;
}

static void set_fsx_line(tms32025_state *cpustate, int state)
{
	if (state != CLEAR_LINE && cpustate->waiting_for_serial_frame)
	{
		cpustate->waiting_for_serial_frame = 0;
		cpustate->IFR = 0x20;
	}
}

INLINE void process_timer(tms32025_state *cpustate, int clocks)
{
	int preclocks, ticks;

	/* easy case: no actual ticks */
again:
	preclocks = CLK - cpustate->timerover;
	if (clocks < preclocks)
	{
		cpustate->timerover += clocks;
		cpustate->icount -= clocks;
		return;
	}

	/* if we're not going to overflow the timer, just count the clocks */
	ticks = 1 + (clocks - preclocks) / CLK;
	if (ticks <= TIM)
	{
		cpustate->icount -= clocks;
		cpustate->timerover = clocks - (ticks - 1) * CLK - preclocks;
		TIM -= ticks;
	}

	/* otherwise, overflow the timer and signal an interrupt */
	else
	{
		cpustate->icount -= preclocks + CLK * TIM;
		cpustate->timerover = 0;
		TIM = PRD;

		cpustate->IFR |= 0x08;
		clocks = process_IRQs(cpustate);		/* Handle Timer IRQ */
		goto again;
	}
}


/****************************************************************************
 *  Execute ICount cycles. Exit when 0 or less
 ****************************************************************************/
static CPU_EXECUTE( tms32025 )
{
	tms32025_state *cpustate = get_safe_token(device);


	/**** Respond to external hold signal */
	if (S_IN(TMS32025_HOLD) == ASSERT_LINE) {
		if (cpustate->hold == 0) {
			S_OUT(TMS32025_HOLDA,ASSERT_LINE);	/* Hold-Ack (active low) */
		}
		cpustate->hold = 1;
		if (HM) {
			cpustate->icount = 0;		/* Exit */
		}
		else {
			if (cpustate->external_mem_access) {
				cpustate->icount = 0;	/* Exit */
			}
		}
	}
	else {
		if (cpustate->hold == 1) {
			S_OUT(TMS32025_HOLDA,CLEAR_LINE);	/* Hold-Ack (active low) */
			process_timer(cpustate, 3);
		}
		cpustate->hold = 0;
	}

	/**** If idling, update timer and/or exit execution, but test for irqs first */
	if (cpustate->idle && cpustate->IFR && cpustate->icount > 0)
		cpustate->icount -= process_IRQs(cpustate);

	while (cpustate->idle && cpustate->icount > 0)
		process_timer(cpustate, cpustate->icount);

	if (cpustate->icount <= 0) debugger_instruction_hook(device, cpustate->PC);


	while (cpustate->icount > 0)
	{
	  cpustate->tms32025_dec_cycles = 0;

		if (cpustate->IFR) {	/* Check IRQ Flag Register for pending IRQs */
			cpustate->tms32025_dec_cycles += process_IRQs(cpustate);
		}

		cpustate->PREVPC = cpustate->PC;

		debugger_instruction_hook(device, cpustate->PC);

		cpustate->opcode.d = M_RDOP(cpustate->PC);
		cpustate->PC++;

		if (cpustate->opcode.b.h == 0xCE)	/* Opcode 0xCExx has many sub-opcodes in its minor byte */
		{
			cpustate->tms32025_dec_cycles += opcode_CE_subset[cpustate->opcode.b.l].cycles;
			(*opcode_CE_subset[cpustate->opcode.b.l].function)(cpustate);
		}
		else if ((cpustate->opcode.w.l & 0xf0f8) == 0xd000)	/* Opcode 0xDxxx has many sub-opcodes in its minor byte */
		{
			cpustate->tms32025_dec_cycles += opcode_Dx_subset[cpustate->opcode.b.l].cycles;
			(*opcode_Dx_subset[cpustate->opcode.b.l].function)(cpustate);
		}
		else			/* Do all opcodes except the CExx and Dxxx ones */
		{
			cpustate->tms32025_dec_cycles += opcode_main[cpustate->opcode.b.h].cycles;
			(*opcode_main[cpustate->opcode.b.h].function)(cpustate);
		}


		if (cpustate->init_load_addr == 2) {		/* Repeat next instruction */
			/****************************************************\
            ******* These instructions are not repeatable ********
            ** ADLK, ANDK, LALK, LRLK, ORK,  SBLK, XORK         **
            ** ADDK, ADRK, LACK, LARK, LDPK, MPYK, RPTK         **
            ** SBRK, SPM,  SUBK, ZAC,  IDLE, RPT,  TRAP         **
            ** BACC, CALA, RET                                  **
            ** B,    BANZ, BBNZ, BBZ,  BC,   BGEZ, BGZ,  BIOZ   **
            ** BNC,  BNV,  BNZ,  BV,   BZ,   CALL, BLEZ, BLZ    **
            \****************************************************/
			cpustate->PREVPC = cpustate->PC;

			debugger_instruction_hook(device, cpustate->PC);

			cpustate->opcode.d = M_RDOP(cpustate->PC);
			cpustate->PC++;
			cpustate->tms32025_dec_cycles += (1*CLK);

			do {
				if (cpustate->opcode.b.h == 0xCE)
				{							/* Do all 0xCExx Opcodes */
					if (cpustate->init_load_addr) {
						cpustate->tms32025_dec_cycles += (1*CLK);
					}
					else {
						cpustate->tms32025_dec_cycles += (1*CLK);
					}
					(*opcode_CE_subset[cpustate->opcode.b.l].function)(cpustate);
				}
				else
				{							/* Do all other opcodes */
					if (cpustate->init_load_addr) {
						cpustate->tms32025_dec_cycles += (1*CLK);
					}
					else {
						cpustate->tms32025_dec_cycles += (1*CLK);
					}
					(*opcode_main[cpustate->opcode.b.h].function)(cpustate);
				}
				cpustate->init_load_addr = 0;
				cpustate->RPTC-- ;
			} while ((INT8)(cpustate->RPTC) != -1);
			cpustate->RPTC = 0;
			cpustate->PFC = cpustate->PC;
			cpustate->init_load_addr = 1;
		}

		process_timer(cpustate, cpustate->tms32025_dec_cycles);

		/**** If device is put into idle mode, exit and wait for an interrupt */
		while (cpustate->idle && cpustate->icount > 0)
			process_timer(cpustate, cpustate->icount);


		/**** If hold pin is active, exit if accessing external memory or if HM is set */
		if (cpustate->hold) {
			if (cpustate->external_mem_access || (HM)) {
				if (cpustate->icount > 0) {
					cpustate->icount = 0;
				}
			}
		}
	}
}



/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/
static void set_irq_line(tms32025_state *cpustate, int irqline, int state)
{
	/* Pending IRQs cannot be cleared */

	if (state != CLEAR_LINE)
	{
		cpustate->IFR |= (1 << irqline);
	}
}


/****************************************************************************
 *  Opcode fetcher
 ****************************************************************************/
static CPU_READOP( tms32025 )
{
	tms32025_state *cpustate = get_safe_token(device);

	void *ptr;

	/* skip if not custom */
	if (!cpustate->pgmmap[offset >> 8])
		return 0;

	ptr = &((UINT8 *)&cpustate->pgmmap[offset >> 8])[offset & 0xff];
	switch (size)
	{
		case 1:	*value = *((UINT8 *) ptr);
		case 2:	*value = *((UINT16 *) ptr);
		case 4:	*value = *((UINT32 *) ptr);
		case 8:	*value = *((UINT64 *) ptr);
	}
	return 1;
}


/****************************************************************************
 *  Memory reader
 ****************************************************************************/
static CPU_READ( tms32025 )
{
	tms32025_state *cpustate = get_safe_token(device);

	void *ptr = NULL;
	UINT64 temp = 0;

	switch (space)
	{
		case AS_PROGRAM:
			ptr = cpustate->pgmmap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case AS_DATA:
			ptr = cpustate->datamap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		default:
		case AS_IO:
			return 0;
	}

	switch (size)
	{
		case 1:
			*value = ((UINT8 *)ptr)[BYTE_XOR_BE(offset & 0xff)];
			break;
		case 2:
			*value = ((UINT16 *)ptr)[(offset & 0xff) / 2];
			break;
		case 4:
			CPU_READ_NAME(tms32025)(device, space, offset + 0, 2, &temp);
			*value = temp << 16;
			CPU_READ_NAME(tms32025)(device, space, offset + 2, 2, &temp);
			*value |= temp & 0xffff;
			break;
		case 8:
			CPU_READ_NAME(tms32025)(device, space, offset + 0, 4, &temp);
			*value = temp << 32;
			CPU_READ_NAME(tms32025)(device, space, offset + 4, 4, &temp);
			*value |= temp & 0xffffffff;
			break;
	}
	return 1;
}


/****************************************************************************
 *  Memory writer
 ****************************************************************************/
static CPU_WRITE( tms32025 )
{
	tms32025_state *cpustate = get_safe_token(device);

	void *ptr = NULL;

	switch (space)
	{
		case AS_PROGRAM:
			ptr = cpustate->pgmmap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case AS_DATA:
			ptr = cpustate->datamap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		default:
		case AS_IO:
			return 0;
	}

	switch (size)
	{
		case 1:
			((UINT8 *)ptr)[BYTE_XOR_BE(offset & 0xff)] = value;
			break;
		case 2:
			((UINT16 *)ptr)[(offset & 0xff) / 2] = value;
			break;
		case 4:
			CPU_WRITE_NAME(tms32025)(device, space, offset + 0, 2, value >> 16);
			CPU_WRITE_NAME(tms32025)(device, space, offset + 2, 2, value);
			break;
		case 8:
			CPU_WRITE_NAME(tms32025)(device, space, offset + 0, 4, value >> 32);
			CPU_WRITE_NAME(tms32025)(device, space, offset + 4, 4, value);
			break;
	}

	return 1;
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( tms32025 )
{
	tms32025_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT0:		set_irq_line(cpustate, TMS32025_INT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT1:		set_irq_line(cpustate, TMS32025_INT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT2:		set_irq_line(cpustate, TMS32025_INT2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_TINT:		set_irq_line(cpustate, TMS32025_TINT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_RINT:		set_irq_line(cpustate, TMS32025_RINT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_XINT:		set_irq_line(cpustate, TMS32025_XINT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_FSX:		set_fsx_line(cpustate, info->i); break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32025_PC:		cpustate->PC = info->i;							break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32025_STK7:		cpustate->STACK[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK6:		cpustate->STACK[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK5:		cpustate->STACK[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK4:		cpustate->STACK[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK3:		cpustate->STACK[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK2:		cpustate->STACK[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK1:		cpustate->STACK[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK0:		cpustate->STACK[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STR0:		cpustate->STR0 = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_STR1:		cpustate->STR1 = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_IFR:		cpustate->IFR = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_RPTC:		cpustate->RPTC = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_ACC:		cpustate->ACC.d = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_PREG:		cpustate->Preg.d = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_TREG:		cpustate->Treg = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR0:		cpustate->AR[0] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR1:		cpustate->AR[1] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR2:		cpustate->AR[2] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR3:		cpustate->AR[3] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR4:		cpustate->AR[4] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR5:		cpustate->AR[5] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR6:		cpustate->AR[6] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR7:		cpustate->AR[7] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_DRR:		M_WRTRAM(cpustate, 0,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_DXR:		M_WRTRAM(cpustate, 1,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_TIM:		M_WRTRAM(cpustate, 2,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_PRD:		M_WRTRAM(cpustate, 3,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_IMR:		M_WRTRAM(cpustate, 4,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_GREG:		M_WRTRAM(cpustate, 5,info->i);					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( tms32025 )
{
	tms32025_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tms32025_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1*CLK;						break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 5*CLK;						break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 17;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = -1;					break;

		case CPUINFO_INT_INPUT_STATE + TMS32025_INT0:		info->i = (cpustate->IFR & 0x01) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT1:		info->i = (cpustate->IFR & 0x02) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT2:		info->i = (cpustate->IFR & 0x04) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_TINT:		info->i = (cpustate->IFR & 0x08) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_RINT:		info->i = (cpustate->IFR & 0x10) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_XINT:		info->i = (cpustate->IFR & 0x20) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->PREVPC;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32025_PC:		info->i = cpustate->PC;							break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32025_STK7:		info->i = cpustate->STACK[7];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK6:		info->i = cpustate->STACK[6];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK5:		info->i = cpustate->STACK[5];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK4:		info->i = cpustate->STACK[4];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK3:		info->i = cpustate->STACK[3];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK2:		info->i = cpustate->STACK[2];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK1:		info->i = cpustate->STACK[1];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK0:		info->i = cpustate->STACK[0];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STR0:		info->i = cpustate->STR0;						break;
		case CPUINFO_INT_REGISTER + TMS32025_STR1:		info->i = cpustate->STR1;						break;
		case CPUINFO_INT_REGISTER + TMS32025_IFR:		info->i = cpustate->IFR;						break;
		case CPUINFO_INT_REGISTER + TMS32025_RPTC:		info->i = cpustate->RPTC;						break;
		case CPUINFO_INT_REGISTER + TMS32025_ACC:		info->i = cpustate->ACC.d;						break;
		case CPUINFO_INT_REGISTER + TMS32025_PREG:		info->i = cpustate->Preg.d;						break;
		case CPUINFO_INT_REGISTER + TMS32025_TREG:		info->i = cpustate->Treg;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR0:		info->i = cpustate->AR[0];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR1:		info->i = cpustate->AR[1];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR2:		info->i = cpustate->AR[2];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR3:		info->i = cpustate->AR[3];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR4:		info->i = cpustate->AR[4];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR5:		info->i = cpustate->AR[5];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR6:		info->i = cpustate->AR[6];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR7:		info->i = cpustate->AR[7];						break;
		case CPUINFO_INT_REGISTER + TMS32025_DRR:		info->i = M_RDRAM(cpustate, 0);					break;
		case CPUINFO_INT_REGISTER + TMS32025_DXR:		info->i = M_RDRAM(cpustate, 1);					break;
		case CPUINFO_INT_REGISTER + TMS32025_TIM:		info->i = M_RDRAM(cpustate, 2);					break;
		case CPUINFO_INT_REGISTER + TMS32025_PRD:		info->i = M_RDRAM(cpustate, 3);					break;
		case CPUINFO_INT_REGISTER + TMS32025_IMR:		info->i = M_RDRAM(cpustate, 4);					break;
		case CPUINFO_INT_REGISTER + TMS32025_GREG:		info->i = M_RDRAM(cpustate, 5);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(tms32025);	break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(tms32025);			break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tms32025);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(tms32025);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(tms32025);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;								break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(tms32025);	break;
		case CPUINFO_FCT_READ:							info->read = CPU_READ_NAME(tms32025);				break;
		case CPUINFO_FCT_WRITE:							info->write = CPU_WRITE_NAME(tms32025);			break;
		case CPUINFO_FCT_READOP:						info->readop = CPU_READOP_NAME(tms32025);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32025");			break;
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Texas Instruments TMS320x25"); break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "1.10");				break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Tony La Porta"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "arp%d%c%c%c%cdp%03x  arb%d%c%c%c%c%c%c%c%c%c%c%cpm%d",
				(cpustate->STR0 & 0xe000) >> 13,
				cpustate->STR0 & 0x1000 ? 'O':'.',
				cpustate->STR0 & 0x0800 ? 'M':'.',
				cpustate->STR0 & 0x0400 ? '.':'?',
				cpustate->STR0 & 0x0200 ? 'I':'.',
				(cpustate->STR0 & 0x01ff),

				(cpustate->STR1 & 0xe000) >> 13,
				cpustate->STR1 & 0x1000 ? 'P':'D',
				cpustate->STR1 & 0x0800 ? 'T':'.',
				cpustate->STR1 & 0x0400 ? 'S':'.',
				cpustate->STR1 & 0x0200 ? 'C':'?',
				cpustate->STR0 & 0x0100 ? '.':'?',
				cpustate->STR1 & 0x0080 ? '.':'?',
				cpustate->STR1 & 0x0040 ? 'H':'.',
				cpustate->STR1 & 0x0020 ? 'F':'.',
				cpustate->STR1 & 0x0010 ? 'X':'.',
				cpustate->STR1 & 0x0008 ? 'f':'.',
				cpustate->STR1 & 0x0004 ? 'o':'i',
				(cpustate->STR1 & 0x0003) );
			break;

		case CPUINFO_STR_REGISTER + TMS32025_PC:		sprintf(info->s, "PC:%04X",  cpustate->PC); break;
		case CPUINFO_STR_REGISTER + TMS32025_STR0:		sprintf(info->s, "STR0:%04X", cpustate->STR0); break;
		case CPUINFO_STR_REGISTER + TMS32025_STR1:		sprintf(info->s, "STR1:%04X", cpustate->STR1); break;
		case CPUINFO_STR_REGISTER + TMS32025_IFR:		sprintf(info->s, "IFR:%04X", cpustate->IFR); break;
		case CPUINFO_STR_REGISTER + TMS32025_RPTC:		sprintf(info->s, "RPTC:%02X", cpustate->RPTC); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK7:		sprintf(info->s, "STK7:%04X", cpustate->STACK[7]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK6:		sprintf(info->s, "STK6:%04X", cpustate->STACK[6]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK5:		sprintf(info->s, "STK5:%04X", cpustate->STACK[5]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK4:		sprintf(info->s, "STK4:%04X", cpustate->STACK[4]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK3:		sprintf(info->s, "STK3:%04X", cpustate->STACK[3]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK2:		sprintf(info->s, "STK2:%04X", cpustate->STACK[2]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK1:		sprintf(info->s, "STK1:%04X", cpustate->STACK[1]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK0:		sprintf(info->s, "STK0:%04X", cpustate->STACK[0]); break;
		case CPUINFO_STR_REGISTER + TMS32025_ACC:		sprintf(info->s, "ACC:%08X", cpustate->ACC.d); break;
		case CPUINFO_STR_REGISTER + TMS32025_PREG:		sprintf(info->s, "P:%08X", cpustate->Preg.d); break;
		case CPUINFO_STR_REGISTER + TMS32025_TREG:		sprintf(info->s, "T:%04X", cpustate->Treg); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR0:		sprintf(info->s, "AR0:%04X", cpustate->AR[0]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR1:		sprintf(info->s, "AR1:%04X", cpustate->AR[1]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR2:		sprintf(info->s, "AR2:%04X", cpustate->AR[2]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR3:		sprintf(info->s, "AR3:%04X", cpustate->AR[3]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR4:		sprintf(info->s, "AR4:%04X", cpustate->AR[4]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR5:		sprintf(info->s, "AR5:%04X", cpustate->AR[5]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR6:		sprintf(info->s, "AR6:%04X", cpustate->AR[6]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR7:		sprintf(info->s, "AR7:%04X", cpustate->AR[7]); break;
		case CPUINFO_STR_REGISTER + TMS32025_DRR:		sprintf(info->s, "DRR:%04X", M_RDRAM(cpustate, 0)); break;
		case CPUINFO_STR_REGISTER + TMS32025_DXR:		sprintf(info->s, "DXR:%04X", M_RDRAM(cpustate, 1)); break;
		case CPUINFO_STR_REGISTER + TMS32025_TIM:		sprintf(info->s, "TIM:%04X", M_RDRAM(cpustate, 2)); break;
		case CPUINFO_STR_REGISTER + TMS32025_PRD:		sprintf(info->s, "PRD:%04X", M_RDRAM(cpustate, 3)); break;
		case CPUINFO_STR_REGISTER + TMS32025_IMR:		sprintf(info->s, "IMR:%04X", M_RDRAM(cpustate, 4)); break;
		case CPUINFO_STR_REGISTER + TMS32025_GREG:		sprintf(info->s, "GREG:%04X", M_RDRAM(cpustate, 5)); break;
	}
}


/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( tms32026 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tms32026);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32026");			break;

		default:										CPU_GET_INFO_CALL(tms32025);			break;
	}
}

DEFINE_LEGACY_CPU_DEVICE(TMS32025, tms32025);
DEFINE_LEGACY_CPU_DEVICE(TMS32026, tms32026);
