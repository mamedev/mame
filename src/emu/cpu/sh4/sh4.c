/*****************************************************************************
 *
 *   sh4.c
 *   Portable Hitachi SH-4 (SH7750 family) emulator
 *
 *   By R. Belmont, based on sh2.c by Juergen Buchmueller, Mariusz Wojcieszek,
 *      Olivier Galibert, Sylvain Glaize, and James Forshaw.
 *
 *
 *   TODO: FPU
 *         DMA
 *         on-board peripherals
 *
 *   DONE: boot/reset setup
 *         64-bit data bus
 *         banked registers
 *         additional registers for supervisor mode
 *         FPU status and data registers
 *         state save for the new registers
 *         interrupts
 *         store queues
 *
 *****************************************************************************/

#include "debugger.h"
#include "sh4.h"
#include "sh4regs.h"
#include <math.h>

/* speed up delay loops, bail out of tight loops */
#define BUSY_LOOP_HACKS 	0
/* work only as described in the datasheet */
#define DATASHEET_STRICT	0

#define VERBOSE 0

#if VERBOSE
#define LOG(x)	logerror x
#else
#define LOG(x)
#endif

#define EXPPRI(pl,po,p,n)	(((4-(pl)) << 24) | ((15-(po)) << 16) | ((p) << 8) | (255-(n)))
#define NMIPRI()			EXPPRI(3,0,16,SH4_INTC_NMI)
#define INTPRI(p,n)			EXPPRI(4,2,p,n)

#define FP_RS(r) sh4.fr[(r)] // binary representation of single precision floating point register r
#define FP_RFS(r) *( (float  *)(sh4.fr+(r)) ) // single precision floating point register r
#define FP_RFD(r) *( (double *)(sh4.fr+(r)) ) // double precision floating point register r
#define FP_XS(r) sh4.xf[(r)] // binary representation of extended single precision floating point register r
#define FP_XFS(r) *( (float  *)(sh4.xf+(r)) ) // single precision extended floating point register r
#define FP_XFD(r) *( (double *)(sh4.xf+(r)) ) // double precision extended floating point register r
#ifdef LSB_FIRST
#define FP_RS2(r) sh4.fr[(r) ^ sh4.fpu_pr]
#define FP_RFS2(r) *( (float  *)(sh4.fr+((r) ^ sh4.fpu_pr)) ) 
#define FP_XS2(r) sh4.xf[(r) ^ sh4.fpu_pr]
#define FP_XFS2(r) *( (float  *)(sh4.xf+((r) ^ sh4.fpu_pr)) ) 
#endif

typedef struct
{
	UINT32	ppc;
	UINT32	pc, spc;
	UINT32	pr;
	UINT32	sr, ssr;
	UINT32	gbr, vbr;
	UINT32	mach, macl;
	UINT32	r[16], rbnk[2][8], sgr;
	UINT32  fr[16], xf[16];
	UINT32	ea;
	UINT32	delay;
	UINT32	cpu_off;
	UINT32	pending_irq;
	UINT32  test_irq;
	UINT32  fpscr;
	UINT32  fpul;
	UINT32  dbr;

	UINT32	exception_priority[128];
	int		exception_requesting[128];

	INT8	irq_line_state[17];
	int 	(*irq_callback)(int irqline);
	UINT32	*m;
	INT8	nmi_line_state;

	int		frt_input;
	int		irln;
	int 	internal_irq_level;
	int 	internal_irq_vector;

	emu_timer *dma_timer[4];
	emu_timer *refresh_timer;
	emu_timer *rtc_timer;
	emu_timer *timer0;
	emu_timer *timer1;
	emu_timer *timer2;
	UINT32	refresh_timer_base;
	int     dma_timer_active[2];

	int     is_slave, cpu_number;
	int		cpu_clock, bus_clock, pm_clock;
	int		fpu_sz, fpu_pr;

	void	(*ftcsr_read_callback)(UINT32 data);
} SH4;

static int sh4_icount;
static SH4 sh4;

static const int tcnt_div[8] = { 4, 16, 64, 256, 1024, 1, 1, 1 };
static const int rtcnt_div[8] = { 0, 4, 16, 64, 256, 1024, 2048, 4096 };
static const int daysmonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static const int dmasize[8] = { 8, 1, 2, 4, 32, 0, 0, 0 };
static const UINT32 exception_priority_default[] = { EXPPRI(1,1,0,0), EXPPRI(1,2,0,1), EXPPRI(1,1,0,2), EXPPRI(1,3,0,3), EXPPRI(1,4,0,4),
	EXPPRI(2,0,0,5), EXPPRI(2,1,0,6), EXPPRI(2,2,0,7), EXPPRI(2,3,0,8), EXPPRI(2,4,0,9), EXPPRI(2,4,0,10), EXPPRI(2,4,0,11), EXPPRI(2,4,0,12),
	EXPPRI(2,5,0,13), EXPPRI(2,5,0,14), EXPPRI(2,6,0,15), EXPPRI(2,6,0,16), EXPPRI(2,7,0,17), EXPPRI(2,7,0,18), EXPPRI(2,8,0,19),
	EXPPRI(2,9,0,20), EXPPRI(2,4,0,21), EXPPRI(2,10,0,22), EXPPRI(3,0,16,SH4_INTC_NMI) };
static const int exception_codes[] = { 0x000, 0x020, 0x000, 0x140, 0x140, 0x1E0, 0x0E0, 0x040, 0x0A0, 0x180, 0x1A0, 0x800, 0x820, 0x0E0,
	0x100, 0x040, 0x060, 0x0A0, 0x0C0, 0x120, 0x080, 0x160, 0x1E0, 0x1C0, 0x200, 0x220, 0x240, 0x260, 0x280, 0x2A0, 0x2C0, 0x2E0, 0x300,
	0x320, 0x340, 0x360, 0x380, 0x3A0, 0x3C0, 0x240, 0x2A0, 0x300, 0x360, 0x600, 0x620, 0x640, 0x660, 0x680, 0x6A0, 0x780, 0x7A0, 0x7C0,
	0x7E0, 0x6C0, 0xB00, 0xB80, 0x400, 0x420, 0x440, 0x460, 0x480, 0x4A0, 0x4C0, 0x4E0, 0x500, 0x520, 0x540, 0x700, 0x720, 0x740, 0x760,
	0x560, 0x580, 0x5A0 };

enum {
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

static void set_irq_line(int irqline, int state);

/* Bits in SR */
#define T	0x00000001
#define S	0x00000002
#define I	0x000000f0
#define Q	0x00000100
#define M	0x00000200
#define FD	0x00008000
#define BL	0x10000000
#define sRB	0x20000000
#define MD	0x40000000

/* 29 bits */
#define AM	0x1fffffff

#define FLAGS	(MD|sRB|BL|FD|M|Q|I|S|T)

/* Bits in FPSCR */
#define RM  0x00000003
#define DN  0x00040000
#define PR  0x00080000
#define SZ  0x00100000
#define FR  0x00200000

#define Rn	((opcode>>8)&15)
#define Rm	((opcode>>4)&15)

/* Called for unimplemented opcodes */
static void TODO(void)
{
}

INLINE UINT8 RB(offs_t A)
{
	if (A >= 0xfe000000)
		return sh4_internal_r(((A & 0x0fc) >> 2) | ((A & 0x1fe0000) >> 11), ~(0xff << ((A & 3)*8))) >> ((A & 3)*8);

	if (A >= 0xe0000000)
		return program_read_byte_64le(A);

/*  if (A >= 0x40000000)
        return 0xa5;*/

	return program_read_byte_64le(A & AM);
}

INLINE UINT16 RW(offs_t A)
{
	if (A >= 0xfe000000)
		return sh4_internal_r(((A & 0x0fc) >> 2) | ((A & 0x1fe0000) >> 11), ~(0xffff << ((A & 2)*8))) >> ((A & 2)*8);

	if (A >= 0xe0000000)
		return program_read_word_64le(A);

/*  if (A >= 0x40000000)
        return 0xa5a5;*/

	return program_read_word_64le(A & AM);
}

INLINE UINT32 RL(offs_t A)
{
	if (A >= 0xfe000000)
		return sh4_internal_r(((A & 0x0fc) >> 2) | ((A & 0x1fe0000) >> 11), 0);

	if (A >= 0xe0000000)
		return program_read_dword_64le(A);

/*  if (A >= 0x40000000)
        return 0xa5a5a5a5;*/

  return program_read_dword_64le(A & AM);
}

INLINE void WB(offs_t A, UINT8 V)
{

	if (A >= 0xfe000000)
	{
		sh4_internal_w(((A & 0x0fc) >> 2) | ((A & 0x1fe0000) >> 11), V << ((A & 3)*8), ~(0xff << ((A & 3)*8)));
		return;
	}

	if (A >= 0xe0000000)
	{
		program_write_byte_64le(A,V);
		return;
	}

/*  if (A >= 0x40000000)
        return;*/

	program_write_byte_64le(A & AM,V);
}

INLINE void WW(offs_t A, UINT16 V)
{
	if (A >= 0xfe000000)
	{
		sh4_internal_w(((A & 0x0fc) >> 2) | ((A & 0x1fe0000) >> 11), V << ((A & 2)*8), ~(0xffff << ((A & 2)*8)));
		return;
	}

	if (A >= 0xe0000000)
	{
		program_write_word_64le(A,V);
		return;
	}

/*  if (A >= 0x40000000)
        return;*/

	program_write_word_64le(A & AM,V);
}

INLINE void WL(offs_t A, UINT32 V)
{
	if (A >= 0xfe000000)
	{
		sh4_internal_w(((A & 0x0fc) >> 2) | ((A & 0x1fe0000) >> 11), V, 0);
		return;
	}

	if (A >= 0xe0000000)
	{
		program_write_dword_64le(A,V);
		return;
	}

/*  if (A >= 0x40000000)
        return;*/

	program_write_dword_64le(A & AM,V);
}

static void sh4_change_register_bank(int to)
{
int s;

	if (to) // 0 -> 1
	{
		for (s = 0;s < 8;s++)
		{
			sh4.rbnk[0][s] = sh4.r[s];
			sh4.r[s] = sh4.rbnk[1][s];
		}
	}
	else // 1 -> 0
	{
		for (s = 0;s < 8;s++)
		{
			sh4.rbnk[1][s] = sh4.r[s];
			sh4.r[s] = sh4.rbnk[0][s];
		}
	}
}

static void sh4_swap_fp_registers(void)
{
int s;
UINT32 z;

	for (s = 0;s <= 15;s++)
	{
		z = sh4.fr[s];
		sh4.fr[s] = sh4.xf[s];
		sh4.xf[s] = z;
	}
}

#ifdef LSB_FIRST
static void sh4_swap_fp_couples(void)
{
int s;
UINT32 z;

	for (s = 0;s <= 15;s = s+2)
	{
		z = sh4.fr[s];
		sh4.fr[s] = sh4.fr[s + 1];
		sh4.fr[s + 1] = z;
		z = sh4.xf[s];
		sh4.xf[s] = sh4.xf[s + 1];
		sh4.xf[s + 1] = z;
	}
}
#endif

#ifdef MAME_DEBUG
static void sh4_syncronize_register_bank(int to)
{
int s;

	for (s = 0;s < 8;s++)
	{
		sh4.rbnk[to][s] = sh4.r[s];
	}
}
#endif

static void sh4_default_exception_priorities(void) // setup default priorities for exceptions
{
int a;

	for (a=0;a <= SH4_INTC_NMI;a++)
		sh4.exception_priority[a] = exception_priority_default[a];
	for (a=SH4_INTC_IRLn0;a <= SH4_INTC_IRLnE;a++)
		sh4.exception_priority[a] = INTPRI(15-(a-SH4_INTC_IRLn0), a);
	sh4.exception_priority[SH4_INTC_IRL0] = INTPRI(13, SH4_INTC_IRL0);
	sh4.exception_priority[SH4_INTC_IRL1] = INTPRI(10, SH4_INTC_IRL1);
	sh4.exception_priority[SH4_INTC_IRL2] = INTPRI(7, SH4_INTC_IRL2);
	sh4.exception_priority[SH4_INTC_IRL3] = INTPRI(4, SH4_INTC_IRL3);
	for (a=SH4_INTC_HUDI;a <= SH4_INTC_ROVI;a++)
		sh4.exception_priority[a] = INTPRI(0, a);
}

static void sh4_exception_recompute(void) // checks if there is any interrupt with high enough priority
{
int a,z;

	sh4.test_irq = 0;
	if ((!sh4.pending_irq) || ((sh4.sr & BL) && (sh4.exception_requesting[SH4_INTC_NMI] == 0)))
		return;
	z = (sh4.sr >> 4) & 15;
	for (a=0;a <= SH4_INTC_ROVI;a++)
	{
		if (sh4.exception_requesting[a])
			if ((((int)sh4.exception_priority[a] >> 8) & 255) > z)
			{
				sh4.test_irq = 1; // will check for exception at end of instructions
				break;
			}
	}
}

INLINE void sh4_exception_request(int exception) // start requesting an exception
{
	if (!sh4.exception_requesting[exception])
	{
		sh4.exception_requesting[exception] = 1;
		sh4.pending_irq++;
		sh4_exception_recompute();
	}
}

INLINE void sh4_exception_unrequest(int exception) // stop requesting an exception
{
	if (sh4.exception_requesting[exception])
	{
		sh4.exception_requesting[exception] = 0;
		sh4.pending_irq--;
		sh4_exception_recompute();
	}
}

INLINE void sh4_exception_checkunrequest(int exception)
{
	if (exception == SH4_INTC_NMI)
		sh4_exception_unrequest(exception);
	if ((exception == SH4_INTC_DMTE0) || (exception == SH4_INTC_DMTE1) ||
		(exception == SH4_INTC_DMTE2) || (exception == SH4_INTC_DMTE3))
		sh4_exception_unrequest(exception);
}

INLINE void sh4_exception(const char *message, int exception) // handle exception
{
	UINT32 vector;

	if (exception < SH4_INTC_NMI)
		return; // Not yet supported
	if (exception == SH4_INTC_NMI) {
		if ((sh4.sr & BL) && (!(sh4.m[ICR] & 0x200)))
			return;
		sh4.m[ICR] &= ~0x200;
		sh4.m[INTEVT] = 0x1c0;
		vector = 0x600;
		sh4.irq_callback(INPUT_LINE_NMI);
		LOG(("SH-4 #%d nmi exception after [%s]\n", cpu_getactivecpu(), message));
	} else {
//      if ((sh4.m[ICR] & 0x4000) && (sh4.nmi_line_state == ASSERT_LINE))
//          return;
		if (sh4.sr & BL)
			return;
		if (((sh4.exception_priority[exception] >> 8) & 255) <= ((sh4.sr >> 4) & 15))
			return;
		sh4.m[INTEVT] = exception_codes[exception];
		vector = 0x600;
		if ((exception >= SH4_INTC_IRL0) && (exception <= SH4_INTC_IRL3))
			sh4.irq_callback(SH4_INTC_IRL0-exception+SH4_IRL0);
		else
			sh4.irq_callback(SH4_IRL3+1);
		LOG(("SH-4 #%d interrupt exception #%d after [%s]\n", cpu_getactivecpu(), exception, message));
	}
	sh4_exception_checkunrequest(exception);

	sh4.spc = sh4.pc;
	sh4.ssr = sh4.sr;
	sh4.sgr = sh4.r[15];

	sh4.sr |= MD;
#ifdef MAME_DEBUG
	sh4_syncronize_register_bank((sh4.sr & sRB) >> 29);
#endif
	if (!(sh4.sr & sRB))
		sh4_change_register_bank(1);
	sh4.sr |= sRB;
	sh4.sr |= BL;
	sh4_exception_recompute();

	/* fetch PC */
	sh4.pc = sh4.vbr + vector;
	change_pc(sh4.pc & AM);
}

INLINE void sh4_check_pending_irq(const char *message) // look for highest priority active exception and handle it
{
int a,irq,z;

	irq = 0;
	z = -1;
	for (a=0;a <= SH4_INTC_ROVI;a++)
	{
		if (sh4.exception_requesting[a])
			if ((int)sh4.exception_priority[a] > z)
			{
				z = sh4.exception_priority[a];
				irq = a;
			}
	}
	if (z >= 0)
		sh4_exception(message, irq);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
INLINE void ADD(UINT32 m, UINT32 n)
{
	sh4.r[n] += sh4.r[m];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
INLINE void ADDI(UINT32 i, UINT32 n)
{
	sh4.r[n] += (INT32)(INT16)(INT8)i;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
INLINE void ADDC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = sh4.r[n] + sh4.r[m];
	tmp0 = sh4.r[n];
	sh4.r[n] = tmp1 + (sh4.sr & T);
	if (tmp0 > tmp1)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
	if (tmp1 > sh4.r[n])
		sh4.sr |= T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1111  1       overflow
 *  ADDV    Rm,Rn
 */
INLINE void ADDV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) sh4.r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh4.r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh4.r[n] += sh4.r[m];
	if ((INT32) sh4.r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			sh4.sr |= T;
		else
			sh4.sr &= ~T;
	}
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1001  1       -
 *  AND     Rm,Rn
 */
INLINE void AND(UINT32 m, UINT32 n)
{
	sh4.r[n] &= sh4.r[m];
}


/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
INLINE void ANDI(UINT32 i)
{
	sh4.r[0] &= i;
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
INLINE void ANDM(UINT32 i)
{
	UINT32 temp;

	sh4.ea = sh4.gbr + sh4.r[0];
	temp = i & RB( sh4.ea );
	WB( sh4.ea, temp );
	sh4_icount -= 2;
}

/*  code                 cycles  t-bit
 *  1000 1011 dddd dddd  3/1     -
 *  BF      disp8
 */
INLINE void BF(UINT32 d)
{
	if ((sh4.sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh4.pc = sh4.ea = sh4.pc + disp * 2 + 2;
		change_pc(sh4.pc & AM);
		sh4_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1111 dddd dddd  3/1     -
 *  BFS     disp8
 */
INLINE void BFS(UINT32 d)
{
	if ((sh4.sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh4.delay = sh4.pc;
		sh4.pc = sh4.ea = sh4.pc + disp * 2 + 2;
		sh4_icount--;
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
		UINT32 next_opcode = RW(sh4.ppc & AM);
		/* BRA  $
         * NOP
         */
		if (next_opcode == 0x0009)
			sh4_icount %= 3;	/* cycles for BRA $ and NOP taken (3) */
	}
#endif
	sh4.delay = sh4.pc;
	sh4.pc = sh4.ea = sh4.pc + disp * 2 + 2;
	sh4_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0010 0011  2       -
 *  BRAF    Rm
 */
INLINE void BRAF(UINT32 m)
{
	sh4.delay = sh4.pc;
	sh4.pc += sh4.r[m] + 2;
	sh4_icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
INLINE void BSR(UINT32 d)
{
	INT32 disp = ((INT32)d << 20) >> 20;

	sh4.pr = sh4.pc + 2;
	sh4.delay = sh4.pc;
	sh4.pc = sh4.ea = sh4.pc + disp * 2 + 2;
	sh4_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0000 0011  2       -
 *  BSRF    Rm
 */
INLINE void BSRF(UINT32 m)
{
	sh4.pr = sh4.pc + 2;
	sh4.delay = sh4.pc;
	sh4.pc += sh4.r[m] + 2;
	sh4_icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
INLINE void BT(UINT32 d)
{
	if ((sh4.sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh4.pc = sh4.ea = sh4.pc + disp * 2 + 2;
		change_pc(sh4.pc & AM);
		sh4_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1101 dddd dddd  2/1     -
 *  BTS     disp8
 */
INLINE void BTS(UINT32 d)
{
	if ((sh4.sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh4.delay = sh4.pc;
		sh4.pc = sh4.ea = sh4.pc + disp * 2 + 2;
		sh4_icount--;
	}
}

/*  code                 cycles  t-bit
 *  0000 0000 0010 1000  1       -
 *  CLRMAC
 */
INLINE void CLRMAC(void)
{
	sh4.mach = 0;
	sh4.macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
INLINE void CLRT(void)
{
	sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
INLINE void CMPEQ(UINT32 m, UINT32 n)
{
	if (sh4.r[n] == sh4.r[m])
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
INLINE void CMPGE(UINT32 m, UINT32 n)
{
	if ((INT32) sh4.r[n] >= (INT32) sh4.r[m])
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
INLINE void CMPGT(UINT32 m, UINT32 n)
{
	if ((INT32) sh4.r[n] > (INT32) sh4.r[m])
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
INLINE void CMPHI(UINT32 m, UINT32 n)
{
	if ((UINT32) sh4.r[n] > (UINT32) sh4.r[m])
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
INLINE void CMPHS(UINT32 m, UINT32 n)
{
	if ((UINT32) sh4.r[n] >= (UINT32) sh4.r[m])
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}


/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
INLINE void CMPPL(UINT32 n)
{
	if ((INT32) sh4.r[n] > 0)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
INLINE void CMPPZ(UINT32 n)
{
	if ((INT32) sh4.r[n] >= 0)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1100  1       comparison result
 * CMP_STR  Rm,Rn
 */
INLINE void CMPSTR(UINT32 m, UINT32 n)
 {
  UINT32 temp;
  INT32 HH, HL, LH, LL;
  temp = sh4.r[n] ^ sh4.r[m];
  HH = (temp >> 24) & 0xff;
  HL = (temp >> 16) & 0xff;
  LH = (temp >> 8) & 0xff;
  LL = temp & 0xff;
  if (HH && HL && LH && LL)
   sh4.sr &= ~T;
  else
   sh4.sr |= T;
 }


/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
INLINE void CMPIM(UINT32 i)
{
	UINT32 imm = (UINT32)(INT32)(INT16)(INT8)i;

	if (sh4.r[0] == imm)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
INLINE void DIV0S(UINT32 m, UINT32 n)
{
	if ((sh4.r[n] & 0x80000000) == 0)
		sh4.sr &= ~Q;
	else
		sh4.sr |= Q;
	if ((sh4.r[m] & 0x80000000) == 0)
		sh4.sr &= ~M;
	else
		sh4.sr |= M;
	if ((sh4.r[m] ^ sh4.r[n]) & 0x80000000)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
INLINE void DIV0U(void)
{
	sh4.sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
INLINE void DIV1(UINT32 m, UINT32 n)
{
	UINT32 tmp0;
	UINT32 old_q;

	old_q = sh4.sr & Q;
	if (0x80000000 & sh4.r[n])
		sh4.sr |= Q;
	else
		sh4.sr &= ~Q;

	sh4.r[n] = (sh4.r[n] << 1) | (sh4.sr & T);

	if (!old_q)
	{
		if (!(sh4.sr & M))
		{
			tmp0 = sh4.r[n];
			sh4.r[n] -= sh4.r[m];
			if(!(sh4.sr & Q))
				if(sh4.r[n] > tmp0)
					sh4.sr |= Q;
				else
					sh4.sr &= ~Q;
			else
				if(sh4.r[n] > tmp0)
					sh4.sr &= ~Q;
				else
					sh4.sr |= Q;
		}
		else
		{
			tmp0 = sh4.r[n];
			sh4.r[n] += sh4.r[m];
			if(!(sh4.sr & Q))
			{
				if(sh4.r[n] < tmp0)
					sh4.sr &= ~Q;
				else
					sh4.sr |= Q;
			}
			else
			{
				if(sh4.r[n] < tmp0)
					sh4.sr |= Q;
				else
					sh4.sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(sh4.sr & M))
		{
			tmp0 = sh4.r[n];
			sh4.r[n] += sh4.r[m];
			if(!(sh4.sr & Q))
				if(sh4.r[n] < tmp0)
					sh4.sr |= Q;
				else
					sh4.sr &= ~Q;
			else
				if(sh4.r[n] < tmp0)
					sh4.sr &= ~Q;
				else
					sh4.sr |= Q;
		}
		else
		{
			tmp0 = sh4.r[n];
			sh4.r[n] -= sh4.r[m];
			if(!(sh4.sr & Q))
				if(sh4.r[n] > tmp0)
					sh4.sr &= ~Q;
				else
					sh4.sr |= Q;
			else
				if(sh4.r[n] > tmp0)
					sh4.sr |= Q;
				else
					sh4.sr &= ~Q;
		}
	}

	tmp0 = (sh4.sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  DMULS.L Rm,Rn */
INLINE void DMULS(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) sh4.r[n];
	tempm = (INT32) sh4.r[m];
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	if ((INT32) (sh4.r[n] ^ sh4.r[m]) < 0)
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
	sh4.mach = Res2;
	sh4.macl = Res0;
	sh4_icount--;
}

/*  DMULU.L Rm,Rn */
INLINE void DMULU(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;

	RnL = sh4.r[n] & 0x0000ffff;
	RnH = (sh4.r[n] >> 16) & 0x0000ffff;
	RmL = sh4.r[m] & 0x0000ffff;
	RmH = (sh4.r[m] >> 16) & 0x0000ffff;
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
	sh4.mach = Res2;
	sh4.macl = Res0;
	sh4_icount--;
}

/*  DT      Rn */
INLINE void DT(UINT32 n)
{
	sh4.r[n]--;
	if (sh4.r[n] == 0)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
#if BUSY_LOOP_HACKS
	{
		UINT32 next_opcode = RW(sh4.ppc & AM);
		/* DT   Rn
         * BF   $-2
         */
		if (next_opcode == 0x8bfd)
		{
			while (sh4.r[n] > 1 && sh4_icount > 4)
			{
				sh4.r[n]--;
				sh4_icount -= 4;	/* cycles for DT (1) and BF taken (3) */
			}
		}
	}
#endif
}

/*  EXTS.B  Rm,Rn */
INLINE void EXTSB(UINT32 m, UINT32 n)
{
	sh4.r[n] = ((INT32)sh4.r[m] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
INLINE void EXTSW(UINT32 m, UINT32 n)
{
	sh4.r[n] = ((INT32)sh4.r[m] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
INLINE void EXTUB(UINT32 m, UINT32 n)
{
	sh4.r[n] = sh4.r[m] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
INLINE void EXTUW(UINT32 m, UINT32 n)
{
	sh4.r[n] = sh4.r[m] & 0x0000ffff;
}

/*  JMP     @Rm */
INLINE void JMP(UINT32 m)
{
	sh4.delay = sh4.pc;
	sh4.pc = sh4.ea = sh4.r[m];
}

/*  JSR     @Rm */
INLINE void JSR(UINT32 m)
{
	sh4.delay = sh4.pc;
	sh4.pr = sh4.pc + 2;
	sh4.pc = sh4.ea = sh4.r[m];
	sh4_icount--;
}


/*  LDC     Rm,SR */
INLINE void LDCSR(UINT32 m)
{
#ifdef MAME_DEBUG
	sh4_syncronize_register_bank((sh4.sr & sRB) >> 29);
#endif
	if ((sh4.r[m] & sRB) != (sh4.sr & sRB))
		sh4_change_register_bank(sh4.r[m] & sRB ? 1 : 0);
	sh4.sr = sh4.r[m] & FLAGS;
	sh4_exception_recompute();
}

/*  LDC     Rm,GBR */
INLINE void LDCGBR(UINT32 m)
{
	sh4.gbr = sh4.r[m];
}

/*  LDC     Rm,VBR */
INLINE void LDCVBR(UINT32 m)
{
	sh4.vbr = sh4.r[m];
}

/*  LDC.L   @Rm+,SR */
INLINE void LDCMSR(UINT32 m)
{
UINT32 old;

	old = sh4.sr;
	sh4.ea = sh4.r[m];
	sh4.sr = RL( sh4.ea ) & FLAGS;
#ifdef MAME_DEBUG
	sh4_syncronize_register_bank((old & sRB) >> 29);
#endif
	if ((old & sRB) != (sh4.sr & sRB))
		sh4_change_register_bank(sh4.sr & sRB ? 1 : 0);
	sh4.r[m] += 4;
	sh4_icount -= 2;
	sh4_exception_recompute();
}

/*  LDC.L   @Rm+,GBR */
INLINE void LDCMGBR(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.gbr = RL( sh4.ea );
	sh4.r[m] += 4;
	sh4_icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
INLINE void LDCMVBR(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.vbr = RL( sh4.ea );
	sh4.r[m] += 4;
	sh4_icount -= 2;
}

/*  LDS     Rm,MACH */
INLINE void LDSMACH(UINT32 m)
{
	sh4.mach = sh4.r[m];
}

/*  LDS     Rm,MACL */
INLINE void LDSMACL(UINT32 m)
{
	sh4.macl = sh4.r[m];
}

/*  LDS     Rm,PR */
INLINE void LDSPR(UINT32 m)
{
	sh4.pr = sh4.r[m];
}

/*  LDS.L   @Rm+,MACH */
INLINE void LDSMMACH(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.mach = RL( sh4.ea );
	sh4.r[m] += 4;
}

/*  LDS.L   @Rm+,MACL */
INLINE void LDSMMACL(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.macl = RL( sh4.ea );
	sh4.r[m] += 4;
}

/*  LDS.L   @Rm+,PR */
INLINE void LDSMPR(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.pr = RL( sh4.ea );
	sh4.r[m] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
INLINE void MAC_L(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) RL( sh4.r[n] );
	sh4.r[n] += 4;
	tempm = (INT32) RL( sh4.r[m] );
	sh4.r[m] += 4;
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
	if (sh4.sr & S)
	{
		Res0 = sh4.macl + Res0;
		if (sh4.macl > Res0)
			Res2++;
		Res2 += (sh4.mach & 0x0000ffff);
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
		sh4.mach = Res2;
		sh4.macl = Res0;
	}
	else
	{
		Res0 = sh4.macl + Res0;
		if (sh4.macl > Res0)
			Res2++;
		Res2 += sh4.mach;
		sh4.mach = Res2;
		sh4.macl = Res0;
	}
	sh4_icount -= 2;
}

/*  MAC.W   @Rm+,@Rn+ */
INLINE void MAC_W(UINT32 m, UINT32 n)
{
	INT32 tempm, tempn, dest, src, ans;
	UINT32 templ;

	tempn = (INT32) RW( sh4.r[n] );
	sh4.r[n] += 2;
	tempm = (INT32) RW( sh4.r[m] );
	sh4.r[m] += 2;
	templ = sh4.macl;
	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);
	if ((INT32) sh4.macl >= 0)
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
	sh4.macl += tempm;
	if ((INT32) sh4.macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (sh4.sr & S)
	{
		if (ans == 1)
			{
				if (src == 0)
					sh4.macl = 0x7fffffff;
				if (src == 2)
					sh4.macl = 0x80000000;
			}
	}
	else
	{
		sh4.mach += tempn;
		if (templ > sh4.macl)
			sh4.mach += 1;
		}
	sh4_icount -= 2;
}

/*  MOV     Rm,Rn */
INLINE void MOV(UINT32 m, UINT32 n)
{
	sh4.r[n] = sh4.r[m];
}

/*  MOV.B   Rm,@Rn */
INLINE void MOVBS(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[n];
	WB( sh4.ea, sh4.r[m] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
INLINE void MOVWS(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[n];
	WW( sh4.ea, sh4.r[m] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
INLINE void MOVLS(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.r[m] );
}

/*  MOV.B   @Rm,Rn */
INLINE void MOVBL(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[m];
	sh4.r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh4.ea );
}

/*  MOV.W   @Rm,Rn */
INLINE void MOVWL(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[m];
	sh4.r[n] = (UINT32)(INT32)(INT16) RW( sh4.ea );
}

/*  MOV.L   @Rm,Rn */
INLINE void MOVLL(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[m];
	sh4.r[n] = RL( sh4.ea );
}

/*  MOV.B   Rm,@-Rn */
INLINE void MOVBM(UINT32 m, UINT32 n)
{
	/* SMG : bug fix, was reading sh4.r[n] */
	UINT32 data = sh4.r[m] & 0x000000ff;

	sh4.r[n] -= 1;
	WB( sh4.r[n], data );
}

/*  MOV.W   Rm,@-Rn */
INLINE void MOVWM(UINT32 m, UINT32 n)
{
	UINT32 data = sh4.r[m] & 0x0000ffff;

	sh4.r[n] -= 2;
	WW( sh4.r[n], data );
}

/*  MOV.L   Rm,@-Rn */
INLINE void MOVLM(UINT32 m, UINT32 n)
{
	UINT32 data = sh4.r[m];

	sh4.r[n] -= 4;
	WL( sh4.r[n], data );
}

/*  MOV.B   @Rm+,Rn */
INLINE void MOVBP(UINT32 m, UINT32 n)
{
	sh4.r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh4.r[m] );
	if (n != m)
		sh4.r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
INLINE void MOVWP(UINT32 m, UINT32 n)
{
	sh4.r[n] = (UINT32)(INT32)(INT16) RW( sh4.r[m] );
	if (n != m)
		sh4.r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
INLINE void MOVLP(UINT32 m, UINT32 n)
{
	sh4.r[n] = RL( sh4.r[m] );
	if (n != m)
		sh4.r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
INLINE void MOVBS0(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[n] + sh4.r[0];
	WB( sh4.ea, sh4.r[m] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
INLINE void MOVWS0(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[n] + sh4.r[0];
	WW( sh4.ea, sh4.r[m] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
INLINE void MOVLS0(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[n] + sh4.r[0];
	WL( sh4.ea, sh4.r[m] );
}

/*  MOV.B   @(R0,Rm),Rn */
INLINE void MOVBL0(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[m] + sh4.r[0];
	sh4.r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh4.ea );
}

/*  MOV.W   @(R0,Rm),Rn */
INLINE void MOVWL0(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[m] + sh4.r[0];
	sh4.r[n] = (UINT32)(INT32)(INT16) RW( sh4.ea );
}

/*  MOV.L   @(R0,Rm),Rn */
INLINE void MOVLL0(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[m] + sh4.r[0];
	sh4.r[n] = RL( sh4.ea );
}

/*  MOV     #imm,Rn */
INLINE void MOVI(UINT32 i, UINT32 n)
{
	sh4.r[n] = (UINT32)(INT32)(INT16)(INT8) i;
}

/*  MOV.W   @(disp8,PC),Rn */
INLINE void MOVWI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	sh4.ea = sh4.pc + disp * 2 + 2;
	sh4.r[n] = (UINT32)(INT32)(INT16) RW( sh4.ea );
}

/*  MOV.L   @(disp8,PC),Rn */
INLINE void MOVLI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	sh4.ea = ((sh4.pc + 2) & ~3) + disp * 4;
	sh4.r[n] = RL( sh4.ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
INLINE void MOVBLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh4.ea = sh4.gbr + disp;
	sh4.r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh4.ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
INLINE void MOVWLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh4.ea = sh4.gbr + disp * 2;
	sh4.r[0] = (INT32)(INT16) RW( sh4.ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
INLINE void MOVLLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh4.ea = sh4.gbr + disp * 4;
	sh4.r[0] = RL( sh4.ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
INLINE void MOVBSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh4.ea = sh4.gbr + disp;
	WB( sh4.ea, sh4.r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
INLINE void MOVWSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh4.ea = sh4.gbr + disp * 2;
	WW( sh4.ea, sh4.r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
INLINE void MOVLSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh4.ea = sh4.gbr + disp * 4;
	WL( sh4.ea, sh4.r[0] );
}

/*  MOV.B   R0,@(disp4,Rn) */
INLINE void MOVBS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh4.ea = sh4.r[n] + disp;
	WB( sh4.ea, sh4.r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rn) */
INLINE void MOVWS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh4.ea = sh4.r[n] + disp * 2;
	WW( sh4.ea, sh4.r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
INLINE void MOVLS4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh4.ea = sh4.r[n] + disp * 4;
	WL( sh4.ea, sh4.r[m] );
}

/*  MOV.B   @(disp4,Rm),R0 */
INLINE void MOVBL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	sh4.ea = sh4.r[m] + disp;
	sh4.r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh4.ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
INLINE void MOVWL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	sh4.ea = sh4.r[m] + disp * 2;
	sh4.r[0] = (UINT32)(INT32)(INT16) RW( sh4.ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
INLINE void MOVLL4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh4.ea = sh4.r[m] + disp * 4;
	sh4.r[n] = RL( sh4.ea );
}

/*  MOVA    @(disp8,PC),R0 */
INLINE void MOVA(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh4.ea = ((sh4.pc + 2) & ~3) + disp * 4;
	sh4.r[0] = sh4.ea;
}

/*  MOVT    Rn */
INLINE void MOVT(UINT32 n)
{
	sh4.r[n] = sh4.sr & T;
}

/*  MUL.L   Rm,Rn */
INLINE void MULL(UINT32 m, UINT32 n)
{
	sh4.macl = sh4.r[n] * sh4.r[m];
	sh4_icount--;
}

/*  MULS    Rm,Rn */
INLINE void MULS(UINT32 m, UINT32 n)
{
	sh4.macl = (INT16) sh4.r[n] * (INT16) sh4.r[m];
}

/*  MULU    Rm,Rn */
INLINE void MULU(UINT32 m, UINT32 n)
{
	sh4.macl = (UINT16) sh4.r[n] * (UINT16) sh4.r[m];
}

/*  NEG     Rm,Rn */
INLINE void NEG(UINT32 m, UINT32 n)
{
	sh4.r[n] = 0 - sh4.r[m];
}

/*  NEGC    Rm,Rn */
INLINE void NEGC(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = sh4.r[m];
	sh4.r[n] = -temp - (sh4.sr & T);
	if (temp || (sh4.sr & T))
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  NOP */
INLINE void NOP(void)
{
}

/*  NOT     Rm,Rn */
INLINE void NOT(UINT32 m, UINT32 n)
{
	sh4.r[n] = ~sh4.r[m];
}

/*  OR      Rm,Rn */
INLINE void OR(UINT32 m, UINT32 n)
{
	sh4.r[n] |= sh4.r[m];
}

/*  OR      #imm,R0 */
INLINE void ORI(UINT32 i)
{
	sh4.r[0] |= i;
	sh4_icount -= 2;
}

/*  OR.B    #imm,@(R0,GBR) */
INLINE void ORM(UINT32 i)
{
	UINT32 temp;

	sh4.ea = sh4.gbr + sh4.r[0];
	temp = RB( sh4.ea );
	temp |= i;
	WB( sh4.ea, temp );
}

/*  ROTCL   Rn */
INLINE void ROTCL(UINT32 n)
{
	UINT32 temp;

	temp = (sh4.r[n] >> 31) & T;
	sh4.r[n] = (sh4.r[n] << 1) | (sh4.sr & T);
	sh4.sr = (sh4.sr & ~T) | temp;
}

/*  ROTCR   Rn */
INLINE void ROTCR(UINT32 n)
{
	UINT32 temp;
	temp = (sh4.sr & T) << 31;
	if (sh4.r[n] & T)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
	sh4.r[n] = (sh4.r[n] >> 1) | temp;
}

/*  ROTL    Rn */
INLINE void ROTL(UINT32 n)
{
	sh4.sr = (sh4.sr & ~T) | ((sh4.r[n] >> 31) & T);
	sh4.r[n] = (sh4.r[n] << 1) | (sh4.r[n] >> 31);
}

/*  ROTR    Rn */
INLINE void ROTR(UINT32 n)
{
	sh4.sr = (sh4.sr & ~T) | (sh4.r[n] & T);
	sh4.r[n] = (sh4.r[n] >> 1) | (sh4.r[n] << 31);
}

/*  RTE */
INLINE void RTE(void)
{
	sh4.delay = sh4.pc;
	sh4.pc = sh4.ea = sh4.spc;
#ifdef MAME_DEBUG
	sh4_syncronize_register_bank((sh4.sr & sRB) >> 29);
#endif
	if ((sh4.ssr & sRB) != (sh4.sr & sRB))
		sh4_change_register_bank(sh4.ssr & sRB ? 1 : 0);
	sh4.sr = sh4.ssr;
	sh4_icount--;
	sh4_exception_recompute();
}

/*  RTS */
INLINE void RTS(void)
{
	sh4.delay = sh4.pc;
	sh4.pc = sh4.ea = sh4.pr;
	sh4_icount--;
}

/*  SETT */
INLINE void SETT(void)
{
	sh4.sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
INLINE void SHAL(UINT32 n)
{
	sh4.sr = (sh4.sr & ~T) | ((sh4.r[n] >> 31) & T);
	sh4.r[n] <<= 1;
}

/*  SHAR    Rn */
INLINE void SHAR(UINT32 n)
{
	sh4.sr = (sh4.sr & ~T) | (sh4.r[n] & T);
	sh4.r[n] = (UINT32)((INT32)sh4.r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
INLINE void SHLL(UINT32 n)
{
	sh4.sr = (sh4.sr & ~T) | ((sh4.r[n] >> 31) & T);
	sh4.r[n] <<= 1;
}

/*  SHLL2   Rn */
INLINE void SHLL2(UINT32 n)
{
	sh4.r[n] <<= 2;
}

/*  SHLL8   Rn */
INLINE void SHLL8(UINT32 n)
{
	sh4.r[n] <<= 8;
}

/*  SHLL16  Rn */
INLINE void SHLL16(UINT32 n)
{
	sh4.r[n] <<= 16;
}

/*  SHLR    Rn */
INLINE void SHLR(UINT32 n)
{
	sh4.sr = (sh4.sr & ~T) | (sh4.r[n] & T);
	sh4.r[n] >>= 1;
}

/*  SHLR2   Rn */
INLINE void SHLR2(UINT32 n)
{
	sh4.r[n] >>= 2;
}

/*  SHLR8   Rn */
INLINE void SHLR8(UINT32 n)
{
	sh4.r[n] >>= 8;
}

/*  SHLR16  Rn */
INLINE void SHLR16(UINT32 n)
{
	sh4.r[n] >>= 16;
}

/*  SLEEP */
INLINE void SLEEP(void)
{
	sh4.pc -= 2;
	sh4_icount -= 2;
	/* Wait_for_exception; */
}

/*  STC     SR,Rn */
INLINE void STCSR(UINT32 n)
{
	sh4.r[n] = sh4.sr;
}

/*  STC     GBR,Rn */
INLINE void STCGBR(UINT32 n)
{
	sh4.r[n] = sh4.gbr;
}

/*  STC     VBR,Rn */
INLINE void STCVBR(UINT32 n)
{
	sh4.r[n] = sh4.vbr;
}

/*  STC.L   SR,@-Rn */
INLINE void STCMSR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.sr );
	sh4_icount--;
}

/*  STC.L   GBR,@-Rn */
INLINE void STCMGBR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.gbr );
	sh4_icount--;
}

/*  STC.L   VBR,@-Rn */
INLINE void STCMVBR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.vbr );
	sh4_icount--;
}

/*  STS     MACH,Rn */
INLINE void STSMACH(UINT32 n)
{
	sh4.r[n] = sh4.mach;
}

/*  STS     MACL,Rn */
INLINE void STSMACL(UINT32 n)
{
	sh4.r[n] = sh4.macl;
}

/*  STS     PR,Rn */
INLINE void STSPR(UINT32 n)
{
	sh4.r[n] = sh4.pr;
}

/*  STS.L   MACH,@-Rn */
INLINE void STSMMACH(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.mach );
}

/*  STS.L   MACL,@-Rn */
INLINE void STSMMACL(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.macl );
}

/*  STS.L   PR,@-Rn */
INLINE void STSMPR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.pr );
}

/*  SUB     Rm,Rn */
INLINE void SUB(UINT32 m, UINT32 n)
{
	sh4.r[n] -= sh4.r[m];
}

/*  SUBC    Rm,Rn */
INLINE void SUBC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = sh4.r[n] - sh4.r[m];
	tmp0 = sh4.r[n];
	sh4.r[n] = tmp1 - (sh4.sr & T);
	if (tmp0 < tmp1)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
	if (tmp1 < sh4.r[n])
		sh4.sr |= T;
}

/*  SUBV    Rm,Rn */
INLINE void SUBV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) sh4.r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh4.r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh4.r[n] -= sh4.r[m];
	if ((INT32) sh4.r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			sh4.sr |= T;
		else
			sh4.sr &= ~T;
	}
	else
		sh4.sr &= ~T;
}

/*  SWAP.B  Rm,Rn */
INLINE void SWAPB(UINT32 m, UINT32 n)
{
	UINT32 temp0, temp1;

	temp0 = sh4.r[m] & 0xffff0000;
	temp1 = (sh4.r[m] & 0x000000ff) << 8;
	sh4.r[n] = (sh4.r[m] >> 8) & 0x000000ff;
	sh4.r[n] = sh4.r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
INLINE void SWAPW(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (sh4.r[m] >> 16) & 0x0000ffff;
	sh4.r[n] = (sh4.r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
INLINE void TAS(UINT32 n)
{
	UINT32 temp;
	sh4.ea = sh4.r[n];
	/* Bus Lock enable */
	temp = RB( sh4.ea );
	if (temp == 0)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB( sh4.ea, temp );
	sh4_icount -= 3;
}

/*  TRAPA   #imm */
INLINE void TRAPA(UINT32 i)
{
	UINT32 imm = i & 0xff;

	sh4.ea = sh4.vbr + imm * 4;

	sh4.r[15] -= 4;
	WL( sh4.r[15], sh4.sr );
	sh4.r[15] -= 4;
	WL( sh4.r[15], sh4.pc );

	sh4.pc = RL( sh4.ea );
	change_pc(sh4.pc & AM);

	sh4_icount -= 7;
}

/*  TST     Rm,Rn */
INLINE void TST(UINT32 m, UINT32 n)
{
	if ((sh4.r[n] & sh4.r[m]) == 0)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  TST     #imm,R0 */
INLINE void TSTI(UINT32 i)
{
	UINT32 imm = i & 0xff;

	if ((imm & sh4.r[0]) == 0)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
INLINE void TSTM(UINT32 i)
{
	UINT32 imm = i & 0xff;

	sh4.ea = sh4.gbr + sh4.r[0];
	if ((imm & RB( sh4.ea )) == 0)
		sh4.sr |= T;
	else
		sh4.sr &= ~T;
	sh4_icount -= 2;
}

/*  XOR     Rm,Rn */
INLINE void XOR(UINT32 m, UINT32 n)
{
	sh4.r[n] ^= sh4.r[m];
}

/*  XOR     #imm,R0 */
INLINE void XORI(UINT32 i)
{
	UINT32 imm = i & 0xff;
	sh4.r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
INLINE void XORM(UINT32 i)
{
	UINT32 imm = i & 0xff;
	UINT32 temp;

	sh4.ea = sh4.gbr + sh4.r[0];
	temp = RB( sh4.ea );
	temp ^= imm;
	WB( sh4.ea, temp );
	sh4_icount -= 2;
}

/*  XTRCT   Rm,Rn */
INLINE void XTRCT(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (sh4.r[m] << 16) & 0xffff0000;
	sh4.r[n] = (sh4.r[n] >> 16) & 0x0000ffff;
	sh4.r[n] |= temp;
}

/*  STC     SSR,Rn */
INLINE void STCSSR(UINT32 n)
{
	sh4.r[n] = sh4.ssr;
}

/*  STC     SPC,Rn */
INLINE void STCSPC(UINT32 n)
{
	sh4.r[n] = sh4.spc;
}

/*  STC     SGR,Rn */
INLINE void STCSGR(UINT32 n)
{
	sh4.r[n] = sh4.sgr;
}

/*  STS     FPUL,Rn */
INLINE void STSFPUL(UINT32 n)
{
	sh4.r[n] = sh4.fpul;
}

/*  STS     FPSCR,Rn */
INLINE void STSFPSCR(UINT32 n)
{
	sh4.r[n] = sh4.fpscr & 0x003FFFFF;
}

/*  STC     DBR,Rn */
INLINE void STCDBR(UINT32 n)
{
	sh4.r[n] = sh4.dbr;
}

/*  STCRBANK   Rm_BANK,Rn */
INLINE void STCRBANK(UINT32 m, UINT32 n)
{
	sh4.r[n] = sh4.rbnk[sh4.sr&sRB ? 0 : 1][m & 7];
}

/*  STCMRBANK   Rm_BANK,@-Rn */
INLINE void STCMRBANK(UINT32 m, UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.rbnk[sh4.sr&sRB ? 0 : 1][m & 7]);
	sh4_icount--;
}

/*  MOVCA.L     R0,@Rn */
INLINE void MOVCAL(UINT32 n)
{
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.r[0] );
}

INLINE void CLRS(void)
{
	sh4.sr &= ~S;
}

INLINE void SETS(void)
{
	sh4.sr |= S;
}

/*  STS.L   SGR,@-Rn */
INLINE void STCMSGR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.sgr );
}

/*  STS.L   FPUL,@-Rn */
INLINE void STSMFPUL(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.fpul );
}

/*  STS.L   FPSCR,@-Rn */
INLINE void STSMFPSCR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.fpscr & 0x003FFFFF);
}

/*  STC.L   DBR,@-Rn */
INLINE void STCMDBR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.dbr );
}

/*  STC.L   SSR,@-Rn */
INLINE void STCMSSR(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.ssr );
}

/*  STC.L   SPC,@-Rn */
INLINE void STCMSPC(UINT32 n)
{
	sh4.r[n] -= 4;
	sh4.ea = sh4.r[n];
	WL( sh4.ea, sh4.spc );
}

/*  LDS.L   @Rm+,FPUL */
INLINE void LDSMFPUL(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.fpul = RL( sh4.ea );
	sh4.r[m] += 4;
}

/*  LDS.L   @Rm+,FPSCR */
INLINE void LDSMFPSCR(UINT32 m)
{
UINT32 s;

	s = sh4.fpscr;
	sh4.ea = sh4.r[m];
	sh4.fpscr = RL( sh4.ea );
	sh4.fpscr &= 0x003FFFFF;
	sh4.r[m] += 4;
	if ((s & FR) != (sh4.fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (sh4.fpscr & PR))
		sh4_swap_fp_couples();
#endif
	sh4.fpu_sz = (sh4.fpscr & SZ) ? 1 : 0;
	sh4.fpu_pr = (sh4.fpscr & PR) ? 1 : 0;
}

/*  LDC.L   @Rm+,DBR */
INLINE void LDCMDBR(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.dbr = RL( sh4.ea );
	sh4.r[m] += 4;
}

/*  LDC.L   @Rn+,Rm_BANK */
INLINE void LDCMRBANK(UINT32 m, UINT32 n)
{
	sh4.ea = sh4.r[n];
	sh4.rbnk[sh4.sr&sRB ? 0 : 1][m & 7] = RL( sh4.ea );
	sh4.r[n] += 4;
}

/*  LDC.L   @Rm+,SSR */
INLINE void LDCMSSR(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.ssr = RL( sh4.ea );
	sh4.r[m] += 4;
}

/*  LDC.L   @Rm+,SPC */
INLINE void LDCMSPC(UINT32 m)
{
	sh4.ea = sh4.r[m];
	sh4.spc = RL( sh4.ea );
	sh4.r[m] += 4;
}

/*  LDS     Rm,FPUL */
INLINE void LDSFPUL(UINT32 m)
{
	sh4.fpul = sh4.r[m];
}

/*  LDS     Rm,FPSCR */
INLINE void LDSFPSCR(UINT32 m)
{
UINT32 s;

	s = sh4.fpscr;
	sh4.fpscr = sh4.r[m] & 0x003FFFFF;
	if ((s & FR) != (sh4.fpscr & FR))
		sh4_swap_fp_registers();
#ifdef LSB_FIRST
	if ((s & PR) != (sh4.fpscr & PR))
		sh4_swap_fp_couples();
#endif
	sh4.fpu_sz = (sh4.fpscr & SZ) ? 1 : 0;
	sh4.fpu_pr = (sh4.fpscr & PR) ? 1 : 0;
}

/*  LDC     Rm,DBR */
INLINE void LDCDBR(UINT32 m)
{
	sh4.dbr = sh4.r[m];
}

/*  SHAD    Rm,Rn */
INLINE void SHAD(UINT32 m,UINT32 n)
{
	if ((sh4.r[m] & 0x80000000) == 0)
		sh4.r[n] = sh4.r[n] << (sh4.r[m] & 0x1F);
	else if ((sh4.r[m] & 0x1F) == 0) {
		if ((sh4.r[n] & 0x80000000) == 0)
			sh4.r[n] = 0;
		else
			sh4.r[n] = 0xFFFFFFFF;
	} else
		sh4.r[n]=(INT32)sh4.r[n] >> ((~sh4.r[m] & 0x1F)+1);
}

/*  SHLD    Rm,Rn */
INLINE void SHLD(UINT32 m,UINT32 n)
{
	if ((sh4.r[m] & 0x80000000) == 0)
		sh4.r[n] = sh4.r[n] << (sh4.r[m] & 0x1F);
	else if ((sh4.r[m] & 0x1F) == 0)
		sh4.r[n] = 0;
	else
		sh4.r[n] = sh4.r[n] >> ((~sh4.r[m] & 0x1F)+1);
}

/*  LDCRBANK   Rn,Rm_BANK */
INLINE void LDCRBANK(UINT32 m, UINT32 n)
{
	sh4.rbnk[sh4.sr&sRB ? 0 : 1][m & 7] = sh4.r[n];
}

/*  LDC     Rm,SSR */
INLINE void LDCSSR(UINT32 m)
{
	sh4.ssr = sh4.r[m];
}

/*  LDC     Rm,SPC */
INLINE void LDCSPC(UINT32 m)
{
	sh4.spc = sh4.r[m];
}

/*  PREF     @Rn */
INLINE void PREFM(UINT32 n)
{
int a;
UINT32 addr,dest,sq;

	addr = sh4.r[n]; // address
	if ((addr >= 0xE0000000) && (addr <= 0xE3FFFFFF))
	{
		sq = (addr & 0x20) >> 5;
		dest = addr & 0x03FFFFE0;
		if (sq == 0)
			dest |= (sh4.m[QACR0] & 0x1C) << 24;
		else
			dest |= (sh4.m[QACR1] & 0x1C) << 24;
		addr = addr & 0xFFFFFFE0;
		for (a = 0;a < 4;a++)
		{
			program_write_qword_64le(dest, program_read_qword_64le(addr));
			addr += 8;
			dest += 8;
		}
	}
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/
INLINE void op0000(UINT16 opcode)
{
	switch (opcode & 0xF)
	{
	case 0x0:
	case 0x1:
		break;
	case 0x2:
		if (opcode & 0x80) {
			STCRBANK(Rm, Rn); return;
		}
		switch (opcode & 0x70)
		{
		case 0x00:
			STCSR(Rn); break;
		case 0x10:
			STCGBR(Rn); break;
		case 0x20:
			STCVBR(Rn); break;
		case 0x30:
			STCSSR(Rn); break;
		case 0x40:
			STCSPC(Rn); break;
		}
		break;
	case 0x3:
		switch (opcode & 0xF0)
		{
		case 0x00:
			BSRF(Rn); break;
		case 0x20:
			BRAF(Rn); break;
		case 0x80:
			PREFM(Rn); break;
		case 0x90:
			TODO(); break;
		case 0xA0:
			TODO(); break;
		case 0xB0:
			TODO(); break;
		case 0xC0:
			MOVCAL(Rn); break;
		}
		break;
	case 0x4:
		MOVBS0(Rm, Rn); break;
	case 0x5:
		MOVWS0(Rm, Rn); break;
	case 0x6:
		MOVLS0(Rm, Rn); break;
	case 0x7:
		MULL(Rm, Rn); break;
	case 0x8:
		switch (opcode & 0x70)
		{
		case 0x00:
			CLRT(); break;
		case 0x10:
			SETT(); break;
		case 0x20:
			CLRMAC(); break;
		case 0x30:
			TODO(); break;
		case 0x40:
			CLRS(); break;
		case 0x50:
			SETS(); break;
		}
		break;
	case 0x9:
		switch (opcode & 0x30)
		{
		case 0x00:
			NOP(); break;
		case 0x10:
			DIV0U(); break;
		case 0x20:
			MOVT(Rn); break;
		}
		break;
	case 0xA:
		switch (opcode & 0x70)
		{
		case 0x00:
			STSMACH(Rn); break;
		case 0x10:
			STSMACL(Rn); break;
		case 0x20:
			STSPR(Rn); break;
		case 0x30:
			STCSGR(Rn); break;
		case 0x50:
			STSFPUL(Rn); break;
		case 0x60:
			STSFPSCR(Rn); break;
		case 0x70:
			STCDBR(Rn); break;
		}
		break;
	case 0xB:
		switch (opcode & 0x30)
		{
		case 0x00:
			RTS(); break;
		case 0x10:
			SLEEP(); break;
		case 0x20:
			RTE(); break;
		}
		break;
	case 0xC:
		MOVBL0(Rm, Rn); break;
	case 0xD:
		MOVWL0(Rm, Rn); break;
	case 0xE:
		MOVLL0(Rm, Rn); break;
	case 0xF:
		MAC_L(Rm, Rn); break;
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
	switch (opcode & 0xF)
	{
	case 0x0:
		switch (opcode & 0x30)
		{
		case 0x00:
			SHLL(Rn); break;
		case 0x10:
			DT(Rn); break;
		case 0x20:
			SHAL(Rn); break;
		}
		break;
	case 0x1:
		switch (opcode & 0x30)
		{
		case 0x00:
			SHLR(Rn); break;
		case 0x10:
			CMPPZ(Rn); break;
		case 0x20:
			SHAR(Rn); break;
		}
		break;
	case 0x2:
		switch (opcode & 0xF0)
		{
		case 0x00:
			STSMMACH(Rn); break;
		case 0x10:
			STSMMACL(Rn); break;
		case 0x20:
			STSMPR(Rn); break;
		case 0x30:
			STCMSGR(Rn); break;
		case 0x50:
			STSMFPUL(Rn); break;
		case 0x60:
			STSMFPSCR(Rn); break;
		case 0xF0:
			STCMDBR(Rn); break;
		}
		break;
	case 0x3:
		if (opcode & 0x80) {
			STCMRBANK(Rm, Rn); return;
		}
		switch (opcode & 0x70)
		{
		case 0x00:
			STCMSR(Rn); break;
		case 0x10:
			STCMGBR(Rn); break;
		case 0x20:
			STCMVBR(Rn); break;
		case 0x30:
			STCMSSR(Rn); break;
		case 0x40:
			STCMSPC(Rn); break;
		}
		break;
	case 0x4:
		switch (opcode & 0x30)
		{
		case 0x00:
			ROTL(Rn); break;
		case 0x20:
			ROTCL(Rn); break;
		}
		break;
	case 0x5:
		switch (opcode & 0x30)
		{
		case 0x00:
			ROTR(Rn); break;
		case 0x10:
			CMPPL(Rn); break;
		case 0x20:
			ROTCR(Rn); break;
		}
		break;
	case 0x6:
		switch (opcode & 0xF0)
		{
		case 0x00:
			LDSMMACH(Rn); break;
		case 0x10:
			LDSMMACL(Rn); break;
		case 0x20:
			LDSMPR(Rn); break;
		case 0x50:
			LDSMFPUL(Rn); break;
		case 0x60:
			LDSMFPSCR(Rn); break;
		case 0xF0:
			LDCMDBR(Rn); break;
		}
		break;
	case 0x7:
		if (opcode & 0x80) {
			LDCMRBANK(Rm,Rn); return;
		}
		switch (opcode & 0x70)
		{
		case 0x00:
			LDCMSR(Rn); break;
		case 0x10:
			LDCMGBR(Rn); break;
		case 0x20:
			LDCMVBR(Rn); break;
		case 0x30:
			LDCMSSR(Rn); break;
		case 0x40:
			LDCMSPC(Rn); break;
		}
		break;
	case 0x8:
		switch (opcode & 0x30)
		{
		case 0x00:
			SHLL2(Rn); break;
		case 0x10:
			SHLL8(Rn); break;
		case 0x20:
			SHLL16(Rn); break;
		}
		break;
	case 0x9:
		switch (opcode & 0x30)
		{
		case 0x00:
			SHLR2(Rn); break;
		case 0x10:
			SHLR8(Rn); break;
		case 0x20:
			SHLR16(Rn); break;
		}
		break;
	case 0xA:
		switch (opcode & 0xF0)
		{
		case 0x00:
			LDSMACH(Rn); break;
		case 0x10:
			LDSMACL(Rn); break;
		case 0x20:
			LDSPR(Rn); break;
		case 0x50:
			LDSFPUL(Rn); break;
		case 0x60:
			LDSFPSCR(Rn); break;
		case 0xF0:
			LDCDBR(Rn); break;
		}
		break;
	case 0xB:
		switch (opcode & 0x30)
		{
		case 0x00:
			JSR(Rn); break;
		case 0x10:
			TAS(Rn); break;
		case 0x20:
			JMP(Rn); break;
		}
		break;
	case 0xC:
		SHAD(Rm,Rn); break;
	case 0xD:
		SHLD(Rm,Rn); break;
	case 0xE:
		if (opcode & 0x80) {
			LDCRBANK(Rm,Rn); return;
		}
		switch (opcode & 0x70)
		{
		case 0x00:
			LDCSR(Rn); break;
		case 0x10:
			LDCGBR(Rn); break;
		case 0x20:
			LDCVBR(Rn); break;
		case 0x30:
			LDCSSR(Rn); break;
		case 0x40:
			LDCSPC(Rn); break;
		}
		break;
	case 0xF:
		MAC_W(Rm, Rn); break;
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

/*  FMOV.S  @Rm+,FRn PR=0 SZ=0 1111nnnnmmmm1001 */
/*  FMOV    @Rm+,DRn PR=0 SZ=1 1111nnn0mmmm1001 */
/*  FMOV    @Rm+,XDn PR=1      1111nnn1mmmm1001 */
INLINE void FMOVMRIFR(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		sh4.ea = sh4.r[m];
		sh4.r[m] += 8;
#ifdef LSB_FIRST
		sh4.xf[n+1] = RL( sh4.ea );
		sh4.xf[n] = RL( sh4.ea+4 );
#else
		sh4.xf[n] = RL( sh4.ea );
		sh4.xf[n+1] = RL( sh4.ea+4 );
#endif
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			n = n & 14;
			sh4.ea = sh4.r[m];
			sh4.fr[n] = RL( sh4.ea );
			sh4.r[m] += 4;
			sh4.fr[n+1] = RL( sh4.r[m] );
			sh4.r[m] += 4;
		} else {              /* SZ = 0 */
			sh4.ea = sh4.r[m];
			sh4.fr[n] = RL( sh4.ea );
			sh4.r[m] += 4;
		}
	}
}

/*  FMOV.S  FRm,@Rn PR=0 SZ=0 1111nnnnmmmm1010 */
/*  FMOV    DRm,@Rn PR=0 SZ=1 1111nnnnmmm01010 */
/*  FMOV    XDm,@Rn PR=1      1111nnnnmmm11010 */
INLINE void FMOVFRMR(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		m= m & 14;
		sh4.ea = sh4.r[n];
#ifdef LSB_FIRST
		WL( sh4.ea,sh4.xf[m+1] );
		WL( sh4.ea+4,sh4.xf[m] );
#else
		WL( sh4.ea,sh4.xf[m] );
		WL( sh4.ea+4,sh4.xf[m+1] );
#endif
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			m= m & 14;
			sh4.ea = sh4.r[n];
			WL( sh4.ea,sh4.fr[m] );
			WL( sh4.ea+4,sh4.fr[m+1] );
		} else {              /* SZ = 0 */
			sh4.ea = sh4.r[n];
			WL( sh4.ea,sh4.fr[m] );
		}
	}
}

/*  FMOV.S  FRm,@-Rn PR=0 SZ=0 1111nnnnmmmm1011 */
/*  FMOV    DRm,@-Rn PR=0 SZ=1 1111nnnnmmm01011 */
/*  FMOV    XDm,@-Rn PR=1      1111nnnnmmm11011 */
INLINE void FMOVFRMDR(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		m= m & 14;
		sh4.r[n] -= 8;
		sh4.ea = sh4.r[n];
#ifdef LSB_FIRST
		WL( sh4.ea,sh4.xf[m+1] );
		WL( sh4.ea+4,sh4.xf[m] );
#else
		WL( sh4.ea,sh4.xf[m] );
		WL( sh4.ea+4,sh4.xf[m+1] );
#endif
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			m= m & 14;
			sh4.r[n] -= 8;
			sh4.ea = sh4.r[n];
			WL( sh4.ea,sh4.fr[m] );
			WL( sh4.ea+4,sh4.fr[m+1] );
		} else {              /* SZ = 0 */
			sh4.r[n] -= 4;
			sh4.ea = sh4.r[n];
			WL( sh4.ea,sh4.fr[m] );
		}
	}
}

/*  FMOV.S  FRm,@(R0,Rn) PR=0 SZ=0 1111nnnnmmmm0111 */
/*  FMOV    DRm,@(R0,Rn) PR=0 SZ=1 1111nnnnmmm00111 */
/*  FMOV    XDm,@(R0,Rn) PR=1      1111nnnnmmm10111 */
INLINE void FMOVFRS0(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		m= m & 14;
		sh4.ea = sh4.r[0] + sh4.r[n];
#ifdef LSB_FIRST
		WL( sh4.ea,sh4.xf[m+1] );
		WL( sh4.ea+4,sh4.xf[m] );
#else
		WL( sh4.ea,sh4.xf[m] );
		WL( sh4.ea+4,sh4.xf[m+1] );
#endif
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			m= m & 14;
			sh4.ea = sh4.r[0] + sh4.r[n];
			WL( sh4.ea,sh4.fr[m] );
			WL( sh4.ea+4,sh4.fr[m+1] );
		} else {              /* SZ = 0 */
			sh4.ea = sh4.r[0] + sh4.r[n];
			WL( sh4.ea,sh4.fr[m] );
		}
	}
}

/*  FMOV.S  @(R0,Rm),FRn PR=0 SZ=0 1111nnnnmmmm0110 */
/*  FMOV    @(R0,Rm),DRn PR=0 SZ=1 1111nnn0mmmm0110 */
/*  FMOV    @(R0,Rm),XDn PR=1      1111nnn1mmmm0110 */
INLINE void FMOVS0FR(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n= n & 14;
		sh4.ea = sh4.r[0] + sh4.r[m];
#ifdef LSB_FIRST
		sh4.xf[n+1] = RL( sh4.ea );
		sh4.xf[n] = RL( sh4.ea+4 );
#else
		sh4.xf[n] = RL( sh4.ea );
		sh4.xf[n+1] = RL( sh4.ea+4 );
#endif
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			n= n & 14;
			sh4.ea = sh4.r[0] + sh4.r[m];
			sh4.fr[n] = RL( sh4.ea );
			sh4.fr[n+1] = RL( sh4.ea+4 );
		} else {              /* SZ = 0 */
			sh4.ea = sh4.r[0] + sh4.r[m];
			sh4.fr[n] = RL( sh4.ea );
		}
	}
}

/*  FMOV.S  @Rm,FRn PR=0 SZ=0 1111nnnnmmmm1000 */
/*  FMOV    @Rm,DRn PR=0 SZ=1 1111nnn0mmmm1000 */
/*  FMOV    @Rm,XDn PR=0 SZ=1 1111nnn1mmmm1000 */
/*  FMOV    @Rm,XDn PR=1      1111nnn1mmmm1000 */
/*  FMOV    @Rm,DRn PR=1      1111nnn0mmmm1000 */
INLINE void FMOVMRFR(UINT32 m,UINT32 n)
{
#if DATASHEET_STRICT
	if (sh4.fpu_pr) { /* PR = 1 */
		n= n & 14;
		sh4.ea = sh4.r[m];
#ifdef LSB_FIRST
		sh4.xf[n+1] = RL( sh4.ea );
		sh4.xf[n] = RL( sh4.ea+4 );
#else
		sh4.xf[n] = RL( sh4.ea );
		sh4.xf[n+1] = RL( sh4.ea+4 );
#endif
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			n= n & 14;
			sh4.ea = sh4.r[m];
			sh4.fr[n] = RL( sh4.ea );
			sh4.fr[n+1] = RL( sh4.ea+4 );
		} else {              /* SZ = 0 */
			sh4.ea = sh4.r[m];
			sh4.fr[n] = RL( sh4.ea );
		}
	}
#else
	if (sh4.fpu_pr) { /* PR = 1 */
		if (n & 1) {
			n= n & 14;
			sh4.ea = sh4.r[m];
#ifdef LSB_FIRST
			sh4.xf[n+1] = RL( sh4.ea );
			sh4.xf[n] = RL( sh4.ea+4 );
#else
			sh4.xf[n] = RL( sh4.ea );
			sh4.xf[n+1] = RL( sh4.ea+4 );
#endif
		} else {
			n= n & 14;
			sh4.ea = sh4.r[m];
#ifdef LSB_FIRST
			sh4.fr[n+1] = RL( sh4.ea );
			sh4.fr[n] = RL( sh4.ea+4 );
#else
			sh4.fr[n] = RL( sh4.ea );
			sh4.fr[n+1] = RL( sh4.ea+4 );
#endif
		}
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			if (n & 1) {
				n= n & 14;
				sh4.ea = sh4.r[m];
				sh4.xf[n] = RL( sh4.ea );
				sh4.xf[n+1] = RL( sh4.ea+4 );
			} else {
				n= n & 14;
				sh4.ea = sh4.r[m];
				sh4.fr[n] = RL( sh4.ea );
				sh4.fr[n+1] = RL( sh4.ea+4 );
			}
		} else {              /* SZ = 0 */
			sh4.ea = sh4.r[m];
			sh4.fr[n] = RL( sh4.ea );
		}
	}
#endif
}

/*  FMOV    FRm,FRn PR=0 SZ=0 FRm -> FRn 1111nnnnmmmm1100 */
/*  FMOV    DRm,DRn PR=0 SZ=1 DRm -> DRn 1111nnn0mmm01100 */
/*  FMOV    XDm,DRn PR=1      XDm -> DRn 1111nnn0mmm11100 */
/*  FMOV    DRm,XDn PR=1      DRm -> XDn 1111nnn1mmm01100 */
/*  FMOV    XDm,XDn PR=1      XDm -> XDn 1111nnn1mmm11100 */
INLINE void FMOVFR(UINT32 m,UINT32 n)
{
#if DATASHEET_STRICT
	if (sh4.fpu_pr) { /* PR = 1 */
		if (m & 1)
			if (n & 1) {
				sh4.xf[n & 14] = sh4.xf[m & 14];
				sh4.xf[n | 1] = sh4.xf[m | 1];
			} else {
				sh4.fr[n] = sh4.xf[m & 14];
				sh4.fr[n | 1] = sh4.xf[m | 1];
			}
		else {
			sh4.xf[n & 14] = sh4.fr[m];
			sh4.xf[n | 1] = sh4.fr[m|1]; // (a&14)+1 -> a|1
		}
	} else {              /* PR = 0 */
		if (sh4.fpu_sz) { /* SZ = 1 */
			sh4.fr[n] = sh4.fr[m];
			sh4.fr[n+1] = sh4.fr[m+1];
		} else {              /* SZ = 0 */
			sh4.fr[n] = sh4.fr[m];
		}
	}
#else
	if (sh4.fpu_sz == 0) /* SZ = 0 */
		sh4.fr[n] = sh4.fr[m];
	else { /* SZ = 1 or PR = 1 */
		if (m & 1)
			if (n & 1) {
				sh4.xf[n & 14] = sh4.xf[m & 14];
				sh4.xf[n | 1] = sh4.xf[m | 1];
			} else {
				sh4.fr[n] = sh4.xf[m & 14];
				sh4.fr[n | 1] = sh4.xf[m | 1];
			}
		else {
			if (n & 1) {
				sh4.xf[n & 14] = sh4.fr[m];
				sh4.xf[n | 1] = sh4.fr[m | 1]; // (a&14)+1 -> a|1
			} else {
				sh4.fr[n] = sh4.fr[m & 14];
				sh4.fr[n | 1] = sh4.fr[m | 1];
			}
		}
	}
#endif
}

/*  FLDI1  FRn 1111nnnn10011101 */
INLINE void FLDI1(UINT32 n)
{
	sh4.fr[n] = 0x3F800000; 
}

/*  FLDI0  FRn 1111nnnn10001101 */
INLINE void FLDI0(UINT32 n)
{
	sh4.fr[n] = 0; 
}

/*  FLDS FRm,FPUL 1111mmmm00011101 */
INLINE void	FLDS(UINT32 m)
{
	sh4.fpul = sh4.fr[m];
}

/*  FSTS FPUL,FRn 1111nnnn00001101 */
INLINE void	FSTS(UINT32 n)
{
	sh4.fr[n] = sh4.fpul;
}

/* FRCHG 1111101111111101 */
INLINE void FRCHG(void)
{
	sh4.fpscr ^= FR;
	sh4_swap_fp_registers();
}

/* FSCHG 1111001111111101 */
INLINE void FSCHG(void)
{
	sh4.fpscr ^= SZ;
	sh4.fpu_sz = (sh4.fpscr & SZ) ? 1 : 0;
}

/* FTRC FRm,FPUL PR=0 1111mmmm00111101 */
/* FTRC DRm,FPUL PR=1 1111mmm000111101 */
INLINE void FTRC(UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		sh4.fpul = (INT32)FP_RFD(n);
	} else {              /* PR = 0 */
		/* read sh4.fr[n] as float -> truncate -> fpul(32) */
		sh4.fpul = (INT32)FP_RFS(n);
	}
}

/* FLOAT FPUL,FRn PR=0 1111nnnn00101101 */
/* FLOAT FPUL,DRn PR=1 1111nnn000101101 */
INLINE void FLOAT(UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		FP_RFD(n) = (double)(INT32)sh4.fpul;
	} else {              /* PR = 0 */
		FP_RFS(n) = (float)(INT32)sh4.fpul;
	}
}

/* FNEG FRn PR=0 1111nnnn01001101 */
/* FNEG DRn PR=1 1111nnn001001101 */
INLINE void FNEG(UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
#ifdef LSB_FIRST
		n = n | 1; // n & 14 + 1
		sh4.fr[n] = sh4.fr[n] ^ 0x80000000;
#else
		n = n & 14;
		sh4.fr[n] = sh4.fr[n] ^ 0x80000000;
#endif
	} else {              /* PR = 0 */
		sh4.fr[n] = sh4.fr[n] ^ 0x80000000;
	}
}


/* FABS FRn PR=0 1111nnnn01011101 */
/* FABS DRn PR=1 1111nnn001011101 */
INLINE void FABS(UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
#ifdef LSB_FIRST
		n = n | 1; // n & 14 + 1
		sh4.fr[n] = sh4.fr[n] & 0x7fffffff;
#else
		n = n & 14;
		sh4.fr[n] = sh4.fr[n] & 0x7fffffff;
#endif
	} else {              /* PR = 0 */
		sh4.fr[n] = sh4.fr[n] & 0x7fffffff;
	}
}

/* FCMP/EQ FRm,FRn PR=0 1111nnnnmmmm0100 */
/* FCMP/EQ DRm,DRn PR=1 1111nnn0mmm00100 */
INLINE void FCMP_EQ(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) == FP_RFD(m))
			sh4.sr |= T;
		else
			sh4.sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) == FP_RFS(m))
			sh4.sr |= T;
		else
			sh4.sr &= ~T;
	}
}

/* FCMP/GT FRm,FRn PR=0 1111nnnnmmmm0101 */
/* FCMP/GT DRm,DRn PR=1 1111nnn0mmm00101 */
INLINE void FCMP_GT(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(n) > FP_RFD(m))
			sh4.sr |= T;
		else
			sh4.sr &= ~T;
	} else {              /* PR = 0 */
		if (FP_RFS(n) > FP_RFS(m))
			sh4.sr |= T;
		else
			sh4.sr &= ~T;
	}
}

/* FCNVDS DRm,FPUL PR=1 1111mmm010111101 */
INLINE void FCNVDS(UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
#ifdef LSB_FIRST
		if (sh4.fpscr & RM) 
			sh4.fr[n] &= 0xe0000000; /* round toward zero*/
#else
		if (sh4.fpscr & RM) 
			sh4.fr[n | 1] &= 0xe0000000; /* round toward zero*/
#endif
		*((float *)&sh4.fpul) = (float)FP_RFD(n);
	}
}

/* FCNVSD FPUL, DRn PR=1 1111nnn010101101 */
INLINE void FCNVSD(UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		FP_RFD(n) = *((float *)&sh4.fpul);
	}
}

/* FADD FRm,FRn PR=0 1111nnnnmmmm0000 */
/* FADD DRm,DRn PR=1 1111nnn0mmm00000 */
INLINE void FADD(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) + FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) + FP_RFS(m);
	}
}

/* FSUB FRm,FRn PR=0 1111nnnnmmmm0001 */
/* FSUB DRm,DRn PR=1 1111nnn0mmm00001 */
INLINE void FSUB(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) - FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) - FP_RFS(m);
	}
}


/* FMUL FRm,FRn PR=0 1111nnnnmmmm0010 */
/* FMUL DRm,DRn PR=1 1111nnn0mmm00010 */
INLINE void FMUL(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		FP_RFD(n) = FP_RFD(n) * FP_RFD(m);
	} else {              /* PR = 0 */
		FP_RFS(n) = FP_RFS(n) * FP_RFS(m);
	}
}

/* FDIV FRm,FRn PR=0 1111nnnnmmmm0011 */
/* FDIV DRm,DRn PR=1 1111nnn0mmm00011 */
INLINE void FDIV(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		m = m & 14;
		if (FP_RFD(m) == 0)
			return;
		FP_RFD(n) = FP_RFD(n) / FP_RFD(m);
	} else {              /* PR = 0 */
		if (FP_RFS(m) == 0)
			return;
		FP_RFS(n) = FP_RFS(n) / FP_RFS(m);
	}
}

/* FMAC FR0,FRm,FRn PR=0 1111nnnnmmmm1110 */
INLINE void FMAC(UINT32 m,UINT32 n)
{
	if (sh4.fpu_pr == 0) { /* PR = 0 */
		FP_RFS(n) = (FP_RFS(0) * FP_RFS(m)) + FP_RFS(n);
	}
}

/* FSQRT FRn PR=0 1111nnnn01101101 */
/* FSQRT DRn PR=1 1111nnnn01101101 */
INLINE void FSQRT(UINT32 n)
{
	if (sh4.fpu_pr) { /* PR = 1 */
		n = n & 14;
		if (FP_RFD(n) < 0)
			return;
		FP_RFD(n) = sqrt(FP_RFD(n));
	} else {              /* PR = 0 */
		if (FP_RFS(n) < 0)
			return;
		FP_RFS(n) = sqrt(FP_RFS(n));
	}
}

/* FIPR FVm,FVn PR=0 1111nnmm11101101 */
INLINE void FIPR(UINT32 n)
{
UINT32 m;
float ml[4];
int a;

	m = (n & 3) << 2;
	n = n & 12;
	for (a = 0;a < 4;a++)
		ml[a] = FP_RFS(n+a) * FP_RFS(m+a);
	FP_RFS(n+3) = ml[0] + ml[1] + ml[2] + ml[3];
}

/* FTRV XMTRX,FVn PR=0 1111nn0111111101 */
INLINE void FTRV(UINT32 n)
{
int i,j;
float sum[4];

	n = n & 12;
	for (i = 0;i < 4;i++) {
		sum[i] = 0;
		for (j=0;j < 4;j++)
			sum[i] += FP_XFS((j << 2) + i)*FP_RFS(n + i);
		FP_RFS(n + i) = sum[i];
	}
}

INLINE void op1111(UINT16 opcode)
{
	switch (opcode & 0xf)
	{
		case 0:
			FADD(Rm,Rn);
			break;
		case 1:
			FSUB(Rm,Rn);
			break;
		case 2:
			FMUL(Rm,Rn);
			break;
		case 3:
			FDIV(Rm,Rn);
			break;
		case 4:
			FCMP_EQ(Rm,Rn);
			break;
		case 5:
			FCMP_GT(Rm,Rn);
			break;
		case 6:
			FMOVS0FR(Rm,Rn);
			break;
		case 7:
			FMOVFRS0(Rm,Rn);
			break;
		case 8:
			FMOVMRFR(Rm,Rn);
			break;
		case 9:
			FMOVMRIFR(Rm,Rn);
			break;
		case 10:
			FMOVFRMR(Rm,Rn);
			break;
		case 11:
			FMOVFRMDR(Rm,Rn);
			break;
		case 12:
			FMOVFR(Rm,Rn);
			break;
		case 13:
			switch (opcode & 0xF0)
			{
				case 0x00:
					FSTS(Rn);
					break;
				case 0x10:
					FLDS(Rn);
					break;
				case 0x20:
					FLOAT(Rn);
					break;
				case 0x30:
					FTRC(Rn);
					break;
				case 0x40:
					FNEG(Rn);
					break;
				case 0x50:
					FABS(Rn);
					break;
				case 0x60:
					FSQRT(Rn);
					break;
				case 0x80:
					FLDI0(Rn);
					break;
				case 0x90:
					FLDI1(Rn);
					break;
				case 0xA0:
					FCNVSD(Rn);
					break;
				case 0xB0:
					FCNVDS(Rn);
					break;
				case 0xE0:
					FIPR(Rn);
					break;
				case 0xF0:
					if (opcode == 0xF3FD)
						FSCHG();
					else if (opcode == 0xFBFD)
						FRCHG();
					else if ((opcode & 0x300) == 0x100)
						FTRV(Rn);
					break;
			}
			break;
		case 14:
			FMAC(Rm,Rn);
			break;
	}
}

/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

static void sh4_reset(void)
{
	void *tsaved[4];
	emu_timer *tsave[5];
	UINT32 *m;
	int cpunum;
	int save_is_slave;
	int	savecpu_clock, savebus_clock, savepm_clock;

	void (*f)(UINT32 data);
	int (*save_irqcallback)(int);

	cpunum = sh4.cpu_number;
	m = sh4.m;
	tsaved[0] = sh4.dma_timer[0];
	tsaved[1] = sh4.dma_timer[1];
	tsaved[2] = sh4.dma_timer[2];
	tsaved[3] = sh4.dma_timer[3];
	tsave[0] = sh4.refresh_timer;
	tsave[1] = sh4.rtc_timer;
	tsave[2] = sh4.timer0;
	tsave[3] = sh4.timer1;
	tsave[4] = sh4.timer2;

	f = sh4.ftcsr_read_callback;
	save_irqcallback = sh4.irq_callback;
	save_is_slave = sh4.is_slave;
	savecpu_clock = sh4.cpu_clock;
	savebus_clock = sh4.bus_clock;
	savepm_clock = sh4.pm_clock;
	memset(&sh4, 0, sizeof(SH4));
	sh4.is_slave = save_is_slave;
	sh4.cpu_clock = savecpu_clock;
	sh4.bus_clock = savebus_clock;
	sh4.pm_clock = savepm_clock;
	sh4.ftcsr_read_callback = f;
	sh4.irq_callback = save_irqcallback;

	sh4.dma_timer[0] = tsaved[0];
	sh4.dma_timer[1] = tsaved[1];
	sh4.dma_timer[2] = tsaved[2];
	sh4.dma_timer[3] = tsaved[3];
	sh4.refresh_timer = tsave[0];
	sh4.rtc_timer = tsave[1];
	sh4.timer0 = tsave[2];
	sh4.timer1 = tsave[3];
	sh4.timer2 = tsave[4];
	sh4.cpu_number = cpunum;
	sh4.m = m;
	memset(sh4.m, 0, 16384*4);
	sh4_default_exception_priorities();
	memset(sh4.exception_requesting, 0, sizeof(sh4.exception_requesting));

	timer_adjust(sh4.rtc_timer, ATTOTIME_IN_HZ(128), cpunum, attotime_zero);
	sh4.m[RCR2] = 0x09;
	sh4.m[TCOR0] = 0xffffffff;
	sh4.m[TCNT0] = 0xffffffff;
	sh4.m[TCOR1] = 0xffffffff;
	sh4.m[TCNT1] = 0xffffffff;
	sh4.m[TCOR2] = 0xffffffff;
	sh4.m[TCNT2] = 0xffffffff;

	sh4.pc = 0xa0000000;
	sh4.r[15] = RL(4);
	sh4.sr = 0x700000f0;
	sh4.fpscr = 0x00040001;
	sh4.fpu_sz = (sh4.fpscr & SZ) ? 1 : 0;
	sh4.fpu_pr = (sh4.fpscr & PR) ? 1 : 0;
	sh4.fpul = 0;
	sh4.dbr = 0;
	change_pc(sh4.pc & AM);

	sh4.internal_irq_level = -1;
	sh4.irln = 15;
}

/* Execute cycles - returns number of cycles actually run */
static int sh4_execute(int cycles)
{
	sh4_icount = cycles;

	if (sh4.cpu_off)
		return 0;

	do
	{
		UINT32 opcode;

		if (sh4.delay)
		{
			opcode = cpu_readop16(WORD2_XOR_LE((UINT32)(sh4.delay & AM)));
			change_pc(sh4.pc & AM);
			sh4.pc -= 2;
		}
		else
			opcode = cpu_readop16(WORD2_XOR_LE((UINT32)(sh4.pc & AM)));

		CALL_MAME_DEBUG;

		sh4.delay = 0;
		sh4.pc += 2;
		sh4.ppc = sh4.pc;

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

		if (sh4.test_irq && !sh4.delay)
		{
			sh4_check_pending_irq("mame_sh4_execute");
		}
		sh4_icount--;
	} while( sh4_icount > 0 );

	return cycles - sh4_icount;
}

/* Get registers, return context size */
static void sh4_get_context(void *dst)
{
	if( dst )
		memcpy(dst, &sh4, sizeof(SH4));
}

/* Set registers */
static void sh4_set_context(void *src)
{
	if( src )
		memcpy(&sh4, src, sizeof(SH4));
}

static UINT32 compute_ticks_refresh_timer(emu_timer *timer, int hertz, int base, int divisor)
{
	// elapsed:total = x : ticks
	// x=elapsed*tics/total -> x=elapsed*(double)100000000/rtcnt_div[(sh4.m[RTCSR] >> 3) & 7]
	// ticks/total=ticks / ((rtcnt_div[(sh4.m[RTCSR] >> 3) & 7] * ticks) / 100000000)=1/((rtcnt_div[(sh4.m[RTCSR] >> 3) & 7] / 100000000)=100000000/rtcnt_div[(sh4.m[RTCSR] >> 3) & 7]
	return base + (UINT32)((attotime_to_double(timer_timeelapsed(timer)) * (double)hertz) / (double)divisor);
}

static void sh4_refresh_timer_recompute(void)
{
UINT32 ticks;

	//if rtcnt < rtcor then rtcor-rtcnt
	//if rtcnt >= rtcor then 256-rtcnt+rtcor=256+rtcor-rtcnt
	ticks = sh4.m[RTCOR]-sh4.m[RTCNT];
	if (ticks < 0)
		ticks = 256 + ticks;
	//((double)rtcnt_div[(sh4.m[RTCSR] >> 3) & 7] / (double)100000000)*ticks
	timer_adjust(sh4.refresh_timer, attotime_mul(attotime_mul(ATTOTIME_IN_HZ(sh4.bus_clock), rtcnt_div[(sh4.m[RTCSR] >> 3) & 7]), ticks), sh4.cpu_number, attotime_zero);
	sh4.refresh_timer_base = sh4.m[RTCNT];
}

/*-------------------------------------------------
    sh4_scale_up_mame_time - multiply a attotime by
    a (constant+1) where 0 <= constant < 2^32
-------------------------------------------------*/

INLINE attotime sh4_scale_up_mame_time(attotime _time1, UINT32 factor1)
{
	return attotime_add(attotime_mul(_time1, factor1), _time1);
}

static UINT32 compute_ticks_timer(emu_timer *timer, int hertz, int divisor)
{
	double ret;

	ret=((attotime_to_double(timer_timeleft(timer)) * (double)hertz) / (double)divisor) - 1;
	return (UINT32)ret;
}

static void sh4_timer0_recompute(void)
{
	double ticks;

	ticks = sh4.m[TCNT0];
	timer_adjust(sh4.timer0, sh4_scale_up_mame_time(attotime_mul(ATTOTIME_IN_HZ(sh4.pm_clock), tcnt_div[sh4.m[TCR0] & 7]), ticks), sh4.cpu_number, attotime_zero);
}

static void sh4_timer1_recompute(void)
{
	double ticks;

	ticks = sh4.m[TCNT1];
	timer_adjust(sh4.timer1, sh4_scale_up_mame_time(attotime_mul(ATTOTIME_IN_HZ(sh4.pm_clock), tcnt_div[sh4.m[TCR1] & 7]), ticks), sh4.cpu_number, attotime_zero);
}

static void sh4_timer2_recompute(void)
{
	double ticks;

	ticks = sh4.m[TCNT2];
	timer_adjust(sh4.timer2, sh4_scale_up_mame_time(attotime_mul(ATTOTIME_IN_HZ(sh4.pm_clock), tcnt_div[sh4.m[TCR2] & 7]), ticks), sh4.cpu_number, attotime_zero);
}

static TIMER_CALLBACK( sh4_refresh_timer_callback )
{
	int cpunum = param;

	cpuintrf_push_context(cpunum);
	sh4.m[RTCNT] = 0;
	sh4_refresh_timer_recompute();
	sh4.m[RTCSR] |= 128;
	if ((sh4.m[MCR] & 4) && !(sh4.m[MCR] & 2))
	{
		sh4.m[RFCR] = (sh4.m[RFCR] + 1) & 1023;
		if (((sh4.m[RTCSR] & 1) && (sh4.m[RFCR] == 512)) || (sh4.m[RFCR] == 0))
		{
			sh4.m[RFCR] = 0;
			sh4.m[RTCSR] |= 4;
		}
	}
	cpuintrf_pop_context();
}

static void increment_rtc_time(int mode)
{
	int carry, year, leap, days;

	if (mode == 0)
	{
		carry = 0;
		sh4.m[RSECCNT] = sh4.m[RSECCNT] + 1;
		if ((sh4.m[RSECCNT] & 0xf) == 0xa)
			sh4.m[RSECCNT] = sh4.m[RSECCNT] + 6;
		if (sh4.m[RSECCNT] == 0x60)
		{
			sh4.m[RSECCNT] = 0;
			carry=1;
		}
		else
			return;
	}
	else
		carry = 1;

	sh4.m[RMINCNT] = sh4.m[RMINCNT] + carry;
	if ((sh4.m[RMINCNT] & 0xf) == 0xa)
		sh4.m[RMINCNT] = sh4.m[RMINCNT] + 6;
	carry=0;
	if (sh4.m[RMINCNT] == 0x60)
	{
		sh4.m[RMINCNT] = 0;
		carry = 1;
	}

	sh4.m[RHRCNT] = sh4.m[RHRCNT] + carry;
	if ((sh4.m[RHRCNT] & 0xf) == 0xa)
		sh4.m[RHRCNT] = sh4.m[RHRCNT] + 6;
	carry = 0;
	if (sh4.m[RHRCNT] == 0x24)
	{
		sh4.m[RHRCNT] = 0;
		carry = 1;
	}

	sh4.m[RWKCNT] = sh4.m[RWKCNT] + carry;
	if (sh4.m[RWKCNT] == 0x7)
	{
		sh4.m[RWKCNT] = 0;
	}

	year = (sh4.m[RYRCNT] & 0xf) + ((sh4.m[RYRCNT] & 0xf0) >> 4)*10 + ((sh4.m[RYRCNT] & 0xf00) >> 8)*100 + ((sh4.m[RYRCNT] & 0xf000) >> 12)*1000;
	leap = 0;
	if (!(year%100))
	{
		if (!(year%400))
			leap = 1;
	}
	else if (!(year%4))
		leap = 1;
	if (sh4.m[RMONCNT] != 2)
		leap = 0;
	days = daysmonth[(sh4.m[RMONCNT] & 0xf) + ((sh4.m[RMONCNT] & 0xf0) >> 4)*10 - 1];

	sh4.m[RDAYCNT] = sh4.m[RDAYCNT] + carry;
	if ((sh4.m[RDAYCNT] & 0xf) == 0xa)
		sh4.m[RDAYCNT] = sh4.m[RDAYCNT] + 6;
	carry = 0;
	if (sh4.m[RDAYCNT] > (days+leap))
	{
		sh4.m[RDAYCNT] = 1;
		carry = 1;
	}

	sh4.m[RMONCNT] = sh4.m[RMONCNT] + carry;
	if ((sh4.m[RMONCNT] & 0xf) == 0xa)
		sh4.m[RMONCNT] = sh4.m[RMONCNT] + 6;
	carry=0;
	if (sh4.m[RMONCNT] == 0x13)
	{
		sh4.m[RMONCNT] = 1;
		carry = 1;
	}

	sh4.m[RYRCNT] = sh4.m[RYRCNT] + carry;
	if ((sh4.m[RYRCNT] & 0xf) >= 0xa)
		sh4.m[RYRCNT] = sh4.m[RYRCNT] + 6;
	if ((sh4.m[RYRCNT] & 0xf0) >= 0xa0)
		sh4.m[RYRCNT] = sh4.m[RYRCNT] + 0x60;
	if ((sh4.m[RYRCNT] & 0xf00) >= 0xa00)
		sh4.m[RYRCNT] = sh4.m[RYRCNT] + 0x600;
	if ((sh4.m[RYRCNT] & 0xf000) >= 0xa000)
		sh4.m[RYRCNT] = 0;
}

static TIMER_CALLBACK( sh4_rtc_timer_callback )
{
	int cpunum = param;

	cpuintrf_push_context(cpunum);
	timer_adjust(sh4.rtc_timer, ATTOTIME_IN_HZ(128), cpunum, attotime_zero);
	sh4.m[R64CNT] = (sh4.m[R64CNT]+1) & 0x7f;
	if (sh4.m[R64CNT] == 64)
	{
		sh4.m[RCR1] |= 0x80;
		increment_rtc_time(0);
		//sh4_exception_request(SH4_INTC_NMI); // TEST
	}
	cpuintrf_pop_context();
}

static TIMER_CALLBACK( sh4_timer0_callback )
{
	int cpunum = param;

	cpuintrf_push_context(cpunum);
	sh4.m[TCNT0] = sh4.m[TCOR0];
	sh4_timer0_recompute();
	sh4.m[TCR0] = sh4.m[TCR0] | 0x100;
	if (sh4.m[TCR0] & 0x20)
		sh4_exception_request(SH4_INTC_TUNI0);
	cpuintrf_pop_context();
}

static TIMER_CALLBACK( sh4_timer1_callback )
{
	int cpunum = param;

	cpuintrf_push_context(cpunum);
	sh4.m[TCNT1] = sh4.m[TCOR1];
	sh4_timer1_recompute();
	sh4.m[TCR1] = sh4.m[TCR1] | 0x100;
	if (sh4.m[TCR1] & 0x20)
		sh4_exception_request(SH4_INTC_TUNI1);
	cpuintrf_pop_context();
}

static TIMER_CALLBACK( sh4_timer2_callback )
{
	int cpunum = param;

	cpuintrf_push_context(cpunum);
	sh4.m[TCNT2] = sh4.m[TCOR2];
	sh4_timer2_recompute();
	sh4.m[TCR2] = sh4.m[TCR2] | 0x100;
	if (sh4.m[TCR2] & 0x20)
		sh4_exception_request(SH4_INTC_TUNI2);
	cpuintrf_pop_context();
}

static TIMER_CALLBACK( sh4_dmac_callback )
{
	int cpunum = param >> 8;
	int channel = param & 255;

	cpuintrf_push_context(cpunum);
	LOG(("SH4.%d: DMA %d complete\n", cpunum, channel));
	sh4.dma_timer_active[channel] = 0;
	switch (channel)
	{
	case 0:
		sh4.m[DMATCR0] = 0;
		sh4.m[CHCR0] |= 2;
		if (sh4.m[CHCR0] & 4)
			sh4_exception_request(SH4_INTC_DMTE0);
		break;
	case 1:
		sh4.m[DMATCR1] = 0;
		sh4.m[CHCR1] |= 2;
		if (sh4.m[CHCR1] & 4)
			sh4_exception_request(SH4_INTC_DMTE1);
		break;
	case 2:
		sh4.m[DMATCR2] = 0;
		sh4.m[CHCR2] |= 2;
		if (sh4.m[CHCR2] & 4)
			sh4_exception_request(SH4_INTC_DMTE2);
		break;
	case 3:
		sh4.m[DMATCR3] = 0;
		sh4.m[CHCR3] |= 2;
		if (sh4.m[CHCR3] & 4)
			sh4_exception_request(SH4_INTC_DMTE3);
		break;
	}
	cpuintrf_pop_context();
}

static int sh4_dma_transfer(int channel, int timermode, UINT32 chcr, UINT32 *sar, UINT32 *dar, UINT32 *dmatcr)
{
	int incs, incd, size;
	UINT32 src, dst, count;

	incd = (chcr >> 14) & 3;
	incs = (chcr >> 12) & 3;
	size = dmasize[(chcr >> 4) & 7];
	if(incd == 3 || incs == 3)
	{
		logerror("SH4: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, chcr);
		return 0;
	}
	src   = *sar;
	dst   = *dar;
	count = *dmatcr;
	if (!count)
		count = 0x1000000;

	LOG(("SH4: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", channel, src, dst, count, chcr, incs, incd, size));

	if (timermode == 1)
	{
		sh4.dma_timer_active[channel] = 1;
		timer_adjust(sh4.dma_timer[channel], ATTOTIME_IN_CYCLES(2*count+1, sh4.cpu_number), (sh4.cpu_number << 8) | channel, attotime_zero);
	}
	else if (timermode == 2)
	{
		sh4.dma_timer_active[channel] = 1;
		timer_adjust(sh4.dma_timer[channel], attotime_zero, (sh4.cpu_number << 8) | channel, attotime_zero);
	}

	src &= AM;
	dst &= AM;

	switch(size)
	{
	case 1: // 8 bit
		for(;count > 0; count --)
		{
			if(incs == 2)
				src --;
			if(incd == 2)
				dst --;
			program_write_byte_64le(dst, program_read_byte_64le(src));
			if(incs == 1)
				src ++;
			if(incd == 1)
				dst ++;
		}
		break;
	case 2: // 16 bit
		src &= ~1;
		dst &= ~1;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 2;
			if(incd == 2)
				dst -= 2;
			program_write_word_64le(dst, program_read_word_64le(src));
			if(incs == 1)
				src += 2;
			if(incd == 1)
				dst += 2;
		}
		break;
	case 8: // 64 bit
		src &= ~7;
		dst &= ~7;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 8;
			if(incd == 2)
				dst -= 8;
			program_write_qword_64le(dst, program_read_qword_64le(src));
			if(incs == 1)
				src += 8;
			if(incd == 1)
				dst += 8;

		}
		break;
	case 4: // 32 bit
		src &= ~3;
		dst &= ~3;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 4;
			if(incd == 2)
				dst -= 4;
			program_write_dword_64le(dst, program_read_dword_64le(src));
			if(incs == 1)
				src += 4;
			if(incd == 1)
				dst += 4;

		}
		break;
	case 32:
		src &= ~31;
		dst &= ~31;
		for(;count > 0; count --)
		{
			if(incs == 2)
				src -= 32;
			if(incd == 2)
				dst -= 32;
			program_write_qword_64le(dst, program_read_qword_64le(src));
			program_write_qword_64le(dst+8, program_read_qword_64le(src+8));
			program_write_qword_64le(dst+16, program_read_qword_64le(src+16));
			program_write_qword_64le(dst+24, program_read_qword_64le(src+24));
			if(incs == 1)
				src += 32;
			if(incd == 1)
				dst += 32;
		}
		break;
	}
	*sar    = (*sar & !AM) | src;
	*dar    = (*dar & !AM) | dst;
	*dmatcr = count;
	return 1;
}

static void sh4_dmac_check(int channel)
{
UINT32 dmatcr,chcr,sar,dar;

	switch (channel)
	{
	case 0:
		sar = sh4.m[SAR0];
		dar = sh4.m[DAR0];
		chcr = sh4.m[CHCR0];
		dmatcr = sh4.m[DMATCR0];
		break;
	case 1:
		sar = sh4.m[SAR1];
		dar = sh4.m[DAR1];
		chcr = sh4.m[CHCR1];
		dmatcr = sh4.m[DMATCR1];
		break;
	case 2:
		sar = sh4.m[SAR2];
		dar = sh4.m[DAR2];
		chcr = sh4.m[CHCR2];
		dmatcr = sh4.m[DMATCR2];
		break;
	case 3:
		sar = sh4.m[SAR3];
		dar = sh4.m[DAR3];
		chcr = sh4.m[CHCR3];
		dmatcr = sh4.m[DMATCR3];
		break;
	default:
		return;
	}
	if (chcr & sh4.m[DMAOR] & 1)
	{
		if ((((chcr >> 8) & 15) < 4) || (((chcr >> 8) & 15) > 6))
			return;
		if (!sh4.dma_timer_active[channel] && !(chcr & 2) && !(sh4.m[DMAOR] & 6))
			sh4_dma_transfer(channel, 1, chcr, &sar, &dar, &dmatcr);
	}
	else
	{
		if (sh4.dma_timer_active[channel])
		{
			logerror("SH4: DMA %d cancelled in-flight but all data transferred", channel);
			timer_adjust(sh4.dma_timer[channel], attotime_never, 0, attotime_zero);
			sh4.dma_timer_active[channel] = 0;
		}
	}
}

static void sh4_dmac_nmi(void) // manage dma when nmi
{
int s;

	sh4.m[DMAOR] |= 2; // nmif = 1
	for (s = 0;s < 4;s++)
	{
		if (sh4.dma_timer_active[s])
		{
			logerror("SH4: DMA %d cancelled due to NMI but all data transferred", s);
			timer_adjust(sh4.dma_timer[s], attotime_never, 0, attotime_zero);
			sh4.dma_timer_active[s] = 0;
		}
	}
}

WRITE32_HANDLER( sh4_internal_w )
{
	UINT32 old = sh4.m[offset];
	COMBINE_DATA(sh4.m+offset);

	//  logerror("sh4_internal_w:  Write %08x (%x), %08x @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, data, mem_mask);

	switch( offset )
	{
		// Memory refresh
	case RTCSR:
		sh4.m[RTCSR] &= 255;
		if ((old >> 3) & 7)
			sh4.m[RTCNT] = compute_ticks_refresh_timer(sh4.refresh_timer, sh4.bus_clock, sh4.refresh_timer_base, rtcnt_div[(old >> 3) & 7]) & 0xff;
		if ((sh4.m[RTCSR] >> 3) & 7)
		{ // activated
			sh4_refresh_timer_recompute();
		}
		else
		{
			timer_adjust(sh4.refresh_timer, attotime_never, 0, attotime_zero);
		}
		break;

	case RTCNT:
		sh4.m[RTCNT] &= 255;
		if ((sh4.m[RTCSR] >> 3) & 7)
		{ // active
			sh4_refresh_timer_recompute();
		}
		break;

	case RTCOR:
		sh4.m[RTCOR] &= 255;
		if ((sh4.m[RTCSR] >> 3) & 7)
		{ // active
			sh4.m[RTCNT] = compute_ticks_refresh_timer(sh4.refresh_timer, sh4.bus_clock, sh4.refresh_timer_base, rtcnt_div[(sh4.m[RTCSR] >> 3) & 7]) & 0xff;
			sh4_refresh_timer_recompute();
		}
		break;

	case RFCR:
		sh4.m[RFCR] &= 1023;
		break;

		// RTC
	case RCR1:
		if ((sh4.m[RCR1] & 8) && (~old & 8)) // 0 -> 1
			sh4.m[RCR1] ^= 1;
		break;

	case RCR2:
		if (sh4.m[RCR2] & 2)
		{
			sh4.m[R64CNT] = 0;
			sh4.m[RCR2] ^= 2;
		}
		if (sh4.m[RCR2] & 4)
		{
			sh4.m[R64CNT] = 0;
			if (sh4.m[RSECCNT] >= 30)
				increment_rtc_time(1);
			sh4.m[RSECCNT] = 0;
		}
		if ((sh4.m[RCR2] & 8) && (~old & 8))
		{ // 0 -> 1
			timer_adjust(sh4.rtc_timer, ATTOTIME_IN_HZ(128), sh4.cpu_number, attotime_zero);
		}
		else if (~(sh4.m[RCR2]) & 8)
		{ // 0
			timer_adjust(sh4.rtc_timer, attotime_never, 0, attotime_zero);
		}
		break;

		// TMU
	case TSTR:
		if (old & 1)
			sh4.m[TCNT0] = compute_ticks_timer(sh4.timer0, sh4.pm_clock, tcnt_div[sh4.m[TCR0] & 7]);
		if ((sh4.m[TSTR] & 1) == 0) {
			timer_adjust(sh4.timer0, attotime_never, 0, attotime_zero);
		} else
			sh4_timer0_recompute();

		if (old & 2)
			sh4.m[TCNT1] = compute_ticks_timer(sh4.timer1, sh4.pm_clock, tcnt_div[sh4.m[TCR1] & 7]);
		if ((sh4.m[TSTR] & 2) == 0) {
			timer_adjust(sh4.timer1, attotime_never, 0, attotime_zero);
		} else
			sh4_timer1_recompute();

		if (old & 4)
			sh4.m[TCNT2] = compute_ticks_timer(sh4.timer2, sh4.pm_clock, tcnt_div[sh4.m[TCR2] & 7]);
		if ((sh4.m[TSTR] & 4) == 0) {
			timer_adjust(sh4.timer2, attotime_never, 0, attotime_zero);
		} else
			sh4_timer2_recompute();
		break;

	case TCR0:
		if (sh4.m[TSTR] & 1)
		{
			sh4.m[TCNT0] = compute_ticks_timer(sh4.timer0, sh4.pm_clock, tcnt_div[old & 7]);
			sh4_timer0_recompute();
		}
		if (!(sh4.m[TCR0] & 0x20) || !(sh4.m[TCR0] & 0x100))
			sh4_exception_unrequest(SH4_INTC_TUNI0);
		break;
	case TCR1:
		if (sh4.m[TSTR] & 2)
		{
			sh4.m[TCNT1] = compute_ticks_timer(sh4.timer1, sh4.pm_clock, tcnt_div[old & 7]);
			sh4_timer1_recompute();
		}
		if (!(sh4.m[TCR1] & 0x20) || !(sh4.m[TCR1] & 0x100))
			sh4_exception_unrequest(SH4_INTC_TUNI1);
		break;
	case TCR2:
		if (sh4.m[TSTR] & 4)
		{
			sh4.m[TCNT2] = compute_ticks_timer(sh4.timer2, sh4.pm_clock, tcnt_div[old & 7]);
			sh4_timer2_recompute();
		}
		if (!(sh4.m[TCR2] & 0x20) || !(sh4.m[TCR2] & 0x100))
			sh4_exception_unrequest(SH4_INTC_TUNI2);
		break;

	case TCOR0:
		if (sh4.m[TSTR] & 1)
		{
			sh4.m[TCNT0] = compute_ticks_timer(sh4.timer0, sh4.pm_clock, tcnt_div[sh4.m[TCR0] & 7]);
			sh4_timer0_recompute();
		}
		break;
	case TCNT0:
		if (sh4.m[TSTR] & 1)
			sh4_timer0_recompute();
		break;
	case TCOR1:
		if (sh4.m[TSTR] & 2)
		{
			sh4.m[TCNT1] = compute_ticks_timer(sh4.timer1, sh4.pm_clock, tcnt_div[sh4.m[TCR1] & 7]);
			sh4_timer1_recompute();
		}
		break;
	case TCNT1:
		if (sh4.m[TSTR] & 2)
			sh4_timer1_recompute();
		break;
	case TCOR2:
		if (sh4.m[TSTR] & 4)
		{
			sh4.m[TCNT2] = compute_ticks_timer(sh4.timer2, sh4.pm_clock, tcnt_div[sh4.m[TCR2] & 7]);
			sh4_timer2_recompute();
		}
		break;
	case TCNT2:
		if (sh4.m[TSTR] & 4)
			sh4_timer2_recompute();
		break;

		// INTC
	case ICR:
		sh4.m[ICR] = (sh4.m[ICR] & 0x7fff) | (old & 0x8000);
		break;
	case IPRA:
		sh4.exception_priority[SH4_INTC_ATI] = INTPRI(sh4.m[IPRA] & 0x000f, SH4_INTC_ATI);
		sh4.exception_priority[SH4_INTC_PRI] = INTPRI(sh4.m[IPRA] & 0x000f, SH4_INTC_PRI);
		sh4.exception_priority[SH4_INTC_CUI] = INTPRI(sh4.m[IPRA] & 0x000f, SH4_INTC_CUI);
		sh4.exception_priority[SH4_INTC_TUNI2] = INTPRI((sh4.m[IPRA] & 0x00f0) >> 4, SH4_INTC_TUNI2);
		sh4.exception_priority[SH4_INTC_TICPI2] = INTPRI((sh4.m[IPRA] & 0x00f0) >> 4, SH4_INTC_TICPI2);
		sh4.exception_priority[SH4_INTC_TUNI1] = INTPRI((sh4.m[IPRA] & 0x0f00) >> 8, SH4_INTC_TUNI1);
		sh4.exception_priority[SH4_INTC_TUNI0] = INTPRI((sh4.m[IPRA] & 0xf000) >> 12, SH4_INTC_TUNI0);
		sh4_exception_recompute();
		break;
	case IPRB:
		sh4.exception_priority[SH4_INTC_SCI1ERI] = INTPRI((sh4.m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1ERI);
		sh4.exception_priority[SH4_INTC_SCI1RXI] = INTPRI((sh4.m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1RXI);
		sh4.exception_priority[SH4_INTC_SCI1TXI] = INTPRI((sh4.m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TXI);
		sh4.exception_priority[SH4_INTC_SCI1TEI] = INTPRI((sh4.m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TEI);
		sh4.exception_priority[SH4_INTC_RCMI] = INTPRI((sh4.m[IPRB] & 0x0f00) >> 8, SH4_INTC_RCMI);
		sh4.exception_priority[SH4_INTC_ROVI] = INTPRI((sh4.m[IPRB] & 0x0f00) >> 8, SH4_INTC_ROVI);
		sh4.exception_priority[SH4_INTC_ITI] = INTPRI((sh4.m[IPRB] & 0xf000) >> 12, SH4_INTC_ITI);
		sh4_exception_recompute();
		break;
	case IPRC:
		sh4.exception_priority[SH4_INTC_HUDI] = INTPRI(sh4.m[IPRC] & 0x000f, SH4_INTC_HUDI);
		sh4.exception_priority[SH4_INTC_SCIFERI] = INTPRI((sh4.m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFERI);
		sh4.exception_priority[SH4_INTC_SCIFRXI] = INTPRI((sh4.m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFRXI);
		sh4.exception_priority[SH4_INTC_SCIFBRI] = INTPRI((sh4.m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFBRI);
		sh4.exception_priority[SH4_INTC_SCIFTXI] = INTPRI((sh4.m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFTXI);
		sh4.exception_priority[SH4_INTC_DMTE0] = INTPRI((sh4.m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE0);
		sh4.exception_priority[SH4_INTC_DMTE1] = INTPRI((sh4.m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE1);
		sh4.exception_priority[SH4_INTC_DMTE2] = INTPRI((sh4.m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE2);
		sh4.exception_priority[SH4_INTC_DMTE3] = INTPRI((sh4.m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE3);
		sh4.exception_priority[SH4_INTC_DMAE] = INTPRI((sh4.m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMAE);
		sh4.exception_priority[SH4_INTC_GPOI] = INTPRI((sh4.m[IPRC] & 0xf000) >> 12, SH4_INTC_GPOI);
		sh4_exception_recompute();
		break;

	// DMA
	case SAR0:
	case SAR1:
	case SAR2:
	case SAR3:
	case DAR0:
	case DAR1:
	case DAR2:
	case DAR3:
	case DMATCR0:
	case DMATCR1:
	case DMATCR2:
	case DMATCR3:
		break;
	case CHCR0:
		sh4_dmac_check(0);
		break;
	case CHCR1:
		sh4_dmac_check(1);
		break;
	case CHCR2:
		sh4_dmac_check(2);
		break;
	case CHCR3:
		sh4_dmac_check(3);
		break;
	case DMAOR:
		if ((sh4.m[DMAOR] & 4) && (~old & 4))
			sh4.m[DMAOR] &= ~4;
		if ((sh4.m[DMAOR] & 2) && (~old & 2))
			sh4.m[DMAOR] &= ~2;
		sh4_dmac_check(0);
		sh4_dmac_check(1);
		sh4_dmac_check(2);
		sh4_dmac_check(3);
		break;

		// Store Queues
	case QACR0:
	case QACR1:
		break;

	case SCBRR2:
		break;

	default:
		logerror("sh4_internal_w:  Unmapped write %08x, %08x @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), data, mem_mask);
		break;
	}
}

READ32_HANDLER( sh4_internal_r )
{
	//  logerror("sh4_internal_r:  Read %08x (%x) @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, mem_mask);
	switch( offset )
	{
	case RTCNT:
		if ((sh4.m[RTCSR] >> 3) & 7)
		{ // activated
			//((double)rtcnt_div[(sh4.m[RTCSR] >> 3) & 7] / (double)100000000)
			//return (refresh_timer_base + (timer_timeelapsed(sh4.refresh_timer) * (double)100000000) / (double)rtcnt_div[(sh4.m[RTCSR] >> 3) & 7]) & 0xff;
			return compute_ticks_refresh_timer(sh4.refresh_timer, sh4.bus_clock, sh4.refresh_timer_base, rtcnt_div[(sh4.m[RTCSR] >> 3) & 7]) & 0xff;
		}
		else
			return sh4.m[RTCNT];
		break;

	case TCNT0:
		if (sh4.m[TSTR] & 1)
			return compute_ticks_timer(sh4.timer0, sh4.pm_clock, tcnt_div[sh4.m[TCR0] & 7]);
		else
			return sh4.m[TCNT0];
		break;
	case TCNT1:
		if (sh4.m[TSTR] & 2)
			return compute_ticks_timer(sh4.timer1, sh4.pm_clock, tcnt_div[sh4.m[TCR1] & 7]);
		else
			return sh4.m[TCNT1];
		break;
	case TCNT2:
		if (sh4.m[TSTR] & 4)
			return compute_ticks_timer(sh4.timer2, sh4.pm_clock, tcnt_div[sh4.m[TCR2] & 7]);
		else
			return sh4.m[TCNT2];
		break;
	}
	return sh4.m[offset];
}

static void sh4_set_frt_input(int cpunum, int state)
{
	if(state == PULSE_LINE)
	{
		sh4_set_frt_input(cpunum, ASSERT_LINE);
		sh4_set_frt_input(cpunum, CLEAR_LINE);
		return;
	}

	cpuintrf_push_context(cpunum);

	if(sh4.frt_input == state) {
		cpuintrf_pop_context();
		return;
	}

	sh4.frt_input = state;

	if(sh4.m[5] & 0x8000) {
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

#if 0
	sh4_timer_resync();
	sh4.icr = sh4.frc;
	sh4.m[4] |= ICF;
	logerror("SH4.%d: ICF activated (%x)\n", sh4.cpu_number, sh4.pc & AM);
	sh4_recalc_irq();
#endif
	cpuintrf_pop_context();
}

static void sh4_set_irln_input(int cpunum, int value)
{
	if (sh4.irln == value)
		return;
	sh4.irln = value;
	cpunum_set_input_line(cpunum, SH4_IRLn, PULSE_LINE);
}

static void set_irq_line(int irqline, int state) // set state of external interrupt line
{
int s;

	if (irqline == INPUT_LINE_NMI)
    {
		if (sh4.nmi_line_state == state)
			return;
		if (sh4.m[ICR] & 0x100)
		{
			if ((state == CLEAR_LINE) && (sh4.nmi_line_state == ASSERT_LINE))  // rising
			{
				LOG(("SH-4 #%d assert nmi\n", cpu_getactivecpu()));
				sh4_exception_request(SH4_INTC_NMI);
				sh4_dmac_nmi();
			}
		}
		else
		{
			if ((state == ASSERT_LINE) && (sh4.nmi_line_state == CLEAR_LINE)) // falling
			{
				LOG(("SH-4 #%d assert nmi\n", cpu_getactivecpu()));
				sh4_exception_request(SH4_INTC_NMI);
				sh4_dmac_nmi();
			}
		}
		if (state == CLEAR_LINE)
			sh4.m[ICR] ^= 0x8000;
		else
			sh4.m[ICR] |= 0x8000;
		sh4.nmi_line_state = state;
	}
	else
	{
		if (sh4.m[ICR] & 0x80) // four independent external interrupt sources
		{
			if (irqline > SH4_IRL3)
				return;
			if (sh4.irq_line_state[irqline] == state)
				return;
			sh4.irq_line_state[irqline] = state;

			if( state == CLEAR_LINE )
			{
				LOG(("SH-4 #%d cleared external irq IRL%d\n", cpu_getactivecpu(), irqline));
				sh4_exception_unrequest(SH4_INTC_IRL0+irqline-SH4_IRL0);
			}
			else
			{
				LOG(("SH-4 #%d assert external irq IRL%d\n", cpu_getactivecpu(), irqline));
				sh4_exception_request(SH4_INTC_IRL0+irqline-SH4_IRL0);
			}
		}
		else // level-encoded interrupt
		{
			if (irqline != SH4_IRLn)
				return;
			if ((sh4.irln > 15) || (sh4.irln < 0))
				return;
			for (s = 0; s < 15; s++)
				sh4_exception_unrequest(SH4_INTC_IRLn0+s);
			if (sh4.irln < 15)
				sh4_exception_request(SH4_INTC_IRLn0+sh4.irln);
			LOG(("SH-4 #%d IRLn0-IRLn3 level #%d\n", cpu_getactivecpu(), sh4.irln));
		}
	}
	if (sh4.test_irq && (!sh4.delay))
		sh4_check_pending_irq("sh4_set_irq_line");
}

static void sh4_parse_configuration(const struct sh4_config *conf)
{
	if(conf)
	{
		switch((conf->md2 << 2) | (conf->md1 << 1) | (conf->md0))
		{
		case 0:
			sh4.cpu_clock = conf->clock;
			sh4.bus_clock = conf->clock / 4;
			sh4.pm_clock = conf->clock / 4;
			break;
		case 1:
			sh4.cpu_clock = conf->clock;
			sh4.bus_clock = conf->clock / 6;
			sh4.pm_clock = conf->clock / 6;
			break;
		case 2:
			sh4.cpu_clock = conf->clock;
			sh4.bus_clock = conf->clock / 3;
			sh4.pm_clock = conf->clock / 6;
			break;
		case 3:
			sh4.cpu_clock = conf->clock;
			sh4.bus_clock = conf->clock / 3;
			sh4.pm_clock = conf->clock / 6;
			break;
		case 4:
			sh4.cpu_clock = conf->clock;
			sh4.bus_clock = conf->clock / 2;
			sh4.pm_clock = conf->clock / 4;
			break;
		case 5:
			sh4.cpu_clock = conf->clock;
			sh4.bus_clock = conf->clock / 2;
			sh4.pm_clock = conf->clock / 4;
			break;
		}
		sh4.is_slave = (~(conf->md7)) & 1;
	}
	else
	{
		sh4.cpu_clock = 200000000;
		sh4.bus_clock = 100000000;
		sh4.pm_clock = 50000000;
		sh4.is_slave = 0;
	}
}

#ifdef MAME_DEBUG
static offs_t sh4_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram)
{
	return DasmSH4( buffer, pc, (oprom[1] << 8) | oprom[0] );
}
#endif /* MAME_DEBUG */

static void sh4_init(int index, int clock, const void *config, int (*irqcallback)(int))
{
	const struct sh4_config *conf = config;

	sh4.timer0 = timer_alloc(sh4_timer0_callback, NULL);
	timer_adjust(sh4.timer0, attotime_never, 0, attotime_zero);
	sh4.timer1 = timer_alloc(sh4_timer1_callback, NULL);
	timer_adjust(sh4.timer1, attotime_never, 0, attotime_zero);
	sh4.timer2 = timer_alloc(sh4_timer2_callback, NULL);
	timer_adjust(sh4.timer2, attotime_never, 0, attotime_zero);

	sh4.dma_timer[0] = timer_alloc(sh4_dmac_callback, NULL);
	timer_adjust(sh4.dma_timer[0], attotime_never, 0, attotime_zero);
	sh4.dma_timer[1] = timer_alloc(sh4_dmac_callback, NULL);
	timer_adjust(sh4.dma_timer[1], attotime_never, 0, attotime_zero);
	sh4.dma_timer[2] = timer_alloc(sh4_dmac_callback, NULL);
	timer_adjust(sh4.dma_timer[2], attotime_never, 0, attotime_zero);
	sh4.dma_timer[3] = timer_alloc(sh4_dmac_callback, NULL);
	timer_adjust(sh4.dma_timer[3], attotime_never, 0, attotime_zero);

	sh4.refresh_timer = timer_alloc(sh4_refresh_timer_callback, NULL);
	timer_adjust(sh4.refresh_timer, attotime_never, 0, attotime_zero);
	sh4.refresh_timer_base = 0;

	sh4.rtc_timer = timer_alloc(sh4_rtc_timer_callback, NULL);
	timer_adjust(sh4.rtc_timer, attotime_never, 0, attotime_zero);

	sh4.m = auto_malloc(16384*4);

	sh4_parse_configuration(conf);

	sh4.cpu_number = index;
	sh4.irq_callback = irqcallback;
	sh4_default_exception_priorities();
	sh4.irln = 15;
	sh4.test_irq = 0;

	state_save_register_item("sh4", index, sh4.pc);
	state_save_register_item("sh4", index, sh4.r[15]);
	state_save_register_item("sh4", index, sh4.sr);
	state_save_register_item("sh4", index, sh4.pr);
	state_save_register_item("sh4", index, sh4.gbr);
	state_save_register_item("sh4", index, sh4.vbr);
	state_save_register_item("sh4", index, sh4.mach);
	state_save_register_item("sh4", index, sh4.macl);
	state_save_register_item("sh4", index, sh4.spc);
	state_save_register_item("sh4", index, sh4.ssr);
	state_save_register_item("sh4", index, sh4.sgr);
	state_save_register_item("sh4", index, sh4.fpscr);
	state_save_register_item("sh4", index, sh4.r[ 0]);
	state_save_register_item("sh4", index, sh4.r[ 1]);
	state_save_register_item("sh4", index, sh4.r[ 2]);
	state_save_register_item("sh4", index, sh4.r[ 3]);
	state_save_register_item("sh4", index, sh4.r[ 4]);
	state_save_register_item("sh4", index, sh4.r[ 5]);
	state_save_register_item("sh4", index, sh4.r[ 6]);
	state_save_register_item("sh4", index, sh4.r[ 7]);
	state_save_register_item("sh4", index, sh4.r[ 8]);
	state_save_register_item("sh4", index, sh4.r[ 9]);
	state_save_register_item("sh4", index, sh4.r[10]);
	state_save_register_item("sh4", index, sh4.r[11]);
	state_save_register_item("sh4", index, sh4.r[12]);
	state_save_register_item("sh4", index, sh4.r[13]);
	state_save_register_item("sh4", index, sh4.r[14]);
	state_save_register_item("sh4", index, sh4.fr[ 0]);
	state_save_register_item("sh4", index, sh4.fr[ 1]);
	state_save_register_item("sh4", index, sh4.fr[ 2]);
	state_save_register_item("sh4", index, sh4.fr[ 3]);
	state_save_register_item("sh4", index, sh4.fr[ 4]);
	state_save_register_item("sh4", index, sh4.fr[ 5]);
	state_save_register_item("sh4", index, sh4.fr[ 6]);
	state_save_register_item("sh4", index, sh4.fr[ 7]);
	state_save_register_item("sh4", index, sh4.fr[ 8]);
	state_save_register_item("sh4", index, sh4.fr[ 9]);
	state_save_register_item("sh4", index, sh4.fr[10]);
	state_save_register_item("sh4", index, sh4.fr[11]);
	state_save_register_item("sh4", index, sh4.fr[12]);
	state_save_register_item("sh4", index, sh4.fr[13]);
	state_save_register_item("sh4", index, sh4.fr[14]);
	state_save_register_item("sh4", index, sh4.fr[15]);
	state_save_register_item("sh4", index, sh4.xf[ 0]);
	state_save_register_item("sh4", index, sh4.xf[ 1]);
	state_save_register_item("sh4", index, sh4.xf[ 2]);
	state_save_register_item("sh4", index, sh4.xf[ 3]);
	state_save_register_item("sh4", index, sh4.xf[ 4]);
	state_save_register_item("sh4", index, sh4.xf[ 5]);
	state_save_register_item("sh4", index, sh4.xf[ 6]);
	state_save_register_item("sh4", index, sh4.xf[ 7]);
	state_save_register_item("sh4", index, sh4.xf[ 8]);
	state_save_register_item("sh4", index, sh4.xf[ 9]);
	state_save_register_item("sh4", index, sh4.xf[10]);
	state_save_register_item("sh4", index, sh4.xf[11]);
	state_save_register_item("sh4", index, sh4.xf[12]);
	state_save_register_item("sh4", index, sh4.xf[13]);
	state_save_register_item("sh4", index, sh4.xf[14]);
	state_save_register_item("sh4", index, sh4.xf[15]);
	state_save_register_item("sh4", index, sh4.ea);
	state_save_register_item("sh4", index, sh4.fpul);
	state_save_register_item("sh4", index, sh4.dbr);
	state_save_register_item_array("sh4", index, sh4.exception_priority);
	state_save_register_item_array("sh4", index, sh4.exception_requesting);

}

static void sh4_dma_ddt(struct sh4_ddt_dma *s)
{
UINT32 chcr;
UINT32 *p32bits;
UINT32 pos,len,siz;

	if (sh4.dma_timer_active[s->channel])
		return;
	if (s->mode >= 0) {
		switch (s->channel)
		{
		case 0:
			if (s->mode & 1)
				s->source = sh4.m[SAR0];
			if (s->mode & 2)
				sh4.m[SAR0] = s->source;
			if (s->mode & 4)
				s->destination = sh4.m[DAR0];
			if (s->mode & 8)
				sh4.m[DAR0] = s->destination;
			break;
		case 1:
			if (s->mode & 1)
				s->source = sh4.m[SAR1];
			if (s->mode & 2)
				sh4.m[SAR1] = s->source;
			if (s->mode & 4)
				s->destination = sh4.m[DAR1];
			if (s->mode & 8)
				sh4.m[DAR1] = s->destination;
			break;
		case 2:
			if (s->mode & 1)
				s->source = sh4.m[SAR2];
			if (s->mode & 2)
				sh4.m[SAR2] = s->source;
			if (s->mode & 4)
				s->destination = sh4.m[DAR2];
			if (s->mode & 8)
				sh4.m[DAR2] = s->destination;
			break;
		case 3:
		default:
			if (s->mode & 1)
				s->source = sh4.m[SAR3];
			if (s->mode & 2)
				sh4.m[SAR3] = s->source;
			if (s->mode & 4)
				s->destination = sh4.m[DAR3];
			if (s->mode & 8)
				sh4.m[DAR3] = s->destination;
			break;
		}
		switch (s->channel)
		{
		case 0:
			chcr = sh4.m[CHCR0];
			len = sh4.m[DMATCR0];
			break;
		case 1:
			chcr = sh4.m[CHCR1];
			len = sh4.m[DMATCR1];
			break;
		case 2:
			chcr = sh4.m[CHCR2];
			len = sh4.m[DMATCR2];
			break;
		case 3:
		default:
			chcr = sh4.m[CHCR3];
			len = sh4.m[DMATCR3];
			break;
		}
		if ((s->direction) == 0) {
			chcr = (chcr & 0xffff3fff) | ((s->mode & 0x30) << 10);
		} else {
			chcr = (chcr & 0xffffcfff) | ((s->mode & 0x30) << 8);
		}
		siz = dmasize[(chcr >> 4) & 7];
		if (siz && (s->size))
			if ((len * siz) != (s->length * s->size))
				return;
		sh4_dma_transfer(s->channel, 0, chcr, &s->source, &s->destination, &len);
	} else {
		if ((s->direction) == 0) {
			len = s->length;
			p32bits = (UINT32 *)(s->buffer);
			for (pos = 0;pos < len;pos++) {
				*p32bits = program_read_dword_64le(s->source);
				p32bits++;
				s->source = s->source + 4;
			}
		} else {
			len = s->length;
			p32bits = (UINT32 *)(s->buffer);
			for (pos = 0;pos < len;pos++) {
				program_write_dword_64le(s->destination, *p32bits);
				p32bits++;
				s->destination = s->destination + 4;
			}
		}
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static void sh4_set_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + SH4_IRL0:		set_irq_line(SH4_IRL0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL1:		set_irq_line(SH4_IRL1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL2:		set_irq_line(SH4_IRL2, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL3:		set_irq_line(SH4_IRL3, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRLn:		set_irq_line(SH4_IRLn, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;

		case CPUINFO_INT_REGISTER + SH4_PC:
		case CPUINFO_INT_PC:							sh4.pc = info->i; sh4.delay = 0;		break;
		case CPUINFO_INT_SP:							sh4.r[15] = info->i;    				break;
		case CPUINFO_INT_REGISTER + SH4_PR:   			sh4.pr = info->i;	   					break;
		case CPUINFO_INT_REGISTER + SH4_SR:
			sh4.sr = info->i;
			sh4_exception_recompute();
			sh4_check_pending_irq("sh4_set_info");
			break;
		case CPUINFO_INT_REGISTER + SH4_GBR:			sh4.gbr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH4_VBR:			sh4.vbr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH4_DBR:			sh4.dbr = info->i;						break;
		case CPUINFO_INT_REGISTER + SH4_MACH: 			sh4.mach = info->i;						break;
		case CPUINFO_INT_REGISTER + SH4_MACL:			sh4.macl = info->i;						break;
		case CPUINFO_INT_REGISTER + SH4_R0:				sh4.r[ 0] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R1:				sh4.r[ 1] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R2:				sh4.r[ 2] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R3:				sh4.r[ 3] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R4:				sh4.r[ 4] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R5:				sh4.r[ 5] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R6:				sh4.r[ 6] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R7:				sh4.r[ 7] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R8:				sh4.r[ 8] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R9:				sh4.r[ 9] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R10:			sh4.r[10] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R11:			sh4.r[11] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R12:			sh4.r[12] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R13:			sh4.r[13] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R14:			sh4.r[14] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_R15:			sh4.r[15] = info->i;					break;
		case CPUINFO_INT_REGISTER + SH4_EA:				sh4.ea = info->i;						break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK0:			sh4.rbnk[0][0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK0:			sh4.rbnk[0][1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK0:			sh4.rbnk[0][2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK0:			sh4.rbnk[0][3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK0:			sh4.rbnk[0][4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK0:			sh4.rbnk[0][5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK0:			sh4.rbnk[0][6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK0:			sh4.rbnk[0][7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK1:			sh4.rbnk[1][0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK1:			sh4.rbnk[1][1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK1:			sh4.rbnk[1][2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK1:			sh4.rbnk[1][3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK1:			sh4.rbnk[1][4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK1:			sh4.rbnk[1][5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK1:			sh4.rbnk[1][6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK1:			sh4.rbnk[1][7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_SPC:			sh4.spc = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_SSR:			sh4.ssr = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_SGR:			sh4.sgr = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FPSCR:			sh4.fpscr = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FPUL:			sh4.fpul = info->i; break;
#ifdef LSB_FIRST
		case CPUINFO_STR_REGISTER + SH4_FR0:			sh4.fr[ 0 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR1:			sh4.fr[ 1 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR2:			sh4.fr[ 2 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR3:			sh4.fr[ 3 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR4:			sh4.fr[ 4 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR5:			sh4.fr[ 5 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR6:			sh4.fr[ 6 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR7:			sh4.fr[ 7 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR8:			sh4.fr[ 8 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR9:			sh4.fr[ 9 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR10:			sh4.fr[10 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR11:			sh4.fr[11 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR12:			sh4.fr[12 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR13:			sh4.fr[13 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR14:			sh4.fr[14 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR15:			sh4.fr[15 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF0:			sh4.xf[ 0 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF1:			sh4.xf[ 1 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF2:			sh4.xf[ 2 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF3:			sh4.xf[ 3 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF4:			sh4.xf[ 4 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF5:			sh4.xf[ 5 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF6:			sh4.xf[ 6 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF7:			sh4.xf[ 7 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF8:			sh4.xf[ 8 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF9:			sh4.xf[ 9 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF10:			sh4.xf[10 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF11:			sh4.xf[11 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF12:			sh4.xf[12 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF13:			sh4.xf[13 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF14:			sh4.xf[14 ^ sh4.fpu_pr] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF15:			sh4.xf[15 ^ sh4.fpu_pr] = info->i; break;
#else
		case CPUINFO_STR_REGISTER + SH4_FR0:			sh4.fr[ 0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR1:			sh4.fr[ 1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR2:			sh4.fr[ 2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR3:			sh4.fr[ 3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR4:			sh4.fr[ 4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR5:			sh4.fr[ 5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR6:			sh4.fr[ 6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR7:			sh4.fr[ 7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR8:			sh4.fr[ 8] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR9:			sh4.fr[ 9] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR10:			sh4.fr[10] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR11:			sh4.fr[11] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR12:			sh4.fr[12] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR13:			sh4.fr[13] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR14:			sh4.fr[14] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_FR15:			sh4.fr[15] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF0:			sh4.xf[ 0] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF1:			sh4.xf[ 1] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF2:			sh4.xf[ 2] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF3:			sh4.xf[ 3] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF4:			sh4.xf[ 4] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF5:			sh4.xf[ 5] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF6:			sh4.xf[ 6] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF7:			sh4.xf[ 7] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF8:			sh4.xf[ 8] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF9:			sh4.xf[ 9] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF10:			sh4.xf[10] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF11:			sh4.xf[11] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF12:			sh4.xf[12] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF13:			sh4.xf[13] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF14:			sh4.xf[14] = info->i; break;
		case CPUINFO_STR_REGISTER + SH4_XF15:			sh4.xf[15] = info->i; break;
#endif

		case CPUINFO_INT_SH4_IRLn_INPUT:				sh4_set_irln_input(cpu_getactivecpu(), info->i); break;
		case CPUINFO_INT_SH4_FRT_INPUT:					sh4_set_frt_input(cpu_getactivecpu(), info->i); break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_SH4_FTCSR_READ_CALLBACK:		sh4.ftcsr_read_callback = (void (*) (UINT32 ))info->f; break;
		case CPUINFO_PTR_SH4_EXTERNAL_DDT_DMA:			sh4_dma_ddt((struct sh4_ddt_dma *)info->f); break;
	}
}

#if 0
/*When OC index mode is off (CCR.OIX = 0)*/
static ADDRESS_MAP_START( sh4_internal_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x1C000000, 0x1C000FFF) AM_RAM AM_MIRROR(0x03FFD000)
	AM_RANGE(0x1C002000, 0x1C002FFF) AM_RAM AM_MIRROR(0x03FFD000)
	AM_RANGE(0xE0000000, 0xE000003F) AM_RAM AM_MIRROR(0x03FFFFC0)
ADDRESS_MAP_END
#endif

/*When OC index mode is on (CCR.OIX = 1)*/
static ADDRESS_MAP_START( sh4_internal_map, ADDRESS_SPACE_PROGRAM, 64 )
	AM_RANGE(0x1C000000, 0x1C000FFF) AM_RAM AM_MIRROR(0x01FFF000)
	AM_RANGE(0x1E000000, 0x1E000FFF) AM_RAM AM_MIRROR(0x01FFF000)
	AM_RANGE(0xE0000000, 0xE000003F) AM_RAM AM_MIRROR(0x03FFFFC0)
ADDRESS_MAP_END


/**************************************************************************
 * Generic get_info
 **************************************************************************/

void sh4_get_info(UINT32 state, cpuinfo *info)
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(sh4);				break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 5;						break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;						break;
		case CPUINFO_INT_ENDIANNESS:					info->i = CPU_IS_LE;				break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;						break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;						break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;						break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;						break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 4;						break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:		info->i = 64;				break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: 	info->i = 32;				break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: 	info->i = 0;				break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:		info->i = 0;				break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 		info->i = 0;				break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 		info->i = 0;				break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + ADDRESS_SPACE_PROGRAM: info->internal_map = construct_map_sh4_internal_map; break;

		case CPUINFO_INT_INPUT_STATE + SH4_IRL0:		info->i = sh4.irq_line_state[SH4_IRL0]; break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL1:		info->i = sh4.irq_line_state[SH4_IRL1]; break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL2:		info->i = sh4.irq_line_state[SH4_IRL2]; break;
		case CPUINFO_INT_INPUT_STATE + SH4_IRL3:		info->i = sh4.irq_line_state[SH4_IRL3]; break;
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = sh4.nmi_line_state;			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = sh4.ppc;						break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + SH4_PC:				info->i = (sh4.delay) ? (sh4.delay & AM) : (sh4.pc & AM); break;
		case CPUINFO_INT_SP:   							info->i = sh4.r[15];					break;
		case CPUINFO_INT_REGISTER + SH4_PR:				info->i = sh4.pr;						break;
		case CPUINFO_INT_REGISTER + SH4_SR:				info->i = sh4.sr;						break;
		case CPUINFO_INT_REGISTER + SH4_GBR:			info->i = sh4.gbr;						break;
		case CPUINFO_INT_REGISTER + SH4_VBR:			info->i = sh4.vbr;						break;
		case CPUINFO_INT_REGISTER + SH4_DBR:			info->i = sh4.dbr;						break;
		case CPUINFO_INT_REGISTER + SH4_MACH:			info->i = sh4.mach;						break;
		case CPUINFO_INT_REGISTER + SH4_MACL:			info->i = sh4.macl;						break;
		case CPUINFO_INT_REGISTER + SH4_R0:				info->i = sh4.r[ 0];					break;
		case CPUINFO_INT_REGISTER + SH4_R1:				info->i = sh4.r[ 1];					break;
		case CPUINFO_INT_REGISTER + SH4_R2:				info->i = sh4.r[ 2];					break;
		case CPUINFO_INT_REGISTER + SH4_R3:				info->i = sh4.r[ 3];					break;
		case CPUINFO_INT_REGISTER + SH4_R4:				info->i = sh4.r[ 4];					break;
		case CPUINFO_INT_REGISTER + SH4_R5:				info->i = sh4.r[ 5];					break;
		case CPUINFO_INT_REGISTER + SH4_R6:				info->i = sh4.r[ 6];					break;
		case CPUINFO_INT_REGISTER + SH4_R7:				info->i = sh4.r[ 7];					break;
		case CPUINFO_INT_REGISTER + SH4_R8:				info->i = sh4.r[ 8];					break;
		case CPUINFO_INT_REGISTER + SH4_R9:				info->i = sh4.r[ 9];					break;
		case CPUINFO_INT_REGISTER + SH4_R10:			info->i = sh4.r[10];					break;
		case CPUINFO_INT_REGISTER + SH4_R11:			info->i = sh4.r[11];					break;
		case CPUINFO_INT_REGISTER + SH4_R12:			info->i = sh4.r[12];					break;
		case CPUINFO_INT_REGISTER + SH4_R13:			info->i = sh4.r[13];					break;
		case CPUINFO_INT_REGISTER + SH4_R14:			info->i = sh4.r[14];					break;
		case CPUINFO_INT_REGISTER + SH4_R15:			info->i = sh4.r[15];					break;
		case CPUINFO_INT_REGISTER + SH4_EA:				info->i = sh4.ea;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = sh4_set_info;			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = sh4_get_context;		break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = sh4_set_context;		break;
		case CPUINFO_PTR_INIT:							info->init = sh4_init;					break;
		case CPUINFO_PTR_RESET:							info->reset = sh4_reset;				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = sh4_execute;			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
#ifdef MAME_DEBUG
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = sh4_dasm;			break;
#endif /* MAME_DEBUG */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &sh4_icount;				break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:						strcpy(info->s, "SH-4");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Hitachi SH7750");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "1.0");				break;
		case CPUINFO_STR_CORE_FILE:					strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright (c) 2007 R. Belmont"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%s%s%s%s%c%c%d%c%c",
					sh4.sr & MD ? "MD ":"   ",
					sh4.sr & sRB ? "RB ":"   ",
					sh4.sr & BL ? "BL ":"   ",
					sh4.sr & FD ? "FD ":"   ",
					sh4.sr & M ? 'M':'.',
					sh4.sr & Q ? 'Q':'.',
					(sh4.sr & I) >> 4,
					sh4.sr & S ? 'S':'.',
					sh4.sr & T ? 'T':'.');
			break;

		case CPUINFO_STR_REGISTER + SH4_PC:				sprintf(info->s, "PC  :%08X", sh4.pc); break;
		case CPUINFO_STR_REGISTER + SH4_SR:				sprintf(info->s, "SR  :%08X", sh4.sr); break;
		case CPUINFO_STR_REGISTER + SH4_PR:				sprintf(info->s, "PR  :%08X", sh4.pr); break;
		case CPUINFO_STR_REGISTER + SH4_GBR:			sprintf(info->s, "GBR :%08X", sh4.gbr); break;
		case CPUINFO_STR_REGISTER + SH4_VBR:			sprintf(info->s, "VBR :%08X", sh4.vbr); break;
		case CPUINFO_STR_REGISTER + SH4_DBR:			sprintf(info->s, "DBR :%08X", sh4.dbr); break;
		case CPUINFO_STR_REGISTER + SH4_MACH:			sprintf(info->s, "MACH:%08X", sh4.mach); break;
		case CPUINFO_STR_REGISTER + SH4_MACL:			sprintf(info->s, "MACL:%08X", sh4.macl); break;
		case CPUINFO_STR_REGISTER + SH4_R0:				sprintf(info->s, "R0  :%08X", sh4.r[ 0]); break;
		case CPUINFO_STR_REGISTER + SH4_R1:				sprintf(info->s, "R1  :%08X", sh4.r[ 1]); break;
		case CPUINFO_STR_REGISTER + SH4_R2:				sprintf(info->s, "R2  :%08X", sh4.r[ 2]); break;
		case CPUINFO_STR_REGISTER + SH4_R3:				sprintf(info->s, "R3  :%08X", sh4.r[ 3]); break;
		case CPUINFO_STR_REGISTER + SH4_R4:				sprintf(info->s, "R4  :%08X", sh4.r[ 4]); break;
		case CPUINFO_STR_REGISTER + SH4_R5:				sprintf(info->s, "R5  :%08X", sh4.r[ 5]); break;
		case CPUINFO_STR_REGISTER + SH4_R6:				sprintf(info->s, "R6  :%08X", sh4.r[ 6]); break;
		case CPUINFO_STR_REGISTER + SH4_R7:				sprintf(info->s, "R7  :%08X", sh4.r[ 7]); break;
		case CPUINFO_STR_REGISTER + SH4_R8:				sprintf(info->s, "R8  :%08X", sh4.r[ 8]); break;
		case CPUINFO_STR_REGISTER + SH4_R9:				sprintf(info->s, "R9  :%08X", sh4.r[ 9]); break;
		case CPUINFO_STR_REGISTER + SH4_R10:			sprintf(info->s, "R10 :%08X", sh4.r[10]); break;
		case CPUINFO_STR_REGISTER + SH4_R11:			sprintf(info->s, "R11 :%08X", sh4.r[11]); break;
		case CPUINFO_STR_REGISTER + SH4_R12:			sprintf(info->s, "R12 :%08X", sh4.r[12]); break;
		case CPUINFO_STR_REGISTER + SH4_R13:			sprintf(info->s, "R13 :%08X", sh4.r[13]); break;
		case CPUINFO_STR_REGISTER + SH4_R14:			sprintf(info->s, "R14 :%08X", sh4.r[14]); break;
		case CPUINFO_STR_REGISTER + SH4_R15:			sprintf(info->s, "R15 :%08X", sh4.r[15]); break;
		case CPUINFO_STR_REGISTER + SH4_EA:				sprintf(info->s, "EA  :%08X", sh4.ea);    break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK0:			sprintf(info->s, "R0 BK 0  :%08X", sh4.rbnk[0][0]); break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK0:			sprintf(info->s, "R1 BK 0 :%08X", sh4.rbnk[0][1]); break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK0:			sprintf(info->s, "R2 BK 0 :%08X", sh4.rbnk[0][2]); break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK0:			sprintf(info->s, "R3 BK 0 :%08X", sh4.rbnk[0][3]); break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK0:			sprintf(info->s, "R4 BK 0 :%08X", sh4.rbnk[0][4]); break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK0:			sprintf(info->s, "R5 BK 0 :%08X", sh4.rbnk[0][5]); break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK0:			sprintf(info->s, "R6 BK 0 :%08X", sh4.rbnk[0][6]); break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK0:			sprintf(info->s, "R7 BK 0 :%08X", sh4.rbnk[0][7]); break;
		case CPUINFO_STR_REGISTER + SH4_R0_BK1:			sprintf(info->s, "R0 BK 1 :%08X", sh4.rbnk[1][0]); break;
		case CPUINFO_STR_REGISTER + SH4_R1_BK1:			sprintf(info->s, "R1 BK 1 :%08X", sh4.rbnk[1][1]); break;
		case CPUINFO_STR_REGISTER + SH4_R2_BK1:			sprintf(info->s, "R2 BK 1 :%08X", sh4.rbnk[1][2]); break;
		case CPUINFO_STR_REGISTER + SH4_R3_BK1:			sprintf(info->s, "R3 BK 1 :%08X", sh4.rbnk[1][3]); break;
		case CPUINFO_STR_REGISTER + SH4_R4_BK1:			sprintf(info->s, "R4 BK 1 :%08X", sh4.rbnk[1][4]); break;
		case CPUINFO_STR_REGISTER + SH4_R5_BK1:			sprintf(info->s, "R5 BK 1 :%08X", sh4.rbnk[1][5]); break;
		case CPUINFO_STR_REGISTER + SH4_R6_BK1:			sprintf(info->s, "R6 BK 1 :%08X", sh4.rbnk[1][6]); break;
		case CPUINFO_STR_REGISTER + SH4_R7_BK1:			sprintf(info->s, "R7 BK 1 :%08X", sh4.rbnk[1][7]); break;
		case CPUINFO_STR_REGISTER + SH4_SPC:			sprintf(info->s, "SPC  :%08X", sh4.spc); break;
		case CPUINFO_STR_REGISTER + SH4_SSR:			sprintf(info->s, "SSR  :%08X", sh4.ssr); break;
		case CPUINFO_STR_REGISTER + SH4_SGR:			sprintf(info->s, "SGR  :%08X", sh4.sgr); break;
		case CPUINFO_STR_REGISTER + SH4_FPSCR:			sprintf(info->s, "FPSCR :%08X", sh4.fpscr); break;
		case CPUINFO_STR_REGISTER + SH4_FPUL:			sprintf(info->s, "FPUL :%08X", sh4.fpul); break;
#ifdef LSB_FIRST
		case CPUINFO_STR_REGISTER + SH4_FR0:			sprintf(info->s, "FR0  :%08X %01.4e", FP_RS2( 0),(double)FP_RFS2( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_FR1:			sprintf(info->s, "FR1  :%08X %01.2e", FP_RS2( 1),(double)FP_RFS2( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_FR2:			sprintf(info->s, "FR2  :%08X %01.2e", FP_RS2( 2),(double)FP_RFS2( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_FR3:			sprintf(info->s, "FR3  :%08X %01.2e", FP_RS2( 3),(double)FP_RFS2( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_FR4:			sprintf(info->s, "FR4  :%08X %01.2e", FP_RS2( 4),(double)FP_RFS2( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_FR5:			sprintf(info->s, "FR5  :%08X %01.2e", FP_RS2( 5),(double)FP_RFS2( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_FR6:			sprintf(info->s, "FR6  :%08X %01.2e", FP_RS2( 6),(double)FP_RFS2( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_FR7:			sprintf(info->s, "FR7  :%08X %01.2e", FP_RS2( 7),(double)FP_RFS2( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_FR8:			sprintf(info->s, "FR8  :%08X %01.2e", FP_RS2( 8),(double)FP_RFS2( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_FR9:			sprintf(info->s, "FR9  :%08X %01.2e", FP_RS2( 9),(double)FP_RFS2( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_FR10:			sprintf(info->s, "FR10 :%08X %01.2e", FP_RS2(10),(double)FP_RFS2(10)); break;
		case CPUINFO_STR_REGISTER + SH4_FR11:			sprintf(info->s, "FR11 :%08X %01.2e", FP_RS2(11),(double)FP_RFS2(11)); break;
		case CPUINFO_STR_REGISTER + SH4_FR12:			sprintf(info->s, "FR12 :%08X %01.2e", FP_RS2(12),(double)FP_RFS2(12)); break;
		case CPUINFO_STR_REGISTER + SH4_FR13:			sprintf(info->s, "FR13 :%08X %01.2e", FP_RS2(13),(double)FP_RFS2(13)); break;
		case CPUINFO_STR_REGISTER + SH4_FR14:			sprintf(info->s, "FR14 :%08X %01.2e", FP_RS2(14),(double)FP_RFS2(14)); break;
		case CPUINFO_STR_REGISTER + SH4_FR15:			sprintf(info->s, "FR15 :%08X %01.2e", FP_RS2(15),(double)FP_RFS2(15)); break;
		case CPUINFO_STR_REGISTER + SH4_XF0:			sprintf(info->s, "XF0  :%08X %01.2e", FP_XS2( 0),(double)FP_XFS2( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_XF1:			sprintf(info->s, "XF1  :%08X %01.2e", FP_XS2( 1),(double)FP_XFS2( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_XF2:			sprintf(info->s, "XF2  :%08X %01.2e", FP_XS2( 2),(double)FP_XFS2( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_XF3:			sprintf(info->s, "XF3  :%08X %01.2e", FP_XS2( 3),(double)FP_XFS2( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_XF4:			sprintf(info->s, "XF4  :%08X %01.2e", FP_XS2( 4),(double)FP_XFS2( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_XF5:			sprintf(info->s, "XF5  :%08X %01.2e", FP_XS2( 5),(double)FP_XFS2( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_XF6:			sprintf(info->s, "XF6  :%08X %01.2e", FP_XS2( 6),(double)FP_XFS2( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_XF7:			sprintf(info->s, "XF7  :%08X %01.2e", FP_XS2( 7),(double)FP_XFS2( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_XF8:			sprintf(info->s, "XF8  :%08X %01.2e", FP_XS2( 8),(double)FP_XFS2( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_XF9:			sprintf(info->s, "XF9  :%08X %01.2e", FP_XS2( 9),(double)FP_XFS2( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_XF10:			sprintf(info->s, "XF10 :%08X %01.2e", FP_XS2(10),(double)FP_XFS2(10)); break;
		case CPUINFO_STR_REGISTER + SH4_XF11:			sprintf(info->s, "XF11 :%08X %01.2e", FP_XS2(11),(double)FP_XFS2(11)); break;
		case CPUINFO_STR_REGISTER + SH4_XF12:			sprintf(info->s, "XF12 :%08X %01.2e", FP_XS2(12),(double)FP_XFS2(12)); break;
		case CPUINFO_STR_REGISTER + SH4_XF13:			sprintf(info->s, "XF13 :%08X %01.2e", FP_XS2(13),(double)FP_XFS2(13)); break;
		case CPUINFO_STR_REGISTER + SH4_XF14:			sprintf(info->s, "XF14 :%08X %01.2e", FP_XS2(14),(double)FP_XFS2(14)); break;
		case CPUINFO_STR_REGISTER + SH4_XF15:			sprintf(info->s, "XF15 :%08X %01.2e", FP_XS2(15),(double)FP_XFS2(15)); break;
#else
		case CPUINFO_STR_REGISTER + SH4_FR0:			sprintf(info->s, "FR0  :%08X %01.4e", FP_RS( 0),(double)FP_RFS( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_FR1:			sprintf(info->s, "FR1  :%08X %01.2e", FP_RS( 1),(double)FP_RFS( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_FR2:			sprintf(info->s, "FR2  :%08X %01.2e", FP_RS( 2),(double)FP_RFS( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_FR3:			sprintf(info->s, "FR3  :%08X %01.2e", FP_RS( 3),(double)FP_RFS( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_FR4:			sprintf(info->s, "FR4  :%08X %01.2e", FP_RS( 4),(double)FP_RFS( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_FR5:			sprintf(info->s, "FR5  :%08X %01.2e", FP_RS( 5),(double)FP_RFS( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_FR6:			sprintf(info->s, "FR6  :%08X %01.2e", FP_RS( 6),(double)FP_RFS( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_FR7:			sprintf(info->s, "FR7  :%08X %01.2e", FP_RS( 7),(double)FP_RFS( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_FR8:			sprintf(info->s, "FR8  :%08X %01.2e", FP_RS( 8),(double)FP_RFS( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_FR9:			sprintf(info->s, "FR9  :%08X %01.2e", FP_RS( 9),(double)FP_RFS( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_FR10:			sprintf(info->s, "FR10 :%08X %01.2e", FP_RS(10),(double)FP_RFS(10)); break;
		case CPUINFO_STR_REGISTER + SH4_FR11:			sprintf(info->s, "FR11 :%08X %01.2e", FP_RS(11),(double)FP_RFS(11)); break;
		case CPUINFO_STR_REGISTER + SH4_FR12:			sprintf(info->s, "FR12 :%08X %01.2e", FP_RS(12),(double)FP_RFS(12)); break;
		case CPUINFO_STR_REGISTER + SH4_FR13:			sprintf(info->s, "FR13 :%08X %01.2e", FP_RS(13),(double)FP_RFS(13)); break;
		case CPUINFO_STR_REGISTER + SH4_FR14:			sprintf(info->s, "FR14 :%08X %01.2e", FP_RS(14),(double)FP_RFS(14)); break;
		case CPUINFO_STR_REGISTER + SH4_FR15:			sprintf(info->s, "FR15 :%08X %01.2e", FP_RS(15),(double)FP_RFS(15)); break;
		case CPUINFO_STR_REGISTER + SH4_XF0:			sprintf(info->s, "XF0  :%08X %01.2e", FP_XS( 0),(double)FP_XFS( 0)); break;
		case CPUINFO_STR_REGISTER + SH4_XF1:			sprintf(info->s, "XF1  :%08X %01.2e", FP_XS( 1),(double)FP_XFS( 1)); break;
		case CPUINFO_STR_REGISTER + SH4_XF2:			sprintf(info->s, "XF2  :%08X %01.2e", FP_XS( 2),(double)FP_XFS( 2)); break;
		case CPUINFO_STR_REGISTER + SH4_XF3:			sprintf(info->s, "XF3  :%08X %01.2e", FP_XS( 3),(double)FP_XFS( 3)); break;
		case CPUINFO_STR_REGISTER + SH4_XF4:			sprintf(info->s, "XF4  :%08X %01.2e", FP_XS( 4),(double)FP_XFS( 4)); break;
		case CPUINFO_STR_REGISTER + SH4_XF5:			sprintf(info->s, "XF5  :%08X %01.2e", FP_XS( 5),(double)FP_XFS( 5)); break;
		case CPUINFO_STR_REGISTER + SH4_XF6:			sprintf(info->s, "XF6  :%08X %01.2e", FP_XS( 6),(double)FP_XFS( 6)); break;
		case CPUINFO_STR_REGISTER + SH4_XF7:			sprintf(info->s, "XF7  :%08X %01.2e", FP_XS( 7),(double)FP_XFS( 7)); break;
		case CPUINFO_STR_REGISTER + SH4_XF8:			sprintf(info->s, "XF8  :%08X %01.2e", FP_XS( 8),(double)FP_XFS( 8)); break;
		case CPUINFO_STR_REGISTER + SH4_XF9:			sprintf(info->s, "XF9  :%08X %01.2e", FP_XS( 9),(double)FP_XFS( 9)); break;
		case CPUINFO_STR_REGISTER + SH4_XF10:			sprintf(info->s, "XF10 :%08X %01.2e", FP_XS(10),(double)FP_XFS(10)); break;
		case CPUINFO_STR_REGISTER + SH4_XF11:			sprintf(info->s, "XF11 :%08X %01.2e", FP_XS(11),(double)FP_XFS(11)); break;
		case CPUINFO_STR_REGISTER + SH4_XF12:			sprintf(info->s, "XF12 :%08X %01.2e", FP_XS(12),(double)FP_XFS(12)); break;
		case CPUINFO_STR_REGISTER + SH4_XF13:			sprintf(info->s, "XF13 :%08X %01.2e", FP_XS(13),(double)FP_XFS(13)); break;
		case CPUINFO_STR_REGISTER + SH4_XF14:			sprintf(info->s, "XF14 :%08X %01.2e", FP_XS(14),(double)FP_XFS(14)); break;
		case CPUINFO_STR_REGISTER + SH4_XF15:			sprintf(info->s, "XF15 :%08X %01.2e", FP_XS(15),(double)FP_XFS(15)); break;
#endif
		case CPUINFO_PTR_SH4_FTCSR_READ_CALLBACK:		info->f = (genf*)sh4.ftcsr_read_callback; break;

	}
}
