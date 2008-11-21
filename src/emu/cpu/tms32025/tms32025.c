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
 #  Correct the mulit-cycle instruction cycle counts
 #  Add support to set ROM & RAM as Internal/External in order to correctly
    compute cycle timings
 #  Possibly add internal memory into here (instead of having it in the driver)
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



#include "debugger.h"
#include "tms32025.h"


#define CLK 4	/* 1 cycle equals 4 clock ticks */		/* PE/DI */


#ifndef INLINE
#define INLINE static inline
#endif


static UINT16 *tms32025_pgmmap[0x200];
static UINT16 *tms32025_datamap[0x200];

#define SET_PC(x)	do { R.PC = (x); change_pc(R.PC<<1); } while (0)

INLINE UINT16 M_RDROM(offs_t addr)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = tms32025_pgmmap[addr >> 7];
	if (ram) return ram[addr & 0x7f];
	return program_read_word_16be(addr << 1);
}

INLINE void M_WRTROM(offs_t addr, UINT16 data)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = tms32025_pgmmap[addr >> 7];
	if (ram) { ram[addr & 0x7f] = data; }
	else program_write_word_16be(addr << 1, data);
}

INLINE UINT16 M_RDRAM(offs_t addr)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = tms32025_datamap[addr >> 7];
	if (ram) return ram[addr & 0x7f];
	return data_read_word_16be(addr << 1);
}

INLINE void M_WRTRAM(offs_t addr, UINT16 data)
{
	UINT16 *ram;
	addr &= 0xffff;
	ram = tms32025_datamap[addr >> 7];
	if (ram) { ram[addr & 0x7f] = data; }
	else data_write_word_16be(addr << 1, data);
}

#define P_IN(A)			(io_read_word_16be((A)<<1))
#define P_OUT(A,V)		(io_write_word_16be(((A)<<1),(V)))
#define S_IN(A)			(io_read_word_16be((A)<<1))
#define S_OUT(A,V)		(io_write_word_16be(((A)<<1),(V)))

#define M_RDOP(A)		((tms32025_pgmmap[(A) >> 7]) ? (tms32025_pgmmap[(A) >> 7][(A) & 0x7f]) : program_decrypted_read_word((A)<<1))
#define M_RDOP_ARG(A)	((tms32025_pgmmap[(A) >> 7]) ? (tms32025_pgmmap[(A) >> 7][(A) & 0x7f]) : program_decrypted_read_word((A)<<1))



typedef struct			/* Page 3-6 (45) shows all registers */
{
	/******************** CPU Internal Registers *******************/
	UINT16	PREVPC;		/* previous program counter */
	UINT16	PC;
	UINT16	PFC;
	UINT16	STR0, STR1;
	UINT8	IFR;
	UINT8	RPTC;
	PAIR	ACC;		/* PAIR defined in os/osd_cpu.h */
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
	cpu_irq_callback irq_callback;
	const device_config *device;
	UINT16 *datamap_save[16];
	UINT16 *pgmmap_save[12];
} tms32025_Regs;

static tms32025_Regs R;
static PAIR  oldacc;
static UINT32 memaccess;
static int tms32025_icount;
typedef void (*opcode_fn) (void);


/************************** Memory mapped registers ****************/
#define DRR 	R.intRAM[0]
#define DXR 	R.intRAM[1]
#define TIM 	R.intRAM[2]
#define PRD 	R.intRAM[3]
#define IMR 	R.intRAM[4]
#define GREG	R.intRAM[5]



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


#define OV		( R.STR0 & OV_FLAG)			/* OV   (Overflow flag) */
#define OVM		( R.STR0 & OVM_FLAG)		/* OVM  (Overflow Mode bit) 1 indicates an overflow */
#define INTM	( R.STR0 & INTM_FLAG)		/* INTM (Interrupt enable flag) 0 enables maskable interrupts */
#define ARP		((R.STR0 & ARP_REG) >> 13)	/* ARP  (Auxiliary Register Pointer) */
#define DP		((R.STR0 & DP_REG) << 7)	/* DP   (Data memory Pointer bit) */
#define ARB		( R.STR1 & ARB_REG)			/* ARB  (Backup Auxiliary Register pointer) */
#define CNF0	( R.STR1 & CNF0_REG)		/* CNF0 (Onchip Ram Config register) */
#define TC		( R.STR1 & TC_FLAG)			/* TC   (Test Control Flag) */
#define SXM		( R.STR1 & SXM_FLAG)		/* SXM  (Sign Extension Mode) */
#define CARRY	( R.STR1 & C_FLAG)			/* C    (Carry Flag for accumulator) */
#define HM		( R.STR1 & HM_FLAG)			/* HM   (Processor Hold Mode) */
#define FSM		( R.STR1 & FSM_FLAG)		/* FSM  (Frame Synchronization Mode - for serial port) */
#define XF		( R.STR1 & FSM_FLAG)		/* XF   (XF output pin status) */
#define FO		( R.STR1 & FO_FLAG)			/* FO   (Serial port Format In/Out mode) */
#define TXM		( R.STR1 & TXM_FLAG)		/* TXM  (Transmit Mode - for serial port) */
#define PM		( R.STR1 & PM_REG)			/* PM   (P register shift Mode. See SHIFT_Preg_TO_ALU below )*/

#define DMA		(DP | (R.opcode.b.l & 0x7f))	/* address used in direct memory access operations */
#define DMApg0	(R.opcode.b.l & 0x7f)			/* address used in direct memory access operations for sst instruction */
#define IND		R.AR[ARP]						/* address used in indirect memory access operations */

INLINE void CLR0(UINT16 flag) { R.STR0 &= ~flag; R.STR0 |= 0x0400; }
INLINE void SET0(UINT16 flag) { R.STR0 |=  flag; R.STR0 |= 0x0400; }
INLINE void CLR1(UINT16 flag) { R.STR1 &= ~flag; R.STR1 |= 0x0180; }
INLINE void SET1(UINT16 flag) { R.STR1 |=  flag; R.STR1 |= 0x0180; }
INLINE void MODIFY_DP (int data) { R.STR0 &= ~DP_REG;  R.STR0 |= (data & DP_REG); R.STR0 |= 0x0400; }
INLINE void MODIFY_PM (int data) { R.STR1 &= ~PM_REG;  R.STR1 |= (data & PM_REG); R.STR1 |= 0x0180; }
INLINE void MODIFY_ARP(int data) { R.STR1 &= ~ARB_REG; R.STR1 |= (R.STR0 & ARP_REG); R.STR1 |= 0x0180;
								   R.STR0 &= ~ARP_REG; R.STR0 |= ((data << 13) & ARP_REG); R.STR0 |= 0x0400; }
#ifdef UNUSED_FUNCTION
INLINE void MODIFY_ARB(int data) { R.STR1 &= ~ARB_REG; R.STR1 |= ((data << 13) & ARB_REG); R.STR1 |= 0x0180; }
#endif

static int mHackIgnoreARP; /* special handling for lst, lst1 instructions */

static UINT16 reverse_carry_add( UINT16 arg0, UINT16 arg1 )
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

INLINE void MODIFY_AR_ARP(void)
{ /* modify address register referenced by ARP */
	switch (R.opcode.b.l & 0x70)		/* Cases ordered by predicted useage */
	{
		case 0x00: /* 000   nop      */
			break;

		case 0x10: /* 001   *-       */
			R.AR[ARP] -- ;
			break;

		case 0x20: /* 010   *+       */
			R.AR[ARP] ++ ;
			break;

		case 0x30: /* 011   reserved */
			break;

		case 0x40: /* 100   *BR0-    */
			R.AR[ARP] = reverse_carry_add(R.AR[ARP],-R.AR[0]);
			break;

		case 0x50: /* 101   *0-      */
			R.AR[ARP] -= R.AR[0];
			break;

		case 0x60: /* 110   *0+      */
			R.AR[ARP] += R.AR[0];
			break;

		case 0x70: /* 111   *BR0+    */
			R.AR[ARP] += reverse_carry_add(R.AR[ARP],R.AR[0]);
			break;

		default:
			break;
	}

	if( !mHackIgnoreARP )
	{
		if (R.opcode.b.l & 8)
		{ /* bit 3 determines if new value is loaded into ARP */
			MODIFY_ARP(R.opcode.b.l & 7);
		}
	}
}

INLINE void CALCULATE_ADD_CARRY(void)
{
	if ( (UINT32)(oldacc.d) > (UINT32)(R.ACC.d) ) {
		SET1(C_FLAG);
	}
	else {
		CLR1(C_FLAG);
	}
}

INLINE void CALCULATE_SUB_CARRY(void)
{
	if ( (UINT32)(oldacc.d) < (UINT32)(R.ACC.d) ) {
		CLR1(C_FLAG);
	}
	else {
		SET1(C_FLAG);
	}
}

INLINE void CALCULATE_ADD_OVERFLOW(INT32 addval)
{
	if ((INT32)(~(oldacc.d ^ addval) & (oldacc.d ^ R.ACC.d)) < 0)
	{
		SET0(OV_FLAG);
		if (OVM)
		{
		// Stroff:HACK! support for overflow capping as implemented results in bad DSP floating point math in many
		// System22 games - for example, the score display in Prop Cycle.
		//  R.ACC.d = ((INT32)oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
		}
	}
}
INLINE void CALCULATE_SUB_OVERFLOW(INT32 subval)
{
	if ((INT32)((oldacc.d ^ subval) & (oldacc.d ^ R.ACC.d)) < 0)
	{
		SET0(OV_FLAG);
		if (OVM)
		{
			R.ACC.d = ((INT32)oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
		}
	}
}

INLINE UINT16 POP_STACK(void)
{
	UINT16 data = R.STACK[7];
	R.STACK[7] = R.STACK[6];
	R.STACK[6] = R.STACK[5];
	R.STACK[5] = R.STACK[4];
	R.STACK[4] = R.STACK[3];
	R.STACK[3] = R.STACK[2];
	R.STACK[2] = R.STACK[1];
	R.STACK[1] = R.STACK[0];
	return data;
}
INLINE void PUSH_STACK(UINT16 data)
{
	R.STACK[0] = R.STACK[1];
	R.STACK[1] = R.STACK[2];
	R.STACK[2] = R.STACK[3];
	R.STACK[3] = R.STACK[4];
	R.STACK[4] = R.STACK[5];
	R.STACK[5] = R.STACK[6];
	R.STACK[6] = R.STACK[7];
	R.STACK[7] = data;
}

INLINE void SHIFT_Preg_TO_ALU(void)
{
	switch(PM)		/* PM (in STR1) is the shift mode for Preg */
	{
		case 0:		R.ALU.d = R.Preg.d; break;
		case 1:		R.ALU.d = (R.Preg.d << 1); break;
		case 2:		R.ALU.d = (R.Preg.d << 4); break;
		case 3:		R.ALU.d = (R.Preg.d >> 6); if (R.Preg.d & 0x80000000) R.ALU.d |= 0xfc000000; break;
		default:	break;
	}
}

INLINE void GETDATA(int shift,int signext)
{
	if (R.opcode.b.l & 0x80)
	{ /* indirect memory access */
		memaccess = IND;
	}
	else
	{ /* direct memory address */
		memaccess = DMA;
	}

	if (memaccess >= 0x800)
	{
		R.external_mem_access = 1;	/* Pause if hold pin is active */
	}
	else
	{
		R.external_mem_access = 0;
	}

	R.ALU.d = (UINT16)M_RDRAM(memaccess);
	if (signext) R.ALU.d = (INT16)R.ALU.d;
	R.ALU.d <<= shift;

	/* next ARP */
	if (R.opcode.b.l & 0x80) MODIFY_AR_ARP();
}

INLINE void PUTDATA(UINT16 data)
{
	if (R.opcode.b.l & 0x80) {
		if (memaccess >= 0x800) R.external_mem_access = 1;	/* Pause if hold pin is active */
		else R.external_mem_access = 0;

		M_WRTRAM(IND,data);
		MODIFY_AR_ARP();
	}
	else {
		if (memaccess >= 0x800) R.external_mem_access = 1;	/* Pause if hold pin is active */
		else R.external_mem_access = 0;

		M_WRTRAM(DMA,data);
	}
}
INLINE void PUTDATA_SST(UINT16 data)
{
	if (R.opcode.b.l & 0x80) memaccess = IND;
	else memaccess = DMApg0;

	if (memaccess >= 0x800) R.external_mem_access = 1;		/* Pause if hold pin is active */
	else R.external_mem_access = 0;

	if (R.opcode.b.l & 0x80) {
		R.opcode.b.l &= 0xf7;					/* Stop ARP changes */
		MODIFY_AR_ARP();
	}
	M_WRTRAM(memaccess,data);
}


/* The following functions are here to fill the void for the */
/* opcode call functions. These functions are never actually called. */
static void opcodes_CE(void) { }
static void opcodes_DX(void) { }

static void illegal(void)
{
		logerror("TMS32025:  PC = %04x,  Illegal opcode = %04x\n", (R.PC-1), R.opcode.w.l);
}

static void abst(void)
{
		if ( (INT32)(R.ACC.d) < 0 ) {
			R.ACC.d = -R.ACC.d;
			if (R.ACC.d == 0x80000000)
			{
				SET0(OV_FLAG);
				if (OVM) R.ACC.d-- ;
			}
		}
		CLR1(C_FLAG);
}
static void add(void)		/* #### add carry support - see page 3-31 (70) #### */
{							/* page 10-13 (348) spru031d */
		oldacc.d = R.ACC.d;
		GETDATA((R.opcode.b.h & 0xf),SXM);
		R.ACC.d += R.ALU.d;

		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void addc(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		if (CARRY) R.ALU.d++;
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void addh(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.ACC.w.h += R.ALU.w.l;
		if ((INT16)(~(oldacc.w.h ^ R.ALU.w.l) & (oldacc.w.h ^ R.ACC.w.h)) < 0) {
			SET0(OV_FLAG);
			if (OVM)
				R.ACC.w.h = ((INT16)oldacc.w.h < 0) ? 0x8000 : 0x7fff;
		}
		if ( ((INT16)(oldacc.w.h) < 0) && ((INT16)(R.ACC.w.h) >= 0) ) {
			SET1(C_FLAG);
		}
		/* Carry flag is not cleared, if no carry occured */
}
static void addk(void)
{
		oldacc.d = R.ACC.d;
		R.ALU.d = (UINT8)R.opcode.b.l;
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void adds(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void addt(void)
{
		oldacc.d = R.ACC.d;
		GETDATA((R.Treg & 0xf),SXM);
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void adlk(void)
{
		oldacc.d = R.ACC.d;
		if (SXM) R.ALU.d =  (INT16)M_RDOP_ARG(R.PC);
		else     R.ALU.d = (UINT16)M_RDOP_ARG(R.PC);
		R.PC++;
		R.ALU.d <<= (R.opcode.b.h & 0xf);
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void adrk(void)
{
		R.AR[ARP] += R.opcode.b.l;
}
static void and(void)
{
		GETDATA(0,0);
		R.ACC.d &= R.ALU.d;
}
static void andk(void)
{
		oldacc.d = R.ACC.d;
		R.ALU.d = (UINT16)M_RDOP_ARG(R.PC);
		R.PC++;
		R.ALU.d <<= (R.opcode.b.h & 0xf);
		R.ACC.d &= R.ALU.d;
		R.ACC.d &= 0x7fffffff;
}
static void apac(void)
{
		oldacc.d = R.ACC.d;
		SHIFT_Preg_TO_ALU();
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void br(void)
{
		SET_PC(M_RDOP_ARG(R.PC));
		MODIFY_AR_ARP();
}
static void bacc(void)
{
		SET_PC(R.ACC.w.l);
}
static void banz(void)
{
		if (R.AR[ARP]) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bbnz(void)
{
		if (TC) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bbz(void)
{
		if (TC == 0) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bc(void)
{
		if (CARRY) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bgez(void)
{
		if ( (INT32)(R.ACC.d) >= 0 ) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bgz(void)
{
		if ( (INT32)(R.ACC.d) > 0 ) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bioz(void)
{
		if (S_IN(TMS32025_BIO) != CLEAR_LINE) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bit(void)
{
		GETDATA(0,0);
		if (R.ALU.d & (0x8000 >> (R.opcode.b.h & 0xf))) SET1(TC_FLAG);
		else CLR1(TC_FLAG);
}
static void bitt(void)
{
		GETDATA(0,0);
		if (R.ALU.d & (0x8000 >> (R.Treg & 0xf))) SET1(TC_FLAG);
		else CLR1(TC_FLAG);
}
static void blez(void)
{
		if ( (INT32)(R.ACC.d) <= 0 ) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void blkd(void)
{										/** Fix cycle timing **/
		if (R.init_load_addr) {
			R.PFC = M_RDOP_ARG(R.PC);
			R.PC++;
		}
		R.ALU.d = M_RDRAM(R.PFC);
		PUTDATA(R.ALU.d);
		R.PFC++;
		R.tms32025_dec_cycles += (1*CLK);
}
static void blkp(void)
{										/** Fix cycle timing **/
		if (R.init_load_addr) {
			R.PFC = M_RDOP_ARG(R.PC);
			R.PC++;
		}
		R.ALU.d = M_RDROM(R.PFC);
		PUTDATA(R.ALU.d);
		R.PFC++;
		R.tms32025_dec_cycles += (2*CLK);
}
static void blz(void)
{
		if ( (INT32)(R.ACC.d) <  0 ) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bnc(void)
{
		if (CARRY == 0) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bnv(void)
{
		if (OV == 0) SET_PC(M_RDOP_ARG(R.PC));
		else {
			R.PC++ ;
			CLR0(OV_FLAG);
		}
		MODIFY_AR_ARP();
}
static void bnz(void)
{
		if (R.ACC.d != 0) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bv(void)
{
		if (OV) {
			SET_PC(M_RDOP_ARG(R.PC));
			CLR0(OV_FLAG);
		}
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void bz(void)
{
		if (R.ACC.d == 0) SET_PC(M_RDOP_ARG(R.PC));
		else R.PC++ ;
		MODIFY_AR_ARP();
}
static void cala(void)
{
		PUSH_STACK(R.PC);
		SET_PC(R.ACC.w.l);
}
static void call(void)
{
		R.PC++ ;
		PUSH_STACK(R.PC);
		SET_PC(M_RDOP_ARG((R.PC - 1)));
		MODIFY_AR_ARP();
}
static void cmpl(void)
{
		R.ACC.d = (~R.ACC.d);
}
static void cmpr(void)
{
		switch (R.opcode.b.l & 3)
		{
			case 00:	if ( (UINT16)(R.AR[ARP]) == (UINT16)(R.AR[0]) ) SET1(TC_FLAG); else CLR1(TC_FLAG); break;
			case 01:	if ( (UINT16)(R.AR[ARP]) <  (UINT16)(R.AR[0]) ) SET1(TC_FLAG); else CLR1(TC_FLAG); break;
			case 02:	if ( (UINT16)(R.AR[ARP])  > (UINT16)(R.AR[0]) ) SET1(TC_FLAG); else CLR1(TC_FLAG); break;
			case 03:	if ( (UINT16)(R.AR[ARP]) != (UINT16)(R.AR[0]) ) SET1(TC_FLAG); else CLR1(TC_FLAG); break;
			default:	break;
		}
}
static void cnfd(void)	/** next two fetches need to use previous CNF value ! **/
{
		CLR1(CNF0_REG);
		tms32025_datamap[4] = &R.intRAM[0x200];			/* B0 */
		tms32025_datamap[5] = &R.intRAM[0x280];			/* B0 */
		tms32025_pgmmap[510] = NULL;
		tms32025_pgmmap[511] = NULL;
}
static void cnfp(void)	/** next two fetches need to use previous CNF value ! **/
{
		SET1(CNF0_REG);
		tms32025_datamap[4] = NULL;						/* B0 */
		tms32025_datamap[5] = NULL;						/* B0 */
		tms32025_pgmmap[510] = &R.intRAM[0x200];
		tms32025_pgmmap[511] = &R.intRAM[0x280];
}
static void conf(void)	/** Need to reconfigure the memory blocks */
{
		switch (R.opcode.b.l & 3)
		{
			case 00:	CLR1(CNF1_REG); CLR1(CNF0_REG);
						tms32025_datamap[4] = &R.intRAM[0x200];	/* B0 */
						tms32025_datamap[5] = &R.intRAM[0x280];	/* B0 */
						tms32025_datamap[6] = &R.intRAM[0x300];	/* B0 */
						tms32025_datamap[7] = &R.intRAM[0x380];	/* B0 */
						tms32025_datamap[8] = &R.intRAM[0x400];	/* B1 */
						tms32025_datamap[9] = &R.intRAM[0x480];	/* B1 */
						tms32025_datamap[10] = &R.intRAM[0x500];	/* B1 */
						tms32025_datamap[11] = &R.intRAM[0x580];	/* B1 */
						tms32025_datamap[12] = &R.intRAM[0x600];	/* B3 */
						tms32025_datamap[13] = &R.intRAM[0x680];	/* B3 */
						tms32025_datamap[14] = &R.intRAM[0x700];	/* B3 */
						tms32025_datamap[15] = &R.intRAM[0x780];	/* B3 */
						tms32025_pgmmap[500] = NULL;
						tms32025_pgmmap[501] = NULL;
						tms32025_pgmmap[502] = NULL;
						tms32025_pgmmap[503] = NULL;
						tms32025_pgmmap[504] = NULL;
						tms32025_pgmmap[505] = NULL;
						tms32025_pgmmap[506] = NULL;
						tms32025_pgmmap[507] = NULL;
						tms32025_pgmmap[508] = NULL;
						tms32025_pgmmap[509] = NULL;
						tms32025_pgmmap[510] = NULL;
						tms32025_pgmmap[511] = NULL;
						break;
			case 01:	CLR1(CNF1_REG); SET1(CNF0_REG);
						tms32025_datamap[4] = NULL;
						tms32025_datamap[5] = NULL;
						tms32025_datamap[6] = NULL;
						tms32025_datamap[7] = NULL;
						tms32025_datamap[8] = &R.intRAM[0x400];	/* B1 */
						tms32025_datamap[9] = &R.intRAM[0x480];	/* B1 */
						tms32025_datamap[10] = &R.intRAM[0x500];	/* B1 */
						tms32025_datamap[11] = &R.intRAM[0x580];	/* B1 */
						tms32025_datamap[12] = &R.intRAM[0x600];	/* B3 */
						tms32025_datamap[13] = &R.intRAM[0x680];	/* B3 */
						tms32025_datamap[14] = &R.intRAM[0x700];	/* B3 */
						tms32025_datamap[15] = &R.intRAM[0x780];	/* B3 */
						tms32025_pgmmap[500] = &R.intRAM[0x200];	/* B0 */
						tms32025_pgmmap[501] = &R.intRAM[0x280];	/* B0 */
						tms32025_pgmmap[502] = &R.intRAM[0x300];	/* B0 */
						tms32025_pgmmap[503] = &R.intRAM[0x380];	/* B0 */
						tms32025_pgmmap[504] = NULL;
						tms32025_pgmmap[505] = NULL;
						tms32025_pgmmap[506] = NULL;
						tms32025_pgmmap[507] = NULL;
						tms32025_pgmmap[508] = NULL;
						tms32025_pgmmap[509] = NULL;
						tms32025_pgmmap[510] = NULL;
						tms32025_pgmmap[511] = NULL;
						break;
			case 02:	SET1(CNF1_REG); CLR1(CNF0_REG);
						tms32025_datamap[4] = NULL;
						tms32025_datamap[5] = NULL;
						tms32025_datamap[6] = NULL;
						tms32025_datamap[7] = NULL;
						tms32025_datamap[8] = NULL;
						tms32025_datamap[9] = NULL;
						tms32025_datamap[10] = NULL;
						tms32025_datamap[11] = NULL;
						tms32025_datamap[12] = &R.intRAM[0x600];	/* B3 */
						tms32025_datamap[13] = &R.intRAM[0x680];	/* B3 */
						tms32025_datamap[14] = &R.intRAM[0x700];	/* B3 */
						tms32025_datamap[15] = &R.intRAM[0x780];	/* B3 */
						tms32025_pgmmap[500] = &R.intRAM[0x200];	/* B0 */
						tms32025_pgmmap[501] = &R.intRAM[0x280];	/* B0 */
						tms32025_pgmmap[502] = &R.intRAM[0x300];	/* B0 */
						tms32025_pgmmap[503] = &R.intRAM[0x380];	/* B0 */
						tms32025_pgmmap[504] = &R.intRAM[0x400];	/* B1 */
						tms32025_pgmmap[505] = &R.intRAM[0x480];	/* B1 */
						tms32025_pgmmap[506] = &R.intRAM[0x500];	/* B1 */
						tms32025_pgmmap[507] = &R.intRAM[0x580];	/* B1 */
						tms32025_pgmmap[508] = NULL;
						tms32025_pgmmap[509] = NULL;
						tms32025_pgmmap[510] = NULL;
						tms32025_pgmmap[511] = NULL;
						break;
			case 03:	SET1(CNF1_REG); SET1(CNF0_REG);
						tms32025_datamap[4] = NULL;
						tms32025_datamap[5] = NULL;
						tms32025_datamap[6] = NULL;
						tms32025_datamap[7] = NULL;
						tms32025_datamap[8] = NULL;
						tms32025_datamap[9] = NULL;
						tms32025_datamap[10] = NULL;
						tms32025_datamap[11] = NULL;
						tms32025_datamap[12] = NULL;
						tms32025_datamap[13] = NULL;
						tms32025_datamap[14] = NULL;
						tms32025_datamap[15] = NULL;
						tms32025_pgmmap[500] = &R.intRAM[0x200];	/* B0 */
						tms32025_pgmmap[501] = &R.intRAM[0x280];	/* B0 */
						tms32025_pgmmap[502] = &R.intRAM[0x300];	/* B0 */
						tms32025_pgmmap[503] = &R.intRAM[0x380];	/* B0 */
						tms32025_pgmmap[504] = &R.intRAM[0x400];	/* B1 */
						tms32025_pgmmap[505] = &R.intRAM[0x480];	/* B1 */
						tms32025_pgmmap[506] = &R.intRAM[0x500];	/* B1 */
						tms32025_pgmmap[507] = &R.intRAM[0x580];	/* B1 */
						tms32025_pgmmap[508] = &R.intRAM[0x600];	/* B3 */
						tms32025_pgmmap[509] = &R.intRAM[0x680];	/* B3 */
						tms32025_pgmmap[510] = &R.intRAM[0x700];	/* B3 */
						tms32025_pgmmap[511] = &R.intRAM[0x780];	/* B3 */
						break;
			default:	break;
		}
}
static void dint(void)
{
		SET0(INTM_FLAG);
}
static void dmov(void)	/** Careful with how memory is configured !! */
{
		GETDATA(0,0);
		M_WRTRAM((memaccess + 1),R.ALU.w.l);
}
static void eint(void)
{
		CLR0(INTM_FLAG);
}
static void fort(void)
{
		if (R.opcode.b.l & 1) SET1(FO_FLAG);
		else CLR1(FO_FLAG);
}
static void idle(void)
{
		CLR0(INTM_FLAG);
		R.idle = 1;
}
static void in(void)
{
		R.ALU.w.l = P_IN( (R.opcode.b.h & 0xf) );
		PUTDATA(R.ALU.w.l);
}
static void lac(void)
{
		GETDATA( (R.opcode.b.h & 0xf),SXM );
		R.ACC.d = R.ALU.d;
}
static void lack(void)		/* ZAC is a subset of this instruction */
{
		R.ACC.d = (UINT8)R.opcode.b.l;
}
static void lact(void)
{
		GETDATA( (R.Treg & 0xf),SXM );
		R.ACC.d = R.ALU.d;
}
static void lalk(void)
{
		if (SXM) {
			R.ALU.d = (INT16)M_RDOP_ARG(R.PC);
			R.ACC.d = R.ALU.d << (R.opcode.b.h & 0xf);
		}
		else {
			R.ALU.d = (UINT16)M_RDOP_ARG(R.PC);
			R.ACC.d = R.ALU.d << (R.opcode.b.h & 0xf);
			R.ACC.d &= 0x7fffffff;
		}
		R.PC++;
}
static void lar_ar0(void)	{ GETDATA(0,0); R.AR[0] = R.ALU.w.l; }
static void lar_ar1(void)	{ GETDATA(0,0); R.AR[1] = R.ALU.w.l; }
static void lar_ar2(void)	{ GETDATA(0,0); R.AR[2] = R.ALU.w.l; }
static void lar_ar3(void)	{ GETDATA(0,0); R.AR[3] = R.ALU.w.l; }
static void lar_ar4(void)	{ GETDATA(0,0); R.AR[4] = R.ALU.w.l; }
static void lar_ar5(void)	{ GETDATA(0,0); R.AR[5] = R.ALU.w.l; }
static void lar_ar6(void)	{ GETDATA(0,0); R.AR[6] = R.ALU.w.l; }
static void lar_ar7(void)	{ GETDATA(0,0); R.AR[7] = R.ALU.w.l; }
static void lark_ar0(void)	{ R.AR[0] = R.opcode.b.l; }
static void lark_ar1(void)	{ R.AR[1] = R.opcode.b.l; }
static void lark_ar2(void)	{ R.AR[2] = R.opcode.b.l; }
static void lark_ar3(void)	{ R.AR[3] = R.opcode.b.l; }
static void lark_ar4(void)	{ R.AR[4] = R.opcode.b.l; }
static void lark_ar5(void)	{ R.AR[5] = R.opcode.b.l; }
static void lark_ar6(void)	{ R.AR[6] = R.opcode.b.l; }
static void lark_ar7(void)	{ R.AR[7] = R.opcode.b.l; }
static void ldp(void)
{
		GETDATA(0,0);
		MODIFY_DP(R.ALU.d & 0x1ff);
}
static void ldpk(void)
{
		MODIFY_DP(R.opcode.w.l & 0x1ff);
}
static void lph(void)
{
		GETDATA(0,0);
		R.Preg.w.h = R.ALU.w.l;
}
static void lrlk(void)
{
		R.ALU.d = (UINT16)M_RDOP_ARG(R.PC);
		R.PC++;
		R.AR[R.opcode.b.h & 7] = R.ALU.w.l;
}
static void lst(void)
{
		mHackIgnoreARP = 1;
		GETDATA(0,0);
		mHackIgnoreARP = 0;

		R.ALU.w.l &= (~INTM_FLAG);
		R.STR0 &= INTM_FLAG;
		R.STR0 |= R.ALU.w.l;		/* Must not affect INTM */
		R.STR0 |= 0x0400;
}
static void lst1(void)
{
		mHackIgnoreARP = 1;
		GETDATA(0,0);
		mHackIgnoreARP = 0;

		R.STR1 = R.ALU.w.l;
		R.STR1 |= 0x0180;
		R.STR0 &= (~ARP_REG);		/* ARB also gets copied to ARP */
		R.STR0 |= (R.STR1 & ARB_REG);
}
static void lt(void)
{
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
}
static void lta(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
		SHIFT_Preg_TO_ALU();
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void ltd(void)	/** Careful with how memory is configured !! */
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
		M_WRTRAM((memaccess+1),R.ALU.w.l);
		SHIFT_Preg_TO_ALU();
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
}
static void ltp(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
		SHIFT_Preg_TO_ALU();
		R.ACC.d = R.ALU.d;
}
static void lts(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
		SHIFT_Preg_TO_ALU();
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}
static void mac(void)			/** RAM blocks B0,B1,B2 may be important ! */
{								/** Fix cycle timing **/
		oldacc.d = R.ACC.d;
		if (R.init_load_addr) {
			R.PFC = M_RDOP_ARG(R.PC);
			R.PC++;
		}
		SHIFT_Preg_TO_ALU();
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
		R.Preg.d = ( (INT16)R.ALU.w.l * (INT16)M_RDROM(R.PFC) );
		R.PFC++;
		R.tms32025_dec_cycles += (2*CLK);
}
static void macd(void)			/** RAM blocks B0,B1,B2 may be important ! */
{								/** Fix cycle timing **/
		oldacc.d = R.ACC.d;
		if (R.init_load_addr) {
			R.PFC = M_RDOP_ARG(R.PC);
			R.PC++;
		}
		SHIFT_Preg_TO_ALU();
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
		GETDATA(0,0);
		if ( (R.opcode.b.l & 0x80) || R.init_load_addr ) {	/* No writing during repitition, or DMA mode */
			M_WRTRAM((memaccess+1),R.ALU.w.l);
		}
		R.Treg = R.ALU.w.l;
		R.Preg.d = ( (INT16)R.ALU.w.l * (INT16)M_RDROM(R.PFC) );
		R.PFC++;
		R.tms32025_dec_cycles += (2*CLK);
}
static void mar(void)		/* LARP and NOP are a subset of this instruction */
{
		if (R.opcode.b.l & 0x80) MODIFY_AR_ARP();
}
static void mpy(void)
{
		GETDATA(0,0);
		R.Preg.d = (INT16)(R.ALU.w.l) * (INT16)(R.Treg);
}
static void mpya(void)
{
		oldacc.d = R.ACC.d;
		SHIFT_Preg_TO_ALU();
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
		GETDATA(0,0);
		R.Preg.d = (INT16)(R.ALU.w.l) * (INT16)(R.Treg);
}
static void mpyk(void)
{
		R.Preg.d = (INT16)R.Treg * ((INT16)(R.opcode.w.l << 3) >> 3);

}
static void mpys(void)
{
		oldacc.d = R.ACC.d;
		SHIFT_Preg_TO_ALU();
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
		GETDATA(0,0);
		R.Preg.d = (INT16)(R.ALU.w.l) * (INT16)(R.Treg);
}
static void mpyu(void)
{
		GETDATA(0,0); R.Preg.d = (UINT16)(R.ALU.w.l) * (UINT16)(R.Treg);
}
static void neg(void)
{
		if (R.ACC.d == 0x80000000) {
			SET0(OV_FLAG);
			if (OVM) R.ACC.d = 0x7fffffff;
		}
		else R.ACC.d = -R.ACC.d;
		if (R.ACC.d) CLR0(C_FLAG);
		else SET0(C_FLAG);
}
/*
static void nop(void) { }   // NOP is a subset of the MAR instruction
*/
static void norm(void)
{
	UINT32 acc = R.ACC.d;

	if( acc == 0 || ((acc^(acc<<1))&(1<<31))!=0 )
	{
		SET1(TC_FLAG); /* 1 -> TC */
	}
	else
	{
		CLR1(TC_FLAG); /* 0 -> TC */
		R.ACC.d <<= 1; /* (ACC)*2 -> ACC */
		MODIFY_AR_ARP();
	}
}
static void or(void)
{
		GETDATA(0,0);
		R.ACC.w.l |= R.ALU.w.l;
}
static void ork(void)
{
		R.ALU.d = (UINT16)M_RDOP_ARG(R.PC);
		R.PC++;
		R.ALU.d <<= (R.opcode.b.h & 0xf);
		R.ACC.d |=  (R.ALU.d & 0x7fffffff);
}
static void out(void)
{
		GETDATA(0,0);
		P_OUT( (R.opcode.b.h & 0xf), R.ALU.w.l );
}
static void pac(void)
{
		SHIFT_Preg_TO_ALU();
		R.ACC.d = R.ALU.d;
}
static void pop(void)
{
		R.ACC.d = (UINT16)POP_STACK();
}
static void popd(void)
{
		R.ALU.d = (UINT16)POP_STACK();
		PUTDATA(R.ALU.w.l);
}
static void pshd(void)
{
		GETDATA(0,0);
		PUSH_STACK(R.ALU.w.l);
}
static void push(void)
{
		PUSH_STACK(R.ACC.w.l);
}
static void rc(void)
{
		CLR1(C_FLAG);
}
static void ret(void)
{
		SET_PC(POP_STACK());
}
static void rfsm(void)				/** serial port mode */
{
		CLR1(FSM_FLAG);
}
static void rhm(void)
{
		CLR1(HM_FLAG);
}
static void rol(void)
{
		R.ALU.d = R.ACC.d;
		R.ACC.d <<= 1;
		if (CARRY) R.ACC.d |= 1;
		if (R.ALU.d & 0x80000000) SET1(C_FLAG);
		else CLR1(C_FLAG);
}
static void ror(void)
{
		R.ALU.d = R.ACC.d;
		R.ACC.d >>= 1;
		if (CARRY) R.ACC.d |= 0x80000000;
		if (R.ALU.d & 1) SET1(C_FLAG);
		else CLR1(C_FLAG);
}
static void rovm(void)
{
		CLR0(OVM_FLAG);
}
static void rpt(void)
{
		GETDATA(0,0);
		R.RPTC = R.ALU.b.l;
		R.init_load_addr = 2;		/* Initiate repeat mode */
}
static void rptk(void)
{
		R.RPTC = R.opcode.b.l;
		R.init_load_addr = 2;		/* Initiate repeat mode */
}
static void rsxm(void)
{
		CLR1(SXM_FLAG);
}
static void rtc(void)
{
		CLR1(TC_FLAG);
}
static void rtxm(void)				/** serial port stuff */
{
		CLR1(TXM_FLAG);
}
static void rxf(void)
{
		CLR1(XF_FLAG);
		S_OUT(TMS32025_XF,CLEAR_LINE);
}
static void sach(void)
{
		R.ALU.d = (R.ACC.d << (R.opcode.b.h & 7));
		PUTDATA(R.ALU.w.h);
}
static void sacl(void)
{
		R.ALU.d = (R.ACC.d << (R.opcode.b.h & 7));
		PUTDATA(R.ALU.w.l);
}
static void sar_ar0(void)	{ PUTDATA(R.AR[0]); }
static void sar_ar1(void)	{ PUTDATA(R.AR[1]); }
static void sar_ar2(void)	{ PUTDATA(R.AR[2]); }
static void sar_ar3(void)	{ PUTDATA(R.AR[3]); }
static void sar_ar4(void)	{ PUTDATA(R.AR[4]); }
static void sar_ar5(void)	{ PUTDATA(R.AR[5]); }
static void sar_ar6(void)	{ PUTDATA(R.AR[6]); }
static void sar_ar7(void)	{ PUTDATA(R.AR[7]); }

static void sblk(void)
{
		oldacc.d = R.ACC.d;
		if (SXM) R.ALU.d =  (INT16)M_RDOP_ARG(R.PC);
		else     R.ALU.d = (UINT16)M_RDOP_ARG(R.PC);
		R.PC++;
		R.ALU.d <<= (R.opcode.b.h & 0xf);
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}
static void tms_sbrk(void)
{
		R.AR[ARP] -= R.opcode.b.l;
}
static void sc(void)
{
		SET1(C_FLAG);
}
static void sfl(void)
{
		R.ALU.d = R.ACC.d;
		R.ACC.d <<= 1;
		if (R.ALU.d & 0x80000000) SET1(C_FLAG);
		else CLR1(C_FLAG);
}
static void sfr(void)
{
		R.ALU.d = R.ACC.d;
		R.ACC.d >>= 1;
		if (SXM) {
			if (R.ALU.d & 0x80000000) R.ACC.d |= 0x80000000;
		}
		if (R.ALU.d & 1) SET1(C_FLAG);
		else CLR1(C_FLAG);
}
static void sfsm(void)				/** serial port mode */
{
		SET1(FSM_FLAG);
}
static void shm(void)
{
		SET1(HM_FLAG);
}
static void sovm(void)
{
		SET0(OVM_FLAG);
}
static void spac(void)
{
		oldacc.d = R.ACC.d;
		SHIFT_Preg_TO_ALU();
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}
static void sph(void)
{
		SHIFT_Preg_TO_ALU();
		PUTDATA(R.ALU.w.h);
}
static void spl(void)
{
		SHIFT_Preg_TO_ALU();
		PUTDATA(R.ALU.w.l);
}
static void spm(void)
{
		MODIFY_PM( (R.opcode.b.l & 3) );
}
static void sqra(void)
{
		oldacc.d = R.ACC.d;
		SHIFT_Preg_TO_ALU();
		R.ACC.d += R.ALU.d;
		CALCULATE_ADD_OVERFLOW(R.ALU.d);
		CALCULATE_ADD_CARRY();
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
		R.Preg.d = ((INT16)R.ALU.w.l * (INT16)R.ALU.w.l);
}
static void sqrs(void)
{
		oldacc.d = R.ACC.d;
		SHIFT_Preg_TO_ALU();
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
		GETDATA(0,0);
		R.Treg = R.ALU.w.l;
		R.Preg.d = ((INT16)R.ALU.w.l * (INT16)R.ALU.w.l);
}
static void sst(void)
{
		PUTDATA_SST(R.STR0);
}
static void sst1(void)
{
		PUTDATA_SST(R.STR1);
}
static void ssxm(void)
{		/** Check instruction description, and make sure right instructions use SXM */
		SET1(SXM_FLAG);
}
static void stc(void)
{
		SET1(TC_FLAG);
}
static void stxm(void)				/** serial port stuff */
{
		SET1(TXM_FLAG);
}
static void sub(void)
{
		oldacc.d = R.ACC.d;
		GETDATA((R.opcode.b.h & 0xf),SXM);
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}
static void subb(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		if (CARRY == 0) R.ALU.d--;
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}


static void subc(void)
{
	/**
     * conditional subtraction, which may be used for division
     * execute 16 times for 16-bit division
     *
     * input:   32 bit numerator in accumulator
     *          16 bit denominator in data memory
     *
     * output:  remainder in upper 16 bits
     *          quotient in lower 16 bits
     */
	GETDATA(15,SXM);
	if( R.ACC.d >= R.ALU.d )
	{
		R.ACC.d = (R.ACC.d - R.ALU.d)*2+1;
	}
	else
	{
		R.ACC.d = R.ACC.d*2;
	}
// Stroff: HACK! support for overflow capping as implemented results in bad DSP floating point math in many
// System22 games - for example, the score display in Prop Cycle.
//  R.ACC.d = ((INT32)oldacc.d < 0) ? 0x80000000 : 0x7fffffff;

//      if ((INT32)((oldacc.d ^ subval ) & (oldacc.d ^ R.ALU.d)) < 0)
//      {
//          SET0(OV_FLAG);
//      }
//      CALCULATE_SUB_CARRY();
}

static void subh(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.ACC.w.h -= R.ALU.w.l;
		if ((INT16)((oldacc.w.h ^ R.ALU.w.l) & (oldacc.w.h ^ R.ACC.w.h)) < 0) {
			SET0(OV_FLAG);
			if (OVM)
				R.ACC.w.h = ((INT16)oldacc.w.h < 0) ? 0x8000 : 0x7fff;
		}
		if ( ((INT16)(oldacc.w.h) >= 0) && ((INT16)(R.ACC.w.h) < 0) ) {
			CLR1(C_FLAG);
		}
		/* Carry flag is not affected, if no borrow occured */
}
static void subk(void)
{
		oldacc.d = R.ACC.d;
		R.ALU.d = (UINT8)R.opcode.b.l;
		R.ACC.d -= R.ALU.b.l;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}
static void subs(void)
{
		oldacc.d = R.ACC.d;
		GETDATA(0,0);
		R.ACC.d -= R.ALU.w.l;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}
static void subt(void)
{
		oldacc.d = R.ACC.d;
		GETDATA((R.Treg & 0xf),SXM);
		R.ACC.d -= R.ALU.d;
		CALCULATE_SUB_OVERFLOW(R.ALU.d);
		CALCULATE_SUB_CARRY();
}
static void sxf(void)
{
		SET1(XF_FLAG);
		S_OUT(TMS32025_XF,ASSERT_LINE);
}
static void tblr(void)
{
		if (R.init_load_addr)
		{
			R.PFC = R.ACC.w.l;
		}
		R.ALU.w.l = M_RDROM(R.PFC);
		if ( (CNF0) && ( (UINT16)(R.PFC) >= 0xff00 ) ) {}	/** TMS32025 only */
		else R.tms32025_dec_cycles += (1*CLK);
		PUTDATA(R.ALU.w.l);
		R.PFC++;
}
static void tblw(void)
{
		if (R.init_load_addr)
		{
			R.PFC = R.ACC.w.l;
		}
		R.tms32025_dec_cycles += (1*CLK);
		GETDATA(0,0);
		if (R.external_mem_access) R.tms32025_dec_cycles += (1*CLK);
		M_WRTROM(R.PFC, R.ALU.w.l);
		R.PFC++;
}
static void trap(void)
{
		PUSH_STACK(R.PC);
		SET_PC(0x001E);		/* Trap vector */
}
static void xor(void)
{
		GETDATA(0,0);
		R.ACC.w.l ^= R.ALU.w.l;
}
static void xork(void)
{
		oldacc.d = R.ACC.d;
		R.ALU.d = M_RDOP_ARG(R.PC);
		R.PC++;
		R.ALU.d <<= (R.opcode.b.h & 0xf);
		R.ACC.d ^= R.ALU.d;
		R.ACC.d |= (oldacc.d & 0x80000000);
}
static void zalh(void)
{
		GETDATA(0,0);
		R.ACC.w.h = R.ALU.w.l;
		R.ACC.w.l = 0x0000;
}
static void zalr(void)
{
		GETDATA(0,0);
		R.ACC.w.h = R.ALU.w.l;
		R.ACC.w.l = 0x8000;
}
static void zals(void)
{
		GETDATA(0,0);
		R.ACC.w.l = R.ALU.w.l;
		R.ACC.w.h = 0x0000;
}


/***********************************************************************
 *  Cycle Timings
 ***********************************************************************/

static const unsigned cycles_main[256]=
{
/*00*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*08*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*10*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*18*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*20*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*28*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*30*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*38*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*40*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*48*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*50*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*58*/		3*CLK, 2*CLK, 1*CLK, 1*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*60*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*68*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*70*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*78*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*80*/		2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*88*/		2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*90*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*98*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*A0*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*A8*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*B0*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*B8*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*C0*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*C8*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*D0*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 0*CLK,
/*D8*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*E0*/		2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*E8*/		2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*F0*/		2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*F8*/		2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK
};

static const unsigned cycles_DX_subset[8]=
{
/*00*/		2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 0
};

static const unsigned cycles_CE_subset[256]=
{
/*00*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*08*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*10*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 0*CLK,
/*18*/		1*CLK, 1*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 2*CLK, 3*CLK,
/*20*/		1*CLK, 1*CLK, 0*CLK, 1*CLK, 2*CLK, 2*CLK, 2*CLK, 1*CLK,
/*28*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*30*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*38*/		1*CLK, 1*CLK, 0*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*40*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*48*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*50*/		1*CLK, 1*CLK, 1*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*58*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*60*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*68*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*70*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*78*/		0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*80*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*88*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*90*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*98*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*A0*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*A8*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*B0*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*B8*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*C0*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*C8*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*D0*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*D8*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*E0*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*E8*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*F0*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*F8*/		0*CLK, 0*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK
};


/***********************************************************************
 *  Opcode Table
 ***********************************************************************/

static const opcode_fn opcode_main[256]=
{
/*00*/ add,			add,		add,		add,		add,		add,		add,		add,
/*08*/ add,			add,		add,		add,		add,		add,		add,		add,
/*10*/ sub,			sub,		sub,		sub,		sub,		sub,		sub,		sub,
/*18*/ sub,			sub,		sub,		sub,		sub,		sub,		sub,		sub,
/*20*/ lac,			lac,		lac,		lac,		lac,		lac,		lac,		lac,
/*28*/ lac,			lac,		lac,		lac,		lac,		lac,		lac,		lac,
/*30*/ lar_ar0,		lar_ar1,	lar_ar2,	lar_ar3,	lar_ar4,	lar_ar5,	lar_ar6,	lar_ar7,
/*38*/ mpy,			sqra,		mpya,		mpys,		lt,			lta,		ltp,		ltd,
/*40*/ zalh,		zals,		lact,		addc,		subh,		subs,		subt,		subc,
/*48*/ addh,		adds,		addt,		rpt,		xor,		or,			and,		subb,
/*50*/ lst,			lst1,		ldp,		lph,		pshd,		mar,		dmov,		bitt,
/*58*/ tblr,		tblw,		sqrs,		lts,		macd,		mac,		bc,			bnc,
/*60*/ sacl,		sacl,		sacl,		sacl,		sacl,		sacl,		sacl,		sacl,
/*68*/ sach,		sach,		sach,		sach,		sach,		sach,		sach,		sach,
/*70*/ sar_ar0,		sar_ar1,	sar_ar2,	sar_ar3,	sar_ar4,	sar_ar5,	sar_ar6,	sar_ar7,
/*78*/ sst,			sst1,		popd,		zalr,		spl,		sph,		adrk,		tms_sbrk,
/*80*/ in,			in,			in,			in,			in,			in,			in,			in,
/*88*/ in,			in,			in,			in,			in,			in,			in,			in,
/*90*/ bit,			bit,		bit,		bit,		bit,		bit,		bit,		bit,
/*98*/ bit,			bit,		bit,		bit,		bit,		bit,		bit,		bit,
/*A0*/ mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,
/*A8*/ mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,
/*B0*/ mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,
/*B8*/ mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,		mpyk,
/*C0*/ lark_ar0,	lark_ar1,	lark_ar2,	lark_ar3,	lark_ar4,	lark_ar5,	lark_ar6,	lark_ar7,
/*C8*/ ldpk,		ldpk,		lack,		rptk,		addk,		subk,		opcodes_CE,	mpyu,
/*D0*/ opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,
/*D8*/ opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,	opcodes_DX,
/*E0*/ out,			out,		out,		out,		out,		out,		out,		out,
/*E8*/ out,			out,		out,		out,		out,		out,		out,		out,
/*F0*/ bv,			bgz,		blez,		blz,		bgez,		bnz,		bz,			bnv,
/*F8*/ bbz,			bbnz,		bioz,		banz,		blkp,		blkd,		call,		br
};

static const opcode_fn opcode_DX_subset[8]=	/* Instructions living under the Dxxx opcode */
{
/*00*/ lrlk,		lalk,		adlk,		sblk,		andk,		ork,		xork,		illegal
};

static const opcode_fn opcode_CE_subset[256]=
{
/*00*/ eint,		dint,		rovm,		sovm,		cnfd,		cnfp,		rsxm,		ssxm,
/*08*/ spm,			spm,		spm,		spm,		rxf,		sxf,		fort,		fort,
/*10*/ illegal,		illegal,	illegal,	illegal,	pac,		apac,		spac,		illegal,
/*18*/ sfl,			sfr,		illegal,	abst,		push,		pop,		trap,		idle,
/*20*/ rtxm,		stxm,		illegal,	neg,		cala,		bacc,		ret,		cmpl,
/*28*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*30*/ rc,			sc,			rtc,		stc,		rol,		ror,		rfsm,		sfsm,
/*38*/ rhm,			shm,		illegal,	illegal,	conf,		conf,		conf,		conf,
/*40*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*48*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*50*/ cmpr,		cmpr,		cmpr,		cmpr,		illegal,	illegal,	illegal,	illegal,
/*58*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*60*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*68*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*70*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*78*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*80*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*88*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*90*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*98*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*A0*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*A8*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*B0*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*B8*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*C0*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*C8*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*D0*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*D8*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*E0*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*E8*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal,
/*F0*/ illegal,		illegal,	norm,		illegal,	illegal,	illegal,	illegal,	illegal,
/*F8*/ illegal,		illegal,	illegal,	illegal,	illegal,	illegal,	illegal,	illegal
};



/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/
static CPU_INIT( tms32025 )
{
	R.intRAM = auto_malloc(0x800*2);
	R.irq_callback = irqcallback;
	R.device = device;

	state_save_register_item("tms32025", device->tag, 0, R.PC);
	state_save_register_item("tms32025", device->tag, 0, R.STR0);
	state_save_register_item("tms32025", device->tag, 0, R.STR1);
	state_save_register_item("tms32025", device->tag, 0, R.PFC);
	state_save_register_item("tms32025", device->tag, 0, R.IFR);
	state_save_register_item("tms32025", device->tag, 0, R.RPTC);
	state_save_register_item("tms32025", device->tag, 0, R.ACC.d);
	state_save_register_item("tms32025", device->tag, 0, R.ALU.d);
	state_save_register_item("tms32025", device->tag, 0, R.Preg.d);
	state_save_register_item("tms32025", device->tag, 0, R.Treg);
	state_save_register_item("tms32025", device->tag, 0, R.AR[0]);
	state_save_register_item("tms32025", device->tag, 0, R.AR[1]);
	state_save_register_item("tms32025", device->tag, 0, R.AR[2]);
	state_save_register_item("tms32025", device->tag, 0, R.AR[3]);
	state_save_register_item("tms32025", device->tag, 0, R.AR[4]);
	state_save_register_item("tms32025", device->tag, 0, R.AR[5]);
	state_save_register_item("tms32025", device->tag, 0, R.AR[6]);
	state_save_register_item("tms32025", device->tag, 0, R.AR[7]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[0]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[1]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[2]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[3]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[4]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[5]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[6]);
	state_save_register_item("tms32025", device->tag, 0, R.STACK[7]);

	state_save_register_item("tms32025", device->tag, 0, R.idle);
	state_save_register_item("tms32025", device->tag, 0, R.hold);
	state_save_register_item("tms32025", device->tag, 0, R.external_mem_access);
	state_save_register_item("tms32025", device->tag, 0, R.init_load_addr);
	state_save_register_item("tms32025", device->tag, 0, R.PREVPC);
}

/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/
static CPU_RESET( tms32025 )
{
	SET_PC(0);			/* Starting address on a reset */
	R.STR0 |= 0x0600;	/* INTM and unused bit set to 1 */
	R.STR0 &= 0xefff;	/* OV cleared to 0. Remaining bits undefined */
	R.STR1 |= 0x07f0;	/* SXM, C, HM, FSM, XF and unused bits set to 1 */
	R.STR1 &= 0xeff0;	/* CNF, FO, TXM, PM bits cleared to 0. Remaining bits undefined */
	R.RPTC = 0;			/* Reset repeat counter to 0 */
	R.IFR = 0;			/* IRQ pending flags */

	S_OUT(TMS32025_XF,ASSERT_LINE);	/* XF flag is high. Must set the pin */

	/* Set the internal memory mapped registers */
	GREG = 0;
	TIM  = 0xffff;
	PRD  = 0xffff;
	IMR  = 0xffc0;

	R.idle = 0;
	R.hold = 0;
	R.init_load_addr = 1;

	/* Reset the Data/Program address banks */
	memset(tms32025_pgmmap, 0, sizeof(tms32025_pgmmap));
	memset(tms32025_datamap, 0, sizeof(tms32025_datamap));

	tms32025_datamap[0] = &R.intRAM[0x000];			/* B2 */
	tms32025_datamap[4] = &R.intRAM[0x200];			/* B0 */
	tms32025_datamap[5] = &R.intRAM[0x280];			/* B0 */
	tms32025_datamap[6] = &R.intRAM[0x300];			/* B1 */
	tms32025_datamap[7] = &R.intRAM[0x380];			/* B1 */
}

#if (HAS_TMS32026)
static CPU_RESET( tms32026 )
{
	CPU_RESET_CALL(tms32025);

	/* Reset the Data/Program address banks */
	memset(tms32025_pgmmap, 0, sizeof(tms32025_pgmmap));
	memset(tms32025_datamap, 0, sizeof(tms32025_datamap));

	tms32025_datamap[0] = &R.intRAM[0x000];			/* B2 */
	tms32025_datamap[4] = &R.intRAM[0x200];			/* B0 */
	tms32025_datamap[5] = &R.intRAM[0x280];			/* B0 */
	tms32025_datamap[6] = &R.intRAM[0x300];			/* B0 */
	tms32025_datamap[7] = &R.intRAM[0x380];			/* B0 */
	tms32025_datamap[8] = &R.intRAM[0x400];			/* B1 */
	tms32025_datamap[9] = &R.intRAM[0x480];			/* B1 */
	tms32025_datamap[10] = &R.intRAM[0x500];			/* B1 */
	tms32025_datamap[11] = &R.intRAM[0x580];			/* B1 */
	tms32025_datamap[12] = &R.intRAM[0x600];			/* B3 */
	tms32025_datamap[13] = &R.intRAM[0x680];			/* B3 */
	tms32025_datamap[14] = &R.intRAM[0x700];			/* B3 */
	tms32025_datamap[15] = &R.intRAM[0x780];			/* B3 */
}
#endif


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/
static CPU_EXIT( tms32025 )
{
}


/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/
static int process_IRQs(void)
{
	/********** Interrupt Flag Register (IFR) **********
        |  5  |  4  |  3  |  2  |  1  |  0  |
        | XINT| RINT| TINT| INT2| INT1| INT0|
    */
	R.tms32025_irq_cycles = 0;

	/* Dont service Interrupts if masked, or prev instruction was EINT ! */

	if ( (INTM == 0) && (R.opcode.w.l != 0xce00) && (R.IFR & IMR) )
	{
		R.tms32025_irq_cycles = (3*CLK);	/* 3 clock cycles used due to PUSH and DINT operation ? */
		PUSH_STACK(R.PC);

		if ((R.IFR & 0x01) && (IMR & 0x01)) {		/* IRQ line 0 */
			//logerror("TMS32025:  Active INT0\n");
			SET_PC(0x0002);
			(*R.irq_callback)(R.device, 0);
			R.idle = 0;
			R.IFR &= (~0x01);
			SET0(INTM_FLAG);
			return R.tms32025_irq_cycles;
		}
		if ((R.IFR & 0x02) && (IMR & 0x02)) {		/* IRQ line 1 */
			//logerror("TMS32025:  Active INT1\n");
			SET_PC(0x0004);
			(*R.irq_callback)(R.device, 1);
			R.idle = 0;
			R.IFR &= (~0x02);
			SET0(INTM_FLAG);
			return R.tms32025_irq_cycles;
		}
		if ((R.IFR & 0x04) && (IMR & 0x04)) {		/* IRQ line 2 */
			//logerror("TMS32025:  Active INT2\n");
			SET_PC(0x0006);
			(*R.irq_callback)(R.device, 2);
			R.idle = 0;
			R.IFR &= (~0x04);
			SET0(INTM_FLAG);
			return R.tms32025_irq_cycles;
		}
		if ((R.IFR & 0x08) && (IMR & 0x08)) {		/* Timer IRQ (internal) */
//          logerror("TMS32025:  Active TINT (Timer)\n");
			SET_PC(0x0018);
			R.idle = 0;
			R.IFR &= (~0x08);
			SET0(INTM_FLAG);
			return R.tms32025_irq_cycles;
		}
		if ((R.IFR & 0x10) && (IMR & 0x10)) {		/* Serial port receive IRQ (internal) */
//          logerror("TMS32025:  Active RINT (Serial recieve)\n");
			DRR = S_IN(TMS32025_DR);
			SET_PC(0x001A);
			R.idle = 0;
			R.IFR &= (~0x10);
			SET0(INTM_FLAG);
			return R.tms32025_irq_cycles;
		}
		if ((R.IFR & 0x20) && (IMR & 0x20)) {		/* Serial port transmit IRQ (internal) */
//          logerror("TMS32025:  Active XINT (Serial transmit)\n");
			S_OUT(TMS32025_DX,DXR);
			SET_PC(0x001C);
			R.idle = 0;
			R.IFR &= (~0x20);
			SET0(INTM_FLAG);
			return R.tms32025_irq_cycles;
		}
	}
	return R.tms32025_irq_cycles;
}

INLINE void process_timer(int clocks)
{
	int preclocks, ticks;

	/* easy case: no actual ticks */
again:
	preclocks = CLK - R.timerover;
	if (clocks < preclocks)
	{
		R.timerover += clocks;
		tms32025_icount -= clocks;
		return;
	}

	/* if we're not going to overflow the timer, just count the clocks */
	ticks = 1 + (clocks - preclocks) / CLK;
	if (ticks <= TIM)
	{
		tms32025_icount -= clocks;
		R.timerover = clocks - (ticks - 1) * CLK - preclocks;
		TIM -= ticks;
	}

	/* otherwise, overflow the timer and signal an interrupt */
	else
	{
		tms32025_icount -= preclocks + CLK * TIM;
		R.timerover = 0;
		TIM = PRD;

		R.IFR |= 0x08;
		clocks = process_IRQs();		/* Handle Timer IRQ */
		goto again;
	}
}


/****************************************************************************
 *  Execute ICount cycles. Exit when 0 or less
 ****************************************************************************/
static CPU_EXECUTE( tms32025 )
{
	tms32025_icount = cycles;


	/**** Respond to external hold signal */
	if (S_IN(TMS32025_HOLD) == ASSERT_LINE) {
		if (R.hold == 0) {
			S_OUT(TMS32025_HOLDA,ASSERT_LINE);	/* Hold-Ack (active low) */
		}
		R.hold = 1;
		if (HM) {
			tms32025_icount = 0;		/* Exit */
		}
		else {
			if (R.external_mem_access) {
				tms32025_icount = 0;	/* Exit */
			}
		}
	}
	else {
		if (R.hold == 1) {
			S_OUT(TMS32025_HOLDA,CLEAR_LINE);	/* Hold-Ack (active low) */
			process_timer(3);
		}
		R.hold = 0;
	}

	/**** If idling, update timer and/or exit execution */
	while (R.idle && tms32025_icount > 0)
		process_timer(tms32025_icount);

	if (tms32025_icount <= 0) debugger_instruction_hook(device, R.PC);


	while (tms32025_icount > 0)
	{
		R.tms32025_dec_cycles = (1*CLK);

		if (R.IFR) {	/* Check IRQ Flag Register for pending IRQs */
			R.tms32025_dec_cycles += process_IRQs();
		}

		R.PREVPC = R.PC;

		debugger_instruction_hook(device, R.PC);

		R.opcode.d = M_RDOP(R.PC);
		R.PC++;

		if (R.opcode.b.h == 0xCE)	/* Opcode 0xCExx has many opcodes in its minor byte */
		{
			R.tms32025_dec_cycles = cycles_CE_subset[R.opcode.b.l];
			(*(opcode_CE_subset[R.opcode.b.l]))();
		}
		else if ((R.opcode.w.l & 0xf0f8) == 0xd000)	/* Opcode 0xDxxx has many opcodes in its minor byte */
		{
			R.tms32025_dec_cycles = cycles_DX_subset[R.opcode.b.l];
			(*(opcode_DX_subset[R.opcode.b.l]))();
		}
		else			/* Do all opcodes except the CExx and Dxxx ones */
		{
			R.tms32025_dec_cycles = cycles_main[R.opcode.b.h];
			(*(opcode_main[R.opcode.b.h]))();
		}


		if (R.init_load_addr == 2) {		/* Repeat next instruction */
			R.PREVPC = R.PC;

			debugger_instruction_hook(device, R.PC);

			R.opcode.d = M_RDOP(R.PC);
			R.PC++;
			R.tms32025_dec_cycles += (1*CLK);

			do {
				if (R.opcode.b.h == 0xCE)
				{							/* Do all 0xCExx Opcodes */
					if (R.init_load_addr) {
						R.tms32025_dec_cycles += (1*CLK);
					}
					else {
						R.tms32025_dec_cycles += (1*CLK);
					}
					(*(opcode_CE_subset[R.opcode.b.l]))();
				}
				if ((R.opcode.w.l & 0xf0f8) == 0xd000)
				{							/* Do all valid 0xDxxx Opcodes */
					if (R.init_load_addr) {
						R.tms32025_dec_cycles += (1*CLK);
					}
					else {
						R.tms32025_dec_cycles += (1*CLK);
					}
					(*(opcode_DX_subset[R.opcode.b.l]))();
				}
				else
				{							/* Do all other opcodes */
					if (R.init_load_addr) {
						R.tms32025_dec_cycles += (1*CLK);
					}
					else {
						R.tms32025_dec_cycles += (1*CLK);
					}
					(*(opcode_main[R.opcode.b.h]))();
				}
				R.init_load_addr = 0;
				R.RPTC-- ;
			} while ((INT8)(R.RPTC) != -1);
			R.RPTC = 0;
			R.PFC = R.PC;
			R.init_load_addr = 1;
		}

		process_timer(R.tms32025_dec_cycles);

		/**** If device is put into idle mode, exit and wait for an interrupt */
		while (R.idle && tms32025_icount > 0)
			process_timer(tms32025_icount);


		/**** If hold pin is active, exit if accessing external memory or if HM is set */
		if (R.hold) {
			if (R.external_mem_access || (HM)) {
				if (tms32025_icount > 0) {
					tms32025_icount = 0;
				}
			}
		}
	}

	return (cycles - tms32025_icount);
}


/****************************************************************************
 *  Get all registers in given buffer
 ****************************************************************************/
static CPU_GET_CONTEXT( tms32025 )
{
	if (dst)
	{
		memcpy(&R.datamap_save, &tms32025_datamap[0], sizeof(R.datamap_save));
		memcpy(&R.pgmmap_save, &tms32025_pgmmap[500], sizeof(R.pgmmap_save));
		*(tms32025_Regs*)dst = R;
	}
}


/****************************************************************************
 *  Set all registers to given values
 ****************************************************************************/
static CPU_SET_CONTEXT( tms32025 )
{
	if (src)
	{
		R = *(tms32025_Regs*)src;
		memcpy(&tms32025_datamap[0], &R.datamap_save, sizeof(R.datamap_save));
		memcpy(&tms32025_pgmmap[500], &R.pgmmap_save, sizeof(R.pgmmap_save));
	}
}


/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/
static void set_irq_line(int irqline, int state)
{
	/* Pending IRQs cannot be cleared */

	if (state != CLEAR_LINE)
	{
		R.IFR |= (1 << irqline);
//      R.IFR &= 0x07;
	}
}


/****************************************************************************
 *  Opcode fetcher
 ****************************************************************************/
static CPU_READOP( tms32025 )
{
	void *ptr;

	/* skip if not custom */
	if (!tms32025_pgmmap[offset >> 8])
		return 0;

	ptr = &((UINT8 *)&tms32025_pgmmap[offset >> 8])[offset & 0xff];
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
	void *ptr = NULL;
	UINT64 temp = 0;

	switch (space)
	{
		case ADDRESS_SPACE_PROGRAM:
			ptr = tms32025_pgmmap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case ADDRESS_SPACE_DATA:
			ptr = tms32025_datamap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case ADDRESS_SPACE_IO:
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
	void *ptr = NULL;

	switch (space)
	{
		case ADDRESS_SPACE_PROGRAM:
			ptr = tms32025_pgmmap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case ADDRESS_SPACE_DATA:
			ptr = tms32025_datamap[offset >> 8];
			if (!ptr)
				return 0;
			break;

		case ADDRESS_SPACE_IO:
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
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT0:		set_irq_line(TMS32025_INT0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT1:		set_irq_line(TMS32025_INT1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT2:		set_irq_line(TMS32025_INT2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_TINT:		set_irq_line(TMS32025_TINT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_RINT:		set_irq_line(TMS32025_RINT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_XINT:		set_irq_line(TMS32025_XINT, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32025_PC:		R.PC = info->i;							break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32025_STK7: 		R.STACK[7] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK6: 		R.STACK[6] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK5: 		R.STACK[5] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK4: 		R.STACK[4] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK3: 		R.STACK[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK2: 		R.STACK[2] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK1: 		R.STACK[1] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK0: 		R.STACK[0] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32025_STR0: 		R.STR0 = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_STR1: 		R.STR1 = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_IFR:  		R.IFR = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_RPTC: 		R.RPTC = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_ACC:  		R.ACC.d = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_PREG: 		R.Preg.d = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_TREG: 		R.Treg = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR0:  		R.AR[0] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR1:  		R.AR[1] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR2:  		R.AR[2] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR3:  		R.AR[3] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR4:  		R.AR[4] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR5:  		R.AR[5] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR6:  		R.AR[6] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR7:  		R.AR[7] = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32025_DRR:		M_WRTRAM(0,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_DXR:		M_WRTRAM(1,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_TIM:		M_WRTRAM(2,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_PRD:		M_WRTRAM(3,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_IMR:		M_WRTRAM(4,info->i);					break;
		case CPUINFO_INT_REGISTER + TMS32025_GREG:		M_WRTRAM(5,info->i);					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( tms32025 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(R);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 6;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1*CLK;						break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 5*CLK;						break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 17;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = -1;					break;

		case CPUINFO_INT_INPUT_STATE + TMS32025_INT0:		info->i = (R.IFR & 0x01) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT1:		info->i = (R.IFR & 0x02) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_INT2:		info->i = (R.IFR & 0x04) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_TINT:		info->i = (R.IFR & 0x08) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_RINT:		info->i = (R.IFR & 0x10) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + TMS32025_XINT:		info->i = (R.IFR & 0x20) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = R.PREVPC;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32025_PC:		info->i = R.PC;							break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32025_STK7:		info->i = R.STACK[7];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK6:		info->i = R.STACK[6];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK5:		info->i = R.STACK[5];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK4:		info->i = R.STACK[4];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK3:		info->i = R.STACK[3];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK2:		info->i = R.STACK[2];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK1:		info->i = R.STACK[1];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STK0:		info->i = R.STACK[0];					break;
		case CPUINFO_INT_REGISTER + TMS32025_STR0:		info->i = R.STR0;						break;
		case CPUINFO_INT_REGISTER + TMS32025_STR1:		info->i = R.STR1;						break;
		case CPUINFO_INT_REGISTER + TMS32025_IFR: 		info->i = R.IFR;						break;
		case CPUINFO_INT_REGISTER + TMS32025_RPTC:		info->i = R.RPTC;						break;
		case CPUINFO_INT_REGISTER + TMS32025_ACC: 		info->i = R.ACC.d;						break;
		case CPUINFO_INT_REGISTER + TMS32025_PREG:		info->i = R.Preg.d;						break;
		case CPUINFO_INT_REGISTER + TMS32025_TREG:		info->i = R.Treg;						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR0: 		info->i = R.AR[0];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR1: 		info->i = R.AR[1];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR2: 		info->i = R.AR[2];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR3: 		info->i = R.AR[3];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR4: 		info->i = R.AR[4];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR5: 		info->i = R.AR[5];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR6: 		info->i = R.AR[6];						break;
		case CPUINFO_INT_REGISTER + TMS32025_AR7: 		info->i = R.AR[7];						break;
		case CPUINFO_INT_REGISTER + TMS32025_DRR: 		info->i = M_RDRAM(0);					break;
		case CPUINFO_INT_REGISTER + TMS32025_DXR: 		info->i = M_RDRAM(1);					break;
		case CPUINFO_INT_REGISTER + TMS32025_TIM: 		info->i = M_RDRAM(2);					break;
		case CPUINFO_INT_REGISTER + TMS32025_PRD: 		info->i = M_RDRAM(3);					break;
		case CPUINFO_INT_REGISTER + TMS32025_IMR: 		info->i = M_RDRAM(4);					break;
		case CPUINFO_INT_REGISTER + TMS32025_GREG:		info->i = M_RDRAM(5);					break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(tms32025);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(tms32025); break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(tms32025); break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(tms32025);				break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(tms32025);			break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(tms32025);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(tms32025);		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(tms32025);		break;
		case CPUINFO_PTR_READ:							info->read = CPU_READ_NAME(tms32025);				break;
		case CPUINFO_PTR_WRITE:							info->write = CPU_WRITE_NAME(tms32025);			break;
		case CPUINFO_PTR_READOP:						info->readop = CPU_READOP_NAME(tms32025);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &tms32025_icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32025");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Texas Instruments TMS320x25"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.10");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Tony La Porta"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "arp%d%c%c%c%cdp%03x  arb%d%c%c%c%c%c%c%c%c%c%c%cpm%d",
				(R.STR0 & 0xe000) >> 13,
				R.STR0 & 0x1000 ? 'O':'.',
				R.STR0 & 0x0800 ? 'M':'.',
				R.STR0 & 0x0400 ? '.':'?',
				R.STR0 & 0x0200 ? 'I':'.',
				(R.STR0 & 0x01ff),

				(R.STR1 & 0xe000) >> 13,
				R.STR1 & 0x1000 ? 'P':'D',
				R.STR1 & 0x0800 ? 'T':'.',
				R.STR1 & 0x0400 ? 'S':'.',
				R.STR1 & 0x0200 ? 'C':'?',
				R.STR0 & 0x0100 ? '.':'?',
				R.STR1 & 0x0080 ? '.':'?',
				R.STR1 & 0x0040 ? 'H':'.',
				R.STR1 & 0x0020 ? 'F':'.',
				R.STR1 & 0x0010 ? 'X':'.',
				R.STR1 & 0x0008 ? 'f':'.',
				R.STR1 & 0x0004 ? 'o':'i',
				(R.STR1 & 0x0003) );
			break;

		case CPUINFO_STR_REGISTER + TMS32025_PC:		sprintf(info->s, "PC:%04X",  R.PC); break;
		case CPUINFO_STR_REGISTER + TMS32025_STR0:		sprintf(info->s, "STR0:%04X", R.STR0); break;
		case CPUINFO_STR_REGISTER + TMS32025_STR1:		sprintf(info->s, "STR1:%04X", R.STR1); break;
		case CPUINFO_STR_REGISTER + TMS32025_IFR:		sprintf(info->s, "IFR:%04X", R.IFR); break;
		case CPUINFO_STR_REGISTER + TMS32025_RPTC:		sprintf(info->s, "RPTC:%02X", R.RPTC); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK7:		sprintf(info->s, "STK7:%04X", R.STACK[7]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK6:		sprintf(info->s, "STK6:%04X", R.STACK[6]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK5:		sprintf(info->s, "STK5:%04X", R.STACK[5]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK4:		sprintf(info->s, "STK4:%04X", R.STACK[4]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK3:		sprintf(info->s, "STK3:%04X", R.STACK[3]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK2:		sprintf(info->s, "STK2:%04X", R.STACK[2]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK1:		sprintf(info->s, "STK1:%04X", R.STACK[1]); break;
		case CPUINFO_STR_REGISTER + TMS32025_STK0:		sprintf(info->s, "STK0:%04X", R.STACK[0]); break;
		case CPUINFO_STR_REGISTER + TMS32025_ACC:		sprintf(info->s, "ACC:%08X", R.ACC.d); break;
		case CPUINFO_STR_REGISTER + TMS32025_PREG:		sprintf(info->s, "P:%08X", R.Preg.d); break;
		case CPUINFO_STR_REGISTER + TMS32025_TREG:		sprintf(info->s, "T:%04X", R.Treg); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR0:		sprintf(info->s, "AR0:%04X", R.AR[0]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR1:		sprintf(info->s, "AR1:%04X", R.AR[1]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR2:		sprintf(info->s, "AR2:%04X", R.AR[2]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR3:		sprintf(info->s, "AR3:%04X", R.AR[3]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR4:		sprintf(info->s, "AR4:%04X", R.AR[4]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR5:		sprintf(info->s, "AR5:%04X", R.AR[5]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR6:		sprintf(info->s, "AR6:%04X", R.AR[6]); break;
		case CPUINFO_STR_REGISTER + TMS32025_AR7:		sprintf(info->s, "AR7:%04X", R.AR[7]); break;
		case CPUINFO_STR_REGISTER + TMS32025_DRR:		sprintf(info->s, "DRR:%04X", M_RDRAM(0)); break;
		case CPUINFO_STR_REGISTER + TMS32025_DXR:		sprintf(info->s, "DXR:%04X", M_RDRAM(1)); break;
		case CPUINFO_STR_REGISTER + TMS32025_TIM:		sprintf(info->s, "TIM:%04X", M_RDRAM(2)); break;
		case CPUINFO_STR_REGISTER + TMS32025_PRD:		sprintf(info->s, "PRD:%04X", M_RDRAM(3)); break;
		case CPUINFO_STR_REGISTER + TMS32025_IMR:		sprintf(info->s, "IMR:%04X", M_RDRAM(4)); break;
		case CPUINFO_STR_REGISTER + TMS32025_GREG:		sprintf(info->s, "GREG:%04X", M_RDRAM(5)); break;
	}
}


#if (HAS_TMS32026)
/**************************************************************************
 * CPU-specific set_info
 **************************************************************************/

CPU_GET_INFO( tms32026 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(tms32026);			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32026");			break;

		default:										CPU_GET_INFO_CALL(tms32025);			break;
	}
}
#endif
