// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*****************************************************************************
 *
 *   sh4comn.cpp
 *
 *   SH-4 non-specific components
 *
 *****************************************************************************/

#include "emu.h"
#include "sh4.h"
#include "sh4regs.h"
#include "sh4comn.h"
#include "sh3comn.h"
#include "sh4tmu.h"
#include "sh4dmac.h"

static const int rtcnt_div[8] = { 0, 4, 16, 64, 256, 1024, 2048, 4096 };
static const int daysmonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };



static const uint32_t exception_priority_default[] = {
	EXPPRI(1, 1, 0, 0),         /* Power-on Reset */
	EXPPRI(1, 2, 0, 1),         /* Manual Reset */
	EXPPRI(1, 1, 0, 2),         /* H-UDI Reset */
	EXPPRI(1, 3, 0, 3),         /* Inst TLB Multiple Hit */
	EXPPRI(1, 4, 0, 4),         /* Data TLB Multiple Hit */

	EXPPRI(2, 0, 0, 5),         /* User break Before Instruction */
	EXPPRI(2, 1, 0, 6),         /* Inst Address Error */
	EXPPRI(2, 2, 0, 7),         /* Inst TLB Miss */
	EXPPRI(2, 3, 0, 8),         /* Inst TLB Protection Violation */
	EXPPRI(2, 4, 0, 9),         /* Illegal Instruction */
	EXPPRI(2, 4, 0, 10),        /* Slot Illegal Instruction */
	EXPPRI(2, 4, 0, 11),        /* FPU Disable */
	EXPPRI(2, 4, 0, 12),        /* Slot FPU Disable */
	EXPPRI(2, 5, 0, 13),        /* Data Address Error (Read) */
	EXPPRI(2, 5, 0, 14),        /* Data Address Error (Write) */
	EXPPRI(2, 6, 0, 15),        /* Data TBL Miss Read */
	EXPPRI(2, 6, 0, 16),        /* Data TBL Miss Write */
	EXPPRI(2, 7, 0, 17),        /* Data TBL Protection Violation Read */
	EXPPRI(2, 7, 0, 18),        /* Data TBL Protection Violation Write */
	EXPPRI(2, 8, 0, 19),        /* FPU Exception */
	EXPPRI(2, 9, 0, 20),        /* Initial Page Write exception */

	EXPPRI(2, 4, 0, 21),        /* Unconditional TRAP */
	EXPPRI(2, 10, 0, 22),       /* User break After Instruction */

	EXPPRI(3, 0, 16, SH4_INTC_NMI) /* NMI */
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
	for (int s = 0; s <= 15; s++)
	{
		uint32_t z = m_sh2_state->m_fr[s];
		m_sh2_state->m_fr[s] = m_sh2_state->m_xf[s];
		m_sh2_state->m_xf[s] = z;
	}
}

void sh34_base_device::sh4_swap_fp_couples()
{
	for (int s = 0; s <= 15; s += 2)
	{
		uint32_t z = m_sh2_state->m_fr[s];
		m_sh2_state->m_fr[s] = m_sh2_state->m_fr[s + 1];
		m_sh2_state->m_fr[s + 1] = z;

		z = m_sh2_state->m_xf[s];
		m_sh2_state->m_xf[s] = m_sh2_state->m_xf[s + 1];
		m_sh2_state->m_xf[s + 1] = z;
	}
}


void sh34_base_device::sh4_change_register_bank(int to)
{
	if (to) // 0 -> 1
	{
		for (int s = 0; s < 8; s++)
		{
			m_sh2_state->m_rbnk[0][s] = m_sh2_state->r[s];
			m_sh2_state->r[s] = m_sh2_state->m_rbnk[1][s];
		}
	}
	else // 1 -> 0
	{
		for (int s = 0; s < 8; s++)
		{
			m_sh2_state->m_rbnk[1][s] = m_sh2_state->r[s];
			m_sh2_state->r[s] = m_sh2_state->m_rbnk[0][s];
		}
	}
}

void sh34_base_device::sh4_syncronize_register_bank(int to)
{
	for (int s = 0; s < 8; s++)
	{
		m_sh2_state->m_rbnk[to][s] = m_sh2_state->r[s];
	}
}

void sh34_base_device::sh4_default_exception_priorities() // setup default priorities for exceptions
{
	for (int a = 0; a <= SH4_INTC_NMI; a++)
		m_exception_priority[a] = exception_priority_default[a];
	for (int a = SH4_INTC_IRLn0; a <= SH4_INTC_IRLnE; a++)
		m_exception_priority[a] = INTPRI(15-(a - SH4_INTC_IRLn0), a);
	m_exception_priority[SH4_INTC_IRL0] = INTPRI(13, SH4_INTC_IRL0);
	m_exception_priority[SH4_INTC_IRL1] = INTPRI(10, SH4_INTC_IRL1);
	m_exception_priority[SH4_INTC_IRL2] = INTPRI(7, SH4_INTC_IRL2);
	m_exception_priority[SH4_INTC_IRL3] = INTPRI(4, SH4_INTC_IRL3);
	for (int a = SH4_INTC_HUDI; a <= SH4_INTC_ROVI; a++)
		m_exception_priority[a] = INTPRI(0, a);
}

void sh34_base_device::sh4_exception_recompute() // checks if there is any interrupt with high enough priority
{
	m_sh2_state->m_test_irq = 0;
	if (!m_sh2_state->m_pending_irq || ((m_sh2_state->sr & BL) && m_exception_requesting[SH4_INTC_NMI] == 0))
		return;
	int z = (m_sh2_state->sr >> 4) & 15;
	for (int a = 0; a <= SH4_INTC_ROVI; a++)
	{
		if (m_exception_requesting[a])
		{
			int pri = ((int)m_exception_priority[a] >> 8) & 255;
			//logerror("pri is %02x z is %02x\n", pri, z);
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
	if (exception == SH4_INTC_DMTE0 || exception == SH4_INTC_DMTE1 || exception == SH4_INTC_DMTE2 || exception == SH4_INTC_DMTE3)
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
	if (debugger_enabled())
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
	if (m_sh2_state->sleep_mode == 1)
	{
		m_sh2_state->sleep_mode = 2;
	}
}

void sh34_base_device::sh4_exception(const char *message, int exception) // handle exception
{
	uint32_t vector;

	if (m_cpu_type == CPU_TYPE_SH4)
	{
		if (exception < SH4_INTC_NMI)
			return; // Not yet supported

		if (exception == SH4_INTC_NMI)
		{
			if ((m_sh2_state->sr & BL) && !(m_icr & 0x200))
				return;

			m_icr &= ~0x200;
			m_intevt = 0x1c0;

			vector = 0x600;
			standard_irq_callback(INPUT_LINE_NMI, m_sh2_state->pc);
			LOG("SH-4 '%s' nmi exception after [%s]\n", tag(), message);
		}
		else
		{
	//      if ((m_icr & 0x4000) && (m_nmi_line_state == ASSERT_LINE))
	//          return;
			if (m_sh2_state->sr & BL)
				return;
			if (((m_exception_priority[exception] >> 8) & 255) <= ((m_sh2_state->sr >> 4) & 15))
				return;
			m_intevt = exception_codes[exception];
			vector = 0x600;
			if (exception >= SH4_INTC_IRL0 && exception <= SH4_INTC_IRL3)
				standard_irq_callback((exception - SH4_INTC_IRL0) + SH4_IRL0, m_sh2_state->pc);
			else
				standard_irq_callback(SH4_IRL3 + 1, m_sh2_state->pc);
			LOG("SH-4 '%s' interrupt exception #%d after [%s]\n", tag(), exception, message);
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

			if (exception >= SH4_INTC_IRL0 && exception <= SH4_INTC_IRL3)
				standard_irq_callback((exception - SH4_INTC_IRL0) + SH4_IRL0, m_sh2_state->pc);
			else
				standard_irq_callback(SH4_IRL3 + 1, m_sh2_state->pc);

			if (sh3_intevt2_exception_codes[exception] == -1)
				fatalerror("sh3_intevt2_exception_codes unpopulated for exception %02x\n", exception);

			m_intevt2 = sh3_intevt2_exception_codes[exception];
			m_expevt = exception_codes[exception];
			if (sh3_intevt2_exception_codes[exception] >= 0x600)
				m_intevt = 0x3e0 - ((m_exception_priority[exception] >> 8) & 255) * 0x20;
			else
				m_intevt = sh3_intevt2_exception_codes[exception];


			LOG("SH-3 '%s' interrupt exception #%d after [%s]\n", tag(), exception, message);
		}

		/***** END ASSUME THIS TO BE WRONG FOR NOW *****/
	}

	sh4_exception_process(exception, vector);
}


uint32_t sh4_base_device::compute_ticks_refresh_timer(emu_timer *timer, int hertz, int base, int divisor)
{
	// elapsed:total = x : ticks
	// x=elapsed*tics/total -> x=elapsed*(double)100000000/rtcnt_div[(m_rtcsr >> 3) & 7]
	// ticks/total=ticks / ((rtcnt_div[(m_rtcsr >> 3) & 7] * ticks) / 100000000)=1/((rtcnt_div[(m_rtcsr >> 3) & 7] / 100000000)=100000000/rtcnt_div[(m_rtcsr >> 3) & 7]
	return base + (uint32_t)((timer->elapsed().as_double() * hertz) / divisor);
}

void sh4_base_device::sh4_refresh_timer_recompute()
{
	//if rtcnt < rtcor then rtcor-rtcnt
	//if rtcnt >= rtcor then 256-rtcnt+rtcor=256+rtcor-rtcnt
	uint32_t ticks = m_rtcor-m_rtcnt;
	if (ticks <= 0)
		ticks = 256 + ticks;
	m_refresh_timer->adjust(attotime::from_hz(m_bus_clock) * rtcnt_div[(m_rtcsr >> 3) & 7] * ticks);
	m_refresh_timer_base = m_rtcnt;
}


TIMER_CALLBACK_MEMBER(sh4_base_device::sh4_refresh_timer_callback)
{
	m_rtcnt = 0;
	sh4_refresh_timer_recompute();
	m_rtcsr |= 128;
	if ((m_mcr & 4) && !(m_mcr & 2))
	{
		m_rfcr = (m_rfcr + 1) & 1023;
		if (((m_rtcsr & 1) && m_rfcr == 512) || m_rfcr == 0)
		{
			m_rfcr = 0;
			m_rtcsr |= 4;
		}
	}
}

void sh4_base_device::increment_rtc_time(int mode)
{
	int carry;
	if (mode == 0)
	{
		carry = 0;
		m_rseccnt = m_rseccnt + 1;
		if ((m_rseccnt & 0xf) == 0xa)
			m_rseccnt = m_rseccnt + 6;
		if (m_rseccnt == 0x60)
		{
			m_rseccnt = 0;
			carry = 1;
		}
		else
			return;
	}
	else
		carry = 1;

	m_rmincnt = m_rmincnt + carry;
	if ((m_rmincnt & 0xf) == 0xa)
		m_rmincnt = m_rmincnt + 6;
	carry = 0;

	if (m_rmincnt == 0x60)
	{
		m_rmincnt = 0;
		carry = 1;
	}

	m_rhrcnt = m_rhrcnt + carry;
	if ((m_rhrcnt & 0xf) == 0xa)
		m_rhrcnt = m_rhrcnt + 6;
	carry = 0;

	if (m_rhrcnt == 0x24)
	{
		m_rhrcnt = 0;
		carry = 1;
	}

	m_rwkcnt = m_rwkcnt + carry;
	if (m_rwkcnt == 0x7)
	{
		m_rwkcnt = 0;
	}

	int days = 0;
	int year = (m_ryrcnt & 0xf) + ((m_ryrcnt & 0xf0) >> 4) * 10 + ((m_ryrcnt & 0xf00) >> 8) * 100 + ((m_ryrcnt & 0xf000) >> 12) * 1000;
	int leap = 0;
	if (!(year % 100))
	{
		if (!(year % 400))
			leap = 1;
	}
	else if (!(year % 4))
		leap = 1;
	if (m_rmoncnt != 2)
		leap = 0;
	if (m_rmoncnt)
		days = daysmonth[(m_rmoncnt & 0xf) + ((m_rmoncnt & 0xf0) >> 4) * 10 - 1];

	m_rdaycnt = m_rdaycnt + carry;
	if ((m_rdaycnt & 0xf) == 0xa)
		m_rdaycnt = m_rdaycnt + 6;
	carry = 0;

	if (m_rdaycnt > (days + leap))
	{
		m_rdaycnt = 1;
		carry = 1;
	}

	m_rmoncnt = m_rmoncnt + carry;
	if ((m_rmoncnt & 0xf) == 0xa)
		m_rmoncnt = m_rmoncnt + 6;
	carry = 0;

	if (m_rmoncnt == 0x13)
	{
		m_rmoncnt = 1;
		carry = 1;
	}

	m_ryrcnt = m_ryrcnt + carry;
	if ((m_ryrcnt & 0xf) >= 0xa)
		m_ryrcnt = m_ryrcnt + 6;
	if ((m_ryrcnt & 0xf0) >= 0xa0)
		m_ryrcnt = m_ryrcnt + 0x60;
	if ((m_ryrcnt & 0xf00) >= 0xa00)
		m_ryrcnt = m_ryrcnt + 0x600;
	if ((m_ryrcnt & 0xf000) >= 0xa000)
		m_ryrcnt = 0;
}

TIMER_CALLBACK_MEMBER(sh4_base_device::sh4_rtc_timer_callback)
{
	m_rtc_timer->adjust(attotime::from_hz(128));

	m_r64cnt = (m_r64cnt + 1) & 0x7f;
	if (m_r64cnt == 64)
	{
		m_rcr1 |= 0x80;
		increment_rtc_time(0);
		//sh4_exception_request(SH4_INTC_NMI); // TEST
	}
}


void sh34_base_device::sh4_dmac_nmi() // manage dma when nmi gets asserted
{
	int s;

	m_dmaor |= DMAOR_NMIF;
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

// CCN
uint32_t sh4_base_device::pteh_r(offs_t offset, uint32_t mem_mask)
{
	return m_pteh;
}

void sh4_base_device::pteh_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// for use with LDTLB opcode
	/*
	    NNNN NNNN NNNN NNNN NNNN NN-- AAAA AAAA

	    N = VPM = Virtual Page Number
	    A = ASID = Address Space Identifier

	    same as the address table part of the utlb but with 2 unused bits (these are sourced from PTEL instead when LDTLB is called)
	*/
	COMBINE_DATA(&m_pteh);
}

uint32_t sh4_base_device::ptel_r(offs_t offset, uint32_t mem_mask)
{
	return m_ptel;
}

void sh4_base_device::ptel_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/*
	        ---P PPPP PPPP PPPP PPPP PP-V zRRz CDHW

	        same format as data array 1 of the utlb
	*/
	COMBINE_DATA(&m_ptel);
}

uint32_t sh4_base_device::ttb_r(offs_t offset, uint32_t mem_mask)
{
	return m_ttb;
}

void sh4_base_device::ttb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ttb);
	logerror("TTB set to %08x\n", m_ttb);
}

uint32_t sh4_base_device::tea_r(offs_t offset, uint32_t mem_mask)
{
	return m_tea;
}

void sh4_base_device::tea_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tea);
	logerror("TEA set to %08x\n", m_tea);
}

uint32_t sh4_base_device::mmucr_r(offs_t offset, uint32_t mem_mask)
{
	return m_mmucr;
}

void sh4_base_device::mmucr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// MMU Control
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
	COMBINE_DATA(&m_mmucr);
	// MMUCR_AT
	m_sh4_mmu_enabled = BIT(data, 0);
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
}

uint8_t sh4_base_device::basra_r(offs_t offset, uint8_t mem_mask)
{
	return m_basra;
}

void sh4_base_device::basra_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_basra);
	logerror("basra_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::basrb_r(offs_t offset, uint8_t mem_mask)
{
	return m_basrb;
}

void sh4_base_device::basrb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_basrb);
	logerror("basrb_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint32_t sh4_base_device::ccr_r(offs_t offset, uint32_t mem_mask)
{
	return m_ccr;
}

void sh4_base_device::ccr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ccr);
	logerror("ccr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::tra_r(offs_t offset, uint32_t mem_mask)
{
	return m_tra;
}

void sh4_base_device::tra_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tra);
	logerror("tra_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::expevt_r(offs_t offset, uint32_t mem_mask)
{
	return m_expevt;
}

void sh4_base_device::expevt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_expevt);
	logerror("expevt: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::intevt_r(offs_t offset, uint32_t mem_mask)
{
	return m_intevt;
}

void sh4_base_device::intevt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intevt);
	logerror("intevt_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::ptea_r(offs_t offset, uint32_t mem_mask)
{
	return m_ptea;
}

void sh4_base_device::ptea_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	/*
	    ---- ---- ---- ---- ---- ---- ---- TSSS

	    same format as data array 2 of the utlb
	*/
	COMBINE_DATA(&m_ptea);
}

uint32_t sh4_base_device::qacr0_r(offs_t offset, uint32_t mem_mask)
{
	return m_qacr0;
}

void sh4_base_device::qacr0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_qacr0);
}

uint32_t sh4_base_device::qacr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_qacr1;
}

void sh4_base_device::qacr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_qacr1);
}

// UBC
uint32_t sh4_base_device::bara_r(offs_t offset, uint32_t mem_mask)
{
	return m_bara;
}

void sh4_base_device::bara_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bara);
	logerror("bara_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint8_t sh4_base_device::bamra_r(offs_t offset, uint8_t mem_mask)
{
	return m_bamra;
}

void sh4_base_device::bamra_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_bamra);
	logerror("bamra_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint16_t sh4_base_device::bbra_r(offs_t offset, uint16_t mem_mask)
{
	return m_bbra;
}

void sh4_base_device::bbra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bbra);
	logerror("bbra_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint32_t sh4_base_device::barb_r(offs_t offset, uint32_t mem_mask)
{
	return m_barb;
}

void sh4_base_device::barb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_barb);
	logerror("barb_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint8_t sh4_base_device::bamrb_r(offs_t offset, uint8_t mem_mask)
{
	return m_bamrb;
}

void sh4_base_device::bamrb_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_bamrb);
	logerror("bamrb_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint16_t sh4_base_device::bbrb_r(offs_t offset, uint16_t mem_mask)
{
	return m_bbrb;
}

void sh4_base_device::bbrb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bbrb);
	logerror("bbrb_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint32_t sh4_base_device::bdrb_r(offs_t offset, uint32_t mem_mask)
{
	return m_bdrb;
}

void sh4_base_device::bdrb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bdrb);
	logerror("bdrb_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::bdmrb_r(offs_t offset, uint32_t mem_mask)
{
	return m_bdmrb;
}

void sh4_base_device::bdmrb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bdmrb);
	logerror("bdmrb_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint16_t sh4_base_device::brcr_r(offs_t offset, uint16_t mem_mask)
{
	return m_brcr;
}

void sh4_base_device::brcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_brcr);
	logerror("brcr_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

// BSC
uint32_t sh4_base_device::bcr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_bcr1;
}

void sh4_base_device::bcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bcr1);
	logerror("bcr1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint16_t sh4_base_device::bcr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_bcr2;
}

void sh4_base_device::bcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr2);
	logerror("bcr2_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint32_t sh4_base_device::wcr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_wcr1;
}

void sh4_base_device::wcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_wcr1);
	logerror("wcr1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::wcr2_r(offs_t offset, uint32_t mem_mask)
{
	return m_wcr2;
}

void sh4_base_device::wcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_wcr2);
	logerror("wcr2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::wcr3_r(offs_t offset, uint32_t mem_mask)
{
	return m_wcr3;
}

void sh4_base_device::wcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_wcr3);
	logerror("wcr3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::mcr_r(offs_t offset, uint32_t mem_mask)
{
	return m_mcr;
}

void sh4_base_device::mcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_mcr);
	logerror("mcr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint16_t sh4_base_device::pcr_r(offs_t offset, uint16_t mem_mask)
{
	return m_pcr;
}

void sh4_base_device::pcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pcr);
	logerror("pcr_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint16_t sh4_base_device::rtcsr_r(offs_t offset, uint16_t mem_mask)
{
	return m_rtcsr;
}

void sh4_base_device::rtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	// Memory refresh
	uint16_t old = m_rtcsr;
	COMBINE_DATA(&m_rtcsr);
	m_rtcsr &= 255;
	if ((old >> 3) & 7)
		m_rtcnt = compute_ticks_refresh_timer(m_refresh_timer, m_bus_clock, m_refresh_timer_base, rtcnt_div[(old >> 3) & 7]) & 0xff;
	if ((m_rtcsr >> 3) & 7)
	{   // activated
		sh4_refresh_timer_recompute();
	}
	else
	{
		m_refresh_timer->adjust(attotime::never);
	}
}

uint16_t sh4_base_device::rtcnt_r(offs_t offset, uint16_t mem_mask)
{
	if ((m_rtcsr >> 3) & 7)
	{ // activated
		//((double)rtcnt_div[(m_rtcsr >> 3) & 7] / (double)100000000)
		//return (refresh_timer_base + (m_refresh_timer->elapsed() * (double)100000000) / (double)rtcnt_div[(m_rtcsr >> 3) & 7]) & 0xff;
		return compute_ticks_refresh_timer(m_refresh_timer, m_bus_clock, m_refresh_timer_base, rtcnt_div[(m_rtcsr >> 3) & 7]) & 0xff;
	}
	else
		return m_rtcnt;
}

void sh4_base_device::rtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcnt);
	m_rtcnt &= 255;
	if ((m_rtcsr >> 3) & 7)
	{   // active
		sh4_refresh_timer_recompute();
	}
}

uint16_t sh4_base_device::rtcor_r(offs_t offset, uint16_t mem_mask)
{
	return m_rtcor;
}

void sh4_base_device::rtcor_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rtcor);
	m_rtcor &= 255;
	if ((m_rtcsr >> 3) & 7)
	{   // active
		m_rtcnt = compute_ticks_refresh_timer(m_refresh_timer, m_bus_clock, m_refresh_timer_base, rtcnt_div[(m_rtcsr >> 3) & 7]) & 0xff;
		sh4_refresh_timer_recompute();
	}
}

uint16_t sh4_base_device::rfcr_r(offs_t offset, uint16_t mem_mask)
{
	return m_rfcr;
}

void sh4_base_device::rfcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_rfcr);
	m_rfcr &= 1023;
}

uint32_t sh4_base_device::pctra_r(offs_t offset, uint32_t mem_mask)
{
	return m_pctra;
}

void sh4_base_device::pctra_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pctra);
	m_ioport16_pullup = 0;
	m_ioport16_direction = 0;
	for (int a = 0; a < 16; a++)
	{
		m_ioport16_direction |= (m_pctra & (1 << (a * 2))) >> a;
		m_ioport16_pullup |= (m_pctra & (1 << (a * 2 + 1))) >> (a + 1);
	}
	m_ioport16_direction &= 0xffff;
	m_ioport16_pullup = (m_ioport16_pullup | m_ioport16_direction) ^ 0xffff;
	if (m_bcr2 & 1)
		m_io->write_dword(SH4_IOPORT_16, (uint64_t)(m_pdtra & m_ioport16_direction) | ((uint64_t)m_pctra << 16));
}

uint16_t sh4_base_device::pdtra_r(offs_t offset, uint16_t mem_mask)
{
	if (m_bcr2 & 1)
		return (m_io->read_dword(SH4_IOPORT_16) & ~m_ioport16_direction) | (m_pdtra & m_ioport16_direction);

	return m_pdtra;
}

void sh4_base_device::pdtra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pdtra);
	if (m_bcr2 & 1)
		m_io->write_dword(SH4_IOPORT_16, (uint64_t)(m_pdtra & m_ioport16_direction) | ((uint64_t)m_pctra << 16));
}

uint32_t sh4_base_device::pctrb_r(offs_t offset, uint32_t mem_mask)
{
	return m_pctrb;
}

void sh4_base_device::pctrb_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pctrb);
	m_ioport4_pullup = 0;
	m_ioport4_direction = 0;
	for (int a = 0; a < 4; a++)
	{
		m_ioport4_direction |= (m_pctrb & (1 << (a * 2))) >> a;
		m_ioport4_pullup |= (m_pctrb & (1 << (a * 2 + 1))) >> (a + 1);
	}
	m_ioport4_direction &= 0xf;
	m_ioport4_pullup = (m_ioport4_pullup | m_ioport4_direction) ^ 0xf;
	if (m_bcr2 & 1)
		m_io->write_dword(SH4_IOPORT_4, (m_pdtrb & m_ioport4_direction) | (m_pctrb << 16));
}

uint16_t sh4_base_device::pdtrb_r(offs_t offset, uint16_t mem_mask)
{
	if (m_bcr2 & 1)
		return (m_io->read_dword(SH4_IOPORT_4) & ~m_ioport4_direction) | (m_pdtrb & m_ioport4_direction);

	return m_pdtrb;
}

void sh4_base_device::pdtrb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_pdtrb);
	if (m_bcr2 & 1)
		m_io->write_dword(SH4_IOPORT_4, (m_pdtrb & m_ioport4_direction) | (m_pctrb << 16));
}

uint16_t sh4_base_device::gpioic_r(offs_t offset, uint16_t mem_mask)
{
	return m_gpioic;
}

void sh4_base_device::gpioic_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_gpioic);
	logerror("gpioic_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

void sh4_base_device::sdmr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	logerror("sdmr2_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

void sh4_base_device::sdmr3_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	logerror("sdmr3_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

// BSC 7750R
uint16_t sh4_base_device::bcr3_r(offs_t offset, uint16_t mem_mask)
{
	return m_bcr3;
}

void sh4_base_device::bcr3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bcr3);
	logerror("bcr3_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint32_t sh4_base_device::bcr4_r(offs_t offset, uint32_t mem_mask)
{
	return m_bcr4;
}

void sh4_base_device::bcr4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_bcr4);
	logerror("bcr4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

// DMAC 7750R
uint32_t sh4_base_device::sar4_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar4;
}

void sh4_base_device::sar4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar4);
	logerror("sar4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dar4_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar4;
}

void sh4_base_device::dar4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar4);
	logerror("dar4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dmatcr4_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr4;
}

void sh4_base_device::dmatcr4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr4);
	logerror("dmatcr4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::chcr4_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr4;
}

void sh4_base_device::chcr4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr4);
	logerror("chcr4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::sar5_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar5;
}

void sh4_base_device::sar5_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar5);
	logerror("sar5_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dar5_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar5;
}

void sh4_base_device::dar5_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar5);
	logerror("dar5_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dmatcr5_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr5;
}

void sh4_base_device::dmatcr5_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr5);
	logerror("dmatcr5_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::chcr5_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr5;
}

void sh4_base_device::chcr5_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr5);
	logerror("chcr5_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::sar6_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar6;
}

void sh4_base_device::sar6_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar6);
	logerror("sar6_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dar6_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar6;
}

void sh4_base_device::dar6_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar6);
	logerror("dar6_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dmatcr6_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr6;
}

void sh4_base_device::dmatcr6_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr6);
	logerror("dmatcr6_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::chcr6_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr6;
}

void sh4_base_device::chcr6_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr6);
	logerror("chcr6_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::sar7_r(offs_t offset, uint32_t mem_mask)
{
	return m_sar7;
}

void sh4_base_device::sar7_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sar7);
	logerror("sar7_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dar7_r(offs_t offset, uint32_t mem_mask)
{
	return m_dar7;
}

void sh4_base_device::dar7_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dar7);
	logerror("dar7_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::dmatcr7_r(offs_t offset, uint32_t mem_mask)
{
	return m_dmatcr7;
}

void sh4_base_device::dmatcr7_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_dmatcr7);
	logerror("dmatcr7_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::chcr7_r(offs_t offset, uint32_t mem_mask)
{
	return m_chcr7;
}

void sh4_base_device::chcr7_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_chcr7);
	logerror("chcr7_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

// CPG
uint16_t sh4_base_device::frqcr_r(offs_t offset, uint16_t mem_mask)
{
	return m_frqcr;
}

void sh4_base_device::frqcr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_frqcr);
	logerror("frqcr_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint8_t sh4_base_device::stbcr_r(offs_t offset, uint8_t mem_mask)
{
	return m_stbcr;
}

void sh4_base_device::stbcr_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_stbcr);
	logerror("stbcr_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::wtcnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_wtcnt;
}

void sh4_base_device::wtcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wtcnt);
	logerror("wtcnt_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint8_t sh4_base_device::wtcsr_r(offs_t offset, uint8_t mem_mask)
{
	return m_wtcsr;
}

void sh4_base_device::wtcsr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_wtcsr);
	logerror("wtcsr_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint8_t sh4_base_device::stbcr2_r(offs_t offset, uint8_t mem_mask)
{
	return m_stbcr2;
}

void sh4_base_device::stbcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_stbcr2);
	logerror("stbcr2_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

// CPG 7750R
uint32_t sh4_base_device::clkstp00_r(offs_t offset, uint32_t mem_mask)
{
	return m_clkstp00;
}

void sh4_base_device::clkstp00_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_clkstp00);
	logerror("clkstp00_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

void sh4_base_device::clkstpclr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("clkstpclr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

// RTC
uint8_t sh4_base_device::r64cnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_r64cnt;
}

uint8_t sh4_base_device::rseccnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_rseccnt;
}

void sh4_base_device::rseccnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rseccnt);
}

uint8_t sh4_base_device::rmincnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_rmincnt;
}

void sh4_base_device::rmincnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rmincnt);
}

uint8_t sh4_base_device::rhrcnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_rhrcnt;
}

void sh4_base_device::rhrcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rhrcnt);
}

uint8_t sh4_base_device::rwkcnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_rwkcnt;
}

void sh4_base_device::rwkcnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rwkcnt);
}

uint8_t sh4_base_device::rdaycnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_rdaycnt;
}

void sh4_base_device::rdaycnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rdaycnt);
}

uint8_t sh4_base_device::rmoncnt_r(offs_t offset, uint8_t mem_mask)
{
	return m_rmoncnt;
}

void sh4_base_device::rmoncnt_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rmoncnt);
}

uint16_t sh4_base_device::ryrcnt_r(offs_t offset, uint16_t mem_mask)
{
	return m_ryrcnt;
}

void sh4_base_device::ryrcnt_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ryrcnt);
}

uint8_t sh4_base_device::rsecar_r(offs_t offset, uint8_t mem_mask)
{
	return m_rsecar;
}

void sh4_base_device::rsecar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rsecar);
	logerror("rsecar_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::rminar_r(offs_t offset, uint8_t mem_mask)
{
	return m_rminar;
}

void sh4_base_device::rminar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rminar);
	logerror("rminar_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::rhrar_r(offs_t offset, uint8_t mem_mask)
{
	return m_rhrar;
}

void sh4_base_device::rhrar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rhrar);
	logerror("rhrar_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::rwkar_r(offs_t offset, uint8_t mem_mask)
{
	return m_rwkar;
}

void sh4_base_device::rwkar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rwkar);
	logerror("rwkar_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::rdayar_r(offs_t offset, uint8_t mem_mask)
{
	return m_rdayar;
}

void sh4_base_device::rdayar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rdayar);
	logerror("rdayar_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::rmonar_r(offs_t offset, uint8_t mem_mask)
{
	return m_rmonar;
}

void sh4_base_device::rmonar_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rmonar);
	logerror("rmonar_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::rcr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_rcr1;
}

void sh4_base_device::rcr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	uint8_t old = m_rcr1;
	COMBINE_DATA(&m_rcr1);
	if ((m_rcr1 & 8) && (~old & 8)) // 0 -> 1
		m_rcr1 ^= 1;
}

uint8_t sh4_base_device::rcr2_r(offs_t offset, uint8_t mem_mask)
{
	return m_rcr2;
}

void sh4_base_device::rcr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	uint8_t old = m_rcr2;
	COMBINE_DATA(&m_rcr2);
	if (m_rcr2 & 2)
	{
		m_r64cnt = 0;
		m_rcr2 ^= 2;
	}
	if (m_rcr2 & 4)
	{
		m_r64cnt = 0;
		if (m_rseccnt >= 30)
			increment_rtc_time(1);
		m_rseccnt = 0;
	}
	if ((m_rcr2 & 8) && (~old & 8))
	{   // 0 -> 1
		m_rtc_timer->adjust(attotime::from_hz(128));
	}
	else if (~m_rcr2 & 8)
	{   // 0
		m_rtc_timer->adjust(attotime::never);
	}
}

// RTC 7750R
uint8_t sh4_base_device::rcr3_r(offs_t offset, uint8_t mem_mask)
{
	return m_rcr3;
}

void sh4_base_device::rcr3_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_rcr3);
	logerror("rcr3_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint16_t sh4_base_device::ryrar_r(offs_t offset, uint16_t mem_mask)
{
	return m_ryrar;
}

void sh4_base_device::ryrar_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ryrar);
	logerror("ryrar_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

// INTC
uint16_t sh4_base_device::icr_r(offs_t offset, uint16_t mem_mask)
{
	return m_icr;
}

void sh4_base_device::icr_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t old = m_icr;
	COMBINE_DATA(&m_icr);
	m_icr = (m_icr & 0x7fff) | (old & 0x8000);
}

uint16_t sh34_base_device::ipra_r(offs_t offset, uint16_t mem_mask)
{
	return m_ipra;
}

void sh34_base_device::ipra_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_ipra);
	/* 15 - 12 TMU0 */
	/* 11 -  8 TMU1 */
	/*  7 -  4 TMU2 */
	/*  3 -  0 RTC  */
	m_exception_priority[SH4_INTC_ATI] = INTPRI(m_ipra & 0x000f, SH4_INTC_ATI);
	m_exception_priority[SH4_INTC_PRI] = INTPRI(m_ipra & 0x000f, SH4_INTC_PRI);
	m_exception_priority[SH4_INTC_CUI] = INTPRI(m_ipra & 0x000f, SH4_INTC_CUI);

	m_exception_priority[SH4_INTC_TUNI2] = INTPRI((m_ipra & 0x00f0) >> 4, SH4_INTC_TUNI2);
	m_exception_priority[SH4_INTC_TICPI2] = INTPRI((m_ipra & 0x00f0) >> 4, SH4_INTC_TICPI2);

	m_exception_priority[SH4_INTC_TUNI1] = INTPRI((m_ipra & 0x0f00) >> 8, SH4_INTC_TUNI1);

	m_exception_priority[SH4_INTC_TUNI0] = INTPRI((m_ipra & 0xf000) >> 12, SH4_INTC_TUNI0);

	logerror("setting priorities TMU0 %01x TMU1 %01x TMU2 %01x RTC %01x\n", (m_ipra & 0xf000) >> 12, (m_ipra & 0x0f00) >> 8, (m_ipra & 0x00f0) >> 4, (m_ipra & 0x000f) >> 0);

	sh4_exception_recompute();
}

uint16_t sh4_base_device::iprb_r(offs_t offset, uint16_t mem_mask)
{
	return m_iprb;
}

void sh4_base_device::iprb_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprb);
	m_exception_priority[SH4_INTC_SCI1ERI] = INTPRI((m_iprb & 0x00f0) >> 4, SH4_INTC_SCI1ERI);
	m_exception_priority[SH4_INTC_SCI1RXI] = INTPRI((m_iprb & 0x00f0) >> 4, SH4_INTC_SCI1RXI);
	m_exception_priority[SH4_INTC_SCI1TXI] = INTPRI((m_iprb & 0x00f0) >> 4, SH4_INTC_SCI1TXI);
	m_exception_priority[SH4_INTC_SCI1TEI] = INTPRI((m_iprb & 0x00f0) >> 4, SH4_INTC_SCI1TEI);
	m_exception_priority[SH4_INTC_RCMI] = INTPRI((m_iprb & 0x0f00) >> 8, SH4_INTC_RCMI);
	m_exception_priority[SH4_INTC_ROVI] = INTPRI((m_iprb & 0x0f00) >> 8, SH4_INTC_ROVI);
	m_exception_priority[SH4_INTC_ITI] = INTPRI((m_iprb & 0xf000) >> 12, SH4_INTC_ITI);
	sh4_exception_recompute();
}

uint16_t sh4_base_device::iprc_r(offs_t offset, uint16_t mem_mask)
{
	return m_iprc;
}

void sh4_base_device::iprc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprc);
	m_exception_priority[SH4_INTC_HUDI] = INTPRI(m_iprc & 0x000f, SH4_INTC_HUDI);
	m_exception_priority[SH4_INTC_SCIFERI] = INTPRI((m_iprc & 0x00f0) >> 4, SH4_INTC_SCIFERI);
	m_exception_priority[SH4_INTC_SCIFRXI] = INTPRI((m_iprc & 0x00f0) >> 4, SH4_INTC_SCIFRXI);
	m_exception_priority[SH4_INTC_SCIFBRI] = INTPRI((m_iprc & 0x00f0) >> 4, SH4_INTC_SCIFBRI);
	m_exception_priority[SH4_INTC_SCIFTXI] = INTPRI((m_iprc & 0x00f0) >> 4, SH4_INTC_SCIFTXI);
	m_exception_priority[SH4_INTC_DMTE0] = INTPRI((m_iprc & 0x0f00) >> 8, SH4_INTC_DMTE0);
	m_exception_priority[SH4_INTC_DMTE1] = INTPRI((m_iprc & 0x0f00) >> 8, SH4_INTC_DMTE1);
	m_exception_priority[SH4_INTC_DMTE2] = INTPRI((m_iprc & 0x0f00) >> 8, SH4_INTC_DMTE2);
	m_exception_priority[SH4_INTC_DMTE3] = INTPRI((m_iprc & 0x0f00) >> 8, SH4_INTC_DMTE3);
	m_exception_priority[SH4_INTC_DMAE] = INTPRI((m_iprc & 0x0f00) >> 8, SH4_INTC_DMAE);
	m_exception_priority[SH4_INTC_GPOI] = INTPRI((m_iprc & 0xf000) >> 12, SH4_INTC_GPOI);
	sh4_exception_recompute();
}

// INTC 7750S
uint16_t sh4_base_device::iprd_r(offs_t offset, uint16_t mem_mask)
{
	return m_iprd;
}

void sh4_base_device::iprd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_iprd);
	logerror("iprd_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

// INTC 7750R
uint32_t sh4_base_device::intpri00_r(offs_t offset, uint32_t mem_mask)
{
	return m_intpri00;
}

void sh4_base_device::intpri00_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intpri00);
	logerror("intpri00_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::intreq00_r(offs_t offset, uint32_t mem_mask)
{
	return m_intreq00;
}

void sh4_base_device::intreq00_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intreq00);
	logerror("intreq00_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::intmsk00_r(offs_t offset, uint32_t mem_mask)
{
	return m_intmsk00;
}

void sh4_base_device::intmsk00_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_intmsk00);
	logerror("intmsk00_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

void sh4_base_device::intmskclr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	logerror("intmskclr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

// TMU 7750R
uint8_t sh4_base_device::tstr2_r(offs_t offset, uint8_t mem_mask)
{
	return m_tstr2;
}

void sh4_base_device::tstr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_tstr2);
	logerror("tstr2_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint32_t sh4_base_device::tcor3_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcor3;
}

void sh4_base_device::tcor3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcor3);
	logerror("tcor3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::tcnt3_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcnt3;
}

void sh4_base_device::tcnt3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcnt3);
	logerror("tcnt3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint16_t sh4_base_device::tcr3_r(offs_t offset, uint16_t mem_mask)
{
	return m_tcr3;
}

void sh4_base_device::tcr3_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tcr3);
	logerror("tcr3_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint32_t sh4_base_device::tcor4_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcor4;
}

void sh4_base_device::tcor4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcor4);
	logerror("tcor4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::tcnt4_r(offs_t offset, uint32_t mem_mask)
{
	return m_tcnt4;
}

void sh4_base_device::tcnt4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_tcnt4);
	logerror("tcnt4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint16_t sh4_base_device::tcr4_r(offs_t offset, uint16_t mem_mask)
{
	return m_tcr4;
}

void sh4_base_device::tcr4_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tcr4);
	logerror("tcr4_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

// SCI
uint8_t sh4_base_device::scsmr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_scsmr1;
}

void sh4_base_device::scsmr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scsmr1);
	logerror("scsmr1_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::scbrr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_scbrr1;
}

void sh4_base_device::scbrr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scbrr1);
	logerror("scbrr1_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::scscr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_scscr1;
}

void sh4_base_device::scscr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scscr1);
	logerror("scscr1_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::sctdr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_sctdr1;
}

void sh4_base_device::sctdr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_sctdr1);
	logerror("sctdr1_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::scssr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_scssr1;
}

void sh4_base_device::scssr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scssr1);
	logerror("scssr1_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::scrdr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_scrdr1;
}

uint8_t sh4_base_device::scscmr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_scscmr1;
}

void sh4_base_device::scscmr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scscmr1);
	logerror("scscmr1_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint8_t sh4_base_device::scsptr1_r(offs_t offset, uint8_t mem_mask)
{
	return m_scsptr1;
}

void sh4_base_device::scsptr1_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scsptr1);
	logerror("scsptr1_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

// SCIF
uint16_t sh4_base_device::scsmr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_scsmr2;
}

void sh4_base_device::scsmr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scsmr2);
	logerror("scsmr2_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint8_t sh4_base_device::scbrr2_r(offs_t offset, uint8_t mem_mask)
{
	return m_scbrr2;
}

void sh4_base_device::scbrr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scbrr2);
	logerror("scbrr2_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint16_t sh4_base_device::scscr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_scscr2;
}

void sh4_base_device::scscr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scscr2);
	logerror("scscr2_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint8_t sh4_base_device::scftdr2_r(offs_t offset, uint8_t mem_mask)
{
	return m_scftdr2;
}

void sh4_base_device::scftdr2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	COMBINE_DATA(&m_scftdr2);
	logerror("scftdr2_w: Unmapped write %02x @ %02x\n", data, mem_mask);
}

uint16_t sh4_base_device::scfsr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_scfsr2;
}

void sh4_base_device::scfsr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_scfsr2 &= data | 0xff0c;
	logerror("scfsr2_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint8_t sh4_base_device::scfrdr2_r(offs_t offset, uint8_t mem_mask)
{
	return m_scfrdr2;
}

uint16_t sh4_base_device::scfcr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_scfcr2;
}

void sh4_base_device::scfcr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scfcr2);
	logerror("scfcr2_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint16_t sh4_base_device::scfdr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_scfdr2;
}

uint16_t sh4_base_device::scsptr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_scsptr2;
}

void sh4_base_device::scsptr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_scsptr2);
	//trips often in aristocrat mk-6
}

uint16_t sh4_base_device::sclsr2_r(offs_t offset, uint16_t mem_mask)
{
	return m_sclsr2;
}

void sh4_base_device::sclsr2_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_sclsr2 &= data | 0xfffe;
	logerror("sclsr2_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

// H-UDI
uint16_t sh4_base_device::sdir_r(offs_t offset, uint16_t mem_mask)
{
	return m_sdir;
}

void sh4_base_device::sdir_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sdir);
	logerror("sdir_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

uint32_t sh4_base_device::sddr_r(offs_t offset, uint32_t mem_mask)
{
	return m_sddr;
}

void sh4_base_device::sddr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_sddr);
	logerror("sddr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

// H-UDI 7750R
uint16_t sh4_base_device::sdint_r(offs_t offset, uint16_t mem_mask)
{
	return m_sdint;
}

void sh4_base_device::sdint_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_sdint);
	logerror("sdint_w: Unmapped write %04x @ %04x\n", data, mem_mask);
}

// PCI 7751
uint32_t sh4_base_device::pciconf0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf0;
}

uint32_t sh4_base_device::pciconf1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf1;
}

void sh4_base_device::pciconf1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf1);
	logerror("pciconf1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf2;
}

void sh4_base_device::pciconf2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf2);
	logerror("pciconf2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf3;
}

void sh4_base_device::pciconf3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf3);
	logerror("pciconf3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf4_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf4;
}

void sh4_base_device::pciconf4_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf4);
	logerror("pciconf4_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf5_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf5;
}

void sh4_base_device::pciconf5_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf5);
	logerror("pciconf5_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf6_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf6;
}

void sh4_base_device::pciconf6_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf6);
	logerror("pciconf6_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf7_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf7;
}

uint32_t sh4_base_device::pciconf8_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf8;
}

uint32_t sh4_base_device::pciconf9_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf9;
}

uint32_t sh4_base_device::pciconf10_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf10;
}

uint32_t sh4_base_device::pciconf11_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf11;
}

void sh4_base_device::pciconf11_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf11);
	logerror("pciconf11_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf12_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf12;
}

uint32_t sh4_base_device::pciconf13_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf13;
}

uint32_t sh4_base_device::pciconf14_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf14;
}

uint32_t sh4_base_device::pciconf15_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf15;
}

void sh4_base_device::pciconf15_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf15);
	logerror("pciconf15_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf16_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf16;
}

void sh4_base_device::pciconf16_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf16);
	logerror("pciconf16_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciconf17_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciconf17;
}

void sh4_base_device::pciconf17_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciconf17);
	logerror("pciconf17_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcicr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcicr;
}

void sh4_base_device::pcicr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcicr);
	logerror("pcicr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcilsr0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcilsr0;
}

void sh4_base_device::pcilsr0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcilsr0);
	logerror("pcilsr0_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcilsr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcilsr1;
}

void sh4_base_device::pcilsr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcilsr1);
	logerror("pcilsr1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcilar0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcilar0;
}

void sh4_base_device::pcilar0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcilar0);
	logerror("pcilar0_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcilar1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcilar1;
}

void sh4_base_device::pcilar1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcilar1);
	logerror("pcilar1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciint_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciint;
}

void sh4_base_device::pciint_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciint);
	logerror("pciint_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciintm_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciintm;
}

void sh4_base_device::pciintm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciintm);
	logerror("pciintm_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcialr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcialr;
}

void sh4_base_device::pcialr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcialr);
	logerror("pcialr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciclr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciclr;
}

void sh4_base_device::pciclr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciclr);
	logerror("pciclr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciaint_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciaint;
}

void sh4_base_device::pciaint_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciaint);
	logerror("pciaint_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciaintm_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciaintm;
}

void sh4_base_device::pciaintm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciaintm);
	logerror("pciaintm_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcibllr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcibllr;
}

void sh4_base_device::pcibllr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcibllr);
	logerror("pcibllr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidmabt_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidmabt;
}

void sh4_base_device::pcidmabt_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidmabt);
	logerror("pcidmabt_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidpa0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidpa0;
}

void sh4_base_device::pcidpa0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidpa0);
	logerror("pcidpa0_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidla0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidla0;
}

void sh4_base_device::pcidla0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidla0);
	logerror("pcidla0_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidtc0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidtc0;
}

void sh4_base_device::pcidtc0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidtc0);
	logerror("pcidtc0_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidcr0_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidcr0;
}

void sh4_base_device::pcidcr0_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidcr0);
	logerror("pcidcr0_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidpa1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidpa1;
}

void sh4_base_device::pcidpa1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidpa1);
	logerror("pcidpa1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidla1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidla1;
}

void sh4_base_device::pcidla1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidla1);
	logerror("pcidla1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidtc1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidtc1;
}

void sh4_base_device::pcidtc1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidtc1);
	logerror("pcidtc1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidcr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidcr1;
}

void sh4_base_device::pcidcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidcr1);
	logerror("pcidcr1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidpa2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidpa2;
}

void sh4_base_device::pcidpa2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidpa2);
	logerror("pcidpa2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidla2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidla2;
}

void sh4_base_device::pcidla2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidla2);
	logerror("pcidla2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidtc2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidtc2;
}

void sh4_base_device::pcidtc2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidtc2);
	logerror("pcidtc2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidcr2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidcr2;
}

void sh4_base_device::pcidcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidcr2);
	logerror("pcidcr2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidpa3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidpa3;
}

void sh4_base_device::pcidpa3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidpa3);
	logerror("pcidpa3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidla3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidla3;
}

void sh4_base_device::pcidla3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidla3);
	logerror("pcidla3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidtc3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidtc3;
}

void sh4_base_device::pcidtc3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidtc3);
	logerror("pcidtc3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcidcr3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcidcr3;
}

void sh4_base_device::pcidcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcidcr3);
	logerror("pcidcr3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcipar_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcipar;
}

void sh4_base_device::pcipar_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcipar);
	logerror("pcipar_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcimbr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcimbr;
}

void sh4_base_device::pcimbr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcimbr);
	logerror("pcimbr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciiobr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciiobr;
}

void sh4_base_device::pciiobr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciiobr);
	logerror("pciiobr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcipint_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcipint;
}

void sh4_base_device::pcipint_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcipint);
	logerror("pcipint_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcipintm_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcipintm;
}

void sh4_base_device::pcipintm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcipintm);
	logerror("pcipintm_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciclkr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciclkr;
}

void sh4_base_device::pciclkr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciclkr);
	logerror("pciclkr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcibcr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcibcr1;
}

void sh4_base_device::pcibcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcibcr1);
	logerror("pcibcr1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcibcr2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcibcr2;
}

void sh4_base_device::pcibcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcibcr2);
	logerror("pcibcr2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcibcr3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcibcr3;
}

void sh4_base_device::pcibcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcibcr3);
	logerror("pcibcr3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciwcr1_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciwcr1;
}

void sh4_base_device::pciwcr1_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciwcr1);
	logerror("pciwcr1_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciwcr2_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciwcr2;
}

void sh4_base_device::pciwcr2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciwcr2);
	logerror("pciwcr2_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pciwcr3_r(offs_t offset, uint32_t mem_mask)
{
	return m_pciwcr3;
}

void sh4_base_device::pciwcr3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pciwcr3);
	logerror("pciwcr3_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcimcr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcimcr;
}

void sh4_base_device::pcimcr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcimcr);
	logerror("pcimcr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcipctr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcipctr;
}

void sh4_base_device::pcipctr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcipctr);
	logerror("pcipctr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcipdtr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcipdtr;
}

void sh4_base_device::pcipdtr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcipdtr);
	logerror("pcipdtr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

uint32_t sh4_base_device::pcipdr_r(offs_t offset, uint32_t mem_mask)
{
	return m_pcipdr;
}

void sh4_base_device::pcipdr_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_pcipdr);
	logerror("pcipdr_w: Unmapped write %08x @ %08x\n", data, mem_mask);
}

void sh34_base_device::set_frt_input(int state)
{
	if (m_sh2_state->m_frt_input == state)
	{
		return;
	}

	m_sh2_state->m_frt_input = state;

#if 0
	if (m_cpu_type == CPU_TYPE_SH4)
	{
		if (m_m[5] & 0x8000)
		{
			if (state == CLEAR_LINE)
			{
				return;
			}
		}
		else
		{
			if (state == ASSERT_LINE)
			{
				return;
			}
		}
	}
	else
	{
		fatalerror("sh4_set_frt_input uses m_m[] with SH3\n");
	}

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

			if (state == CLEAR_LINE)
			{
				LOG("SH-4 '%s' cleared external irq IRL%d\n", tag(), irqline);
				sh4_exception_unrequest(SH4_INTC_IRL0 + irqline - SH4_IRL0);
			}
			else
			{
				LOG("SH-4 '%s' assert external irq IRL%d\n", tag(), irqline);
				sh4_exception_request(SH4_INTC_IRL0 + irqline - SH4_IRL0);
			}

		}

		/***** END ASSUME THIS TO BE WRONG FOR NOW *****/
	}
	else
	{
		if (irqline == INPUT_LINE_NMI)
		{
			if (m_nmi_line_state == state)
				return;

			if (m_icr & 0x100)
			{
				if (state == CLEAR_LINE && m_nmi_line_state == ASSERT_LINE)  // rising
				{
					LOG("SH-4 '%s' assert nmi\n", tag());
					sh4_exception_request(SH4_INTC_NMI);
					sh4_dmac_nmi();
				}
			}
			else
			{
				if (state == ASSERT_LINE && m_nmi_line_state == CLEAR_LINE) // falling
				{
					LOG("SH-4 '%s' assert nmi\n", tag());
					sh4_exception_request(SH4_INTC_NMI);
					sh4_dmac_nmi();
				}
			}
			if (state == CLEAR_LINE)
				m_icr ^= 0x8000;
			else
				m_icr |= 0x8000;
			m_nmi_line_state = state;
		}
		else
		{
			if (m_icr & 0x80) // four independent external interrupt sources
			{
				if (irqline > SH4_IRL3)
					return;
				if (m_irq_line_state[irqline] == state)
					return;
				m_irq_line_state[irqline] = state;

				if (state == CLEAR_LINE)
				{
					LOG("SH-4 '%s' cleared external irq IRL%d\n", tag(), irqline);
					sh4_exception_unrequest(SH4_INTC_IRL0 + irqline - SH4_IRL0);
				}
				else
				{
					LOG("SH-4 '%s' assert external irq IRL%d\n", tag(), irqline);
					sh4_exception_request(SH4_INTC_IRL0 + irqline - SH4_IRL0);
				}
			}
			else // level-encoded interrupt
			{
				if (irqline != SH4_IRLn)
					return;
				if (m_irln > 15 || m_irln < 0)
					return;
				for (int s = 0; s < 15; s++)
					sh4_exception_unrequest(SH4_INTC_IRLn0 + s);
				if (m_irln < 15)
					sh4_exception_request(SH4_INTC_IRLn0 + m_irln);
				LOG("SH-4 '%s' IRLn0-IRLn3 level #%d\n", tag(), m_irln);
			}
		}
		if (m_sh2_state->m_test_irq && !m_sh2_state->m_delay)
			sh4_check_pending_irq("sh4_set_irq_line");
	}
}

void sh34_base_device::sh4_parse_configuration()
{
	if (m_clock > 0)
	{
		switch ((m_md[2] << 2) | (m_md[1] << 1) | (m_md[0]))
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
		m_is_slave = (~m_md[7]) & 1;
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
	uint32_t topaddr = address&0xfff00000;

	for (int i = 0; i < 64; i++)
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
	if (!m_sh4_mmu_enabled || m_mmuhack != 1)
		return address;
	else
	{
		uint32_t topaddr = address & 0xfff00000;

		for (int i = 0; i < 64; i++)
		{
			uint32_t topcmp = (m_utlb[i].VPN << 10) & 0xfff00000;
			if (topcmp == topaddr)
				return (address & 0x000fffff) | ((m_utlb[i].PPN << 10) & 0xfff00000);
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

	LOG("sh4_utlb_address_array_w %08x %08x\n", offset, data);
	const bool associative = BIT(offset, 4);

	if (!associative)
	{
		// non-associative mode
		uint8_t i = (offset >> 5) & 63;
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
	uint8_t i = (offset >> 5) & 63;
	uint32_t ret = m_utlb[i].VPN << 10;
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
	LOG("sh4_utlb_data_array1_w %08x %08x\n", offset, data);
	uint8_t i = (offset >> 5) & 63;
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
	uint8_t i = (offset >> 5) & 63;
	uint32_t ret = m_utlb[i].PPN << 10;
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
	LOG("sh4_utlb_data_array2_w %08x %08x\n", offset, data);
	uint8_t i = (offset >> 5) & 63;
	m_utlb[i].TC = (data & 0x00000008) >> 3;
	m_utlb[i].SA = (data & 0x00000007) >> 0;
}


uint64_t sh4_base_device::sh4_utlb_data_array2_r(offs_t offset)
{
	uint8_t i = (offset >> 5) & 63;
	return (m_utlb[i].TC << 3) | (m_utlb[i].SA);
}
