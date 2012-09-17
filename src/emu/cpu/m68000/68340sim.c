/* 68340 SIM module */

#include "emu.h"
#include "m68kcpu.h"


READ16_HANDLER( m68340_internal_sim_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = space.device().safe_pc();

		switch (offset<<1)
		{
			case m68340SIM_MCR:
				logerror("%08x m68340_internal_sim_r %04x, (%04x) (MCR - Module Configuration Register)\n", pc, offset*2,mem_mask);
				return space.machine().rand();

			case m68340SIM_SYNCR:
				logerror("%08x m68340_internal_sim_r %04x, (%04x) (SYNCR - Clock Synthesizer Register)\n", pc, offset*2,mem_mask);
				return space.machine().rand();

			case m68340SIM_AVR_RSR:
				logerror("%08x m68340_internal_sim_r %04x, (%04x) (AVR, RSR - Auto Vector Register, Reset Status Register)\n", pc, offset*2,mem_mask);
				return space.machine().rand();

			case m68340SIM_SWIV_SYPCR:
				logerror("%08x m68340_internal_sim_r %04x, (%04x) (SWIV_SYPCR - Software Interrupt Vector, System Protection Control Register)\n", pc, offset*2,mem_mask);
				return space.machine().rand();

			case m68340SIM_PICR:
				logerror("%08x m68340_internal_sim_r %04x, (%04x) (PICR - Periodic Interrupt Control Register)\n", pc, offset*2,mem_mask);
				return space.machine().rand();

			case m68340SIM_PITR:
				logerror("%08x m68340_internal_sim_r %04x, (%04x) (PITR - Periodic Interrupt Timer Register)\n", pc, offset*2,mem_mask);
				return space.machine().rand();

			case m68340SIM_SWSR:
				logerror("%08x m68340_internal_sim_r %04x, (%04x) (SWSR - Software Service)\n", pc, offset*2,mem_mask);
				return space.machine().rand();

			default:
				logerror("%08x m68340_internal_sim_r %04x, (%04x)\n", pc, offset*2,mem_mask);


		}
	}

	return 0x0000;
}

READ8_HANDLER( m68340_internal_sim_ports_r )
{
	offset += 0x10;
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = space.device().safe_pc();

		switch (offset)
		{
			case m68340SIM_PORTA:
				logerror("%08x m68340_internal_sim_r %04x (PORTA - Port A Data)\n", pc, offset);
				return space.machine().rand();

			case m68340SIM_DDRA:
				logerror("%08x m68340_internal_sim_r %04x (DDRA - Port A Data Direction)\n", pc, offset);
				return space.machine().rand();

			case m68340SIM_PPRA1:
				logerror("%08x m68340_internal_sim_r %04x (PPRA1 - Port A Pin Assignment 1)\n", pc, offset);
				return space.machine().rand();

			case m68340SIM_PPRA2:
				logerror("%08x m68340_internal_sim_r %04x (PPRA2 - Port A Pin Assignment 2)\n", pc, offset);
				return space.machine().rand();

			case m68340SIM_PORTB:
				logerror("%08x m68340_internal_sim_r %04x (PORTB - Port B Data 0)\n", pc, offset);
				return space.machine().rand();

			case m68340SIM_PORTB1:
				logerror("%08x m68340_internal_sim_r %04x (PORTB1 - Port B Data 1)\n", pc, offset);
				return space.machine().rand();

			case m68340SIM_DDRB:
				logerror("%08x m68340_internal_sim_r %04x (DDR - Port B Data Direction)\n", pc, offset);
				return space.machine().rand();

			case m68340SIM_PPARB:
				logerror("%08x m68340_internal_sim_r %04x (PPARB - Port B Pin Assignment)\n", pc, offset);
				return space.machine().rand();

			default:
				logerror("%08x m68340_internal_sim_r %04x (ILLEGAL?)\n", pc, offset);
				return space.machine().rand();

		}
	}

	return 0x00;
}

READ32_HANDLER( m68340_internal_sim_cs_r )
{
	offset += m68340SIM_AM_CS0>>2;

	m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = space.device().safe_pc();

		switch (offset<<2)
		{
			case m68340SIM_AM_CS0:	return sim->m_am[0];
			case m68340SIM_BA_CS0:	return sim->m_ba[0];
			case m68340SIM_AM_CS1:	return sim->m_am[1];
			case m68340SIM_BA_CS1:	return sim->m_ba[1];
			case m68340SIM_AM_CS2:	return sim->m_am[2];
			case m68340SIM_BA_CS2:	return sim->m_ba[2];
			case m68340SIM_AM_CS3:	return sim->m_am[3];
			case m68340SIM_BA_CS3:	return sim->m_ba[3];

			default:
				logerror("%08x m68340_internal_sim_r %08x, (%08x)\n", pc, offset*4,mem_mask);

		}
	}

	return 0x00000000;
}

WRITE16_HANDLER( m68340_internal_sim_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = space.device().safe_pc();

		switch (offset<<1)
		{
			case m68340SIM_MCR:
				logerror("%08x m68340_internal_sim_w %04x, %04x (%04x) (MCR - Module Configuration Register)\n", pc, offset*2,data,mem_mask);
				break;

			case m68340SIM_SYNCR:
				logerror("%08x m68340_internal_sim_w %04x, %04x (%04x) (SYNCR - Clock Synthesizer Register)\n", pc, offset*2,data,mem_mask);
				break;


			case m68340SIM_AVR_RSR:
				logerror("%08x m68340_internal_sim_w %04x, %04x (%04x) (AVR, RSR - Auto Vector Register, Reset Status Register)\n", pc, offset*2,data,mem_mask);
				break;

			case m68340SIM_SWIV_SYPCR:
				logerror("%08x m68340_internal_sim_w %04x, %04x (%04x) (SWIV_SYPCR - Software Interrupt Vector, System Protection Control Register)\n", pc, offset*2,data,mem_mask);
				break;

			case m68340SIM_PICR:
				logerror("%08x m68340_internal_sim_w %04x, %04x (%04x) (PICR - Periodic Interrupt Control Register)\n", pc, offset*2,data,mem_mask);
				break;

			case m68340SIM_PITR:
				logerror("%08x m68340_internal_sim_w %04x, %04x (%04x) (PITR - Periodic Interrupt Timer Register)\n", pc, offset*2,data,mem_mask);
				break;

			case m68340SIM_SWSR:
				// basically watchdog, you must write an alternating pattern of 0x55 / 0xaa to keep the watchdog from resetting the system
				//logerror("%08x m68340_internal_sim_w %04x, %04x (%04x) (SWSR - Software Service)\n", pc, offset*2,data,mem_mask);
				break;

			default:
				logerror("%08x m68340_internal_sim_w %04x, %04x (%04x)\n", pc, offset*2,data,mem_mask);

		}
	}
}

WRITE8_HANDLER( m68340_internal_sim_ports_w )
{
	offset += 0x10;
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = space.device().safe_pc();

		switch (offset)
		{
			case m68340SIM_PORTA:
				logerror("%08x m68340_internal_sim_w %04x, %02x (PORTA - Port A Data)\n", pc, offset,data);
				break;

			case m68340SIM_DDRA:
				logerror("%08x m68340_internal_sim_w %04x, %02x (DDRA - Port A Data Direction)\n", pc, offset,data);
				break;

			case m68340SIM_PPRA1:
				logerror("%08x m68340_internal_sim_w %04x, %02x (PPRA1 - Port A Pin Assignment 1)\n", pc, offset,data);
				break;

			case m68340SIM_PPRA2:
				logerror("%08x m68340_internal_sim_w %04x, %02x (PPRA2 - Port A Pin Assignment 2)\n", pc, offset,data);
				break;

			case m68340SIM_PORTB:
				logerror("%08x m68340_internal_sim_w %04x, %02x (PORTB - Port B Data)\n", pc, offset,data);
				break;

			case m68340SIM_PORTB1:
				logerror("%08x m68340_internal_sim_w %04x, %02x (PORTB1 - Port B Data - mirror)\n", pc, offset,data);
				break;

			case m68340SIM_DDRB:
				logerror("%08x m68340_internal_sim_w %04x, %02x (DDR - Port B Data Direction)\n", pc, offset,data);
				break;

			case m68340SIM_PPARB:
				logerror("%08x m68340_internal_sim_w %04x, %02x (PPARB - Port B Pin Assignment)\n", pc, offset,data);
				break;

			default:
				logerror("%08x m68340_internal_sim_w %04x, %02x (ILLEGAL?)\n", pc, offset,data);
				break;

		}
	}
}

WRITE32_HANDLER( m68340_internal_sim_cs_w )
{
	offset += m68340SIM_AM_CS0>>2;
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space.device());
	m68340_sim* sim = m68k->m68340SIM;
	assert(sim != NULL);

	if (sim)
	{
		int pc = space.device().safe_pc();

		switch (offset<<2)
		{
			case m68340SIM_AM_CS0:
				COMBINE_DATA(&sim->m_am[0]);
				break;

			case m68340SIM_BA_CS0:
				COMBINE_DATA(&sim->m_ba[0]);
				break;

			case m68340SIM_AM_CS1:
				COMBINE_DATA(&sim->m_am[1]);
				break;

			case m68340SIM_BA_CS1:
				COMBINE_DATA(&sim->m_ba[1]);
				break;

			case m68340SIM_AM_CS2:
				COMBINE_DATA(&sim->m_am[2]);
				break;

			case m68340SIM_BA_CS2:
				COMBINE_DATA(&sim->m_ba[2]);
				break;

			case m68340SIM_AM_CS3:
				COMBINE_DATA(&sim->m_am[3]);
				break;

			case m68340SIM_BA_CS3:
				COMBINE_DATA(&sim->m_ba[3]);
				break;

			default:
				logerror("%08x m68340_internal_sim_w %08x, %08x (%08x)\n", pc, offset*4,data,mem_mask);
				break;

		}
	}

}


void m68340_sim::reset(void)
{

}
