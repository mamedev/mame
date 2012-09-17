/*****************************************************************************
 *
 *   sh4comn.c
 *
 *   SH-4 non-specific components
 *
 *****************************************************************************/

#include "emu.h"
#include "debugger.h"
#include "sh4.h"
#include "sh4regs.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"
#include "sh4dmac.h"

static const int rtcnt_div[8] = { 0, 4, 16, 64, 256, 1024, 2048, 4096 };
static const int daysmonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



static const UINT32 exception_priority_default[] = {
	EXPPRI(1,1,0,0),			 /* Power-on Reset */
	EXPPRI(1,2,0,1),			 /* Manual Reset */
	EXPPRI(1,1,0,2),			 /* H-UDI Reset */
	EXPPRI(1,3,0,3),			 /* Inst TLB Multiple Hit */
	EXPPRI(1,4,0,4),			 /* Data TLB Multiple Hit */

	EXPPRI(2,0,0,5),			/* User break Before Instruction */
	EXPPRI(2,1,0,6),			/* Inst Address Error */
	EXPPRI(2,2,0,7),			/* Inst TLB Miss */
	EXPPRI(2,3,0,8),			/* Inst TLB Protection Violation */
	EXPPRI(2,4,0,9),			/* Illegal Instruction */
	EXPPRI(2,4,0,10),			/* Slot Illegal Instruction */
	EXPPRI(2,4,0,11),			/* FPU Disable */
	EXPPRI(2,4,0,12),			/* Slot FPU Disable */
	EXPPRI(2,5,0,13),			/* Data Address Error (Read) */
	EXPPRI(2,5,0,14),			/* Data Address Error (Write) */
	EXPPRI(2,6,0,15),			/* Data TBL Miss Read */
	EXPPRI(2,6,0,16),			/* Data TBL Miss Write */
	EXPPRI(2,7,0,17),			/* Data TBL Protection Violation Read */
	EXPPRI(2,7,0,18),			/* Data TBL Protection Violation Write */
	EXPPRI(2,8,0,19),			/* FPU Exception */
	EXPPRI(2,9,0,20),			/* Initial Page Write exception */

	EXPPRI(2,4,0,21),			/* Unconditional TRAP */
	EXPPRI(2,10,0,22),			/* User break After Instruction */

	EXPPRI(3,0,16,SH4_INTC_NMI)	/* NMI */
	/* This is copied to a table, and the IRQ priorities filled in later */
};

static const int exception_codes[] =

{ 0x000, /* Power-on Reset */
  0x020, /* Manual Reset */
  0x000, /* H-UDI Reset */
  0x140, /* Inst TLB Multiple Hit */
  0x140, /* Data TLB Multiple Hit */

  0x1E0, /* User break Before Instruction */
  0x0E0, /* Inst Address Error */
  0x040, /* Inst TLB Miss */
  0x0A0, /* Inst TLB Protection Violation */
  0x180, /* Illegal Instruction */
  0x1A0, /* Slot Illegal Instruction */
  0x800, /* FPU Disable */
  0x820, /* Slot FPU Disable */
  0x0E0, /* Data Address Error (Read) */
  0x100, /* Data Address Error (Write) */
  0x040, /* Data TBL Miss Read */
  0x060, /* Data TBL Miss Write */
  0x0A0, /* Data TBL Protection Violation Read */
  0x0C0, /* Data TBL Protection Violation Write */
  0x120, /* FPU Exception */
  0x080, /* Initial Page Write exception */

  0x160, /* Unconditional TRAP */
  0x1E0, /* User break After Instruction */

  0x1C0, /* NMI */     /* SH4_INTC_NMI=23 represents this location in this list.. */

  0x200, /* EX Irq 0 */
  0x220, /*        1 */
  0x240, /*        2 */
  0x260, /*        3 */
  0x280, /*        4 */
  0x2A0, /*        5 */
  0x2C0, /*        6 */
  0x2E0, /*        7 */
  0x300, /*        8 */
  0x320, /*        9 */
  0x340, /*        A */
  0x360, /*        B */
  0x380, /*        C */
  0x3A0, /*        D */
  0x3C0, /*        E */

  0x240, /* SH4_INTC_IRL0 */
  0x2A0, /* SH4_INTC_IRL1 */
  0x300, /* SH4_INTC_IRL2 */
  0x360, /* SH4_INTC_IRL3 */

  0x600, /* HUDI */
  0x620, /* SH4_INTC_GPOI */
  0x640, /* SH4_INTC_DMTE0 */
  0x660, /* SH4_INTC_DMTE1 */
  0x680, /* SH4_INTC_DMTE2 */
  0x6A0, /* SH4_INTC_DMTE3 */

  0x780, /* SH4_INTC_DMTE4 */
  0x7A0, /* SH4_INTC_DMTE5 */
  0x7C0, /* SH4_INTC_DMTE6 */
  0x7E0, /* SH4_INTC_DMTE7 */

  0x6C0, /* SH4_INTC_DMAE */

  0xB00, /* SH4_INTC_TUNI3 */
  0xB80, /* SH4_INTC_TUNI4 */
  0x400, /* SH4_INTC_TUNI0 */
  0x420, /* SH4_INTC_TUNI1 */
  0x440, /* SH4_INTC_TUNI2 */
  0x460, /* SH4_INTC_TICPI2 */
  0x480, /* SH4_INTC_ATI */
  0x4A0, /* SH4_INTC_PRI */
  0x4C0, /* SH4_INTC_CUI */
  0x4E0, /* SH4_INTC_SCI1ERI */
  0x500, /* SH4_INTC_SCI1RXI */
  0x520, /* SH4_INTC_SCI1TXI */
  0x540, /* SH4_INTC_SCI1TEI */

  0x700, /* SH4_INTC_SCIFERI */
  0x720, /* SH4_INTC_SCIFRXI */
  0x740, /* SH4_INTC_SCIFBRI */
  0x760, /* SH4_INTC_SCIFTXI */
  0x560, /* SH4_INTC_ITI */
  0x580, /* SH4_INTC_RCMI */
  0x5A0 /* SH4_INTC_ROVI */
};

/* SH3 INTEVT2 uses a different table - values of -1 aren't filled in yet, some may not exist on the sh3. */
/* The above table should differ too, some things depend on the interrupt level rather than beign fixed values */

static const int sh3_intevt2_exception_codes[] =

{ 0x000, /* Power-on Reset */
  -1, /* Manual Reset */
  -1, /* H-UDI Reset */
  -1, /* Inst TLB Multiple Hit */
  -1, /* Data TLB Multiple Hit */

  -1, /* User break Before Instruction */
  -1, /* Inst Address Error */
  -1, /* Inst TLB Miss */
  -1, /* Inst TLB Protection Violation */
  -1, /* Illegal Instruction */
  -1, /* Slot Illegal Instruction */
  -1, /* FPU Disable */
  -1, /* Slot FPU Disable */
  -1, /* Data Address Error (Read) */
  -1, /* Data Address Error (Write) */
  -1, /* Data TBL Miss Read */
  -1, /* Data TBL Miss Write */
  -1, /* Data TBL Protection Violation Read */
  -1, /* Data TBL Protection Violation Write */
  -1, /* FPU Exception */
  -1, /* Initial Page Write exception */

  -1, /* Unconditional TRAP */
  -1, /* User break After Instruction */

  -1, /* NMI */     /* SH4_INTC_NMI=23 represents this location in this list.. */

  -1, /* EX Irq 0 */
  -1, /*        1 */
  -1, /*        2 */
  -1, /*        3 */
  -1, /*        4 */
  -1, /*        5 */
  -1, /*        6 */
  -1, /*        7 */
  -1, /*        8 */
  -1, /*        9 */
  -1, /*        A */
  -1, /*        B */
  -1, /*        C */
  -1, /*        D */
  -1, /*        E */

  0x600, /* SH4_INTC_IRL0 */
  0x620, /* SH4_INTC_IRL1 */
  0x640, /* SH4_INTC_IRL2 */
  0x660, /* SH4_INTC_IRL3 */
    /* todo: SH3 should have lines 4+5 too? */

  -1, /* HUDI */
  -1, /* SH4_INTC_GPOI */
  -1, /* SH4_INTC_DMTE0 */
  -1, /* SH4_INTC_DMTE1 */
  -1, /* SH4_INTC_DMTE2 */
  -1, /* SH4_INTC_DMTE3 */

  -1, /* SH4_INTC_DMTE4 */
  -1, /* SH4_INTC_DMTE5 */
  -1, /* SH4_INTC_DMTE6 */
  -1, /* SH4_INTC_DMTE7 */

  -1, /* SH4_INTC_DMAE */

  -1, /* SH4_INTC_TUNI3 */
  -1, /* SH4_INTC_TUNI4 */
  0x400, /* SH4_INTC_TUNI0 */
  0x420, /* SH4_INTC_TUNI1 */
  0x440, /* SH4_INTC_TUNI2 */
  0x460, /* SH4_INTC_TICPI2 */
  -1, /* SH4_INTC_ATI */
  -1, /* SH4_INTC_PRI */
  -1, /* SH4_INTC_CUI */
  -1, /* SH4_INTC_SCI1ERI */
  -1, /* SH4_INTC_SCI1RXI */
  -1, /* SH4_INTC_SCI1TXI */
  -1, /* SH4_INTC_SCI1TEI */

  -1, /* SH4_INTC_SCIFERI */
  -1, /* SH4_INTC_SCIFRXI */
  -1, /* SH4_INTC_SCIFBRI */
  -1, /* SH4_INTC_SCIFTXI */
  -1, /* SH4_INTC_ITI */
  -1, /* SH4_INTC_RCMI */
  -1 /* SH4_INTC_ROVI */
};



void sh4_change_register_bank(sh4_state *sh4, int to)
{
int s;

	if (to) // 0 -> 1
	{
		for (s = 0;s < 8;s++)
		{
			sh4->rbnk[0][s] = sh4->r[s];
			sh4->r[s] = sh4->rbnk[1][s];
		}
	}
	else // 1 -> 0
	{
		for (s = 0;s < 8;s++)
		{
			sh4->rbnk[1][s] = sh4->r[s];
			sh4->r[s] = sh4->rbnk[0][s];
		}
	}
}

void sh4_swap_fp_registers(sh4_state *sh4)
{
int s;
UINT32 z;

	for (s = 0;s <= 15;s++)
	{
		z = sh4->fr[s];
		sh4->fr[s] = sh4->xf[s];
		sh4->xf[s] = z;
	}
}

#ifdef LSB_FIRST
void sh4_swap_fp_couples(sh4_state *sh4)
{
int s;
UINT32 z;

	for (s = 0;s <= 15;s = s+2)
	{
		z = sh4->fr[s];
		sh4->fr[s] = sh4->fr[s + 1];
		sh4->fr[s + 1] = z;
		z = sh4->xf[s];
		sh4->xf[s] = sh4->xf[s + 1];
		sh4->xf[s + 1] = z;
	}
}
#endif

void sh4_syncronize_register_bank(sh4_state *sh4, int to)
{
int s;

	for (s = 0;s < 8;s++)
	{
		sh4->rbnk[to][s] = sh4->r[s];
	}
}

void sh4_default_exception_priorities(sh4_state *sh4) // setup default priorities for exceptions
{
int a;

	for (a=0;a <= SH4_INTC_NMI;a++)
		sh4->exception_priority[a] = exception_priority_default[a];
	for (a=SH4_INTC_IRLn0;a <= SH4_INTC_IRLnE;a++)
		sh4->exception_priority[a] = INTPRI(15-(a-SH4_INTC_IRLn0), a);
	sh4->exception_priority[SH4_INTC_IRL0] = INTPRI(13, SH4_INTC_IRL0);
	sh4->exception_priority[SH4_INTC_IRL1] = INTPRI(10, SH4_INTC_IRL1);
	sh4->exception_priority[SH4_INTC_IRL2] = INTPRI(7, SH4_INTC_IRL2);
	sh4->exception_priority[SH4_INTC_IRL3] = INTPRI(4, SH4_INTC_IRL3);
	for (a=SH4_INTC_HUDI;a <= SH4_INTC_ROVI;a++)
		sh4->exception_priority[a] = INTPRI(0, a);
}

void sh4_exception_recompute(sh4_state *sh4) // checks if there is any interrupt with high enough priority
{
	int a,z;

	sh4->test_irq = 0;
	if ((!sh4->pending_irq) || ((sh4->sr & BL) && (sh4->exception_requesting[SH4_INTC_NMI] == 0)))
		return;
	z = (sh4->sr >> 4) & 15;
	for (a=0;a <= SH4_INTC_ROVI;a++)
	{
		if (sh4->exception_requesting[a])
		{
			int pri = (((int)sh4->exception_priority[a] >> 8) & 255);
			//logerror("pri is %02x z is %02x\n",pri,z);
			if (pri > z)
			{
				//logerror("will test\n");
				sh4->test_irq = 1; // will check for exception at end of instructions
				break;
			}
		}
	}
}

void sh4_exception_request(sh4_state *sh4, int exception) // start requesting an exception
{
	//logerror("sh4_exception_request a\n");
	if (!sh4->exception_requesting[exception])
	{
		//logerror("sh4_exception_request b\n");
		sh4->exception_requesting[exception] = 1;
		sh4->pending_irq++;
		sh4_exception_recompute(sh4);
	}
}

void sh4_exception_unrequest(sh4_state *sh4, int exception) // stop requesting an exception
{
	if (sh4->exception_requesting[exception])
	{
		sh4->exception_requesting[exception] = 0;
		sh4->pending_irq--;
		sh4_exception_recompute(sh4);
	}
}

void sh4_exception_checkunrequest(sh4_state *sh4, int exception)
{
	if (exception == SH4_INTC_NMI)
		sh4_exception_unrequest(sh4, exception);
	if ((exception == SH4_INTC_DMTE0) || (exception == SH4_INTC_DMTE1) ||
		(exception == SH4_INTC_DMTE2) || (exception == SH4_INTC_DMTE3))
		sh4_exception_unrequest(sh4, exception);
}

void sh4_exception(sh4_state *sh4, const char *message, int exception) // handle exception
{
	UINT32 vector;


	if (sh4->cpu_type == CPU_TYPE_SH4)
	{
		if (exception < SH4_INTC_NMI)
			return; // Not yet supported
		if (exception == SH4_INTC_NMI) {
			if ((sh4->sr & BL) && (!(sh4->m[ICR] & 0x200)))
				return;

			sh4->m[ICR] &= ~0x200;
			sh4->m[INTEVT] = 0x1c0;


			vector = 0x600;
			sh4->irq_callback(sh4->device, INPUT_LINE_NMI);
			LOG(("SH-4 '%s' nmi exception after [%s]\n", sh4->device->tag(), message));
		} else {
	//      if ((sh4->m[ICR] & 0x4000) && (sh4->nmi_line_state == ASSERT_LINE))
	//          return;
			if (sh4->sr & BL)
				return;
			if (((sh4->exception_priority[exception] >> 8) & 255) <= ((sh4->sr >> 4) & 15))
				return;
			sh4->m[INTEVT] = exception_codes[exception];
			vector = 0x600;
			if ((exception >= SH4_INTC_IRL0) && (exception <= SH4_INTC_IRL3))
				sh4->irq_callback(sh4->device, SH4_INTC_IRL0-exception+SH4_IRL0);
			else
				sh4->irq_callback(sh4->device, SH4_IRL3+1);
			LOG(("SH-4 '%s' interrupt exception #%d after [%s]\n", sh4->device->tag(), exception, message));
		}
	}
	else /* SH3 exceptions */
	{
		/***** ASSUME THIS TO BE WRONG FOR NOW *****/

		if (exception < SH4_INTC_NMI)
			return; // Not yet supported
		if (exception == SH4_INTC_NMI)
		{
			return;
		}
		else
		{

			if (sh4->sr & BL)
				return;
			if (((sh4->exception_priority[exception] >> 8) & 255) <= ((sh4->sr >> 4) & 15))
				return;


			vector = 0x600;

			if ((exception >= SH4_INTC_IRL0) && (exception <= SH4_INTC_IRL3))
				sh4->irq_callback(sh4->device, SH4_INTC_IRL0-exception+SH4_IRL0);
			else
				sh4->irq_callback(sh4->device, SH4_IRL3+1);

			if (sh3_intevt2_exception_codes[exception]==-1)
				fatalerror("sh3_intevt2_exception_codes unpopulated for exception %02x\n", exception);

			sh4->m_sh3internal_lower[INTEVT2] = sh3_intevt2_exception_codes[exception];
			sh4->m_sh3internal_upper[SH3_EXPEVT_ADDR] = exception_codes[exception];


			LOG(("SH-3 '%s' interrupt exception #%d after [%s]\n", sh4->device->tag(), exception, message));
		}

		/***** END ASSUME THIS TO BE WRONG FOR NOW *****/
	}
	sh4_exception_checkunrequest(sh4, exception);

	sh4->spc = sh4->pc;
	sh4->ssr = sh4->sr;
	sh4->sgr = sh4->r[15];

	sh4->sr |= MD;
	if ((sh4->device->machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank(sh4, (sh4->sr & sRB) >> 29);
	if (!(sh4->sr & sRB))
		sh4_change_register_bank(sh4, 1);
	sh4->sr |= sRB;
	sh4->sr |= BL;
	sh4_exception_recompute(sh4);

	/* fetch PC */
	sh4->pc = sh4->vbr + vector;
	/* wake up if a sleep opcode is triggered */
	if(sh4->sleep_mode == 1) { sh4->sleep_mode = 2; }
}


static UINT32 compute_ticks_refresh_timer(emu_timer *timer, int hertz, int base, int divisor)
{
	// elapsed:total = x : ticks
	// x=elapsed*tics/total -> x=elapsed*(double)100000000/rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]
	// ticks/total=ticks / ((rtcnt_div[(sh4->m[RTCSR] >> 3) & 7] * ticks) / 100000000)=1/((rtcnt_div[(sh4->m[RTCSR] >> 3) & 7] / 100000000)=100000000/rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]
	return base + (UINT32)((timer->elapsed().as_double() * (double)hertz) / (double)divisor);
}

static void sh4_refresh_timer_recompute(sh4_state *sh4)
{
UINT32 ticks;

	if (sh4->cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_refresh_timer_recompute uses sh4->m[] with SH3\n");


	//if rtcnt < rtcor then rtcor-rtcnt
	//if rtcnt >= rtcor then 256-rtcnt+rtcor=256+rtcor-rtcnt
	ticks = sh4->m[RTCOR]-sh4->m[RTCNT];
	if (ticks <= 0)
		ticks = 256 + ticks;
	sh4->refresh_timer->adjust(attotime::from_hz(sh4->bus_clock) * rtcnt_div[(sh4->m[RTCSR] >> 3) & 7] * ticks);
	sh4->refresh_timer_base = sh4->m[RTCNT];
}


static TIMER_CALLBACK( sh4_refresh_timer_callback )
{
	sh4_state *sh4 = (sh4_state *)ptr;

	if (sh4->cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_refresh_timer_callback uses sh4->m[] with SH3\n");

	sh4->m[RTCNT] = 0;
	sh4_refresh_timer_recompute(sh4);
	sh4->m[RTCSR] |= 128;
	if ((sh4->m[MCR] & 4) && !(sh4->m[MCR] & 2))
	{
		sh4->m[RFCR] = (sh4->m[RFCR] + 1) & 1023;
		if (((sh4->m[RTCSR] & 1) && (sh4->m[RFCR] == 512)) || (sh4->m[RFCR] == 0))
		{
			sh4->m[RFCR] = 0;
			sh4->m[RTCSR] |= 4;
		}
	}
}

static void increment_rtc_time(sh4_state *sh4, int mode)
{
	int carry, year, leap, days;

	if (sh4->cpu_type != CPU_TYPE_SH4)
		fatalerror("increment_rtc_time uses sh4->m[] with SH3\n");

	if (mode == 0)
	{
		carry = 0;
		sh4->m[RSECCNT] = sh4->m[RSECCNT] + 1;
		if ((sh4->m[RSECCNT] & 0xf) == 0xa)
			sh4->m[RSECCNT] = sh4->m[RSECCNT] + 6;
		if (sh4->m[RSECCNT] == 0x60)
		{
			sh4->m[RSECCNT] = 0;
			carry=1;
		}
		else
			return;
	}
	else
		carry = 1;

	sh4->m[RMINCNT] = sh4->m[RMINCNT] + carry;
	if ((sh4->m[RMINCNT] & 0xf) == 0xa)
		sh4->m[RMINCNT] = sh4->m[RMINCNT] + 6;
	carry=0;
	if (sh4->m[RMINCNT] == 0x60)
	{
		sh4->m[RMINCNT] = 0;
		carry = 1;
	}

	sh4->m[RHRCNT] = sh4->m[RHRCNT] + carry;
	if ((sh4->m[RHRCNT] & 0xf) == 0xa)
		sh4->m[RHRCNT] = sh4->m[RHRCNT] + 6;
	carry = 0;
	if (sh4->m[RHRCNT] == 0x24)
	{
		sh4->m[RHRCNT] = 0;
		carry = 1;
	}

	sh4->m[RWKCNT] = sh4->m[RWKCNT] + carry;
	if (sh4->m[RWKCNT] == 0x7)
	{
		sh4->m[RWKCNT] = 0;
	}

	days = 0;
	year = (sh4->m[RYRCNT] & 0xf) + ((sh4->m[RYRCNT] & 0xf0) >> 4)*10 + ((sh4->m[RYRCNT] & 0xf00) >> 8)*100 + ((sh4->m[RYRCNT] & 0xf000) >> 12)*1000;
	leap = 0;
	if (!(year%100))
	{
		if (!(year%400))
			leap = 1;
	}
	else if (!(year%4))
		leap = 1;
	if (sh4->m[RMONCNT] != 2)
		leap = 0;
	if (sh4->m[RMONCNT])
		days = daysmonth[(sh4->m[RMONCNT] & 0xf) + ((sh4->m[RMONCNT] & 0xf0) >> 4)*10 - 1];

	sh4->m[RDAYCNT] = sh4->m[RDAYCNT] + carry;
	if ((sh4->m[RDAYCNT] & 0xf) == 0xa)
		sh4->m[RDAYCNT] = sh4->m[RDAYCNT] + 6;
	carry = 0;
	if (sh4->m[RDAYCNT] > (days+leap))
	{
		sh4->m[RDAYCNT] = 1;
		carry = 1;
	}

	sh4->m[RMONCNT] = sh4->m[RMONCNT] + carry;
	if ((sh4->m[RMONCNT] & 0xf) == 0xa)
		sh4->m[RMONCNT] = sh4->m[RMONCNT] + 6;
	carry=0;
	if (sh4->m[RMONCNT] == 0x13)
	{
		sh4->m[RMONCNT] = 1;
		carry = 1;
	}

	sh4->m[RYRCNT] = sh4->m[RYRCNT] + carry;
	if ((sh4->m[RYRCNT] & 0xf) >= 0xa)
		sh4->m[RYRCNT] = sh4->m[RYRCNT] + 6;
	if ((sh4->m[RYRCNT] & 0xf0) >= 0xa0)
		sh4->m[RYRCNT] = sh4->m[RYRCNT] + 0x60;
	if ((sh4->m[RYRCNT] & 0xf00) >= 0xa00)
		sh4->m[RYRCNT] = sh4->m[RYRCNT] + 0x600;
	if ((sh4->m[RYRCNT] & 0xf000) >= 0xa000)
		sh4->m[RYRCNT] = 0;
}

static TIMER_CALLBACK( sh4_rtc_timer_callback )
{
	sh4_state *sh4 = (sh4_state *)ptr;

	if (sh4->cpu_type != CPU_TYPE_SH4)
	{
		logerror("sh4_rtc_timer_callback uses sh4->m[] with SH3\n");
		return;
	}

	sh4->rtc_timer->adjust(attotime::from_hz(128));
	sh4->m[R64CNT] = (sh4->m[R64CNT]+1) & 0x7f;
	if (sh4->m[R64CNT] == 64)
	{
		sh4->m[RCR1] |= 0x80;
		increment_rtc_time(sh4, 0);
		//sh4_exception_request(sh4, SH4_INTC_NMI); // TEST
	}
}


static void sh4_dmac_nmi(sh4_state *sh4) // manage dma when nmi gets asserted
{
	int s;

	sh4->SH4_DMAOR |= DMAOR_NMIF;
	for (s = 0;s < 4;s++)
	{
		if (sh4->dma_timer_active[s])
		{
			logerror("SH4: DMA %d cancelled due to NMI but all data transferred", s);
			sh4->dma_timer[s]->adjust(attotime::never, s);
			sh4->dma_timer_active[s] = 0;
		}
	}
}

void sh4_handler_ipra_w(sh4_state *sh4, UINT32 data, UINT32 mem_mask)
{
	COMBINE_DATA(&sh4->SH4_IPRA);
	/* 15 - 12 TMU0 */
	/* 11 -  8 TMU1 */
	/*  7 -  4 TMU2 */
	/*  3 -  0 RTC  */
	sh4->exception_priority[SH4_INTC_ATI]     = INTPRI(sh4->SH4_IPRA & 0x000f, SH4_INTC_ATI);
	sh4->exception_priority[SH4_INTC_PRI]     = INTPRI(sh4->SH4_IPRA & 0x000f, SH4_INTC_PRI);
	sh4->exception_priority[SH4_INTC_CUI]     = INTPRI(sh4->SH4_IPRA & 0x000f, SH4_INTC_CUI);

	sh4->exception_priority[SH4_INTC_TUNI2]  = INTPRI((sh4->SH4_IPRA & 0x00f0) >> 4, SH4_INTC_TUNI2);
	sh4->exception_priority[SH4_INTC_TICPI2] = INTPRI((sh4->SH4_IPRA & 0x00f0) >> 4, SH4_INTC_TICPI2);

	sh4->exception_priority[SH4_INTC_TUNI1]  = INTPRI((sh4->SH4_IPRA & 0x0f00) >> 8, SH4_INTC_TUNI1);

	sh4->exception_priority[SH4_INTC_TUNI0]  = INTPRI((sh4->SH4_IPRA & 0xf000) >> 12, SH4_INTC_TUNI0);

	logerror("setting priorities TMU0 %01x TMU1 %01x TMU2 %01x RTC %01x\n", (sh4->SH4_IPRA & 0xf000)>>12, (sh4->SH4_IPRA & 0x0f00)>>8, (sh4->SH4_IPRA & 0x00f0)>>4, (sh4->SH4_IPRA & 0x000f)>>0);

	sh4_exception_recompute(sh4);
}


WRITE32_HANDLER( sh4_internal_w )
{
	sh4_state *sh4 = get_safe_token(&space.device());
	int a;
	UINT32 addr = (offset << 2) + 0xfe000000;
	offset = ((addr & 0xfc) >> 2) | ((addr & 0x1fe0000) >> 11);

	if (sh4->cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_internal_w uses sh4->m[] with SH3\n");

	UINT32 old = sh4->m[offset];
	COMBINE_DATA(sh4->m+offset);

//  printf("sh4_internal_w:  Write %08x (%x), %08x @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, data, mem_mask);

	switch( offset )
	{
	case MMUCR: // MMU Control
		if (data & MMUCR_AT)
		{
			printf("SH4 MMU Enabled\n");
			printf("If you're seeing this, but running something other than a Naomi GD-ROM game then chances are it won't work\n");
			printf("The MMU emulation is a hack specific to that system\n");
			sh4->sh4_mmu_enabled = 1;

			// should be a different bit!
			{
				int i;
				for (i=0;i<64;i++)
				{
					sh4->sh4_tlb_address[i] = 0;
					sh4->sh4_tlb_data[i] = 0;
				}

			}
		}
		else
		{
			sh4->sh4_mmu_enabled = 0;
		}

		break;

		// Memory refresh
	case RTCSR:
		sh4->m[RTCSR] &= 255;
		if ((old >> 3) & 7)
			sh4->m[RTCNT] = compute_ticks_refresh_timer(sh4->refresh_timer, sh4->bus_clock, sh4->refresh_timer_base, rtcnt_div[(old >> 3) & 7]) & 0xff;
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // activated
			sh4_refresh_timer_recompute(sh4);
		}
		else
		{
			sh4->refresh_timer->adjust(attotime::never);
		}
		break;

	case RTCNT:
		sh4->m[RTCNT] &= 255;
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // active
			sh4_refresh_timer_recompute(sh4);
		}
		break;

	case RTCOR:
		sh4->m[RTCOR] &= 255;
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // active
			sh4->m[RTCNT] = compute_ticks_refresh_timer(sh4->refresh_timer, sh4->bus_clock, sh4->refresh_timer_base, rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]) & 0xff;
			sh4_refresh_timer_recompute(sh4);
		}
		break;

	case RFCR:
		sh4->m[RFCR] &= 1023;
		break;

		// RTC
	case RCR1:
		if ((sh4->m[RCR1] & 8) && (~old & 8)) // 0 -> 1
			sh4->m[RCR1] ^= 1;
		break;

	case RCR2:
		if (sh4->m[RCR2] & 2)
		{
			sh4->m[R64CNT] = 0;
			sh4->m[RCR2] ^= 2;
		}
		if (sh4->m[RCR2] & 4)
		{
			sh4->m[R64CNT] = 0;
			if (sh4->m[RSECCNT] >= 30)
				increment_rtc_time(sh4, 1);
			sh4->m[RSECCNT] = 0;
		}
		if ((sh4->m[RCR2] & 8) && (~old & 8))
		{ // 0 -> 1
			sh4->rtc_timer->adjust(attotime::from_hz(128));
		}
		else if (~(sh4->m[RCR2]) & 8)
		{ // 0
			sh4->rtc_timer->adjust(attotime::never);
		}
		break;

/*********************************************************************************************************************
        TMU (Timer Unit)
*********************************************************************************************************************/
	case SH4_TSTR_ADDR:	sh4_handle_tstr_addr_w(sh4,data,mem_mask);   break;
	case SH4_TCR0_ADDR:	sh4_handle_tcr0_addr_w(sh4,data,mem_mask);   break;
	case SH4_TCR1_ADDR: sh4_handle_tcr1_addr_w(sh4,data,mem_mask);   break;
	case SH4_TCR2_ADDR: sh4_handle_tcr2_addr_w(sh4,data,mem_mask);   break;
	case SH4_TCOR0_ADDR: sh4_handle_tcor0_addr_w(sh4,data,mem_mask); break;
	case SH4_TCNT0_ADDR: sh4_handle_tcnt0_addr_w(sh4,data,mem_mask); break;
	case SH4_TCOR1_ADDR: sh4_handle_tcor1_addr_w(sh4,data,mem_mask); break;
	case SH4_TCNT1_ADDR: sh4_handle_tcnt1_addr_w(sh4,data,mem_mask); break;
	case SH4_TCOR2_ADDR: sh4_handle_tcor2_addr_w(sh4,data,mem_mask); break;
	case SH4_TCNT2_ADDR: sh4_handle_tcnt2_addr_w(sh4,data,mem_mask); break;
	case SH4_TOCR_ADDR: sh4_handle_tocr_addr_w(sh4,data,mem_mask);   break; // not supported
	case SH4_TCPR2_ADDR: sh4_handle_tcpr2_addr_w(sh4,data,mem_mask); break; // not supported
/*********************************************************************************************************************
        INTC (Interrupt Controller)
*********************************************************************************************************************/
	case ICR:
		sh4->m[ICR] = (sh4->m[ICR] & 0x7fff) | (old & 0x8000);
		break;
	case IPRA: sh4_handler_ipra_w(sh4, data, mem_mask); break;
	case IPRB:
		sh4->exception_priority[SH4_INTC_SCI1ERI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1ERI);
		sh4->exception_priority[SH4_INTC_SCI1RXI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1RXI);
		sh4->exception_priority[SH4_INTC_SCI1TXI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TXI);
		sh4->exception_priority[SH4_INTC_SCI1TEI] = INTPRI((sh4->m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TEI);
		sh4->exception_priority[SH4_INTC_RCMI] = INTPRI((sh4->m[IPRB] & 0x0f00) >> 8, SH4_INTC_RCMI);
		sh4->exception_priority[SH4_INTC_ROVI] = INTPRI((sh4->m[IPRB] & 0x0f00) >> 8, SH4_INTC_ROVI);
		sh4->exception_priority[SH4_INTC_ITI] = INTPRI((sh4->m[IPRB] & 0xf000) >> 12, SH4_INTC_ITI);
		sh4_exception_recompute(sh4);
		break;
	case IPRC:
		sh4->exception_priority[SH4_INTC_HUDI] = INTPRI(sh4->m[IPRC] & 0x000f, SH4_INTC_HUDI);
		sh4->exception_priority[SH4_INTC_SCIFERI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFERI);
		sh4->exception_priority[SH4_INTC_SCIFRXI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFRXI);
		sh4->exception_priority[SH4_INTC_SCIFBRI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFBRI);
		sh4->exception_priority[SH4_INTC_SCIFTXI] = INTPRI((sh4->m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFTXI);
		sh4->exception_priority[SH4_INTC_DMTE0] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE0);
		sh4->exception_priority[SH4_INTC_DMTE1] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE1);
		sh4->exception_priority[SH4_INTC_DMTE2] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE2);
		sh4->exception_priority[SH4_INTC_DMTE3] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE3);
		sh4->exception_priority[SH4_INTC_DMAE] = INTPRI((sh4->m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMAE);
		sh4->exception_priority[SH4_INTC_GPOI] = INTPRI((sh4->m[IPRC] & 0xf000) >> 12, SH4_INTC_GPOI);
		sh4_exception_recompute(sh4);
		break;
/*********************************************************************************************************************
        DMAC (DMA Controller)
*********************************************************************************************************************/
	case SH4_SAR0_ADDR: sh4_handle_sar0_addr_w(sh4,data,mem_mask);   break;
	case SH4_SAR1_ADDR: sh4_handle_sar1_addr_w(sh4,data,mem_mask);   break;
	case SH4_SAR2_ADDR: sh4_handle_sar2_addr_w(sh4,data,mem_mask);   break;
	case SH4_SAR3_ADDR: sh4_handle_sar3_addr_w(sh4,data,mem_mask);   break;
	case SH4_DAR0_ADDR: sh4_handle_dar0_addr_w(sh4,data,mem_mask);   break;
	case SH4_DAR1_ADDR: sh4_handle_dar1_addr_w(sh4,data,mem_mask);   break;
	case SH4_DAR2_ADDR: sh4_handle_dar2_addr_w(sh4,data,mem_mask);   break;
	case SH4_DAR3_ADDR: sh4_handle_dar3_addr_w(sh4,data,mem_mask);   break;
	case SH4_DMATCR0_ADDR: sh4_handle_dmatcr0_addr_w(sh4,data,mem_mask);   break;
	case SH4_DMATCR1_ADDR: sh4_handle_dmatcr1_addr_w(sh4,data,mem_mask);   break;
	case SH4_DMATCR2_ADDR: sh4_handle_dmatcr2_addr_w(sh4,data,mem_mask);   break;
	case SH4_DMATCR3_ADDR: sh4_handle_dmatcr3_addr_w(sh4,data,mem_mask);   break;
	case SH4_CHCR0_ADDR: sh4_handle_chcr0_addr_w(sh4,data,mem_mask);   break;
	case SH4_CHCR1_ADDR: sh4_handle_chcr1_addr_w(sh4,data,mem_mask);   break;
	case SH4_CHCR2_ADDR: sh4_handle_chcr2_addr_w(sh4,data,mem_mask);   break;
	case SH4_CHCR3_ADDR: sh4_handle_chcr3_addr_w(sh4,data,mem_mask);   break;
	case SH4_DMAOR_ADDR: sh4_handle_dmaor_addr_w(sh4,data,mem_mask);   break;
/*********************************************************************************************************************
        Store Queues
*********************************************************************************************************************/
	case QACR0:
	case QACR1:
		break;
/*********************************************************************************************************************
        I/O
*********************************************************************************************************************/
	case PCTRA:
		sh4->ioport16_pullup = 0;
		sh4->ioport16_direction = 0;
		for (a=0;a < 16;a++) {
			sh4->ioport16_direction |= (sh4->m[PCTRA] & (1 << (a*2))) >> a;
			sh4->ioport16_pullup |= (sh4->m[PCTRA] & (1 << (a*2+1))) >> (a+1);
		}
		sh4->ioport16_direction &= 0xffff;
		sh4->ioport16_pullup = (sh4->ioport16_pullup | sh4->ioport16_direction) ^ 0xffff;
		if (sh4->m[BCR2] & 1)
			sh4->io->write_dword(SH4_IOPORT_16, (UINT64)(sh4->m[PDTRA] & sh4->ioport16_direction) | ((UINT64)sh4->m[PCTRA] << 16));
		break;
	case PDTRA:
		if (sh4->m[BCR2] & 1)
			sh4->io->write_dword(SH4_IOPORT_16, (UINT64)(sh4->m[PDTRA] & sh4->ioport16_direction) | ((UINT64)sh4->m[PCTRA] << 16));
		break;
	case PCTRB:
		sh4->ioport4_pullup = 0;
		sh4->ioport4_direction = 0;
		for (a=0;a < 4;a++) {
			sh4->ioport4_direction |= (sh4->m[PCTRB] & (1 << (a*2))) >> a;
			sh4->ioport4_pullup |= (sh4->m[PCTRB] & (1 << (a*2+1))) >> (a+1);
		}
		sh4->ioport4_direction &= 0xf;
		sh4->ioport4_pullup = (sh4->ioport4_pullup | sh4->ioport4_direction) ^ 0xf;
		if (sh4->m[BCR2] & 1)
			sh4->io->write_dword(SH4_IOPORT_4, (sh4->m[PDTRB] & sh4->ioport4_direction) | (sh4->m[PCTRB] << 16));
		break;
	case PDTRB:
		if (sh4->m[BCR2] & 1)
			sh4->io->write_dword(SH4_IOPORT_4, (sh4->m[PDTRB] & sh4->ioport4_direction) | (sh4->m[PCTRB] << 16));
		break;

	case SCBRR2:
		break;

	case SCSPTR2: //trips often in aristocrat mk-6
		break;

	default:
		logerror("sh4_internal_w:  Unmapped write %08x, %08x @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), data, mem_mask);
		break;
	}
}

READ32_HANDLER( sh4_internal_r )
{
	sh4_state *sh4 = get_safe_token(&space.device());

	if (sh4->cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_internal_r uses sh4->m[] with SH3\n");

	UINT32 addr = (offset << 2) + 0xfe000000;
	offset = ((addr & 0xfc) >> 2) | ((addr & 0x1fe0000) >> 11);

//  printf("sh4_internal_r:  Read %08x (%x) @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, mem_mask);

	switch( offset )
	{
	case VERSION:
		return PVR_SH7091;	// 0x040205c1, this is what a real SH7091 in a Dreamcast returns - the later Naomi BIOSes check and care!
	case PRR:
		return 0;
	case IPRD:
		return 0x00000000;	// SH7750 ignores writes here and always returns zero
	case RTCNT:
		if ((sh4->m[RTCSR] >> 3) & 7)
		{ // activated
			//((double)rtcnt_div[(sh4->m[RTCSR] >> 3) & 7] / (double)100000000)
			//return (refresh_timer_base + (sh4->refresh_timer->elapsed() * (double)100000000) / (double)rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]) & 0xff;
			return compute_ticks_refresh_timer(sh4->refresh_timer, sh4->bus_clock, sh4->refresh_timer_base, rtcnt_div[(sh4->m[RTCSR] >> 3) & 7]) & 0xff;
		}
		else
			return sh4->m[RTCNT];

/*********************************************************************************************************************
        INTC (Interrupt Controller)
*********************************************************************************************************************/

	case IPRA:
		return sh4->SH4_IPRA;

/*********************************************************************************************************************
        TMU (Timer Unit)
*********************************************************************************************************************/
	case SH4_TSTR_ADDR:  return sh4_handle_tstr_addr_r(sh4, mem_mask);
	case SH4_TCR0_ADDR:  return sh4_handle_tcr0_addr_r(sh4, mem_mask);
	case SH4_TCR1_ADDR:  return sh4_handle_tcr1_addr_r(sh4, mem_mask);
	case SH4_TCR2_ADDR:  return sh4_handle_tcr2_addr_r(sh4, mem_mask);
	case SH4_TCNT0_ADDR: return sh4_handle_tcnt0_addr_r(sh4, mem_mask);
	case SH4_TCNT1_ADDR: return sh4_handle_tcnt1_addr_r(sh4, mem_mask);
	case SH4_TCNT2_ADDR: return sh4_handle_tcnt2_addr_r(sh4, mem_mask);
	case SH4_TCOR0_ADDR: return sh4_handle_tcor0_addr_r(sh4, mem_mask);
	case SH4_TCOR1_ADDR: return sh4_handle_tcor1_addr_r(sh4, mem_mask);
	case SH4_TCOR2_ADDR: return sh4_handle_tcor2_addr_r(sh4, mem_mask);
	case SH4_TOCR_ADDR:  return sh4_handle_tocr_addr_r(sh4, mem_mask); // not supported
	case SH4_TCPR2_ADDR: return sh4_handle_tcpr2_addr_r(sh4, mem_mask); // not supported
/*********************************************************************************************************************
        DMAC (DMA Controller)
*********************************************************************************************************************/
	case SH4_SAR0_ADDR: return sh4_handle_sar0_addr_r(sh4,mem_mask);
	case SH4_SAR1_ADDR: return sh4_handle_sar1_addr_r(sh4,mem_mask);
	case SH4_SAR2_ADDR: return sh4_handle_sar2_addr_r(sh4,mem_mask);
	case SH4_SAR3_ADDR: return sh4_handle_sar3_addr_r(sh4,mem_mask);
	case SH4_DAR0_ADDR: return sh4_handle_dar0_addr_r(sh4,mem_mask);
	case SH4_DAR1_ADDR: return sh4_handle_dar1_addr_r(sh4,mem_mask);
	case SH4_DAR2_ADDR: return sh4_handle_dar2_addr_r(sh4,mem_mask);
	case SH4_DAR3_ADDR: return sh4_handle_dar3_addr_r(sh4,mem_mask);
	case SH4_DMATCR0_ADDR: return sh4_handle_dmatcr0_addr_r(sh4,mem_mask);
	case SH4_DMATCR1_ADDR: return sh4_handle_dmatcr1_addr_r(sh4,mem_mask);
	case SH4_DMATCR2_ADDR: return sh4_handle_dmatcr2_addr_r(sh4,mem_mask);
	case SH4_DMATCR3_ADDR: return sh4_handle_dmatcr3_addr_r(sh4,mem_mask);
	case SH4_CHCR0_ADDR: return sh4_handle_chcr0_addr_r(sh4,mem_mask);
	case SH4_CHCR1_ADDR: return sh4_handle_chcr1_addr_r(sh4,mem_mask);
	case SH4_CHCR2_ADDR: return sh4_handle_chcr2_addr_r(sh4,mem_mask);
	case SH4_CHCR3_ADDR: return sh4_handle_chcr3_addr_r(sh4,mem_mask);
	case SH4_DMAOR_ADDR: return sh4_handle_dmaor_addr_r(sh4,mem_mask);
/*********************************************************************************************************************
        I/O Ports
*********************************************************************************************************************/

	case PDTRA:
		if (sh4->m[BCR2] & 1)
			return (sh4->io->read_dword(SH4_IOPORT_16) & ~sh4->ioport16_direction) | (sh4->m[PDTRA] & sh4->ioport16_direction);
		break;
	case PDTRB:
		if (sh4->m[BCR2] & 1)
			return (sh4->io->read_dword(SH4_IOPORT_4) & ~sh4->ioport4_direction) | (sh4->m[PDTRB] & sh4->ioport4_direction);
		break;

		// SCIF (UART with FIFO)
	case SCFSR2:
		return 0x60; //read-only status register
	}
	return sh4->m[offset];
}

void sh4_set_frt_input(device_t *device, int state)
{
	sh4_state *sh4 = get_safe_token(device);

	if (sh4->cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_set_frt_input uses sh4->m[] with SH3\n");

	if(state == PULSE_LINE)
	{
		sh4_set_frt_input(device, ASSERT_LINE);
		sh4_set_frt_input(device, CLEAR_LINE);
		return;
	}

	if(sh4->frt_input == state) {
		return;
	}

	sh4->frt_input = state;

	if (sh4->cpu_type == CPU_TYPE_SH4)
	{

		if(sh4->m[5] & 0x8000) {
			if(state == CLEAR_LINE) {
				return;
			}
		} else {
			if(state == ASSERT_LINE) {
				return;
			}
		}
	}
	else
	{
		fatalerror("sh4_set_frt_input uses sh4->m[] with SH3\n");
	}

#if 0
	sh4_timer_resync();
	sh4->icr = sh4->frc;
	sh4->m[4] |= ICF;
	logerror("SH4 '%s': ICF activated (%x)\n", sh4->device->tag(), sh4->pc & AM);
	sh4_recalc_irq();
#endif
}

void sh4_set_irln_input(device_t *device, int value)
{
	sh4_state *sh4 = get_safe_token(device);

	if (sh4->irln == value)
		return;
	sh4->irln = value;
	device->execute().set_input_line(SH4_IRLn, ASSERT_LINE);
	device->execute().set_input_line(SH4_IRLn, CLEAR_LINE);
}

void sh4_set_irq_line(sh4_state *sh4, int irqline, int state) // set state of external interrupt line
{
	if (sh4->cpu_type == CPU_TYPE_SH3)
	{
		/***** ASSUME THIS TO BE WRONG FOR NOW *****/

		if (irqline == INPUT_LINE_NMI)
		{
			fatalerror("SH3 NMI Unimplemented\n");
		}
		else
		{
			//if (irqline > SH4_IRL3)
			//  return;
			if (sh4->irq_line_state[irqline] == state)
				return;
			sh4->irq_line_state[irqline] = state;

			if( state == CLEAR_LINE )
			{
				LOG(("SH-4 '%s' cleared external irq IRL%d\n", sh4->device->tag(), irqline));
				sh4_exception_unrequest(sh4, SH4_INTC_IRL0+irqline-SH4_IRL0);
			}
			else
			{
				LOG(("SH-4 '%s' assert external irq IRL%d\n", sh4->device->tag(), irqline));
				sh4_exception_request(sh4, SH4_INTC_IRL0+irqline-SH4_IRL0);
			}

		}

		/***** END ASSUME THIS TO BE WRONG FOR NOW *****/
	}
	else
	{
		int s;

		if (irqline == INPUT_LINE_NMI)
		{
			if (sh4->nmi_line_state == state)
				return;
			if (sh4->m[ICR] & 0x100)
			{
				if ((state == CLEAR_LINE) && (sh4->nmi_line_state == ASSERT_LINE))  // rising
				{
					LOG(("SH-4 '%s' assert nmi\n", sh4->device->tag()));
					sh4_exception_request(sh4, SH4_INTC_NMI);
					sh4_dmac_nmi(sh4);
				}
			}
			else
			{
				if ((state == ASSERT_LINE) && (sh4->nmi_line_state == CLEAR_LINE)) // falling
				{
					LOG(("SH-4 '%s' assert nmi\n", sh4->device->tag()));
					sh4_exception_request(sh4, SH4_INTC_NMI);
					sh4_dmac_nmi(sh4);
				}
			}
			if (state == CLEAR_LINE)
				sh4->m[ICR] ^= 0x8000;
			else
				sh4->m[ICR] |= 0x8000;
			sh4->nmi_line_state = state;
		}
		else
		{
			if (sh4->m[ICR] & 0x80) // four independent external interrupt sources
			{
				if (irqline > SH4_IRL3)
					return;
				if (sh4->irq_line_state[irqline] == state)
					return;
				sh4->irq_line_state[irqline] = state;

				if( state == CLEAR_LINE )
				{
					LOG(("SH-4 '%s' cleared external irq IRL%d\n", sh4->device->tag(), irqline));
					sh4_exception_unrequest(sh4, SH4_INTC_IRL0+irqline-SH4_IRL0);
				}
				else
				{
					LOG(("SH-4 '%s' assert external irq IRL%d\n", sh4->device->tag(), irqline));
					sh4_exception_request(sh4, SH4_INTC_IRL0+irqline-SH4_IRL0);
				}
			}
			else // level-encoded interrupt
			{
				if (irqline != SH4_IRLn)
					return;
				if ((sh4->irln > 15) || (sh4->irln < 0))
					return;
				for (s = 0; s < 15; s++)
					sh4_exception_unrequest(sh4, SH4_INTC_IRLn0+s);
				if (sh4->irln < 15)
					sh4_exception_request(sh4, SH4_INTC_IRLn0+sh4->irln);
				LOG(("SH-4 '%s' IRLn0-IRLn3 level #%d\n", sh4->device->tag(), sh4->irln));
			}
		}
		if (sh4->test_irq && (!sh4->delay))
			sh4_check_pending_irq(sh4, "sh4_set_irq_line");
	}
}

void sh4_parse_configuration(sh4_state *sh4, const struct sh4_config *conf)
{
	if(conf)
	{
		switch((conf->md2 << 2) | (conf->md1 << 1) | (conf->md0))
		{
		case 0:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 4;
			sh4->pm_clock = conf->clock / 4;
			break;
		case 1:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 6;
			sh4->pm_clock = conf->clock / 6;
			break;
		case 2:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 3;
			sh4->pm_clock = conf->clock / 6;
			break;
		case 3:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 3;
			sh4->pm_clock = conf->clock / 6;
			break;
		case 4:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 2;
			sh4->pm_clock = conf->clock / 4;
			break;
		case 5:
			sh4->cpu_clock = conf->clock;
			sh4->bus_clock = conf->clock / 2;
			sh4->pm_clock = conf->clock / 4;
			break;
		}
		sh4->is_slave = (~(conf->md7)) & 1;
	}
	else
	{
		sh4->cpu_clock = 200000000;
		sh4->bus_clock = 100000000;
		sh4->pm_clock = 50000000;
		sh4->is_slave = 0;
	}
}

void sh4_common_init(device_t *device)
{
	sh4_state *sh4 = get_safe_token(device);
	int i;

	for (i=0; i<3; i++)
	{
		sh4->timer[i] = device->machine().scheduler().timer_alloc(FUNC(sh4_timer_callback), sh4);
		sh4->timer[i]->adjust(attotime::never, i);
	}

	for (i=0; i<4; i++)
	{
		sh4->dma_timer[i] = device->machine().scheduler().timer_alloc(FUNC(sh4_dmac_callback), sh4);
		sh4->dma_timer[i]->adjust(attotime::never, i);
	}

	sh4->refresh_timer = device->machine().scheduler().timer_alloc(FUNC(sh4_refresh_timer_callback), sh4);
	sh4->refresh_timer->adjust(attotime::never);
	sh4->refresh_timer_base = 0;

	sh4->rtc_timer = device->machine().scheduler().timer_alloc(FUNC(sh4_rtc_timer_callback), sh4);
	sh4->rtc_timer->adjust(attotime::never);

	sh4->m = auto_alloc_array(device->machine(), UINT32, 16384);
}

UINT32 sh4_getsqremap(sh4_state *sh4, UINT32 address)
{
	if (!sh4->sh4_mmu_enabled)
		return address;
	else
	{
		int i;
		UINT32 topaddr = address&0xfff00000;

		for (i=0;i<64;i++)
		{
			UINT32 topcmp = sh4->sh4_tlb_address[i]&0xfff00000;
			if (topcmp==topaddr)
				return (address&0x000fffff) | ((sh4->sh4_tlb_data[i])&0xfff00000);
		}

	}

	return address;
}

READ64_HANDLER( sh4_tlb_r )
{
	sh4_state *sh4 = get_safe_token(&space.device());

	int offs = offset*8;

	if (offs >= 0x01000000)
	{
		UINT8 i = (offs>>8)&63;
		return sh4->sh4_tlb_data[i];
	}
	else
	{
		UINT8 i = (offs>>8)&63;
		return sh4->sh4_tlb_address[i];
	}
}

WRITE64_HANDLER( sh4_tlb_w )
{
	sh4_state *sh4 = get_safe_token(&space.device());

	int offs = offset*8;

	if (offs >= 0x01000000)
	{
		UINT8 i = (offs>>8)&63;
		sh4->sh4_tlb_data[i]  = data&0xffffffff;
	}
	else
	{
		UINT8 i = (offs>>8)&63;
		sh4->sh4_tlb_address[i] = data&0xffffffff;
	}
}

