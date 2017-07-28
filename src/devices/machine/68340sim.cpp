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

//#define VERBOSE  (LOG_PORTS|LOG_SETUP|LOG_READ)
#define LOG_OUTPUT_FUNC printf // Needs always to be enabled as the default value 'logerror' is not available here

#include "logmacro.h"

//#define LOG(...) LOGMASKED(LOG_GENERAL,   __VA_ARGS__) // Already defined in logmacro.h
#define LOGSETUP(...) LOGMASKED(LOG_SETUP, __VA_ARGS__)
#define LOGR(...)     LOGMASKED(LOG_READ,  __VA_ARGS__)
#define LOGPORTS(...) LOGMASKED(LOG_PORTS, __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#define LLFORMAT "%I64d"
#else
#define FUNCNAME __PRETTY_FUNCTION__
#define LLFORMAT "%lld"
#endif

#define m68340SIM_MCR          (0x00)
//                             (0x02)
#define m68340SIM_SYNCR        (0x04)
#define m68340SIM_AVR_RSR      (0x06)
//                             (0x08)
//                             (0x0a)
//                             (0x0c)
//                             (0x0e)
#define m68340SIM_PORTA        (0x11)
#define m68340SIM_DDRA         (0x13)
#define m68340SIM_PPARA1       (0x15)
#define m68340SIM_PPARA2       (0x17)
#define m68340SIM_PORTB        (0x19)
#define m68340SIM_PORTB1       (0x1b)
#define m68340SIM_DDRB         (0x1d)
#define m68340SIM_PPARB        (0x1f)
#define m68340SIM_SWIV_SYPCR   (0x20)
#define m68340SIM_PICR         (0x22)
#define m68340SIM_PITR         (0x24)
#define m68340SIM_SWSR         (0x26)
//                             (0x28)
//                             (0x2a)
//                             (0x2c)
//                             (0x2e)
//                             (0x30)
//                             (0x32)
//                             (0x34)
//                             (0x36)
//                             (0x38)
//                             (0x3a)
//                             (0x3c)
//                             (0x3e)
#define m68340SIM_AM_CS0       (0x40)
#define m68340SIM_BA_CS0       (0x44)
#define m68340SIM_AM_CS1       (0x48)
#define m68340SIM_BA_CS1       (0x4c)
#define m68340SIM_AM_CS2       (0x50)
#define m68340SIM_BA_CS2       (0x54)
#define m68340SIM_AM_CS3       (0x58)
#define m68340SIM_BA_CS3       (0x5c)



READ16_MEMBER( m68340_cpu_device::m68340_internal_sim_r )
{
	LOGR("%s\n", FUNCNAME);
	assert(m68340SIM);
	//m68340_sim &sim = *m68340SIM;

	int pc = space.device().safe_pc();

	switch (offset<<1)
	{
		case m68340SIM_MCR:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x) (MCR - Module Configuration Register) - not implemented\n", pc, offset*2,mem_mask);
			return space.machine().rand();

		case m68340SIM_SYNCR:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x) (SYNCR - Clock Synthesizer Register) - not implemented\n", pc, offset*2,mem_mask);
			return space.machine().rand();

		case m68340SIM_AVR_RSR:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x) (AVR, RSR - Auto Vector Register, Reset Status Register) - not implemented\n", pc, offset*2,mem_mask);
			return space.machine().rand();

		case m68340SIM_SWIV_SYPCR:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x) (SWIV_SYPCR - Software Interrupt Vector, System Protection Control Register) - not implemented\n", pc, offset*2,mem_mask);
			return space.machine().rand();

		case m68340SIM_PICR:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x) (PICR - Periodic Interrupt Control Register) - not implemented\n", pc, offset*2,mem_mask);
			return space.machine().rand();

		case m68340SIM_PITR:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x) (PITR - Periodic Interrupt Timer Register) - not implemented\n", pc, offset*2,mem_mask);
			return space.machine().rand();

		case m68340SIM_SWSR:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x) (SWSR - Software Service) - not implemented\n", pc, offset*2,mem_mask);
			return space.machine().rand();

		default:
			LOGR("- %08x m68340_internal_sim_r %04x, (%04x)\n", pc, offset*2,mem_mask);
	}

	return 0x0000;
}

READ8_MEMBER( m68340_cpu_device::m68340_internal_sim_ports_r )
{
	LOGR("%s\n", FUNCNAME);
	offset += 0x10;
	assert(m68340SIM);
	m68340_sim &sim = *m68340SIM;

	int pc = space.device().safe_pc();
	int val =  space.machine().rand();

	switch (offset)
	{
		case m68340SIM_PORTA:
			LOGR("- %08x m68340_internal_sim_r %04x (PORTA - Port A Data)\n", pc, offset);
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

		case m68340SIM_DDRA:
			LOGR("- %08x m68340_internal_sim_r %04x (DDRA - Port A Data Direction)\n", pc, offset);
			val = sim.m_ddra;
			break;

		case m68340SIM_PPARA1:
			LOGR("- %08x m68340_internal_sim_r %04x (PPRA1 - Port A Pin Assignment 1)\n", pc, offset);
			val = sim.m_ppara1;
			break;

		case m68340SIM_PPARA2:
			LOGR("- %08x m68340_internal_sim_r %04x (PPRA2 - Port A Pin Assignment 2) - not implemented\n", pc, offset);
			val = sim.m_ppara2;
			break;

		case m68340SIM_PORTB1:
			LOGR("- %08x m68340_internal_sim_r %04x (PORTB1 - Port B Data 1)\n", pc, offset);
			// Fallthrough to mirror register
		case m68340SIM_PORTB:
			LOGR("- %08x m68340_internal_sim_r %04x (PORTB - Port B Data 0)\n", pc, offset);
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

		case m68340SIM_DDRB:
			LOGR("- %08x m68340_internal_sim_r %04x (DDR - Port B Data Direction)\n", pc, offset);
			val = sim.m_ddrb;
			break;

		case m68340SIM_PPARB:
			LOGR("- %08x m68340_internal_sim_r %04x (PPARB - Port B Pin Assignment)\n", pc, offset);
			val = sim.m_pparb;
			break;

		default:
			LOGR("- %08x m68340_internal_sim_r %04x (ILLEGAL?)\n", pc, offset);
			logerror("%08x m68340_internal_sim_r %04x (ILLEGAL?)\n", pc, offset);
			break;
	}
	LOGR(" * Reg %02x -> %02x - %s\n", offset, val, std::array<char const *, 16>
		 {{"", "PORTA", "", "DDRA", "", "PPARA1", "", "PPARA2", "", "PORTB","", "PORTB1", "", "DDRB", "", "PPARB"}}[offset - 0x10]);

	return val;
}

READ32_MEMBER( m68340_cpu_device::m68340_internal_sim_cs_r )
{
	LOGR("%s\n", FUNCNAME);
	offset += m68340SIM_AM_CS0>>2;

	assert(m68340SIM);
	m68340_sim &sim = *m68340SIM;

	int pc = space.device().safe_pc();

	switch (offset<<2)
	{
		case m68340SIM_AM_CS0:  return sim.m_am[0];
		case m68340SIM_BA_CS0:  return sim.m_ba[0];
		case m68340SIM_AM_CS1:  return sim.m_am[1];
		case m68340SIM_BA_CS1:  return sim.m_ba[1];
		case m68340SIM_AM_CS2:  return sim.m_am[2];
		case m68340SIM_BA_CS2:  return sim.m_ba[2];
		case m68340SIM_AM_CS3:  return sim.m_am[3];
		case m68340SIM_BA_CS3:  return sim.m_ba[3];

		default:
			logerror("%08x m68340_internal_sim_r %08x, (%08x)\n", pc, offset*4,mem_mask);
	}

	return 0x00000000;
}

WRITE16_MEMBER( m68340_cpu_device::m68340_internal_sim_w )
{
	LOGSETUP("%s\n", FUNCNAME);
	assert(m68340SIM);
	//m68340_sim &sim = *m68340SIM;

	int pc = space.device().safe_pc();

	switch (offset<<1)
	{
		case m68340SIM_MCR:
			LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) (MCR - Module Configuration Register) - not implemented\n", pc, offset*2,data,mem_mask);
			break;

		case m68340SIM_SYNCR:
			LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) (SYNCR - Clock Synthesizer Register) - not implemented\n", pc, offset*2,data,mem_mask);
			break;


		case m68340SIM_AVR_RSR:
			LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) (AVR, RSR - Auto Vector Register, Reset Status Register)\n", pc, offset*2,data,mem_mask);
			COMBINE_DATA(&m_avr);
			break;

		case m68340SIM_SWIV_SYPCR:
			LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) (SWIV_SYPCR - Software Interrupt Vector, System Protection Control Register) - not implemented\n", pc, offset*2,data,mem_mask);
			break;

		case m68340SIM_PICR:
			LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) (PICR - Periodic Interrupt Control Register)\n", pc, offset*2,data,mem_mask);
			COMBINE_DATA(&m_picr);
			break;

		case m68340SIM_PITR:
			LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) (PITR - Periodic Interrupt Timer Register)\n", pc, offset*2,data,mem_mask);
			COMBINE_DATA(&m_pitr);
			if (m_pitr !=0 ) // hack
			{
				//LOGSETUP("- timer set\n");
				m_irq_timer->adjust(cycles_to_attotime(20000)); // hack
			}

			break;

		case m68340SIM_SWSR:
			// basically watchdog, you must write an alternating pattern of 0x55 / 0xaa to keep the watchdog from resetting the system
			//LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) (SWSR - Software Service)\n", pc, offset*2,data,mem_mask);
			break;

		default:
			LOGSETUP("- %08x m68340_internal_sim_w %04x, %04x (%04x) - not implemented\n", pc, offset*2,data,mem_mask);

	}
}

WRITE8_MEMBER( m68340_cpu_device::m68340_internal_sim_ports_w )
{
	LOG("%s", FUNCNAME);
	offset += 0x10;
	assert(m68340SIM);
	m68340_sim &sim = *m68340SIM;

	int pc = space.device().safe_pc();

	LOGSETUP(" * Reg %02x <- %02x - %s\n", offset, data, std::array<char const *, 16>
		 {{"", "PORTA", "", "DDRA", "", "PPRA1", "", "PPRA2", "", "PORTB","", "PORTB1", "", "DDRB", "", "PPARB"}}[offset - 0x10]);

	switch (offset)
	{
		case m68340SIM_PORTA:
			LOGPORTS("- %08x %04x, %02x (PORTA - Port A Data)\n", pc, offset,data);
			sim.m_porta = (data & sim.m_ddra & sim.m_ppara1);

			// callback
			m_pa_out_cb ((offs_t)0, sim.m_porta);
			break;

		case m68340SIM_DDRA:
			LOGPORTS("- %08x %04x, %02x (DDRA - Port A Data Direction)\n", pc, offset,data);
			sim.m_ddra = data;
			break;

		case m68340SIM_PPARA1:
			LOGPORTS("- %08x %04x, %02x (PPARA1 - Port A Pin Assignment 1)\n", pc, offset,data);
			sim.m_ppara1 = data;
			break;

		case m68340SIM_PPARA2:
			LOGPORTS("- %08x %04x, %02x (PPARA2 - Port A Pin Assignment 2)\n", pc, offset,data);
			sim.m_ppara2 = data;
			break;

		case m68340SIM_PORTB1:
			LOGPORTS("- %08x %04x, %02x (PORTB1 - Port B Data - mirror)\n", pc, offset,data);
			// Falling through to mirrored register portb
		case m68340SIM_PORTB:
			LOGPORTS("- %08x %04x, %02x (PORTB - Port B Data)\n", pc, offset,data);
			sim.m_portb = (data & sim.m_ddrb & sim.m_pparb);

			// callback
			m_pb_out_cb ((offs_t)0, sim.m_portb);
			break;

		case m68340SIM_DDRB:
			LOGPORTS("- %08x %04x, %02x (DDR - Port B Data Direction)\n", pc, offset,data);
			sim.m_ddrb = data;
			break;

		case m68340SIM_PPARB:
			LOGPORTS("- %08x %04x, %02x (PPARB - Port B Pin Assignment)\n", pc, offset,data);
			sim.m_pparb = data;
			break;

		default:
			LOGPORTS("- %08x %04x, %02x (ILLEGAL?) - not implemented\n", pc, offset,data);
			logerror("%08x m68340_internal_sim_w %04x, %02x (ILLEGAL?)\n", pc, offset,data);
			break;
	}
}

WRITE32_MEMBER( m68340_cpu_device::m68340_internal_sim_cs_w )
{
	LOGSETUP("%s\n", FUNCNAME);
	offset += m68340SIM_AM_CS0>>2;
	assert(m68340SIM);
	m68340_sim &sim = *m68340SIM;

	int pc = space.device().safe_pc();

	switch (offset<<2)
	{
		case m68340SIM_AM_CS0:
			COMBINE_DATA(&sim.m_am[0]);
			break;

		case m68340SIM_BA_CS0:
			COMBINE_DATA(&sim.m_ba[0]);
			break;

		case m68340SIM_AM_CS1:
			COMBINE_DATA(&sim.m_am[1]);
			break;

		case m68340SIM_BA_CS1:
			COMBINE_DATA(&sim.m_ba[1]);
			break;

		case m68340SIM_AM_CS2:
			COMBINE_DATA(&sim.m_am[2]);
			break;

		case m68340SIM_BA_CS2:
			COMBINE_DATA(&sim.m_ba[2]);
			break;

		case m68340SIM_AM_CS3:
			COMBINE_DATA(&sim.m_am[3]);
			break;

		case m68340SIM_BA_CS3:
			COMBINE_DATA(&sim.m_ba[3]);
			break;

		default:
			LOGSETUP("- %08x m68340_internal_sim_w %08x, %08x (%08x) - not implemented\n", pc, offset*4,data,mem_mask);
			logerror("%08x m68340_internal_sim_w %08x, %08x (%08x)\n", pc, offset*4,data,mem_mask);
			break;
	}
}

void m68340_cpu_device::do_timer_irq()
{
	//logerror("do_timer_irq\n");
	int timer_irq_level  = (m_picr & 0x0700)>>8;
	int timer_irq_vector = (m_picr & 0x00ff)>>0;

	if (timer_irq_level) // 0 is irq disabled
	{
		int use_autovector = (m_avr >> timer_irq_level)&1;

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

TIMER_CALLBACK_MEMBER(m68340_cpu_device::periodic_interrupt_timer_callback)
{
	do_timer_irq();
	m_irq_timer->adjust(cycles_to_attotime(20000)); // hack
}

void m68340_cpu_device::start_68340_sim()
{
	m_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m68340_cpu_device::periodic_interrupt_timer_callback),this));

	// resolve callbacks Port A
	m_pa_out_cb.resolve_safe();
	m_pa_in_cb.resolve();

	// resolve callbacks Port B
	m_pb_out_cb.resolve_safe();
	m_pb_in_cb.resolve();
}

void m68340_sim::reset()
{
	LOGSETUP("%s\n", FUNCNAME);

	// Ports - only setup those that are affected by reset
	m_ddra   = 0x00;
	m_ppara1 = 0xff;
	m_ppara2 = 0x00;
	m_ddrb   = 0x00;
	m_pparb  = 0xff;
}
