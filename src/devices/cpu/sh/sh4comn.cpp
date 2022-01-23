// license:BSD-3-Clause
// copyright-holders:R. Belmont
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



static const uint32_t exception_priority_default[] = {
	EXPPRI(1,1,0,0),             /* Power-on Reset */
	EXPPRI(1,2,0,1),             /* Manual Reset */
	EXPPRI(1,1,0,2),             /* H-UDI Reset */
	EXPPRI(1,3,0,3),             /* Inst TLB Multiple Hit */
	EXPPRI(1,4,0,4),             /* Data TLB Multiple Hit */

	EXPPRI(2,0,0,5),            /* User break Before Instruction */
	EXPPRI(2,1,0,6),            /* Inst Address Error */
	EXPPRI(2,2,0,7),            /* Inst TLB Miss */
	EXPPRI(2,3,0,8),            /* Inst TLB Protection Violation */
	EXPPRI(2,4,0,9),            /* Illegal Instruction */
	EXPPRI(2,4,0,10),           /* Slot Illegal Instruction */
	EXPPRI(2,4,0,11),           /* FPU Disable */
	EXPPRI(2,4,0,12),           /* Slot FPU Disable */
	EXPPRI(2,5,0,13),           /* Data Address Error (Read) */
	EXPPRI(2,5,0,14),           /* Data Address Error (Write) */
	EXPPRI(2,6,0,15),           /* Data TBL Miss Read */
	EXPPRI(2,6,0,16),           /* Data TBL Miss Write */
	EXPPRI(2,7,0,17),           /* Data TBL Protection Violation Read */
	EXPPRI(2,7,0,18),           /* Data TBL Protection Violation Write */
	EXPPRI(2,8,0,19),           /* FPU Exception */
	EXPPRI(2,9,0,20),           /* Initial Page Write exception */

	EXPPRI(2,4,0,21),           /* Unconditional TRAP */
	EXPPRI(2,10,0,22),          /* User break After Instruction */

	EXPPRI(3,0,16,SH4_INTC_NMI) /* NMI */
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
	0x020, /* Manual Reset */
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

	0x160, /* Unconditional TRAP */
	-1, /* User break After Instruction */

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

	0x600, /* SH4_INTC_IRL0 */
	0x620, /* SH4_INTC_IRL1 */
	0x640, /* SH4_INTC_IRL2 */
	0x660, /* SH4_INTC_IRL3 */
	/* todo: SH3 should have lines 4+5 too? */

	-1, /* HUDI */
	-1, /* SH4_INTC_GPOI */
	0x800, /* SH4_INTC_DMTE0 */
	0x820, /* SH4_INTC_DMTE1 */
	0x840, /* SH4_INTC_DMTE2 */
	0x860, /* SH4_INTC_DMTE3 */

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

void sh34_base_device::sh4_swap_fp_registers()
{
	int s;
	uint32_t z;

	for (s = 0;s <= 15;s++)
	{
		z = m_sh2_state->m_fr[s];
		m_sh2_state->m_fr[s] = m_sh2_state->m_xf[s];
		m_sh2_state->m_xf[s] = z;
	}
}

void sh34_base_device::sh4_swap_fp_couples()
{
	int s;
	uint32_t z;

	for (s = 0;s <= 15;s = s+2)
	{
		z = m_sh2_state->m_fr[s];
		m_sh2_state->m_fr[s] = m_sh2_state->m_fr[s + 1];
		m_sh2_state->m_fr[s + 1] = z;
		z = m_sh2_state->m_xf[s];
		m_sh2_state->m_xf[s] = m_sh2_state->m_xf[s + 1];
		m_sh2_state->m_xf[s + 1] = z;
	}
}


void sh34_base_device::sh4_change_register_bank(int to)
{
	int s;

	if (to) // 0 -> 1
	{
		for (s = 0;s < 8;s++)
		{
			m_sh2_state->m_rbnk[0][s] = m_sh2_state->r[s];
			m_sh2_state->r[s] = m_sh2_state->m_rbnk[1][s];
		}
	}
	else // 1 -> 0
	{
		for (s = 0;s < 8;s++)
		{
			m_sh2_state->m_rbnk[1][s] = m_sh2_state->r[s];
			m_sh2_state->r[s] = m_sh2_state->m_rbnk[0][s];
		}
	}
}

void sh34_base_device::sh4_syncronize_register_bank(int to)
{
	int s;

	for (s = 0;s < 8;s++)
	{
		m_sh2_state->m_rbnk[to][s] = m_sh2_state->r[s];
	}
}

void sh34_base_device::sh4_default_exception_priorities() // setup default priorities for exceptions
{
	int a;

	for (a=0;a <= SH4_INTC_NMI;a++)
		m_exception_priority[a] = exception_priority_default[a];
	for (a=SH4_INTC_IRLn0;a <= SH4_INTC_IRLnE;a++)
		m_exception_priority[a] = INTPRI(15-(a-SH4_INTC_IRLn0), a);
	m_exception_priority[SH4_INTC_IRL0] = INTPRI(13, SH4_INTC_IRL0);
	m_exception_priority[SH4_INTC_IRL1] = INTPRI(10, SH4_INTC_IRL1);
	m_exception_priority[SH4_INTC_IRL2] = INTPRI(7, SH4_INTC_IRL2);
	m_exception_priority[SH4_INTC_IRL3] = INTPRI(4, SH4_INTC_IRL3);
	for (a=SH4_INTC_HUDI;a <= SH4_INTC_ROVI;a++)
		m_exception_priority[a] = INTPRI(0, a);
}

void sh34_base_device::sh4_exception_recompute() // checks if there is any interrupt with high enough priority
{
	int a,z;

	m_sh2_state->m_test_irq = 0;
	if ((!m_sh2_state->m_pending_irq) || ((m_sh2_state->sr & BL) && (m_exception_requesting[SH4_INTC_NMI] == 0)))
		return;
	z = (m_sh2_state->sr >> 4) & 15;
	for (a=0;a <= SH4_INTC_ROVI;a++)
	{
		if (m_exception_requesting[a])
		{
			int pri = (((int)m_exception_priority[a] >> 8) & 255);
			//logerror("pri is %02x z is %02x\n",pri,z);
			if (pri > z)
			{
				//logerror("will test\n");
				m_sh2_state->m_test_irq = 1; // will check for exception at end of instructions
				break;
			}
		}
	}
}

void sh34_base_device::sh4_exception_request(int exception) // start requesting an exception
{
	//logerror("sh4_exception_request a\n");
	if (!m_exception_requesting[exception])
	{
		//logerror("sh4_exception_request b\n");
		m_exception_requesting[exception] = 1;
		m_sh2_state->m_pending_irq++;
		sh4_exception_recompute();
	}
}

void sh34_base_device::sh4_exception_unrequest(int exception) // stop requesting an exception
{
	if (m_exception_requesting[exception])
	{
		m_exception_requesting[exception] = 0;
		m_sh2_state->m_pending_irq--;
		sh4_exception_recompute();
	}
}

void sh34_base_device::sh4_exception_checkunrequest(int exception)
{
	if (exception == SH4_INTC_NMI)
		sh4_exception_unrequest(exception);
	if ((exception == SH4_INTC_DMTE0) || (exception == SH4_INTC_DMTE1) ||
		(exception == SH4_INTC_DMTE2) || (exception == SH4_INTC_DMTE3))
		sh4_exception_unrequest(exception);
}


void sh34_base_device::sh4_exception_process(int exception, uint32_t vector)
{
	sh4_exception_checkunrequest(exception);

	m_sh2_state->m_spc = m_sh2_state->pc;
	m_sh2_state->m_ssr = m_sh2_state->sr;
	m_sh2_state->m_sgr = m_sh2_state->r[15];

	//printf("stored m_spc %08x m_ssr %08x m_sgr %08x\n", m_spc, m_ssr, m_sgr);

	m_sh2_state->sr |= MD;
	if ((machine().debug_flags & DEBUG_FLAG_ENABLED) != 0)
		sh4_syncronize_register_bank((m_sh2_state->sr & sRB) >> 29);
	if (!(m_sh2_state->sr & sRB))
		sh4_change_register_bank(1);
	m_sh2_state->sr |= sRB;
	m_sh2_state->sr |= BL;
	sh4_exception_recompute();

	/* fetch PC */
	m_sh2_state->pc = m_sh2_state->vbr + vector;
	m_willjump = 1; // for DRC
	/* wake up if a sleep opcode is triggered */
	if(m_sh2_state->sleep_mode == 1) { m_sh2_state->sleep_mode = 2; }
}

void sh34_base_device::sh4_exception(const char *message, int exception) // handle exception
{
	uint32_t vector;

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		if (exception < SH4_INTC_NMI)
			return; // Not yet supported
		if (exception == SH4_INTC_NMI) {
			if ((m_sh2_state->sr & BL) && (!(m_m[ICR] & 0x200)))
				return;

			m_m[ICR] &= ~0x200;
			m_m[INTEVT] = 0x1c0;


			vector = 0x600;
			standard_irq_callback(INPUT_LINE_NMI);
			LOG(("SH-4 '%s' nmi exception after [%s]\n", tag(), message));
		} else {
	//      if ((m_m[ICR] & 0x4000) && (m_nmi_line_state == ASSERT_LINE))
	//          return;
			if (m_sh2_state->sr & BL)
				return;
			if (((m_exception_priority[exception] >> 8) & 255) <= ((m_sh2_state->sr >> 4) & 15))
				return;
			m_m[INTEVT] = exception_codes[exception];
			vector = 0x600;
			if ((exception >= SH4_INTC_IRL0) && (exception <= SH4_INTC_IRL3))
				standard_irq_callback((exception-SH4_INTC_IRL0)+SH4_IRL0);
			else
				standard_irq_callback(SH4_IRL3+1);
			LOG(("SH-4 '%s' interrupt exception #%d after [%s]\n", tag(), exception, message));
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
			if (m_sh2_state->sr & BL)
				return;
			if (((m_exception_priority[exception] >> 8) & 255) <= ((m_sh2_state->sr >> 4) & 15))
				return;


			vector = 0x600;

			if ((exception >= SH4_INTC_IRL0) && (exception <= SH4_INTC_IRL3))
				standard_irq_callback((exception-SH4_INTC_IRL0)+SH4_IRL0);
			else
				standard_irq_callback(SH4_IRL3+1);

			if (sh3_intevt2_exception_codes[exception]==-1)
				fatalerror("sh3_intevt2_exception_codes unpopulated for exception %02x\n", exception);

			m_sh3internal_lower[INTEVT2] = sh3_intevt2_exception_codes[exception];
			m_sh3internal_upper[SH3_EXPEVT_ADDR] = exception_codes[exception];
			if (sh3_intevt2_exception_codes[exception] >= 0x600)
				m_sh3internal_upper[SH3_INTEVT_ADDR] = 0x3E0 - ((m_exception_priority[exception] >> 8) & 255) * 0x20;
			else
				m_sh3internal_upper[SH3_INTEVT_ADDR] = sh3_intevt2_exception_codes[exception];


			LOG(("SH-3 '%s' interrupt exception #%d after [%s]\n", tag(), exception, message));
		}

		/***** END ASSUME THIS TO BE WRONG FOR NOW *****/
	}
	sh4_exception_process(exception, vector);
}


uint32_t sh34_base_device::compute_ticks_refresh_timer(emu_timer *timer, int hertz, int base, int divisor)
{
	// elapsed:total = x : ticks
	// x=elapsed*tics/total -> x=elapsed*(double)100000000/rtcnt_div[(m_m[RTCSR] >> 3) & 7]
	// ticks/total=ticks / ((rtcnt_div[(m_m[RTCSR] >> 3) & 7] * ticks) / 100000000)=1/((rtcnt_div[(m_m[RTCSR] >> 3) & 7] / 100000000)=100000000/rtcnt_div[(m_m[RTCSR] >> 3) & 7]
	return base + (uint32_t)((timer->elapsed().as_double() * (double)hertz) / (double)divisor);
}

void sh34_base_device::sh4_refresh_timer_recompute()
{
	uint32_t ticks;

	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_refresh_timer_recompute uses m_m[] with SH3\n");


	//if rtcnt < rtcor then rtcor-rtcnt
	//if rtcnt >= rtcor then 256-rtcnt+rtcor=256+rtcor-rtcnt
	ticks = m_m[RTCOR]-m_m[RTCNT];
	if (ticks <= 0)
		ticks = 256 + ticks;
	m_refresh_timer->adjust(attotime::from_hz(m_bus_clock) * rtcnt_div[(m_m[RTCSR] >> 3) & 7] * ticks);
	m_refresh_timer_base = m_m[RTCNT];
}


TIMER_CALLBACK_MEMBER( sh34_base_device::sh4_refresh_timer_callback )
{
	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_refresh_timer_callback uses m_m[] with SH3\n");

	m_m[RTCNT] = 0;
	sh4_refresh_timer_recompute();
	m_m[RTCSR] |= 128;
	if ((m_m[MCR] & 4) && !(m_m[MCR] & 2))
	{
		m_m[RFCR] = (m_m[RFCR] + 1) & 1023;
		if (((m_m[RTCSR] & 1) && (m_m[RFCR] == 512)) || (m_m[RFCR] == 0))
		{
			m_m[RFCR] = 0;
			m_m[RTCSR] |= 4;
		}
	}
}

void sh34_base_device::increment_rtc_time(int mode)
{
	int carry, year, leap, days;

	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("increment_rtc_time uses m_m[] with SH3\n");

	if (mode == 0)
	{
		carry = 0;
		m_m[RSECCNT] = m_m[RSECCNT] + 1;
		if ((m_m[RSECCNT] & 0xf) == 0xa)
			m_m[RSECCNT] = m_m[RSECCNT] + 6;
		if (m_m[RSECCNT] == 0x60)
		{
			m_m[RSECCNT] = 0;
			carry=1;
		}
		else
			return;
	}
	else
		carry = 1;

	m_m[RMINCNT] = m_m[RMINCNT] + carry;
	if ((m_m[RMINCNT] & 0xf) == 0xa)
		m_m[RMINCNT] = m_m[RMINCNT] + 6;
	carry=0;
	if (m_m[RMINCNT] == 0x60)
	{
		m_m[RMINCNT] = 0;
		carry = 1;
	}

	m_m[RHRCNT] = m_m[RHRCNT] + carry;
	if ((m_m[RHRCNT] & 0xf) == 0xa)
		m_m[RHRCNT] = m_m[RHRCNT] + 6;
	carry = 0;
	if (m_m[RHRCNT] == 0x24)
	{
		m_m[RHRCNT] = 0;
		carry = 1;
	}

	m_m[RWKCNT] = m_m[RWKCNT] + carry;
	if (m_m[RWKCNT] == 0x7)
	{
		m_m[RWKCNT] = 0;
	}

	days = 0;
	year = (m_m[RYRCNT] & 0xf) + ((m_m[RYRCNT] & 0xf0) >> 4)*10 + ((m_m[RYRCNT] & 0xf00) >> 8)*100 + ((m_m[RYRCNT] & 0xf000) >> 12)*1000;
	leap = 0;
	if (!(year%100))
	{
		if (!(year%400))
			leap = 1;
	}
	else if (!(year%4))
		leap = 1;
	if (m_m[RMONCNT] != 2)
		leap = 0;
	if (m_m[RMONCNT])
		days = daysmonth[(m_m[RMONCNT] & 0xf) + ((m_m[RMONCNT] & 0xf0) >> 4)*10 - 1];

	m_m[RDAYCNT] = m_m[RDAYCNT] + carry;
	if ((m_m[RDAYCNT] & 0xf) == 0xa)
		m_m[RDAYCNT] = m_m[RDAYCNT] + 6;
	carry = 0;
	if (m_m[RDAYCNT] > (days+leap))
	{
		m_m[RDAYCNT] = 1;
		carry = 1;
	}

	m_m[RMONCNT] = m_m[RMONCNT] + carry;
	if ((m_m[RMONCNT] & 0xf) == 0xa)
		m_m[RMONCNT] = m_m[RMONCNT] + 6;
	carry=0;
	if (m_m[RMONCNT] == 0x13)
	{
		m_m[RMONCNT] = 1;
		carry = 1;
	}

	m_m[RYRCNT] = m_m[RYRCNT] + carry;
	if ((m_m[RYRCNT] & 0xf) >= 0xa)
		m_m[RYRCNT] = m_m[RYRCNT] + 6;
	if ((m_m[RYRCNT] & 0xf0) >= 0xa0)
		m_m[RYRCNT] = m_m[RYRCNT] + 0x60;
	if ((m_m[RYRCNT] & 0xf00) >= 0xa00)
		m_m[RYRCNT] = m_m[RYRCNT] + 0x600;
	if ((m_m[RYRCNT] & 0xf000) >= 0xa000)
		m_m[RYRCNT] = 0;
}

TIMER_CALLBACK_MEMBER( sh34_base_device::sh4_rtc_timer_callback )
{
	if (m_cpu_type != CPU_TYPE_SH4)
	{
		logerror("sh4_rtc_timer_callback uses m_m[] with SH3\n");
		return;
	}

	m_rtc_timer->adjust(attotime::from_hz(128));
	m_m[R64CNT] = (m_m[R64CNT]+1) & 0x7f;
	if (m_m[R64CNT] == 64)
	{
		m_m[RCR1] |= 0x80;
		increment_rtc_time(0);
		//sh4_exception_request(SH4_INTC_NMI); // TEST
	}
}


void sh34_base_device::sh4_dmac_nmi() // manage dma when nmi gets asserted
{
	int s;

	m_SH4_DMAOR |= DMAOR_NMIF;
	for (s = 0;s < 4;s++)
	{
		if (m_dma_timer_active[s])
		{
			logerror("SH4: DMA %d cancelled due to NMI but all data transferred", s);
			m_dma_timer[s]->adjust(attotime::never, s);
			m_dma_timer_active[s] = 0;
		}
	}
}

void sh34_base_device::sh4_handler_ipra_w(uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_SH4_IPRA);
	/* 15 - 12 TMU0 */
	/* 11 -  8 TMU1 */
	/*  7 -  4 TMU2 */
	/*  3 -  0 RTC  */
	m_exception_priority[SH4_INTC_ATI]     = INTPRI(m_SH4_IPRA & 0x000f, SH4_INTC_ATI);
	m_exception_priority[SH4_INTC_PRI]     = INTPRI(m_SH4_IPRA & 0x000f, SH4_INTC_PRI);
	m_exception_priority[SH4_INTC_CUI]     = INTPRI(m_SH4_IPRA & 0x000f, SH4_INTC_CUI);

	m_exception_priority[SH4_INTC_TUNI2]  = INTPRI((m_SH4_IPRA & 0x00f0) >> 4, SH4_INTC_TUNI2);
	m_exception_priority[SH4_INTC_TICPI2] = INTPRI((m_SH4_IPRA & 0x00f0) >> 4, SH4_INTC_TICPI2);

	m_exception_priority[SH4_INTC_TUNI1]  = INTPRI((m_SH4_IPRA & 0x0f00) >> 8, SH4_INTC_TUNI1);

	m_exception_priority[SH4_INTC_TUNI0]  = INTPRI((m_SH4_IPRA & 0xf000) >> 12, SH4_INTC_TUNI0);

	logerror("setting priorities TMU0 %01x TMU1 %01x TMU2 %01x RTC %01x\n", (m_SH4_IPRA & 0xf000)>>12, (m_SH4_IPRA & 0x0f00)>>8, (m_SH4_IPRA & 0x00f0)>>4, (m_SH4_IPRA & 0x000f)>>0);

	sh4_exception_recompute();
}


void sh4_base_device::sh4_internal_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	int a;
	uint32_t addr = (offset << 2) + 0xfe000000;
	offset = ((addr & 0xfc) >> 2) | ((addr & 0x1fe0000) >> 11);

	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_internal_w uses m_m[] with SH3\n");

	uint32_t old = m_m[offset];
	COMBINE_DATA(m_m+offset);

//  printf("sh4_internal_w:  Write %08x (%x), %08x @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, data, mem_mask);

	switch( offset )
	{
	case PTEH: // for use with LDTLB opcode
		m_m[PTEH] &= 0xffffffff;
		/*
		    NNNN NNNN NNNN NNNN NNNN NN-- AAAA AAAA

		    N = VPM = Virtual Page Number
		    A = ASID = Address Space Identifier

		    same as the address table part of the utlb but with 2 unused bits (these are sourced from PTEL instead when LDTLB is called)
		*/


		break;

	case PTEL:
		m_m[PTEL] &= 0xffffffff;
		/*
		     ---P PPPP PPPP PPPP PPPP PP-V zRRz CDHW

		     same format as data array 1 of the utlb
		*/
		break;

	case PTEA:
		m_m[PTEA] &= 0xffffffff;
		/*
		    ---- ---- ---- ---- ---- ---- ---- TSSS

		    same format as data array 2 of the utlb
		*/
		break;

	case TTB:
		m_m[TTB] &= 0xffffffff;
		logerror("TTB set to %08x\n", data);
		break;

	case TEA:
		m_m[TEA] &= 0xffffffff;
		logerror("TEA set to %08x\n", data);
		break;

	/*
	    LLLL LL-- BBBB BB-- CCCC CCQV ---- -T-A

	    L = LRUI = Least recently used ITLB
	    B = URB = UTLB replace boundary
	    C = URC = UTLB replace counter
	    Q = SQMD = Store Queue Mode Bit
	    V = SV = Single Virtual Mode Bit
	    T = TI = TLB invalidate
	    A = AT = Address translation bit (enable)
	*/
	case MMUCR: // MMU Control
		m_m[MMUCR] &= 0xffffffff;
		// MMUCR_AT
		m_sh4_mmu_enabled = bool(BIT(data, 0));
		logerror("%s: MMUCR %08x (enable: %d)\n", machine().describe_context(), data, m_sh4_mmu_enabled);

		if (m_sh4_mmu_enabled)
		{
			// Newer versions of the Dreamcast Katana SDK use MMU to remap the SQ write-back space (cfr. ikaruga and several later NAOMI GD-ROM releases)
			// Anything beyond that is bound to fail,
			// i.e. DC WinCE games, DC Linux distros, v2 Sega checkers, aristmk6.cpp
#if 0
			if (m_mmuhack == 1)
			{
				printf("SH4 MMU Enabled\n");
				printf("If you're seeing this, but running something other than a Naomi GD-ROM game then chances are it won't work\n");
				printf("The MMU emulation is a hack specific to that system\n");
			}
#endif

			if (m_mmuhack == 2)
			{
				for (int i = 0; i < 64; i++)
				{
					if (m_utlb[i].V)
					{
						// FIXME: potentially verbose, move to logmacro.h pattern
						// cfr. MMU Check_4 in v2.xx DC CHECKER
						logerror("(entry %02x | ASID: %02x VPN: %08x V: %02x PPN: %08x SZ: %02x SH: %02x C: %02x PPR: %02x D: %02x WT %02x: SA: %02x TC: %02x)\n",
							i,
							m_utlb[i].ASID,
							m_utlb[i].VPN << 10,
							m_utlb[i].V,
							m_utlb[i].PPN << 10,
							m_utlb[i].PSZ,
							m_utlb[i].SH,
							m_utlb[i].C,
							m_utlb[i].PPR,
							m_utlb[i].D,
							m_utlb[i].WT,
							m_utlb[i].SA,
							m_utlb[i].TC);
					}
				}
			}
		}

		break;

		// Memory refresh
	case RTCSR:
		m_m[RTCSR] &= 255;
		if ((old >> 3) & 7)
			m_m[RTCNT] = compute_ticks_refresh_timer(m_refresh_timer, m_bus_clock, m_refresh_timer_base, rtcnt_div[(old >> 3) & 7]) & 0xff;
		if ((m_m[RTCSR] >> 3) & 7)
		{ // activated
			sh4_refresh_timer_recompute();
		}
		else
		{
			m_refresh_timer->adjust(attotime::never);
		}
		break;

	case RTCNT:
		m_m[RTCNT] &= 255;
		if ((m_m[RTCSR] >> 3) & 7)
		{ // active
			sh4_refresh_timer_recompute();
		}
		break;

	case RTCOR:
		m_m[RTCOR] &= 255;
		if ((m_m[RTCSR] >> 3) & 7)
		{ // active
			m_m[RTCNT] = compute_ticks_refresh_timer(m_refresh_timer, m_bus_clock, m_refresh_timer_base, rtcnt_div[(m_m[RTCSR] >> 3) & 7]) & 0xff;
			sh4_refresh_timer_recompute();
		}
		break;

	case RFCR:
		m_m[RFCR] &= 1023;
		break;

		// RTC
	case RCR1:
		if ((m_m[RCR1] & 8) && (~old & 8)) // 0 -> 1
			m_m[RCR1] ^= 1;
		break;

	case RCR2:
		if (m_m[RCR2] & 2)
		{
			m_m[R64CNT] = 0;
			m_m[RCR2] ^= 2;
		}
		if (m_m[RCR2] & 4)
		{
			m_m[R64CNT] = 0;
			if (m_m[RSECCNT] >= 30)
				increment_rtc_time(1);
			m_m[RSECCNT] = 0;
		}
		if ((m_m[RCR2] & 8) && (~old & 8))
		{ // 0 -> 1
			m_rtc_timer->adjust(attotime::from_hz(128));
		}
		else if (~(m_m[RCR2]) & 8)
		{ // 0
			m_rtc_timer->adjust(attotime::never);
		}
		break;

/*********************************************************************************************************************
        TMU (Timer Unit)
*********************************************************************************************************************/
	case SH4_TSTR_ADDR: sh4_handle_tstr_addr_w(data,mem_mask);   break;
	case SH4_TCR0_ADDR: sh4_handle_tcr0_addr_w(data,mem_mask);   break;
	case SH4_TCR1_ADDR: sh4_handle_tcr1_addr_w(data,mem_mask);   break;
	case SH4_TCR2_ADDR: sh4_handle_tcr2_addr_w(data,mem_mask);   break;
	case SH4_TCOR0_ADDR: sh4_handle_tcor0_addr_w(data,mem_mask); break;
	case SH4_TCNT0_ADDR: sh4_handle_tcnt0_addr_w(data,mem_mask); break;
	case SH4_TCOR1_ADDR: sh4_handle_tcor1_addr_w(data,mem_mask); break;
	case SH4_TCNT1_ADDR: sh4_handle_tcnt1_addr_w(data,mem_mask); break;
	case SH4_TCOR2_ADDR: sh4_handle_tcor2_addr_w(data,mem_mask); break;
	case SH4_TCNT2_ADDR: sh4_handle_tcnt2_addr_w(data,mem_mask); break;
	case SH4_TOCR_ADDR: sh4_handle_tocr_addr_w(data,mem_mask);   break; // not supported
	case SH4_TCPR2_ADDR: sh4_handle_tcpr2_addr_w(data,mem_mask); break; // not supported
/*********************************************************************************************************************
        INTC (Interrupt Controller)
*********************************************************************************************************************/
	case ICR:
		m_m[ICR] = (m_m[ICR] & 0x7fff) | (old & 0x8000);
		break;
	case IPRA: sh4_handler_ipra_w(data, mem_mask); break;
	case IPRB:
		m_exception_priority[SH4_INTC_SCI1ERI] = INTPRI((m_m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1ERI);
		m_exception_priority[SH4_INTC_SCI1RXI] = INTPRI((m_m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1RXI);
		m_exception_priority[SH4_INTC_SCI1TXI] = INTPRI((m_m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TXI);
		m_exception_priority[SH4_INTC_SCI1TEI] = INTPRI((m_m[IPRB] & 0x00f0) >> 4, SH4_INTC_SCI1TEI);
		m_exception_priority[SH4_INTC_RCMI] = INTPRI((m_m[IPRB] & 0x0f00) >> 8, SH4_INTC_RCMI);
		m_exception_priority[SH4_INTC_ROVI] = INTPRI((m_m[IPRB] & 0x0f00) >> 8, SH4_INTC_ROVI);
		m_exception_priority[SH4_INTC_ITI] = INTPRI((m_m[IPRB] & 0xf000) >> 12, SH4_INTC_ITI);
		sh4_exception_recompute();
		break;
	case IPRC:
		m_exception_priority[SH4_INTC_HUDI] = INTPRI(m_m[IPRC] & 0x000f, SH4_INTC_HUDI);
		m_exception_priority[SH4_INTC_SCIFERI] = INTPRI((m_m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFERI);
		m_exception_priority[SH4_INTC_SCIFRXI] = INTPRI((m_m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFRXI);
		m_exception_priority[SH4_INTC_SCIFBRI] = INTPRI((m_m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFBRI);
		m_exception_priority[SH4_INTC_SCIFTXI] = INTPRI((m_m[IPRC] & 0x00f0) >> 4, SH4_INTC_SCIFTXI);
		m_exception_priority[SH4_INTC_DMTE0] = INTPRI((m_m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE0);
		m_exception_priority[SH4_INTC_DMTE1] = INTPRI((m_m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE1);
		m_exception_priority[SH4_INTC_DMTE2] = INTPRI((m_m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE2);
		m_exception_priority[SH4_INTC_DMTE3] = INTPRI((m_m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMTE3);
		m_exception_priority[SH4_INTC_DMAE] = INTPRI((m_m[IPRC] & 0x0f00) >> 8, SH4_INTC_DMAE);
		m_exception_priority[SH4_INTC_GPOI] = INTPRI((m_m[IPRC] & 0xf000) >> 12, SH4_INTC_GPOI);
		sh4_exception_recompute();
		break;
/*********************************************************************************************************************
        DMAC (DMA Controller)
*********************************************************************************************************************/
	case SH4_SAR0_ADDR: sh4_handle_sar0_addr_w(data,mem_mask);   break;
	case SH4_SAR1_ADDR: sh4_handle_sar1_addr_w(data,mem_mask);   break;
	case SH4_SAR2_ADDR: sh4_handle_sar2_addr_w(data,mem_mask);   break;
	case SH4_SAR3_ADDR: sh4_handle_sar3_addr_w(data,mem_mask);   break;
	case SH4_DAR0_ADDR: sh4_handle_dar0_addr_w(data,mem_mask);   break;
	case SH4_DAR1_ADDR: sh4_handle_dar1_addr_w(data,mem_mask);   break;
	case SH4_DAR2_ADDR: sh4_handle_dar2_addr_w(data,mem_mask);   break;
	case SH4_DAR3_ADDR: sh4_handle_dar3_addr_w(data,mem_mask);   break;
	case SH4_DMATCR0_ADDR: sh4_handle_dmatcr0_addr_w(data,mem_mask);   break;
	case SH4_DMATCR1_ADDR: sh4_handle_dmatcr1_addr_w(data,mem_mask);   break;
	case SH4_DMATCR2_ADDR: sh4_handle_dmatcr2_addr_w(data,mem_mask);   break;
	case SH4_DMATCR3_ADDR: sh4_handle_dmatcr3_addr_w(data,mem_mask);   break;
	case SH4_CHCR0_ADDR: sh4_handle_chcr0_addr_w(data,mem_mask);   break;
	case SH4_CHCR1_ADDR: sh4_handle_chcr1_addr_w(data,mem_mask);   break;
	case SH4_CHCR2_ADDR: sh4_handle_chcr2_addr_w(data,mem_mask);   break;
	case SH4_CHCR3_ADDR: sh4_handle_chcr3_addr_w(data,mem_mask);   break;
	case SH4_DMAOR_ADDR: sh4_handle_dmaor_addr_w(data,mem_mask);   break;
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
		m_ioport16_pullup = 0;
		m_ioport16_direction = 0;
		for (a=0;a < 16;a++) {
			m_ioport16_direction |= (m_m[PCTRA] & (1 << (a*2))) >> a;
			m_ioport16_pullup |= (m_m[PCTRA] & (1 << (a*2+1))) >> (a+1);
		}
		m_ioport16_direction &= 0xffff;
		m_ioport16_pullup = (m_ioport16_pullup | m_ioport16_direction) ^ 0xffff;
		if (m_m[BCR2] & 1)
			m_io->write_dword(SH4_IOPORT_16, (uint64_t)(m_m[PDTRA] & m_ioport16_direction) | ((uint64_t)m_m[PCTRA] << 16));
		break;
	case PDTRA:
		if (m_m[BCR2] & 1)
			m_io->write_dword(SH4_IOPORT_16, (uint64_t)(m_m[PDTRA] & m_ioport16_direction) | ((uint64_t)m_m[PCTRA] << 16));
		break;
	case PCTRB:
		m_ioport4_pullup = 0;
		m_ioport4_direction = 0;
		for (a=0;a < 4;a++) {
			m_ioport4_direction |= (m_m[PCTRB] & (1 << (a*2))) >> a;
			m_ioport4_pullup |= (m_m[PCTRB] & (1 << (a*2+1))) >> (a+1);
		}
		m_ioport4_direction &= 0xf;
		m_ioport4_pullup = (m_ioport4_pullup | m_ioport4_direction) ^ 0xf;
		if (m_m[BCR2] & 1)
			m_io->write_dword(SH4_IOPORT_4, (m_m[PDTRB] & m_ioport4_direction) | (m_m[PCTRB] << 16));
		break;
	case PDTRB:
		if (m_m[BCR2] & 1)
			m_io->write_dword(SH4_IOPORT_4, (m_m[PDTRB] & m_ioport4_direction) | (m_m[PCTRB] << 16));
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

uint32_t sh4_base_device::sh4_internal_r(offs_t offset, uint32_t mem_mask)
{
	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_internal_r uses m_m[] with SH3\n");

	uint32_t addr = (offset << 2) + 0xfe000000;
	offset = ((addr & 0xfc) >> 2) | ((addr & 0x1fe0000) >> 11);

//  printf("sh4_internal_r:  Read %08x (%x) @ %08x\n", 0xfe000000+((offset & 0x3fc0) << 11)+((offset & 0x3f) << 2), offset, mem_mask);

	switch( offset )
	{
	case VERSION:
		return PVR_SH7091;  // 0x040205c1, this is what a real SH7091 in a Dreamcast returns - the later Naomi BIOSes check and care!
	case PRR:
		return 0;
	case IPRD:
		return 0x00000000;  // SH7750 ignores writes here and always returns zero
	case RTCNT:
		if ((m_m[RTCSR] >> 3) & 7)
		{ // activated
			//((double)rtcnt_div[(m_m[RTCSR] >> 3) & 7] / (double)100000000)
			//return (refresh_timer_base + (m_refresh_timer->elapsed() * (double)100000000) / (double)rtcnt_div[(m_m[RTCSR] >> 3) & 7]) & 0xff;
			return compute_ticks_refresh_timer(m_refresh_timer, m_bus_clock, m_refresh_timer_base, rtcnt_div[(m_m[RTCSR] >> 3) & 7]) & 0xff;
		}
		else
			return m_m[RTCNT];

/*********************************************************************************************************************
        INTC (Interrupt Controller)
*********************************************************************************************************************/

	case IPRA:
		return m_SH4_IPRA;

/*********************************************************************************************************************
        TMU (Timer Unit)
*********************************************************************************************************************/
	case SH4_TSTR_ADDR:  return sh4_handle_tstr_addr_r(mem_mask);
	case SH4_TCR0_ADDR:  return sh4_handle_tcr0_addr_r(mem_mask);
	case SH4_TCR1_ADDR:  return sh4_handle_tcr1_addr_r(mem_mask);
	case SH4_TCR2_ADDR:  return sh4_handle_tcr2_addr_r(mem_mask);
	case SH4_TCNT0_ADDR: return sh4_handle_tcnt0_addr_r(mem_mask);
	case SH4_TCNT1_ADDR: return sh4_handle_tcnt1_addr_r(mem_mask);
	case SH4_TCNT2_ADDR: return sh4_handle_tcnt2_addr_r(mem_mask);
	case SH4_TCOR0_ADDR: return sh4_handle_tcor0_addr_r(mem_mask);
	case SH4_TCOR1_ADDR: return sh4_handle_tcor1_addr_r(mem_mask);
	case SH4_TCOR2_ADDR: return sh4_handle_tcor2_addr_r(mem_mask);
	case SH4_TOCR_ADDR:  return sh4_handle_tocr_addr_r(mem_mask); // not supported
	case SH4_TCPR2_ADDR: return sh4_handle_tcpr2_addr_r(mem_mask); // not supported
/*********************************************************************************************************************
        DMAC (DMA Controller)
*********************************************************************************************************************/
	case SH4_SAR0_ADDR: return sh4_handle_sar0_addr_r(mem_mask);
	case SH4_SAR1_ADDR: return sh4_handle_sar1_addr_r(mem_mask);
	case SH4_SAR2_ADDR: return sh4_handle_sar2_addr_r(mem_mask);
	case SH4_SAR3_ADDR: return sh4_handle_sar3_addr_r(mem_mask);
	case SH4_DAR0_ADDR: return sh4_handle_dar0_addr_r(mem_mask);
	case SH4_DAR1_ADDR: return sh4_handle_dar1_addr_r(mem_mask);
	case SH4_DAR2_ADDR: return sh4_handle_dar2_addr_r(mem_mask);
	case SH4_DAR3_ADDR: return sh4_handle_dar3_addr_r(mem_mask);
	case SH4_DMATCR0_ADDR: return sh4_handle_dmatcr0_addr_r(mem_mask);
	case SH4_DMATCR1_ADDR: return sh4_handle_dmatcr1_addr_r(mem_mask);
	case SH4_DMATCR2_ADDR: return sh4_handle_dmatcr2_addr_r(mem_mask);
	case SH4_DMATCR3_ADDR: return sh4_handle_dmatcr3_addr_r(mem_mask);
	case SH4_CHCR0_ADDR: return sh4_handle_chcr0_addr_r(mem_mask);
	case SH4_CHCR1_ADDR: return sh4_handle_chcr1_addr_r(mem_mask);
	case SH4_CHCR2_ADDR: return sh4_handle_chcr2_addr_r(mem_mask);
	case SH4_CHCR3_ADDR: return sh4_handle_chcr3_addr_r(mem_mask);
	case SH4_DMAOR_ADDR: return sh4_handle_dmaor_addr_r(mem_mask);
/*********************************************************************************************************************
        I/O Ports
*********************************************************************************************************************/

	case PDTRA:
		if (m_m[BCR2] & 1)
			return (m_io->read_dword(SH4_IOPORT_16) & ~m_ioport16_direction) | (m_m[PDTRA] & m_ioport16_direction);
		break;
	case PDTRB:
		if (m_m[BCR2] & 1)
			return (m_io->read_dword(SH4_IOPORT_4) & ~m_ioport4_direction) | (m_m[PDTRB] & m_ioport4_direction);
		break;

		// SCIF (UART with FIFO)
	case SCFSR2:
		return 0x60; //read-only status register
	}
	return m_m[offset];
}

void sh34_base_device::set_frt_input(int state)
{
	if (m_cpu_type != CPU_TYPE_SH4)
		fatalerror("sh4_set_frt_input uses m_m[] with SH3\n");

	if(m_sh2_state->m_frt_input == state) {
		return;
	}

	m_sh2_state->m_frt_input = state;

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		if(m_m[5] & 0x8000) {
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
		fatalerror("sh4_set_frt_input uses m_m[] with SH3\n");
	}

#if 0
	sh4_timer_resync();
	m_icr = m_frc;
	m_m[4] |= ICF;
	logerror("SH4 '%s': ICF activated (%x)\n", tag(), m_sh2_state->pc & AM);
	sh4_recalc_irq();
#endif
}

void sh34_base_device::sh4_set_irln_input(int value)
{
	if (m_irln == value)
		return;
	m_irln = value;
	set_input_line(SH4_IRLn, ASSERT_LINE);
	set_input_line(SH4_IRLn, CLEAR_LINE);
}

void sh34_base_device::execute_set_input(int irqline, int state) // set state of external interrupt line
{
	if (m_cpu_type == CPU_TYPE_SH3)
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
			if (m_irq_line_state[irqline] == state)
				return;
			m_irq_line_state[irqline] = state;

			if( state == CLEAR_LINE )
			{
				LOG(("SH-4 '%s' cleared external irq IRL%d\n", tag(), irqline));
				sh4_exception_unrequest(SH4_INTC_IRL0+irqline-SH4_IRL0);
			}
			else
			{
				LOG(("SH-4 '%s' assert external irq IRL%d\n", tag(), irqline));
				sh4_exception_request(SH4_INTC_IRL0+irqline-SH4_IRL0);
			}

		}

		/***** END ASSUME THIS TO BE WRONG FOR NOW *****/
	}
	else
	{
		int s;

		if (irqline == INPUT_LINE_NMI)
		{
			if (m_nmi_line_state == state)
				return;
			if (m_m[ICR] & 0x100)
			{
				if ((state == CLEAR_LINE) && (m_nmi_line_state == ASSERT_LINE))  // rising
				{
					LOG(("SH-4 '%s' assert nmi\n", tag()));
					sh4_exception_request(SH4_INTC_NMI);
					sh4_dmac_nmi();
				}
			}
			else
			{
				if ((state == ASSERT_LINE) && (m_nmi_line_state == CLEAR_LINE)) // falling
				{
					LOG(("SH-4 '%s' assert nmi\n", tag()));
					sh4_exception_request(SH4_INTC_NMI);
					sh4_dmac_nmi();
				}
			}
			if (state == CLEAR_LINE)
				m_m[ICR] ^= 0x8000;
			else
				m_m[ICR] |= 0x8000;
			m_nmi_line_state = state;
		}
		else
		{
			if (m_m[ICR] & 0x80) // four independent external interrupt sources
			{
				if (irqline > SH4_IRL3)
					return;
				if (m_irq_line_state[irqline] == state)
					return;
				m_irq_line_state[irqline] = state;

				if( state == CLEAR_LINE )
				{
					LOG(("SH-4 '%s' cleared external irq IRL%d\n", tag(), irqline));
					sh4_exception_unrequest(SH4_INTC_IRL0+irqline-SH4_IRL0);
				}
				else
				{
					LOG(("SH-4 '%s' assert external irq IRL%d\n", tag(), irqline));
					sh4_exception_request(SH4_INTC_IRL0+irqline-SH4_IRL0);
				}
			}
			else // level-encoded interrupt
			{
				if (irqline != SH4_IRLn)
					return;
				if ((m_irln > 15) || (m_irln < 0))
					return;
				for (s = 0; s < 15; s++)
					sh4_exception_unrequest(SH4_INTC_IRLn0+s);
				if (m_irln < 15)
					sh4_exception_request(SH4_INTC_IRLn0+m_irln);
				LOG(("SH-4 '%s' IRLn0-IRLn3 level #%d\n", tag(), m_irln));
			}
		}
		if (m_sh2_state->m_test_irq && (!m_sh2_state->m_delay))
			sh4_check_pending_irq("sh4_set_irq_line");
	}
}

void sh34_base_device::sh4_parse_configuration()
{
	if(m_clock > 0)
	{
		switch((m_md[2] << 2) | (m_md[1] << 1) | (m_md[0]))
		{
		case 0:
			m_cpu_clock = m_clock;
			m_bus_clock = m_clock / 4;
			m_pm_clock = m_clock / 4;
			break;
		case 1:
			m_cpu_clock = m_clock;
			m_bus_clock = m_clock / 6;
			m_pm_clock = m_clock / 6;
			break;
		case 2:
			m_cpu_clock = m_clock;
			m_bus_clock = m_clock / 3;
			m_pm_clock = m_clock / 6;
			break;
		case 3:
			m_cpu_clock = m_clock;
			m_bus_clock = m_clock / 3;
			m_pm_clock = m_clock / 6;
			break;
		case 4:
			m_cpu_clock = m_clock;
			m_bus_clock = m_clock / 2;
			m_pm_clock = m_clock / 4;
			break;
		case 5:
			m_cpu_clock = m_clock;
			m_bus_clock = m_clock / 2;
			m_pm_clock = m_clock / 4;
			break;
		}
		m_is_slave = (~(m_md[7])) & 1;
	}
	else
	{
		m_cpu_clock = 200000000;
		m_bus_clock = 100000000;
		m_pm_clock = 50000000;
		m_is_slave = 0;
	}
}

uint32_t sh34_base_device::get_remap(uint32_t address)
{
	return address;
}

uint32_t sh4_base_device::get_remap(uint32_t address)
{
	if (m_mmuhack != 2)
		return address;

	// is this the correct way around?
	int i;
	uint32_t topaddr = address&0xfff00000;

	for (i=0;i<64;i++)
	{
		if (m_utlb[i].V)
		{
			uint32_t topcmp = (m_utlb[i].PPN << 10) & 0xfff00000;
			if (topcmp == topaddr)
				return (address & 0x000fffff) | ((m_utlb[i].VPN << 10) & 0xfff00000);
		}
	}

	//printf("address not in UTLB? %08x\n", address);


	return address;
}

uint32_t sh34_base_device::sh4_getsqremap(uint32_t address)
{
	return address;
}

uint32_t sh4_base_device::sh4_getsqremap(uint32_t address)
{
	if (!m_sh4_mmu_enabled || (m_mmuhack != 1))
		return address;
	else
	{
		int i;
		uint32_t topaddr = address&0xfff00000;

		for (i=0;i<64;i++)
		{
			uint32_t topcmp = (m_utlb[i].VPN<<10)&0xfff00000;
			if (topcmp==topaddr)
				return (address&0x000fffff) | ((m_utlb[i].PPN<<10)&0xfff00000);
		}
	}

	return address;
}


void sh4_base_device::sh4_utlb_address_array_w(offs_t offset, uint64_t data)
{
/*  uses bits 13:8 of address to select which UTLB entry we're addressing
    bit 7 of the address enables 'associative' mode, causing a search
    operation rather than a direct write.

    NNNN NNNN NNNN NNNN NNNN NNDV AAAA AAAA

    N = VPN = Virtual Page Number
    D = Dirty Bit
    V = Validity Bit
    A = ASID = Address Space Identifier
*/

	logerror("sh4_utlb_address_array_w %08x %08x\n", offset, data);
	int offs = offset << 3;

	uint8_t associative = (offs >> 7) & 1;

	if (!associative)
	{
		// non-associative mode
		uint8_t i = (offs >> 8) & 63;

		m_utlb[i].VPN =  (data & 0xfffffc00) >> 10;
		m_utlb[i].D =    (data & 0x00000200) >> 9;
		m_utlb[i].V =    (data & 0x00000100) >> 8;
		m_utlb[i].ASID = (data & 0x000000ff) >> 0;
	}
	else
	{
		// associative mode
		fatalerror("SH4MMU: associative mode writes unsupported\n");
	}
}

uint64_t sh4_base_device::sh4_utlb_address_array_r(offs_t offset)
{
	// associative bit is ignored for reads
	int offs = offset*8;

	uint32_t ret = 0;

	uint8_t i = (offs >> 8) & 63;

	ret |= m_utlb[i].VPN << 10;
	ret |= m_utlb[i].D << 9;
	ret |= m_utlb[i].V << 8;
	ret |= m_utlb[i].ASID << 0;

	return ret;
}


void sh4_base_device::sh4_utlb_data_array1_w(offs_t offset, uint64_t data)
{
/*  uses bits 13:8 of address to select which UTLB entry we're addressing

    ---P PPPP PPPP PPPP PPPP PP-V zRRz CDHW

    P = PPN = Physical page number
    V = Validity bit
    z = SZ = Page Size (2 bits, split)
    D = Dirty Bit
    R = PR = Protection Key Data
    C = Cacheable bit
    H = Share status
    W = Write through
    - = unused (should be 0)
*/
	logerror("sh4_utlb_data_array1_w %08x %08x\n", offset, data);
	int offs = offset*8;

	uint8_t i = (offs>>8)&63;

	m_utlb[i].PPN = (data & 0x1ffffc00) >> 10;
	m_utlb[i].V =   (data & 0x00000100) >> 8;
	m_utlb[i].PSZ = (data & 0x00000080) >> 6;
	m_utlb[i].PSZ |=(data & 0x00000010) >> 4;
	m_utlb[i].PPR=  (data & 0x00000060) >> 5;
	m_utlb[i].C =   (data & 0x00000008) >> 3;
	m_utlb[i].D =   (data & 0x00000004) >> 2;
	m_utlb[i].SH =  (data & 0x00000002) >> 1;
	m_utlb[i].WT =  (data & 0x00000001) >> 0;
}


uint64_t sh4_base_device::sh4_utlb_data_array1_r(offs_t offset)
{
	uint32_t ret = 0;
	int offs = offset*8;

	uint8_t i = (offs>>8)&63;

	ret |= m_utlb[i].PPN << 10;
	ret |= m_utlb[i].V << 8;
	ret |= (m_utlb[i].PSZ & 2) << 6;
	ret |= (m_utlb[i].PSZ & 1) << 4;
	ret |= m_utlb[i].PPR << 5;
	ret |= m_utlb[i].C << 3;
	ret |= m_utlb[i].D << 2;
	ret |= m_utlb[i].SH << 1;
	ret |= m_utlb[i].WT << 0;

	return ret;
}



void sh4_base_device::sh4_utlb_data_array2_w(offs_t offset, uint64_t data)
{
/*  uses bits 13:8 of address to select which UTLB entry we're addressing

    ---- ---- ---- ---- ---- ---- ---- TSSS

    T = TC = Timing Control
    S = SA = Space attributes
    - = unused (should be 0)

*/

	logerror("sh4_utlb_data_array2_w %08x %08x\n", offset, data);
	int offs = offset*8;

	uint8_t i = (offs>>8)&63;

	m_utlb[i].TC = (data & 0x00000008) >> 3;
	m_utlb[i].SA = (data & 0x00000007) >> 0;
}


uint64_t sh4_base_device::sh4_utlb_data_array2_r(offs_t offset)
{
	uint32_t ret = 0;
	int offs = offset*8;

	uint8_t i = (offs>>8)&63;

	ret |= m_utlb[i].TC << 3;
	ret |= m_utlb[i].SA << 0;

	return ret;
}
