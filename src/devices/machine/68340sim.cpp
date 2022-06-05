// license:BSD-3-Clause
// copyright-holders:David Haywood, Joakim Larsson Edstrom
/* 68340 SIM module */

#include "emu.h"
#include "68340.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//#define LOG_GENERAL (1U <<  0) // Already defined in logmacro.h
#define LOG_SETUP   (1U <<  1)
#define LOG_READ    (1U <<  2)
#define LOG_PORTS   (1U <<  3)
#define LOG_SIM     (1U <<  4)
#define LOG_CLOCK   (1U <<  5)
#define LOG_DATA    (1U <<  6)
#define LOG_INT     (1U <<  7)
#define LOG_PIT     (1U <<  8)
#define LOG_CS      (1U <<  9)

#define VERBOSE  (LOG_PIT)
#define LOG_OUTPUT_FUNC printf // Needs always to be enabled as the default value 'logerror' is not available here

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGPORTS(...) LOGMASKED(LOG_PORTS, __VA_ARGS__)
#define LOGSIM(...)   LOGMASKED(LOG_SIM,   __VA_ARGS__)
#define LOGCLOCK(...) LOGMASKED(LOG_CLOCK, __VA_ARGS__)
#define LOGDATA(...)  LOGMASKED(LOG_DATA,  __VA_ARGS__)
#define LOGINT(...)   LOGMASKED(LOG_INT,   __VA_ARGS__)
#define LOGPIT(...)   LOGMASKED(LOG_PIT,   __VA_ARGS__)
#define LOGCS(...)    LOGMASKED(LOG_CS,    __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

uint16_t m68340_cpu_device::m68340_internal_sim_r(offs_t offset, uint16_t mem_mask)
{
	LOGR("%s\n", FUNCNAME);
	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;
	int val = 0;

	switch (offset * 2)
	{
		case m68340_sim::REG_MCR:
			LOGSIM("- %08x %s %04x, (%04x) (MCR - Module Configuration Register) - not implemented\n", m_ppc, FUNCNAME, offset * 2, mem_mask);
			val = sim.m_mcr;
			break;

		case m68340_sim::REG_SYNCR:
			LOGSIM("- %08x %s %04x, (%04x) (SYNCR - Clock Synthesizer Register) - not implemented\n", m_ppc, FUNCNAME, offset*2,mem_mask);
			val = sim.m_syncr;
			break;

		case m68340_sim::REG_AVR_RSR: // Manual seems to say that AVR only autovectors externally triggered interrupts (INT1-INT7)
			LOGSIM("- %08x %s %04x, (%04x) (AVR, RSR - Auto Vector Register, Reset Status Register) - not implemented\n", m_ppc, FUNCNAME, offset*2,mem_mask);
			val = sim.m_avr_rsr;
			break;

		case m68340_sim::REG_SWIV_SYPCR:
			LOGSIM("- %08x %s %04x, (%04x) (SWIV_SYPCR - Software Interrupt Vector, System Protection Control Register) - not implemented\n", m_ppc, FUNCNAME, offset*2,mem_mask);
			val = sim.m_swiv_sypcr;
			break;

		case m68340_sim::REG_PICR:
			LOGPIT("- %08x %s %04x, (%04x) (PICR - Periodic Interrupt Control Register) - not implemented\n", m_ppc, FUNCNAME, offset*2,mem_mask);
			val = sim.m_picr;
			break;

		case m68340_sim::REG_PITR:
			LOGPIT("- %08x %s %04x, (%04x) (PITR - Periodic Interrupt Timer Register) - not implemented\n", m_ppc, FUNCNAME, offset*2,mem_mask);
			val = sim.m_pitr;
			break;

		case m68340_sim::REG_SWSR:
			LOGSIM("- %08x %s %04x, (%04x) (SWSR - Software Service) - not implemented\n", m_ppc, FUNCNAME, offset*2,mem_mask);
			val = sim.m_swsr;
			break;

		default:
			logerror("- %08x %s %04x, (%04x) (unsupported register)\n", m_ppc, FUNCNAME, offset * 2, mem_mask);
			LOGSIM("- %08x %s %04x, (%04x) (unsupported register)\n", m_ppc, FUNCNAME, offset * 2, mem_mask);
	}

	LOGR(" * Reg %02x -> %02x - %s\n", offset * 2, val,
		 ((offset * 2) >= 0x10 && (offset * 2) < 0x20) || (offset * 2) >= 0x60 ? "Error - should not happen" :
		 std::array<char const *, 8> {{"MCR", "reserved", "SYNCR", "AVR/RSR", "SWIV/SYPCR", "PICR", "PITR", "SWSR"}}[(offset * 2) <= m68340_sim::REG_AVR_RSR ? offset : offset - 0x10 + 0x04]);

	return val;
}

void m68340_cpu_device::m68340_internal_sim_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOG("\n%s\n", FUNCNAME);
	LOGSETUP(" * Reg %02x <- %02x - %s\n", offset * 2, data,
		 ((offset * 2) >= 0x10 && (offset * 2) < 0x20) || (offset * 2) >= 0x60 ? "Error - should not happen" :
		 std::array<char const *, 8> {{"MCR", "reserved", "SYNCR", "AVR/RSR", "SWIV/SYPCR", "PICR", "PITR", "SWSR"}}[(offset * 2) <= m68340_sim::REG_AVR_RSR ? offset : offset - 0x10 + 0x04]);

	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;

	switch (offset<<1)
	{
		case m68340_sim::REG_MCR:
			COMBINE_DATA(&sim.m_mcr);
			LOGSIM("PC: %08x %s %04x, %04x (%04x) (MCR - Module Configuration Register)\n", m_ppc, FUNCNAME, offset * 2, data, mem_mask);
			LOGPIT("- FRZ1: Watchdog and PIT timer are %s\n", (data & m68340_sim::REG_MCR_FRZ1) == 0 ? "enabled" : "disabled");
			LOGSIM("- FRZ0: The BUS monitor is %s\n", (data & m68340_sim::REG_MCR_FRZ0) == 0 ? "enabled" : "disabled");
			LOGSIM("- FIRQ: Full Interrupt Request Mode %s\n", data & m68340_sim::REG_MCR_FIRQ ? "used on port B" : "supressed, adding 4 chip select lines on Port B");
			LOGSIM("- SHEN0-SHEN1: Show Cycle Enable %02x - not implemented\n", ((data & m68340_sim::REG_MCR_SHEN) >> 8));
			LOGSIM("- Supervisor registers %s - not implemented\n", data & m68340_sim::REG_MCR_SVREG ? "requries supervisor privileges" : "can be accessed by user privileged software");
			LOGSIM("- Interrupt Arbitration level: %02x\n", data & m68340_sim::REG_MCR_ARBLV);
			break;

		case m68340_sim::REG_SYNCR:
			LOGSIM("PC: %08x %s %04x, %04x (%04x) (SYNCR - Clock Synthesizer Register) - not implemented\n", m_ppc, FUNCNAME, offset * 2, data, mem_mask);
			COMBINE_DATA(&sim.m_syncr);
			LOGSIM("- W    : VCO x %d\n", data & m68340_sim::REG_SYNCR_W ? 4 : 1);
			LOGSIM("- X    : System clock / %d\n", data & m68340_sim::REG_SYNCR_X ? 1 : 2);
			LOGSIM("- Y5-Y0: Divider: %d\n", (data & m68340_sim::REG_SYNCR_Y_MSK) >> 8);
			LOGSIM("- SLIMP: Loss of VCO input reference: %s\n", data & m68340_sim::REG_SYNCR_SLIMP ? "yes" : "no");
			LOGSIM("- SLOCK: Synthesizer lock: %s\n", data & m68340_sim::REG_SYNCR_SLOCK ? "yes" : "no");
			LOGSIM("- RSTEN: Loss of signal causes %s\n", data & m68340_sim::REG_SYNCR_RSTEN ? "system reset" : "limp mode (no external reference)");
			LOGSIM("- STSIM: LPSTOP makes SIM40 to use %s\n", data & m68340_sim::REG_SYNCR_STSIM ? "VCO" : "external clock/oscillator");
			LOGSIM("- STEXT: LPSTOP makes CLKOUT %s\n", data & m68340_sim::REG_SYNCR_STEXT ? "driven by SIM40 clock" : "low");

			// Adjust VCO
			if (m_clock_mode == m68340_sim::CLOCK_MODE_CRYSTAL)
			{
				set_unscaled_clock(m_crystal *
								   (4 << (((sim.m_syncr & m68340_sim::REG_SYNCR_W) != 0 ? 2 : 0) + ((sim.m_syncr & m68340_sim::REG_SYNCR_X) != 0 ? 1 : 0))) *
								   (((sim.m_syncr & m68340_sim::REG_SYNCR_Y_MSK) >> 8) & 0x3f));
				LOGCLOCK( " - Clock: %d [0x%08x]\n", clock(), clock());
			}
			break;

		case m68340_sim::REG_AVR_RSR:
			LOGSIM("PC: %08x %s %04x, %04x (%04x) (AVR, RSR - Auto Vector Register, Reset Status Register)\n", m_ppc, FUNCNAME, offset * 2, data, mem_mask);
			COMBINE_DATA(&sim.m_avr_rsr);
			LOGSIM("- AVR: AV7-AV1 autovector bits:  %02x\n", ((data & m68340_sim::REG_AVR_VEC) >> 8) & 0xff);
			LOGSIM("- RSR: Last reset type:  %02x - not implemented\n", (data & m68340_sim::REG_RSR_RESBITS) & 0xff);
			break;

		case m68340_sim::REG_SWIV_SYPCR:
			LOGSIM("PC: %08x %s %04x, %04x (%04x) (SWIV_SYPCR - Software Interrupt Vector, System Protection Control Register) - not implemented\n", m_ppc, FUNCNAME, offset * 2, data, mem_mask);
			COMBINE_DATA(&sim.m_swiv_sypcr);
			LOGSIM("- SWIV: Software watchdog Interrupt Vector: %02x\n", ((data & m68340_sim::REG_SWIV_VEC) >> 8) & 0xff);
			LOGSIM("- SWE : Software watchdog %s\n", (data & m68340_sim::REG_SYPCR_SWE) ? "enabled" : "disabled");
			LOGSIM("- SWRI: Software watchdog causes %s\n", (data & m68340_sim::REG_SYPCR_SWRI) ? "system reset" : "level 7 IRQ (NMI)");
			LOGSIM("- SWT : Software watchdog timing 2^%s/EXTAL Input Frequency\n",
				   std::array<char const *, 8> {{"9", "11", "13", "15", "18", "20", "22", "24"}}[((data & m68340_sim::REG_SYPCR_SWT) >> 4) | ((sim.m_pitr & m68340_sim::REG_PITR_SWP) >> 7)]);
			break;

		case m68340_sim::REG_PICR:
			LOGPIT("PC: %08x %s %04x, %04x (%04x) (PICR - Periodic Interrupt Control Register)\n", m_ppc, FUNCNAME, offset*2,data,mem_mask);
			COMBINE_DATA(&sim.m_picr);
			LOGPIT("- PIRQL: Periodic Interrupt Level  %d%s\n", (data & m68340_sim::REG_PICR_PIRQL) >> 8, (data & m68340_sim::REG_PICR_PIRQL) == 0 ? " (disabled)" : "");
			LOGPIT("- PIV  : Periodic Interrupt Vector %02x\n", (data & m68340_sim::REG_PICR_PIVEC));
			update_ipl();
			break;

		case m68340_sim::REG_PITR:
			LOGPIT("PC: %08x %s %04x, %04x (%04x) (PITR - Periodic Interrupt Timer Register)\n", m_ppc, FUNCNAME, offset*2,data,mem_mask);
			COMBINE_DATA(&sim.m_pitr);
			LOGSIM("- SWP  : Software watchdog prescale factor is %d\n", (data & m68340_sim::REG_PITR_SWP) ? 512 : 1);
			LOGPIT("- PTP  : Periodic timer prescale factor is %d\n", (data & m68340_sim::REG_PITR_PTP) ? 512 : 1);
			LOGPIT("- COUNT: is %d%s\n", (data & m68340_sim::REG_PITR_COUNT), (data & m68340_sim::REG_PITR_COUNT) == 0 ? " (off)" : "d");
			if ( (sim.m_mcr & m68340_sim::REG_MCR_FRZ1) == 0 && (sim.m_pitr & m68340_sim::REG_PITR_COUNT) != 0)
			{
				LOGPIT("Starting PIT timer\n");
				sim.m_pit_counter = sim.m_pitr & m68340_sim::REG_PITR_COUNT;
				m_irq_timer->adjust(cycles_to_attotime((m_crystal / 4) / ((sim.m_pitr & m68340_sim::REG_PITR_PTP) != 0 ? 512 : 1)));
			}

			break;

		case m68340_sim::REG_SWSR:
			// basically watchdog, you must write an alternating pattern of 0x55 / 0xaa to keep the watchdog from resetting the system
			//LOGSIM("- %08x %s %04x, %04x (%04x) (SWSR - Software Service)\n", m_ppc, FUNCNAME, offset*2,data,mem_mask);
			break;

		default:
			LOGSIM("- %08x %s %04x, %04x (%04x) - not implemented\n", m_ppc, FUNCNAME, offset*2,data,mem_mask);

	}
}

uint8_t m68340_cpu_device::m68340_internal_sim_ports_r(offs_t offset)
{
	LOGR("%s\n", FUNCNAME);
	offset += 0x10;
	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;

	int val =  machine().rand();

	switch (offset)
	{
		case m68340_sim::REG_PORTA:
			LOGR("- %08x %s %04x (PORTA - Port A Data)\n", m_ppc, FUNCNAME, offset);
			sim.m_porta &= sim.m_ddra;
			// TODO: call callback

			if (!m_pa_in_cb.isnull())
			{
				sim.m_porta |= (m_pa_in_cb() & ~sim.m_ddra);
			}
#if 0
			else
			{
				sim.m_porta |= (sim.m_pail & ~sim.m_ddra);
			}
#endif
			val = sim.m_porta;
			break;

		case m68340_sim::REG_DDRA:
			LOGR("- %08x %s %04x (DDRA - Port A Data Direction)\n", m_ppc, FUNCNAME, offset);
			val = sim.m_ddra;
			break;

		case m68340_sim::REG_PPARA1:
			LOGR("- %08x %s %04x (PPRA1 - Port A Pin Assignment 1)\n", m_ppc, FUNCNAME, offset);
			val = sim.m_ppara1;
			break;

		case m68340_sim::REG_PPARA2:
			LOGR("- %08x %s %04x (PPRA2 - Port A Pin Assignment 2) - not implemented\n", m_ppc, FUNCNAME, offset);
			val = sim.m_ppara2;
			break;

		case m68340_sim::REG_PORTB1:
			LOGR("- %08x %s %04x (PORTB1 - Port B Data 1)\n", m_ppc, FUNCNAME, offset);
			// Fallthrough to mirror register
			[[fallthrough]];
		case m68340_sim::REG_PORTB:
			LOGR("- %08x %s %04x (PORTB - Port B Data 0)\n", m_ppc, FUNCNAME, offset);
			sim.m_portb &= sim.m_ddrb;
			// TODO: call callback

			if (!m_pb_in_cb.isnull())
			{
				sim.m_portb |= (m_pb_in_cb() & ~sim.m_ddrb);
			}
#if 0
			else
			{
				sim.m_portb |= (sim.m_pbil & ~sim.m_ddrb);
			}
#endif
			val = sim.m_portb;
			break;

		case m68340_sim::REG_DDRB:
			LOGR("- %08x %s %04x (DDR - Port B Data Direction)\n", m_ppc, FUNCNAME, offset);
			val = sim.m_ddrb;
			break;

		case m68340_sim::REG_PPARB:
			LOGR("- %08x %s %04x (PPARB - Port B Pin Assignment)\n", m_ppc, FUNCNAME, offset);
			val = sim.m_pparb;
			break;

		default:
			LOGR("- %08x %s %04x (ILLEGAL?)\n", m_ppc, FUNCNAME, offset);
			logerror("%08x m68340_internal_sim_r %04x (ILLEGAL?)\n", m_ppc, FUNCNAME, offset);
			break;
	}
	LOGR(" * Reg %02x -> %02x - %s\n", offset, val, std::array<char const *, 16>
		 {{"", "PORTA", "", "DDRA", "", "PPARA1", "", "PPARA2", "", "PORTB","", "PORTB1", "", "DDRB", "", "PPARB"}}[offset - 0x10]);

	return val;
}

void m68340_cpu_device::m68340_internal_sim_ports_w(offs_t offset, uint8_t data)
{
	LOG("%s", FUNCNAME);
	offset += 0x10;
	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;

	LOGSETUP(" * Reg %02x <- %02x - %s\n", offset, data, std::array<char const *, 8>
			 {{"PORTA", "DDRA", "PPRA1", "PPRA2", "PORTB", "PORTB1", "DDRB", "PPARB"}}[(offset - 0x10) / 2]);
	switch (offset)
	{
		case m68340_sim::REG_PORTA:
			LOGDATA("- %08x %04x, %02x (PORTA - Port A Data)\n", m_ppc, offset,data);
			sim.m_porta = (data & sim.m_ddra & sim.m_ppara1);

			// callback
			m_pa_out_cb ((offs_t)0, sim.m_porta);
			break;

		case m68340_sim::REG_DDRA:
			LOGPORTS("- %08x %04x, %02x (DDRA - Port A Data Direction)\n", m_ppc, offset,data);
			sim.m_ddra = data;
			break;

		case m68340_sim::REG_PPARA1:
			LOGPORTS("- %08x %04x, %02x (PPARA1 - Port A Pin Assignment 1)\n", m_ppc, offset,data);
			sim.m_ppara1 = data;
			break;

		case m68340_sim::REG_PPARA2:
			LOGPORTS("- %08x %04x, %02x (PPARA2 - Port A Pin Assignment 2)\n", m_ppc, offset,data);
			sim.m_ppara2 = data;
			break;

		case m68340_sim::REG_PORTB1:
			LOGDATA("- %08x %04x, %02x (PORTB1 - Port B Data - mirror)\n", m_ppc, offset,data);
			// Falling through to mirrored register portb
			[[fallthrough]];
		case m68340_sim::REG_PORTB:
			LOGDATA("- %08x %04x, %02x (PORTB - Port B Data)\n", m_ppc, offset,data);
			sim.m_portb = (data & sim.m_ddrb & sim.m_pparb);

			// callback
			m_pb_out_cb ((offs_t)0, sim.m_portb);
			break;

		case m68340_sim::REG_DDRB:
			LOGPORTS("- %08x %04x, %02x (DDR - Port B Data Direction)\n", m_ppc, offset,data);
			sim.m_ddrb = data;
			break;

		case m68340_sim::REG_PPARB:
			LOGPORTS("- %08x %04x, %02x (PPARB - Port B Pin Assignment)\n", m_ppc, offset,data);
			sim.m_pparb = data;
			break;

		default:
			LOGPORTS("- %08x %s %04x, %02x (ILLEGAL?) - not implemented\n", m_ppc, FUNCNAME, offset,data);
			logerror("%08x m68340_internal_sim_ports_w %04x, %02x (ILLEGAL?)\n", m_ppc, offset,data);
			break;
	}
}

uint32_t m68340_cpu_device::m68340_internal_sim_cs_r(offs_t offset, uint32_t mem_mask)
{
	LOGR("%s\n", FUNCNAME);
	offset += m68340_sim::REG_AM_CS0>>2;

	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;

	switch (offset<<2)
	{
		case m68340_sim::REG_AM_CS0:  return sim.m_am[0];
		case m68340_sim::REG_BA_CS0:  return sim.m_ba[0];
		case m68340_sim::REG_AM_CS1:  return sim.m_am[1];
		case m68340_sim::REG_BA_CS1:  return sim.m_ba[1];
		case m68340_sim::REG_AM_CS2:  return sim.m_am[2];
		case m68340_sim::REG_BA_CS2:  return sim.m_ba[2];
		case m68340_sim::REG_AM_CS3:  return sim.m_am[3];
		case m68340_sim::REG_BA_CS3:  return sim.m_ba[3];

		default:
			logerror("%08x m68340_internal_sim_r %08x, (%08x)\n", m_ppc, offset*4,mem_mask);
	}

	return 0x00000000;
}

void m68340_cpu_device::m68340_internal_sim_cs_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOG("%s\n", FUNCNAME);
	offset += m68340_sim::REG_AM_CS0>>2;

	if (offset & 1)
	{
	  LOGCS("%08x Base address CS%d %08x, %08x (%08x) ", m_ppc, (offset - 0x10) / 2, offset * 4, data, mem_mask);
	  LOGCS("- Base: %08x BFC:%02x WP:%d FTE:%d NCS:%d Valid: %s\n", data & 0xffffff00, (data & 0xf0) >> 4, data & 0x08 ? 1 : 0, data & 0x04 ? 1 : 0, data & 0x02 ? 1 : 0, data & 0x01 ? "Yes" : "No");
	}
	else
	{
	  LOGCS("%08x Address mask CS%d %08x, %08x (%08x) ", m_ppc, (offset - 0x10) / 2, offset * 4, data, mem_mask);
	  LOGCS("- Mask: %08x FCM:%02x DD:%d PS: %s\n", data & 0xffffff00, (data & 0xf0) >> 4, (data >> 2) & 0x03, std::array<char const *, 4>{{"Reserved", "16-Bit", "8-bit", "External DSACK response"}}[data & 0x03]);
	}

	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;

	switch (offset << 2)
	{
		case m68340_sim::REG_AM_CS0:
			COMBINE_DATA(&sim.m_am[0]);
			break;

		case m68340_sim::REG_BA_CS0:
			COMBINE_DATA(&sim.m_ba[0]);
			break;

		case m68340_sim::REG_AM_CS1:
			COMBINE_DATA(&sim.m_am[1]);
			break;

		case m68340_sim::REG_BA_CS1:
			COMBINE_DATA(&sim.m_ba[1]);
			break;

		case m68340_sim::REG_AM_CS2:
			COMBINE_DATA(&sim.m_am[2]);
			break;

		case m68340_sim::REG_BA_CS2:
			COMBINE_DATA(&sim.m_ba[2]);
			break;

		case m68340_sim::REG_AM_CS3:
			COMBINE_DATA(&sim.m_am[3]);
			break;

		case m68340_sim::REG_BA_CS3:
			COMBINE_DATA(&sim.m_ba[3]);
			break;

		default:
			logerror("%08x m68340_internal_sim_cs_w %08x, %08x (%08x)\n", m_ppc, offset*4,data,mem_mask);
			break;
	}
}

TIMER_CALLBACK_MEMBER(m68340_cpu_device::periodic_interrupt_timer_callback)
{
	do_tick_pit();
}

void m68340_cpu_device::start_68340_sim()
{
	LOG("%s\n", FUNCNAME);
	LOGCLOCK( " - Clock: %d [0x%08x]\n", clock(), clock());
	m_irq_timer = timer_alloc(FUNC(m68340_cpu_device::periodic_interrupt_timer_callback), this);

	// resolve callbacks Port A
	m_pa_out_cb.resolve_safe();
	m_pa_in_cb.resolve();

	// resolve callbacks Port B
	m_pb_out_cb.resolve_safe();
	m_pb_in_cb.resolve();

	// Setup correct VCO/clock speed based on reset values and crystal
	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;
	switch (m_clock_mode)
	{
	case m68340_sim::CLOCK_MODE_EXT:
		LOGCLOCK("External Clock Mode\n");
		break;
	case m68340_sim::CLOCK_MODE_EXT_PLL:
		LOGCLOCK("External Clock Mode with PLL - not implemented\n");
		logerror("External Clock Mode with PLL - not implemented\n");
		break;
	case m68340_sim::CLOCK_MODE_CRYSTAL:
		LOGCLOCK("Crystal Mode with VCO and PLL\n");
		set_unscaled_clock(m_crystal *
						   (4 << (((sim.m_syncr & m68340_sim::REG_SYNCR_W) != 0 ? 2 : 0) + ((sim.m_syncr & m68340_sim::REG_SYNCR_X) != 0 ? 1 : 0))) *
						   (((sim.m_syncr & m68340_sim::REG_SYNCR_Y_MSK) >> 8) & 0x3f));
		LOGCLOCK("SYNCR: %04x\n", sim.m_syncr);
		break;
	default:
		logerror("Unknown Clock mode, check schematics and/or the source code\n");
	}
	LOGCLOCK( " - Clock: %d [0x%08x]\n", clock(), clock());
}

void m68340_sim::reset()
{
	LOG("%s\n", FUNCNAME);

	// Ports - only setup those that are affected by reset
	m_ddra   = 0x00;
	m_ppara1 = 0xff;
	m_ppara2 = 0x00;
	m_ddrb   = 0x00;
	m_pparb  = 0xff;

	// SIM
	m_mcr   = 0x608f;
	m_syncr = 0x3f00;
	m_avr_rsr = 0x0000;
	m_swiv_sypcr = 0x0f00;
	m_picr= 0x000f;
	m_pitr= 0x0000; // TODO: read MODCK pin to set the clock modes for watch dog and periodic timer
	m_swsr= 0x0000;
	m_pit_irq = false;
}

void m68340_sim::module_reset()
{
	// SYS set in RSR, nothing else happens
	m_avr_rsr = (m_avr_rsr & 0xff00) | 0x02;
}

/* do_tick_pit works on whole clock cycles, no flank support */
void m68340_cpu_device::do_tick_pit()
{
	assert(m_m68340SIM);
	m68340_sim &sim = *m_m68340SIM;

	sim.m_pit_counter--;
	if ( ( (sim.m_mcr & m68340_sim::REG_MCR_FRZ1) == 0) &&
		 ( (sim.m_pit_counter != 0 || (sim.m_pitr & m68340_sim::REG_PITR_COUNT) != 0)))
	{
		LOGPIT("Re-arming PIT timer\n");
		sim.m_pit_counter = sim.m_pitr & m68340_sim::REG_PITR_COUNT;
		m_irq_timer->adjust(cycles_to_attotime((m_crystal / 4) / ((sim.m_pitr & m68340_sim::REG_PITR_PTP) != 0 ? 512 : 1)));
	}
	else
	{
		LOGPIT("Stopping PIT timer dues to %s\n", (sim.m_mcr & m68340_sim::REG_MCR_FRZ1) != 0 ? "Freeze" : "Counter disabled");
		m_irq_timer->adjust(attotime::never);
	}

	if (sim.m_pit_counter == 0) // Zero detect
	{
		LOGPIT("PIT timer reached zero!\n");
		sim.m_pit_irq = true;
		update_ipl();
	}
}

WRITE_LINE_MEMBER( m68340_cpu_device::extal_w )
{
	LOGPIT("%s H1 set to %d\n", FUNCNAME, state);
	m_extal = state;
	if (m_extal == ASSERT_LINE)
	{
		do_tick_pit();
	}
}

uint8_t m68340_cpu_device::pit_irq_level() const
{
	return m_m68340SIM->m_pit_irq ? (m_m68340SIM->m_picr & m68340_sim::REG_PICR_PIRQL) >> 8 : 0;
}

uint8_t m68340_cpu_device::pit_arbitrate(uint8_t level) const
{
	if (pit_irq_level() == level)
		return m_m68340SIM->m_mcr & m68340_sim::REG_MCR_ARBLV;
	else
		return 0;
}

uint8_t m68340_cpu_device::pit_iack()
{
	m_m68340SIM->m_pit_irq = false;
	update_ipl();
	return m_m68340SIM->m_picr & m68340_sim::REG_PICR_PIVEC;
}
