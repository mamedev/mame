/*****************************************************************************
 *
 *   sh2.c
 *   Portable Hitachi SH-2 (SH7600 family) emulator
 *
 *   Copyright Juergen Buchmueller <pullmoll@t-online.de>,
 *   all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was adapted to the MAME CPU core requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************/

/*****************************************************************************
    Changes
    20051129 Mariusz Wojcieszek
    - introduced cpu_readop16() for opcode fetching

    20050813 Mariusz Wojcieszek
    - fixed 64 bit / 32 bit division in division unit

    20031015 O. Galibert
    - dma fixes, thanks to sthief

    20031013 O. Galibert, A. Giles
    - timer fixes
    - multi-cpu simplifications

    20030915 O. Galibert
    - fix DMA1 irq vector
    - ignore writes to DRCRx
    - fix cpu number issues
    - fix slave/master recognition
    - fix wrong-cpu-in-context problem with the timers

    20021020 O. Galibert
    - DMA implementation, lightly tested
    - change_pc() crap fixed
    - delay slot in debugger fixed
    - add divide box mirrors
    - Nicola-ify the indentation
    - Uncrapify sh2_internal_*
    - Put back nmi support that had been lost somehow

    20020914 R. Belmont
    - Initial SH2 internal timers implementation, based on code by O. Galibert.
      Makes music work in galspanic4/s/s2, panic street, cyvern, other SKNS games.
    - Fix to external division, thanks to "spice" on the E2J board.
      Corrects behavior of s1945ii turret boss.

    20020302 Olivier Galibert (galibert@mame.net)
    - Fixed interrupt in delay slot
    - Fixed rotcr
    - Fixed div1
    - Fixed mulu
    - Fixed negc

    20020301 R. Belmont
    - Fixed external division

    20020225 Olivier Galibert (galibert@mame.net)
    - Fixed interrupt handling

    20010207 Sylvain Glaize (mokona@puupuu.org)

    - Bug fix in INLINE void MOVBM(UINT32 m, UINT32 n) (see comment)
    - Support of full 32 bit addressing (RB, RW, RL and WB, WW, WL functions)
        reason : when the two high bits of the address are set, access is
        done directly in the cache data array. The SUPER KANEKO NOVA SYSTEM
        sets the stack pointer here, using these addresses as usual RAM access.

        No real cache support has been added.
    - Read/Write memory format correction (_bew to _bedw) (see also SH2
        definition in cpuintrf.c and DasmSH2(..) in sh2dasm.c )

    20010623 James Forshaw (TyRaNiD@totalise.net)

    - Modified operation of sh2_exception. Done cause mame irq system is stupid, and
      doesnt really seem designed for any more than 8 interrupt lines.

    20010701 James Forshaw (TyRaNiD@totalise.net)

    - Fixed DIV1 operation. Q bit now correctly generated

    20020218 Added save states (mokona@puupuu.org)

 *****************************************************************************/

#include "debugger.h"
#include "deprecat.h"
#include "sh2.h"

/* speed up delay loops, bail out of tight loops */
#define BUSY_LOOP_HACKS 	1

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

typedef struct
{
	int irq_vector;
	int irq_priority;
} irq_entry;

typedef struct
{
	UINT32	ppc;
	UINT32	pc;
	UINT32	pr;
	UINT32	sr;
	UINT32	gbr, vbr;
	UINT32	mach, macl;
	UINT32	r[16];
	UINT32	ea;
	UINT32	delay;
	UINT32	cpu_off;
	UINT32	dvsr, dvdnth, dvdntl, dvcr;
	UINT32	pending_irq;
	UINT32    test_irq;
	irq_entry     irq_queue[16];

	INT8	irq_line_state[17];
	int 	(*irq_callback)(int irqline);
	UINT32	*m;
	INT8  nmi_line_state;

	UINT16 	frc;
	UINT16 	ocra, ocrb, icr;
	UINT64 	frc_base;

	int		frt_input;
	int 	internal_irq_level;
	int 	internal_irq_vector;

	emu_timer *timer;
	emu_timer *dma_timer[2];
	int     dma_timer_active[2];

	int     is_slave, cpu_number;

	void	(*ftcsr_read_callback)(UINT32 data);
} SH2;

static int sh2_icount;
static SH2 sh2;

// Atrocious hack that makes the soldivid music correct

//static const int div_tab[4] = { 3, 5, 7, 0 };
static const int div_tab[4] = { 3, 5, 3, 0 };

enum {
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

static TIMER_CALLBACK( sh2_timer_callback );

#define T	0x00000001
#define S	0x00000002
#define I	0x000000f0
#define Q	0x00000100
#define M	0x00000200

#define AM	0xc7ffffff

#define FLAGS	(M|Q|I|S|T)

#define Rn	((opcode>>8)&15)
#define Rm	((opcode>>4)&15)

INLINE UINT8 RB(offs_t A)
{
	if (A >= 0xe0000000)
		return sh2_internal_r(Machine, (A & 0x1fc)>>2, ~(0xff << (((~A) & 3)*8))) >> (((~A) & 3)*8);

	if (A >= 0xc0000000)
		return program_read_byte_32be(A);

	if (A >= 0x40000000)
		return 0xa5;

	return program_read_byte_32be(A & AM);
}

INLINE UINT16 RW(offs_t A)
{
	if (A >= 0xe0000000)
		return sh2_internal_r(Machine, (A & 0x1fc)>>2, ~(0xffff << (((~A) & 2)*8))) >> (((~A) & 2)*8);

	if (A >= 0xc0000000)
		return program_read_word_32be(A);

	if (A >= 0x40000000)
		return 0xa5a5;

	return program_read_word_32be(A & AM);
}

INLINE UINT32 RL(offs_t A)
{
	if (A >= 0xe0000000)
		return sh2_internal_r(Machine, (A & 0x1fc)>>2, 0);

	if (A >= 0xc0000000)
		return program_read_dword_32be(A);

	if (A >= 0x40000000)
		return 0xa5a5a5a5;

  return program_read_dword_32be(A & AM);
}

INLINE void WB(offs_t A, UINT8 V)
{

	if (A >= 0xe0000000)
	{
		sh2_internal_w(Machine, (A & 0x1fc)>>2, V << (((~A) & 3)*8), ~(0xff << (((~A) & 3)*8)));
		return;
	}

	if (A >= 0xc0000000)
	{
		program_write_byte_32be(A,V);
		return;
	}

	if (A >= 0x40000000)
		return;

	program_write_byte_32be(A & AM,V);
}

INLINE void WW(offs_t A, UINT16 V)
{
	if (A >= 0xe0000000)
	{
		sh2_internal_w(Machine, (A & 0x1fc)>>2, V << (((~A) & 2)*8), ~(0xffff << (((~A) & 2)*8)));
		return;
	}

	if (A >= 0xc0000000)
	{
		program_write_word_32be(A,V);
		return;
	}

	if (A >= 0x40000000)
		return;

	program_write_word_32be(A & AM,V);
}

INLINE void WL(offs_t A, UINT32 V)
{
	if (A >= 0xe0000000)
	{
		sh2_internal_w(Machine, (A & 0x1fc)>>2, V, 0);
		return;
	}

	if (A >= 0xc0000000)
	{
		program_write_dword_32be(A,V);
		return;
	}

	if (A >= 0x40000000)
		return;

	program_write_dword_32be(A & AM,V);
}

INLINE void sh2_exception(const char *message, int irqline)
{
	int vector;

	if (irqline != 16)
	{
		if (irqline <= ((sh2.sr >> 4) & 15)) /* If the cpu forbids this interrupt */
			return;

		// if this is an sh2 internal irq, use its vector
		if (sh2.internal_irq_level == irqline)
		{
			vector = sh2.internal_irq_vector;
			LOG(("SH-2 #%d exception #%d (internal vector: $%x) after [%s]\n", cpu_getactivecpu(), irqline, vector, message));
		}
		else
		{
			if(sh2.m[0x38] & 0x00010000)
			{
				vector = sh2.irq_callback(irqline);
				LOG(("SH-2 #%d exception #%d (external vector: $%x) after [%s]\n", cpu_getactivecpu(), irqline, vector, message));
			}
			else
			{
				sh2.irq_callback(irqline);
				vector = 64 + irqline/2;
				LOG(("SH-2 #%d exception #%d (autovector: $%x) after [%s]\n", cpu_getactivecpu(), irqline, vector, message));
			}
		}
	}
	else
	{
		vector = 11;
		LOG(("SH-2 #%d nmi exception (autovector: $%x) after [%s]\n", cpu_getactivecpu(), vector, message));
	}

	sh2.r[15] -= 4;
	WL( sh2.r[15], sh2.sr );		/* push SR onto stack */
	sh2.r[15] -= 4;
	WL( sh2.r[15], sh2.pc );		/* push PC onto stack */

	/* set I flags in SR */
	if (irqline > SH2_INT_15)
		sh2.sr = sh2.sr | I;
	else
		sh2.sr = (sh2.sr & ~I) | (irqline << 4);

	/* fetch PC */
	sh2.pc = RL( sh2.vbr + vector * 4 );
	change_pc(sh2.pc & AM);
}

#define CHECK_PENDING_IRQ(message)				\
do {											\
	int irq = -1;								\
	if (sh2.pending_irq & (1 <<  0)) irq =	0;	\
	if (sh2.pending_irq & (1 <<  1)) irq =	1;	\
	if (sh2.pending_irq & (1 <<  2)) irq =	2;	\
	if (sh2.pending_irq & (1 <<  3)) irq =	3;	\
	if (sh2.pending_irq & (1 <<  4)) irq =	4;	\
	if (sh2.pending_irq & (1 <<  5)) irq =	5;	\
	if (sh2.pending_irq & (1 <<  6)) irq =	6;	\
	if (sh2.pending_irq & (1 <<  7)) irq =	7;	\
	if (sh2.pending_irq & (1 <<  8)) irq =	8;	\
	if (sh2.pending_irq & (1 <<  9)) irq =	9;	\
	if (sh2.pending_irq & (1 << 10)) irq = 10;	\
	if (sh2.pending_irq & (1 << 11)) irq = 11;	\
	if (sh2.pending_irq & (1 << 12)) irq = 12;	\
	if (sh2.pending_irq & (1 << 13)) irq = 13;	\
	if (sh2.pending_irq & (1 << 14)) irq = 14;	\
	if (sh2.pending_irq & (1 << 15)) irq = 15;	\
	if ((sh2.internal_irq_level != -1) && (sh2.internal_irq_level > irq)) irq = sh2.internal_irq_level; \
	if (irq >= 0)								\
		sh2_exception(message,irq); 			\
} while(0)

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
INLINE void ADD(UINT32 m, UINT32 n)
{
	sh2.r[n] += sh2.r[m];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
INLINE void ADDI(UINT32 i, UINT32 n)
{
	sh2.r[n] += (INT32)(INT16)(INT8)i;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
INLINE void ADDC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = sh2.r[n] + sh2.r[m];
	tmp0 = sh2.r[n];
	sh2.r[n] = tmp1 + (sh2.sr & T);
	if (tmp0 > tmp1)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
	if (tmp1 > sh2.r[n])
		sh2.sr |= T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1111  1       overflow
 *  ADDV    Rm,Rn
 */
INLINE void ADDV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) sh2.r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh2.r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh2.r[n] += sh2.r[m];
	if ((INT32) sh2.r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			sh2.sr |= T;
		else
			sh2.sr &= ~T;
	}
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1001  1       -
 *  AND     Rm,Rn
 */
INLINE void AND(UINT32 m, UINT32 n)
{
	sh2.r[n] &= sh2.r[m];
}


/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
INLINE void ANDI(UINT32 i)
{
	sh2.r[0] &= i;
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
INLINE void ANDM(UINT32 i)
{
	UINT32 temp;

	sh2.ea = sh2.gbr + sh2.r[0];
	temp = i & RB( sh2.ea );
	WB( sh2.ea, temp );
	sh2_icount -= 2;
}

/*  code                 cycles  t-bit
 *  1000 1011 dddd dddd  3/1     -
 *  BF      disp8
 */
INLINE void BF(UINT32 d)
{
	if ((sh2.sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2.pc = sh2.ea = sh2.pc + disp * 2 + 2;
		change_pc(sh2.pc & AM);
		sh2_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1111 dddd dddd  3/1     -
 *  BFS     disp8
 */
INLINE void BFS(UINT32 d)
{
	if ((sh2.sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2.delay = sh2.pc;
		sh2.pc = sh2.ea = sh2.pc + disp * 2 + 2;
		sh2_icount--;
	}
}

/*  code                 cycles  t-bit
 *  1010 dddd dddd dddd  2       -
 *  BRA     disp12
 */
INLINE void BRA(UINT32 d)
{
	INT32 disp = ((INT32)d << 20) >> 20;

#if BUSY_LOOP_HACKS
	if (disp == -2)
	{
		UINT32 next_opcode = RW(sh2.ppc & AM);
		/* BRA  $
         * NOP
         */
		if (next_opcode == 0x0009)
			sh2_icount %= 3;	/* cycles for BRA $ and NOP taken (3) */
	}
#endif
	sh2.delay = sh2.pc;
	sh2.pc = sh2.ea = sh2.pc + disp * 2 + 2;
	sh2_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0010 0011  2       -
 *  BRAF    Rm
 */
INLINE void BRAF(UINT32 m)
{
	sh2.delay = sh2.pc;
	sh2.pc += sh2.r[m] + 2;
	sh2_icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
INLINE void BSR(UINT32 d)
{
	INT32 disp = ((INT32)d << 20) >> 20;

	sh2.pr = sh2.pc + 2;
	sh2.delay = sh2.pc;
	sh2.pc = sh2.ea = sh2.pc + disp * 2 + 2;
	sh2_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0000 0011  2       -
 *  BSRF    Rm
 */
INLINE void BSRF(UINT32 m)
{
	sh2.pr = sh2.pc + 2;
	sh2.delay = sh2.pc;
	sh2.pc += sh2.r[m] + 2;
	sh2_icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
INLINE void BT(UINT32 d)
{
	if ((sh2.sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2.pc = sh2.ea = sh2.pc + disp * 2 + 2;
		change_pc(sh2.pc & AM);
		sh2_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1101 dddd dddd  2/1     -
 *  BTS     disp8
 */
INLINE void BTS(UINT32 d)
{
	if ((sh2.sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2.delay = sh2.pc;
		sh2.pc = sh2.ea = sh2.pc + disp * 2 + 2;
		sh2_icount--;
	}
}

/*  code                 cycles  t-bit
 *  0000 0000 0010 1000  1       -
 *  CLRMAC
 */
INLINE void CLRMAC(void)
{
	sh2.mach = 0;
	sh2.macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
INLINE void CLRT(void)
{
	sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
INLINE void CMPEQ(UINT32 m, UINT32 n)
{
	if (sh2.r[n] == sh2.r[m])
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
INLINE void CMPGE(UINT32 m, UINT32 n)
{
	if ((INT32) sh2.r[n] >= (INT32) sh2.r[m])
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
INLINE void CMPGT(UINT32 m, UINT32 n)
{
	if ((INT32) sh2.r[n] > (INT32) sh2.r[m])
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
INLINE void CMPHI(UINT32 m, UINT32 n)
{
	if ((UINT32) sh2.r[n] > (UINT32) sh2.r[m])
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
INLINE void CMPHS(UINT32 m, UINT32 n)
{
	if ((UINT32) sh2.r[n] >= (UINT32) sh2.r[m])
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}


/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
INLINE void CMPPL(UINT32 n)
{
	if ((INT32) sh2.r[n] > 0)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
INLINE void CMPPZ(UINT32 n)
{
	if ((INT32) sh2.r[n] >= 0)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1100  1       comparison result
 * CMP_STR  Rm,Rn
 */
INLINE void CMPSTR(UINT32 m, UINT32 n)
 {
  UINT32 temp;
  INT32 HH, HL, LH, LL;
  temp = sh2.r[n] ^ sh2.r[m];
  HH = (temp >> 24) & 0xff;
  HL = (temp >> 16) & 0xff;
  LH = (temp >> 8) & 0xff;
  LL = temp & 0xff;
  if (HH && HL && LH && LL)
   sh2.sr &= ~T;
  else
   sh2.sr |= T;
 }


/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
INLINE void CMPIM(UINT32 i)
{
	UINT32 imm = (UINT32)(INT32)(INT16)(INT8)i;

	if (sh2.r[0] == imm)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
INLINE void DIV0S(UINT32 m, UINT32 n)
{
	if ((sh2.r[n] & 0x80000000) == 0)
		sh2.sr &= ~Q;
	else
		sh2.sr |= Q;
	if ((sh2.r[m] & 0x80000000) == 0)
		sh2.sr &= ~M;
	else
		sh2.sr |= M;
	if ((sh2.r[m] ^ sh2.r[n]) & 0x80000000)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
INLINE void DIV0U(void)
{
	sh2.sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
INLINE void DIV1(UINT32 m, UINT32 n)
{
	UINT32 tmp0;
	UINT32 old_q;

	old_q = sh2.sr & Q;
	if (0x80000000 & sh2.r[n])
		sh2.sr |= Q;
	else
		sh2.sr &= ~Q;

	sh2.r[n] = (sh2.r[n] << 1) | (sh2.sr & T);

	if (!old_q)
	{
		if (!(sh2.sr & M))
		{
			tmp0 = sh2.r[n];
			sh2.r[n] -= sh2.r[m];
			if(!(sh2.sr & Q))
				if(sh2.r[n] > tmp0)
					sh2.sr |= Q;
				else
					sh2.sr &= ~Q;
			else
				if(sh2.r[n] > tmp0)
					sh2.sr &= ~Q;
				else
					sh2.sr |= Q;
		}
		else
		{
			tmp0 = sh2.r[n];
			sh2.r[n] += sh2.r[m];
			if(!(sh2.sr & Q))
			{
				if(sh2.r[n] < tmp0)
					sh2.sr &= ~Q;
				else
					sh2.sr |= Q;
			}
			else
			{
				if(sh2.r[n] < tmp0)
					sh2.sr |= Q;
				else
					sh2.sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(sh2.sr & M))
		{
			tmp0 = sh2.r[n];
			sh2.r[n] += sh2.r[m];
			if(!(sh2.sr & Q))
				if(sh2.r[n] < tmp0)
					sh2.sr |= Q;
				else
					sh2.sr &= ~Q;
			else
				if(sh2.r[n] < tmp0)
					sh2.sr &= ~Q;
				else
					sh2.sr |= Q;
		}
		else
		{
			tmp0 = sh2.r[n];
			sh2.r[n] -= sh2.r[m];
			if(!(sh2.sr & Q))
				if(sh2.r[n] > tmp0)
					sh2.sr &= ~Q;
				else
					sh2.sr |= Q;
			else
				if(sh2.r[n] > tmp0)
					sh2.sr |= Q;
				else
					sh2.sr &= ~Q;
		}
	}

	tmp0 = (sh2.sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  DMULS.L Rm,Rn */
INLINE void DMULS(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) sh2.r[n];
	tempm = (INT32) sh2.r[m];
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	if ((INT32) (sh2.r[n] ^ sh2.r[m]) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	sh2.mach = Res2;
	sh2.macl = Res0;
	sh2_icount--;
}

/*  DMULU.L Rm,Rn */
INLINE void DMULU(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;

	RnL = sh2.r[n] & 0x0000ffff;
	RnH = (sh2.r[n] >> 16) & 0x0000ffff;
	RmL = sh2.r[m] & 0x0000ffff;
	RmH = (sh2.r[m] >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	sh2.mach = Res2;
	sh2.macl = Res0;
	sh2_icount--;
}

/*  DT      Rn */
INLINE void DT(UINT32 n)
{
	sh2.r[n]--;
	if (sh2.r[n] == 0)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
#if BUSY_LOOP_HACKS
	{
		UINT32 next_opcode = RW(sh2.ppc & AM);
		/* DT   Rn
         * BF   $-2
         */
		if (next_opcode == 0x8bfd)
		{
			while (sh2.r[n] > 1 && sh2_icount > 4)
			{
				sh2.r[n]--;
				sh2_icount -= 4;	/* cycles for DT (1) and BF taken (3) */
			}
		}
	}
#endif
}

/*  EXTS.B  Rm,Rn */
INLINE void EXTSB(UINT32 m, UINT32 n)
{
	sh2.r[n] = ((INT32)sh2.r[m] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
INLINE void EXTSW(UINT32 m, UINT32 n)
{
	sh2.r[n] = ((INT32)sh2.r[m] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
INLINE void EXTUB(UINT32 m, UINT32 n)
{
	sh2.r[n] = sh2.r[m] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
INLINE void EXTUW(UINT32 m, UINT32 n)
{
	sh2.r[n] = sh2.r[m] & 0x0000ffff;
}

/*  JMP     @Rm */
INLINE void JMP(UINT32 m)
{
	sh2.delay = sh2.pc;
	sh2.pc = sh2.ea = sh2.r[m];
}

/*  JSR     @Rm */
INLINE void JSR(UINT32 m)
{
	sh2.delay = sh2.pc;
	sh2.pr = sh2.pc + 2;
	sh2.pc = sh2.ea = sh2.r[m];
	sh2_icount--;
}


/*  LDC     Rm,SR */
INLINE void LDCSR(UINT32 m)
{
	sh2.sr = sh2.r[m] & FLAGS;
	sh2.test_irq = 1;
}

/*  LDC     Rm,GBR */
INLINE void LDCGBR(UINT32 m)
{
	sh2.gbr = sh2.r[m];
}

/*  LDC     Rm,VBR */
INLINE void LDCVBR(UINT32 m)
{
	sh2.vbr = sh2.r[m];
}

/*  LDC.L   @Rm+,SR */
INLINE void LDCMSR(UINT32 m)
{
	sh2.ea = sh2.r[m];
	sh2.sr = RL( sh2.ea ) & FLAGS;
	sh2.r[m] += 4;
	sh2_icount -= 2;
	sh2.test_irq = 1;
}

/*  LDC.L   @Rm+,GBR */
INLINE void LDCMGBR(UINT32 m)
{
	sh2.ea = sh2.r[m];
	sh2.gbr = RL( sh2.ea );
	sh2.r[m] += 4;
	sh2_icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
INLINE void LDCMVBR(UINT32 m)
{
	sh2.ea = sh2.r[m];
	sh2.vbr = RL( sh2.ea );
	sh2.r[m] += 4;
	sh2_icount -= 2;
}

/*  LDS     Rm,MACH */
INLINE void LDSMACH(UINT32 m)
{
	sh2.mach = sh2.r[m];
}

/*  LDS     Rm,MACL */
INLINE void LDSMACL(UINT32 m)
{
	sh2.macl = sh2.r[m];
}

/*  LDS     Rm,PR */
INLINE void LDSPR(UINT32 m)
{
	sh2.pr = sh2.r[m];
}

/*  LDS.L   @Rm+,MACH */
INLINE void LDSMMACH(UINT32 m)
{
	sh2.ea = sh2.r[m];
	sh2.mach = RL( sh2.ea );
	sh2.r[m] += 4;
}

/*  LDS.L   @Rm+,MACL */
INLINE void LDSMMACL(UINT32 m)
{
	sh2.ea = sh2.r[m];
	sh2.macl = RL( sh2.ea );
	sh2.r[m] += 4;
}

/*  LDS.L   @Rm+,PR */
INLINE void LDSMPR(UINT32 m)
{
	sh2.ea = sh2.r[m];
	sh2.pr = RL( sh2.ea );
	sh2.r[m] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
INLINE void MAC_L(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) RL( sh2.r[n] );
	sh2.r[n] += 4;
	tempm = (INT32) RL( sh2.r[m] );
	sh2.r[m] += 4;
	if ((INT32) (tempn ^ tempm) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	if (sh2.sr & S)
	{
		Res0 = sh2.macl + Res0;
		if (sh2.macl > Res0)
			Res2++;
		Res2 += (sh2.mach & 0x0000ffff);
		if (((INT32) Res2 < 0) && (Res2 < 0xffff8000))
		{
			Res2 = 0x00008000;
			Res0 = 0x00000000;
		}
		else if (((INT32) Res2 > 0) && (Res2 > 0x00007fff))
		{
			Res2 = 0x00007fff;
			Res0 = 0xffffffff;
		}
		sh2.mach = Res2;
		sh2.macl = Res0;
	}
	else
	{
		Res0 = sh2.macl + Res0;
		if (sh2.macl > Res0)
			Res2++;
		Res2 += sh2.mach;
		sh2.mach = Res2;
		sh2.macl = Res0;
	}
	sh2_icount -= 2;
}

/*  MAC.W   @Rm+,@Rn+ */
INLINE void MAC_W(UINT32 m, UINT32 n)
{
	INT32 tempm, tempn, dest, src, ans;
	UINT32 templ;

	tempn = (INT32) RW( sh2.r[n] );
	sh2.r[n] += 2;
	tempm = (INT32) RW( sh2.r[m] );
	sh2.r[m] += 2;
	templ = sh2.macl;
	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);
	if ((INT32) sh2.macl >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) tempm >= 0)
	{
		src = 0;
		tempn = 0;
	}
	else
	{
		src = 1;
		tempn = 0xffffffff;
	}
	src += dest;
	sh2.macl += tempm;
	if ((INT32) sh2.macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (sh2.sr & S)
	{
		if (ans == 1)
			{
				if (src == 0)
					sh2.macl = 0x7fffffff;
				if (src == 2)
					sh2.macl = 0x80000000;
			}
	}
	else
	{
		sh2.mach += tempn;
		if (templ > sh2.macl)
			sh2.mach += 1;
		}
	sh2_icount -= 2;
}

/*  MOV     Rm,Rn */
INLINE void MOV(UINT32 m, UINT32 n)
{
	sh2.r[n] = sh2.r[m];
}

/*  MOV.B   Rm,@Rn */
INLINE void MOVBS(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[n];
	WB( sh2.ea, sh2.r[m] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
INLINE void MOVWS(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[n];
	WW( sh2.ea, sh2.r[m] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
INLINE void MOVLS(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[n];
	WL( sh2.ea, sh2.r[m] );
}

/*  MOV.B   @Rm,Rn */
INLINE void MOVBL(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[m];
	sh2.r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh2.ea );
}

/*  MOV.W   @Rm,Rn */
INLINE void MOVWL(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[m];
	sh2.r[n] = (UINT32)(INT32)(INT16) RW( sh2.ea );
}

/*  MOV.L   @Rm,Rn */
INLINE void MOVLL(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[m];
	sh2.r[n] = RL( sh2.ea );
}

/*  MOV.B   Rm,@-Rn */
INLINE void MOVBM(UINT32 m, UINT32 n)
{
	/* SMG : bug fix, was reading sh2.r[n] */
	UINT32 data = sh2.r[m] & 0x000000ff;

	sh2.r[n] -= 1;
	WB( sh2.r[n], data );
}

/*  MOV.W   Rm,@-Rn */
INLINE void MOVWM(UINT32 m, UINT32 n)
{
	UINT32 data = sh2.r[m] & 0x0000ffff;

	sh2.r[n] -= 2;
	WW( sh2.r[n], data );
}

/*  MOV.L   Rm,@-Rn */
INLINE void MOVLM(UINT32 m, UINT32 n)
{
	UINT32 data = sh2.r[m];

	sh2.r[n] -= 4;
	WL( sh2.r[n], data );
}

/*  MOV.B   @Rm+,Rn */
INLINE void MOVBP(UINT32 m, UINT32 n)
{
	sh2.r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh2.r[m] );
	if (n != m)
		sh2.r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
INLINE void MOVWP(UINT32 m, UINT32 n)
{
	sh2.r[n] = (UINT32)(INT32)(INT16) RW( sh2.r[m] );
	if (n != m)
		sh2.r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
INLINE void MOVLP(UINT32 m, UINT32 n)
{
	sh2.r[n] = RL( sh2.r[m] );
	if (n != m)
		sh2.r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
INLINE void MOVBS0(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[n] + sh2.r[0];
	WB( sh2.ea, sh2.r[m] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
INLINE void MOVWS0(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[n] + sh2.r[0];
	WW( sh2.ea, sh2.r[m] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
INLINE void MOVLS0(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[n] + sh2.r[0];
	WL( sh2.ea, sh2.r[m] );
}

/*  MOV.B   @(R0,Rm),Rn */
INLINE void MOVBL0(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[m] + sh2.r[0];
	sh2.r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh2.ea );
}

/*  MOV.W   @(R0,Rm),Rn */
INLINE void MOVWL0(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[m] + sh2.r[0];
	sh2.r[n] = (UINT32)(INT32)(INT16) RW( sh2.ea );
}

/*  MOV.L   @(R0,Rm),Rn */
INLINE void MOVLL0(UINT32 m, UINT32 n)
{
	sh2.ea = sh2.r[m] + sh2.r[0];
	sh2.r[n] = RL( sh2.ea );
}

/*  MOV     #imm,Rn */
INLINE void MOVI(UINT32 i, UINT32 n)
{
	sh2.r[n] = (UINT32)(INT32)(INT16)(INT8) i;
}

/*  MOV.W   @(disp8,PC),Rn */
INLINE void MOVWI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	sh2.ea = sh2.pc + disp * 2 + 2;
	sh2.r[n] = (UINT32)(INT32)(INT16) RW( sh2.ea );
}

/*  MOV.L   @(disp8,PC),Rn */
INLINE void MOVLI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	sh2.ea = ((sh2.pc + 2) & ~3) + disp * 4;
	sh2.r[n] = RL( sh2.ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
INLINE void MOVBLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2.ea = sh2.gbr + disp;
	sh2.r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh2.ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
INLINE void MOVWLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2.ea = sh2.gbr + disp * 2;
	sh2.r[0] = (INT32)(INT16) RW( sh2.ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
INLINE void MOVLLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2.ea = sh2.gbr + disp * 4;
	sh2.r[0] = RL( sh2.ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
INLINE void MOVBSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2.ea = sh2.gbr + disp;
	WB( sh2.ea, sh2.r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
INLINE void MOVWSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2.ea = sh2.gbr + disp * 2;
	WW( sh2.ea, sh2.r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
INLINE void MOVLSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2.ea = sh2.gbr + disp * 4;
	WL( sh2.ea, sh2.r[0] );
}

/*  MOV.B   R0,@(disp4,Rn) */
INLINE void MOVBS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2.ea = sh2.r[n] + disp;
	WB( sh2.ea, sh2.r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rn) */
INLINE void MOVWS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2.ea = sh2.r[n] + disp * 2;
	WW( sh2.ea, sh2.r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
INLINE void MOVLS4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2.ea = sh2.r[n] + disp * 4;
	WL( sh2.ea, sh2.r[m] );
}

/*  MOV.B   @(disp4,Rm),R0 */
INLINE void MOVBL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	sh2.ea = sh2.r[m] + disp;
	sh2.r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh2.ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
INLINE void MOVWL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	sh2.ea = sh2.r[m] + disp * 2;
	sh2.r[0] = (UINT32)(INT32)(INT16) RW( sh2.ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
INLINE void MOVLL4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2.ea = sh2.r[m] + disp * 4;
	sh2.r[n] = RL( sh2.ea );
}

/*  MOVA    @(disp8,PC),R0 */
INLINE void MOVA(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2.ea = ((sh2.pc + 2) & ~3) + disp * 4;
	sh2.r[0] = sh2.ea;
}

/*  MOVT    Rn */
INLINE void MOVT(UINT32 n)
{
	sh2.r[n] = sh2.sr & T;
}

/*  MUL.L   Rm,Rn */
INLINE void MULL(UINT32 m, UINT32 n)
{
	sh2.macl = sh2.r[n] * sh2.r[m];
	sh2_icount--;
}

/*  MULS    Rm,Rn */
INLINE void MULS(UINT32 m, UINT32 n)
{
	sh2.macl = (INT16) sh2.r[n] * (INT16) sh2.r[m];
}

/*  MULU    Rm,Rn */
INLINE void MULU(UINT32 m, UINT32 n)
{
	sh2.macl = (UINT16) sh2.r[n] * (UINT16) sh2.r[m];
}

/*  NEG     Rm,Rn */
INLINE void NEG(UINT32 m, UINT32 n)
{
	sh2.r[n] = 0 - sh2.r[m];
}

/*  NEGC    Rm,Rn */
INLINE void NEGC(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = sh2.r[m];
	sh2.r[n] = -temp - (sh2.sr & T);
	if (temp || (sh2.sr & T))
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  NOP */
INLINE void NOP(void)
{
}

/*  NOT     Rm,Rn */
INLINE void NOT(UINT32 m, UINT32 n)
{
	sh2.r[n] = ~sh2.r[m];
}

/*  OR      Rm,Rn */
INLINE void OR(UINT32 m, UINT32 n)
{
	sh2.r[n] |= sh2.r[m];
}

/*  OR      #imm,R0 */
INLINE void ORI(UINT32 i)
{
	sh2.r[0] |= i;
	sh2_icount -= 2;
}

/*  OR.B    #imm,@(R0,GBR) */
INLINE void ORM(UINT32 i)
{
	UINT32 temp;

	sh2.ea = sh2.gbr + sh2.r[0];
	temp = RB( sh2.ea );
	temp |= i;
	WB( sh2.ea, temp );
}

/*  ROTCL   Rn */
INLINE void ROTCL(UINT32 n)
{
	UINT32 temp;

	temp = (sh2.r[n] >> 31) & T;
	sh2.r[n] = (sh2.r[n] << 1) | (sh2.sr & T);
	sh2.sr = (sh2.sr & ~T) | temp;
}

/*  ROTCR   Rn */
INLINE void ROTCR(UINT32 n)
{
	UINT32 temp;
	temp = (sh2.sr & T) << 31;
	if (sh2.r[n] & T)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
	sh2.r[n] = (sh2.r[n] >> 1) | temp;
}

/*  ROTL    Rn */
INLINE void ROTL(UINT32 n)
{
	sh2.sr = (sh2.sr & ~T) | ((sh2.r[n] >> 31) & T);
	sh2.r[n] = (sh2.r[n] << 1) | (sh2.r[n] >> 31);
}

/*  ROTR    Rn */
INLINE void ROTR(UINT32 n)
{
	sh2.sr = (sh2.sr & ~T) | (sh2.r[n] & T);
	sh2.r[n] = (sh2.r[n] >> 1) | (sh2.r[n] << 31);
}

/*  RTE */
INLINE void RTE(void)
{
	sh2.ea = sh2.r[15];
	sh2.delay = sh2.pc;
	sh2.pc = RL( sh2.ea );
	sh2.r[15] += 4;
	sh2.ea = sh2.r[15];
	sh2.sr = RL( sh2.ea ) & FLAGS;
	sh2.r[15] += 4;
	sh2_icount -= 3;
	sh2.test_irq = 1;
}

/*  RTS */
INLINE void RTS(void)
{
	sh2.delay = sh2.pc;
	sh2.pc = sh2.ea = sh2.pr;
	sh2_icount--;
}

/*  SETT */
INLINE void SETT(void)
{
	sh2.sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
INLINE void SHAL(UINT32 n)
{
	sh2.sr = (sh2.sr & ~T) | ((sh2.r[n] >> 31) & T);
	sh2.r[n] <<= 1;
}

/*  SHAR    Rn */
INLINE void SHAR(UINT32 n)
{
	sh2.sr = (sh2.sr & ~T) | (sh2.r[n] & T);
	sh2.r[n] = (UINT32)((INT32)sh2.r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
INLINE void SHLL(UINT32 n)
{
	sh2.sr = (sh2.sr & ~T) | ((sh2.r[n] >> 31) & T);
	sh2.r[n] <<= 1;
}

/*  SHLL2   Rn */
INLINE void SHLL2(UINT32 n)
{
	sh2.r[n] <<= 2;
}

/*  SHLL8   Rn */
INLINE void SHLL8(UINT32 n)
{
	sh2.r[n] <<= 8;
}

/*  SHLL16  Rn */
INLINE void SHLL16(UINT32 n)
{
	sh2.r[n] <<= 16;
}

/*  SHLR    Rn */
INLINE void SHLR(UINT32 n)
{
	sh2.sr = (sh2.sr & ~T) | (sh2.r[n] & T);
	sh2.r[n] >>= 1;
}

/*  SHLR2   Rn */
INLINE void SHLR2(UINT32 n)
{
	sh2.r[n] >>= 2;
}

/*  SHLR8   Rn */
INLINE void SHLR8(UINT32 n)
{
	sh2.r[n] >>= 8;
}

/*  SHLR16  Rn */
INLINE void SHLR16(UINT32 n)
{
	sh2.r[n] >>= 16;
}

/*  SLEEP */
INLINE void SLEEP(void)
{
	sh2.pc -= 2;
	sh2_icount -= 2;
	/* Wait_for_exception; */
}

/*  STC     SR,Rn */
INLINE void STCSR(UINT32 n)
{
	sh2.r[n] = sh2.sr;
}

/*  STC     GBR,Rn */
INLINE void STCGBR(UINT32 n)
{
	sh2.r[n] = sh2.gbr;
}

/*  STC     VBR,Rn */
INLINE void STCVBR(UINT32 n)
{
	sh2.r[n] = sh2.vbr;
}

/*  STC.L   SR,@-Rn */
INLINE void STCMSR(UINT32 n)
{
	sh2.r[n] -= 4;
	sh2.ea = sh2.r[n];
	WL( sh2.ea, sh2.sr );
	sh2_icount--;
}

/*  STC.L   GBR,@-Rn */
INLINE void STCMGBR(UINT32 n)
{
	sh2.r[n] -= 4;
	sh2.ea = sh2.r[n];
	WL( sh2.ea, sh2.gbr );
	sh2_icount--;
}

/*  STC.L   VBR,@-Rn */
INLINE void STCMVBR(UINT32 n)
{
	sh2.r[n] -= 4;
	sh2.ea = sh2.r[n];
	WL( sh2.ea, sh2.vbr );
	sh2_icount--;
}

/*  STS     MACH,Rn */
INLINE void STSMACH(UINT32 n)
{
	sh2.r[n] = sh2.mach;
}

/*  STS     MACL,Rn */
INLINE void STSMACL(UINT32 n)
{
	sh2.r[n] = sh2.macl;
}

/*  STS     PR,Rn */
INLINE void STSPR(UINT32 n)
{
	sh2.r[n] = sh2.pr;
}

/*  STS.L   MACH,@-Rn */
INLINE void STSMMACH(UINT32 n)
{
	sh2.r[n] -= 4;
	sh2.ea = sh2.r[n];
	WL( sh2.ea, sh2.mach );
}

/*  STS.L   MACL,@-Rn */
INLINE void STSMMACL(UINT32 n)
{
	sh2.r[n] -= 4;
	sh2.ea = sh2.r[n];
	WL( sh2.ea, sh2.macl );
}

/*  STS.L   PR,@-Rn */
INLINE void STSMPR(UINT32 n)
{
	sh2.r[n] -= 4;
	sh2.ea = sh2.r[n];
	WL( sh2.ea, sh2.pr );
}

/*  SUB     Rm,Rn */
INLINE void SUB(UINT32 m, UINT32 n)
{
	sh2.r[n] -= sh2.r[m];
}

/*  SUBC    Rm,Rn */
INLINE void SUBC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = sh2.r[n] - sh2.r[m];
	tmp0 = sh2.r[n];
	sh2.r[n] = tmp1 - (sh2.sr & T);
	if (tmp0 < tmp1)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
	if (tmp1 < sh2.r[n])
		sh2.sr |= T;
}

/*  SUBV    Rm,Rn */
INLINE void SUBV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) sh2.r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh2.r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh2.r[n] -= sh2.r[m];
	if ((INT32) sh2.r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			sh2.sr |= T;
		else
			sh2.sr &= ~T;
	}
	else
		sh2.sr &= ~T;
}

/*  SWAP.B  Rm,Rn */
INLINE void SWAPB(UINT32 m, UINT32 n)
{
	UINT32 temp0, temp1;

	temp0 = sh2.r[m] & 0xffff0000;
	temp1 = (sh2.r[m] & 0x000000ff) << 8;
	sh2.r[n] = (sh2.r[m] >> 8) & 0x000000ff;
	sh2.r[n] = sh2.r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
INLINE void SWAPW(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (sh2.r[m] >> 16) & 0x0000ffff;
	sh2.r[n] = (sh2.r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
INLINE void TAS(UINT32 n)
{
	UINT32 temp;
	sh2.ea = sh2.r[n];
	/* Bus Lock enable */
	temp = RB( sh2.ea );
	if (temp == 0)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB( sh2.ea, temp );
	sh2_icount -= 3;
}

/*  TRAPA   #imm */
INLINE void TRAPA(UINT32 i)
{
	UINT32 imm = i & 0xff;

	sh2.ea = sh2.vbr + imm * 4;

	sh2.r[15] -= 4;
	WL( sh2.r[15], sh2.sr );
	sh2.r[15] -= 4;
	WL( sh2.r[15], sh2.pc );

	sh2.pc = RL( sh2.ea );
	change_pc(sh2.pc & AM);

	sh2_icount -= 7;
}

/*  TST     Rm,Rn */
INLINE void TST(UINT32 m, UINT32 n)
{
	if ((sh2.r[n] & sh2.r[m]) == 0)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  TST     #imm,R0 */
INLINE void TSTI(UINT32 i)
{
	UINT32 imm = i & 0xff;

	if ((imm & sh2.r[0]) == 0)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
INLINE void TSTM(UINT32 i)
{
	UINT32 imm = i & 0xff;

	sh2.ea = sh2.gbr + sh2.r[0];
	if ((imm & RB( sh2.ea )) == 0)
		sh2.sr |= T;
	else
		sh2.sr &= ~T;
	sh2_icount -= 2;
}

/*  XOR     Rm,Rn */
INLINE void XOR(UINT32 m, UINT32 n)
{
	sh2.r[n] ^= sh2.r[m];
}

/*  XOR     #imm,R0 */
INLINE void XORI(UINT32 i)
{
	UINT32 imm = i & 0xff;
	sh2.r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
INLINE void XORM(UINT32 i)
{
	UINT32 imm = i & 0xff;
	UINT32 temp;

	sh2.ea = sh2.gbr + sh2.r[0];
	temp = RB( sh2.ea );
	temp ^= imm;
	WB( sh2.ea, temp );
	sh2_icount -= 2;
}

/*  XTRCT   Rm,Rn */
INLINE void XTRCT(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (sh2.r[m] << 16) & 0xffff0000;
	sh2.r[n] = (sh2.r[n] >> 16) & 0x0000ffff;
	sh2.r[n] |= temp;
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/

INLINE void op0000(UINT16 opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: NOP();						break;
	case 0x01: NOP();						break;
	case 0x02: STCSR(Rn);					break;
	case 0x03: BSRF(Rn);					break;
	case 0x04: MOVBS0(Rm, Rn);				break;
	case 0x05: MOVWS0(Rm, Rn);				break;
	case 0x06: MOVLS0(Rm, Rn);				break;
	case 0x07: MULL(Rm, Rn);				break;
	case 0x08: CLRT();						break;
	case 0x09: NOP();						break;
	case 0x0a: STSMACH(Rn); 				break;
	case 0x0b: RTS();						break;
	case 0x0c: MOVBL0(Rm, Rn);				break;
	case 0x0d: MOVWL0(Rm, Rn);				break;
	case 0x0e: MOVLL0(Rm, Rn);				break;
	case 0x0f: MAC_L(Rm, Rn);				break;

	case 0x10: NOP();						break;
	case 0x11: NOP();						break;
	case 0x12: STCGBR(Rn);					break;
	case 0x13: NOP();						break;
	case 0x14: MOVBS0(Rm, Rn);				break;
	case 0x15: MOVWS0(Rm, Rn);				break;
	case 0x16: MOVLS0(Rm, Rn);				break;
	case 0x17: MULL(Rm, Rn);				break;
	case 0x18: SETT();						break;
	case 0x19: DIV0U(); 					break;
	case 0x1a: STSMACL(Rn); 				break;
	case 0x1b: SLEEP(); 					break;
	case 0x1c: MOVBL0(Rm, Rn);				break;
	case 0x1d: MOVWL0(Rm, Rn);				break;
	case 0x1e: MOVLL0(Rm, Rn);				break;
	case 0x1f: MAC_L(Rm, Rn);				break;

	case 0x20: NOP();						break;
	case 0x21: NOP();						break;
	case 0x22: STCVBR(Rn);					break;
	case 0x23: BRAF(Rn);					break;
	case 0x24: MOVBS0(Rm, Rn);				break;
	case 0x25: MOVWS0(Rm, Rn);				break;
	case 0x26: MOVLS0(Rm, Rn);				break;
	case 0x27: MULL(Rm, Rn);				break;
	case 0x28: CLRMAC();					break;
	case 0x29: MOVT(Rn);					break;
	case 0x2a: STSPR(Rn);					break;
	case 0x2b: RTE();						break;
	case 0x2c: MOVBL0(Rm, Rn);				break;
	case 0x2d: MOVWL0(Rm, Rn);				break;
	case 0x2e: MOVLL0(Rm, Rn);				break;
	case 0x2f: MAC_L(Rm, Rn);				break;

	case 0x30: NOP();						break;
	case 0x31: NOP();						break;
	case 0x32: NOP();						break;
	case 0x33: NOP();						break;
	case 0x34: MOVBS0(Rm, Rn);				break;
	case 0x35: MOVWS0(Rm, Rn);				break;
	case 0x36: MOVLS0(Rm, Rn);				break;
	case 0x37: MULL(Rm, Rn);				break;
	case 0x38: NOP();						break;
	case 0x39: NOP();						break;
	case 0x3c: MOVBL0(Rm, Rn);				break;
	case 0x3d: MOVWL0(Rm, Rn);				break;
	case 0x3e: MOVLL0(Rm, Rn);				break;
	case 0x3f: MAC_L(Rm, Rn);				break;
	case 0x3a: NOP();						break;
	case 0x3b: NOP();						break;



	}
}

INLINE void op0001(UINT16 opcode)
{
	MOVLS4(Rm, opcode & 0x0f, Rn);
}

INLINE void op0010(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBS(Rm, Rn); 				break;
	case  1: MOVWS(Rm, Rn); 				break;
	case  2: MOVLS(Rm, Rn); 				break;
	case  3: NOP(); 						break;
	case  4: MOVBM(Rm, Rn); 				break;
	case  5: MOVWM(Rm, Rn); 				break;
	case  6: MOVLM(Rm, Rn); 				break;
	case  7: DIV0S(Rm, Rn); 				break;
	case  8: TST(Rm, Rn);					break;
	case  9: AND(Rm, Rn);					break;
	case 10: XOR(Rm, Rn);					break;
	case 11: OR(Rm, Rn);					break;
	case 12: CMPSTR(Rm, Rn);				break;
	case 13: XTRCT(Rm, Rn); 				break;
	case 14: MULU(Rm, Rn);					break;
	case 15: MULS(Rm, Rn);					break;
	}
}

INLINE void op0011(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: CMPEQ(Rm, Rn); 				break;
	case  1: NOP(); 						break;
	case  2: CMPHS(Rm, Rn); 				break;
	case  3: CMPGE(Rm, Rn); 				break;
	case  4: DIV1(Rm, Rn);					break;
	case  5: DMULU(Rm, Rn); 				break;
	case  6: CMPHI(Rm, Rn); 				break;
	case  7: CMPGT(Rm, Rn); 				break;
	case  8: SUB(Rm, Rn);					break;
	case  9: NOP(); 						break;
	case 10: SUBC(Rm, Rn);					break;
	case 11: SUBV(Rm, Rn);					break;
	case 12: ADD(Rm, Rn);					break;
	case 13: DMULS(Rm, Rn); 				break;
	case 14: ADDC(Rm, Rn);					break;
	case 15: ADDV(Rm, Rn);					break;
	}
}

INLINE void op0100(UINT16 opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: SHLL(Rn);					break;
	case 0x01: SHLR(Rn);					break;
	case 0x02: STSMMACH(Rn);				break;
	case 0x03: STCMSR(Rn);					break;
	case 0x04: ROTL(Rn);					break;
	case 0x05: ROTR(Rn);					break;
	case 0x06: LDSMMACH(Rn);				break;
	case 0x07: LDCMSR(Rn);					break;
	case 0x08: SHLL2(Rn);					break;
	case 0x09: SHLR2(Rn);					break;
	case 0x0a: LDSMACH(Rn); 				break;
	case 0x0b: JSR(Rn); 					break;
	case 0x0c: NOP();						break;
	case 0x0d: NOP();						break;
	case 0x0e: LDCSR(Rn);					break;
	case 0x0f: MAC_W(Rm, Rn);				break;

	case 0x10: DT(Rn);						break;
	case 0x11: CMPPZ(Rn);					break;
	case 0x12: STSMMACL(Rn);				break;
	case 0x13: STCMGBR(Rn); 				break;
	case 0x14: NOP();						break;
	case 0x15: CMPPL(Rn);					break;
	case 0x16: LDSMMACL(Rn);				break;
	case 0x17: LDCMGBR(Rn); 				break;
	case 0x18: SHLL8(Rn);					break;
	case 0x19: SHLR8(Rn);					break;
	case 0x1a: LDSMACL(Rn); 				break;
	case 0x1b: TAS(Rn); 					break;
	case 0x1c: NOP();						break;
	case 0x1d: NOP();						break;
	case 0x1e: LDCGBR(Rn);					break;
	case 0x1f: MAC_W(Rm, Rn);				break;

	case 0x20: SHAL(Rn);					break;
	case 0x21: SHAR(Rn);					break;
	case 0x22: STSMPR(Rn);					break;
	case 0x23: STCMVBR(Rn); 				break;
	case 0x24: ROTCL(Rn);					break;
	case 0x25: ROTCR(Rn);					break;
	case 0x26: LDSMPR(Rn);					break;
	case 0x27: LDCMVBR(Rn); 				break;
	case 0x28: SHLL16(Rn);					break;
	case 0x29: SHLR16(Rn);					break;
	case 0x2a: LDSPR(Rn);					break;
	case 0x2b: JMP(Rn); 					break;
	case 0x2c: NOP();						break;
	case 0x2d: NOP();						break;
	case 0x2e: LDCVBR(Rn);					break;
	case 0x2f: MAC_W(Rm, Rn);				break;

	case 0x30: NOP();						break;
	case 0x31: NOP();						break;
	case 0x32: NOP();						break;
	case 0x33: NOP();						break;
	case 0x34: NOP();						break;
	case 0x35: NOP();						break;
	case 0x36: NOP();						break;
	case 0x37: NOP();						break;
	case 0x38: NOP();						break;
	case 0x39: NOP();						break;
	case 0x3a: NOP();						break;
	case 0x3b: NOP();						break;
	case 0x3c: NOP();						break;
	case 0x3d: NOP();						break;
	case 0x3e: NOP();						break;
	case 0x3f: MAC_W(Rm, Rn);				break;

	}
}

INLINE void op0101(UINT16 opcode)
{
	MOVLL4(Rm, opcode & 0x0f, Rn);
}

INLINE void op0110(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBL(Rm, Rn); 				break;
	case  1: MOVWL(Rm, Rn); 				break;
	case  2: MOVLL(Rm, Rn); 				break;
	case  3: MOV(Rm, Rn);					break;
	case  4: MOVBP(Rm, Rn); 				break;
	case  5: MOVWP(Rm, Rn); 				break;
	case  6: MOVLP(Rm, Rn); 				break;
	case  7: NOT(Rm, Rn);					break;
	case  8: SWAPB(Rm, Rn); 				break;
	case  9: SWAPW(Rm, Rn); 				break;
	case 10: NEGC(Rm, Rn);					break;
	case 11: NEG(Rm, Rn);					break;
	case 12: EXTUB(Rm, Rn); 				break;
	case 13: EXTUW(Rm, Rn); 				break;
	case 14: EXTSB(Rm, Rn); 				break;
	case 15: EXTSW(Rm, Rn); 				break;
	}
}

INLINE void op0111(UINT16 opcode)
{
	ADDI(opcode & 0xff, Rn);
}

INLINE void op1000(UINT16 opcode)
{
	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: MOVBS4(opcode & 0x0f, Rm); 	break;
	case  1 << 8: MOVWS4(opcode & 0x0f, Rm); 	break;
	case  2<< 8: NOP(); 				break;
	case  3<< 8: NOP(); 				break;
	case  4<< 8: MOVBL4(Rm, opcode & 0x0f); 	break;
	case  5<< 8: MOVWL4(Rm, opcode & 0x0f); 	break;
	case  6<< 8: NOP(); 				break;
	case  7<< 8: NOP(); 				break;
	case  8<< 8: CMPIM(opcode & 0xff);		break;
	case  9<< 8: BT(opcode & 0xff); 		break;
	case 10<< 8: NOP(); 				break;
	case 11<< 8: BF(opcode & 0xff); 		break;
	case 12<< 8: NOP(); 				break;
	case 13<< 8: BTS(opcode & 0xff);		break;
	case 14<< 8: NOP(); 				break;
	case 15<< 8: BFS(opcode & 0xff);		break;
	}
}


INLINE void op1001(UINT16 opcode)
{
	MOVWI(opcode & 0xff, Rn);
}

INLINE void op1010(UINT16 opcode)
{
	BRA(opcode & 0xfff);
}

INLINE void op1011(UINT16 opcode)
{
	BSR(opcode & 0xfff);
}

INLINE void op1100(UINT16 opcode)
{
	switch (opcode & (15<<8))
	{
	case  0<<8: MOVBSG(opcode & 0xff); 		break;
	case  1<<8: MOVWSG(opcode & 0xff); 		break;
	case  2<<8: MOVLSG(opcode & 0xff); 		break;
	case  3<<8: TRAPA(opcode & 0xff);		break;
	case  4<<8: MOVBLG(opcode & 0xff); 		break;
	case  5<<8: MOVWLG(opcode & 0xff); 		break;
	case  6<<8: MOVLLG(opcode & 0xff); 		break;
	case  7<<8: MOVA(opcode & 0xff);		break;
	case  8<<8: TSTI(opcode & 0xff);		break;
	case  9<<8: ANDI(opcode & 0xff);		break;
	case 10<<8: XORI(opcode & 0xff);		break;
	case 11<<8: ORI(opcode & 0xff);			break;
	case 12<<8: TSTM(opcode & 0xff);		break;
	case 13<<8: ANDM(opcode & 0xff);		break;
	case 14<<8: XORM(opcode & 0xff);		break;
	case 15<<8: ORM(opcode & 0xff);			break;
	}
}

INLINE void op1101(UINT16 opcode)
{
	MOVLI(opcode & 0xff, Rn);
}

INLINE void op1110(UINT16 opcode)
{
	MOVI(opcode & 0xff, Rn);
}

INLINE void op1111(UINT16 opcode)
{
	NOP();
}

/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

static void sh2_reset(void)
{
	void *tsave, *tsaved0, *tsaved1;
	UINT32 *m;
	int cpunum;
	int save_is_slave;

	void (*f)(UINT32 data);
	int (*save_irqcallback)(int);

	cpunum = sh2.cpu_number;
	m = sh2.m;
	tsave = sh2.timer;
	tsaved0 = sh2.dma_timer[0];
	tsaved1 = sh2.dma_timer[1];

	f = sh2.ftcsr_read_callback;
	save_irqcallback = sh2.irq_callback;
	save_is_slave = sh2.is_slave;
	memset(&sh2, 0, sizeof(SH2));
	sh2.is_slave = save_is_slave;
	sh2.ftcsr_read_callback = f;
	sh2.irq_callback = save_irqcallback;

	sh2.timer = tsave;
	sh2.dma_timer[0] = tsaved0;
	sh2.dma_timer[1] = tsaved1;
	sh2.cpu_number = cpunum;
	sh2.m = m;
	memset(sh2.m, 0, 0x200);

	sh2.pc = RL(0);
	sh2.r[15] = RL(4);
	sh2.sr = I;
	change_pc(sh2.pc & AM);

	sh2.internal_irq_level = -1;
}

/* Execute cycles - returns number of cycles actually run */
static int sh2_execute(int cycles)
{
	sh2_icount = cycles;

	if (sh2.cpu_off)
		return 0;

	do
	{
		UINT32 opcode;

		if (sh2.delay)
		{
			opcode = cpu_readop16(WORD_XOR_BE((UINT32)(sh2.delay & AM)));
			change_pc(sh2.pc & AM);
			sh2.pc -= 2;
		}
		else
			opcode = cpu_readop16(WORD_XOR_BE((UINT32)(sh2.pc & AM)));

		CALL_DEBUGGER(sh2.pc);

		sh2.delay = 0;
		sh2.pc += 2;
		sh2.ppc = sh2.pc;

		switch (opcode & ( 15 << 12))
		{
		case  0<<12: op0000(opcode); break;
		case  1<<12: op0001(opcode); break;
		case  2<<12: op0010(opcode); break;
		case  3<<12: op0011(opcode); break;
		case  4<<12: op0100(opcode); break;
		case  5<<12: op0101(opcode); break;
		case  6<<12: op0110(opcode); break;
		case  7<<12: op0111(opcode); break;
		case  8<<12: op1000(opcode); break;
		case  9<<12: op1001(opcode); break;
		case 10<<12: op1010(opcode); break;
		case 11<<12: op1011(opcode); break;
		case 12<<12: op1100(opcode); break;
		case 13<<12: op1101(opcode); break;
		case 14<<12: op1110(opcode); break;
		default: op1111(opcode); break;
		}

		if(sh2.test_irq && !sh2.delay)
		{
			CHECK_PENDING_IRQ("mame_sh2_execute");
			sh2.test_irq = 0;
		}
		sh2_icount--;
	} while( sh2_icount > 0 );

	return cycles - sh2_icount;
}

/* Get registers, return context size */
static void sh2_get_context(void *dst)
{
	if( dst )
		memcpy(dst, &sh2, sizeof(SH2));
}

/* Set registers */
static void sh2_set_context(void *src)
{
	if( src )
		memcpy(&sh2, src, sizeof(SH2));
}

static void sh2_timer_resync(void)
{
	int divider = div_tab[(sh2.m[5] >> 8) & 3];
	UINT64 cur_time = cpunum_gettotalcycles(sh2.cpu_number);

	if(divider)
		sh2.frc += (cur_time - sh2.frc_base) >> divider;
	sh2.frc_base = cur_time;
}

static void sh2_timer_activate(void)
{
	int max_delta = 0xfffff;
	UINT16 frc;

	timer_adjust_oneshot(sh2.timer, attotime_never, 0);

	frc = sh2.frc;
	if(!(sh2.m[4] & OCFA)) {
		UINT16 delta = sh2.ocra - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(sh2.m[4] & OCFB) && (sh2.ocra <= sh2.ocrb || !(sh2.m[4] & 0x010000))) {
		UINT16 delta = sh2.ocrb - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(sh2.m[4] & OVF) && !(sh2.m[4] & 0x010000)) {
		int delta = 0x10000 - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(max_delta != 0xfffff) {
		int divider = div_tab[(sh2.m[5] >> 8) & 3];
		if(divider) {
			max_delta <<= divider;
			sh2.frc_base = cpunum_gettotalcycles(sh2.cpu_number);
			timer_adjust_oneshot(sh2.timer, ATTOTIME_IN_CYCLES(max_delta, sh2.cpu_number), sh2.cpu_number);
		} else {
			logerror("SH2.%d: Timer event in %d cycles of external clock", sh2.cpu_number, max_delta);
		}
	}
}

static void sh2_recalc_irq(void)
{
	int irq = 0, vector = -1;
	int  level;

	// Timer irqs
	if((sh2.m[4]>>8) & sh2.m[4] & (ICF|OCFA|OCFB|OVF))
	{
		level = (sh2.m[0x18] >> 24) & 15;
		if(level > irq)
		{
			int mask = (sh2.m[4]>>8) & sh2.m[4];
			irq = level;
			if(mask & ICF)
				vector = (sh2.m[0x19] >> 8) & 0x7f;
			else if(mask & (OCFA|OCFB))
				vector = sh2.m[0x19] & 0x7f;
			else
				vector = (sh2.m[0x1a] >> 24) & 0x7f;
		}
	}

	// DMA irqs
	if((sh2.m[0x63] & 6) == 6) {
		level = (sh2.m[0x38] >> 8) & 15;
		if(level > irq) {
			irq = level;
			vector = (sh2.m[0x68] >> 24) & 0x7f;
		}
	}

	if((sh2.m[0x67] & 6) == 6) {
		level = (sh2.m[0x38] >> 8) & 15;
		if(level > irq) {
			irq = level;
			vector = (sh2.m[0x6a] >> 24) & 0x7f;
		}
	}


	sh2.internal_irq_level = irq;
	sh2.internal_irq_vector = vector;
	sh2.test_irq = 1;
}

static TIMER_CALLBACK( sh2_timer_callback )
{
	UINT16 frc;
	int cpunum = param;

	cpuintrf_push_context(cpunum);
	sh2_timer_resync();

	frc = sh2.frc;

	if(frc == sh2.ocrb)
		sh2.m[4] |= OCFB;

	if(frc == 0x0000)
		sh2.m[4] |= OVF;

	if(frc == sh2.ocra)
	{
		sh2.m[4] |= OCFA;

		if(sh2.m[4] & 0x010000)
			sh2.frc = 0;
	}

	sh2_recalc_irq();
	sh2_timer_activate();

	cpuintrf_pop_context();
}

static TIMER_CALLBACK( sh2_dmac_callback )
{
	int cpunum = param >> 1;
	int dma = param & 1;

	cpuintrf_push_context(cpunum);
	LOG(("SH2.%d: DMA %d complete\n", cpunum, dma));
	sh2.m[0x63+4*dma] |= 2;
	sh2.dma_timer_active[dma] = 0;
	sh2_recalc_irq();
	cpuintrf_pop_context();
}

static void sh2_dmac_check(int dma)
{
	if(sh2.m[0x63+4*dma] & sh2.m[0x6c] & 1)
	{
		if(!sh2.dma_timer_active[dma] && !(sh2.m[0x63+4*dma] & 2))
		{
			int incs, incd, size;
			UINT32 src, dst, count;
			incd = (sh2.m[0x63+4*dma] >> 14) & 3;
			incs = (sh2.m[0x63+4*dma] >> 12) & 3;
			size = (sh2.m[0x63+4*dma] >> 10) & 3;
			if(incd == 3 || incs == 3)
			{
				logerror("SH2: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, sh2.m[0x63+4*dma]);
				return;
			}
			src   = sh2.m[0x60+4*dma];
			dst   = sh2.m[0x61+4*dma];
			count = sh2.m[0x62+4*dma];
			if(!count)
				count = 0x1000000;

			LOG(("SH2: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", dma, src, dst, count, sh2.m[0x63+4*dma], incs, incd, size));

			sh2.dma_timer_active[dma] = 1;
			timer_adjust_oneshot(sh2.dma_timer[dma], ATTOTIME_IN_CYCLES(2*count+1, sh2.cpu_number), (sh2.cpu_number<<1)|dma);

			src &= AM;
			dst &= AM;

			switch(size)
			{
			case 0:
				for(;count > 0; count --)
				{
					if(incs == 2)
						src --;
					if(incd == 2)
						dst --;
					program_write_byte_32be(dst, program_read_byte_32be(src));
					if(incs == 1)
						src ++;
					if(incd == 1)
						dst ++;
				}
				break;
			case 1:
				src &= ~1;
				dst &= ~1;
				for(;count > 0; count --)
				{
					if(incs == 2)
						src -= 2;
					if(incd == 2)
						dst -= 2;
					program_write_word_32be(dst, program_read_word_32be(src));
					if(incs == 1)
						src += 2;
					if(incd == 1)
						dst += 2;
				}
				break;
			case 2:
				src &= ~3;
				dst &= ~3;
				for(;count > 0; count --)
				{
					if(incs == 2)
						src -= 4;
					if(incd == 2)
						dst -= 4;
					program_write_dword_32be(dst, program_read_dword_32be(src));
					if(incs == 1)
						src += 4;
					if(incd == 1)
						dst += 4;

				}
				break;
			case 3:
				src &= ~3;
				dst &= ~3;
				count &= ~3;
				for(;count > 0; count -= 4)
				{
					if(incd == 2)
						dst -= 16;
					program_write_dword_32be(dst, program_read_dword_32be(src));
					program_write_dword_32be(dst+4, program_read_dword_32be(src+4));
					program_write_dword_32be(dst+8, program_read_dword_32be(src+8));
					program_write_dword_32be(dst+12, program_read_dword_32be(src+12));
					src += 16;
					if(incd == 1)
						dst += 16;
				}
				break;
			}
		}
	}
	else
	{
		if(sh2.dma_timer_active[dma])
		{
			logerror("SH2: DMA %d cancelled in-flight", dma);
			timer_adjust_oneshot(sh2.dma_timer[dma], attotime_never, 0);
			sh2.dma_timer_active[dma] = 0;
		}
	}
}

WRITE32_HANDLER( sh2_internal_w )
{
	UINT32 old = sh2.m[offset];
	COMBINE_DATA(sh2.m+offset);

	//  if(offset != 0x20)
	//      logerror("sh2_internal_w:  Write %08x (%x), %08x @ %08x\n", 0xfffffe00+offset*4, offset, data, mem_mask);

	switch( offset )
	{
		// Timers
	case 0x04: // TIER, FTCSR, FRC
		if((mem_mask & 0x00ffffff) != 0xffffff)
			sh2_timer_resync();
		logerror("SH2.%d: TIER write %04x @ %04x\n", sh2.cpu_number, data >> 16, mem_mask>>16);
		sh2.m[4] = (sh2.m[4] & ~(ICF|OCFA|OCFB|OVF)) | (old & sh2.m[4] & (ICF|OCFA|OCFB|OVF));
		COMBINE_DATA(&sh2.frc);
		if((mem_mask & 0x00ffffff) != 0xffffff)
			sh2_timer_activate();
		sh2_recalc_irq();
		break;
	case 0x05: // OCRx, TCR, TOCR
		logerror("SH2.%d: TCR write %08x @ %08x\n", sh2.cpu_number, data, mem_mask);
		sh2_timer_resync();
		if(sh2.m[5] & 0x10)
			sh2.ocrb = (sh2.ocrb & (mem_mask >> 16)) | ((data & ~mem_mask) >> 16);
		else
			sh2.ocra = (sh2.ocra & (mem_mask >> 16)) | ((data & ~mem_mask) >> 16);
		sh2_timer_activate();
		break;

	case 0x06: // ICR
		break;

		// Interrupt vectors
	case 0x18: // IPRB, VCRA
	case 0x19: // VCRB, VCRC
	case 0x1a: // VCRD
		sh2_recalc_irq();
		break;

		// DMA
	case 0x1c: // DRCR0, DRCR1
		break;

		// Watchdog
	case 0x20: // WTCNT, RSTCSR
		break;

		// Standby and cache
	case 0x24: // SBYCR, CCR
		break;

		// Interrupt vectors cont.
	case 0x38: // ICR, IRPA
		break;
	case 0x39: // VCRWDT
		break;

		// Division box
	case 0x40: // DVSR
		break;
	case 0x41: // DVDNT
		{
			INT32 a = sh2.m[0x41];
			INT32 b = sh2.m[0x40];
			LOG(("SH2 #%d div+mod %d/%d\n", cpu_getactivecpu(), a, b));
			if (b)
			{
				sh2.m[0x45] = a / b;
				sh2.m[0x44] = a % b;
			}
			else
			{
				sh2.m[0x42] |= 0x00010000;
				sh2.m[0x45] = 0x7fffffff;
				sh2.m[0x44] = 0x7fffffff;
				sh2_recalc_irq();
			}
			break;
		}
	case 0x42: // DVCR
		sh2.m[0x42] = (sh2.m[0x42] & ~0x00001000) | (old & sh2.m[0x42] & 0x00010000);
		sh2_recalc_irq();
		break;
	case 0x43: // VCRDIV
		sh2_recalc_irq();
		break;
	case 0x44: // DVDNTH
		break;
	case 0x45: // DVDNTL
		{
			INT64 a = sh2.m[0x45] | ((UINT64)(sh2.m[0x44]) << 32);
			INT64 b = (INT32)sh2.m[0x40];
			LOG(("SH2 #%d div+mod %lld/%lld\n", cpu_getactivecpu(), a, b));
			if (b)
			{
				INT64 q = a / b;
				if (q != (INT32)q)
				{
					sh2.m[0x42] |= 0x00010000;
					sh2.m[0x45] = 0x7fffffff;
					sh2.m[0x44] = 0x7fffffff;
					sh2_recalc_irq();
				}
				else
				{
					sh2.m[0x45] = q;
					sh2.m[0x44] = a % b;
				}
			}
			else
			{
				sh2.m[0x42] |= 0x00010000;
				sh2.m[0x45] = 0x7fffffff;
				sh2.m[0x44] = 0x7fffffff;
				sh2_recalc_irq();
			}
			break;
		}

		// DMA controller
	case 0x60: // SAR0
	case 0x61: // DAR0
		break;
	case 0x62: // DTCR0
		sh2.m[0x62] &= 0xffffff;
		break;
	case 0x63: // CHCR0
		sh2.m[0x63] = (sh2.m[0x63] & ~2) | (old & sh2.m[0x63] & 2);
		sh2_dmac_check(0);
		break;
	case 0x64: // SAR1
	case 0x65: // DAR1
		break;
	case 0x66: // DTCR1
		sh2.m[0x66] &= 0xffffff;
		break;
	case 0x67: // CHCR1
		sh2.m[0x67] = (sh2.m[0x67] & ~2) | (old & sh2.m[0x67] & 2);
		sh2_dmac_check(1);
		break;
	case 0x68: // VCRDMA0
	case 0x6a: // VCRDMA1
		sh2_recalc_irq();
		break;
	case 0x6c: // DMAOR
		sh2.m[0x6c] = (sh2.m[0x6c] & ~6) | (old & sh2.m[0x6c] & 6);
		sh2_dmac_check(0);
		sh2_dmac_check(1);
		break;

		// Bus controller
	case 0x78: // BCR1
	case 0x79: // BCR2
	case 0x7a: // WCR
	case 0x7b: // MCR
	case 0x7c: // RTCSR
	case 0x7d: // RTCNT
	case 0x7e: // RTCOR
		break;

	default:
		logerror("sh2_internal_w:  Unmapped write %08x, %08x @ %08x\n", 0xfffffe00+offset*4, data, mem_mask);
		break;
	}
}

READ32_HANDLER( sh2_internal_r )
{
	//  logerror("sh2_internal_r:  Read %08x (%x) @ %08x\n", 0xfffffe00+offset*4, offset, mem_mask);
	switch( offset )
	{
	case 0x04: // TIER, FTCSR, FRC
		if ( mem_mask == 0xff00ffff )
			if ( sh2.ftcsr_read_callback != NULL )
				sh2.ftcsr_read_callback( (sh2.m[4] & 0xffff0000) | sh2.frc );
		sh2_timer_resync();
		return (sh2.m[4] & 0xffff0000) | sh2.frc;
	case 0x05: // OCRx, TCR, TOCR
		if(sh2.m[5] & 0x10)
			return (sh2.ocrb << 16) | (sh2.m[5] & 0xffff);
		else
			return (sh2.ocra << 16) | (sh2.m[5] & 0xffff);
	case 0x06: // ICR
		return sh2.icr << 16;

	case 0x38: // ICR, IPRA
		return (sh2.m[0x38] & 0x7fffffff) | (sh2.nmi_line_state == ASSERT_LINE ? 0 : 0x80000000);

	case 0x78: // BCR1
		return sh2.is_slave ? 0x00008000 : 0;

	case 0x41: // dvdntl mirrors
	case 0x47:
		return sh2.m[0x45];

	case 0x46: // dvdnth mirror
		return sh2.m[0x44];
	}
	return sh2.m[offset];
}

static void sh2_set_frt_input(int cpunum, int state)
{
	if(state == PULSE_LINE)
	{
		sh2_set_frt_input(cpunum, ASSERT_LINE);
		sh2_set_frt_input(cpunum, CLEAR_LINE);
		return;
	}

	cpuintrf_push_context(cpunum);

	if(sh2.frt_input == state) {
		cpuintrf_pop_context();
		return;
	}

	sh2.frt_input = state;

	if(sh2.m[5] & 0x8000) {
		if(state == CLEAR_LINE) {
			cpuintrf_pop_context();
			return;
		}
	} else {
		if(state == ASSERT_LINE) {
			cpuintrf_pop_context();
			return;
		}
	}

	sh2_timer_resync();
	sh2.icr = sh2.frc;
	sh2.m[4] |= ICF;
	logerror("SH2.%d: ICF activated (%x)\n", sh2.cpu_number, sh2.pc & AM);
	sh2_recalc_irq();
	cpuintrf_pop_context();
}

static void set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		if (sh2.nmi_line_state == state)
			return;
		sh2.nmi_line_state = state;

		if( state == CLEAR_LINE )
		{
			LOG(("SH-2 #%d cleared nmi\n", cpu_getactivecpu()));
		}
		else
		{
			LOG(("SH-2 #%d assert nmi\n", cpu_getactivecpu()));
			sh2_exception("sh2_set_irq_line/nmi", 16);
		}
	}
	else
	{
		if (sh2.irq_line_state[irqline] == state)
			return;
		sh2.irq_line_state[irqline] = state;

		if( state == CLEAR_LINE )
		{
			LOG(("SH-2 #%d cleared irq #%d\n", cpu_getactivecpu(), irqline));
			sh2.pending_irq &= ~(1 << irqline);
		}
		else
		{
			LOG(("SH-2 #%d assert irq #%d\n", cpu_getactivecpu(), irqline));
			sh2.pending_irq |= 1 << irqline;
			if(sh2.delay)
				sh2.test_irq = 1;
			else
				CHECK_PENDING_IRQ("sh2_set_irq_line");
		}
	}
}

#ifdef ENABLE_DEBUGGER
static offs_t sh2_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return DasmSH2( buffer, pc, (oprom[0] << 8) | oprom[1] );
}
#endif /* ENABLE_DEBUGGER */

static void sh2_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const struct sh2_config *conf = config;

	sh2.timer = timer_alloc(sh2_timer_callback, NULL);
	timer_adjust_oneshot(sh2.timer, attotime_never, 0);

	sh2.dma_timer[0] = timer_alloc(sh2_dmac_callback, NULL);
	timer_adjust_oneshot(sh2.dma_timer[0], attotime_never, 0);

	sh2.dma_timer[1] = timer_alloc(sh2_dmac_callback, NULL);
	timer_adjust_oneshot(sh2.dma_timer[1], attotime_never, 0);

	sh2.m = auto_malloc(0x200);

	if(conf)
		sh2.is_slave = conf->is_slave;
	else
		sh2.is_slave = 0;

	sh2.cpu_number = index;
	sh2.irq_callback = irqcallback;

	state_save_register_item("sh2", index, sh2.pc);
	state_save_register_item("sh2", index, sh2.r[15]);
	state_save_register_item("sh2", index, sh2.sr);
	state_save_register_item("sh2", index, sh2.pr);
	state_save_register_item("sh2", index, sh2.gbr);
	state_save_register_item("sh2", index, sh2.vbr);
	state_save_register_item("sh2", index, sh2.mach);
	state_save_register_item("sh2", index, sh2.macl);
	state_save_register_item("sh2", index, sh2.r[ 0]);
	state_save_register_item("sh2", index, sh2.r[ 1]);
	state_save_register_item("sh2", index, sh2.r[ 2]);
	state_save_register_item("sh2", index, sh2.r[ 3]);
	state_save_register_item("sh2", index, sh2.r[ 4]);
	state_save_register_item("sh2", index, sh2.r[ 5]);
	state_save_register_item("sh2", index, sh2.r[ 6]);
	state_save_register_item("sh2", index, sh2.r[ 7]);
	state_save_register_item("sh2", index, sh2.r[ 8]);
	state_save_register_item("sh2", index, sh2.r[ 9]);
	state_save_register_item("sh2", index, sh2.r[10]);
	state_save_register_item("sh2", index, sh2.r[11]);
	state_save_register_item("sh2", index, sh2.r[12]);
	state_save_register_item("sh2", index, sh2.r[13]);
	state_save_register_item("sh2", index, sh2.r[14]);
	state_save_register_item("sh2", index, sh2.ea);
}



/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void sh2_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLIN:	set_irq_line(SH2_INT_VBLIN, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLOUT:	set_irq_line(SH2_INT_VBLOUT, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_HBLIN:	set_irq_line(SH2_INT_HBLIN, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER0:	set_irq_line(SH2_INT_TIMER0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER1:	set_irq_line(SH2_INT_TIMER1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DSP:		set_irq_line(SH2_INT_DSP, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SOUND:	set_irq_line(SH2_INT_SOUND, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SMPC:	set_irq_line(SH2_INT_SMPC, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_PAD:		set_irq_line(SH2_INT_PAD, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA2:	set_irq_line(SH2_INT_DMA2, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA1:	set_irq_line(SH2_INT_DMA1, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA0:	set_irq_line(SH2_INT_DMA0, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMAILL:	set_irq_line(SH2_INT_DMAILL, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SPRITE:	set_irq_line(SH2_INT_SPRITE, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_14:		set_irq_line(SH2_INT_14, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_15:		set_irq_line(SH2_INT_15, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_ABUS:	set_irq_line(SH2_INT_ABUS, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_REGISTER + SH2_PC:
		case CPUINFO_INT_PC:							sh2.pc = info->i; sh2.delay = 0;		break;
		case CPUINFO_INT_SP:							sh2.r[15] = info->i;    				break;
		case CPUINFO_INT_REGISTER + SH2_PR:   			sh2.pr = info->i;	   					break;
		case CPUINFO_INT_REGISTER + SH2_SR:				sh2.sr = info->i; CHECK_PENDING_IRQ("sh2_set_reg"); break;
		case CPUINFO_INT_REGISTER + SH2_GBR:			sh2.gbr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_VBR:			sh2.vbr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_MACH: 			sh2.mach = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_MACL:			sh2.macl = info->i;						break;
		case CPUINFO_INT_REGISTER + SH2_R0:				sh2.r[ 0] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R1:				sh2.r[ 1] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R2:				sh2.r[ 2] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R3:				sh2.r[ 3] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R4:				sh2.r[ 4] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R5:				sh2.r[ 5] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R6:				sh2.r[ 6] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R7:				sh2.r[ 7] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R8:				sh2.r[ 8] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R9:				sh2.r[ 9] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R10:			sh2.r[10] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R11:			sh2.r[11] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R12:			sh2.r[12] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R13:			sh2.r[13] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R14:			sh2.r[14] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_R15:			sh2.r[15] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH2_EA:				sh2.ea = info->i;						break;

		case CPUINFO_INT_SH2_FRT_INPUT:					sh2_set_frt_input(cpu_getactivecpu(), info->i); break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK:		sh2.ftcsr_read_callback = (void (*) (UINT32 ))info->f; break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

void sh2_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(sh2);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 16;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_BE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLIN:	info->i = sh2.irq_line_state[SH2_INT_VBLIN]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_VBLOUT:	info->i = sh2.irq_line_state[SH2_INT_VBLOUT]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_HBLIN:	info->i = sh2.irq_line_state[SH2_INT_HBLIN]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER0:	info->i = sh2.irq_line_state[SH2_INT_TIMER0]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_TIMER1:	info->i = sh2.irq_line_state[SH2_INT_TIMER1]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DSP:		info->i = sh2.irq_line_state[SH2_INT_DSP]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SOUND:	info->i = sh2.irq_line_state[SH2_INT_SOUND]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SMPC:	info->i = sh2.irq_line_state[SH2_INT_SMPC];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_PAD:		info->i = sh2.irq_line_state[SH2_INT_PAD]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA2:	info->i = sh2.irq_line_state[SH2_INT_DMA2];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA1:	info->i = sh2.irq_line_state[SH2_INT_DMA1];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMA0:	info->i = sh2.irq_line_state[SH2_INT_DMA0];	break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_DMAILL:	info->i = sh2.irq_line_state[SH2_INT_DMAILL]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_SPRITE:	info->i = sh2.irq_line_state[SH2_INT_SPRITE]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_14:		info->i = sh2.irq_line_state[SH2_INT_14]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_15:		info->i = sh2.irq_line_state[SH2_INT_15]; break;
		case CPUINFO_INT_INPUT_STATE + SH2_INT_ABUS:	info->i = sh2.irq_line_state[SH2_INT_ABUS];	break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = sh2.nmi_line_state;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = sh2.ppc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SH2_PC:				info->i = (sh2.delay) ? (sh2.delay & AM) : (sh2.pc & AM); break;
		case CPUINFO_INT_SP:   							info->i = sh2.r[15];					break;
		case CPUINFO_INT_REGISTER + SH2_PR:				info->i = sh2.pr;						break;
		case CPUINFO_INT_REGISTER + SH2_SR:				info->i = sh2.sr;						break;
		case CPUINFO_INT_REGISTER + SH2_GBR:			info->i = sh2.gbr;						break;
		case CPUINFO_INT_REGISTER + SH2_VBR:			info->i = sh2.vbr;						break;
		case CPUINFO_INT_REGISTER + SH2_MACH:			info->i = sh2.mach;						break;
		case CPUINFO_INT_REGISTER + SH2_MACL:			info->i = sh2.macl;						break;
		case CPUINFO_INT_REGISTER + SH2_R0:				info->i = sh2.r[ 0];					break;
		case CPUINFO_INT_REGISTER + SH2_R1:				info->i = sh2.r[ 1];					break;
		case CPUINFO_INT_REGISTER + SH2_R2:				info->i = sh2.r[ 2];					break;
		case CPUINFO_INT_REGISTER + SH2_R3:				info->i = sh2.r[ 3];					break;
		case CPUINFO_INT_REGISTER + SH2_R4:				info->i = sh2.r[ 4];					break;
		case CPUINFO_INT_REGISTER + SH2_R5:				info->i = sh2.r[ 5];					break;
		case CPUINFO_INT_REGISTER + SH2_R6:				info->i = sh2.r[ 6];					break;
		case CPUINFO_INT_REGISTER + SH2_R7:				info->i = sh2.r[ 7];					break;
		case CPUINFO_INT_REGISTER + SH2_R8:				info->i = sh2.r[ 8];					break;
		case CPUINFO_INT_REGISTER + SH2_R9:				info->i = sh2.r[ 9];					break;
		case CPUINFO_INT_REGISTER + SH2_R10:			info->i = sh2.r[10];					break;
		case CPUINFO_INT_REGISTER + SH2_R11:			info->i = sh2.r[11];					break;
		case CPUINFO_INT_REGISTER + SH2_R12:			info->i = sh2.r[12];					break;
		case CPUINFO_INT_REGISTER + SH2_R13:			info->i = sh2.r[13];					break;
		case CPUINFO_INT_REGISTER + SH2_R14:			info->i = sh2.r[14];					break;
		case CPUINFO_INT_REGISTER + SH2_R15:			info->i = sh2.r[15];					break;
		case CPUINFO_INT_REGISTER + SH2_EA:				info->i = sh2.ea;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = sh2_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = sh2_get_context;		break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = sh2_set_context;		break;
		case CPUINFO_PTR_INIT:							info->init = sh2_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = sh2_reset;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = sh2_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef ENABLE_DEBUGGER
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = sh2_dasm;			break;
#endif /* ENABLE_DEBUGGER */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &sh2_icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "SH-2");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Hitachi SH7600");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.01");				break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%d%c%c",
					sh2.sr & M ? 'M':'.',
					sh2.sr & Q ? 'Q':'.',
					(sh2.sr & I) >> 4,
					sh2.sr & S ? 'S':'.',
					sh2.sr & T ? 'T':'.');
			break;

		case CPUINFO_STR_REGISTER + SH2_PC:				sprintf(info->s, "PC  :%08X", sh2.pc); break;
		case CPUINFO_STR_REGISTER + SH2_SR:				sprintf(info->s, "SR  :%08X", sh2.sr); break;
		case CPUINFO_STR_REGISTER + SH2_PR:				sprintf(info->s, "PR  :%08X", sh2.pr); break;
		case CPUINFO_STR_REGISTER + SH2_GBR:			sprintf(info->s, "GBR :%08X", sh2.gbr); break;
		case CPUINFO_STR_REGISTER + SH2_VBR:			sprintf(info->s, "VBR :%08X", sh2.vbr); break;
		case CPUINFO_STR_REGISTER + SH2_MACH:			sprintf(info->s, "MACH:%08X", sh2.mach); break;
		case CPUINFO_STR_REGISTER + SH2_MACL:			sprintf(info->s, "MACL:%08X", sh2.macl); break;
		case CPUINFO_STR_REGISTER + SH2_R0:				sprintf(info->s, "R0  :%08X", sh2.r[ 0]); break;
		case CPUINFO_STR_REGISTER + SH2_R1:				sprintf(info->s, "R1  :%08X", sh2.r[ 1]); break;
		case CPUINFO_STR_REGISTER + SH2_R2:				sprintf(info->s, "R2  :%08X", sh2.r[ 2]); break;
		case CPUINFO_STR_REGISTER + SH2_R3:				sprintf(info->s, "R3  :%08X", sh2.r[ 3]); break;
		case CPUINFO_STR_REGISTER + SH2_R4:				sprintf(info->s, "R4  :%08X", sh2.r[ 4]); break;
		case CPUINFO_STR_REGISTER + SH2_R5:				sprintf(info->s, "R5  :%08X", sh2.r[ 5]); break;
		case CPUINFO_STR_REGISTER + SH2_R6:				sprintf(info->s, "R6  :%08X", sh2.r[ 6]); break;
		case CPUINFO_STR_REGISTER + SH2_R7:				sprintf(info->s, "R7  :%08X", sh2.r[ 7]); break;
		case CPUINFO_STR_REGISTER + SH2_R8:				sprintf(info->s, "R8  :%08X", sh2.r[ 8]); break;
		case CPUINFO_STR_REGISTER + SH2_R9:				sprintf(info->s, "R9  :%08X", sh2.r[ 9]); break;
		case CPUINFO_STR_REGISTER + SH2_R10:			sprintf(info->s, "R10 :%08X", sh2.r[10]); break;
		case CPUINFO_STR_REGISTER + SH2_R11:			sprintf(info->s, "R11 :%08X", sh2.r[11]); break;
		case CPUINFO_STR_REGISTER + SH2_R12:			sprintf(info->s, "R12 :%08X", sh2.r[12]); break;
		case CPUINFO_STR_REGISTER + SH2_R13:			sprintf(info->s, "R13 :%08X", sh2.r[13]); break;
		case CPUINFO_STR_REGISTER + SH2_R14:			sprintf(info->s, "R14 :%08X", sh2.r[14]); break;
		case CPUINFO_STR_REGISTER + SH2_R15:			sprintf(info->s, "R15 :%08X", sh2.r[15]); break;
		case CPUINFO_STR_REGISTER + SH2_EA:				sprintf(info->s, "EA  :%08X", sh2.ea);    break;

		case CPUINFO_PTR_SH2_FTCSR_READ_CALLBACK:		info->f = (genf*)sh2.ftcsr_read_callback; break;

	}
}
