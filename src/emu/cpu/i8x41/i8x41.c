/*****************************************************************************
 *
 *   i8x41.c
 *   Portable UPI-41/8041/8741/8042/8742 emulator V0.2
 *
 *   Copyright (c) 1999 Juergen Buchmueller, all rights reserved.
 *
 *   Copyright (C) 1998,1999,2000 Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 *  This work is solely based on the
 *  'Intel(tm) UPI(tm)-41AH/42AH Users Manual'
 *
 *
 *  **** Change Log ****
 *  HJB (19-Dec-2004) changed version to 0.3
 *   - Tried to handle accesses to registers in get_info/set_info
 *     before i8x41.ram is is initialized.
 *   - cosmetics: readability in get_info/set_info, replaced non-ASCII
 *     codes in comments, add 'ex' tabstop definition
 *
 *  TLP (10-Jan-2003) Changed ver from 0.1 to 0.2
 *   - Changed the internal RAM mask from 3Fh to FFh . The i8x41/i8x42 have
 *     128/256 bytes of internal RAM respectively.
 *   - Added output port data to the debug register view window.
 *   - Added some missing break commands to the set_reg switch function.
 *   - Changed Ports 1 and 2 to latched types (Quasi-bidirectional).
 *   - Stopped illegal access to Port 0 and 3 (they don't exist).
 *   - Changed ANLD, ORLD and MOVD instructions to act through Port 2 in
 *     nibble mode.
 *   - Copied F0 and moved F1 flags to the STATE flag bits where they belong.
 *   - Corrected the 'addr' field by changing it from UINT8 to UINT16 for:
 *     'INC @Rr' 'MOV @Rr,A' 'MOV @Rr,#N' 'XCH A,@Rr' 'XCHD A,@Rr'
 *   - Added mask to TIMER when the TEST1 Counter overflows.
 *   - Seperated the prescaler out of the timer/counter, in order to correct
 *     the TEST1 input counter step.
 *   - Moved TEST0 and TEST1 status flags out of the STATE register.
 *     STATE register uses these upper bits for user definable purposes.
 *   - TEST0 and TEST1 input lines are now sampled during the JTx/JNTx
 *     instructions.
 *   - Two methods for updating TEST1 input during counter mode are now
 *     supported depending on the mode of use required.
 *     You can use the Interrupt method, or input port read method.
 *   - TIMER is now only controlled by the timer or counter (not both)
 *     ie, When Starting the Counter, Stop the Timer and viceversa.
 *   - Nested IRQs of any sort are no longer allowed, however IRQs can
 *     become pending while a current interrupt is being serviced.
 *   - IBF Interrupt now has priority over the Timer Interrupt, when they
 *     occur simultaneously.
 *   - Add the external Interrupt FLAGS (Port 24, Port 25).
 *  To Do:
 *   - Add the external DMA FLAGS (Port 26, Port 27).  Page 4 and 37
 *
 *****************************************************************************/

#include "debugger.h"
#include "i8x41.h"

typedef struct {
	UINT16	ppc;
	UINT16	pc;
	UINT8	timer;
	UINT8	prescaler;
	UINT16	subtype;
	UINT8	a;
	UINT8	psw;
	UINT8	state;
	UINT8	enable;
	UINT8	control;
	UINT8	dbbi;
	UINT8	dbbo;
	UINT8	p1;
	UINT8	p2;
	UINT8	p2_hs;
	UINT8	*ram;
	int 	(*irq_callback)(int irqline);
}	I8X41;

int i8x41_ICount;

static I8X41 i8x41;

#define RM(a)	program_read_byte_8(a)
#define WM(a,v) program_write_byte_8(a,v)
#define RP(a)	io_read_byte_8(a)
#define WP(a,v) io_write_byte_8(a,v)
#define ROP(pc) cpu_readop(pc)
#define ROP_ARG(pc) cpu_readop_arg(pc)

/* PC vectors */
#define V_RESET 0x000	/* power on address */
#define V_IBF	0x003	/* input buffer full interrupt vector */
#define V_TIMER 0x007	/* timer/counter interrupt vector */

/*
 * Memory locations
 * Note:
 * 000-3ff      internal ROM for 8x41 (1K)
 * 400-7ff      (more) internal for 8x42 type (2K)
 * 800-8ff      internal RAM
 */
#define M_IRAM	0x800	/* internal RAM is mapped here */
#define M_BANK0 0x800	/* register bank 0 (8 times 8 bits) */
#define M_STACK 0x808	/* stack (8 times 16 bits) */
#define M_BANK1 0x818	/* register bank 1 (8 times 8 bits) */
#define M_USER	0x820	/* user memory (224 times 8 bits) */

/* PSW flag bits */
#define FC		0x80	/* carry flag */
#define FA		0x40	/* auxiliary carry flag */
#define Ff0		0x20	/* flag 0 - same flag as F0 below */
#define BS		0x10	/* bank select */
#define FU		0x08	/* unused */
#define SP		0x07	/* lower three bits are used as stack pointer */

/* STATE flag bits */
#define OBF 	0x01	/* output buffer full */
#define IBF 	0x02	/* input buffer full */
#define F0		0x04	/* flag 0 - same flag as Ff0 above */
#define F1		0x08	/* flag 1 */

/* ENABLE flag bits */
#define IBFI	0x01	/* input buffer full interrupt */
#define TCNTI	0x02	/* timer/counter interrupt */
#define DMA 	0x04	/* DMA mode */
#define FLAGS	0x08	/* FLAGS mode */
#define T		0x10	/* timer */
#define CNT 	0x20	/* counter */

/* CONTROL flag bits */
#define IBFI_EXEC	0x01	/* IBFI is currently being serviced */
#define IBFI_PEND	0x02	/* IBFI is pending */
#define TIRQ_EXEC	0x04	/* Timer interrupt is currently being serviced */
#define TIRQ_PEND	0x08	/* Timer interrupt is pending */
#define TEST1		0x10	/* Test1 line mode */
#define TOVF		0x20	/* Timer Overflow Flag */

#define IRQ_EXEC	0x05	/* Mask for IRQs being serviced */
#define IRQ_PEND	0x0a	/* Mask for IRQs pending */


/* shorter names for the I8x41 structure elements */
#define PPC 		i8x41.ppc
#define PC			i8x41.pc
#define A			i8x41.a
#define PSW 		i8x41.psw
#define DBBI		i8x41.dbbi
#define DBBO		i8x41.dbbo
#define R(n)		i8x41.ram[((PSW & BS) ? M_BANK1:M_BANK0)+(n)]
#define STATE		i8x41.state
#define ENABLE		i8x41.enable
#define TIMER		i8x41.timer
#define PRESCALER	i8x41.prescaler
#define P1			i8x41.p1
#define P2			i8x41.p2
#define P2_HS		i8x41.p2_hs		/* Port 2 Hand Shaking */
#define CONTROL		i8x41.control


static void set_irq_line(int irqline, int state);


/************************************************************************
 *  Shortcuts
 ************************************************************************/

INLINE void PUSH_PC_TO_STACK(void)
{
	WM( M_STACK + (PSW&SP) * 2 + 0, PC & 0xff);
	WM( M_STACK + (PSW&SP) * 2 + 1, ((PC >> 8) & 0x0f) | (PSW & 0xf0) );
	PSW = (PSW & ~SP) | ((PSW + 1) & SP);
}


/************************************************************************
 *  Emulate the Instructions
 ************************************************************************/

/***********************************
 *  illegal opcodes
 ***********************************/
INLINE void illegal(void)
{
	logerror("i8x41 #%d: illegal opcode at 0x%03x: %02x\n", cpu_getactivecpu(), PC, ROP(PC));
}

/***********************************
 *  0110 1rrr *  ADD     A,Rr
 ***********************************/
INLINE void add_r(int r)
{
	UINT8 res = A + R(r);
	if( res < A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0110 000r
 *  ADD     A,@Rr
 ***********************************/
INLINE void add_rm(int r)
{
	UINT8 res = A + RM( M_IRAM + (R(r) & I8X42_intRAM_MASK) );
	if( res < A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0000 0011 7654 3210
 *  ADD     A,#n
 ***********************************/
INLINE void add_i(void)
{
	UINT8 res = A + ROP_ARG(PC);
	PC++;
	if( res < A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0111 1rrr
 *  ADDC    A,Rr
 ***********************************/
INLINE void addc_r(int r)
{
	UINT8 res = A + R(r) + (PSW >> 7);
	if( res <= A ) PSW |= FC;
	if( (res & 0x0f) <= (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0111 000r
 *  ADDC    A,@Rr
 ***********************************/
INLINE void addc_rm(int r)
{
	UINT8 res = A + RM( M_IRAM + (R(r) & I8X42_intRAM_MASK) ) + (PSW >> 7);
	if( res <= A ) PSW |= FC;
	if( (res & 0x0f) <= (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0001 0011 7654 3210
 *  ADDC    A,#n
 ***********************************/
INLINE void addc_i(void)
{
	UINT8 res = A + ROP_ARG(PC);
	PC++;
	if( res < A ) PSW |= FC;
	if( (res & 0x0f) < (A & 0x0f) ) PSW |= FA;
	A = res;
}

/***********************************
 *  0101 1rrr
 *  ANL     A,Rr
 ***********************************/
INLINE void anl_r(int r)
{
	A = A & R(r);
}

/***********************************
 *  0101 000r
 *  ANL     A,@Rr
 ***********************************/
INLINE void anl_rm(int r)
{
	A = A & RM( M_IRAM + (R(r) & I8X42_intRAM_MASK) );
}

/***********************************
 *  0101 0011 7654 3210
 *  ANL     A,#n
 ***********************************/
INLINE void anl_i(void)
{
	A = A & ROP_ARG(PC);
	PC++;
}

/***********************************
 *  1001 10pp 7654 3210
 *  ANL     Pp,#n
 ***********************************/
INLINE void anl_p_i(int p)
{
	UINT8 val = ROP_ARG(PC);
	PC++;
	/* changed to latched port scheme */
	switch (p)
	{
		case 00:	break;	/* invalid port */
		case 01:	P1 &= val; WP(p, P1); break;
		case 02:	P2 &= val; WP(p, (P2 & P2_HS) ); break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  1001 11pp 7654 3210
 *  ANLD    Pp,A
 ***********************************/
INLINE void anld_p_a(int p)
{
	/* added proper expanded port setup */
	WP(2, (P2 & 0xf0) | 0x0c | p); /* AND mode */
	WP(I8X41_ps, 0);	/* activate command strobe */
	WP(2, (A & 0x0f)); 	/* Expander to take care of AND function */
	WP(I8X41_ps, 1);	/* release command strobe */
}

/***********************************
 *  aaa1 0100 7654 3210
 *  CALL    addr
 ***********************************/
INLINE void call_i(int page)
{
	UINT8 adr = ROP_ARG(PC);
	PC++;
	PUSH_PC_TO_STACK();
	PC = page | adr;
}

/***********************************
 *  0010 0111
 *  CLR     A
 ***********************************/
INLINE void clr_a(void)
{
	A = 0;
}

/***********************************
 *  1001 0111
 *  CLR     C
 ***********************************/
INLINE void clr_c(void)
{
	PSW &= ~FC;
}

/***********************************
 *  1000 0101
 *  CLR     F0
 ***********************************/
INLINE void clr_f0(void)
{
	PSW &= ~Ff0;
	STATE &= ~F0;
}

/***********************************
 *  1010 0101
 *  CLR     F1
 ***********************************/
INLINE void clr_f1(void)
{
	STATE &= ~F1;
}

/***********************************
 *  0011 0111
 *  CPL     A
 ***********************************/
INLINE void cpl_a(void)
{
	A = ~A;
}

/***********************************
 *  1010 0111
 *  CPL     C
 ***********************************/
INLINE void cpl_c(void)
{
	PSW ^= FC;
}

/***********************************
 *  1001 0101
 *  CPL     F0
 ***********************************/
INLINE void cpl_f0(void)
{
	PSW ^= Ff0;
	STATE ^= F0;
}

/***********************************
 *  1011 0101
 *  CPL     F1
 ***********************************/
INLINE void cpl_f1(void)
{
	STATE ^= F1;
}

/***********************************
 *  0101 0111
 *  DA      A
 ***********************************/
INLINE void da_a(void)
{
	UINT8 res = A + ((PSW & FA) || ((A & 0x0f) > 0x09)) ? 0x06 : 0x00;
	if( (PSW & FC) || ((res & 0xf0) > 0x90) )
		res += 0x60;
	if( res < A )
		PSW |= FC;
	else
		PSW &= ~FC;
	A = res;
}

/***********************************
 *  0000 0111
 *  DEC     A
 ***********************************/
INLINE void dec_a(void)
{
	A -= 1;
}

/***********************************
 *  1100 1rrr
 *  DEC     Rr
 ***********************************/
INLINE void dec_r(int r)
{
	R(r) -= 1;
}

/***********************************
 *  0001 0101
 *  DIS     I
 ***********************************/
INLINE void dis_i(void)
{
	ENABLE &= ~IBFI;	/* disable input buffer full interrupt */
}

/***********************************
 *  0011 0101
 *  DIS     TCNTI
 ***********************************/
INLINE void dis_tcnti(void)
{
	ENABLE &= ~TCNTI;	/* disable timer/counter interrupt */
}

/***********************************
 *  0111 1rrr 7654 3210
 *  DJNZ    Rr,addr
 ***********************************/
INLINE void djnz_r_i(int r)
{
	UINT8 adr = ROP_ARG(PC);
	PC++;
	R(r) -= 1;
	if( R(r) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1110 0101
 *  EN      DMA
 ***********************************/
INLINE void en_dma(void)
{
	ENABLE |= DMA;		/* enable DMA handshake lines */
	P2_HS &= 0xbf;
	WP(0x02, (P2 & P2_HS) );
}

/***********************************
 *  1111 0101
 *  EN      FLAGS
 ***********************************/
INLINE void en_flags(void)
{
	if( 0 == (ENABLE & FLAGS) )
	{
		/* Configure upper lines on Port 2 for IRQ handshaking (P24 and P25) */

		ENABLE |= FLAGS;
		if( STATE & OBF ) P2_HS |= 0x10;
		else P2_HS &= 0xef;
		if( STATE & IBF ) P2_HS |= 0x20;
		else P2_HS &= 0xdf;
		WP(0x02, (P2 & P2_HS) );
	}
}

/***********************************
 *  0000 0101
 *  EN      I
 ***********************************/
INLINE void en_i(void)
{
	if( 0 == (ENABLE & IBFI) )
	{
		ENABLE |= IBFI;		/* enable input buffer full interrupt */
		if( STATE & IBF )	/* already got data in the buffer? */
			set_irq_line(I8X41_INT_IBF, HOLD_LINE);
	}
}

/***********************************
 *  0010 0101
 *  EN      TCNTI
 ***********************************/
INLINE void en_tcnti(void)
{
	ENABLE |= TCNTI;	/* enable timer/counter interrupt */
}

/***********************************
 *  0010 0010
 *  IN      A,DBB
 ***********************************/
INLINE void in_a_dbb(void)
{
	if( i8x41.irq_callback )
		(*i8x41.irq_callback)(I8X41_INT_IBF);

	STATE &= ~IBF;					/* clear input buffer full flag */
	if( ENABLE & FLAGS )
	{
		P2_HS &= 0xdf;
		if( STATE & OBF ) P2_HS |= 0x10;
		else P2_HS &= 0xef;
		WP(0x02, (P2 & P2_HS) );	/* Clear the DBBI IRQ out on P25 */
	}
	A = DBBI;
}

/***********************************
 *  0000 10pp
 *  IN      A,Pp
 ***********************************/
INLINE void in_a_p(int p)
{
	/* changed to latched port scheme */
	switch( p )
	{
		case 00:	break;	/* invalid port */
		case 01:	A = (RP(p) & P1); break;
		case 02:	A = (RP(p) & P2); break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  0001 0111
 *  INC     A
 ***********************************/
INLINE void inc_a(void)
{
	A += 1;
}

/***********************************
 *  0001 1rrr
 *  INC     Rr
 ***********************************/
INLINE void inc_r(int r)
{
	R(r) += 1;
}

/***********************************
 *  0001 000r
 *  INC  @  Rr
 ***********************************/
INLINE void inc_rm(int r)
{
	UINT16 addr = M_IRAM + (R(r) & I8X42_intRAM_MASK);
	WM( addr, RM(addr) + 1 );
}

/***********************************
 *  bbb1 0010
 *  JBb     addr
 ***********************************/
INLINE void jbb_i(int bit)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( A & (1 << bit) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1111 0110
 *  JC      addr
 ***********************************/
INLINE void jc_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( PSW & FC )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1011 0110
 *  JF0     addr
 ***********************************/
INLINE void jf0_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( STATE & F0 )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0111 0110
 *  JF1     addr
 ***********************************/
INLINE void jf1_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( STATE & F1 )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  aaa0 0100
 *  JMP     addr
 ***********************************/
INLINE void jmp_i(int page)
{
	/* err.. do we have 10 or 11 PC bits?
     * CALL is said to use 0aa1 (4 pages)
     * JMP is said to use aaa0 (8 pages)
     */
	UINT8 adr = ROP_ARG(PC);
	PC = page | adr;
}

/***********************************
 *  1011 0011
 *  JMP  @  A
 ***********************************/
INLINE void jmpp_a(void)
{
	UINT16 adr = (PC & 0x700) | A;
	PC = (PC & 0x700) | RM(adr);
}

/***********************************
 *  1110 0110
 *  JNC     addr
 ***********************************/
INLINE void jnc_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( !(PSW & FC) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1101 0110
 *  JNIBF   addr
 ***********************************/
INLINE void jnibf_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( 0 == (STATE & IBF) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0010 0110
 *  JNT0    addr
 ***********************************/
INLINE void jnt0_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( 0 == RP(I8X41_t0) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0100 0110
 *  JNT1    addr
 ***********************************/
INLINE void jnt1_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( !(ENABLE & CNT) )
	{
		UINT8 level = RP(I8X41_t1);
		if( level ) CONTROL |= TEST1;
		else CONTROL &= ~TEST1;
	}
	if( !(CONTROL & TEST1) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1001 0110
 *  JNZ     addr
 ***********************************/
INLINE void jnz_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( A )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1000 0110
 *  JOBF    addr
 ***********************************/
INLINE void jobf_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( STATE & OBF )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0001 0110
 *  JTF     addr
 ***********************************/
INLINE void jtf_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( CONTROL & TOVF )
		PC = (PC & 0x700) | adr;
	CONTROL &= ~TOVF;
}

/***********************************
 *  0011 0110
 *  JT0     addr
 ***********************************/
INLINE void jt0_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( RP(I8X41_t0) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0101 0110
 *  JT1     addr
 ***********************************/
INLINE void jt1_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( !(ENABLE & CNT) )
	{
		UINT8 level = RP(I8X41_t1);
		if( level ) CONTROL |= TEST1;
		else CONTROL &= ~TEST1;
	}
	if( (CONTROL & TEST1) )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  1100 0110
 *  JZ      addr
 ***********************************/
INLINE void jz_i(void)
{
	UINT8 adr = ROP_ARG(PC);
	PC += 1;
	if( !A )
		PC = (PC & 0x700) | adr;
}

/***********************************
 *  0010 0011
 *  MOV     A,#n
 ***********************************/
INLINE void mov_a_i(void)
{
	A = ROP(PC);
	PC += 1;
}

/***********************************
 *  1100 0111
 *  MOV     A,PSW
 ***********************************/
INLINE void mov_a_psw(void)
{
	A = PSW;
}

/***********************************
 *  1111 1rrr
 *  MOV     A,Rr
 ***********************************/
INLINE void mov_a_r(int r)
{
	A = R(r);
}

/***********************************
 *  1111 000r
 *  MOV     A,Rr
 ***********************************/
INLINE void mov_a_rm(int r)
{
	A = RM( M_IRAM + (R(r) & I8X42_intRAM_MASK) );
}

/***********************************
 *  0100 0010
 *  MOV     A,T
 ***********************************/
INLINE void mov_a_t(void)
{
	A = TIMER;
}

/***********************************
 *  1101 0111
 *  MOV     PSW,A
 ***********************************/
INLINE void mov_psw_a(void)
{
	PSW = A;
}

/***********************************
 *  1010 1rrr
 *  MOV     Rr,A
 ***********************************/
INLINE void mov_r_a(int r)
{
	R(r) = A;
}

/***********************************
 *  1011 1rrr
 *  MOV     Rr,#n
 ***********************************/
INLINE void mov_r_i(int r)
{
	UINT8 val = ROP_ARG(PC);
	PC += 1;
	R(r) = val;
}

/***********************************
 *  1010 000r
 *  MOV     @Rr,A
 ***********************************/
INLINE void mov_rm_a(int r)
{
	WM( M_IRAM + (R(r) & I8X42_intRAM_MASK), A );
}

/***********************************
 *  1011 000r
 *  MOV     @Rr,#n
 ***********************************/
INLINE void mov_rm_i(int r)
{
	UINT8 val = ROP_ARG(PC);
	PC += 1;
	WM( M_IRAM + (R(r) & I8X42_intRAM_MASK), val );
}

/***********************************
 *  1001 0000
 *  MOV     STS,A
 ***********************************/
INLINE void mov_sts_a(void)
{
	STATE = (STATE & 0x0f) | (A & 0xf0);
}

/***********************************
 *  0110 0010
 *  MOV     T,A
 ***********************************/
INLINE void mov_t_a(void)
{
	TIMER = A;
}

/***********************************
 *  0000 11pp
 *  MOVD    A,Pp
 ***********************************/
INLINE void movd_a_p(int p)
{
	/* added proper expanded port setup */
	WP(2, (P2 & 0xf0) | 0x00 | p);	/* READ mode */
	WP(I8X41_ps, 0);		/* activate command strobe */
	A = RP(2) & 0xf;
	WP(I8X41_ps, 1);		/* release command strobe */
}

/***********************************
 *  0011 11pp
 *  MOVD    Pp,A
 ***********************************/
INLINE void movd_p_a(int p)
{
	/* added proper expanded port setup */
	WP(2, (P2 & 0xf0) | 0x04 | p);	/* WRITE mode */
	WP(I8X41_ps, 0);		/* activate command strobe */
	WP(2, A & 0x0f);
	WP(I8X41_ps, 1);		/* release command strobe */
}

/***********************************
 *  1010 0011
 *  MOVP    A,@A
 ***********************************/
INLINE void movp_a_am(void)
{
	UINT16 addr = (PC & 0x700) | A;
	A = RM(addr);
}

/***********************************
 *  1110 0011
 *  MOVP3   A,@A
 ***********************************/
INLINE void movp3_a_am(void)
{
	UINT16 addr = 0x300 | A;
	A = RM(addr);
}

/***********************************
 *  0000 0000
 *  NOP
 ***********************************/
INLINE void nop(void)
{
}

/***********************************
 *  0100 1rrr
 *  ORL     A,Rr
 ***********************************/
INLINE void orl_r(int r)
{
	A = A | R(r);
}

/***********************************
 *  0100 000r
 *  ORL     A,@Rr
 ***********************************/
INLINE void orl_rm(int r)
{
	A = A | RM( M_IRAM + (R(r) & I8X42_intRAM_MASK) );
}

/***********************************
 *  0100 0011 7654 3210
 *  ORL     A,#n
 ***********************************/
INLINE void orl_i(void)
{
	UINT8 val = ROP_ARG(PC);
	PC++;
	A = A | val;
}

/***********************************
 *  1000 10pp 7654 3210
 *  ORL     Pp,#n
 ***********************************/
INLINE void orl_p_i(int p)
{
	UINT8 val = ROP_ARG(PC);
	PC++;
	/* changed to latched port scheme */
	switch (p)
	{
		case 00:	break;	/* invalid port */
		case 01:	P1 |= val; WP(p, P1); break;
		case 02:	P2 |= val; WP(p, P2); break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  1000 11pp 7654 3210
 *  ORLD    Pp,A
 ***********************************/
INLINE void orld_p_a(int p)
{
	/* added proper expanded port setup */
	WP(2, (P2 & 0xf0) | 0x08 | p);	/* OR mode */
	WP(I8X41_ps, 0);	/* activate command strobe */
	WP(2, A & 0x0f);	/* Expander to take care of OR function */
	WP(I8X41_ps, 1);	/* release command strobe */
}

/***********************************
 *  0000 0010
 *  OUT     DBB,A
 ***********************************/
INLINE void out_dbb_a(void)
{
	DBBO = A;			/* DBB output buffer */
	STATE |= OBF;		/* assert the output buffer full flag */
	if( ENABLE & FLAGS )
	{
		P2_HS |= 0x10;
		if( STATE & IBF ) P2_HS |= 0x20;
		else P2_HS &= 0xdf;
		WP(0x02, (P2 & P2_HS) );	/* Assert the DBBO IRQ out on P24 */
	}
}

/***********************************
 *  0011 10pp
 *  OUT     Pp,A
 ***********************************/
INLINE void out_p_a(int p)
{
	/* changed to latched port scheme */
	switch (p)
	{
		case 00:	break;	/* invalid port */
		case 01:	WP(p, A); P1 = A; break;
		case 02:	WP(p, A); P2 = A; break;
		case 03:	break;	/* invalid port */
		default:	break;
	}
}

/***********************************
 *  1000 0011
 *  RET
 ***********************************/
INLINE void ret(void)
{
	UINT8 msb;
	PSW = (PSW & ~SP) | ((PSW - 1) & SP);
	msb = RM(M_STACK + (PSW&SP) * 2 + 1);
	PC = RM(M_STACK + (PSW&SP) * 2 + 0);
	PC |= (msb << 8) & 0x700;
}

/***********************************
 *  1001 0011
 *  RETR
 ***********************************/
INLINE void retr(void)
{
	UINT8 msb;
	PSW = (PSW & ~SP) | ((PSW - 1) & SP);
	msb = RM(M_STACK + (PSW&SP) * 2 + 1);
	PC = RM(M_STACK + (PSW&SP) * 2 + 0);
	PC |= (msb << 8) & 0x700;
	PSW = (PSW & 0x0f) | (msb & 0xf0);
	CONTROL &= ~IBFI_EXEC;
	CONTROL &= ~TIRQ_EXEC;
}

/***********************************
 *  1110 0111
 *  RL      A
 ***********************************/
INLINE void rl_a(void)
{
	A = (A << 1) | (A >> 7);
}

/***********************************
 *  1111 0111
 *  RLC     A
 ***********************************/
INLINE void rlc_a(void)
{
	UINT8 c = PSW >> 7;
	PSW = (PSW & ~FC) | (A >> 7);
	A = (A << 1) | c;
}

/***********************************
 *  0111 0111
 *  RR      A
 ***********************************/
INLINE void rr_a(void)
{
	A = (A >> 1) | (A << 7);
}

/***********************************
 *  0110 0111
 *  RRC     A
 ***********************************/
INLINE void rrc_a(void)
{
	UINT8 c = PSW & 0x80;
	PSW = (PSW & ~FC) | (A << 7);
	A = (A >> 1) | c;
}

/***********************************
 *  1100 0101
 *  SEL     RB0
 ***********************************/
INLINE void sel_rb0(void)
{
	PSW &= ~BS;
}

/***********************************
 *  1101 0101
 *  SEL     RB1
 ***********************************/
INLINE void sel_rb1(void)
{
	PSW |= BS;
}

/***********************************
 *  0110 0101
 *  STOP    TCNT
 ***********************************/
INLINE void stop_tcnt(void)
{
	ENABLE &= ~(T|CNT);
}

/***********************************
 *  0100 0101
 *  STRT    CNT
 ***********************************/
INLINE void strt_cnt(void)
{
	ENABLE |= CNT;
	ENABLE &= ~T;
}

/***********************************
 *  0101 0101
 *  STRT    T
 ***********************************/
INLINE void strt_t(void)
{
	ENABLE |= T;
	ENABLE &= ~CNT;
}

/***********************************
 *  0100 0111
 *  SWAP    A
 ***********************************/
INLINE void swap_a(void)
{
	A = (A << 4) | (A >> 4);
}

/***********************************
 *  0010 1rrr
 *  XCH     A,Rr
 ***********************************/
INLINE void xch_a_r(int r)
{
	UINT8 tmp = R(r);
	R(r) = A;
	A = tmp;
}

/***********************************
 *  0010 000r
 *  XCH     A,@Rr
 ***********************************/
INLINE void xch_a_rm(int r)
{
	UINT16 addr = M_IRAM + (R(r) & I8X42_intRAM_MASK);
	UINT8 tmp = RM(addr);
	WM( addr, A );
	A = tmp;
}

/***********************************
 *  0011 000r
 *  XCHD    A,@Rr
 ***********************************/
INLINE void xchd_a_rm(int r)
{
	UINT16 addr = M_IRAM + (R(r) & I8X42_intRAM_MASK);
	UINT8 tmp = RM(addr);
	WM( addr, (tmp & 0xf0) | (A & 0x0f) );
	A = (A & 0xf0) | (tmp & 0x0f);
}

/***********************************
 *  1101 1rrr
 *  XRL     A,Rr
 ***********************************/
INLINE void xrl_r(int r)
{
	A = A ^ R(r);
}

/***********************************
 *  1101 000r
 *  XRL     A,@Rr
 ***********************************/
INLINE void xrl_rm(int r)
{
	A = A ^ RM( M_IRAM + (R(r) & I8X42_intRAM_MASK) );
}

/***********************************
 *  1101 0011 7654 3210
 *  XRL     A,#n
 ***********************************/
INLINE void xrl_i(void)
{
	UINT8 val = ROP_ARG(PC);
	PC++;
	A = A ^ val;
}


/***********************************************************************
 *  Cycle Timings
 ***********************************************************************/

static UINT8 i8x41_cycles[] = {
	1,1,1,2,2,1,1,1,2,2,2,2,2,2,2,2,
	1,1,2,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,2,1,2,1,2,1,2,2,2,2,2,2,2,2,
	1,1,1,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,2,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,
	1,1,2,1,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,2,2,1,2,1,2,2,2,2,2,2,2,2,
	1,1,2,2,1,1,2,1,2,2,2,2,2,2,2,2,
	1,1,1,2,2,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,1,2,1,2,2,2,2,2,2,2,2,
	1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,2,1,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,1,2,2,1,2,1,1,1,1,1,1,1,1,1,
	1,1,2,1,2,1,2,1,2,2,2,2,2,2,2,2
};


/****************************************************************************
 *  Inits CPU emulation
 ****************************************************************************/

static void i8x41_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	i8x41.irq_callback = irqcallback;

	state_save_register_item("i8x41", index, i8x41.ppc);
	state_save_register_item("i8x41", index, i8x41.pc);
	state_save_register_item("i8x41", index, i8x41.timer);
	state_save_register_item("i8x41", index, i8x41.prescaler);
	state_save_register_item("i8x41", index, i8x41.subtype);
	state_save_register_item("i8x41", index, i8x41.a);
	state_save_register_item("i8x41", index, i8x41.psw);
	state_save_register_item("i8x41", index, i8x41.state);
	state_save_register_item("i8x41", index, i8x41.enable);
	state_save_register_item("i8x41", index, i8x41.control);
	state_save_register_item("i8x41", index, i8x41.dbbi);
	state_save_register_item("i8x41", index, i8x41.dbbo);
	state_save_register_item("i8x41", index, i8x41.p1);
	state_save_register_item("i8x41", index, i8x41.p2);
	state_save_register_item("i8x41", index, i8x41.p2_hs);
}


/****************************************************************************
 *  Reset registers to their initial values
 ****************************************************************************/

static void i8x41_reset(void)
{
	int (*save_irqcallback)(int) = i8x41.irq_callback;
	memset(&i8x41, 0, sizeof(I8X41));
	i8x41.irq_callback = save_irqcallback;

	/* default to 8041 behaviour for DBBI/DBBO and extended commands */
	i8x41.subtype = 8041;
	/* ugly hack.. excuse my lazyness */
	i8x41.ram = memory_region(REGION_CPU1 + cpu_getactivecpu());
	ENABLE = IBFI | TCNTI;
	DBBI = 0xff;
	DBBO = 0xff;
	/* Set Ports 1 and 2 to input mode */
	P1   = 0xff;
	P2   = 0xff;
	P2_HS= 0xff;
}


/****************************************************************************
 *  Shut down CPU emulation
 ****************************************************************************/

static void i8x41_exit(void)
{
	/* nothing to do */
}


/****************************************************************************
 *  Execute cycles - returns number of cycles actually run
 ****************************************************************************/

static int i8x41_execute(int cycles)
{
	int inst_cycles, T1_level;

	i8x41_ICount = cycles;

	do
	{
		UINT8 op = cpu_readop(PC);

		PPC = PC;

		CALL_MAME_DEBUG;

		PC += 1;
		i8x41_ICount -= i8x41_cycles[op];

		switch( op )
		{
		/* opcode cycles bitmask */
		case 0x00: /* 1: 0000 0000 */
			nop();
			break;
		case 0x01: /* 1: 0000 0001 */
			illegal();
			break;
		case 0x02: /* 1: 0000 0010 */
			out_dbb_a();
			break;
		case 0x03: /* 2: 0000 0011 */
			add_i();
			break;
		case 0x04: /* 2: aaa0 0100 */
			jmp_i(0x000);
			break;
		case 0x05: /* 1: 0000 0101 */
			en_i();
			break;
		case 0x06: /* 1: 0000 0110 */
			illegal();
			break;
		case 0x07: /* 1: 0000 0111 */
			dec_a();
			break;
		case 0x08: /* 2: 0000 10pp */
		case 0x09: /* 2: 0000 10pp */
		case 0x0a: /* 2: 0000 10pp */
		case 0x0b: /* 2: 0000 10pp */
			in_a_p(op & 3);
			break;
		case 0x0c: /* 2: 0000 11pp */
		case 0x0d: /* 2: 0000 11pp */
		case 0x0e: /* 2: 0000 11pp */
		case 0x0f: /* 2: 0000 11pp */
			movd_a_p(op & 3);
			break;
		case 0x10: /* 1: 0001 000r */
			inc_rm(0);
			break;
		case 0x11: /* 1: 0001 000r */
			inc_rm(1);
			break;
		case 0x12: /* 2: bbb1 0010 */
			jbb_i(0);
			break;
		case 0x13: /* 2: 0001 0011 */
			addc_i();
			break;
		case 0x14: /* 2: aaa1 0100 */
			call_i(0x000);
			break;
		case 0x15: /* 1: 0001 0101 */
			dis_i();
			break;
		case 0x16: /* 2: 0001 0110 */
			jtf_i();
			break;
		case 0x17: /* 1: 0001 0111 */
			inc_a();
			break;
		case 0x18: /* 1: 0001 1rrr */
		case 0x19: /* 1: 0001 1rrr */
		case 0x1a: /* 1: 0001 1rrr */
		case 0x1b: /* 1: 0001 1rrr */
		case 0x1c: /* 1: 0001 1rrr */
		case 0x1d: /* 1: 0001 1rrr */
		case 0x1e: /* 1: 0001 1rrr */
		case 0x1f: /* 1: 0001 1rrr */
			inc_r(op & 7);
			break;
		case 0x20: /* 1: 0010 000r */
			xch_a_rm(0);
			break;
		case 0x21: /* 1: 0010 000r */
			xch_a_rm(1);
			break;
		case 0x22: /* 1: 0010 0010 */
			in_a_dbb();
			break;
		case 0x23: /* 2: 0010 0011 */
			mov_a_i();
			break;
		case 0x24: /* 2: aaa0 0100 */
			jmp_i(0x100);
			break;
		case 0x25: /* 1: 0010 0101 */
			en_tcnti();
			break;
		case 0x26: /* 2: 0010 0110 */
			jnt0_i();
			break;
		case 0x27: /* 1: 0010 0111 */
			clr_a();
			break;
		case 0x28: /* 1: 0010 1rrr */
		case 0x29: /* 1: 0010 1rrr */
		case 0x2a: /* 1: 0010 1rrr */
		case 0x2b: /* 1: 0010 1rrr */
		case 0x2c: /* 1: 0010 1rrr */
		case 0x2d: /* 1: 0010 1rrr */
		case 0x2e: /* 1: 0010 1rrr */
		case 0x2f: /* 1: 0010 1rrr */
			xch_a_r(op & 7);
			break;
		case 0x30: /* 1: 0011 000r */
			xchd_a_rm(0);
			break;
		case 0x31: /* 1: 0011 000r */
			xchd_a_rm(1);
			break;
		case 0x32: /* 2: bbb1 0010 */
			jbb_i(1);
			break;
		case 0x33: /* 1: 0011 0101 */
			illegal();
			break;
		case 0x34: /* 2: aaa1 0100 */
			call_i(0x100);
			break;
		case 0x35: /* 1: 0000 0101 */
			dis_tcnti();
			break;
		case 0x36: /* 2: 0011 0110 */
			jt0_i();
			break;
		case 0x37: /* 1: 0011 0111 */
			cpl_a();
			break;
		case 0x38: /* 2: 0011 10pp */
		case 0x39: /* 2: 0011 10pp */
		case 0x3a: /* 2: 0011 10pp */
		case 0x3b: /* 2: 0011 10pp */
			out_p_a(op & 3);
			break;
		case 0x3c: /* 2: 0011 11pp */
		case 0x3d: /* 2: 0011 11pp */
		case 0x3e: /* 2: 0011 11pp */
		case 0x3f: /* 2: 0011 11pp */
			movd_p_a(op & 3);
			break;
		case 0x40: /* 1: 0100 000r */
			orl_rm(0);
			break;
		case 0x41: /* 1: 0100 000r */
			orl_rm(1);
			break;
		case 0x42: /* 1: 0100 0010 */
			mov_a_t();
			break;
		case 0x43: /* 2: 0100 0011 */
			orl_i();
			break;
		case 0x44: /* 2: aaa0 0100 */
			jmp_i(0x200);
			break;
		case 0x45: /* 1: 0100 0101 */
			strt_cnt();
			break;
		case 0x46: /* 2: 0100 0110 */
			jnt1_i();
			break;
		case 0x47: /* 1: 0100 0111 */
			swap_a();
			break;
		case 0x48: /* 1: 0100 1rrr */
		case 0x49: /* 1: 0100 1rrr */
		case 0x4a: /* 1: 0100 1rrr */
		case 0x4b: /* 1: 0100 1rrr */
		case 0x4c: /* 1: 0100 1rrr */
		case 0x4d: /* 1: 0100 1rrr */
		case 0x4e: /* 1: 0100 1rrr */
		case 0x4f: /* 1: 0100 1rrr */
			orl_r(op & 7);
			break;
		case 0x50: /* 1: 0101 000r */
			anl_rm(0);
			break;
		case 0x51: /* 1: 0101 000r */
			anl_rm(1);
			break;
		case 0x52: /* 2: bbb1 0010 */
			jbb_i(2);
			break;
		case 0x53: /* 2: 0101 0011 */
			anl_i();
			break;
		case 0x54: /* 2: aaa1 0100 */
			call_i(0x200);
			break;
		case 0x55: /* 1: 0101 0101 */
			strt_t();
			break;
		case 0x56: /* 2: 0101 0110 */
			jt1_i();
			break;
		case 0x57: /* 1: 0101 0111 */
			da_a();
			break;
		case 0x58: /* 1: 0101 1rrr */
		case 0x59: /* 1: 0101 1rrr */
		case 0x5a: /* 1: 0101 1rrr */
		case 0x5b: /* 1: 0101 1rrr */
		case 0x5c: /* 1: 0101 1rrr */
		case 0x5d: /* 1: 0101 1rrr */
		case 0x5e: /* 1: 0101 1rrr */
		case 0x5f: /* 1: 0101 1rrr */
			anl_r(op & 7);
			break;
		case 0x60: /* 1: 0110 000r */
			add_rm(0);
			break;
		case 0x61: /* 1: 0110 000r */
			add_rm(1);
			break;
		case 0x62: /* 1: 0110 0010 */
			mov_t_a();
			break;
		case 0x63: /* 1: 0110 0011 */
			illegal();
			break;
		case 0x64: /* 2: aaa0 0100 */
			jmp_i(0x300);
			break;
		case 0x65: /* 1: 0110 0101 */
			stop_tcnt();
			break;
		case 0x66: /* 1: 0110 0110 */
			illegal();
			break;
		case 0x67: /* 1: 0110 0111 */
			rrc_a();
			break;
		case 0x68: /* 1: 0110 1rrr */
		case 0x69: /* 1: 0110 1rrr */
		case 0x6a: /* 1: 0110 1rrr */
		case 0x6b: /* 1: 0110 1rrr */
		case 0x6c: /* 1: 0110 1rrr */
		case 0x6d: /* 1: 0110 1rrr */
		case 0x6e: /* 1: 0110 1rrr */
		case 0x6f: /* 1: 0110 1rrr */
			add_r(op & 7);
			break;
		case 0x70: /* 1: 0111 000r */
			addc_rm(0);
			break;
		case 0x71: /* 1: 0111 000r */
			addc_rm(1);
			break;
		case 0x72: /* 2: bbb1 0010 */
			jbb_i(3);
			break;
		case 0x73: /* 1: 0111 0011 */
			illegal();
			break;
		case 0x74: /* 2: aaa1 0100 */
			call_i(0x300);
			break;
		case 0x75: /* 1: 0111 0101 */
			illegal();
			break;
		case 0x76: /* 2: 0111 0110 */
			jf1_i();
			break;
		case 0x77: /* 1: 0111 0111 */
			rr_a();
			break;
		case 0x78: /* 1: 0111 1rrr */
		case 0x79: /* 1: 0111 1rrr */
		case 0x7a: /* 1: 0111 1rrr */
		case 0x7b: /* 1: 0111 1rrr */
		case 0x7c: /* 1: 0111 1rrr */
		case 0x7d: /* 1: 0111 1rrr */
		case 0x7e: /* 1: 0111 1rrr */
		case 0x7f: /* 1: 0111 1rrr */
			addc_r(op & 7);
			break;
		case 0x80: /* 1: 1000 0000 */
			illegal();
			break;
		case 0x81: /* 1: 1000 0001 */
			illegal();
			break;
		case 0x82: /* 1: 1000 0010 */
			illegal();
			break;
		case 0x83: /* 2: 1000 0011 */
			ret();
			break;
		case 0x84: /* 2: aaa0 0100 */
			jmp_i(0x400);
			break;
		case 0x85: /* 1: 1000 0101 */
			clr_f0();
			break;
		case 0x86: /* 2: 1000 0110 */
			jobf_i();
			break;
		case 0x87: /* 1: 1000 0111 */
			illegal();
			break;
		case 0x88: /* 2: 1000 10pp */
		case 0x89: /* 2: 1000 10pp */
		case 0x8a: /* 2: 1000 10pp */
		case 0x8b: /* 2: 1000 10pp */
			orl_p_i(op & 3);
			break;
		case 0x8c: /* 2: 1000 11pp */
		case 0x8d: /* 2: 1000 11pp */
		case 0x8e: /* 2: 1000 11pp */
		case 0x8f: /* 2: 1000 11pp */
			orld_p_a(op & 7);
			break;
		case 0x90: /* 1: 1001 0000 */
			mov_sts_a();
			break;
		case 0x91: /* 1: 1001 0001 */
			illegal();
			break;
		case 0x92: /* 2: bbb1 0010 */
			jbb_i(4);
			break;
		case 0x93: /* 2: 1001 0011 */
			retr();
			break;
		case 0x94: /* 1: aaa1 0100 */
			call_i(0x400);
			break;
		case 0x95: /* 1: 1001 0101 */
			cpl_f0();
			break;
		case 0x96: /* 2: 1001 0110 */
			jnz_i();
			break;
		case 0x97: /* 1: 1001 0111 */
			clr_c();
			break;
		case 0x98: /* 2: 1001 10pp , illegal port */
		case 0x99: /* 2: 1001 10pp */
		case 0x9a: /* 2: 1001 10pp */
		case 0x9b: /* 2: 1001 10pp , illegal port */
			anl_p_i(op & 3);
			break;
		case 0x9c: /* 2: 1001 11pp */
		case 0x9d: /* 2: 1001 11pp */
		case 0x9e: /* 2: 1001 11pp */
		case 0x9f: /* 2: 1001 11pp */
			anld_p_a(op & 7);
			break;
		case 0xa0: /* 1: 1010 000r */
			mov_rm_a(0);
			break;
		case 0xa1: /* 1: 1010 000r */
			mov_rm_a(1);
			break;
		case 0xa2: /* 1: 1010 0010 */
			illegal();
			break;
		case 0xa3: /* 2: 1010 0011 */
			movp_a_am();
			break;
		case 0xa4: /* 2: aaa0 0100 */
			jmp_i(0x500);
			break;
		case 0xa5: /* 1: 1010 0101 */
			clr_f1();
			break;
		case 0xa6: /* 1: 1010 0110 */
			illegal();
			break;
		case 0xa7: /* 1: 1010 0111 */
			cpl_c();
			break;
		case 0xa8: /* 1: 1010 1rrr */
		case 0xa9: /* 1: 1010 1rrr */
		case 0xaa: /* 1: 1010 1rrr */
		case 0xab: /* 1: 1010 1rrr */
		case 0xac: /* 1: 1010 1rrr */
		case 0xad: /* 1: 1010 1rrr */
		case 0xae: /* 1: 1010 1rrr */
		case 0xaf: /* 1: 1010 1rrr */
			mov_r_a(op & 7);
			break;
		case 0xb0: /* 2: 1011 000r */
			mov_rm_i(0);
			break;
		case 0xb1: /* 2: 1011 000r */
			mov_rm_i(1);
			break;
		case 0xb2: /* 2: bbb1 0010 */
			jbb_i(5);
			break;
		case 0xb3: /* 2: 1011 0011 */
			jmpp_a();
			break;
		case 0xb4: /* 2: aaa1 0100 */
			call_i(0x500);
			break;
		case 0xb5: /* 1: 1011 0101 */
			cpl_f1();
			break;
		case 0xb6: /* 2: 1011 0110 */
			jf0_i();
			break;
		case 0xb7: /* 1: 1011 0111 */
			illegal();
			break;
		case 0xb8: /* 2: 1011 1rrr */
		case 0xb9: /* 2: 1011 1rrr */
		case 0xba: /* 2: 1011 1rrr */
		case 0xbb: /* 2: 1011 1rrr */
		case 0xbc: /* 2: 1011 1rrr */
		case 0xbd: /* 2: 1011 1rrr */
		case 0xbe: /* 2: 1011 1rrr */
		case 0xbf: /* 2: 1011 1rrr */
			mov_r_i(op & 7);
			break;
		case 0xc0: /* 1: 1100 0000 */
			illegal();
			break;
		case 0xc1: /* 1: 1100 0001 */
			illegal();
			break;
		case 0xc2: /* 1: 1100 0010 */
			illegal();
			break;
		case 0xc3: /* 1: 1100 0011 */
			illegal();
			break;
		case 0xc4: /* 2: aaa0 0100 */
			jmp_i(0x600);
			break;
		case 0xc5: /* 1: 1100 0101 */
			sel_rb0();
			break;
		case 0xc6: /* 2: 1100 0110 */
			jz_i();
			break;
		case 0xc7: /* 1: 1100 0111 */
			mov_a_psw();
			break;
		case 0xc8: /* 1: 1100 1rrr */
		case 0xc9: /* 1: 1100 1rrr */
		case 0xca: /* 1: 1100 1rrr */
		case 0xcb: /* 1: 1100 1rrr */
		case 0xcc: /* 1: 1100 1rrr */
		case 0xcd: /* 1: 1100 1rrr */
		case 0xcf: /* 1: 1100 1rrr */
			dec_r(op & 7);
			break;
		case 0xd0: /* 1: 1101 000r */
			xrl_rm(0);
			break;
		case 0xd1: /* 1: 1101 000r */
			xrl_rm(1);
			break;
		case 0xd2: /* 2: bbb1 0010 */
			jbb_i(6);
			break;
		case 0xd3: /* 1: 1101 0011 */
			xrl_i();
			break;
		case 0xd4: /* 2: aaa1 0100 */
			call_i(0x600);
			break;
		case 0xd5: /* 1: 1101 0101 */
			sel_rb1();
			break;
		case 0xd6: /* 2: 1101 0110 */
			jnibf_i();
			break;
		case 0xd7: /* 1: 1101 0111 */
			mov_psw_a();
			break;
		case 0xd8: /* 1: 1101 1rrr */
		case 0xd9: /* 1: 1101 1rrr */
		case 0xda: /* 1: 1101 1rrr */
		case 0xdb: /* 1: 1101 1rrr */
		case 0xdc: /* 1: 1101 1rrr */
		case 0xdd: /* 1: 1101 1rrr */
		case 0xde: /* 1: 1101 1rrr */
		case 0xdf: /* 1: 1101 1rrr */
			xrl_r(op & 7);
			break;
		case 0xe0: /* 1: 1110 0000 */
			illegal();
			break;
		case 0xe1: /* 1: 1110 0001 */
			illegal();
			break;
		case 0xe2: /* 1: 1110 0010 */
			illegal();
			break;
		case 0xe3: /* 2: 1110 0011 */
			movp3_a_am();
			break;
		case 0xe4: /* 2: aaa0 0100 */
			jmp_i(0x700);
			break;
		case 0xe5: /* 1: 1110 0101 */
			en_dma();
			break;
		case 0xe6: /* 2: 1110 0110 */
			jnc_i();
			break;
		case 0xe7: /* 1: 1110 0111 */
			rl_a();
			break;
		case 0xe8: /* 2: 1110 1rrr */
		case 0xe9: /* 2: 1110 1rrr */
		case 0xea: /* 2: 1110 1rrr */
		case 0xeb: /* 2: 1110 1rrr */
		case 0xec: /* 2: 1110 1rrr */
		case 0xed: /* 2: 1110 1rrr */
		case 0xee: /* 2: 1110 1rrr */
		case 0xef: /* 2: 1110 1rrr */
			djnz_r_i(op & 7);
			break;
		case 0xf0: /* 1: 1111 000r */
			mov_a_rm(0);
			break;
		case 0xf1: /* 1: 1111 000r */
			mov_a_rm(1);
			break;
		case 0xf2: /* 2: bbb1 0010 */
			jbb_i(7);
			break;
		case 0xf3: /* 1: 1111 0011 */
			illegal();
			break;
		case 0xf4: /* 2: aaa1 0100 */
			call_i(0x700);
			break;
		case 0xf5: /* 1: 1111 0101 */
			en_flags();
			break;
		case 0xf6: /* 2: 1111 0110 */
			jc_i();
			break;
		case 0xf7: /* 1: 1111 0111 */
			rlc_a();
			break;
		case 0xf8: /* 1: 1111 1rrr */
		case 0xf9: /* 1: 1111 1rrr */
		case 0xfa: /* 1: 1111 1rrr */
		case 0xfb: /* 1: 1111 1rrr */
		case 0xfc: /* 1: 1111 1rrr */
		case 0xfd: /* 1: 1111 1rrr */
		case 0xfe: /* 1: 1111 1rrr */
		case 0xff: /* 1: 1111 1rrr */
			mov_a_r(op & 7);
			break;
		}


		if( ENABLE & CNT )
		{
			inst_cycles = i8x41_cycles[op];
			for ( ; inst_cycles > 0; inst_cycles-- )
			{
				T1_level = RP(I8X41_t1);
				if( (CONTROL & TEST1) && (T1_level == 0) )	/* Negative Edge */
				{
					TIMER++;
					if (TIMER == 0)
					{
						CONTROL |= TOVF;
						if( ENABLE & TCNTI )
							CONTROL |= TIRQ_PEND;
					}
				}
				if( T1_level ) CONTROL |= TEST1;
				else CONTROL &= ~TEST1;
			}
		}

		if( ENABLE & T )
		{
			PRESCALER += i8x41_cycles[op];
			/**** timer is prescaled by 32 ****/
			if( PRESCALER >= 32 )
			{
				PRESCALER -= 32;
				TIMER++;
				if( TIMER == 0 )
				{
					CONTROL |= TOVF;
					if( ENABLE & TCNTI )
						CONTROL |= TIRQ_PEND;
				}
			}
		}

		if( CONTROL & IRQ_PEND )	/* Are any Interrupts Pending ? */
		{
			if( 0 == (CONTROL & IRQ_EXEC) )	/* Are any Interrupts being serviced ? */
			{
				if( (ENABLE & IBFI) && (CONTROL & IBFI_PEND) )
				{
					PUSH_PC_TO_STACK();
					PC = V_IBF;
					CONTROL &= ~IBFI_PEND;
					CONTROL |= IBFI_EXEC;
					i8x41_ICount -= 2;
				}
			}
			if( 0 == (CONTROL & IRQ_EXEC) )	/* Are any Interrupts being serviced ? */
			{
				if( (ENABLE & TCNTI) && (CONTROL & TIRQ_PEND) )
				{
					PUSH_PC_TO_STACK();
					PC = V_TIMER;
					CONTROL &= ~TIRQ_PEND;
					CONTROL |= TIRQ_EXEC;
					if( ENABLE & T ) PRESCALER += 2;	/* 2 states */
					i8x41_ICount -= 2;		/* 2 states to take interrupt */
				}
			}
		}


	} while( i8x41_ICount > 0 );

	return cycles - i8x41_ICount;
}


/****************************************************************************
 *  Get all registers in given buffer
 ****************************************************************************/

static void i8x41_get_context(void *dst)
{
	if( dst )
		memcpy(dst, &i8x41, sizeof(I8X41));
}


/****************************************************************************
 *  Set all registers to given values
 ****************************************************************************/

static void i8x41_set_context(void *src)
{
	if( src )
		memcpy(&i8x41, src, sizeof(I8X41));
}

/****************************************************************************
 *  Set IRQ line state
 ****************************************************************************/

static void set_irq_line(int irqline, int state)
{
	switch( irqline )
	{
	case I8X41_INT_IBF:
		if (state != CLEAR_LINE)
		{
			STATE |= IBF;
			if (ENABLE & IBFI)
			{
				CONTROL |= IBFI_PEND;
			}
		}
		else
		{
			STATE &= ~IBF;
		}
		break;

	case I8X41_INT_TEST1:
		if( state != CLEAR_LINE )
		{
			CONTROL |= TEST1;
		}
		else
		{
			/* high to low transition? */
			if( CONTROL & TEST1 )
			{
				/* counting enabled? */
				if( ENABLE & CNT )
				{
					TIMER++;
					if( TIMER == 0 )
					{
						CONTROL |= TOVF;
						CONTROL |= TIRQ_PEND;
					}
				}
			}
			CONTROL &= ~TEST1;
		}
		break;
	}
}


/**************************************************************************
 * Register accesses catching uninitialized i8x41.ram pointer
 **************************************************************************/
#define GETR(n) (NULL == i8x41.ram ? 0 : \
	i8x41.ram[((PSW & BS) ? M_BANK1:M_BANK0)+(n)])

#define SETR(n,v) do { \
	if (NULL != i8x41.ram) { \
		i8x41.ram[((PSW & BS) ? M_BANK1:M_BANK0)+(n)] = (v); \
	} \
} while (0)

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void i8x41_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + I8X41_INT_IBF:	set_irq_line(I8X41_INT_IBF, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + I8X41_INT_TEST1:	set_irq_line(I8X41_INT_TEST1, info->i);	break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8X41_PC:			PC = info->i & 0x7ff;					break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + I8X41_SP:			PSW = (PSW & ~SP) | (info->i & SP);		break;

		case CPUINFO_INT_REGISTER + I8X41_PSW:			PSW = info->i;							break;
		case CPUINFO_INT_REGISTER + I8X41_A:			A = info->i;							break;
		case CPUINFO_INT_REGISTER + I8X41_T:			TIMER = info->i & 0x1fff;				break;
		case CPUINFO_INT_REGISTER + I8X41_R0:			SETR(0, info->i);						break;
		case CPUINFO_INT_REGISTER + I8X41_R1:			SETR(1, info->i);						break;
		case CPUINFO_INT_REGISTER + I8X41_R2:			SETR(2, info->i);						break;
		case CPUINFO_INT_REGISTER + I8X41_R3:			SETR(3, info->i);						break;
		case CPUINFO_INT_REGISTER + I8X41_R4:			SETR(4, info->i);						break;
		case CPUINFO_INT_REGISTER + I8X41_R5:			SETR(5, info->i);						break;
		case CPUINFO_INT_REGISTER + I8X41_R6:			SETR(6, info->i);						break;
		case CPUINFO_INT_REGISTER + I8X41_R7:			SETR(7, info->i);						break;

		case CPUINFO_INT_REGISTER + I8X41_DATA:
			DBBI = info->i;
			if( i8x41.subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			STATE &= ~F1;
			STATE |= IBF;
			if( ENABLE & IBFI )
				CONTROL |= IBFI_PEND;
			if( ENABLE & FLAGS)
			{
				P2_HS |= 0x20;
				if( 0 == (STATE & OBF) ) P2_HS |= 0x10;
				else P2_HS &= 0xef;
				WP(0x02, (P2 & P2_HS) );	/* Assert the DBBI IRQ out on P25 */
			}
			break;

		case CPUINFO_INT_REGISTER + I8X41_DATA_DASM:
			/* Same as I8X41_DATA, except this is used by the */
			/* debugger and does not upset the flag states */
			DBBI = info->i;
			if( i8x41.subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			break;

		case CPUINFO_INT_REGISTER + I8X41_CMND:
			DBBI = info->i;
			if( i8x41.subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			STATE |= F1;
			STATE |= IBF;
			if( ENABLE & IBFI )
				CONTROL |= IBFI_PEND;
			if( ENABLE & FLAGS)
			{
				P2_HS |= 0x20;
				if( 0 == (STATE & OBF) ) P2_HS |= 0x10;
				else P2_HS &= 0xef;
				WP(0x02, (P2 & P2_HS) );	/* Assert the DBBI IRQ out on P25 */
			}
			break;

		case CPUINFO_INT_REGISTER + I8X41_CMND_DASM:
			/* Same as I8X41_CMND, except this is used by the */
			/* debugger and does not upset the flag states */
			DBBI = info->i;
			if( i8x41.subtype == 8041 ) /* plain 8041 had no split input/output DBB buffers */
				DBBO = info->i;
			break;

		case CPUINFO_INT_REGISTER + I8X41_STAT:
			logerror("i8x41 #%d:%03x  Setting STAT DBBI to %02x\n", cpu_getactivecpu(), PC, (UINT8)info->i);
			/* writing status.. hmm, should we issue interrupts here too? */
			STATE = info->i;
			break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void i8x41_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(i8x41);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 2;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;					break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = I8X41_CLOCK_DIVIDER;			break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 2;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + I8X41_INT_IBF:	info->i = (STATE & IBF) ? ASSERT_LINE : CLEAR_LINE; break;
		case CPUINFO_INT_INPUT_STATE + I8X41_INT_TEST1:	info->i = (STATE & TEST1) ? ASSERT_LINE : CLEAR_LINE; break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + I8X41_PC:			info->i = PC;							break;

		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + I8X41_SP:			info->i = PSW & SP;						break;

		case CPUINFO_INT_REGISTER + I8X41_PSW:			info->i = PSW;							break;
		case CPUINFO_INT_REGISTER + I8X41_A:			info->i = A;							break;
		case CPUINFO_INT_REGISTER + I8X41_T:			info->i = TIMER;						break;
		case CPUINFO_INT_REGISTER + I8X41_R0:			info->i = GETR(0);						break;
		case CPUINFO_INT_REGISTER + I8X41_R1:			info->i = GETR(1);						break;
		case CPUINFO_INT_REGISTER + I8X41_R2:			info->i = GETR(2);						break;
		case CPUINFO_INT_REGISTER + I8X41_R3:			info->i = GETR(3);						break;
		case CPUINFO_INT_REGISTER + I8X41_R4:			info->i = GETR(4);						break;
		case CPUINFO_INT_REGISTER + I8X41_R5:			info->i = GETR(5);						break;
		case CPUINFO_INT_REGISTER + I8X41_R6:			info->i = GETR(6);						break;
		case CPUINFO_INT_REGISTER + I8X41_R7:			info->i = GETR(7);						break;

		case CPUINFO_INT_REGISTER + I8X41_DATA:
			STATE &= ~OBF;	/* reset the output buffer full flag */
			if( ENABLE & FLAGS)
			{
				P2_HS &= 0xef;
				if( STATE & IBF ) P2_HS |= 0x20;
				else P2_HS &= 0xdf;
				WP(0x02, (P2 & P2_HS) );	/* Clear the DBBO IRQ out on P24 */
			}
			info->i = DBBO;
			break;

		case CPUINFO_INT_REGISTER + I8X41_DATA_DASM:
			/* Same as I8X41_DATA, except this is used by the */
			/* debugger and does not upset the flag states */
			info->i = DBBO;
			break;

		case CPUINFO_INT_REGISTER + I8X41_STAT:
			logerror("i8x41 #%d:%03x  Reading STAT %02x\n", cpu_getactivecpu(), PC, STATE);
			info->i = STATE;
			break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = i8x41_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = i8x41_get_context;	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = i8x41_set_context;	break;
		case CPUINFO_PTR_INIT:							info->init = i8x41_init;				break;
		case CPUINFO_PTR_RESET:							info->reset = i8x41_reset;				break;
		case CPUINFO_PTR_EXIT:							info->exit = i8x41_exit;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = i8x41_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = i8x41_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &i8x41_ICount;			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "I8X41");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Intel 8x41");			break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.3");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 1999 Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				i8x41.psw & 0x80 ? 'C':'.',
				i8x41.psw & 0x40 ? 'A':'.',
				i8x41.psw & 0x20 ? '0':'.',
				i8x41.psw & 0x10 ? 'B':'.',
				i8x41.psw & 0x08 ? '?':'.',
				i8x41.psw & 0x04 ? 's':'.',
				i8x41.psw & 0x02 ? 's':'.',
				i8x41.psw & 0x01 ? 's':'.');
			break;

		case CPUINFO_STR_REGISTER + I8X41_PC:			sprintf(info->s, "PC:%04X", i8x41.pc);	break;
		case CPUINFO_STR_REGISTER + I8X41_SP:			sprintf(info->s, "S:%X", i8x41.psw & SP); break;
		case CPUINFO_STR_REGISTER + I8X41_PSW:			sprintf(info->s, "PSW:%02X", i8x41.psw); break;
		case CPUINFO_STR_REGISTER + I8X41_A:			sprintf(info->s, "A:%02X", i8x41.a);	break;
		case CPUINFO_STR_REGISTER + I8X41_T:			sprintf(info->s, "T:%02X.%02X", i8x41.timer, (i8x41.prescaler & 0x1f) ); break;
		case CPUINFO_STR_REGISTER + I8X41_R0:			sprintf(info->s, "R0:%02X", GETR(0));	break;
		case CPUINFO_STR_REGISTER + I8X41_R1:			sprintf(info->s, "R1:%02X", GETR(1));	break;
		case CPUINFO_STR_REGISTER + I8X41_R2:			sprintf(info->s, "R2:%02X", GETR(2));	break;
		case CPUINFO_STR_REGISTER + I8X41_R3:			sprintf(info->s, "R3:%02X", GETR(3));	break;
		case CPUINFO_STR_REGISTER + I8X41_R4:			sprintf(info->s, "R4:%02X", GETR(4));	break;
		case CPUINFO_STR_REGISTER + I8X41_R5:			sprintf(info->s, "R5:%02X", GETR(5));	break;
		case CPUINFO_STR_REGISTER + I8X41_R6:			sprintf(info->s, "R6:%02X", GETR(6));	break;
		case CPUINFO_STR_REGISTER + I8X41_R7:			sprintf(info->s, "R7:%02X", GETR(7));	break;
		case CPUINFO_STR_REGISTER + I8X41_P1:			sprintf(info->s, "P1:%02X", i8x41.p1);	break;
		case CPUINFO_STR_REGISTER + I8X41_P2:			sprintf(info->s, "P2:%02X", i8x41.p2);	break;
		case CPUINFO_STR_REGISTER + I8X41_DATA_DASM:	sprintf(info->s, "DBBI:%02X", i8x41.dbbi); break;
		case CPUINFO_STR_REGISTER + I8X41_CMND_DASM:	sprintf(info->s, "DBBO:%02X", i8x41.dbbo); break;
		case CPUINFO_STR_REGISTER + I8X41_STAT:			sprintf(info->s, "STAT:%02X", i8x41.state);	break;
	}
}
