/* 68307 SIM module */

#include "emu.h"
#include "m68kcpu.h"


READ16_HANDLER( m68307_internal_sim_r )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_sim* sim = m68k->m68307SIM;
	assert(sim != NULL);

	int pc = cpu_get_pc(&space->device());
	logerror("%08x m68307_internal_sim_r %08x, (%04x)\n", pc, offset*2,mem_mask);

	if (sim)
	{
		switch (offset<<1)
		{
			case m68307SIM_BR0:	return (sim->m_br[0]);
			case m68307SIM_OR0:	return (sim->m_or[0]);
			case m68307SIM_BR1:	return (sim->m_br[1]);
			case m68307SIM_OR1:	return (sim->m_or[1]);
			case m68307SIM_BR2:	return (sim->m_br[2]);
			case m68307SIM_OR2:	return (sim->m_or[2]);
			case m68307SIM_BR3:	return (sim->m_br[3]);
			case m68307SIM_OR3:	return (sim->m_or[3]);
		}
	}


	return 0x0000;
}

	m68307_porta_read_callback m_m68307_porta_r;
	m68307_porta_write_callback m_m68307_porta_w;
	m68307_portb_read_callback m_m68307_portb_r;
	m68307_portb_write_callback m_m68307_portb_w;

WRITE16_HANDLER( m68307_internal_sim_w )
{
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	m68307_sim* sim = m68k->m68307SIM;
	assert(sim != NULL);

	int pc = cpu_get_pc(&space->device());

	if (sim)
	{
		switch (offset<<1)
		{

			case m68307SIM_PACNT:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port A (8-bit) Control Register - PACNT)\n", pc, offset*2,data,mem_mask);
				sim->write_pacnt(data,mem_mask);
				break;

			case m68307SIM_PADDR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port A (8-bit) Direction Register - PADDR)\n", pc, offset*2,data,mem_mask);
				sim->write_paddr(data,mem_mask);
				break;

			case m68307SIM_PADAT:
				sim->write_padat(space, data,mem_mask);
				break;

			case m68307SIM_PBCNT:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port B (16-bit) Control Register - PBCNT)\n", pc, offset*2,data,mem_mask);
				sim->write_pbcnt(data,mem_mask);
				break;

			case m68307SIM_PBDDR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Port B (16-bit) Direction Register - PBDDR)\n", pc, offset*2,data,mem_mask);
				sim->write_pbddr(data,mem_mask);
				break;

			case m68307SIM_PBDAT:
				sim->write_pbdat(space, data,mem_mask);
				break;


			case m68307SIM_LICR1:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Latched Interrupt Control Register 1 - LICR1)\n", pc, offset*2,data,mem_mask);
				sim->write_licr1(data,mem_mask);
				break;

			case m68307SIM_LICR2:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Latched Interrupt Control Register 2 - LICR2)\n", pc, offset*2,data,mem_mask);
				sim->write_licr2(data,mem_mask);
				break;

			case m68307SIM_PICR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Peripheral Interrupt Control Register - PICR)\n", pc, offset*2,data,mem_mask);
				sim->write_picr(data,mem_mask);
				break;
			
			case m68307SIM_PIVR:
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x) (Peripheral Interrupt Vector Register - PIVR)\n", pc, offset*2,data,mem_mask);
				sim->write_pivr(data,mem_mask);
				break;

			case m68307SIM_BR0:
				COMBINE_DATA(&sim->m_br[0]);
				break;
			case m68307SIM_OR0:
				COMBINE_DATA(&sim->m_or[0]);
				break;
			case m68307SIM_BR1:
				COMBINE_DATA(&sim->m_br[1]);
				break;
			case m68307SIM_OR1:
				COMBINE_DATA(&sim->m_or[1]);
				break;
			case m68307SIM_BR2:
				COMBINE_DATA(&sim->m_br[2]);
				break;
			case m68307SIM_OR2:
				COMBINE_DATA(&sim->m_or[2]);
				break;
			case m68307SIM_BR3:
				COMBINE_DATA(&sim->m_br[3]);
				break;
			case m68307SIM_OR3:
				COMBINE_DATA(&sim->m_or[3]);
				break;



			default :
				logerror("%08x m68307_internal_sim_w %08x, %04x (%04x)\n", pc, offset*2,data,mem_mask);
				break;

		}
	}
}


void m68307_sim::write_pacnt(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pacnt);
}

void m68307_sim::write_paddr(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_paddr);
}

void m68307_sim::write_padat(address_space *space, UINT16 data, UINT16 mem_mask)
{
	int pc = cpu_get_pc(&space->device());
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	COMBINE_DATA(&m_padat);

	if (m68k->m_m68307_porta_w)
	{
		m68k->m_m68307_porta_w(space, 0, data);
	}
	else
	{
		logerror("%08x m68307_internal_sim_w %04x (%04x) (Port A (8-bit) Data Register - PADAT)\n", pc, data,mem_mask);
	}
}

void m68307_sim::write_pbcnt(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pbcnt);
}

void m68307_sim::write_pbddr(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pbddr);
}

void m68307_sim::write_pbdat(address_space *space, UINT16 data, UINT16 mem_mask)
{
	int pc = cpu_get_pc(&space->device());
	m68ki_cpu_core *m68k = m68k_get_safe_token(&space->device());
	COMBINE_DATA(&m_pbdat);

	if (m68k->m_m68307_portb_w)
	{
		m68k->m_m68307_portb_w(space, 0, data, mem_mask);
	}
	else
	{
		logerror("%08x m68307_internal_sim_w %04x (%04x) (Port B (16-bit) Data Register - PBDAT)\n", pc, data,mem_mask);
	}
}

void m68307_sim::write_licr1(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_licr1);
	data = m_licr1;
	logerror("m_licr1 value %04x : Details :\n", data);
	logerror("int4ipl %01x\n", (data>>0)&7);
	logerror("pir4    %01x\n", (data>>3)&1);
	logerror("int3ipl %01x\n", (data>>4)&7);
	logerror("pir3    %01x\n", (data>>7)&1);
	logerror("int2ipl %01x\n", (data>>8)&7);
	logerror("pir2    %01x\n", (data>>11)&1);
	logerror("int1ipl %01x\n", (data>>12)&7);
	logerror("pir1    %01x\n", (data>>15)&1);
	logerror("\n");
}

void m68307_sim::write_licr2(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_licr2);
	data = m_licr2;
	logerror("m_licr2 value %04x : Details :\n", data);
	logerror("int8ipl %01x\n", (data>>0)&7);
	logerror("pir8    %01x\n", (data>>3)&1);
	logerror("int7ipl %01x\n", (data>>4)&7);
	logerror("pir7    %01x\n", (data>>7)&1);
	logerror("int6ipl %01x\n", (data>>8)&7);
	logerror("pir6    %01x\n", (data>>11)&1);
	logerror("int5ipl %01x\n", (data>>12)&7);
	logerror("pir5    %01x\n", (data>>15)&1);
	logerror("\n");
}


void m68307_sim::write_picr(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_picr);
	data = m_picr;
	logerror("picr value %04x : Details :\n", data);
	logerror("mbipl %01x\n", (data>>0)&7);
	logerror("uaipl %01x\n", (data>>4)&7);
	logerror("t2ipl %01x\n", (data>>8)&7);
	logerror("t1ipl %01x\n", (data>>12)&7);
	logerror("\n");
}

void m68307_sim::write_pivr(UINT16 data, UINT16 mem_mask)
{
	COMBINE_DATA(&m_pivr);
	data = m_pivr;
	logerror("pivr value %04x : Details :\n", data);
	logerror("unused %01x\n", (data>>0)&0xf);
	logerror("high vector %01x\n", (data>>4)&0xf);
}

void m68307_sim::reset(void)
{
	for (int i=0;i<4;i++)
	{
		m_br[i] = 0xc001;
		m_or[i] = 0xdffd;
	}
}
