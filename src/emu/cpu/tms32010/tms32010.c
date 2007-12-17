 /**************************************************************************\
 *                 Texas Instruments TMS32010 DSP Emulator                  *
 *                                                                          *
 *                  Copyright (C) 1999-2004+ Tony La Porta                  *
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
 *                                                                          *
 \**************************************************************************/



#include "tms32010.h"
#include "debugger.h"


/* 1 cycle equals 4 clock ticks */
#if 0
#define CLK  TMS32010_CLOCK_DIVIDER
#else
#define CLK  1		/* Moved the clock timing back into the driver */
#endif


#ifndef INLINE
#define INLINE static inline
#endif


#define M_RDROM(A)		TMS32010_ROM_RDMEM(A)
#define M_WRTROM(A,V)	TMS32010_ROM_WRMEM(A,V)
#define M_RDRAM(A)		TMS32010_RAM_RDMEM(A)
#define M_WRTRAM(A,V)	TMS32010_RAM_WRMEM(A,V)
#define M_RDOP(A)		TMS32010_RDOP(A)
#define M_RDOP_ARG(A)	TMS32010_RDOP_ARG(A)
#define P_IN(A)			TMS32010_In(A)
#define P_OUT(A,V)		TMS32010_Out(A,V)
#define BIO_IN			TMS32010_BIO_In



typedef struct			/* Page 3-6 shows all registers */
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

	/********************** Status data ****************************/
	PAIR	opcode;
	int		INTF;		/* Pending Interrupt flag */
} tms32010_Regs;

static tms32010_Regs R;
static PAIR oldacc;
static UINT16 memaccess;
static int tms32010_icount;
static int addr_mask;
typedef void (*opcode_fn) (void);


/********  The following is the Status (Flag) register definition.  *********/
/* 15 | 14  |  13  | 12 | 11 | 10 | 9 |  8  | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0  */
/* OV | OVM | INTM |  1 |  1 |  1 | 1 | ARP | 1 | 1 | 1 | 1 | 1 | 1 | 1 | DP */
#define OV_FLAG		0x8000	/* OV   (Overflow flag) 1 indicates an overflow */
#define OVM_FLAG	0x4000	/* OVM  (Overflow Mode bit) 1 forces ACC overflow to greatest positive or negative saturation value */
#define INTM_FLAG	0x2000	/* INTM (Interrupt Mask flag) 0 enables maskable interrupts */
#define ARP_REG		0x0100	/* ARP  (Auxiliary Register Pointer) */
#define DP_REG		0x0001	/* DP   (Data memory Pointer (bank) bit) */

#define OV		( R.STR & OV_FLAG)			/* OV   (Overflow flag) */
#define OVM		( R.STR & OVM_FLAG)			/* OVM  (Overflow Mode bit) 1 indicates an overflow */
#define INTM	( R.STR & INTM_FLAG)		/* INTM (Interrupt enable flag) 0 enables maskable interrupts */
#define ARP		((R.STR & ARP_REG) >> 8)	/* ARP  (Auxiliary Register Pointer) */
#define DP		((R.STR & DP_REG) << 7)		/* DP   (Data memory Pointer bit) */

#define DMA_DP	(DP | (R.opcode.b.l & 0x7f))	/* address used in direct memory access operations */
#define DMA_DP1	(0x80 | R.opcode.b.l)			/* address used in direct memory access operations for sst instruction */
#define IND		(R.AR[ARP] & 0xff)				/* address used in indirect memory access operations */




/************************************************************************
 *  Shortcuts
 ************************************************************************/

INLINE void CLR(UINT16 flag) { R.STR &= ~flag; R.STR |= 0x1efe; }
INLINE void SET(UINT16 flag) { R.STR |=  flag; R.STR |= 0x1efe; }


INLINE void CALCULATE_ADD_OVERFLOW(INT32 addval)
{
	if ((INT32)(~(oldacc.d ^ addval) & (oldacc.d ^ R.ACC.d)) < 0) {
		SET(OV_FLAG);
		if (OVM)
			R.ACC.d = ((INT32)oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}
INLINE void CALCULATE_SUB_OVERFLOW(INT32 subval)
{
	if ((INT32)((oldacc.d ^ subval) & (oldacc.d ^ R.ACC.d)) < 0) {
		SET(OV_FLAG);
		if (OVM)
			R.ACC.d = ((INT32)oldacc.d < 0) ? 0x80000000 : 0x7fffffff;
	}
}

INLINE UINT16 POP_STACK(void)
{
	UINT16 data = R.STACK[3];
	R.STACK[3] = R.STACK[2];
	R.STACK[2] = R.STACK[1];
	R.STACK[1] = R.STACK[0];
	return (data & addr_mask);
}
INLINE void PUSH_STACK(UINT16 data)
{
	R.STACK[0] = R.STACK[1];
	R.STACK[1] = R.STACK[2];
	R.STACK[2] = R.STACK[3];
	R.STACK[3] = (data & addr_mask);
}

INLINE void GET_MEM_ADDR(UINT16 DMA)
{
	if (R.opcode.b.l & 0x80)
		memaccess = IND;
	else
		memaccess = DMA;
}
INLINE void UPDATE_AR(void)
{
	if (R.opcode.b.l & 0x30) {
		UINT16 tmpAR = R.AR[ARP];
		if (R.opcode.b.l & 0x20) tmpAR++ ;
		if (R.opcode.b.l & 0x10) tmpAR-- ;
		R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (tmpAR & 0x01ff);
	}
}
INLINE void UPDATE_ARP(void)
{
	if (~R.opcode.b.l & 0x08) {
		if (R.opcode.b.l & 0x01) SET(ARP_REG);
		else CLR(ARP_REG);
	}
}


INLINE void getdata(UINT8 shift,UINT8 signext)
{
	GET_MEM_ADDR(DMA_DP);
	R.ALU.d = (UINT16)M_RDRAM(memaccess);
	if (signext) R.ALU.d = (INT16)R.ALU.d;
	R.ALU.d <<= shift;
	if (R.opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
}

INLINE void putdata(UINT16 data)
{
	GET_MEM_ADDR(DMA_DP);
	if (R.opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
	M_WRTRAM(memaccess,data);
}
INLINE void putdata_sar(UINT8 data)
{
	GET_MEM_ADDR(DMA_DP);
	if (R.opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
	M_WRTRAM(memaccess,R.AR[data]);
}
INLINE void putdata_sst(UINT16 data)
{
	GET_MEM_ADDR(DMA_DP1);		/* Page 1 only */
	if (R.opcode.b.l & 0x80) {
		UPDATE_AR();
	}
	M_WRTRAM(memaccess,data);
}



/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/* This following function is here to fill in the void for */
/* the opcode call function. This function is never called. */

static void other_7F_opcodes(void)  { }


static void illegal(void)
{
		logerror("TMS32010:  PC=%04x,  Illegal opcode = %04x\n", (R.PC-1), R.opcode.w.l);
}

static void abst(void)
{
		if ( (INT32)(R.ACC.d) < 0 ) {
			R.ACC.d = -R.ACC.d;
			if (OVM && (R.ACC.d == 0x80000000)) R.ACC.d-- ;
		}
}

/*** The manual does not mention overflow with the ADD? instructions *****
 *** however I implemented overflow, coz it doesnt make sense otherwise **
 *** and newer generations of this type of chip supported it. I think ****
 *** the manual is wrong (apart from other errors the manual has). *******

static void add_sh(void)    { getdata(R.opcode.b.h,1); R.ACC.d += R.ALU.d; }
static void addh(void)      { getdata(0,0); R.ACC.d += (R.ALU.d << 16); }
 ***/

static void add_sh(void)
{
	oldacc.d = R.ACC.d;
	getdata((R.opcode.b.h & 0xf),1);
	R.ACC.d += R.ALU.d;
	CALCULATE_ADD_OVERFLOW(R.ALU.d);
}
static void addh(void)
{
	oldacc.d = R.ACC.d;
	getdata(0,0);
	R.ACC.w.h += R.ALU.w.l;
	if ((INT16)(~(oldacc.w.h ^ R.ALU.w.h) & (oldacc.w.h ^ R.ACC.w.h)) < 0) {
		SET(OV_FLAG);
		if (OVM)
			R.ACC.w.h = ((INT16)oldacc.w.h < 0) ? 0x8000 : 0x7fff;
	}
}
static void adds(void)
{
	oldacc.d = R.ACC.d;
	getdata(0,0);
	R.ACC.d += R.ALU.d;
	CALCULATE_ADD_OVERFLOW(R.ALU.d);
}
static void and(void)
{
	getdata(0,0);
	R.ACC.d &= R.ALU.d;
}
static void apac(void)
{
	oldacc.d = R.ACC.d;
	R.ACC.d += R.Preg.d;
	CALCULATE_ADD_OVERFLOW(R.Preg.d);
}
static void br(void)
{
	R.PC = M_RDOP_ARG(R.PC);
}
static void banz(void)
{
	if (R.AR[ARP] & 0x01ff)
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
	R.ALU.w.l = R.AR[ARP];
	R.ALU.w.l-- ;
	R.AR[ARP] = (R.AR[ARP] & 0xfe00) | (R.ALU.w.l & 0x01ff);
}
static void bgez(void)
{
	if ( (INT32)(R.ACC.d) >= 0 )
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
}
static void bgz(void)
{
	if ( (INT32)(R.ACC.d) > 0 )
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
}
static void bioz(void)
{
	if (BIO_IN != CLEAR_LINE)
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
}
static void blez(void)
{
	if ( (INT32)(R.ACC.d) <= 0 )
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
}
static void blz(void)
{
	if ( (INT32)(R.ACC.d) <  0 )
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
}
static void bnz(void)
{
	if (R.ACC.d != 0)
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
}
static void bv(void)
{
	if (OV) {
		R.PC = M_RDOP_ARG(R.PC);
		CLR(OV_FLAG);
	}
	else
		R.PC++ ;
}
static void bz(void)
{
	if (R.ACC.d == 0)
		R.PC = M_RDOP_ARG(R.PC);
	else
		R.PC++ ;
}
static void cala(void)
{
	PUSH_STACK(R.PC);
	R.PC = R.ACC.w.l & addr_mask;
}
static void call(void)
{
	R.PC++ ;
	PUSH_STACK(R.PC);
	R.PC = M_RDOP_ARG((R.PC - 1)) & addr_mask;
}
static void dint(void)
{
	SET(INTM_FLAG);
}
static void dmov(void)
{
	getdata(0,0);
	M_WRTRAM((memaccess + 1),R.ALU.w.l);
}
static void eint(void)
{
	CLR(INTM_FLAG);
}
static void in_p(void)
{
	R.ALU.w.l = P_IN( (R.opcode.b.h & 7) );
	putdata(R.ALU.w.l);
}
static void lac_sh(void)
{
	getdata((R.opcode.b.h & 0x0f),1);
	R.ACC.d = R.ALU.d;
}
static void lack(void)
{
	R.ACC.d = R.opcode.b.l;
}
static void lar_ar0(void)
{
	getdata(0,0);
	R.AR[0] = R.ALU.w.l;
}
static void lar_ar1(void)
{
	getdata(0,0);
	R.AR[1] = R.ALU.w.l;
}
static void lark_ar0(void)
{
	R.AR[0] = R.opcode.b.l;
}
static void lark_ar1(void)
{
	R.AR[1] = R.opcode.b.l;
}
static void larp_mar(void)
{
	if (R.opcode.b.l & 0x80) {
		UPDATE_AR();
		UPDATE_ARP();
	}
}
static void ldp(void)
{
	getdata(0,0);
	if (R.ALU.d & 1)
		SET(DP_REG);
	else
		CLR(DP_REG);
}
static void ldpk(void)
{
	if (R.opcode.b.l & 1)
		SET(DP_REG);
	else
		CLR(DP_REG);
}
static void lst(void)
{
	R.opcode.b.l |= 0x08; /* Next arp not supported here, so mask it */
	getdata(0,0);
	R.ALU.w.l &= (~INTM_FLAG);	/* Must not affect INTM */
	R.STR &= INTM_FLAG;
	R.STR |= R.ALU.w.l;
	R.STR |= 0x1efe;
}
static void lt(void)
{
	getdata(0,0);
	R.Treg = R.ALU.w.l;
}
static void lta(void)
{
	oldacc.d = R.ACC.d;
	getdata(0,0);
	R.Treg = R.ALU.w.l;
	R.ACC.d += R.Preg.d;
	CALCULATE_ADD_OVERFLOW(R.Preg.d);
}
static void ltd(void)
{
	oldacc.d = R.ACC.d;
	getdata(0,0);
	R.Treg = R.ALU.w.l;
	M_WRTRAM((memaccess + 1),R.ALU.w.l);
	R.ACC.d += R.Preg.d;
	CALCULATE_ADD_OVERFLOW(R.Preg.d);
}
static void mpy(void)
{
	getdata(0,0);
	R.Preg.d = (INT16)R.ALU.w.l * (INT16)R.Treg;
	if (R.Preg.d == 0x40000000) R.Preg.d = 0xc0000000;
}
static void mpyk(void)
{
	R.Preg.d = (INT16)R.Treg * ((INT16)(R.opcode.w.l << 3) >> 3);
}
static void nop(void)
{
	/* Nothing to do */
}
static void or(void)
{
	getdata(0,0);
	R.ACC.w.l |= R.ALU.w.l;
}
static void out_p(void)
{
	getdata(0,0);
	P_OUT( (R.opcode.b.h & 7), R.ALU.w.l );
}
static void pac(void)
{
	R.ACC.d = R.Preg.d;
}
static void pop(void)
{
	R.ACC.w.l = POP_STACK();
	R.ACC.w.h = 0x0000;
}
static void push(void)
{
	PUSH_STACK(R.ACC.w.l);
}
static void ret(void)
{
	R.PC = POP_STACK();
}
static void rovm(void)
{
	CLR(OVM_FLAG);
}
static void sach_sh(void)
{
	R.ALU.d = (R.ACC.d << (R.opcode.b.h & 7));
	putdata(R.ALU.w.h);
}
static void sacl(void)
{
	putdata(R.ACC.w.l);
}
static void sar_ar0(void)
{
	putdata_sar(0);
}
static void sar_ar1(void)
{
	putdata_sar(1);
}
static void sovm(void)
{
	SET(OVM_FLAG);
}
static void spac(void)
{
	oldacc.d = R.ACC.d;
	R.ACC.d -= R.Preg.d;
	CALCULATE_SUB_OVERFLOW(R.Preg.d);
}
static void sst(void)
{
	putdata_sst(R.STR);
}
static void sub_sh(void)
{
	oldacc.d = R.ACC.d;
	getdata((R.opcode.b.h & 0x0f),1);
	R.ACC.d -= R.ALU.d;
	CALCULATE_SUB_OVERFLOW(R.ALU.d);
}
static void subc(void)
{
	oldacc.d = R.ACC.d;
	getdata(15,0);
	R.ALU.d = (INT32) R.ACC.d - R.ALU.d;
	if ((INT32)((oldacc.d ^ R.ALU.d) & (oldacc.d ^ R.ACC.d)) < 0)
	    SET(OV_FLAG);
	if ( (INT32)(R.ALU.d) >= 0 )
	    R.ACC.d = ((R.ALU.d << 1) + 1);
	else
	    R.ACC.d = (R.ACC.d << 1);
}
static void subh(void)
{
	oldacc.d = R.ACC.d;
	getdata(16,0);
	R.ACC.d -= R.ALU.d;
	CALCULATE_SUB_OVERFLOW(R.ALU.d);
}
static void subs(void)
{
	oldacc.d = R.ACC.d;
	getdata(0,0);
	R.ACC.d -= R.ALU.d;
	CALCULATE_SUB_OVERFLOW(R.ALU.d);
}
static void tblr(void)
{
	R.ALU.d = M_RDROM((R.ACC.w.l & addr_mask));
	putdata(R.ALU.w.l);
	R.STACK[0] = R.STACK[1];
}
static void tblw(void)
{
	getdata(0,0);
	M_WRTROM(((R.ACC.w.l & addr_mask)),R.ALU.w.l);
	R.STACK[0] = R.STACK[1];
}
static void xor(void)
{
	getdata(0,0);
	R.ACC.w.l ^= R.ALU.w.l;
}
static void zac(void)
{
	R.ACC.d = 0;
}
static void zalh(void)
{
	getdata(0,0);
	R.ACC.w.h = R.ALU.w.l;
	R.ACC.w.l = 0x0000;
}
static void zals(void)
{
	getdata(0,0);
	R.ACC.w.l = R.ALU.w.l;
	R.ACC.w.h = 0x0000;
}


/***********************************************************************
 *  Cycle Timings
 ***********************************************************************/

static unsigned cycles_main[256]=
{
/*00*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*10*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*20*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*30*/	1*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 1*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*40*/	2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK,
/*50*/	1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*60*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 3*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*70*/	1*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 3*CLK, 1*CLK, 0*CLK,
/*80*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*90*/	1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK,
/*A0*/	0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*B0*/	0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*C0*/	0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*D0*/	0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*E0*/	0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK,
/*F0*/	0*CLK, 0*CLK, 0*CLK, 0*CLK, 2*CLK, 2*CLK, 2*CLK, 0*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK, 2*CLK
};

static unsigned cycles_7F_other[32]=
{
/*80*/	1*CLK, 1*CLK, 1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 1*CLK, 1*CLK, 1*CLK, 1*CLK, 2*CLK, 2*CLK, 1*CLK, 1*CLK,
/*90*/	1*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 0*CLK, 2*CLK, 2*CLK, 0*CLK, 0*CLK
};


/***********************************************************************
 *  Opcode Table
 ***********************************************************************/

static opcode_fn opcode_main[256]=
{
/*00*/  add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh
/*08*/ ,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh		,add_sh
/*10*/ ,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh
/*18*/ ,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh		,sub_sh
/*20*/ ,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh
/*28*/ ,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh		,lac_sh
/*30*/ ,sar_ar0		,sar_ar1	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*38*/ ,lar_ar0		,lar_ar1	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*40*/ ,in_p		,in_p		,in_p		,in_p		,in_p		,in_p		,in_p		,in_p
/*48*/ ,out_p		,out_p		,out_p		,out_p		,out_p		,out_p		,out_p		,out_p
/*50*/ ,sacl		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*58*/ ,sach_sh		,sach_sh	,sach_sh	,sach_sh	,sach_sh	,sach_sh	,sach_sh	,sach_sh
/*60*/ ,addh		,adds		,subh		,subs		,subc		,zalh		,zals		,tblr
/*68*/ ,larp_mar	,dmov		,lt			,ltd		,lta		,mpy		,ldpk		,ldp
/*70*/ ,lark_ar0	,lark_ar1	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*78*/ ,xor			,and		,or			,lst		,sst		,tblw		,lack		,other_7F_opcodes
/*80*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*88*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*90*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*98*/ ,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk		,mpyk
/*A0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*A8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*B0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*B8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*C0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*C8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*D0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*D8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*E0*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*E8*/ ,illegal		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*F0*/ ,illegal		,illegal	,illegal	,illegal	,banz		,bv			,bioz		,illegal
/*F8*/ ,call		,br			,blz		,blez		,bgz		,bgez		,bnz		,bz
};

static opcode_fn opcode_7F_other[32]=
{
/*80*/  nop			,dint		,eint		,illegal	,illegal	,illegal	,illegal	,illegal
/*88*/ ,abst		,zac		,rovm		,sovm		,cala		,ret		,pac		,apac
/*90*/ ,spac		,illegal	,illegal	,illegal	,illegal	,illegal	,illegal	,illegal
/*98*/ ,illegal		,illegal	,illegal	,illegal	,push		,pop		,illegal	,illegal
};



/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/
static void tms32010_init (int index, int clock, const void *config, int (*irqcallback)(int))
{
	state_save_register_item("tms32010", index, R.PC);
	state_save_register_item("tms32010", index, R.PREVPC);
	state_save_register_item("tms32010", index, R.STR);
	state_save_register_item("tms32010", index, R.ACC.d);
	state_save_register_item("tms32010", index, R.ALU.d);
	state_save_register_item("tms32010", index, R.Preg.d);
	state_save_register_item("tms32010", index, R.Treg);
	state_save_register_item("tms32010", index, R.AR[0]);
	state_save_register_item("tms32010", index, R.AR[1]);
	state_save_register_item("tms32010", index, R.STACK[0]);
	state_save_register_item("tms32010", index, R.STACK[1]);
	state_save_register_item("tms32010", index, R.STACK[2]);
	state_save_register_item("tms32010", index, R.STACK[3]);
	state_save_register_item("tms32010", index, R.INTF);
	state_save_register_item("tms32010", index, R.opcode.d);
}

/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/
static void tms32010_reset (void)
{
	R.PC    = 0;
	R.STR   = 0x7efe;	// OV cleared
	R.ACC.d = 0;
	R.INTF  = TMS32010_INT_NONE;
	addr_mask = 0x0fff;	/* TMS32010 can only address 0x0fff */
						/* however other TMS3201x devices   */
						/* can address up to 0xffff (incase */
						/* their support is ever added).    */
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/
static void tms32010_exit (void)
{
	/* nothing to do ? */
}


/****************************************************************************
 *  Issue an interrupt if necessary
 ****************************************************************************/
static int Ext_IRQ(void)
{
	if (INTM == 0)
	{
		logerror("TMS32010:  EXT INTERRUPT\n");
		R.INTF = TMS32010_INT_NONE;
		SET(INTM_FLAG);
		PUSH_STACK(R.PC);
		R.PC = 0x0002;
		return (3*CLK);	/* 3 cycles used due to PUSH and DINT operation ? */
	}
	return (0*CLK);
}



/****************************************************************************
 *  Execute IPeriod. Return 0 if emulation should be stopped
 ****************************************************************************/
static int tms32010_execute(int cycles)
{
	tms32010_icount = cycles;

	do
	{
		if (R.INTF) {
			/* Dont service INT if prev instruction was MPY, MPYK or EINT */
			if ((R.opcode.b.h != 0x6d) && ((R.opcode.b.h & 0xe0) != 0x80) && (R.opcode.w.l != 0x7f82))
				tms32010_icount -= Ext_IRQ();
		}

		R.PREVPC = R.PC;

		CALL_MAME_DEBUG;

		R.opcode.d = M_RDOP(R.PC);
		R.PC++;

		if (R.opcode.b.h != 0x7f)	{ /* Do all opcodes except the 7Fxx ones */
			tms32010_icount -= cycles_main[R.opcode.b.h];
			(*(opcode_main[R.opcode.b.h]))();
		}
		else { /* Opcode major byte 7Fxx has many opcodes in its minor byte */
			tms32010_icount -= cycles_7F_other[(R.opcode.b.l & 0x1f)];
			(*(opcode_7F_other[(R.opcode.b.l & 0x1f)]))();
		}
	} while (tms32010_icount>0);

	return cycles - tms32010_icount;
}

/****************************************************************************
 *  Get all registers in given buffer
 ****************************************************************************/
static void tms32010_get_context (void *dst)
{
	if( dst )
		*(tms32010_Regs*)dst = R;
}

/****************************************************************************
 *  Set all registers to given values
 ****************************************************************************/
static void tms32010_set_context (void *src)
{
	if (src)
		R = *(tms32010_Regs*)src;
}


/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/
static void set_irq_line(int irqline, int state)
{
	/* Pending Interrupts cannot be cleared! */
	if (state == ASSERT_LINE) R.INTF |=  TMS32010_INT_PENDING;
}


/****************************************************************************
 *  Internal Memory Map
 ****************************************************************************/

static ADDRESS_MAP_START( tms32010_ram, ADDRESS_SPACE_DATA, 16 )
	AM_RANGE(0x00, 0x7f) AM_RAM		/* Page 0 */
	AM_RANGE(0x80, 0x8f) AM_RAM		/* Page 1 */
ADDRESS_MAP_END


/**************************************************************************
 *  Generic set_info
 **************************************************************************/

static void tms32010_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(0, info->i);				break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32010_PC:		R.PC = info->i;							break;
		/* This is actually not a stack pointer, but the stack contents */
		/* Stack is a 4 level First In Last Out stack */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32010_STK3:		R.STACK[3] = info->i;					break;
		case CPUINFO_INT_REGISTER + TMS32010_STR:		R.STR    = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32010_ACC:		R.ACC.d  = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32010_PREG:		R.Preg.d = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32010_TREG:		R.Treg   = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32010_AR0:		R.AR[0]  = info->i;						break;
		case CPUINFO_INT_REGISTER + TMS32010_AR1:		R.AR[1]  = info->i;						break;
	}
}



/**************************************************************************
 *  Generic get_info
 **************************************************************************/

void tms32010_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(R);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 1;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = TMS32010_CLOCK_DIVIDER;		break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1*CLK;						break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 3*CLK;						break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 12;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = -1;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 5;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = -1;					break;

		case CPUINFO_INT_INPUT_STATE + 0:				info->i = (R.INTF & TMS32010_INT_PENDING) ? ASSERT_LINE : CLEAR_LINE;	break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = R.PREVPC;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + TMS32010_PC:		info->i = R.PC;							break;
		/* This is actually not a stack pointer, but the stack contents */
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + TMS32010_STK3:		info->i = R.STACK[3];					break;
		case CPUINFO_INT_REGISTER + TMS32010_ACC: 		info->i = R.ACC.d;						break;
		case CPUINFO_INT_REGISTER + TMS32010_STR: 		info->i = R.STR;						break;
		case CPUINFO_INT_REGISTER + TMS32010_PREG:		info->i = R.Preg.d;						break;
		case CPUINFO_INT_REGISTER + TMS32010_TREG:		info->i = R.Treg;						break;
		case CPUINFO_INT_REGISTER + TMS32010_AR0: 		info->i = R.AR[0];						break;
		case CPUINFO_INT_REGISTER + TMS32010_AR1: 		info->i = R.AR[1];						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = tms32010_set_info;		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = tms32010_get_context; break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = tms32010_set_context; break;
		case CPUINFO_PTR_INIT:							info->init = tms32010_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = tms32010_reset;			break;
		case CPUINFO_PTR_EXIT:							info->exit = tms32010_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = tms32010_execute;		break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = tms32010_dasm;		break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &tms32010_icount;		break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_DATA:	info->internal_map = construct_map_tms32010_ram; break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "TMS32010");			break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Texas Instruments TMS32010"); break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.22");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (C)1999-2004+ by Tony La Porta"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
				R.STR & 0x8000 ? 'O':'.',
				R.STR & 0x4000 ? 'M':'.',
				R.STR & 0x2000 ? 'I':'.',
				R.STR & 0x1000 ? '.':'?',
				R.STR & 0x0800 ? 'a':'?',
				R.STR & 0x0400 ? 'r':'?',
				R.STR & 0x0200 ? 'p':'?',
				R.STR & 0x0100 ? '1':'0',
				R.STR & 0x0080 ? '.':'?',
				R.STR & 0x0040 ? '.':'?',
				R.STR & 0x0020 ? '.':'?',
				R.STR & 0x0010 ? '.':'?',
				R.STR & 0x0008 ? '.':'?',
				R.STR & 0x0004 ? 'd':'?',
				R.STR & 0x0002 ? 'p':'?',
				R.STR & 0x0001 ? '1':'0');
			break;

		case CPUINFO_STR_REGISTER + TMS32010_PC:   	sprintf(info->s, "PC:%04X",   R.PC); break;
		case CPUINFO_STR_REGISTER + TMS32010_SP:   	sprintf(info->s, "SP:%X", 0); /* fake stack pointer */ break;
		case CPUINFO_STR_REGISTER + TMS32010_STR:  	sprintf(info->s, "STR:%04X",  R.STR); break;
		case CPUINFO_STR_REGISTER + TMS32010_ACC:  	sprintf(info->s, "ACC:%08X",  R.ACC.d); break;
		case CPUINFO_STR_REGISTER + TMS32010_PREG: 	sprintf(info->s, "P:%08X",    R.Preg.d); break;
		case CPUINFO_STR_REGISTER + TMS32010_TREG: 	sprintf(info->s, "T:%04X",    R.Treg); break;
		case CPUINFO_STR_REGISTER + TMS32010_AR0:  	sprintf(info->s, "AR0:%04X",  R.AR[0]); break;
		case CPUINFO_STR_REGISTER + TMS32010_AR1:  	sprintf(info->s, "AR1:%04X",  R.AR[1]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK0: 	sprintf(info->s, "STK0:%04X", R.STACK[0]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK1: 	sprintf(info->s, "STK1:%04X", R.STACK[1]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK2: 	sprintf(info->s, "STK2:%04X", R.STACK[2]); break;
		case CPUINFO_STR_REGISTER + TMS32010_STK3: 	sprintf(info->s, "STK3:%04X", R.STACK[3]); break;
	}
}
