// license:BSD-3-Clause
// copyright-holders:David Haywood, Joakim Larsson Edstrom
/* 68340 TIMER module */

/*
 * TODO:
 * - Make the timer a device and instantiate two from 68340.cpp
 * - implement all timer modes, only "Input Capture/Output Compare" mode is verified to work
 */

#include "emu.h"
#include "68340.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//#define LOG_GENERAL (1U <<  0) // Already defined in logmacro.h
#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_TIMER   (1U <<  3)
#define LOG_INT     (1U <<  4)

//#define VERBOSE  (LOG_SETUP|LOG_INT|LOG_TIMER)
#define LOG_OUTPUT_FUNC printf // Needs always to be enabled as the default value 'logerror' is not available here

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGTIMER(...) LOGMASKED(LOG_TIMER, __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

READ16_MEMBER( m68340_cpu_device::m68340_internal_timer_r )
{
	LOGSETUP("%s\n", FUNCNAME);

	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;
	int val = 0;

	int pc = space.device().safe_pc();
	logerror("%08x m68340_internal_timer_r %08x, (%08x)\n", pc, offset * 2, mem_mask);

	int id = (offset * 2) < 0x40 ? 0 : 1; // Timer1 or Timer2

	/*Setting the STP bit stops all clocks within the timer module except for the clock
	  from the IMB. The clock from the IMB remains active to allow the CPU32 access to the MCR.
	  Accesses to timer module registers while in stop mode produce a bus error. */
	if ( (timer.m_mcr[id] & m68340_timer::REG_MCR_STP) && ((offset * 2) % 0x40) != m68340_timer::REG_MCR)
	{
		logerror("Attempt to access timer registers while timer clocks are stopped, STP bit in MCR is set!");
		return val; // TODO: Should cause BUSERROR
	}

	switch ((offset * 2) % 0x40)
	{
	case m68340_timer::REG_MCR:
			val = timer.m_mcr[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (MCR - Module Configuration Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	case m68340_timer::REG_IR:
			val = timer.m_ir[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (IR - Interrupt Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	case m68340_timer::REG_CR:
			val = timer.m_cr[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (CR - Control Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	case m68340_timer::REG_SR:
			val = timer.m_sr[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (SR - Status/Prescaler Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	case m68340_timer::REG_CNTR:
			val = timer.m_cntr_reg[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (CNTR - Counter Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	case m68340_timer::REG_PREL1:
			val = timer.m_prel1[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (PREL1 - Preload 1 Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	case m68340_timer::REG_PREL2:
			val = timer.m_prel2[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (PREL2 - Preload 2 Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	case m68340_timer::REG_COM:
			val = timer.m_com[id];
			LOGTIMER("- %08x %s %04x, %04x (%04x) (COM - Compare Register)\n", pc, FUNCNAME, offset * 2, val, mem_mask);
			break;
	default:
			LOGTIMER("- %08x FUNCNAME %08x, %08x (%08x) - not implemented\n", pc, offset * 2, val, mem_mask);
			logerror("%08x m68340_internal_timer_r %08x, %08x (%08x)\n", pc, offset * 2, val, mem_mask);
			break;
	}
	LOGR(" * Timer%d Reg %02x [%02x] -> %02x - %s\n", id + 1, offset * 2 % 0x40, offset, val, ((offset * 2) % 0x40) > 0x12 ? "reserved" : std::array<char const *, 9> {{"MCR", "reserved", "IR", "CR", "SR", "CNTR", "PREL1", "PREL2", "COM"}}[offset % 0x20]);
	return val;
}

WRITE16_MEMBER( m68340_cpu_device::m68340_internal_timer_w )
{
	int id = (offset * 2) < 0x40 ? 0 : 1; // Timer1 or Timer2

	LOGSETUP("\n%s\n", FUNCNAME);
	LOGSETUP(" * Timer%d Reg %02x [%02x] <- %02x - %s\n", id + 1, (offset * 2) % 0x40, offset, data,
			 ((offset * 2) % 0x40) > 0x12 ? "reserved" : std::array<char const *, 9>
			 {{"MCR", "reserved", "IR", "CR", "SR", "CNTR", "PREL1", "PREL2", "COM"}}[offset % 0x20]);

	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	/*Setting the STP bit stops all clocks within the timer module except for the clock
	  from the IMB. The clock from the IMB remains active to allow the CPU32 access to the MCR.
	  Accesses to timer module registers while in stop mode produce a bus error. */
	if ( (timer.m_mcr[id] & m68340_timer::REG_MCR_STP) && ((offset * 2) % 0x40) != m68340_timer::REG_MCR)
	{
		logerror("Attempt to access timer registers while timer clocks are stopped, STP bit in MCR is set!");
		return; // TODO: Should cause BUSERROR
	}

	switch ((offset * 2) % 0x40)
	{
	case m68340_timer::REG_MCR:
		COMBINE_DATA(&timer.m_mcr[id]);
		LOGTIMER("PC: %08x %s %04x, %04x (%04x) (MCR - Module Configuration Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		LOGTIMER("- Clocks are %s\n", data & m68340_timer::REG_MCR_STP ? "stopped" : "running");
		LOGTIMER("- Freeze signal %s - not implemented\n", data & m68340_timer::REG_MCR_FRZ1 ? "stops execution" : "is ignored");
		LOGTIMER("- Supervisor registers %s - not implemented\n", data & m68340_timer::REG_MCR_SUPV ? "requries supervisor privileges" : "can be accessed by user privileged software");
		LOGTIMER("- Interrupt Arbitration level: %02x - not implemented\n", data & m68340_timer::REG_MCR_ARBLV);
		break;
	case m68340_timer::REG_IR:
		COMBINE_DATA(&timer.m_ir[id]);
		LOGTIMER("PC: %08x %s %04x, %04x (%04x) (IR - Interrupt Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		LOGTIMER("- Interrupt level : %02x\n", (data & m68340_timer::REG_IR_INTLEV) >> 8);
		LOGTIMER("- Interrupt vector: %02x\n", (data & m68340_timer::REG_IR_INTVEC));
		break;
	case m68340_timer::REG_CR:
		COMBINE_DATA(&timer.m_cr[id]);
		LOGTIMER("PC %08x %s %04x, %04x (%04x) (CR - Module Control Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		LOGTIMER("- Software reset: %s\n", (data & m68340_timer::REG_CR_SWR) ? "inactive" : "active" );
		LOGTIMER("- Enabled interrupts: %02x TO:%d TG:%d TC:%d\n",
			 data & m68340_timer::REG_CR_INTMSK,
			 (data & m68340_timer::REG_CR_IE0) ? 1 : 0,
			 (data & m68340_timer::REG_CR_IE1) ? 1 : 0,
			 (data & m68340_timer::REG_CR_IE2) ? 1 : 0);
		LOGTIMER("- TGE signal, TGATE%d is %s\n", id + 1, (data & m68340_timer::REG_CR_TGE) ? "enabled" : "disabled");
		LOGTIMER("- PCLK: Counter uses %s\n", (data & m68340_timer::REG_CR_PCLK) ? "prescaler" : "clock");
		LOGTIMER("- CPE: Counter is %s\n", (data & m68340_timer::REG_CR_CPE) ? "enabled" : "disabled");
		LOGTIMER("- CLK: Clock is %s\n", (data & m68340_timer::REG_CR_CLK) ? "TIN (external)" : "system clock / 2");
		LOGTIMER("- Prescaler: Divide by %d\n", (data & m68340_timer::REG_CR_POT_MASK) ? ( 1 << ((data & m68340_timer::REG_CR_POT_MASK) >> 5)) : 256);
		LOGTIMER("- Prescaler: Divide by %d\n", (0x101 << ((data & m68340_timer::REG_CR_POT_MASK) >> 5) & 0x1fe));
		LOGTIMER("- MODE: %s\n", std::array<char const *, 8>
			 {{  "Input Capture/Output Compare",
				   "Square-Wave Generator - not implemented",
				   "Variable Duty-Cycle Square-Wave Generator - not implemented",
				   "Variable-Width Single-Shot Pulse Generator - not implemented",
				   "Pulse Width Measurement - not implemented",
				   "Period Measurement - not implemented",
				   "Event Count - not implemented",
				   "Timer Bypass (Simple Test Method) - not implemented"
			 }}[data & m68340_timer::REG_CR_MODE_MASK]);

		LOGTIMER("- OC: %s mode\n", std::array<char const *, 4>{{"Disabled", "Toggle", "Zero", "One"}}[data & m68340_timer::REG_CR_OC_MASK]);

		/* The timer is enabled when the counter prescaler enable (CPE) and SWRx bits in the CR
		   are set. Once enabled, the counter enable (ON) bit in the SR is set, and the next falling
		   edge of the counter clock causes the counter to be loaded with the value in the preload 1
		   register (PREL1). TODO: make sure of the intial load of PREL1 on first falling flank */
		if (timer.m_cr[id] & m68340_timer::REG_CR_SWR)
		{
			if (timer.m_cr[id] & m68340_timer::REG_CR_CPE)
			{
				timer.m_sr[id] |= m68340_timer::REG_SR_ON; // Starts the counter
				LOGTIMER("Starts counter %d\n", id);
				if ((timer.m_cr[id] & m68340_timer::REG_CR_CLK) == 0)
				{
					LOGTIMER("- Using system clock/2\n");
					timer.m_timer[id]->adjust(cycles_to_attotime( (clock() / 2) / (0x101 << ((timer.m_cr[id] & m68340_timer::REG_CR_POT_MASK) >> 5) & 0x1fe) * 2));
				}
				else
				{
					LOGTIMER("- Using TIN%d\n", id);
				}
			}
			else
			{
				timer.m_sr[id] &= ~m68340_timer::REG_SR_ON; // Stops the counter
				LOGTIMER("Stops counter %d\n", id);
				timer.m_timer[id]->adjust(attotime::never);
			}
		}
		else
		{ // TODO: Detect Disable mode setting line to three state
			if ((timer.m_cr[id] & m68340_timer::REG_CR_OC_MASK) == m68340_timer::REG_CR_OC_ONE)
			{
				id == 0 ? m_tout1_out_cb(ASSERT_LINE) : m_tout2_out_cb(ASSERT_LINE);
			}
			else
			{
				id == 0 ? m_tout1_out_cb(CLEAR_LINE) : m_tout2_out_cb(CLEAR_LINE);
			}
		}
		break;
	case m68340_timer::REG_SR:
		LOGTIMER("PC %08x %s %04x, %04x (%04x) (SR - Status/Prescaler Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);

		/* IRQ - Interrupt Request bit
		   1 = An interrupt condition has occurred. This bit is the logical OR of the enabled TO, TG, and TC interrupt bits.
		   0 = The bit(s) that caused the interrupt condition has been cleared. If an IRQ signal has been asserted, it is negated when this bit is cleared. */
		data = (timer.m_sr[id] & ~(data & (m68340_timer::REG_SR_TO | m68340_timer::REG_SR_TG | m68340_timer::REG_SR_TC))); // Clear only the set interrupt bits
		if ((data & (m68340_timer::REG_SR_TO | m68340_timer::REG_SR_TG | m68340_timer::REG_SR_TC)) == 0)
		{
			data &= ~m68340_timer::REG_SR_IRQ;
			// TODO: clear IRQ line
		}
		COMBINE_DATA(&timer.m_sr[id]);
		LOGTIMER("PC %08x %s %04x, %04x (%04x) (SR - Status/Prescaler Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		LOGTIMER("- IRQ: %s\n", (data & m68340_timer::REG_SR_IRQ) ? "Yes" : "None");
		LOGTIMER("- TO TimeOut int      : %s\n", (data & m68340_timer::REG_SR_TO) ? "Asserted" : "Cleared");
		LOGTIMER("- TG Timer Gate int   : %s\n", (data & m68340_timer::REG_SR_TG) ? "Asserted" : "Cleared");
		LOGTIMER("- TC Timer Counter int: %s\n", (data & m68340_timer::REG_SR_TC) ? "Asserted" : "Cleared");
		LOGTIMER("- TGL: %s\n", (data & m68340_timer::REG_SR_TGL) ? "Negated" : "Asserted");
		LOGTIMER("- ON Counter is: %s\n", (data & m68340_timer::REG_SR_ON) ? "Enabled" : "Disabled");
		LOGTIMER("- OUT: Tout is %s\n", (data & m68340_timer::REG_SR_OUT) ? "1" : "0 or three-stated");
		LOGTIMER("- COM: Compare is %s\n", (data & m68340_timer::REG_SR_COM) ? "Match" : "Cleared");
		LOGTIMER("- PO7-PO0: %02x\n", (data & m68340_timer::REG_SR_PSC_OUT));
		break;
	case m68340_timer::REG_CNTR:
		COMBINE_DATA(&timer.m_cntr_reg[id]);
		LOGTIMER("- %08x %s %04x, %04x (%04x) (CNTR - Counter Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		break;
	case m68340_timer::REG_PREL1:
		COMBINE_DATA(&timer.m_prel1[id]);
		LOGTIMER("PC %08x %s %04x, %04x (%04x) (PREL1 - Preload 1 Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		LOGTIMER("- PR1-15 to PR1-0: %04x\n", (data & 0xffff));
		break;
	case m68340_timer::REG_PREL2:
		COMBINE_DATA(&timer.m_prel2[id]);
		LOGTIMER("PC %08x %s %04x, %04x (%04x) (PREL2 - Preload 2 Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		LOGTIMER("- PR2-15 to PR2-0: %04x\n", (data & 0xffff));
		break;
	case m68340_timer::REG_COM:
		COMBINE_DATA(&timer.m_com[id]);
		LOGTIMER("PC %08x %s %04x, %04x (%04x) (COM - Compare Register)\n", pc, FUNCNAME, offset * 2, data, mem_mask);
		LOGTIMER("- COM15-COM0: %04x\n", (data & 0xfff));
		break;
	default:
		LOGTIMER("- %08x FUNCNAME %08x, %08x (%08x) - not implemented\n", pc, offset * 2, data, mem_mask);
		logerror("%08x m68340_internal_sim_w %08x, %08x (%08x)\n", pc, offset * 2, data, mem_mask);
		break;
	}

	int pc = space.device().safe_pc();
	logerror("%08x m68340_internal_timer_w %08x, %08x (%08x)\n", pc, offset * 2, data, mem_mask);
}

void m68340_timer::reset()
{
	LOGSETUP("%s\n", FUNCNAME);
}

WRITE_LINE_MEMBER( m68340_cpu_device::tin1_w)
{
	LOGTIMER("%s\n", FUNCNAME);

	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	timer.m_tin[0] = state;
}

WRITE_LINE_MEMBER( m68340_cpu_device::tgate1_w)
{
	LOGTIMER("%s\n", FUNCNAME);

	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	timer.m_tgate[0] = state;
	if (state == ASSERT_LINE)
	{
		if (timer.m_cr[0] & m68340_timer::REG_CR_TGE)
		{
			timer.m_sr[0] |= m68340_timer::REG_SR_TG;
			if (timer.m_cr[0] & m68340_timer::REG_CR_IE1)
			{
				LOGTIMER(" - TG interrupt");
				do_timer_irq(0);
				timer.m_sr[0] |= m68340_timer::REG_SR_IRQ;

			}
		}
		timer.m_sr[0] |= m68340_timer::REG_SR_TGL;
	}
	else
	{
		timer.m_sr[0] &= ~m68340_timer::REG_SR_TGL;
	}
}

WRITE_LINE_MEMBER( m68340_cpu_device::tin2_w)
{
	LOGTIMER("%s\n", FUNCNAME);

	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	timer.m_tin[1] = state;
}

WRITE_LINE_MEMBER( m68340_cpu_device::tgate2_w)
{
	LOGTIMER("%s\n", FUNCNAME);

	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	timer.m_tgate[1] = state;
	if (state == ASSERT_LINE)
	{
		if (timer.m_cr[1] & m68340_timer::REG_CR_TGE)
		{
			timer.m_sr[1] |= m68340_timer::REG_SR_TG;
			if (timer.m_cr[1] & m68340_timer::REG_CR_IE1)
			{
				LOGTIMER(" - TG interrupt");
				do_timer_irq(1);
				timer.m_sr[1] |= m68340_timer::REG_SR_IRQ;

			}
		}
		timer.m_sr[1] |= m68340_timer::REG_SR_TGL;
	}
	else
	{
		timer.m_sr[1] &= ~m68340_timer::REG_SR_TGL;
	}
}

TIMER_CALLBACK_MEMBER(m68340_cpu_device::timer1_callback)
{
	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;
	do_timer_tick(0);
	if ((timer.m_sr[0] & m68340_timer::REG_SR_ON) != 0)
	{
		LOGTIMER("Re-arming timer 1 using system clock/2 as base %d Hz\n", (clock() / 2) / (0x101 << ((timer.m_cr[0] & m68340_timer::REG_CR_POT_MASK) >> 5) & 0x1fe));
		timer.m_timer[0]->adjust(cycles_to_attotime( (clock() / 2) / (0x101 << ((timer.m_cr[0] & m68340_timer::REG_CR_POT_MASK) >> 5) & 0x1fe) * 2));
	}
}

TIMER_CALLBACK_MEMBER(m68340_cpu_device::timer2_callback)
{
	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;
	do_timer_tick(1);
	if ((timer.m_sr[1] & m68340_timer::REG_SR_ON) != 0)
	{
	  LOGTIMER("Re-arming timer 2 using system clock/2 as base: %d Hz\n", (clock() / 2) / (0x101 << ((timer.m_cr[1] & m68340_timer::REG_CR_POT_MASK) >> 5) & 0x1fe));
	  timer.m_timer[1]->adjust(cycles_to_attotime( (clock() / 2) / (0x101 << ((timer.m_cr[1] & m68340_timer::REG_CR_POT_MASK) >> 5) & 0x1fe) * 2));
	}
}

void m68340_cpu_device::start_68340_timer()
{
	LOGSETUP("%s\n", FUNCNAME);

	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	timer.m_timer[0] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m68340_cpu_device::timer1_callback),this));
	timer.m_timer[1] = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m68340_cpu_device::timer2_callback),this));

	// resolve callbacks Port A
	m_pa_out_cb.resolve_safe();
	m_pa_in_cb.resolve();

	// Resolve Timer callbacks
	m_tout1_out_cb.resolve_safe();
	m_tgate1_in_cb.resolve_safe();
	m_tin1_in_cb.resolve_safe();
	m_tout2_out_cb.resolve_safe();
	m_tgate2_in_cb.resolve_safe();
	m_tin2_in_cb.resolve_safe();
}

void m68340_cpu_device::do_timer_irq(int id)
{
	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	assert(m68340SIM);
	m68340_sim &sim = *m68340SIM;

	int timer_irq_level  = (timer.m_ir[id] & 0x0700) >> 8;
	int timer_irq_vector = (timer.m_ir[id] & 0x00ff) >> 0;

	if (timer_irq_level) // 0 is irq disabled
	{
		int use_autovector = (sim.m_avr_rsr >> (8 + timer_irq_level)) & 1;

		LOGINT("TIMER IRQ triggered, Lvl: %d using Vector: %d (0 = auto vector)\n", timer_irq_level, use_autovector ? 0 : timer_irq_vector);

		if (use_autovector)
		{
			//logerror("irq with autovector\n");
			set_input_line(timer_irq_level, HOLD_LINE);
		}
		else
		{
			//logerror("irq without autovector\n");
			set_input_line_and_vector(timer_irq_level, HOLD_LINE, timer_irq_vector);
		}
	}
}

/* do_timer_tick works on flanks, thus half clock cycles, to capture both rising and falling clock flanks */
void m68340_cpu_device::do_timer_tick( int id )
{
	assert(m68340TIMER);
	m68340_timer &timer = *m68340TIMER;

	timer.m_timer_counter[id]--; // Count flanks, least significant bit is state of the clock line
	if ( (timer.m_mcr[id] & m68340_timer::REG_MCR_STP) == 0)
	{
		if (timer.m_timer_counter[id] & 1) // Raising flank, copy shadow to register
		{
		  // Shadow the counter only if we are NOT in the ICOC mode WHILE the TG bit is set
		  if (!((timer.m_cr[id] & m68340_timer::REG_CR_MODE_MASK) == m68340_timer::REG_CR_MODE_ICOC &&
			(timer.m_sr[id] & m68340_timer::REG_SR_TG) != 0))
			timer.m_cntr_reg[id] = timer.m_cntr[id];
		}
		else // Falling flank
		{
			timer.m_cntr[id]--;

			/* TC - Timer Compare Interrupt
			   1 = This bit is set when the counter transitions (off a clock/event falling edge) to the
			       value in the COM. This bit does not affect the programmed IRQ signal if the IE0
			       bit in the CR is cleared.
			   0 = This bit is cleared by the timer whenever the RESET signal is asserted on the
			       IMB, regardless of the mode of operation. This bit may also be cleared by writing
			       a one to it. Writing a zero to this bit does not alter its contents. This bit is not
			       affected by disabling the timer (SWR = 0).*/
			if (timer.m_cntr[id] == timer.m_com[id]) // Check COM register
			{
				timer.m_sr[id] |= (m68340_timer::REG_SR_TC | m68340_timer::REG_SR_COM);
			}
		}

		LOGINT("%s reached\n", timer.m_cntr_reg[id] == 0 ? "Timeout" : "COM value");
		/* OC1/OC0 - Output Control
		   These bits select the conditions under which TOUTx changes These
		   bits may have a different effect when in the input capture/output compare mode.*/
		switch (timer.m_cr[id] & m68340_timer::REG_CR_OC_MASK)
		{
		case m68340_timer::REG_CR_OC_DISABLED:
			/* Disabled - TOUTx is disabled and three-stated TODO: support three-stated level */
			break;
		case m68340_timer::REG_CR_OC_TOGGLE:
			/* Toggle Mode - If the timer is disabled (SWR = 0) when this encoding is programmed,
			   TOUTx is immediately set to zero. If the timer is enabled (SWR = 1), timeout events
			   (counter reaches $0000) toggle TOUTx.

			   In the input capture/output compare mode,
			   TOUTx is immediately set to zero if the timer is disabled (SWR = 0). If the timer is
			   enabled (SWR = 1), timer compare events toggle TOUTx. (Timer compare events occur
			   when the counter reaches the value stored in the COM.)*/
		  if ((timer.m_cr[id] & m68340_timer::REG_CR_MODE_MASK) == m68340_timer::REG_CR_MODE_ICOC) // Detect Input Capture/Output Compare mode
			{
				if ((timer.m_sr[id] & m68340_timer::REG_SR_COM) != 0) // timer reached compare value?
				{
					if (id == 0)
					{
						m_tout1_out_cb((timer.m_tout[0]++ & 1) != 0 ? ASSERT_LINE : CLEAR_LINE);
					}
					else
					{
						m_tout2_out_cb((timer.m_tout[1]++ & 1) != 0 ? ASSERT_LINE : CLEAR_LINE);
					}
				}
			}
			else // Any oher mode
			{
				if (timer.m_cntr_reg[id] == 0) // Counter reached timeout?
				{
					if (id == 0)
					{
						m_tout1_out_cb((timer.m_tout[0]++ & 1) != 0 ? ASSERT_LINE : CLEAR_LINE);
					}
					else
					{
						m_tout2_out_cb((timer.m_tout[1]++ & 1) != 0 ? ASSERT_LINE : CLEAR_LINE);
					}
				}
			}
			break;
		case m68340_timer::REG_CR_OC_ZERO:
			/* Zero Mode - If the timer is disabled (SWR = 0) when this encoding is programmed,
			   TOUTx is immediately set to zero. If the timer is enabled (SWR = 1), TOUTx will be set
			   to zero at the next timeout.

			   In the input capture/output compare mode, TOUTx is
			   immediately set to zero if the timer is disabled (SWR = 0). If the timer is enabled (SWR
			   = 1), TOUTx will be set to zero at timeouts and set to one at timer compare events. If
			   the COM is $0000, TOUTx will be set to zero at the timeout/timer compare event.*/
			if ((timer.m_cr[id] & m68340_timer::REG_CR_MODE_MASK) == m68340_timer::REG_CR_MODE_ICOC) // Detect Input Capture/Output Compare mode
			{
				if ((timer.m_sr[id] & m68340_timer::REG_SR_COM) != 0) // timer reached compare value?
				{
					if (id == 0)
					{
						m_tout1_out_cb(ASSERT_LINE);
					}
					else
					{
						m_tout2_out_cb(ASSERT_LINE);
					}
				}
				if (timer.m_cntr_reg[id] == 0) // timer reached timeout value?
				{
					if (id == 0)
					{
						m_tout1_out_cb(CLEAR_LINE);
					}
					else
					{
						m_tout2_out_cb(CLEAR_LINE);
					}
				}
			}
			else
			{
				if (timer.m_cntr_reg[id] == 0) // timer reached timeout value?
				{
					if (id == 0)
					{
						m_tout1_out_cb(CLEAR_LINE);
					}
					else
					{
						m_tout2_out_cb(CLEAR_LINE);
					}
				}
			}
			break;
				case m68340_timer::REG_CR_OC_ONE:
					/* One Mode - If the timer is disabled (SWR = 0) when this encoding is programmed,
					   TOUTx is immediately set to one. If the timer is enabled (SWR = 1), TOUTx will be set
					   to one at the next timeout.

					   In the input capture/output compare mode, TOUTx is
					   immediately set to one if the timer is disabled (SWR = 0). If the timer is enabled (SWR =
					   1), TOUTx will be set to one at timeouts and set to zero at timer compare events. If the
					   COM is $0000, TOUTx will be set to one at the timeout/timer compare event.*/
			if ((timer.m_cr[id] & m68340_timer::REG_CR_MODE_MASK) == m68340_timer::REG_CR_MODE_ICOC) // Detect Input Capture/Output Compare mode
			{
				if ((timer.m_sr[id] & m68340_timer::REG_SR_COM) != 0) // timer reached compare value?
				{
					if (id == 0)
					{
						m_tout1_out_cb(CLEAR_LINE);
					}
					else
					{
						m_tout2_out_cb(CLEAR_LINE);
					}
				}
				if (timer.m_cntr_reg[id] == 0) // timer reached timeout value?
				{
					if (id == 0)
					{
						m_tout1_out_cb(ASSERT_LINE);
					}
					else
					{
						m_tout2_out_cb(ASSERT_LINE);
					}
				}
			}
			else
			{
				if (timer.m_cntr_reg[id] == 0) // timer reached timeout value?
				{
					if (id == 0)
					{
						m_tout1_out_cb(ASSERT_LINE);
					}
					else
					{
						m_tout2_out_cb(ASSERT_LINE);
					}
				}
			}
			break;
		default:
			LOGTIMER("Wrong TOUT mode, fix the code!\n");
		}

		if (timer.m_cntr_reg[id] == 0) // timer reached timeout value?
		{
			timer.m_cntr[id] = timer.m_prel1[id]; // TODO: Support prel2 for certain modes
			if (timer.m_cr[id] & m68340_timer::REG_CR_IE2)
			{
				LOGTIMER(" - TO interrupt");
				do_timer_irq(id);
				timer.m_sr[id] |= m68340_timer::REG_SR_IRQ;
			}
		}
		if ((timer.m_sr[id] & m68340_timer::REG_SR_COM) != 0) // timer reached compare value? )
		{
			if (timer.m_cr[id] & m68340_timer::REG_CR_IE0)
			{
				LOGTIMER(" - TC interrupt");
				do_timer_irq(id);
				timer.m_sr[id] |= m68340_timer::REG_SR_IRQ;

			}
		}
	}
}
