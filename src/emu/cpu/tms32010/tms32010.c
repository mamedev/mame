 /**************************************************************************\
 *                 Texas Instruments TMS32010 DSP Emulator                  *
 *                                                                          *
 *                  Copyright Tony La Porta                                 *
 *      You are not allowed to distribute this software commercially.       *
 *                      Written for the MAME project.                       *
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
 *  TLP (13-Jul-2002)                                                       *
 *   - Added Save-State support                                             *
 *   - Converted the pending_irq flag to INTF (a real flag in this device)  *
 *   - Fixed the ignore Interrupt Request for previous critical             *
 *     instructions requiring an extra instruction to be processed. For     *
 *     this reason, instant IRQ servicing cannot be supported here, so      *
 *     INTF needs to be polled within the instruction execution loop        *
 *   - Removed IRQ callback (IRQ ACK not supported on this device)          *
 *   - A pending IRQ will remain pending until it's serviced. De-asserting  *
 *     the IRQ Pin does not remove a pending IRQ state                      *
 *   - BIO is no longer treated as an IRQ line. It's polled when required.  *
 *     This is the true behaviour of the device                             *
 *   - Removed the Clear OV flag from overflow instructions. Overflow       *
 *     instructions can only set the flag. Flag test instructions clear it  *
 *   - Fixed the ABST, SUBC and SUBH instructions                           *
 *   - Fixed the signedness in many equation based instructions             *
 *   - Added the missing Previous PC to the get_register function           *
 *   - Changed Cycle timings to include clock ticks                         *
 *   - Converted some registers from ints to pairs for much cleaner code    *
 *  TLP (20-Jul-2002) Ver 1.10                                              *
 *   - Fixed the dissasembly from the debugger                              *
 *   - Changed all references from TMS320C10 to TMS32010                    *
 *  ASG (24-Sep-2002) Ver 1.20                                              *
 *   - Fixed overflow handling                                              *
 *   - Simplified logic in a few locations                                  *
 *  TLP (22-Feb-2004) Ver 1.21                                              *
 *   - Overflow for ADDH only affects upper 16bits (was modifying 32 bits)  *
 *   - Internal Data Memory map is assigned here now                        *
 *   - Cycle counts for invalid opcodes 7F1E and 7F1F are now 0             *
 *  RK  (23-Nov-2006) Ver 1.22                                              *
 *   - Fixed state of the Overflow Flag on reset                            *
 *   - Fixed the SUBC instruction which was incorrectly zeroing the divisor *
 *  TLP (13-Jul-2010) Ver 1.30                                              *
 *   - LST instruction was incorrectly setting an Indirect Addressing       *
 *     feature when Direct Addressing mode was selected                     *
 *   - Added TMS32015 and TMS32016 variants                                 *
 *  TLP (27-Jul-2010) Ver 1.31                                              *
 *   - Corrected cycle timing for conditional branch instructions           *
 *                                                                          *
 \**************************************************************************/


#include "emu.h"
#include "debugger.h"
#include "tms32010.h"



#define M_RDROM(A)		TMS32010_ROM_RDMEM(A)
#define M_WRTROM(A,V)	TMS32010_ROM_WRMEM(A,V)
#define M_RDRAM(A)		TMS32010_RAM_RDMEM(A)
#define M_WRTRAM(A,V)	TMS32010_RAM_WRMEM(A,V)
#define M_RDOP(A)		TMS32010_RDOP(A)
#define M_RDOP_ARG(A)	TMS32010_RDOP_ARG(A)
#define P_IN(A)			TMS32010_In(A)
#define P_OUT(A,V)		TMS32010_Out(A,V)
#define BIO_IN			TMS32010_BIO_In



struct tms32010_state
{
	/******************** CPU Internal Registers *******************/
	UINT16	PC;
	UINT16	PREVPC;		/* previous program counter */
	UINT16	STR;
	PAIR	ACC;
	PAIR	ALU;
	PAIR	Preg;
	UINT16	Treg;
	UINT16	AR[2];
	UINT16	STACK[4];

	PAIR	opcode;
	int		INTF;		/* Pending Interrupt flag */
	int		icount;
	PAIR	oldacc;
	UINT16	memaccess;
	int		addr_mask;

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *data;
	address_space *io;
};

INLINE tms32010_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == TMS32010 ||
		   device->type() == TMS32015 ||
		   device->type() == TMS32016);
	return (tms32010_state *)downcast<legacy_cpu_device *>(device)->token();
}

/* opcode table entry */
struct tms32010_opcode
{
	UINT8	cycles;
	void	(*function)(tms32010_state *);
};

INLINE int add_branch_cycle(tms32010_state *cpustate);


/*********  The following is the Status (Flag) register definition.  *********/
/* 15 | 14  |  13  | 12 | 11 | 10 | 9 |  8  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0  */
/* OV | OVM | INTM |  1 |  1 |  1 | 1 | ARP | 1 | 1 | 1 | 1 | 1 | 1 | 1 | DP */
#define OV_FLAG		0x8000	/* OV   (Overflow flag) 1 indicates an overflow */
#define OVM_FLAG	0x4000	/* OVM  (Overflow Mode bit) 1 forces ACC overflow to greatest positive or negative saturation value */
#define INTM_FLAG	0x2000	/* INTM (Interrupt Mask flag) 0 enables maskable interrupts */
#define ARP_REG		0x0100	/* ARP  (Auxiliary Register Pointer) */
#define DP_REG		0x0001	/* DP   (Data memory Pointer (bank) bit) */

#define OV		( cpustate->STR & OV_FLAG)			/* OV   (Overflow flag) */
#define OVM		( cpustate->STR & OVM_FLAG)			/* OVM  (Overflow Mode bit) 1 indicates an overflow */
#define INTM	( cpustate->STR & INTM_FLAG)		/* INTM (Interrupt enable flag) 0 enables maskable interrupts */
#define ARP		((cpustate->STR & ARP_REG) >> 8)	/* ARP  (Auxiliary Register Pointer) */
#define DP		((cpustate->STR & DP_REG) << 7)		/* DP   (Data memory Pointer bit) */

#define DMA_DP	(DP | (cpustate->opcode.b.l & 0x7f))	/* address used in direct memory access operations */
#define DMA_DP1	(0x80 | cpustate->opcode.b.l)			/* address used in direct memory access operations for sst instruction */
#define IND		(cpustate->AR[ARP] & 0xff)				/* address used in indirect memory access operations */



/****************************************************************************
 *  Read the state of the BIO pin
 */

#define TMS32010_BIO_In (cpustate->io->read_word(TMS32010_BIO<<1))


/****************************************************************************
 *  Input a word from given I/O port
 */

#define TMS32010_In(Port) (cpustate->io->read_word((Port)<<1))


/****************************************************************************
 *  Output a word to given I/O port
 */

#define TMS32010_Out(Port,Value) (cpustate->io->write_word((Port)<<1,Value))



/****************************************************************************
 *  Read a word from given ROM memory location
 */

#define TMS32010_ROM_RDMEM(A) (cpustate->program->read_word((A)<<1))


/****************************************************************************
 *  Write a word to given ROM memory location
 */

#define TMS32010_ROM_WRMEM(A,V) (cpustate->program->write_word((A)<<1,V))



/****************************************************************************
 *  Read a word from given RAM memory location
 */

#define TMS32010_RAM_RDMEM(A) (cpustate->data->read_word((A)<<1))


/****************************************************************************
 *  Write a word to given RAM memory location
 */

#define TMS32010_RAM_WRMEM(A,V) (cpustate->data->write_word((A)<<1,V))



/****************************************************************************
 *  TMS32010_RDOP() is identical to TMS32010_RDMEM() except it is used for reading
 *  opcodes. In case of system with memory mapped I/O, this function can be
 *  used to greatly speed up emulation
 */

#define TMS32010_RDOP(A) (cpustate->direct->read_decrypted_word((A)<<1))


/****************************************************************************
 *  TMS32010_RDOP_ARG() is identical to TMS32010_RDOP() except it is used
 *  for reading opcode arguments. This difference can be used to support systems
 *  that use different encoding mechanisms for opcodes and opcode arguments
 */

#define TMS32010_RDOP_ARG(A) (cpustate->direct->read_raw_word((A)<<1))


/************************************************************************
 *  Shortcuts
 ************************************************************************/

INLINE void CLR(tms32010_state *cpustate, UINT16 flag) { cpustate->STR &= ~flag; cpustate->STR |= 0x1efe; }
INLINE void SET(tms32010_state *cpustate, UINT16 flag) { cpustate->STR |=  flag; cpustate->STR |= 0x1efe; }


INLINE void CALCULATE_ADD_OVERFLOW(tms32010_state *cpustate, INT32 addval)
{
	if ((INT32)(~(cpustate->oldacc.d ^ addval) & (cpustate->oldacc.d ^ cpustate->ACC.d)) < 0) {
		SET(cpustate, OV_FLAG);
		if (OVM)
			cpustate->ACC.d = ((INT32)cpustate->oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}
INLINE void CALCULATE_SUB_OVERFLOW(tms32010_state *cpustate, INT32 subval)
{
	if ((INT32)((cpustate->oldacc.d ^ subval) & (cpustate->oldacc.d ^ cpustate->ACC.d)) < 0) {
		SET(cpustate, OV_FLAG);
		if (OVM)
			cpustate->ACC.d = ((INT32)cpustate->oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}

INLINE UINT16 POP_STACK(tms32010_state *cpustate)
{
	UINT16 data = cpustate->STACK[3];
	cpustate->STACK[3] = cpustate->STACK[2];
	cpustate->STACK[2] = cpustate->STACK[1];
	cpustate->STACK[1] = cpustate->STACK[0];
	return (data & cpustate->addr_mask);
}
INLINE void PUSH_STACK(tms32010_state *cpustate, UINT16 data)
{
	cpustate->STACK[0] = cpustate->STACK[1];
	cpustate->STACK[1] = cpustate->STACK[2];
	cpustate->STACK[2] = cpustate->STACK[3];
	cpustate->STACK[3] = (data & cpustate->addr_mask);
}

INLINE void UPDATE_AR(tms32010_state *cpustate)
{
	if (cpustate->opcode.b.l & 0x30) {
		UINT16 tmpAR = cpustate->AR[ARP];
		if (cpustate->opcode.b.l & 0x20) tmpAR++ ;
		if (cpustate->opcode.b.l & 0x10) tmpAR-- ;
		cpustate->AR[ARP] = (cpustate->AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
	}
}
INLINE void UPDATE_ARP(tms32010_state *cpustate)
{
	if (~cpustate->opcode.b.l & 0x08) {
		if (cpustate->opcode.b.l & 0x01) SET(cpustate, ARP_REG);
		else CLR(cpustate, ARP_REG);
	}
}


INLINE void getdata(tms32010_state *cpustate, UINT8 shift,UINT8 signext)
{
	if (cpustate->opcode.b.l & 0x80)
		cpustate->memaccess = IND;
	else
		cpustate->memaccess = DMA_DP;

	cpustate->ALU.d = (UINT16)M_RDRAM(cpustate->memaccess);
	if (signext) cpustate->ALU.d = (INT16)cpustate->ALU.d;
	cpustate->ALU.d <<= shift;
	if (cpustate->opcode.b.l & 0x80) {
		UPDATE_AR(cpustate);
		UPDATE_ARP(cpustate);
	}
}

INLINE void putdata(tms32010_state *cpustate, UINT16 data)
{
	if (cpustate->opcode.b.l & 0x80)
		cpustate->memaccess = IND;
	else
		cpustate->memaccess = DMA_DP;

	if (cpustate->opcode.b.l & 0x80) {
		UPDATE_AR(cpustate);
		UPDATE_ARP(cpustate);
	}
	M_WRTRAM(cpustate->memaccess,data);
}
INLINE void putdata_sar(tms32010_state *cpustate, UINT8 data)
{
	if (cpustate->opcode.b.l & 0x80)
		cpustate->memaccess = IND;
	else
		cpustate->memaccess = DMA_DP;

	if (cpustate->opcode.b.l & 0x80) {
		UPDATE_AR(cpustate);
		UPDATE_ARP(cpustate);
	}
	M_WRTRAM(cpustate->memaccess,cpustate->AR[data]);
}
INLINE void putdata_sst(tms32010_state *cpustate, UINT16 data)
{
	if (cpustate->opcode.b.l & 0x80)
		cpustate->memaccess = IND;
	else
		cpustate->memaccess = DMA_DP1;	/* Page 1 only */

	if (cpustate->opcode.b.l & 0x80) {
		UPDATE_AR(cpustate);
	}
	M_WRTRAM(cpustate->memaccess,data);
}



/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */

static void opcodes_7F(tms32010_state *cpustate)  { }


static void illegal(tms32010_state *cpustate)
{
	logerror("TMS32010:  PC=%04x,  Illegal opcode = %04x\n", (cpustate->PC-1), cpustate->opcode.w.l);
}

static void abst(tms32010_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) < 0 ) {
		cpustate->ACC.d = -cpustate->ACC.d;
		if (OVM && (cpustate->ACC.d == 0x80000000)) cpustate->ACC.d-- ;
	}
}

/*** The manual doesn't mention overflow with the ADD? instructions however ***
 *** overflow is implemented here, because it makes little sense otherwise ****
 *** while newer generations of this type of chip supported it. The ***********
 *** manual may be wrong wrong (apart from other errors the manual has). ******

static void add_sh(cpustate)    { getdata(cpustate, cpustate->opcode.b.h,1); cpustate->ACC.d += cpustate->ALU.d; }
static void addh(cpustate)      { getdata(cpustate, 0,0); cpustate->ACC.d += (cpustate->ALU.d << 16); }
 ***/

static void add_sh(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, (cpustate->opcode.b.h & 0xf),1);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
}
static void addh(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, 0,0);
	cpustate->ACC.w.h += cpustate->ALU.w.l;
	if ((INT16)(~(cpustate->oldacc.w.h ^ cpustate->ALU.w.h) & (cpustate->oldacc.w.h ^ cpustate->ACC.w.h)) < 0) {
		SET(cpustate, OV_FLAG);
		if (OVM)
			cpustate->ACC.w.h = ((INT16)cpustate->oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
static void adds(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, 0,0);
	cpustate->ACC.d += cpustate->ALU.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->ALU.d);
}
static void and_(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->ACC.d &= cpustate->ALU.d;
}
static void apac(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	cpustate->ACC.d += cpustate->Preg.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->Preg.d);
}
static void br(tms32010_state *cpustate)
{
	cpustate->PC = M_RDOP_ARG(cpustate->PC);
}
static void banz(tms32010_state *cpustate)
{
	if (cpustate->AR[ARP] & 0x01ff) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
	cpustate->ALU.w.l = cpustate->AR[ARP];
	cpustate->ALU.w.l-- ;
	cpustate->AR[ARP] = (cpustate->AR[ARP] & 0xfe00) | (cpustate->ALU.w.l & 0x01ff);
}
static void bgez(tms32010_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) >= 0 ) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void bgz(tms32010_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) > 0 ) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void bioz(tms32010_state *cpustate)
{
	if (BIO_IN != CLEAR_LINE) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void blez(tms32010_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) <= 0 ) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void blz(tms32010_state *cpustate)
{
	if ( (INT32)(cpustate->ACC.d) <  0 ) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void bnz(tms32010_state *cpustate)
{
	if (cpustate->ACC.d != 0) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void bv(tms32010_state *cpustate)
{
	if (OV) {
		CLR(cpustate, OV_FLAG);
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void bz(tms32010_state *cpustate)
{
	if (cpustate->ACC.d == 0) {
		cpustate->PC = M_RDOP_ARG(cpustate->PC);
		cpustate->icount -= add_branch_cycle(cpustate);
	}
	else
		cpustate->PC++ ;
}
static void cala(tms32010_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->PC);
	cpustate->PC = cpustate->ACC.w.l & cpustate->addr_mask;
}
static void call(tms32010_state *cpustate)
{
	cpustate->PC++ ;
	PUSH_STACK(cpustate, cpustate->PC);
	cpustate->PC = M_RDOP_ARG((cpustate->PC - 1));
}
static void dint(tms32010_state *cpustate)
{
	SET(cpustate, INTM_FLAG);
}
static void dmov(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	M_WRTRAM((cpustate->memaccess + 1),cpustate->ALU.w.l);
}
static void eint(tms32010_state *cpustate)
{
	CLR(cpustate, INTM_FLAG);
}
static void in_p(tms32010_state *cpustate)
{
	cpustate->ALU.w.l = P_IN( (cpustate->opcode.b.h & 7) );
	putdata(cpustate, cpustate->ALU.w.l);
}
static void lac_sh(tms32010_state *cpustate)
{
	getdata(cpustate, (cpustate->opcode.b.h & 0x0f),1);
	cpustate->ACC.d = cpustate->ALU.d;
}
static void lack(tms32010_state *cpustate)
{
	cpustate->ACC.d = cpustate->opcode.b.l;
}
static void lar_ar0(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->AR[0] = cpustate->ALU.w.l;
}
static void lar_ar1(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->AR[1] = cpustate->ALU.w.l;
}
static void lark_ar0(tms32010_state *cpustate)
{
	cpustate->AR[0] = cpustate->opcode.b.l;
}
static void lark_ar1(tms32010_state *cpustate)
{
	cpustate->AR[1] = cpustate->opcode.b.l;
}
static void larp_mar(tms32010_state *cpustate)
{
	if (cpustate->opcode.b.l & 0x80) {
		UPDATE_AR(cpustate);
		UPDATE_ARP(cpustate);
	}
}
static void ldp(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	if (cpustate->ALU.d & 1)
		SET(cpustate, DP_REG);
	else
		CLR(cpustate, DP_REG);
}
static void ldpk(tms32010_state *cpustate)
{
	if (cpustate->opcode.b.l & 1)
		SET(cpustate, DP_REG);
	else
		CLR(cpustate, DP_REG);
}
static void lst(tms32010_state *cpustate)
{
	if (cpustate->opcode.b.l & 0x80) {
		cpustate->opcode.b.l |= 0x08; /* In Indirect Addressing mode, next ARP is not supported here so mask it */
	}
	getdata(cpustate, 0,0);
	cpustate->ALU.w.l &= (~INTM_FLAG);	/* Must not affect INTM */
	cpustate->STR &= INTM_FLAG;
	cpustate->STR |= cpustate->ALU.w.l;
	cpustate->STR |= 0x1efe;
}
static void lt(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->Treg = cpustate->ALU.w.l;
}
static void lta(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, 0,0);
	cpustate->Treg = cpustate->ALU.w.l;
	cpustate->ACC.d += cpustate->Preg.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->Preg.d);
}
static void ltd(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, 0,0);
	cpustate->Treg = cpustate->ALU.w.l;
	M_WRTRAM((cpustate->memaccess + 1),cpustate->ALU.w.l);
	cpustate->ACC.d += cpustate->Preg.d;
	CALCULATE_ADD_OVERFLOW(cpustate, cpustate->Preg.d);
}
static void mpy(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->Preg.d = (INT16)cpustate->ALU.w.l * (INT16)cpustate->Treg;
	if (cpustate->Preg.d == 0x40000000) cpustate->Preg.d = 0xc0000000;
}
static void mpyk(tms32010_state *cpustate)
{
	cpustate->Preg.d = (INT16)cpustate->Treg * ((INT16)(cpustate->opcode.w.l << 3) >> 3);
}
static void nop(tms32010_state *cpustate)
{
	/* Nothing to do */
}
static void or_(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->ACC.w.l |= cpustate->ALU.w.l;
}
static void out_p(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	P_OUT( (cpustate->opcode.b.h & 7), cpustate->ALU.w.l );
}
static void pac(tms32010_state *cpustate)
{
	cpustate->ACC.d = cpustate->Preg.d;
}
static void pop(tms32010_state *cpustate)
{
	cpustate->ACC.w.l = POP_STACK(cpustate);
	cpustate->ACC.w.h = 0x0000;
}
static void push(tms32010_state *cpustate)
{
	PUSH_STACK(cpustate, cpustate->ACC.w.l);
}
static void ret(tms32010_state *cpustate)
{
	cpustate->PC = POP_STACK(cpustate);
}
static void rovm(tms32010_state *cpustate)
{
	CLR(cpustate, OVM_FLAG);
}
static void sach_sh(tms32010_state *cpustate)
{
	cpustate->ALU.d = (cpustate->ACC.d << (cpustate->opcode.b.h & 7));
	putdata(cpustate, cpustate->ALU.w.h);
}
static void sacl(tms32010_state *cpustate)
{
	putdata(cpustate, cpustate->ACC.w.l);
}
static void sar_ar0(tms32010_state *cpustate)
{
	putdata_sar(cpustate, 0);
}
static void sar_ar1(tms32010_state *cpustate)
{
	putdata_sar(cpustate, 1);
}
static void sovm(tms32010_state *cpustate)
{
	SET(cpustate, OVM_FLAG);
}
static void spac(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	cpustate->ACC.d -= cpustate->Preg.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->Preg.d);
}
static void sst(tms32010_state *cpustate)
{
	putdata_sst(cpustate, cpustate->STR);
}
static void sub_sh(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, (cpustate->opcode.b.h & 0x0f),1);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
}
static void subc(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, 15,0);
	cpustate->ALU.d = (INT32) cpustate->ACC.d - cpustate->ALU.d;
	if ((INT32)((cpustate->oldacc.d ^ cpustate->ALU.d) & (cpustate->oldacc.d ^ cpustate->ACC.d)) < 0)
		SET(cpustate, OV_FLAG);
	if ( (INT32)(cpustate->ALU.d) >= 0 )
		cpustate->ACC.d = ((cpustate->ALU.d << 1) + 1);
	else
		cpustate->ACC.d = (cpustate->ACC.d << 1);
}
static void subh(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, 16,0);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
}
static void subs(tms32010_state *cpustate)
{
	cpustate->oldacc.d = cpustate->ACC.d;
	getdata(cpustate, 0,0);
	cpustate->ACC.d -= cpustate->ALU.d;
	CALCULATE_SUB_OVERFLOW(cpustate, cpustate->ALU.d);
}
static void tblr(tms32010_state *cpustate)
{
	cpustate->ALU.d = M_RDROM((cpustate->ACC.w.l & cpustate->addr_mask));
	putdata(cpustate, cpustate->ALU.w.l);
	cpustate->STACK[0] = cpustate->STACK[1];
}
static void tblw(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	M_WRTROM(((cpustate->ACC.w.l & cpustate->addr_mask)),cpustate->ALU.w.l);
	cpustate->STACK[0] = cpustate->STACK[1];
}
static void xor_(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->ACC.w.l ^= cpustate->ALU.w.l;
}
static void zac(tms32010_state *cpustate)
{
	cpustate->ACC.d = 0;
}
static void zalh(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->ACC.w.h = cpustate->ALU.w.l;
	cpustate->ACC.w.l = 0x0000;
}
static void zals(tms32010_state *cpustate)
{
	getdata(cpustate, 0,0);
	cpustate->ACC.w.l = cpustate->ALU.w.l;
	cpustate->ACC.w.h = 0x0000;
}



/***********************************************************************
 *  Opcode Table (Cycles, Instruction)
 ***********************************************************************/

/* Conditional Branch instructions take two cycles when the test condition is met and the branch performed */

static const tms32010_opcode opcode_main[256]=
{
/*00*/  {1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},
/*08*/  {1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},{1, add_sh	},
/*10*/  {1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},
/*18*/  {1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},{1, sub_sh	},
/*20*/  {1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},
/*28*/  {1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},{1, lac_sh	},
/*30*/  {1, sar_ar0	},{1, sar_ar1	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*38*/  {1, lar_ar0	},{1, lar_ar1	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*40*/  {2, in_p	},{2, in_p		},{2, in_p		},{2, in_p		},{2, in_p		},{2, in_p		},{2, in_p		},{2, in_p		},
/*48*/  {2, out_p	},{2, out_p		},{2, out_p		},{2, out_p		},{2, out_p		},{2, out_p		},{2, out_p		},{2, out_p		},
/*50*/  {1, sacl	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*58*/  {1, sach_sh	},{1, sach_sh	},{1, sach_sh	},{1, sach_sh	},{1, sach_sh	},{1, sach_sh	},{1, sach_sh	},{1, sach_sh	},
/*60*/  {1, addh	},{1, adds		},{1, subh		},{1, subs		},{1, subc		},{1, zalh		},{1, zals		},{3, tblr		},
/*68*/  {1, larp_mar},{1, dmov		},{1, lt		},{1, ltd		},{1, lta		},{1, mpy		},{1, ldpk		},{1, ldp		},
/*70*/  {1, lark_ar0},{1, lark_ar1	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*78*/  {1, xor_	},{1, and_		},{1, or_		},{1, lst		},{1, sst		},{3, tblw		},{1, lack		},{0, opcodes_7F	},
/*80*/  {1, mpyk	},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},
/*88*/  {1, mpyk	},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},
/*90*/  {1, mpyk	},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},
/*98*/  {1, mpyk	},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},{1, mpyk		},
/*A0*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*A8*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*B0*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*B8*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*C0*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*C8*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*D0*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*D8*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*E0*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*E8*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*F0*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{1, banz		},{1, bv		},{1, bioz		},{0, illegal	},
/*F8*/  {2, call	},{2, br		},{1, blz		},{1, blez		},{1, bgz		},{1, bgez		},{1, bnz		},{1, bz		}
};

static const tms32010_opcode opcode_7F[32]=
{
/*80*/  {1, nop		},{1, dint		},{1, eint		},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*88*/  {1, abst	},{1, zac		},{1, rovm		},{1, sovm		},{2, cala		},{2, ret		},{1, pac		},{1, apac		},
/*90*/  {1, spac	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},
/*98*/  {0, illegal	},{0, illegal	},{0, illegal	},{0, illegal	},{2, push		},{2, pop		},{0, illegal	},{0, illegal	}
};

INLINE int add_branch_cycle(tms32010_state *cpustate)
{
	return opcode_main[cpustate->opcode.b.h].cycles;
}

/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

static CPU_INIT( tms32010 )
{
	tms32010_state *cpustate = get_safe_token(device);

	device->save_item(NAME(cpustate->PC));
	device->save_item(NAME(cpustate->PREVPC));
	device->save_item(NAME(cpustate->STR));
	device->save_item(NAME(cpustate->ACC.d));
	device->save_item(NAME(cpustate->ALU.d));
	device->save_item(NAME(cpustate->Preg.d));
	device->save_item(NAME(cpustate->Treg));
	device->save_item(NAME(cpustate->AR[0]));
	device->save_item(NAME(cpustate->AR[1]));
	device->save_item(NAME(cpustate->STACK[0]));
	device->save_item(NAME(cpustate->STACK[1]));
	device->save_item(NAME(cpustate->STACK[2]));
	device->save_item(NAME(cpustate->STACK[3]));
	device->save_item(NAME(cpustate->INTF));
	device->save_item(NAME(cpustate->icount));
	device->save_item(NAME(cpustate->opcode.d));
	device->save_item(NAME(cpustate->oldacc.d));
	device->save_item(NAME(cpustate->memaccess));
	device->save_item(NAME(cpustate->addr_mask));

	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->data = &device->space(AS_DATA);
	cpustate->io = &device->space(AS_IO);
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/

static CPU_EXIT( tms32010 ) { }


/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/

static void set_irq_line(tms32010_state *cpustate, int irqline, int state)
{
	/* Pending Interrupts cannot be cleared! */
	if (state == ASSERT_LINE) cpustate->INTF |= TMS32010_INT_PENDING;
}



/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/

static int Ext_IRQ(tms32010_state *cpustate)
{
	if (INTM == 0)
	{
		logerror("TMS32010:  EXT INTERRUPT\n");
		cpustate->INTF = TMS32010_INT_NONE;
		SET(cpustate, INTM_FLAG);
		PUSH_STACK(cpustate, cpustate->PC);
		cpustate->PC = 0x0002;
		return (opcode_7F[0x1c].cycles + opcode_7F[0x01].cycles);	/* 3 cycles used due to PUSH and DINT operation ? */
	}
	return (0);
}


/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/

static CPU_EXECUTE( tms32010 )
{
	tms32010_state *cpustate = get_safe_token(device);

	do
	{
		if (cpustate->INTF) {
			/* Dont service INT if previous instruction was MPY, MPYK or EINT */
			if ((cpustate->opcode.b.h != 0x6d) && ((cpustate->opcode.b.h & 0xe0) != 0x80) && (cpustate->opcode.w.l != 0x7f82))
				cpustate->icount -= Ext_IRQ(cpustate);
		}

		cpustate->PREVPC = cpustate->PC;

		debugger_instruction_hook(device, cpustate->PC);

		cpustate->opcode.d = M_RDOP(cpustate->PC);
		cpustate->PC++;

		if (cpustate->opcode.b.h != 0x7f)	{ /* Do all opcodes except the 7Fxx ones */
			cpustate->icount -= opcode_main[cpustate->opcode.b.h].cycles;
			(*opcode_main[cpustate->opcode.b.h].function)(cpustate);
		}
		else { /* Opcode major byte 7Fxx has many opcodes in its minor byte */
			cpustate->icount -= opcode_7F[(cpustate->opcode.b.l & 0x1f)].cycles;
			(*opcode_7F[(cpustate->opcode.b.l & 0x1f)].function)(cpustate);
		}
	} while (cpustate->icount > 0);
}



/****************************************************************************
 *  TMS32010 Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( tms32010_ram, AS_DATA, 16, legacy_cpu_device )
	AM_RANGE(0x00, 0x7f) AM_RAM		/* Page 0 */
	AM_RANGE(0x80, 0x8f) AM_RAM		/* Page 1 */
ADDRESS_MAP_END


/****************************************************************************
 *  TMS32010 Reset registers to their initial values
 ****************************************************************************/

static CPU_RESET( tms32010 )
{
	tms32010_state *cpustate = get_safe_token(device);

	cpustate->PC    = 0;
	cpustate->ACC.d = 0;
	cpustate->INTF  = TMS32010_INT_NONE;
	/* Setup Status Register : 7efe */
	CLR(cpustate, (OV_FLAG | ARP_REG | DP_REG));
	SET(cpustate, (OVM_FLAG | INTM_FLAG));

	cpustate->addr_mask = 0x0fff;
}


/**************************************************************************
 *  TMS32010 set_info
 **************************************************************************/

static CPU_SET_INFO( tms32010 )
{
	tms32010_state *cpustate = get_safe_token(device);

	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32010_PC:		cpustate->PC = info->i;			break;
		/* This is actually not a stack pointer, but the stack contents */
		/* Stack is a 4 level First In Last Out stack */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32010_STK3:		cpustate->STACK[3] = info->i;	break;
		case CPUINFO_INT_REGISTER + TMS32010_STR:		cpustate->STR    = info->i;		break;
		case CPUINFO_INT_REGISTER + TMS32010_ACC:		cpustate->ACC.d  = info->i;		break;
		case CPUINFO_INT_REGISTER + TMS32010_PREG:		cpustate->Preg.d = info->i;		break;
		case CPUINFO_INT_REGISTER + TMS32010_TREG:		cpustate->Treg   = info->i;		break;
		case CPUINFO_INT_REGISTER + TMS32010_AR0:		cpustate->AR[0]  = info->i;		break;
		case CPUINFO_INT_REGISTER + TMS32010_AR1:		cpustate->AR[1]  = info->i;		break;
	}
}



/**************************************************************************
 *  TMS32010 get_info
 **************************************************************************/

CPU_GET_INFO( tms32010 )
{
	tms32010_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;

	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(tms32010_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;				break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 4;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 3;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 12;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 5;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = -1;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = (cpustate->INTF & TMS32010_INT_PENDING) ? ASSERT_LINE : CLEAR_LINE;	break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = cpustate->PREVPC;				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32010_PC:		info->i = cpustate->PC;					break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32010_STK3:		info->i = cpustate->STACK[3];			break;
		case CPUINFO_INT_REGISTER + TMS32010_ACC:		info->i = cpustate->ACC.d;				break;
		case CPUINFO_INT_REGISTER + TMS32010_STR:		info->i = cpustate->STR;				break;
		case CPUINFO_INT_REGISTER + TMS32010_PREG:		info->i = cpustate->Preg.d;				break;
		case CPUINFO_INT_REGISTER + TMS32010_TREG:		info->i = cpustate->Treg;				break;
		case CPUINFO_INT_REGISTER + TMS32010_AR0:		info->i = cpustate->AR[0];				break;
		case CPUINFO_INT_REGISTER + TMS32010_AR1:		info->i = cpustate->AR[1];				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(tms32010);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(tms32010);				break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tms32010);				break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(tms32010);				break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(tms32010);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;									break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(tms32010);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;					break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:	info->internal_map16 = ADDRESS_MAP_NAME(tms32010_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32010");					break;
		case CPUINFO_STR_FAMILY:						strcpy(info->s, "Texas Instruments TMS32010");	break;
		case CPUINFO_STR_VERSION:						strcpy(info->s, "1.31");						break;
		case CPUINFO_STR_SOURCE_FILE:					strcpy(info->s, __FILE__);						break;
		case CPUINFO_STR_CREDITS:						strcpy(info->s, "Copyright Tony La Porta");		break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				cpustate->STR & 0x8000 ? 'O':'.',
				cpustate->STR & 0x4000 ? 'M':'.',
				cpustate->STR & 0x2000 ? 'I':'.',
				cpustate->STR & 0x1000 ? '.':'?',
				cpustate->STR & 0x0800 ? 'a':'?',
				cpustate->STR & 0x0400 ? 'r':'?',
				cpustate->STR & 0x0200 ? 'p':'?',
				cpustate->STR & 0x0100 ? '1':'0',
				cpustate->STR & 0x0080 ? '.':'?',
				cpustate->STR & 0x0040 ? '.':'?',
				cpustate->STR & 0x0020 ? '.':'?',
				cpustate->STR & 0x0010 ? '.':'?',
				cpustate->STR & 0x0008 ? '.':'?',
				cpustate->STR & 0x0004 ? 'd':'?',
				cpustate->STR & 0x0002 ? 'p':'?',
				cpustate->STR & 0x0001 ? '1':'0');
			break;

		case CPUINFO_STR_REGISTER + TMS32010_PC:	sprintf(info->s, "PC:%04X",   cpustate->PC); break;
		case CPUINFO_STR_REGISTER + TMS32010_SP:	sprintf(info->s, "SP:%X", 0); /* fake stack pointer */ break;
		case CPUINFO_STR_REGISTER + TMS32010_STR:	sprintf(info->s, "STR:%04X",  cpustate->STR); break;
		case CPUINFO_STR_REGISTER + TMS32010_ACC:	sprintf(info->s, "ACC:%08X",  cpustate->ACC.d); break;
		case CPUINFO_STR_REGISTER + TMS32010_PREG:	sprintf(info->s, "P:%08X",    cpustate->Preg.d); break;
		case CPUINFO_STR_REGISTER + TMS32010_TREG:	sprintf(info->s, "T:%04X",    cpustate->Treg); break;
		case CPUINFO_STR_REGISTER + TMS32010_AR0:	sprintf(info->s, "AR0:%04X",  cpustate->AR[0]); break;
		case CPUINFO_STR_REGISTER + TMS32010_AR1:	sprintf(info->s, "AR1:%04X",  cpustate->AR[1]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK0:	sprintf(info->s, "STK0:%04X", cpustate->STACK[0]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK1:	sprintf(info->s, "STK1:%04X", cpustate->STACK[1]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK2:	sprintf(info->s, "STK2:%04X", cpustate->STACK[2]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK3:	sprintf(info->s, "STK3:%04X", cpustate->STACK[3]); break;
	}
}


/****************************************************************************
 *  TMS32015 Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( tms32015_ram, AS_DATA, 16, tms32015_device )
	AM_RANGE(0x00, 0x7f) AM_RAM		/* Page 0 */
	AM_RANGE(0x80, 0xff) AM_RAM		/* Page 1 */
ADDRESS_MAP_END


/**************************************************************************
 *  TMS32015 CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( tms32015 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map16 = ADDRESS_MAP_NAME(tms32015_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32015");	break;

		default:										CPU_GET_INFO_CALL(tms32010);	break;
	}
}



/****************************************************************************
 *  TMS32016 Reset registers to their initial values
 ****************************************************************************/

static CPU_RESET( tms32016 )
{
	tms32010_state *cpustate = get_safe_token(device);

	CPU_RESET_CALL(tms32010);
	cpustate->addr_mask = 0xffff;
}


/**************************************************************************
 *  TMS32016 CPU-specific get_info
 **************************************************************************/

CPU_GET_INFO( tms32016 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(tms32016);		break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:		info->internal_map16 = ADDRESS_MAP_NAME(tms32015_ram);	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32016");	break;

		default:										CPU_GET_INFO_CALL(tms32010);	break;
	}
}



DEFINE_LEGACY_CPU_DEVICE(TMS32010, tms32010);
DEFINE_LEGACY_CPU_DEVICE(TMS32015, tms32015);
DEFINE_LEGACY_CPU_DEVICE(TMS32016, tms32016);
